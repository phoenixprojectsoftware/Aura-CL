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
#include <queue>
#include <AL/alc.h>
#include <AL/al.h>
#include <ogg/ogg.h>
#include <vorbis/vorbisfile.h>

#if defined(_WIN32) && !defined(FOUNDATION)
#include <winsani_in.h>
#include <Windows.h>
#include <winsani_out.h>
#endif

struct QueuedTrack
{
	std::string path;
	bool loop;
};

class CSoundtrackSystem
{
public:
	CSoundtrackSystem();

	bool Init();
	bool LoadSoundtrack(const std::string& filename); // handles wav and ogg soundtracks
	void Play(bool loop);
	void Stop();
	void SetVolumeFromCvar();
	void Shutdown();
	void Update();

	void PauseMusic();
	void ResumeMusic();

	void Queue(const std::string& filename, bool loop = false);

	ALuint m_source = 0;

	std::queue<QueuedTrack> m_queue;
	bool m_loop = false; // loop mode of the currently playing track in the queue

	std::string m_currentPath;

private:
	bool LoadWav(const std::string& filename);
	bool LoadOgg(const std::string& filename);
	bool LoadMp3(const std::string& filename);

	ALCdevice* m_device = nullptr;
	ALCcontext* m_context = nullptr;
	ALuint m_buffer = 0;

	bool m_paused = false;
};

extern CSoundtrackSystem g_SoundtrackSystem;
