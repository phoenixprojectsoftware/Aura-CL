/****
*
* Copyright (c) 2021-2026 The Phoenix Project Software. Some Rights Reserved.
*
* AURA
*
* music system
*
*
****/

#pragma once
#include <string>
#include <map>

class CMusicSystem
{
public:
	void Init();
	void Play();
	void Stop();
	void Shutdown();

	bool m_bMapMusic = false;

private:
	void LoadFile(const char* pszPath);

	std::string m_Command;
	bool m_bIsPlaying = false;
};

extern CMusicSystem g_MusicSystem;
