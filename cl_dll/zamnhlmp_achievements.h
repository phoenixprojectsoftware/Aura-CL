/****
 *
 * Copyright (c) 2025 The Phoenix Project Software SVG. All Rights Reserved.
 *
 * AURA-CL ACHIEVEMENT MANAGER
 *
 * This code contains segments licensed to The Phoenix Project Software SVG from Valve. Copyright (c) 2022 Valve.
 *
 *
 ****/

#include <steamworks/steam_api.h>
#include <string>

static const char* g_PhoenixSteamIDs[] = {
	"0:1:420777771", // sabian
	"0:0:141116975", // ryley
	"0:1:419649759", // glitchgod
	"0:0:192404376", // enzo
	"0:1:54795934", // jan
	"0:1:63945665", // rafael
	"0:1:484919737", // frostlander
	"0:1:457283837", // berony
	"0:1:516167454", // hitoshii
	"0:0:421607015", // galexion
};
static const int g_NumPhoenixIDs = sizeof(g_PhoenixSteamIDs) / sizeof(g_PhoenixSteamIDs[0]);

static const char* g_LANSteamID[] = {
	"ID_LAN", // LAN
};
static const int g_NumLANIDs = sizeof(g_LANSteamID) / sizeof(g_LANSteamID[0]);

inline const char* GetAchievementAPIName(int achievementID)
{
	switch (achievementID)
	{
	case 0:
		return "ACH_PHOENIX_PARTY"; // Phoenix Party. be in a game with a tpps member.
	case 1:
		return "ACH_FIRST_BLOOD";
	case 2:
		return "ACH_LAN";
	case 3:
		return "ACH_WELCOME";
	case 4:
		return "ACH_PHOENIX_DAY";
	case 5:
		return "ACH_CST";
	case 6:
		return "ACH_CLOSE_CALL"; // Close Call
	case 7:
		return "ACH_O2";
	case 8:
		return "ACH_TRIFECTA";
	case 9:
		return "ACH_WES01";
	case 10:
		return "ACH_WES02";
	case 11:
		return "ACH_WES03";
	case 12:
		return "ACH_WES04";
	case 13:
		return "ACH_WES05";
	case 14:
		return "ACH_WES06";
	case 15:
		return "ACH_WES07";
	case 16:
		return "ACH_SNARKPIT";
	case 17:
		return "ACH_XFIRE_STRIKE";
	case 18:
		return "ACH_ROCKET";
	case 19:
		return "ACH_DISPLACER";
	case 20:
		return "ACH_PENGUIN";
	case 21:
		return "ACH_UNARMED"; // pointy end
	default:
		return nullptr;
	}
}

// =====================
// STATISTICS START
// =====================
#define PLR_KILL_STATS "plr_kill"
#define PLR_UW_KILLS_STATS "uw_kill"
#define PLR_MELEE_KILLS_STATS "ml_kill"
#define XFIRE_WIN_STATS "xfire_win"
#define PLR_WIN_STATS "win"
#define TRIPMINE_KILL_STATS "trp_kill"
#define SNARK_KILL_STATS "sqk_kill"
#define FLAG_CARRIER_KILL_STATS "flg_kill"
#define SNIPER_KILL_STATS "snp_kill"
#define KASINO_WIN_STATS "kasino_wins"
#define SPRAY_STATS "spray"
#define PLR_MATCH_STATS "matches_played"
// =====================
// STATISTICS END
// =====================

// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SHARED_ZOMBIEPANIC_ACHIEVEMENTS
#define SHARED_ZOMBIEPANIC_ACHIEVEMENTS
#pragma once


enum EAchievements
{
	ACH_KILLS100 = 0,
	ACH_500KILLS,
	ACH_1000KILLS,
	ACH_KILLS10000,
	ACH_FIRST_BLOOD,
	ACH_O2,
	ACH_UNARMED,
	ACH_UNARMED25,
	ACH_CLOSE_CALL,
	ACH_LAN,
	ACH_WELCOME,
	ACH_PHOENIX_PARTY,
	ACH_PHOENIX_DAY,
	ACH_CST,
	ACH_TRIFECTA,
	ACH_TRIP20,
	ACH_SNARK10,
	ACH_SNIPER10,
	ACH_DISPLACER,
	ACH_PENGUIN,
	ACH_MATCHES20,

	ACHV_MAX
};

#endif // SHARED_ZOMBIEPANIC_ACHIEVEMENTS