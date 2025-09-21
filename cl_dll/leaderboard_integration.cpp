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
#ifndef _HALO

#include "leaderboard_integration.h"
#include <iostream>

CSteamLeaderboards g_Leaderboards;
SteamLeaderboard_t g_hPlayersKilledLeaderboard = 0;

CSteamLeaderboards::CSteamLeaderboards() : m_hLeaderboard(0), m_bLeaderboardFound(false)
{
}

void CSteamLeaderboards::Init()
{
	FindOrCreateLeaderboard();
}

void CSteamLeaderboards::FindOrCreateLeaderboard()
{
    if (SteamUserStats())
    {
        SteamAPICall_t hSteamAPICall = SteamUserStats()->FindOrCreateLeaderboard(
            "PlayersKilled", k_ELeaderboardSortMethodDescending, k_ELeaderboardDisplayTypeNumeric);

        // Store the async result
        CCallResult<CSteamLeaderboards, LeaderboardFindResult_t> m_CallResultFindLeaderboard;
        m_CallResultFindLeaderboard.Set(hSteamAPICall, this, &CSteamLeaderboards::OnFindLeaderboard);
    }
}

void CSteamLeaderboards::OnFindLeaderboard(LeaderboardFindResult_t* pResult, bool bIOFailure)
{
    if (!bIOFailure && pResult->m_bLeaderboardFound)
    {
        m_hLeaderboard = pResult->m_hSteamLeaderboard;
        g_hPlayersKilledLeaderboard = m_hLeaderboard; // sync to global
        m_bLeaderboardFound = true;
        std::cout << "[Steam] Leaderboard found: " << m_hLeaderboard << std::endl;
    }
}

void CSteamLeaderboards::UploadScore(int score)
{
    if (m_bLeaderboardFound && SteamUserStats())
    {
        SteamUserStats()->UploadLeaderboardScore(
            m_hLeaderboard, k_ELeaderboardUploadScoreMethodKeepBest, score, nullptr, 0);
    }
}
#endif
