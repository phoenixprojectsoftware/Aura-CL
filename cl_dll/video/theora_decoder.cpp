/****
*
* Copyright (c) 2021-2026 The Phoenix Project Software. Some Rights Reserved.
*
* AURA
*
* THEORA DECODER
*
*
****/

#include "theora_decoder.h"

#include <cstring>
#include <cmath>
#include <algorithm>

static unsigned char ClampByte(int value)
{
	if (value < 0)
		return 0;

	if (value > 255)
		return 255;

	return static_cast<unsigned char>(value);
}

CTheoraDecoder::CTheoraDecoder()
{
	std::memset(&m_oy, 0, sizeof(m_oy));
	std::memset(&m_og, 0, sizeof(m_og));
	std::memset(&m_op, 0, sizeof(m_op));
	std::memset(&m_to, 0, sizeof(m_to));

	th_info_init(&m_ti);
	th_comment_init(&m_tc);
}

CTheoraDecoder::~CTheoraDecoder()
{
	Close();

	th_comment_clear(&m_tc);
	th_info_clear(&m_ti);
}

bool CTheoraDecoder::Open(const char* pszPath)
{
	Close();

	if (!pszPath || !pszPath[0])
		return false;

	m_pFile = std::fopen(pszPath, "rb");

	if (!m_pFile)
		return false;

	ogg_sync_init(&m_oy);
	m_bOggSyncInit = true;

	th_info_clear(&m_ti);
	th_comment_clear(&m_tc);

	th_info_init(&m_ti);
	th_comment_init(&m_tc);

	m_pSetup = nullptr;
	m_pDecoder = nullptr;

	m_bHeadersDone = false;
	m_bFinished = false;
	m_iTheoraSerial = -1;
	m_iFrameIndex = 0;
	m_flLastFrameTime = 0.0;

	if (!ParseHeaders())
	{
		Close();
		return false;
	}

	m_pDecoder = th_decode_alloc(&m_ti, m_pSetup);

	if (!m_pDecoder)
	{
		Close();
		return false;
	}

	m_iFrameWidth = m_ti.pic_width;
	m_iFrameHeight = m_ti.pic_height;

	if (m_ti.fps_denominator != 0)
		m_flFPS = static_cast<double>(m_ti.fps_numerator) / static_cast<double>(m_ti.fps_denominator);
	else
		m_flFPS = 30.0;

	m_bHeadersDone = true;

	return true;
}

void CTheoraDecoder::Close()
{
	if (m_pDecoder)
	{
		th_decode_free(m_pDecoder);
		m_pDecoder = nullptr;
	}

	if (m_pSetup)
	{
		th_setup_free(m_pSetup);
		m_pSetup = nullptr;
	}

	if (m_bTheoraStreamInit)
	{
		ogg_stream_clear(&m_to);
		m_bTheoraStreamInit = false;
	}

	if (m_bOggSyncInit)
	{
		ogg_sync_clear(&m_oy);
		m_bOggSyncInit = false;
	}

	if (m_pFile)
	{
		std::fclose(m_pFile);
		m_pFile = nullptr;
	}

	m_bHeadersDone = false;
	m_bFinished = false;
	m_iTheoraSerial = -1;
	m_iFrameWidth = 0;
	m_iFrameHeight = 0;
	m_iFrameIndex = 0;
	m_flLastFrameTime = 0.0;
}

bool CTheoraDecoder::BufferData()
{
	if (!m_pFile)
		return false;

	char* pBuffer = ogg_sync_buffer(&m_oy, 4096);

	const size_t bytes = std::fread(pBuffer, 1, 4096, m_pFile);

	ogg_sync_wrote(&m_oy, static_cast<long>(bytes));

	if (bytes == 0)
	{
		m_bFinished = true;
		return false;
	}

	return true;
}

