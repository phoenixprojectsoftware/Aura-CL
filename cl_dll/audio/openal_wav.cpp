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

#include <cstdio>
#include <vector>
#include <string>
#include <iostream>
#include "../hud.h"
#include "../cl_util.h"
#include "../../common/parsemsg.h"
#include "openal_wav.h"

#include <convar.h>

CSoundtrackSystem g_SoundtrackSystem;

bool CSoundtrackSystem::Init()
{
	m_device = alcOpenDevice(nullptr);
	if (!m_device)
		return false;

	m_context = alcCreateContext(m_device, nullptr);
	if (!m_context)
		return false;

	alcMakeContextCurrent(m_context);
	alGenSources(1, &m_source);
	return true;

	ALenum err = alGetError();
	if (err != AL_NO_ERROR)
		gEngfuncs.Con_Printf("OpenAL error after alGenSources: %d\n", err);
}

bool CSoundtrackSystem::LoadSoundtrack(const std::string& filename)
{
    std::string lower = filename;
    for (auto& c : lower) c = tolower(c);

	if (lower.find(".ogg") != std::string::npos || lower.find(".rts") != std::string::npos) // .rts is also ogg
        return LoadOgg(filename);
    else
        return LoadWav(filename);
}

bool CSoundtrackSystem::LoadWav(const std::string& filename)
{

    FILE* file = fopen(filename.c_str(), "rb");
    if (!file)
    {
        gEngfuncs.Con_Printf("CSoundtrackSystem: Failed to open file: %s\n", filename.c_str());
        return false;
    }

    char riff[4];
    fread(riff, 1, 4, file);
    if (strncmp(riff, "RIFF", 4) != 0)
    {
        gEngfuncs.Con_Printf("CSoundtrackSystem: Not a RIFF file: %s\n", filename.c_str());
        fclose(file);
        return false;
    }

    fseek(file, 8, SEEK_SET);
    char wave[4];
    fread(wave, 1, 4, file);
    if (strncmp(wave, "WAVE", 4) != 0)
    {
        gEngfuncs.Con_Printf("CSoundtrackSystem: Not a WAVE file: %s\n", filename.c_str());
        fclose(file);
        return false;
    }

    // Variables for fmt and data
    short channels = 0;
    int sampleRate = 0;
    short bitsPerSample = 0;
    std::vector<char> audioData;

    // Scan chunks
    while (!feof(file))
    {
        char chunkId[4];
        int chunkSize = 0;
        if (fread(chunkId, 1, 4, file) != 4) break;
        if (fread(&chunkSize, sizeof(int), 1, file) != 1) break;

        if (strncmp(chunkId, "fmt ", 4) == 0)
        {
            short audioFormat;
            fread(&audioFormat, sizeof(short), 1, file);
            fread(&channels, sizeof(short), 1, file);
            fread(&sampleRate, sizeof(int), 1, file);
            int byteRate;
            fread(&byteRate, sizeof(int), 1, file);
            short blockAlign;
            fread(&blockAlign, sizeof(short), 1, file);
            fread(&bitsPerSample, sizeof(short), 1, file);

            if (audioFormat != 1) // PCM
            {
                gEngfuncs.Con_Printf("CSoundtrackSystem: Unsupported WAV format: %d\n", audioFormat);
                fclose(file);
                return false;
            }

            // Skip any remaining bytes in fmt chunk
            if (chunkSize > 16)
                fseek(file, chunkSize - 16, SEEK_CUR);
        }
        else if (strncmp(chunkId, "data", 4) == 0)
        {
            audioData.resize(chunkSize);
            if (fread(audioData.data(), 1, chunkSize, file) != (size_t)chunkSize)
            {
                gEngfuncs.Con_Printf("CSoundtrackSystem: Failed to read WAV data\n");
                fclose(file);
                return false;
            }
            break; // data found
        }
        else
        {
            // Unknown chunk, skip
            fseek(file, chunkSize, SEEK_CUR);
        }
    }

    fclose(file);

    if (audioData.empty())
    {
        gEngfuncs.Con_Printf("CSoundtrackSystem: No audio data found in %s\n", filename.c_str());
        return false;
    }

    // Determine OpenAL format
    ALenum format = 0;
    if (channels == 1 && bitsPerSample == 8) format = AL_FORMAT_MONO8;
    else if (channels == 1 && bitsPerSample == 16) format = AL_FORMAT_MONO16;
    else if (channels == 2 && bitsPerSample == 8) format = AL_FORMAT_STEREO8;
    else if (channels == 2 && bitsPerSample == 16) format = AL_FORMAT_STEREO16;
    else
    {
        gEngfuncs.Con_Printf("CSoundtrackSystem: Unsupported WAV format: %d channels, %d bits\n", channels, bitsPerSample);
        return false;
    }

    // Generate buffer and source
    alGenBuffers(1, &m_buffer);
    alBufferData(m_buffer, format, audioData.data(), static_cast<ALsizei>(audioData.size()), sampleRate);

    ALenum err = alGetError();
    if (err != AL_NO_ERROR)
    {
        gEngfuncs.Con_Printf("CSoundtrackSystem: OpenAL error %d on buffer data\n", err);
        return false;
    }

    alSourcei(m_source, AL_BUFFER, m_buffer);

    err = alGetError();
    if (err != AL_NO_ERROR)
    {
        gEngfuncs.Con_Printf("CSoundtrackSystem: OpenAL error %d on source bind\n", err);
        return false;
    }

    gEngfuncs.Con_Printf("CSoundtrackSystem: Loaded WAV file: %s\n", filename.c_str());
    return true;
}

