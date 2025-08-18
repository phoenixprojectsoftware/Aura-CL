/***
*
*	Copyright (c) 1999, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
// death notice
//
#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>
#include <sstream>

#include "vgui_TeamFortressViewport.h"

DECLARE_MESSAGE( m_DeathNotice, DeathMsg );

struct DeathNoticeItem {
	char szKiller[MAX_PLAYER_NAME_LENGTH*2];
	char szVictim[MAX_PLAYER_NAME_LENGTH*2];
	char szWeapon[32];
	int iId;	// the index number of the associated sprite
	int iSuicide;
	int iTeamKill;
	int iNonPlayerKill;
	float flDisplayTime;
	float *KillerColor;
	float *VictimColor;
};

#define MAX_DEATHNOTICES	8
static int DEATHNOTICE_DISPLAY_TIME = 6;

#define DEATHNOTICE_TOP		32

DeathNoticeItem rgDeathNoticeList[ MAX_DEATHNOTICES + 1 ];

float g_ColorBlue[3]	= { 0.6, 0.8, 1.0 };
float g_ColorRed[3]		= { 1.0, 0.25, 0.25 };
float g_ColorGreen[3]	= { 0.6, 1.0, 0.6 };
float g_ColorYellow[3]	= { 1.0, 0.7, 0.0 };
float g_ColorGrey[3]	= { 0.8, 0.8, 0.8 };

cvar_t *m_pCvarKillSnd;
cvar_t *m_pCvarKillSndPath;

float *GetClientColor( int clientIndex )
{
	switch ( g_PlayerExtraInfo[clientIndex].teamnumber )
	{
	case 1:	return g_ColorBlue;
	case 2: return g_ColorRed;
	case 3: return g_ColorYellow;
	case 4: return g_ColorGreen;
	case 0: return g_ColorYellow;

		default	: return g_ColorGrey;
	}

	return NULL;
}

int CHudDeathNotice :: Init( void )
{
	gHUD.AddHudElem( this );

	HOOK_MESSAGE( DeathMsg );

	CVAR_CREATE( "hud_deathnotice_time", "6", FCVAR_ARCHIVE );

	m_pCvarKillSnd = CVAR_CREATE( "cl_killsound", "1", FCVAR_ARCHIVE );
	m_pCvarKillSndPath = CVAR_CREATE( "cl_killsound_path", "buttons/bell1.wav", FCVAR_ARCHIVE );

	return 1;
}


void CHudDeathNotice :: InitHUDData( void )
{
	memset( rgDeathNoticeList, 0, sizeof(rgDeathNoticeList) );
}


int CHudDeathNotice :: VidInit( void )
{
	m_HUD_d_skull = gHUD.GetSpriteIndex( "d_skull" );

	return 1;
}

int CHudDeathNotice::Draw(float flTime)
{
	int y, r, g, b;
	int gap = 35;

	for (int i = 0; i < MAX_DEATHNOTICES; i++)
	{
		if (rgDeathNoticeList[i].iId == 0)
			break;

		if (rgDeathNoticeList[i].flDisplayTime < flTime)
		{
			memmove(&rgDeathNoticeList[i], &rgDeathNoticeList[i + 1], sizeof(DeathNoticeItem) * (MAX_DEATHNOTICES - i));
			i--;
			continue;
		}

		rgDeathNoticeList[i].flDisplayTime = min(rgDeathNoticeList[i].flDisplayTime, gHUD.m_flTime + DEATHNOTICE_DISPLAY_TIME);

		if (gViewPort && gViewPort->AllowedToPrintText())
		{
			y = DEATHNOTICE_TOP + 2 + (gap * i);

			const char* killer = rgDeathNoticeList[i].szKiller;
			const char* victim = rgDeathNoticeList[i].szVictim;
			const char* weapon = rgDeathNoticeList[i].szWeapon;

			std::ostringstream msg;

			bool isLocalKiller = false;
			bool isLocalVictim = false;

			for (int j = 1; j < MAX_PLAYERS; j++)
			{
				if (g_PlayerInfoList[j].name)
				{
					if (!strcmp(g_PlayerInfoList[j].name, killer) && g_PlayerInfoList[j].thisplayer)
						isLocalKiller = true;
					if (!strcmp(g_PlayerInfoList[j].name, victim) && g_PlayerInfoList[j].thisplayer)
						isLocalVictim = true;
				}
			}

			if (rgDeathNoticeList[i].iNonPlayerKill)
			{
				msg << killer << " destroyed " << victim;
			}
			else if (rgDeathNoticeList[i].iSuicide)
			{
				if (!strcmp(weapon, "world"))
					msg << victim << " died";
				else
					msg << victim << " killed themselves";
			}
			else if (rgDeathNoticeList[i].iTeamKill)
			{
				msg << killer << " teamkilled " << victim;
			}
			else if (isLocalKiller)
			{
				msg << "You killed " << victim;
				if (*weapon)
					msg << " with " << weapon;
			}
			else if (isLocalVictim)
			{
				msg << killer << " killed you";
				if (*weapon)
					msg << " with " << weapon;
			}
			else
			{
				msg << killer << " killed " << victim;
				if (*weapon)
					msg << " with " << weapon;
			}

			// Color
			r = 255; g = 80; b = 0;
			if (rgDeathNoticeList[i].iTeamKill)
			{
				r = 10; g = 240; b = 10;
			}

			std::string str = msg.str();
			char temp[256];
			strncpy(temp, str.c_str(), sizeof(temp));
			temp[sizeof(temp) - 1] = '\0';

			int textWidth = 0, textHeight = 0;
			gEngfuncs.pfnDrawConsoleStringLen(temp, &textWidth, &textHeight);

			int drawX = ScreenWidth - textWidth - 10; // Right aligned

			gHUD.DrawConsoleStringWithColorTags(drawX, y, temp, true, r / 255.0f, g / 255.0f, b / 255.0f);
		}
	}

	return 1;
}

// This message handler may be better off elsewhere
int CHudDeathNotice::MsgFunc_DeathMsg(const char* pszName, int iSize, void* pbuf)
{
	m_iFlags |= HUD_ACTIVE;

	BEGIN_READ(pbuf, iSize);

	int killer = READ_BYTE();
	int victim = READ_BYTE();

	char killedwith[32];
	strcpy(killedwith, "d_");
	strncat(killedwith, READ_STRING(), ARRAYSIZE(killedwith) - 3);

	if (gViewPort)
		gViewPort->DeathMsg(killer, victim);

	gHUD.m_Spectator.DeathMessage(victim);

	int i;
	for (i = 0; i < MAX_DEATHNOTICES; i++)
	{
		if (rgDeathNoticeList[i].iId == 0)
			break;
	}
	if (i == MAX_DEATHNOTICES)
	{
		memmove(rgDeathNoticeList, rgDeathNoticeList + 1, sizeof(DeathNoticeItem) * MAX_DEATHNOTICES);
		i = MAX_DEATHNOTICES - 1;
	}

	if (gViewPort)
		gViewPort->GetAllPlayersInfo();

	const char* killer_name = g_PlayerInfoList[killer].name;
	if (!killer_name)
	{
		killer_name = "";
		rgDeathNoticeList[i].szKiller[0] = 0;
	}
	else
	{
		rgDeathNoticeList[i].KillerColor = GetClientColor(killer);
		strncpy(rgDeathNoticeList[i].szKiller, killer_name, MAX_PLAYER_NAME_LENGTH);
		rgDeathNoticeList[i].szKiller[MAX_PLAYER_NAME_LENGTH - 1] = 0;
	}

	const char* victim_name = nullptr;
	if ((char)victim != -1)
		victim_name = g_PlayerInfoList[victim].name;
	if (!victim_name)
	{
		victim_name = "";
		rgDeathNoticeList[i].szVictim[0] = 0;
	}
	else
	{
		rgDeathNoticeList[i].VictimColor = GetClientColor(victim);
		strncpy(rgDeathNoticeList[i].szVictim, victim_name, MAX_PLAYER_NAME_LENGTH);
		rgDeathNoticeList[i].szVictim[MAX_PLAYER_NAME_LENGTH - 1] = 0;
	}

	// Store the weapon name (without "d_")
	strncpy(rgDeathNoticeList[i].szWeapon, killedwith + 2, sizeof(rgDeathNoticeList[i].szWeapon));
	rgDeathNoticeList[i].szWeapon[sizeof(rgDeathNoticeList[i].szWeapon) - 1] = 0;

	if ((char)victim == -1)
	{
		rgDeathNoticeList[i].iNonPlayerKill = TRUE;
		strcpy(rgDeathNoticeList[i].szVictim, killedwith + 2);
	}
	else
	{
		if (killer == victim || killer == 0)
			rgDeathNoticeList[i].iSuicide = TRUE;

		if (!strcmp(killedwith, "d_teammate"))
			rgDeathNoticeList[i].iTeamKill = TRUE;
	}

	int spr = gHUD.GetSpriteIndex(killedwith);
	rgDeathNoticeList[i].iId = spr;

	DEATHNOTICE_DISPLAY_TIME = CVAR_GET_FLOAT("hud_deathnotice_time");
	rgDeathNoticeList[i].flDisplayTime = gHUD.m_flTime + DEATHNOTICE_DISPLAY_TIME;

	// Play local kill sound
	if ((g_PlayerInfoList[killer].thisplayer || g_iUser2 == killer) &&
		!rgDeathNoticeList[i].iNonPlayerKill &&
		!rgDeathNoticeList[i].iSuicide &&
		m_pCvarKillSnd->value > 0.0f)
	{
		PlaySound(m_pCvarKillSndPath->string, m_pCvarKillSnd->value);
	}

	// Console output
	if (rgDeathNoticeList[i].iNonPlayerKill)
	{
		ConsolePrint(rgDeathNoticeList[i].szKiller);
		ConsolePrint(" destroyed ");
		ConsolePrint(rgDeathNoticeList[i].szVictim);
	}
	else if (rgDeathNoticeList[i].iSuicide)
	{
		ConsolePrint(rgDeathNoticeList[i].szVictim);
		if (!strcmp(killedwith, "d_world"))
			ConsolePrint(" died");
		else
			ConsolePrint(" killed self");
	}
	else if (rgDeathNoticeList[i].iTeamKill)
	{
		ConsolePrint(rgDeathNoticeList[i].szKiller);
		ConsolePrint(" killed teammate ");
		ConsolePrint(rgDeathNoticeList[i].szVictim);
	}
	else
	{
		ConsolePrint(rgDeathNoticeList[i].szKiller);
		ConsolePrint(" killed ");
		ConsolePrint(rgDeathNoticeList[i].szVictim);
		if (rgDeathNoticeList[i].szWeapon[0])
		{
			ConsolePrint(" with ");
			// Pretty weapon names
			if (!strcmp(rgDeathNoticeList[i].szWeapon, "egon"))
				ConsolePrint("gluon gun");
			else if (!strcmp(rgDeathNoticeList[i].szWeapon, "gauss"))
				ConsolePrint("tau cannon");
			else
				ConsolePrint(rgDeathNoticeList[i].szWeapon);
		}
	}
	ConsolePrint("\n");

	return 1;
}




