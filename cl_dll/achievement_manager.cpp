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

#include "stage_level.h"
#include "achievement_manager.h"
#include <ctime>

#if defined (_STEAMWORKS) && !defined (_HALO)
CAchievementMgr g_AchievementMgr;

void CAchievementMgr::UnlockAchievement(int achievementID)
{
	if (!g_StageLevel.IsCurrentMapHashValid())
		return;

	const char* apiName = GetAchievementAPIName(achievementID);
	if (apiName && SteamUserStats())
	{
		if (SteamUserStats()->SetAchievement(apiName))
		{
			SteamUserStats()->StoreStats();
		}
	}
}

void CAchievementMgr::UnlockAchievementByName(const char* apiName)
{
	if (!g_StageLevel.IsCurrentMapHashValid())
		return;

	if (!apiName || !apiName[0])
		return;

	if (SteamUserStats())
	{
		if (SteamUserStats()->SetAchievement(apiName))
		{
			SteamUserStats()->StoreStats();
		}
	}
}

bool CAchievementMgr::isAchievementUnlocked(int achievementID)
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

void CAchievementMgr::CheckSpecialDay()
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
			UnlockAchievement(4);
}

void CAchievementMgr::StatIncrement(const char* pchName)
{
	if (!g_StageLevel.IsCurrentMapHashValid())
		return;

	int statValue = 0;

	if (SteamUserStats() && SteamUserStats()->GetStat(pchName, &statValue))
	{
		SteamUserStats()->SetStat(pchName, statValue + 1);
		SteamUserStats()->StoreStats();
	}
}

#endif