bool CSoundtrackSystem::LoadOgg(const std::string& filename)
{
    OggVorbis_File vf;
    if (ov_fopen(filename.c_str(), &vf) < 0)
    {
		gEngfuncs.Con_Printf("CSoundtrackSystem: Failed to open OGG file: %s\n", filename.c_str());
        return false;
    }

    vorbis_info* info = ov_info(&vf, -1);
    int channels = info->channels;
	long sampleRate = info->rate;
    ALenum format = (channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

    std::vector<char> pcm;
	char buffer[4096];
    int bitstream;

    long bytes;
    while ((bytes = ov_read(&vf, buffer, sizeof(buffer), 0,2,1, &bitstream)) > 0)
		pcm.insert(pcm.end(), buffer, buffer + bytes);

    ov_clear(&vf);

    if (pcm.empty())
    {
		gEngfuncs.Con_Printf("CSoundtrackSystem: No audio data found in %s\n", filename.c_str());
		return false;
    }

    if (m_buffer)
		alDeleteBuffers(1, &m_buffer);

    alGenBuffers(1, &m_buffer);
    alBufferData(m_buffer, format, pcm.data(), (ALsizei)pcm.size(), sampleRate);
    alSourcei(m_source, AL_BUFFER, m_buffer);

	gEngfuncs.Con_Printf("CSoundtrackSystem: Loaded OGG file: %s\n", filename.c_str());
    return true;
}

void CSoundtrackSystem::Play(bool loop)
{
	alSourcei(m_source, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
    SetVolumeFromCvar();
    alSourcePlay(m_source);
}

void CSoundtrackSystem::Stop()
{
	alSourceStop(m_source);
}

void CSoundtrackSystem::SetVolumeFromCvar()
{
	cvar_t* mp3_volume = gEngfuncs.pfnGetCvarPointer("MP3Volume");

	float vol = 1.0f;
	if (mp3_volume)
		vol = mp3_volume->value;

	if (vol < 0.0f) vol = 0.0f;
	if (vol > 1.0f) vol = 1.0f;

	alSourcef(m_source, AL_GAIN, vol);
}

void CSoundtrackSystem::Shutdown()
{
	alDeleteSources(1, &m_source);
	alDeleteBuffers(1, &m_buffer);
	alcDestroyContext(m_context);
	alcCloseDevice(m_device);
}

CON_COMMAND(soundtrack_play, "file.ext [loop]")
{
    if (!g_SoundtrackSystem.Init())
    {
        gEngfuncs.Con_Printf("soundtrack_play: Failed to initialize OpenAL\n");
		return;
    }

    if (gEngfuncs.Cmd_Argc() < 2)
    {
        gEngfuncs.Con_Printf("Usage: soundtrack_play <file.ext> [loop]\n");
        return;
	}

    std::string file = gEngfuncs.Cmd_Argv(1);
	std::string path = std::string(gEngfuncs.pfnGetGameDirectory()) + "/sound/" + file;

    if (!g_SoundtrackSystem.LoadSoundtrack(path))
        return;

	bool loop = (gEngfuncs.Cmd_Argc() >= 3 && !stricmp(gEngfuncs.Cmd_Argv(2), "loop"));
	g_SoundtrackSystem.Play(loop);
}

CON_COMMAND(soundtrack_stop, "Stop soundtrack")
{
    g_SoundtrackSystem.Stop();
}