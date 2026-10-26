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

#include "achievement_manager.h"
#include "cl_gametype.h"

DECLARE_MESSAGE( m_DeathNotice, DeathMsg );
DECLARE_MESSAGE(m_DeathNotice, FFKill);

struct DeathNoticeItem {
	char szKiller[MAX_PLAYER_NAME_LENGTH*2];
	char szVictim[MAX_PLAYER_NAME_LENGTH*2];
	char szWeapon[32];
	int iId;	// the index number of the associated sprite
	int iSuicide;
	int iTeamKill;
	int iNonPlayerKill;
	int iFirefightKill;
	float flDisplayTime;
	float *KillerColor;
	float *VictimColor;
};

#define MAX_DEATHNOTICES	8
static int DEATHNOTICE_DISPLAY_TIME = 6;

#define DEATHNOTICE_TOP		32

DeathNoticeItem rgDeathNoticeList[ MAX_DEATHNOTICES + 1 ];
CAchievementMgr g_KillStats;

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
	HOOK_MESSAGE(FFKill);

	CVAR_CREATE( "hud_deathnotice_time", "6", FCVAR_ARCHIVE );

	m_pCvarKillSnd = CVAR_CREATE( "cl_killsound", "1", FCVAR_ARCHIVE );
	m_pCvarKillSndPath = CVAR_CREATE( "cl_killsound_path", "player/killsound.wav", FCVAR_ARCHIVE );

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

			if (rgDeathNoticeList[i].iFirefightKill)
			{
				if (isLocalKiller)
				{
					msg << "You killed " << victim;
				}
				else
				{
					msg << killer << " killed " << victim;
				}

				if (*weapon)
				{
					msg << " with " << weapon;
				}
			}
			else if (rgDeathNoticeList[i].iNonPlayerKill)
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
				if (*weapon && INSTAGIB != gHUD.GetGameType())
					msg << " with " << weapon;
			}
			else if (isLocalVictim)
			{
				msg << killer << " killed you";
				if (*weapon && INSTAGIB != gHUD.GetGameType())
					msg << " with " << weapon;
			}
			else
			{
				msg << killer << " killed " << victim;
				if (*weapon && INSTAGIB != gHUD.GetGameType())
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

int32 plrKillStatValue = 0;
int32 plrMeleeKillStat = 0;
int32 tripmineKillStat = 0;
int32 sniperKillStat = 0;
int32 snarkKillStat = 0;

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

		// firefight kill etc. - we prevent corrupted data here
		strncpy(rgDeathNoticeList[i].szVictim, "an enemy", sizeof(rgDeathNoticeList[i].szVictim));
#ifdef _DEBUG
		ConsolePrint("FIREFIGHT KILL DETECTED\n");
#endif
		rgDeathNoticeList[i].szVictim[sizeof(rgDeathNoticeList[i].szVictim) - 1] = 0;
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

	// Kill increment + sound logic.
	if ((g_PlayerInfoList[killer].thisplayer || g_iUser2 == killer) &&
		!rgDeathNoticeList[i].iNonPlayerKill &&
		!rgDeathNoticeList[i].iSuicide)
	{
#if defined(_STEAMWORKS) && !defined(_HALO)
		if (INSTAGIB != g_iGameType && OITC != g_iGameType)
		{
			g_AchievementMgr.StatIncrement(PLR_KILL_STATS);
		}

		if (!g_AchievementMgr.isAchievementUnlocked(1))
		{
			g_AchievementMgr.UnlockAchievement(1);
			gEngfuncs.Con_Printf("You just earned GAMERSCORE, baby. Why? ACH_FIRST_BLOOD\n");
		}

		// KILL TRACKING - WEAPONS
		// DISPLACER ACHIEVEMENT
		if (!stricmp(rgDeathNoticeList[i].szWeapon, "displacer_ball"))
		{
			if (!g_AchievementMgr.isAchievementUnlocked(19))
				g_AchievementMgr.UnlockAchievement(19);

			g_KillStats.StatIncrement(DISPLACER_KILLS);
		}

		// PENGUIN ACHIEVEMENT
		if (!stricmp(rgDeathNoticeList[i].szWeapon, "penguin"))
		{
			if (!g_AchievementMgr.isAchievementUnlocked(20))
				g_AchievementMgr.UnlockAchievement(20);
			else
				gEngfuncs.Con_Printf("It looks like you've already done this. Very clever....\n");

			g_KillStats.StatIncrement(PENGUIN_KILLS);
		}

		// TRIPMINE KILL STAT
		if (!stricmp(rgDeathNoticeList[i].szWeapon, "tripmine"))
		{
			g_KillStats.StatIncrement(TRIPMINE_KILLS);
		}

		// SNIPER RIFLE KILL STAT
		if (!stricmp(rgDeathNoticeList[i].szWeapon, "sniperrifle"))
		{
			g_KillStats.StatIncrement(SNIPER_KILLS);
		}

		// SNARK KILL STAT
		if (!stricmp(rgDeathNoticeList[i].szWeapon, "snark"))
		{
			g_KillStats.StatIncrement(SNARK_KILLS);
		}

		if (!stricmp(rgDeathNoticeList[i].szWeapon, "gauss"))
		{
			g_KillStats.StatIncrement(TAU_KILLS);
		}

		if (!stricmp(rgDeathNoticeList[i].szWeapon, "egon"))
		{
			g_KillStats.StatIncrement(GLUON_KILLS);
		}

		if (!stricmp(rgDeathNoticeList[i].szWeapon, "357"))
		{
			g_KillStats.StatIncrement(MAGNUM_KILLS);
		}

		if (!stricmp(rgDeathNoticeList[i].szWeapon, "eagle"))
		{
			g_KillStats.StatIncrement(DEAGLE_KILLS);
		}

		if (!stricmp(rgDeathNoticeList[i].szWeapon, "9mmAR"))
		{
			g_KillStats.StatIncrement(MP5_KILLS);
		}

		if (!stricmp(rgDeathNoticeList[i].szWeapon, "grenade"))
		{
			g_KillStats.StatIncrement(GRENADE_KILLS);
		}

		if (!stricmp(rgDeathNoticeList[i].szWeapon, "br"))
		{
			g_KillStats.StatIncrement(OLR_KILLS);
		}

		if (!stricmp(rgDeathNoticeList[i].szWeapon, "shotgun"))
		{
			g_KillStats.StatIncrement(SHOTGUN_KILLS);
		}

		if (!stricmp(rgDeathNoticeList[i].szWeapon, "crossbow"))
		{
			g_KillStats.StatIncrement(XBOW_KILLS);
		}

		if (!stricmp(rgDeathNoticeList[i].szWeapon, "rpg"))
		{
			g_KillStats.StatIncrement(RPG_KILLS);
		}

		if (!stricmp(rgDeathNoticeList[i].szWeapon, "hornet"))
		{
			g_KillStats.StatIncrement(HIVEHAND_KILLS);
		}

		if (!stricmp(rgDeathNoticeList[i].szWeapon, "tripmine"))
		{
			g_KillStats.StatIncrement(TRIPMINE_KILLS);
		}

		if (!stricmp(rgDeathNoticeList[i].szWeapon, "satchel"))
		{
			g_KillStats.StatIncrement(SATCHEL_KILLS);
		}

		if (!stricmp(rgDeathNoticeList[i].szWeapon, "m249"))
		{
			g_KillStats.StatIncrement(SAW_KILLS);
		}

		if (!stricmp(rgDeathNoticeList[i].szWeapon, "spore"))
		{
			g_KillStats.StatIncrement(SPORE_KILLS);
		}

		if (!stricmp(rgDeathNoticeList[i].szWeapon, "shock_rifle"))
		{
			g_KillStats.StatIncrement(SHOCK_KILLS);
		}

		if (!stricmp(rgDeathNoticeList[i].szWeapon, "9mmhandgun"))
		{
			g_KillStats.StatIncrement(GLOCK_KILLS);
		}

		// CLOSE CALL ACHIEVEMENT
		if (gHUD.m_Health.m_iHealth <= 10)
			if (!g_AchievementMgr.isAchievementUnlocked(6))
				g_AchievementMgr.UnlockAchievement(6);

		// POINTY END ACHIEVEMENT
		if (!stricmp(rgDeathNoticeList[i].szWeapon, "crowbar") || !stricmp(rgDeathNoticeList[i].szWeapon, "knife") || (!stricmp(rgDeathNoticeList[i].szWeapon, "pipewrench")))
		{
			if (!g_AchievementMgr.isAchievementUnlocked(21))
				g_AchievementMgr.UnlockAchievement(21);

			g_KillStats.StatIncrement(PLR_MELEE_KILLS_STATS);
		}

		// UNDERWATER KILL STAT
		int uwKills = 0;
		extern int g_iClientWaterLevel;
		if (g_iClientWaterLevel >= 2)
		{
			g_AchievementMgr.StatIncrement(PLR_UW_KILLS_STATS);
		}
#endif

		if (m_pCvarKillSnd->value > 0.0f)
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
			// Shut up. Get out.
		}
	}
	ConsolePrint("\n");

	return 1;
}