bool CTheoraDecoder::ParseHeaders()
{
	int theoraHeaders = 0;

	while (theoraHeaders < 3)
	{
		int result = ogg_sync_pageout(&m_oy, &m_og);

		if (result == 0)
		{
			if (!BufferData())
				return false;

			continue;
		}

		if (result < 0)
			continue;

		if (!ogg_page_bos(&m_og))
		{
			if (m_bTheoraStreamInit)
				ogg_stream_pagein(&m_to, &m_og);

			break;
		}

		ogg_stream_state testStream;
		std::memset(&testStream, 0, sizeof(testStream));

		if (ogg_stream_init(&testStream, ogg_page_serialno(&m_og)) != 0)
			return false;

		ogg_stream_pagein(&testStream, &m_og);

		if (ogg_stream_packetout(&testStream, &m_op) == 1)
		{
			int headerResult = th_decode_headerin(&m_ti, &m_tc, &m_pSetup, &m_op);

			if (headerResult >= 0)
			{
				m_iTheoraSerial = ogg_page_serialno(&m_og);

				if (ogg_stream_init(&m_to, m_iTheoraSerial) != 0)
				{
					ogg_stream_clear(&testStream);
					return false;
				}

				m_bTheoraStreamInit = true;

				ogg_stream_clear(&testStream);

				ogg_stream_pagein(&m_to, &m_og);

				while (theoraHeaders < 3)
				{
					int packetResult = ogg_stream_packetout(&m_to, &m_op);

					if (packetResult == 1)
					{
						int decodeResult = th_decode_headerin(&m_ti, &m_tc, &m_pSetup, &m_op);

						if (decodeResult >= 0)
							++theoraHeaders;

						continue;
					}

					if (packetResult == 0)
						break;

					if (packetResult < 0)
						continue;
				}

				break;
			}
		}

		ogg_stream_clear(&testStream);
	}

	while (theoraHeaders < 3)
	{
		int result = ogg_sync_pageout(&m_oy, &m_og);

		if (result == 0)
		{
			if (!BufferData())
				return false;

			continue;
		}

		if (result < 0)
			continue;

		if (ogg_page_serialno(&m_og) != m_iTheoraSerial)
			continue;

		ogg_stream_pagein(&m_to, &m_og);

		while (theoraHeaders < 3)
		{
			int packetResult = ogg_stream_packetout(&m_to, &m_op);

			if (packetResult == 1)
			{
				int decodeResult = th_decode_headerin(&m_ti, &m_tc, &m_pSetup, &m_op);

				if (decodeResult >= 0)
					++theoraHeaders;

				continue;
			}

			if (packetResult == 0)
				break;

			if (packetResult < 0)
				continue;
		}
	}

	return theoraHeaders >= 3;
}

bool CTheoraDecoder::DecodeToTime(double flTime, unsigned char* pRGBAOut)
{
	if (!m_bHeadersDone || !m_pDecoder || !pRGBAOut)
		return false;

	if (m_flFPS <= 0.0)
		m_flFPS = 30.0;

	bool decodedAny = false;

	while (!m_bFinished)
	{
		// m_iFrameIndex is the next frame we would decode.
		const double nextFrameTime = static_cast<double>(m_iFrameIndex) / m_flFPS;

		// Not time for the next frame yet.
		// Keep displaying the current texture.
		if (flTime + 0.0001 < nextFrameTime)
			break;

		double decodedFrameTime = 0.0;

		if (!DecodeNextFrame(pRGBAOut, decodedFrameTime))
			break;

		decodedAny = true;
		m_flLastFrameTime = decodedFrameTime;
	}

	return decodedAny;
}

bool CTheoraDecoder::DecodeNextFrame(unsigned char* pRGBAOut, double& flFrameTime)
{
	if (!m_pDecoder || !pRGBAOut)
		return false;

	while (!m_bFinished)
	{
		int packetResult = ogg_stream_packetout(&m_to, &m_op);

		if (packetResult == 1)
		{
			if (th_decode_packetin(m_pDecoder, &m_op, nullptr) == 0)
			{
				th_ycbcr_buffer yuv;

				if (th_decode_ycbcr_out(m_pDecoder, yuv) == 0)
				{
					ConvertYUVToRGBA(yuv, pRGBAOut);

					flFrameTime = static_cast<double>(m_iFrameIndex) / m_flFPS;
					++m_iFrameIndex;

					return true;
				}
			}

			continue;
		}

		if (packetResult < 0)
			continue;

		int pageResult = ogg_sync_pageout(&m_oy, &m_og);

		if (pageResult == 1)
		{
			if (ogg_page_serialno(&m_og) == m_iTheoraSerial)
				ogg_stream_pagein(&m_to, &m_og);

			continue;
		}

		if (pageResult < 0)
			continue;

		if (!BufferData())
			return false;
	}

	return false;
}

void CTheoraDecoder::ConvertYUVToRGBA(const th_ycbcr_buffer yuv, unsigned char* pRGBAOut)
{
	const int width = m_iFrameWidth;
	const int height = m_iFrameHeight;

	const unsigned char* yPlane = yuv[0].data;
	const unsigned char* uPlane = yuv[1].data;
	const unsigned char* vPlane = yuv[2].data;

	const int yStride = yuv[0].stride;
	const int uStride = yuv[1].stride;
	const int vStride = yuv[2].stride;

	for (int y = 0; y < height; ++y)
	{
		unsigned char* dst = pRGBAOut + y * width * 4;

		for (int x = 0; x < width; ++x)
		{
			const int Y = yPlane[y * yStride + x];

			// Most Theora content is 4:2:0, hence x / 2 and y / 2.
			// This simple converter assumes 4:2:0. We can expand this later.
			const int U = uPlane[(y / 2) * uStride + (x / 2)] - 128;
			const int V = vPlane[(y / 2) * vStride + (x / 2)] - 128;

			const int C = Y - 16;
			const int D = U;
			const int E = V;

			const int R = (298 * C + 409 * E + 128) >> 8;
			const int G = (298 * C - 100 * D - 208 * E + 128) >> 8;
			const int B = (298 * C + 516 * D + 128) >> 8;

			dst[x * 4 + 0] = ClampByte(R);
			dst[x * 4 + 1] = ClampByte(G);
			dst[x * 4 + 2] = ClampByte(B);
			dst[x * 4 + 3] = 255;
		}
	}
}
