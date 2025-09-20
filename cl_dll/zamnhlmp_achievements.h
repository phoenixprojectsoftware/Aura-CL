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
	default:
		return nullptr;
	}
}

// =====================
// STATISTICS START
// =====================
#define PLR_KILL_STATS "plr_kill"
// =====================
// STATISTICS END
// =====================