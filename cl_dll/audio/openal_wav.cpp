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

#define DR_MP3_IMPLEMENTATION
#include <dr_mp3.h>

#include <convar.h>

CSoundtrackSystem g_SoundtrackSystem;

CSoundtrackSystem::CSoundtrackSystem() : m_device(nullptr), m_context(nullptr), m_source(0), m_buffer(0) {}

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

    if (lower.find(".ogg") != std::string::npos || lower.find(".rts") != std::string::npos) // .rts should also be ogg.
        return LoadOgg(filename);
    else if (lower.find(".mp3") != std::string::npos)
        return LoadMp3(filename);
    else
        return LoadWav(filename);

    m_currentPath = filename;
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
    // Clear previous AL error
    while (alGetError() != AL_NO_ERROR) {}

    OggVorbis_File vf{};
    if (ov_fopen(filename.c_str(), &vf) < 0)
    {
        gEngfuncs.Con_Printf("CSoundtrackSystem: Failed to open OGG: %s\n", filename.c_str());
        return false;
    }

    int current_bs = -1;
    vorbis_info* vi = ov_info(&vf, -1);
    if (!vi)
    {
        gEngfuncs.Con_Printf("CSoundtrackSystem: ov_info failed for %s\n", filename.c_str());
        ov_clear(&vf);
        return false;
    }

    if (vi->channels < 1 || vi->channels > 2)
    {
        gEngfuncs.Con_Printf("CSoundtrackSystem: Unsupported OGG channel count: %d (only 1/2)\n", vi->channels);
        ov_clear(&vf);
        return false;
    }

    const ALenum format = (vi->channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
    const ALsizei rate = static_cast<ALsizei>(vi->rate);

    std::vector<char> pcm;
    pcm.reserve(256 * 1024);

    // Decode loop: little-endian (0), 16-bit (word=2), signed (1)
    constexpr int bigendian = 0;
    constexpr int word = 2;
    constexpr int sgned = 1;

    char buffer[4096];
    for (;;)
    {
        int bs = 0;
        long bytes = ov_read(&vf, buffer, static_cast<int>(sizeof(buffer)),
            bigendian, word, sgned, &bs);
        if (bytes == 0)
            break;                  // EOF
        if (bytes < 0)
        {
            // Error or hole in data; log and continue or abort for severe errors
            gEngfuncs.Con_Printf("CSoundtrackSystem: ov_read error %ld on %s\n", bytes, filename.c_str());
            // You can 'continue' for recoverable errors like OV_HOLE; here we fail hard:
            ov_clear(&vf);
            return false;
        }

        // Bitstream change? Sanity check format doesn’t change mid-file.
        if (bs != current_bs)
        {
            current_bs = bs;
            vorbis_info* vi2 = ov_info(&vf, -1);
            if (!vi2 || vi2->channels != vi->channels || vi2->rate != vi->rate)
            {
                gEngfuncs.Con_Printf("CSoundtrackSystem: Chained OGG with differing format not supported\n");
                ov_clear(&vf);
                return false;
            }
        }

        pcm.insert(pcm.end(), buffer, buffer + bytes);
    }

    ov_clear(&vf);

    if (pcm.empty())
    {
        gEngfuncs.Con_Printf("CSoundtrackSystem: No PCM decoded for %s\n", filename.c_str());
        return false;
    }

    if (m_buffer)
        alDeleteBuffers(1, &m_buffer);

    alGenBuffers(1, &m_buffer);
    if (ALenum e = alGetError(); e != AL_NO_ERROR)
    {
        gEngfuncs.Con_Printf("CSoundtrackSystem: alGenBuffers error %d\n", e);
        return false;
    }

    alBufferData(m_buffer, format, pcm.data(), static_cast<ALsizei>(pcm.size()), rate);
    if (ALenum e = alGetError(); e != AL_NO_ERROR)
    {
        gEngfuncs.Con_Printf("CSoundtrackSystem: alBufferData error %d\n", e);
        return false;
    }

    alSourcei(m_source, AL_BUFFER, m_buffer);
    if (ALenum e = alGetError(); e != AL_NO_ERROR)
    {
        gEngfuncs.Con_Printf("CSoundtrackSystem: alSourcei(AL_BUFFER) error %d\n", e);
        return false;
    }

    gEngfuncs.Con_Printf("CSoundtrackSystem: Loaded OGG: %s (%d ch, %d Hz, %zu bytes)\n",
        filename.c_str(), vi->channels, vi->rate, pcm.size());
    return true;
}

