/****
*
* Copyright (c) 2021-2025 The Phoenix Project Software. Some Rights Reserved.
*
* AURA
*
* Steam Leaderboards - NOT for Excession right now
*
*
****/

#pragma once

#ifndef _HALO

#include <steamworks/steam_api.h>

class CSteamLeaderboards
{
public:
	CSteamLeaderboards();
	void Init();
	void FindOrCreateLeaderboard();
	void OnFindLeaderboard(LeaderboardFindResult_t* pResult, bool bIOFailure);
	void UploadScore(int score);

	SteamLeaderboard_t GetLeaderboardHandle() const { return m_hLeaderboard; }

private:
	SteamLeaderboard_t m_hLeaderboard = 0;
	bool m_bLeaderboardFound;
};

extern CSteamLeaderboards g_Leaderboards;
extern SteamLeaderboard_t g_hPlayersKilledLeaderboard;
#endif