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

CWavPlayer g_WavPlayer;

bool CWavPlayer::Init()
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

bool CWavPlayer::LoadWav(const std::string& filename)
{
    // Construct full path: <gamedir>/sound/<filename>
    const char* gamedir = gEngfuncs.pfnGetGameDirectory();
    std::string path = filename;

    FILE* file = fopen(path.c_str(), "rb");
    if (!file)
    {
        gEngfuncs.Con_Printf("CWavPlayer: Failed to open file: %s\n", path.c_str());
        return false;
    }

    char riff[4];
    fread(riff, 1, 4, file);
    if (strncmp(riff, "RIFF", 4) != 0)
    {
        gEngfuncs.Con_Printf("CWavPlayer: Not a RIFF file: %s\n", path.c_str());
        fclose(file);
        return false;
    }

    fseek(file, 8, SEEK_SET);
    char wave[4];
    fread(wave, 1, 4, file);
    if (strncmp(wave, "WAVE", 4) != 0)
    {
        gEngfuncs.Con_Printf("CWavPlayer: Not a WAVE file: %s\n", path.c_str());
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
                gEngfuncs.Con_Printf("CWavPlayer: Unsupported WAV format: %d\n", audioFormat);
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
                gEngfuncs.Con_Printf("CWavPlayer: Failed to read WAV data\n");
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
        gEngfuncs.Con_Printf("CWavPlayer: No audio data found in %s\n", path.c_str());
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
        gEngfuncs.Con_Printf("CWavPlayer: Unsupported WAV format: %d channels, %d bits\n", channels, bitsPerSample);
        return false;
    }

    // Generate buffer and source
    alGenBuffers(1, &m_buffer);
    alBufferData(m_buffer, format, audioData.data(), static_cast<ALsizei>(audioData.size()), sampleRate);

    ALenum err = alGetError();
    if (err != AL_NO_ERROR)
    {
        gEngfuncs.Con_Printf("CWavPlayer: OpenAL error %d on buffer data\n", err);
        return false;
    }

    alSourcei(m_source, AL_BUFFER, m_buffer);

    err = alGetError();
    if (err != AL_NO_ERROR)
    {
        gEngfuncs.Con_Printf("CWavPlayer: OpenAL error %d on source bind\n", err);
        return false;
    }

    gEngfuncs.Con_Printf("CWavPlayer: Loaded WAV file: %s\n", path.c_str());
    return true;
}

void CWavPlayer::Play()
{
	alSourcePlay(m_source);
}

void CWavPlayer::Stop()
{
	alSourceStop(m_source);
}

void CWavPlayer::SetVolumeFromCvar()
{
	cvar_t* mp3_volume = gEngfuncs.pfnGetCvarPointer("MP3Volume");

	float vol = 1.0f;
	if (mp3_volume)
		vol = mp3_volume->value;

	if (vol < 0.0f) vol = 0.0f;
	if (vol > 1.0f) vol = 1.0f;

	alSourcef(m_source, AL_GAIN, vol);
}

void CWavPlayer::PlayCmd()
{
	if (!g_WavPlayer.Init())
	{
		gEngfuncs.Con_Printf("WHAT THE FUCK?!?\n");
		return;
	}

	if (gEngfuncs.Cmd_Argc() < 2)
	{
		gEngfuncs.Con_Printf("Usage: al_play <filename.wav> [play|loop]\n");
		return;
	}

	const char* filename = gEngfuncs.Cmd_Argv(1);
	const char* mode = (gEngfuncs.Cmd_Argc() >= 3) ? gEngfuncs.Cmd_Argv(2) : "play";

	std::string path;
	std::string snd = "sound/";
	const char* gamedir = gEngfuncs.pfnGetGameDirectory();
	path = std::string(gamedir) + "/" + snd + filename;

	if (!g_WavPlayer.LoadWav(path))
	{
		gEngfuncs.Con_Printf("Failed to load WAV file from %s.\n", path.c_str());
		return;
	}
	else
		gEngfuncs.Con_Printf("Loading WAV from %s.\n", path.c_str());

	if (stricmp(mode, "loop") == 0)
		alSourcei(g_WavPlayer.m_source, AL_LOOPING, AL_TRUE);
	else
		alSourcei(g_WavPlayer.m_source, AL_LOOPING, AL_FALSE);

	g_WavPlayer.SetVolumeFromCvar();
	g_WavPlayer.Play();
}

void CWavPlayer::StopCmd()
{
	g_WavPlayer.Stop();
	gEngfuncs.Con_Printf("stopped\n");
}

void CWavPlayer::Shutdown()
{
	alDeleteSources(1, &m_source);
	alDeleteBuffers(1, &m_buffer);
	alcDestroyContext(m_context);
	alcCloseDevice(m_device);
}

CON_COMMAND(al_play, "None")
{
	g_WavPlayer.PlayCmd();
}

CON_COMMAND(al_stop, "none")
{
	g_WavPlayer.StopCmd();
}