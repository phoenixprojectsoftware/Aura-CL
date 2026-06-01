/****
*
* Copyright (c) 2021-2026 The Phoenix Project Software. Some Rights Reserved.
*
* AURA
*
* music system
*
* This used to be serverside but is now clientside as we can now track the gametype
* from the client. Since the clientside transfer, map themes override gametypes.
****/

#include "../hud.h"
#include "../cl_gametype.h"
#include "music.h"

#include <stdio.h>
#include <string.h>

CMusicSystem g_MusicSystem;

const char* MusicLevelName()
{
	static char map[64];

	const char* full = gEngfuncs.pfnGetLevelName();

	const char* slash = strrchr(full, '/');
	if (!slash)
		slash = full;
	else
		slash++;

	strncpy(map, slash, sizeof(map));
	map[sizeof(map) - 1] = '\0';

	char* dot = strrchr(map, '.');
	if (dot)
		*dot = '\0';

	return map;
}

char* AgGametypeName()
{
	switch (gHUD.GetGameType())
	{
	case ARENA: return "arena";
	case CTF: return "ctf";
	case LMS: return "lms";
	case ARCADE: return "arcade";
	case SGBOW: return "sgbow";
	case INSTAGIB: return "instagib";
	case DOM: return "dom";
	case HIDEANDSEEK: return "hideandseek";
	case FIREFIGHT: return "firefight";
	case FIESTAFIGHT: return "fiestafight";
	case FIESTA: return "fiesta";
	case SWAT: return "swat";
	case HLDM: return "hldm";
	case BUSTERS: return "busters";
	case CHILL: return "chill";
	case STANDARD:
	default:
		return "none";
	}
}

bool MusicFileExists(const char* path)
{
	int fileSize = 0;
	byte* pFile = (byte*)gEngfuncs.COM_LoadFile(path, 5, &fileSize);

	if (pFile)
	{
		gEngfuncs.COM_FreeFile(pFile);
		return true;
	}

	return false;
}

void CMusicSystem::Init()
{
	char szPath[256];

	Stop();

	if (FIREFIGHT == gHUD.GetGameType() || FIESTAFIGHT == gHUD.GetGameType())
		return;

	m_Command.clear();
	m_bMapMusic = false;

	// 1. map first
	snprintf(szPath, sizeof(szPath), "mus/LEVEL/%s.mus", MusicLevelName());

	if (MusicFileExists(szPath))
	{
		m_bMapMusic = true;

		gEngfuncs.Con_Printf(
			"CMusicSystem: found map music for %s via %s\n",
			MusicLevelName(),
			szPath
		);
	}
	else
	{
		const char* pszGametype = AgGametypeName();

		snprintf(szPath, sizeof(szPath), "mus/GAMETYPE/%s.mus", pszGametype);

		if (MusicFileExists(szPath))
		{
			gEngfuncs.Con_Printf(
				"CMusicSystem: using gametype music for %s\n",
				pszGametype
			);
		}
		else
		{
			gEngfuncs.Con_Printf(
				"CMusicSystem: no map/gametype music found, using default\n"
			);

			snprintf(szPath, sizeof(szPath), "mus/default.mus");
		}
	}

	LoadFile(szPath);
}

void CMusicSystem::LoadFile(const char* pszPath)
{
	int fileSize = 0;
	char* pFile = (char*)gEngfuncs.COM_LoadFile(pszPath, 5, &fileSize);

	if (!pFile)
	{
		gEngfuncs.Con_Printf("CMusicSystem: failed to load %s\n", pszPath);
		return;
	}

	std::string buffer(pFile, fileSize);
	buffer.push_back('\0');

	gEngfuncs.COM_FreeFile(pFile);

	char* ctx = nullptr;
	char* line = strtok_s(&buffer[0], "\r\n", &ctx);

	while (line)
	{
		while (*line == ' ' || *line == '\t')
			++line;

		if (*line == '#' || *line == '\0')
		{
			line = strtok_s(nullptr, "\r\n", &ctx);
			continue;
		}

		char cmd[512];
		snprintf(cmd, sizeof(cmd), "mp3 play sound/music/%s\n", line);

		m_Command = cmd;
		break;
	}
}

void CMusicSystem::Play()
{
	if (m_Command.empty())
		return;

	if (gHUD.GetGameType() == FIREFIGHT || gHUD.GetGameType() == FIESTAFIGHT)
		return;

	gEngfuncs.pfnClientCmd(m_Command.c_str());

	gEngfuncs.Con_Printf("CMusicSystem: playing%s\n", m_Command.c_str());

	m_bIsPlaying = true;
}

void CMusicSystem::Stop()
{
	gEngfuncs.pfnClientCmd("mp3 stop\n");
	m_bIsPlaying = false;
}

void CMusicSystem::Shutdown()
{
	Stop();
	m_Command.clear();
}
