/****
*
* Copyright (c) 2021-2025 The Phoenix Project Software. Some Rights Reserved.
*
* AURA
*
* openal
*
*
****/

#pragma once

#include <string>
#include <AL/al.h>
#include <AL/alc.h>

class CWavPlayer
{
public:
	bool Init();
	bool LoadWav(const std::string& filename);
	void Play();
	void Stop();
	void SetVolumeFromCvar();
	void PlayCmd();
	void StopCmd();
	void Shutdown();

private:
	ALCdevice* m_device = nullptr;
	ALCcontext* m_context = nullptr;
	ALuint m_source = 0;
	ALuint m_buffer = 0;
};

extern CWavPlayer g_WavPlayer;
