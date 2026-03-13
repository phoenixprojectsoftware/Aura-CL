/****
*
* Copyright (c) 2021-2026 The Phoenix Project Software. Some Rights Reserved.
*
* AURA
*
* Ableton Push2 display.
*
*
****/

#include "push2_display.h"

#ifdef _WIN32
#include "../../common/winsani_in.h"
#include "../engineclientcmd.h"
#include "../../common/winsani_out.h"
#else
#include "../engineclientcmd.h"
#endif

#include <libusb.h>
#include <RtMidi.h>

#include <vector>
#include <string>
#include <cstring>

#define PUSH2_VENDOR_ID  0x2982
#define PUSH2_PRODUCT_ID 0x1967
#define PUSH2_ENDPOINT_BULK_OUT 0x01

static const unsigned char g_Push2FrameHeader[16] =
{
	0xFF, 0xCC, 0xAA, 0x88,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
};

bool CPush2Display::Init()
{
	if (libusb_init(&m_Context) != 0)
	{
		gEngfuncs.Con_Printf("Push2: libusb_init failed\n");
		return false;
	}

	m_Handle = libusb_open_device_with_vid_pid(m_Context, PUSH2_VENDOR_ID, PUSH2_PRODUCT_ID);

	if (!m_Handle)
	{
		gEngfuncs.Con_DPrintf("Push2: device not found\n");
		return false;
	}

	gEngfuncs.Con_DPrintf("Push2: USB device detected\n");

	libusb_set_auto_detach_kernel_driver(m_Handle, 1);

	m_Interface = 0;

	if (libusb_claim_interface(m_Handle, m_Interface) != 0)
	{
		gEngfuncs.Con_DPrintf("Push2: failed to claim USB interface 0\n");
		libusb_close(m_Handle);
		m_Handle = nullptr;
		return false;
	}

	if (!InitMidi())
	{
		gEngfuncs.Con_DPrintf("Push2: MIDI output init failed\n");
		// Not fatal for USB open, but the screen backlight may stay off.
	}
	else
	{
		SendDisplayBrightness(255);
	}

	Clear(Encode565(255, 0, 255)); // purple sanity test
	Present();

	gEngfuncs.Con_DPrintf("Push2: init complete\n");
	return true;
}

void CPush2Display::Shutdown()
{
	ShutdownMidi();

	if (m_Handle)
	{
		if (m_Interface >= 0)
			libusb_release_interface(m_Handle, m_Interface);

		libusb_close(m_Handle);
		m_Handle = nullptr;
	}

	if (m_Context)
	{
		libusb_exit(m_Context);
		m_Context = nullptr;
	}

	m_Interface = -1;
}

bool CPush2Display::InitMidi()
{
	try
	{
		m_pMidiOut = new RtMidiOut();
	}
	catch (...)
	{
		m_pMidiOut = nullptr;
		return false;
	}

	return OpenMidiOutPortByName("Push");
}

void CPush2Display::ShutdownMidi()
{
	if (m_pMidiOut)
	{
		try
		{
			m_pMidiOut->closePort();
		}
		catch (...)
		{
		}

		delete m_pMidiOut;
		m_pMidiOut = nullptr;
	}
}

bool CPush2Display::OpenMidiOutPortByName(const char* pszNeedle)
{
	if (!m_pMidiOut)
		return false;

	const unsigned int count = m_pMidiOut->getPortCount();

	for (unsigned int i = 0; i < count; ++i)
	{
		std::string name = m_pMidiOut->getPortName(i);

		if (name.find(pszNeedle) != std::string::npos)
		{
			m_pMidiOut->openPort(i);
			gEngfuncs.Con_DPrintf("Push2: opened MIDI out port: %s\n", name.c_str());
			return true;
		}
	}

	gEngfuncs.Con_DPrintf("Push2: no matching MIDI out port found\n");
	return false;
}

bool CPush2Display::SendDisplayBrightness(unsigned int brightness)
{
	if (!m_pMidiOut)
		return false;

	if (brightness > 255)
		brightness = 255;

	// Push 2 display brightness SysEx:
	// F0 00 21 1D 01 01 08 <LSB 7 bits> <MSB 1 bit> F7
	std::vector<unsigned char> msg;
	msg.reserve(10);

	msg.push_back(0xF0);
	msg.push_back(0x00);
	msg.push_back(0x21);
	msg.push_back(0x1D);
	msg.push_back(0x01);
	msg.push_back(0x01);
	msg.push_back(0x08);
	msg.push_back(static_cast<unsigned char>(brightness & 0x7F));
	msg.push_back(static_cast<unsigned char>((brightness >> 7) & 0x01));
	msg.push_back(0xF7);

	try
	{
		m_pMidiOut->sendMessage(&msg);
		gEngfuncs.Con_DPrintf("Push2: display brightness set to %u\n", brightness);
		return true;
	}
	catch (...)
	{
		gEngfuncs.Con_DPrintf("Push2: failed to send brightness SysEx\n");
		return false;
	}
}

bool CPush2Display::Present()
{
	if (!m_Handle)
		return false;

	// 16-byte frame header + 160 lines * 2048 bytes
	std::vector<unsigned char> packet;
	packet.resize(16 + (PUSH2_HEIGHT * 2048), 0);

	// Header
	std::memcpy(packet.data(), g_Push2FrameHeader, sizeof(g_Push2FrameHeader));

	// Payload begins after 16-byte header
	unsigned char* pPayload = packet.data() + 16;

	for (int y = 0; y < PUSH2_HEIGHT; ++y)
	{
		unsigned char* pLine = pPayload + (y * 2048);

		// First 1920 bytes = 960 pixels * 2 bytes
		for (int x = 0; x < PUSH2_WIDTH; ++x)
		{
			const uint16_t pixel = m_Framebuffer[(y * PUSH2_WIDTH) + x];

			// Little-endian write
			pLine[x * 2 + 0] = static_cast<unsigned char>(pixel & 0xFF);
			pLine[x * 2 + 1] = static_cast<unsigned char>((pixel >> 8) & 0xFF);
		}

		// Last 128 bytes remain 0 padding.
		// NOTE: XOR transform not yet applied here.
	}

	int transferred = 0;
	const int totalSize = static_cast<int>(packet.size());

	const int result = libusb_bulk_transfer(
		m_Handle,
		PUSH2_ENDPOINT_BULK_OUT,
		packet.data(),
		totalSize,
		&transferred,
		0
	);

	if (result != 0 || transferred != totalSize)
	{
		gEngfuncs.Con_DPrintf("Push2: bulk transfer failed result=%d transferred=%d expected=%d\n",
			result, transferred, totalSize);
		return false;
	}

	return true;
}

void CPush2Display::Clear(uint16_t color)
{
	for (int i = 0; i < PUSH2_WIDTH * PUSH2_HEIGHT; ++i)
		m_Framebuffer[i] = color;
}

uint16_t CPush2Display::Encode565(uint8_t r, uint8_t g, uint8_t b)
{
	return static_cast<uint16_t>(
		((r >> 3) << 11) |
		((g >> 2) << 5) |
		((b >> 3) << 0)
		);
}
