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

#if defined (_STEAMWORKS)
#include "zamnhlmp_achievements.h"

#pragma once

class CAchievementMgr
{
public:
	void UnlockAchievement(int achievementID);

	void SystemAchievement(int achievementID); // an achievement which doesn't need map verification.

	void UnlockAchievementByName(const char* apiName);

	bool isAchievementUnlocked(int achievementID);

	void CheckSpecialDay();

	void StatIncrement(const char* pchName);
};

extern CAchievementMgr g_AchievementMgr; // global achievement object. don't use this too much.
#endif
