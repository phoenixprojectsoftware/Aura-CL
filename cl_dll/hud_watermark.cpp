#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "update_checker.h"
#include "versioninfo.h"
#ifdef _STEAMWORKS
#include <steamworks/steam_api.h>
#endif

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

	return 1;
}

int CHudWatermark::Draw(float time)
{
	if (refresh_draw_until || (draw_until > gHUD.m_flTime + 15.0f)) 
	{
		refresh_draw_until = false;
		draw_until = gHUD.m_flTime + 15.0f;
	}

#ifndef CLOSED_BETA
	if (gHUD.m_flTime >= draw_until) 
	{
		m_iFlags &= ~HUD_ACTIVE;
		return 0;
	}
#endif

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

	sprintf(displayString, "Half-Life: Cross Product Multiplayer %s", zamnhlmpVersion);

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


	if (hud_watermark->value == 1)
	{
#if defined(_STEAMWORKS) && (CLOSED_BETA)
		gEngfuncs.pfnDrawString(ScreenWidth / 20, CharHeight, "STEAM CLOSED BETA", r, g, b);
		gEngfuncs.pfnDrawString(ScreenWidth / 20, CharHeight * 2, displayString, r, g, b); // read from version.txt
		gEngfuncs.pfnDrawString(ScreenWidth / 20, CharHeight * 3, "cl build " __DATE__, r, g, b);
		gEngfuncs.pfnDrawString(ScreenWidth / 20, CharHeight * 4, username, r, g, b);
		gEngfuncs.pfnDrawString(ScreenWidth / 20, CharHeight * 5, steamIDString, r, g, b);

		gEngfuncs.pfnDrawString((ScreenWidth - textWidth) / 2, ScreenHeight - CharHeight * 2, steamIDString, r, g, b);
#else
		gEngfuncs.pfnDrawString(ScreenWidth / 20, gHUD.m_scrinfo.iCharHeight, "Aura client build " __DATE__, r, g, b);
		gEngfuncs.pfnDrawString(ScreenWidth / 20, gHUD.m_scrinfo.iCharHeight * 2, displayString, r, g, b); // read from version.txt
		gEngfuncs.pfnDrawString(ScreenWidth / 20, gHUD.m_scrinfo.iCharHeight * 3, season, r, g, b);
#endif

		if (update_is_available)
			gEngfuncs.pfnDrawString(ScreenWidth / 20, gHUD.m_scrinfo.iCharHeight / 2 * 7, " ", r, g, b);
	}

	return 0;
}