int CHudDeathNotice::MsgFunc_FFKill(
	const char* pszName,
	int iSize,
	void* pbuf)
{
	m_iFlags |= HUD_ACTIVE;

	BEGIN_READ(pbuf, iSize);

	const int iKiller = READ_BYTE();

	// READ_STRING() may reuse the same internal buffer, so copy each
	// string before reading the next one.
	char szMonsterName[
		MAX_PLAYER_NAME_LENGTH * 2];

	const char* pszReadMonsterName =
		READ_STRING();

	strncpy(
		szMonsterName,
		pszReadMonsterName ?
		pszReadMonsterName :
		"Enemy",
		sizeof(szMonsterName) - 1);

	szMonsterName[
		sizeof(szMonsterName) - 1] = '\0';

	char szWeaponName[32];

	const char* pszReadWeaponName =
		READ_STRING();

	strncpy(
		szWeaponName,
		pszReadWeaponName ?
		pszReadWeaponName :
		"world",
		sizeof(szWeaponName) - 1);

	szWeaponName[
		sizeof(szWeaponName) - 1] = '\0';

	// Validate before accessing g_PlayerInfoList.
	if (iKiller <= 0 ||
		iKiller >= MAX_PLAYERS)
	{
		return 1;
	}

	if (gViewPort)
	{
		gViewPort->GetAllPlayersInfo();
	}

	const bool bLocalKiller =
		g_PlayerInfoList[iKiller].thisplayer ||
		g_iUser2 == iKiller;

	int iNotice = 0;

	for (iNotice = 0;
		iNotice < MAX_DEATHNOTICES;
		++iNotice)
	{
		if (rgDeathNoticeList[iNotice].iId == 0)
		{
			break;
		}
	}

	if (iNotice == MAX_DEATHNOTICES)
	{
		memmove(
			rgDeathNoticeList,
			rgDeathNoticeList + 1,
			sizeof(DeathNoticeItem) *
			(MAX_DEATHNOTICES - 1));

		iNotice =
			MAX_DEATHNOTICES - 1;
	}

	DeathNoticeItem& notice =
		rgDeathNoticeList[iNotice];

	memset(
		&notice,
		0,
		sizeof(notice));

	const char* pszKillerName =
		g_PlayerInfoList[iKiller].name;

	if (!pszKillerName)
	{
		pszKillerName = "";
	}

	strncpy(
		notice.szKiller,
		pszKillerName,
		sizeof(notice.szKiller) - 1);

	notice.szKiller[
		sizeof(notice.szKiller) - 1] = '\0';

	strncpy(
		notice.szVictim,
		szMonsterName,
		sizeof(notice.szVictim) - 1);

	notice.szVictim[
		sizeof(notice.szVictim) - 1] = '\0';

	strncpy(
		notice.szWeapon,
		szWeaponName,
		sizeof(notice.szWeapon) - 1);

	notice.szWeapon[
		sizeof(notice.szWeapon) - 1] = '\0';

	notice.KillerColor =
		GetClientColor(iKiller);

	notice.VictimColor =
		g_ColorGrey;

	notice.iFirefightKill = TRUE;

	char szSpriteName[64];

	snprintf(
		szSpriteName,
		sizeof(szSpriteName),
		"d_%s",
		notice.szWeapon);

	szSpriteName[
		sizeof(szSpriteName) - 1] = '\0';

	notice.iId =
		gHUD.GetSpriteIndex(
			szSpriteName);

	// iId == 0 is also treated as an empty notice slot.
	if (notice.iId <= 0)
	{
		notice.iId =
			m_HUD_d_skull;
	}

	DEATHNOTICE_DISPLAY_TIME =
		CVAR_GET_FLOAT(
			"hud_deathnotice_time");

	notice.flDisplayTime =
		gHUD.m_flTime +
		DEATHNOTICE_DISPLAY_TIME;

#if defined(_STEAMWORKS) && !defined(_HALO)
	if (bLocalKiller)
	{
		g_AchievementMgr.StatIncrement(
			PLR_KILL_STATS);
	}
#endif

	if (bLocalKiller &&
		m_pCvarKillSnd &&
		m_pCvarKillSndPath &&
		m_pCvarKillSnd->value > 0.0f)
	{
		PlaySound(
			m_pCvarKillSndPath->string,
			m_pCvarKillSnd->value);
	}

	return 1;
}
