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
#include <AL/alc.h>
#include <AL/al.h>
#include <ogg/ogg.h>
#include <vorbis/vorbisfile.h>

class CSoundtrackSystem
{
public:
	bool Init();
	bool LoadSoundtrack(const std::string& filename); // handles wav and ogg soundtracks
	void Play(bool loop);
	void Stop();
	void SetVolumeFromCvar();
	void Shutdown();

private:
	bool LoadWav(const std::string& filename);
	bool LoadOgg(const std::string& filename);

	ALCdevice* m_device = nullptr;
	ALCcontext* m_context = nullptr;
	ALuint m_source = 0;
	ALuint m_buffer = 0;
};

extern CSoundtrackSystem g_SoundtrackSystem;
