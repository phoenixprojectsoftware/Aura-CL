/****
*
* Copyright (c) 2021-2026 The Phoenix Project Software. Some Rights Reserved.
*
* AURA
*
* A/V STREAM
*
****/

#include "av_stream.h"

#include "../hud.h"
#include "../cl_util.h"

#include <cstring>

CAVStream::CAVStream()
{
	std::memset(&m_VorbisFile, 0, sizeof(m_VorbisFile));
	std::memset(m_Buffers, 0, sizeof(m_Buffers));
}

CAVStream::~CAVStream()
{
	Close();
}

bool CAVStream::Open(const char* pszPath)
{
	Close();

	if (!pszPath || !pszPath[0])
		return false;

	while (alGetError() != AL_NO_ERROR) {}

	if (ov_fopen(pszPath, &m_VorbisFile) < 0)
	{
		gEngfuncs.Con_Printf("CAVStream: failed to open OGG: %s\n", pszPath);
		return false;
	}

	vorbis_info* vi = ov_info(&m_VorbisFile, -1);

	if (!vi)
	{
		gEngfuncs.Con_Printf("CAVStream: ov_info failed: %s\n", pszPath);
		ov_clear(&m_VorbisFile);
		return false;
	}

	if (vi->channels == 1)
		m_Format = AL_FORMAT_MONO16;
	else if (vi->channels == 2)
		m_Format = AL_FORMAT_STEREO16;
	else
	{
		gEngfuncs.Con_Printf("CAVStream: unsupported channel count: %d\n", vi->channels);
		ov_clear(&m_VorbisFile);
		return false;
	}

	m_Frequency = static_cast<ALsizei>(vi->rate);

	alGenSources(1, &m_Source);
	alGenBuffers(NUM_BUFFERS, m_Buffers);

	if (ALenum err = alGetError(); err != AL_NO_ERROR)
	{
		gEngfuncs.Con_Printf("CAVStream: OpenAL init error %d\n", err);
		Close();
		return false;
	}

	alSourcef(m_Source, AL_GAIN, 1.0f);
	alSourcef(m_Source, AL_PITCH, 1.0f);
	alSourcei(m_Source, AL_LOOPING, AL_FALSE);

	m_bOpen = true;
	m_bPlaying = false;
	m_bFinished = false;

	gEngfuncs.Con_Printf(
		"CAVStream: opened %s (%d ch, %ld Hz)\n",
		pszPath,
		vi->channels,
		vi->rate
	);

	return true;
}

void CAVStream::Close()
{
	Stop();

	if (m_Source)
	{
		alDeleteSources(1, &m_Source);
		m_Source = 0;
	}

	if (m_Buffers[0])
	{
		alDeleteBuffers(NUM_BUFFERS, m_Buffers);
		std::memset(m_Buffers, 0, sizeof(m_Buffers));
	}

	if (m_bOpen)
	{
		ov_clear(&m_VorbisFile);
		m_bOpen = false;
	}

	m_bPlaying = false;
	m_bFinished = false;
}

void CAVStream::Play()
{
	if (!m_bOpen || !m_Source)
		return;

	if (!QueueInitialBuffers())
	{
		m_bFinished = true;
		return;
	}

	alSourcePlay(m_Source);
	m_bPlaying = true;
}

void CAVStream::Stop()
{
	if (!m_Source)
		return;

	alSourceStop(m_Source);
	ClearQueuedBuffers();

	m_bPlaying = false;
}

void CAVStream::ClearQueuedBuffers()
{
	if (!m_Source)
		return;

	ALint queued = 0;
	alGetSourcei(m_Source, AL_BUFFERS_QUEUED, &queued);

	while (queued-- > 0)
	{
		ALuint buffer = 0;
		alSourceUnqueueBuffers(m_Source, 1, &buffer);
	}
}

bool CAVStream::QueueInitialBuffers()
{
	int queued = 0;

	for (int i = 0; i < NUM_BUFFERS; ++i)
	{
		if (FillBuffer(m_Buffers[i]))
		{
			alSourceQueueBuffers(m_Source, 1, &m_Buffers[i]);
			++queued;
		}
	}

	return queued > 0;
}

bool CAVStream::FillBuffer(ALuint buffer)
{
	if (!m_bOpen || !buffer)
		return false;

	char pcm[BUFFER_BYTES];
	int totalBytes = 0;

	while (totalBytes < BUFFER_BYTES)
	{
		int bitstream = 0;

		long bytes = ov_read(
			&m_VorbisFile,
			pcm + totalBytes,
			BUFFER_BYTES - totalBytes,
			0, // little endian
			2, // 16-bit
			1, // signed
			&bitstream
		);

		if (bytes == 0)
		{
			m_bFinished = true;
			break;
		}

		if (bytes < 0)
		{
			gEngfuncs.Con_Printf("CAVStream: ov_read error %ld\n", bytes);
			continue;
		}

		totalBytes += bytes;
	}

	if (totalBytes <= 0)
		return false;

	alBufferData(buffer, m_Format, pcm, totalBytes, m_Frequency);

	if (ALenum err = alGetError(); err != AL_NO_ERROR)
	{
		gEngfuncs.Con_Printf("CAVStream: alBufferData error %d\n", err);
		return false;
	}

	return true;
}

void CAVStream::Update()
{
	if (!m_bOpen || !m_Source || !m_bPlaying)
		return;

	ALint processed = 0;
	alGetSourcei(m_Source, AL_BUFFERS_PROCESSED, &processed);

	while (processed-- > 0)
	{
		ALuint buffer = 0;
		alSourceUnqueueBuffers(m_Source, 1, &buffer);

		if (!m_bFinished && FillBuffer(buffer))
			alSourceQueueBuffers(m_Source, 1, &buffer);
	}

	ALint state = 0;
	alGetSourcei(m_Source, AL_SOURCE_STATE, &state);

	if (state != AL_PLAYING)
	{
		ALint queued = 0;
		alGetSourcei(m_Source, AL_BUFFERS_QUEUED, &queued);

		if (queued > 0 && !m_bFinished)
		{
			alSourcePlay(m_Source);
		}
		else if (m_bFinished)
		{
			m_bPlaying = false;
		}
	}
}
