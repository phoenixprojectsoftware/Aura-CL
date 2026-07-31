/****
*
* Copyright (c) 2021-2026 The Phoenix Project Software. Some Rights Reserved.
*
* AURA
*
* VIDEO PLAYER
*
*
****/

#include "../hud.h"
#include "../cl_util.h"

#include "video_player.h"
#include "theora_decoder.h"
#include "av_stream.h"

#ifdef _WIN32
#include "../../common/winsani_in.h"
#include <Windows.h>
#include "../../common/winsani_out.h"
#endif

#include <gl/GL.h>
#include <cstring>

CVideoPlayer gVideoPlayer;

bool CVideoPlayer::Init()
{
	return true;
}

void CVideoPlayer::Shutdown()
{
	Stop();
	DestroyTexture();
}

bool CVideoPlayer::Play(const char* pszVideoPath, const char* pszAudioPath)
{
	Stop();

	if (!pszVideoPath || !pszVideoPath[0])
		return false;

	m_pDecoder = new CTheoraDecoder();

	if (!m_pDecoder->Open(pszVideoPath))
	{
		delete m_pDecoder;
		m_pDecoder = nullptr;
		return false;
	}

	m_iFrameWide = m_pDecoder->GetWidth();
	m_iFrameTall = m_pDecoder->GetHeight();

	if (m_iFrameWide <= 0 || m_iFrameTall <= 0)
	{
		Stop();
		return false;
	}

	m_pRGBA = new unsigned char[m_iFrameWide * m_iFrameTall * 4];
	std::memset(m_pRGBA, 0, m_iFrameWide * m_iFrameTall * 4);

	if (pszAudioPath && pszAudioPath[0])
	{
		m_pAudio = new CAVStream();

		if (m_pAudio->Open(pszAudioPath))
		{
			m_pAudio->Play();
		}
		else
		{
			delete m_pAudio;
			m_pAudio = nullptr;

			gEngfuncs.Con_Printf("CVideoPlayer: continuing without audio\n");
		}
	}

	m_bPlaying = true;
	m_flStartTime = gHUD.m_flTime;

	CreateTextureIfNeeded();

	return true;
}

void CVideoPlayer::Stop()
{
	m_bPlaying = false;

	if (m_pAudio)
	{
		m_pAudio->Close();
		delete m_pAudio;
		m_pAudio = nullptr;
	}

	if (m_pDecoder)
	{
		m_pDecoder->Close();
		delete m_pDecoder;
		m_pDecoder = nullptr;
	}

	if (m_pRGBA)
	{
		delete[] m_pRGBA;
		m_pRGBA = nullptr;
	}

	m_iFrameTall = 0;
	m_iFrameWide = 0;
}

void CVideoPlayer::Update(float flClientTime)
{
	if (!m_bPlaying || !m_pDecoder)
		return;

	if (m_pAudio)
		m_pAudio->Update();

	const double flVideoTime = static_cast<double>(flClientTime - m_flStartTime);

	const bool bDecodedNewFrame = m_pDecoder->DecodeToTime(flVideoTime, m_pRGBA);

	if (bDecodedNewFrame)
	{
		UploadFrame();
		return;
	}

	if (m_pDecoder->IsFinished())
		Stop();
}

void CVideoPlayer::CreateTextureIfNeeded()
{
	if (m_iTexture != 0)
		return;

	glGenTextures(1, &m_iTexture);
	glBindTexture(GL_TEXTURE_2D, m_iTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	m_iTexWide = m_iFrameWide;
	m_iTexTall = m_iFrameTall;

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		m_iTexWide,
		m_iTexTall,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		m_pRGBA
	);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void CVideoPlayer::DestroyTexture()
{
	if (m_iTexture)
	{
		glDeleteTextures(1, &m_iTexture);
		m_iTexture = 0;
	}

	m_iTexWide = 0;
	m_iTexTall = 0;
}

bool CVideoPlayer::UploadFrame()
{
	if (!m_bPlaying || !m_pRGBA || m_iTexture == 0)
		return false;

	glBindTexture(GL_TEXTURE_2D, m_iTexture);

	glTexSubImage2D(
		GL_TEXTURE_2D,
		0,
		0,
		0,
		m_iFrameWide,
		m_iFrameTall,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		m_pRGBA
	);

	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}

void CVideoPlayer::Draw()
{
	if (!m_bPlaying || m_iTexture == 0)
		return;

	const int screenWide = ScreenWidth;
	const int screenTall = ScreenHeight;

	glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TEXTURE_BIT);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_iTexture);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, screenWide, screenTall, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex2f((float)screenWide, 0.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex2f((float)screenWide, (float)screenTall);
	glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, (float)screenTall);
	glEnd();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, 0);

	glPopAttrib();
}
