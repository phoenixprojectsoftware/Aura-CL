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

#pragma once

#include <cstdio>

#include <ogg/ogg.h>
#include <theora/theoradec.h>

class CTheoraDecoder
{
public:
	CTheoraDecoder();
	~CTheoraDecoder();

	bool Open(const char* pszPath);
	void Close();

	bool DecodeToTime(double flTime, unsigned char* pRGBAOut);

	bool IsFinished() const { return m_bFinished; }

	int GetWidth() const { return m_iFrameWidth; }
	int GetHeight() const { return m_iFrameHeight; }
	double GetFPS() const { return m_flFPS; }

private:
	bool BufferData();
	bool ParseHeaders();
	bool QueuePage(ogg_page* pPage);
	bool DecodeNextFrame(unsigned char* pRGBAOut, double& flFrameTime);
	void ConvertYUVToRGBA(const th_ycbcr_buffer yuv, unsigned char* pRGBAOut);

private:
	FILE* m_pFile = nullptr;

	ogg_sync_state m_oy;
	ogg_page m_og;
	ogg_packet m_op;

	ogg_stream_state m_to;

	th_info m_ti;
	th_comment m_tc;
	th_setup_info* m_pSetup = nullptr;
	th_dec_ctx* m_pDecoder = nullptr;

	bool m_bOggSyncInit = false;
	bool m_bTheoraStreamInit = false;
	bool m_bHeadersDone = false;
	bool m_bFinished = false;

	int m_iTheoraSerial = -1;

	int m_iFrameWidth = 0;
	int m_iFrameHeight = 0;

	double m_flFPS = 30.0;
	double m_flLastFrameTime = 0.0;

	long long m_iFrameIndex = 0;
};
