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

#include "achievement_manager.h"
#include <ctime>

#if defined (_STEAMWORKS) && !defined (_HALO)
void UnlockAchievement(int achievementID)
{
	const char* apiName = GetAchievementAPIName(achievementID);
	if (apiName && SteamUserStats())
	{
		if (SteamUserStats()->SetAchievement(apiName))
		{
			SteamUserStats()->StoreStats();
		}
	}
}

bool isAchievementUnlocked(int achievementID)
{
	const char* apiName = GetAchievementAPIName(achievementID);
	if (apiName && SteamUserStats())
	{
		bool achieved = false;
		SteamUserStats()->GetAchievement(apiName, &achieved);
		return achieved;
	}
	return false;
}

#ifdef DATE_ACH
bool IsSpecialDate()
{
	// Get the Steam server time
	uint32 steamTime = SteamUtils()->GetServerRealTime();

	// convert to calendar date
	time_t t = static_cast<time_t>(steamTime);
	tm* timeinfo = gmtime(&t); // UTC

	int day = timeinfo->tm_mday;
	int month = timeinfo->tm_mon; // 0-based, so Jan is 0

	// check if it's 5th April
	return (day == 5 && month == 3);
}
#endif

#endif