bool CSoundtrackSystem::LoadMp3(const std::string& filename)
{
    drmp3 mp3{};
    if (!drmp3_init_file(&mp3, filename.c_str(), nullptr)) {
        gEngfuncs.Con_Printf("MP3: failed to open %s\n", filename.c_str());
        return false;
    }

    drmp3_uint64 frameCount = drmp3_get_pcm_frame_count(&mp3);
    if (frameCount == 0) { drmp3_uninit(&mp3); return false; }

    std::vector<int16_t> pcm(frameCount * mp3.channels);
    drmp3_uint64 got = drmp3_read_pcm_frames_s16(&mp3, frameCount, pcm.data());
    drmp3_uninit(&mp3);

    if (got == 0) { gEngfuncs.Con_Printf("MP3: decode produced 0 frames\n"); return false; }

    const ALenum format =
        (mp3.channels == 1) ? AL_FORMAT_MONO16 :
        (mp3.channels == 2) ? AL_FORMAT_STEREO16 : 0;

    if (format == 0) {
        gEngfuncs.Con_Printf("MP3: unsupported channel count %d\n", mp3.channels);
        return false;
    }

    if (m_buffer && alIsBuffer(m_buffer)) { alDeleteBuffers(1, &m_buffer); m_buffer = 0; }

    alGenBuffers(1, &m_buffer);
    alBufferData(m_buffer, format,
        pcm.data(),
        static_cast<ALsizei>(got * mp3.channels * sizeof(int16_t)),
        mp3.sampleRate);

    ALenum err = alGetError();
    if (err != AL_NO_ERROR) {
        gEngfuncs.Con_Printf("OpenAL: alBufferData error %d\n", err);
        return false;
    }

    alSourcei(m_source, AL_BUFFER, m_buffer);
    gEngfuncs.Con_Printf("Loaded MP3: %s (%d ch, %d Hz)\n",
        filename.c_str(), mp3.channels, mp3.sampleRate);
    return true;
}

