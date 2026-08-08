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

#pragma once

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
	"0:0:457283837", // berony
	"0:1:516167454", // hitoshii
	"0:0:421607015", // galexion
	"0:0:105253777", // finger
	"0:1:77747696", // playmann
	"0:0:102486805", // ivan naii_
	"0:1:175018298", // maxresdefault
	"0:0:461917277" // Georgie L.
};
static const int g_NumPhoenixIDs = sizeof(g_PhoenixSteamIDs) / sizeof(g_PhoenixSteamIDs[0]);

inline bool IsPhoenixID(const char* steamID)
{
	if (!steamID || steamID[0] == '\0')
	{
		return false;
	}

	for (int index = 0; index < g_NumPhoenixIDs; ++index)
	{
		if (strcmp(steamID, g_PhoenixSteamIDs[index]) == 0)
		{
			return true;
		}
	}

	return false;
}

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
#define PLR_KILL_STATS "player_kill" // new key name after system overhaul
#define OLD_KILL_STAT "plr_kill"
#define PLR_UW_KILLS_STATS "uw_kill"
#define PLR_MELEE_KILLS_STATS "ml_kill"
#define PLR_MATCH_STATS "matches_played"

#define GLOCK_SHOTS "glock_shots"
#define GLOCK_KILLS "glock_kills"
#define MAGNUM_SHOTS "magnum_shots"
#define MAGNUM_KILLS "magnum_kills"
#define DEAGLE_SHOTS "deagle_shots"
#define DEAGLE_KILLS "deagle_kills"
#define MP5_SHOTS "mp5_shots"
#define MP5_ALTFIRE "mp5_altfire"
#define MP5_KILLS "mp5_kills"
#define M203_KILLS "m203_kills"
#define OLR_SHOTS "olr_shots"
#define OLR_KILLS "olr_kills"
#define SHOTGUN_SHOTS "shotgun_shots"
#define SHOTGUN_KILLS "shotgun_kills"
#define XBOW_SHOTS "xbow_shots"
#define XBOW_KILLS "xbow_kills"
#define RPG_SHOTS "rpg_shots"
#define RPG_KILLS "rpg_kills"
#define HX40_SHOTS "hx40_shots"
#define TAU_SHOTS "tau_shots"
#define TAU_KILLS "tau_kills"
#define GLUON_KILLS "gluon_kills"
#define HIVEHAND_SHOTS "hivehand_shots"
#define HIVEHAND_KILLS "hivehand_kills"
#define TRIPMINE_SHOTS "tripmine_shots"
#define TRIPMINE_KILLS "trp_kill"
#define SNARK_SHOTS "snark_shots"
#define SNARK_KILLS "sqk_kill"
#define PENGUIN_SHOTS "penguin_shots"
#define PENGUIN_KILLS "penguin_kills"
#define SAW_SHOTS "saw_shots"
#define SAW_KILLS "saw_kills"
#define DISPLACER_SHOTS "displacer_shots"
#define DISPLACER_SELF "displacer_self"
#define DISPLACER_KILLS "displacer_kills"
#define SNIPER_SHOTS "sniper_shots"
#define SNIPER_KILLS "snp_kill"
#define SPORE_SHOTS "spore_shots"
#define SPORE_KILLS "spore_kills"
#define SHOCK_SHOTS "shock_shots"
#define SHOCK_KILLS "shock_kills"
#define SATCHEL_KILLS "satchel_kills"
#define GRENADE_KILLS "grenade_kills"


// =====================
// STATISTICS END
// =====================

enum EAchievements
{
	ACH_100KILLS = 0,
	ACH_500KILLS,
	ACH_1000KILLS,
	ACH_HELLO_STEAMDB,
	ACH_FIRST_BLOOD,
	ACH_O2,
	ACH_UNARMED,
	ACH_UNARMED25,
	ACH_CLOSE_CALL,
	ACH_XFIRE_STRIKE,
	ACH_SNARKPIT,
	ACH_WELCOME,
	ACH_PHOENIX_PARTY,
	ACH_PHOENIX_DAY,
	ACH_CST,
	ACH_TRIP20,
	ACH_SNARK10,
	ACH_SNIPER10,
	ACH_DISPLACER,
	ACH_PENGUIN,
	ACH_MATCHES20,

	ACHV_MAX
};
