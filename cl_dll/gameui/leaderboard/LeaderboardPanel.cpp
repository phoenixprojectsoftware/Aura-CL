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

#include "LeaderboardPanel.h"
#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Button.h>
#include <vgui/IVGui.h>
#include <iostream>

using namespace vgui2;

CLeaderboardPanel::CLeaderboardPanel(VPANEL parent) : BaseClass(nullptr, "LeaderboardPanel")
{
	SetParent(parent);
	SetSize(400, 300);
	SetTitle("Leaderboard", true);
	SetVisible(false);
	SetSizeable(false);

	m_pList = new ListPanel(this, "LeaderboardList");
	m_pList->AddColumnHeader(0, "rank", "RANK", 50, 0);
	m_pList->AddColumnHeader(1, "name", "PLAYER", 200, 0);
	m_pList->AddColumnHeader(2, "score", "KILLS", 100, 0);
	m_pList->SetAutoResize(ListPanel::PIN_TOPLEFT, ListPanel::AUTORESIZE_DOWN, 0, 0, 0, 0);
	m_pList->SetBounds(10, 30, 380, 240);

	AddActionSignalTarget(this);
}

void CLeaderboardPanel::Activate()
{
	BaseClass::Activate();
	MoveToFront();
	RequestFocus();
	MoveToCenterOfScreen();
	RefreshLeaderboard();
}

void CLeaderboardPanel::OnClose()
{
	SetVisible(false);
}

void CLeaderboardPanel::RefreshLeaderboard()
{
	if (g_Leaderboards.GetLeaderboardHandle() == 0)
	{
		// Leaderboard not ready yet
		return;
	}

	SteamAPICall_t hSteamAPICall = SteamUserStats()->DownloadLeaderboardEntries(
		g_Leaderboards.GetLeaderboardHandle(),
		k_ELeaderboardDataRequestGlobal,
		1, 10); // top 10

	m_SteamCallResultDownloadScores.Set(hSteamAPICall, this, &CLeaderboardPanel::OnDownloadScores);
}

void CLeaderboardPanel::OnDownloadScores(LeaderboardScoresDownloaded_t* pResult, bool bIOFailure)
{
	if (bIOFailure || pResult->m_cEntryCount <= 0)
		return;

	m_pList->RemoveAll();

	for (int i = 0; i < pResult->m_cEntryCount; i++)
	{
		LeaderboardEntry_t entry;
		int32 details[1];
		SteamUserStats()->GetDownloadedLeaderboardEntry(
			pResult->m_hSteamLeaderboardEntries,
			i,
			&entry,
			details,
			1);

		const char* name = SteamFriends()->GetFriendPersonaName(entry.m_steamIDUser);

		KeyValues* row = new KeyValues("data");
		row->SetInt("rank", entry.m_nGlobalRank);
		row->SetString("name", name);
		row->SetInt("score", entry.m_nScore);

		m_pList->AddItem(row, 0, false, false);
	}
}
#endif
