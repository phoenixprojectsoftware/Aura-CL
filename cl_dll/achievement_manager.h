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

void UnlockAchievement(int achievementID);

bool isAchievementUnlocked(int achievementID);

void CheckSpecialDay();

#endif
