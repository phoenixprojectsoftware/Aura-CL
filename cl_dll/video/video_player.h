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

#pragma once

class CTheoraDecoder;
class CAVStream;

class CVideoPlayer
{
public:
	bool Init();
	void Shutdown();

	bool Play(const char* pszVideoPath, const char* pszAudioPath = nullptr);
	void Stop();

	void Update(float flClientTime);
	void Draw();

	bool IsPlaying() const { return m_bPlaying; }

private:
	bool UploadFrame();
	void CreateTextureIfNeeded();
	void DestroyTexture();

private:
	CTheoraDecoder* m_pDecoder = nullptr;
	CAVStream* m_pAudio = nullptr;

	bool m_bPlaying = false;
	float m_flStartTime = 0.0f;

	unsigned int m_iTexture = 0;
	int m_iTexWide = 0;
	int m_iTexTall = 0;

	unsigned char* m_pRGBA = nullptr;
	int m_iFrameWide = 0;
	int m_iFrameTall = 0;
};

extern CVideoPlayer gVideoPlayer;
