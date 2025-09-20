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

void CheckSpecialDay()
{
	std::time_t t = std::time(nullptr);
	std::tm* now = std::localtime(&t);

	if (!now)
		return;

	int day = now->tm_mday;

	if (day == 5)
		if (!isAchievementUnlocked(5))
			UnlockAchievement(5);

	if (day == 20)
		if (!isAchievementUnlocked(4))
			UnlockAchievement(5);
}

#endif
