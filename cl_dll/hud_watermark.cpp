#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "update_checker.h"
//#include <filesystem>
#include "versioninfo.h"

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

	if (gHUD.m_flTime >= draw_until) 
	{
		m_iFlags &= ~HUD_ACTIVE;
		return 0;
	}

	int r, g, b;
	UnpackRGB(r, g, b, gHUD.m_iDefaultHUDColor);

	char zamnhlmpVersion[256];
	char displayString[256];


		char filepath[260];
		snprintf(filepath, sizeof(filepath), "zamnhlmp/aura/version.txt");
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
	const char* sv_aura_version;
	sv_aura_version = CVAR_GET_STRING("sv_aura_version");

	if (hud_watermark->value == 1)
	{

		gEngfuncs.pfnDrawString(ScreenWidth / 20, gHUD.m_scrinfo.iCharHeight, "Aura client build " __DATE__, r, g, b);
		gEngfuncs.pfnDrawString(ScreenWidth / 20, gHUD.m_scrinfo.iCharHeight * 2, displayString, r, g, b);
		gEngfuncs.pfnDrawString(ScreenWidth / 20, gHUD.m_scrinfo.iCharHeight * 3, season, r, g, b);

		if (update_is_available)
			gEngfuncs.pfnDrawString(ScreenWidth / 20, gHUD.m_scrinfo.iCharHeight / 2 * 7, " ", r, g, b);
	}

	return 0;
}
