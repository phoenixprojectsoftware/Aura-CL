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
};
static const int g_NumPhoenixIDs = sizeof(g_PhoenixSteamIDs) / sizeof(g_PhoenixSteamIDs[0]);

inline const char* GetAchievementAPIName(int achievementID)
{
	switch (achievementID)
	{
	case 0:
		return "ACH_PHOENIX_PARTY"; // Phoenix Party. be in a game with a tpps member.
	case 1:
		return "ACH_FIRST_BLOOD";
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