void CSoundtrackSystem::Play(bool loop)
{
    m_loop = loop;

    alSourcei(m_source, AL_LOOPING, AL_FALSE);
    SetVolumeFromCvar();
    alSourcePlay(m_source);

    if (loop && m_queue.empty())
        gEngfuncs.Con_Printf("CSoundtrackSystem: LOOP enabled\n");
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

void CSoundtrackSystem::Queue(const std::string& filename, bool loop)
{
    m_queue.push({ filename, loop });
    gEngfuncs.Con_Printf("Queued: %s (loop=%s)\n", filename.c_str(), loop ? "true" : "false");

    // if nothing is playing, let's get started right away
    ALint state = 0;
    alGetSourcei(m_source, AL_SOURCE_STATE, &state);
    if (state != AL_PLAYING && m_queue.size() == 1) // just added 1ST item
    {
        QueuedTrack next = m_queue.front();
        m_queue.pop();
        if (LoadSoundtrack(next.path))
            Play(next.loop);
    }
}

void CSoundtrackSystem::Update()
{
    ALint state = 0;
    alGetSourcei(m_source, AL_SOURCE_STATE, &state);

    // still playing
    if (state == AL_PLAYING)
        return;

    // track finished and should loop, but only if queue is empty
    if (state == AL_STOPPED && m_loop && m_queue.empty())
    {
        alSourcePlay(m_source);
        return;
    }

    // if queue has items, play next
    if (!m_queue.empty())
    {
        QueuedTrack next = m_queue.front();
        m_queue.pop();

        // load and play the next item. play will decide whether to actually set openal looping based on whether the queue is empty after this pop.
        if (LoadSoundtrack(next.path))
            Play(next.loop);
        return;
    }

    // otherwise: stop
}

void CSoundtrackSystem::PauseMusic()
{
    if (m_paused)
        return;

    ALint state;
    alGetSourcei(m_source, AL_SOURCE_STATE, &state);

    if (state == AL_PLAYING)
    {
        m_paused = true;
        alSourcePause(m_source);
        gEngfuncs.Con_Printf("MUSIC PAUSED\n");
    }
}

void CSoundtrackSystem::ResumeMusic()
{
    if (!m_paused)
        return;

    m_paused = false;
    alSourcePlay(m_source);
    gEngfuncs.Con_Printf("MUSIC RESUMED\n");
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
    // construct the full path from hl.exe
	std::string path = std::string(gEngfuncs.pfnGetGameDirectory()) + "/sound/music/" + file;

    if (!g_SoundtrackSystem.LoadSoundtrack(path))
        return;

    bool loop = (gEngfuncs.Cmd_Argc() >= 3 && !stricmp(gEngfuncs.Cmd_Argv(2), "loop"));
	g_SoundtrackSystem.Play(loop);
}

CON_COMMAND(soundtrack_queue, "Queue soundtrack")
{
    if (gEngfuncs.Cmd_Argc() < 2)
    {
        gEngfuncs.Con_Printf("Usage: soundtrack_queue <file.ext> [loop]\n");
        return;
    }

    std::string file = gEngfuncs.Cmd_Argv(1);
    std::string path = std::string(gEngfuncs.pfnGetGameDirectory()) + "/sound/music/" + file;

    bool loop = false;
    if (gEngfuncs.Cmd_Argc() >= 3 && !stricmp(gEngfuncs.Cmd_Argv(2), "loop"))
        loop = true;

    g_SoundtrackSystem.Queue(path, loop);
}

CON_COMMAND(soundtrack_skip, "Skips current soundtrack")
{
    alSourceStop(g_SoundtrackSystem.m_source);
}

CON_COMMAND(soundtrack_clear, "Clears the score queue")
{
    while (!g_SoundtrackSystem.m_queue.empty())
        g_SoundtrackSystem.m_queue.pop();
    gEngfuncs.Con_Printf("Soundtrack queue cleared.\n");
}

CON_COMMAND(soundtrack_current, "Show currently playing soundtrack")
{
    ALint state = 0;
    alGetSourcei(g_SoundtrackSystem.m_source, AL_SOURCE_STATE, &state);

    if (g_SoundtrackSystem.m_currentPath.empty())
    {
        gEngfuncs.Con_Printf("No track has been played yet.\n");
        return;
    }

    const char* stateStr = (state == AL_PLAYING) ? "PLAYING" :
        (state == AL_PAUSED) ? "PAUSED" :
        "STOPPED";

    gEngfuncs.Con_Printf("Current soundtrack:\n");
    gEngfuncs.Con_Printf("  File: %s\n", g_SoundtrackSystem.m_currentPath.c_str());
    gEngfuncs.Con_Printf("  State: %s\n", stateStr);
    gEngfuncs.Con_Printf("  Looping: %s\n", g_SoundtrackSystem.m_loop ? "true" : "false");
}

CON_COMMAND(soundtrack_queue_print, "List queued soundtrack items")
{
    if (g_SoundtrackSystem.m_queue.empty())
    {
        gEngfuncs.Con_Printf("Queue is empty.\n");
        return;
    }

    gEngfuncs.Con_Printf("Queued tracks:\n");

    // Make a temporary copy so we can iterate safely
    auto q = g_SoundtrackSystem.m_queue;
    int index = 1;
    while (!q.empty())
    {
        const auto& item = q.front();
        gEngfuncs.Con_Printf("  %d) %s  (loop=%s)\n",
            index++,
            item.path.c_str(),
            item.loop ? "true" : "false");
        q.pop();
    }
}


CON_COMMAND(soundtrack_stop, "Stop soundtrack")
{
    g_SoundtrackSystem.Stop();
}

#if defined(_DEBUG) || defined(CLOSED_BETA)
CON_COMMAND(soundtrack_debug, "Show soundtrack debug info")
{
    gEngfuncs.Con_Printf("\n[Soundtrack Debug]\n");
    gEngfuncs.Con_Printf("Current: %s\n", g_SoundtrackSystem.m_currentPath.c_str());
    gEngfuncs.Con_Printf("Loop: %s\n", g_SoundtrackSystem.m_loop ? "true" : "false");
    gEngfuncs.Con_Printf("Queue length: %zu\n", g_SoundtrackSystem.m_queue.size());
    gEngfuncs.Con_Printf("\n");
}
#endif