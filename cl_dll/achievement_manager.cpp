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
#include "hud.h"
#include "cl_gametype.h"
#include <ctime>

CAchievementMgr g_AchievementMgr;

void CAchievementMgr::UnlockAchievement(int achievementID)
{
#ifndef _HALO
	if (!g_StageLevel.IsCurrentMapHashValid())
		return;

	if (SANDBOX == g_iGameType)
		return;

	const char* apiName = GetAchievementAPIName(achievementID);
	if (apiName && SteamUserStats())
	{
		if (SteamUserStats()->SetAchievement(apiName))
		{
			SteamUserStats()->StoreStats();
		}
	}
#endif
}

void CAchievementMgr::SystemAchievement(int achievementID)
{
#ifndef _HALO
	const char* apiName = GetAchievementAPIName(achievementID);
	if (apiName && SteamUserStats())
	{
		if (SteamUserStats()->SetAchievement(apiName))
		{
			SteamUserStats()->StoreStats();
		}
	}
#endif
}

void CAchievementMgr::UnlockAchievementByName(const char* apiName)
{
#ifndef _HALO
	if (!g_StageLevel.IsCurrentMapHashValid())
		return;

	if (SANDBOX == g_iGameType)
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
#endif
}

bool CAchievementMgr::isAchievementUnlocked(int achievementID)
{
#ifndef _HALO
	const char* apiName = GetAchievementAPIName(achievementID);
	if (apiName && SteamUserStats())
	{
		bool achieved = false;
		SteamUserStats()->GetAchievement(apiName, &achieved);
		return achieved;
	}
#endif
	return false;
}

void CAchievementMgr::CheckSpecialDay()
{
#ifndef _HALO
	std::time_t t = std::time(nullptr);
	std::tm* now = std::localtime(&t);

	if (!now)
		return;

	int day = now->tm_mday;

	if (day == 5)
		if (!isAchievementUnlocked(5))
			SystemAchievement(5);

	if (day == 20)
		if (!isAchievementUnlocked(4))
			SystemAchievement(4);
#endif
}

void CAchievementMgr::StatIncrement(const char* pchName, int increment)
{
#ifndef _HALO
	if (!g_StageLevel.IsCurrentMapHashValid())
		return;

	if (SANDBOX == g_iGameType)
		return;

	int statValue = 0;

	if (SteamUserStats() && SteamUserStats()->GetStat(pchName, &statValue))
	{
		SteamUserStats()->SetStat(pchName, statValue + increment);
		SteamUserStats()->StoreStats();
	}
#endif
}
