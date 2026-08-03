#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "update_checker.h"
#include "versioninfo.h"
#ifdef _STEAMWORKS
#include <steamworks/steam_api.h>
#endif

#include "audio/music.h"

bool CHudWatermark::IsBetaApp()
{
	return SteamUtils()->GetAppID() == 3903990;
}

bool CHudWatermark::Neptune()
{
	return SteamUtils()->IsSteamRunningOnSteamDeck();
}

#define DRAW_STRING gEngfuncs.pfnDrawString
int CHudWatermark::Init()
{
	m_iFlags = 0;

	gHUD.AddHudElem(this);
	return 0;
}

int CHudWatermark::VidInit()
{
	m_iFlags |= HUD_ACTIVE;
	refresh_draw_until = true;
	update_is_available = update_checker::is_update_available();
	hasMusicPlayed = false;

	return 1;
}

int CHudWatermark::Draw(float time)
{
	if (refresh_draw_until || (draw_until > gHUD.m_flTime + 15.0f)) 
	{
		refresh_draw_until = false;
		draw_until = gHUD.m_flTime + 15.0f;
		m_bDrawInfo = true;
	}

	if (gHUD.m_flTime >= draw_until) 
	{
		if (!hasMusicPlayed)
		{
			g_MusicSystem.Play();
			hasMusicPlayed = true;
		}
#ifdef PHX_FINAL
		m_bDrawInfo = false;
#endif
	}

	int r, g, b;
	UnpackRGB(r, g, b, gHUD.m_iDefaultHUDColor);

	char zamnhlmpVersion[256];
	char displayString[256];

	const char*  gamedir = gEngfuncs.pfnGetGameDirectory();

		char filepath[260];
		snprintf(filepath, sizeof(filepath), "%s/aura/version.txt", gamedir);
		FILE* file = fopen(filepath, "r");

		if (file != NULL)
		{
			if (fgets(zamnhlmpVersion, sizeof(zamnhlmpVersion), file) != NULL)
			{
				size_t len = strlen(zamnhlmpVersion);
				if (len > 0 && zamnhlmpVersion[len - 1] == '\n')
				{
					zamnhlmpVersion[len - 1] = '\0';
				}
			}

			fclose(file);
		}
		else
		{
			gEngfuncs.Con_Printf("version.txt not found");
		}

#ifndef _HALO
	sprintf(displayString, "Half-Life: Cross Product Multiplayer %s", zamnhlmpVersion);
#else
	sprintf(displayString, "Halo: GoldSource %s", zamnhlmpVersion);
#endif

	extern cvar_t* hud_watermark;

#ifdef _STEAMWORKS
	CSteamID steamID = SteamUser()->GetSteamID();
	uint32 accountID = steamID.GetAccountID();
	uint32 authServer = steamID.GetEAccountType() == k_EAccountTypeIndividual ? steamID.GetUnAccountInstance() & 1 : 0;
	uint32 accountNumber = accountID;

	const char* username = SteamFriends()->GetPersonaName();

	static char steamIDString[64];
	snprintf(steamIDString, sizeof(steamIDString), "STEAM_0:%u:%u", authServer, accountNumber / 2);
#endif

	int charHeight = CharHeight;
	int charWidth = 8; // Assuming a fixed character width for simplicity

#ifdef _STEAMWORKS
	int textWidth = strlen(steamIDString) * charWidth;
#endif

#if !defined(CLOSED_BETA)
	if (hud_watermark->value == 1)
	{
#endif
#if defined(_STEAMWORKS) && defined(CLOSED_BETA) || defined(_DEBUG)
		DRAW_STRING(ScreenWidth / 20, CharHeight, "BETA - work in progress", r, g, b);
		DRAW_STRING(ScreenWidth / 20, CharHeight * 2, zamnhlmpVersion, r, g, b); // read from version.txt
		DRAW_STRING(ScreenWidth / 20, CharHeight * 3, "client built " __DATE__, r, g, b);
		DRAW_STRING(ScreenWidth / 20, CharHeight * 4, username, r, g, b);
		DRAW_STRING(ScreenWidth / 20, CharHeight * 5, steamIDString, r, g, b);
#ifndef _DEBUG
		DRAW_STRING(ScreenWidth / 20, CharHeight * 6, "CONFIDENTIAL - INTERNAL USE ONLY", 255, 0, 0);

		if (Neptune())
			DRAW_STRING(ScreenWidth / 20, CharHeight * 7, "STEAM DECK mode", 128, 128, 128);

		DRAW_STRING(ScreenWidth / 20, CharHeight * 10, "Property of The Phoenix Project Software SVG", r, g, b);
#else
		DRAW_STRING(ScreenWidth / 20, CharHeight * 6, "DEBUG BUILD - internal use only.", 255, 0, 0);
#endif
		if (!pszGamemode.empty())
			DRAW_STRING((ScreenWidth - textWidth) / 2, ScreenHeight - CharHeight * 2, pszGamemode.c_str(), r, g, b);
		// DRAW_STRING((ScreenWidth - textWidth) / 2, ScreenHeight - CharHeight * 2, steamIDString, r, g, b);
#else // Release
		if (m_bDrawInfo)
		{
			DRAW_STRING(ScreenWidth / 20, gHUD.m_scrinfo.iCharHeight, "Aura client build " __DATE__, r, g, b);
			DRAW_STRING(ScreenWidth / 20, gHUD.m_scrinfo.iCharHeight * 2, displayString, r, g, b); // read from version.txt
#ifndef _HALO
			gEngfuncs.pfnDrawString(ScreenWidth / 20, gHUD.m_scrinfo.iCharHeight * 3, season, r, g, b);
#endif
			DRAW_STRING(ScreenWidth / 20, gHUD.m_scrinfo.iCharHeight * 4, "Press / to open the COMMAND MENU.", 0, 255, 0);
		}

		if(!pszGamemode.empty())
			DRAW_STRING((ScreenWidth - textWidth) / 2, ScreenHeight - CharHeight * 2, pszGamemode.c_str(), r, g, b);
#endif
#if !defined(CLOSED_BETA)
	}
#endif

	if (!IsBetaApp() && gHUD.m_Settings.IsOldServer())
	{
		DRAW_STRING(ScreenWidth / 20, gHUD.m_scrinfo.iCharHeight * 8, "OUTDATED SERVER!", 255, 0, 0);
		DRAW_STRING(ScreenWidth / 20, gHUD.m_scrinfo.iCharHeight * 9, "Whoever is running this server must update it.", 255, 0, 0);
	}

	return 0;
}
