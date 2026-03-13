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

#pragma once

#define PUSH2_BLACK 0x0000
#define PUSH2_WHITE 0xFFFF
#define PUSH2_RED 0xF800
#define PUSH2_GREEN 0x0780
#define PUSH2_BLUE 0x001F
#define PUSH2_PHX 0xB81F

#include <cstdint>
#include <RtMidi.h>

struct libusb_context;
struct libusb_device_handle;

class CPush2Display
{
public:
	bool Init();
	void Shutdown();

	bool Present();
	void Clear(uint16_t color);

private:
	bool InitMidi();
	void ShutdownMidi();
	bool SendDisplayBrightness(unsigned int brightness);
	bool OpenMidiOutPortByName(const char* pszNeedle);

	static uint16_t Encode565(uint8_t r, uint8_t g, uint8_t b);

private:
	libusb_context* m_Context = nullptr;
	libusb_device_handle* m_Handle = nullptr;
	RtMidiOut* m_pMidiOut = nullptr;

	int m_Interface = -1;

	static constexpr int PUSH2_WIDTH = 960;
	static constexpr int PUSH2_HEIGHT = 160;
	static constexpr int PUSH2_FRAMEBUFFER_SIZE = PUSH2_WIDTH * PUSH2_HEIGHT * 2;

	uint16_t m_Framebuffer[PUSH2_WIDTH * PUSH2_HEIGHT];
};

