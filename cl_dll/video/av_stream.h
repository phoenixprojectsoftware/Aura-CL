/****
*
* Copyright (c) 2021-2026 The Phoenix Project Software. Some Rights Reserved.
*
* AURA
*
* A/V STREAM
*
*
****/

#pragma once

#include <string>
#include <vorbis/vorbisfile.h>
#include <AL/al.h>

class CAVStream
{
public:
	CAVStream();
	~CAVStream();

	bool Open(const char* pszPath);
	void Close();

	void Play();
	void Stop();
	void Update();

	bool IsPlaying() const { return m_bPlaying; }
	bool IsFinished() const { return m_bFinished; }

private:
	bool FillBuffer(ALuint buffer);
	bool QueueInitialBuffers();
	void ClearQueuedBuffers();

private:
	static constexpr int NUM_BUFFERS = 4;
	static constexpr int BUFFER_BYTES = 64 * 1024;

	OggVorbis_File m_VorbisFile{};

	bool m_bOpen = false;
	bool m_bPlaying = false;
	bool m_bFinished = false;

	ALuint m_Source = 0;
	ALuint m_Buffers[NUM_BUFFERS] = {};

	ALenum m_Format = AL_FORMAT_STEREO16;
	ALsizei m_Frequency = 44100;
};
