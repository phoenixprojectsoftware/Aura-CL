/****
*
* Copyright (c) 2021-2025 The Phoenix Project Software. Some Rights Reserved.
*
* AURA
*
* Steam Player Stats Panel
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

using namespace vgui2;

namespace
{
	struct SteamStatRow
	{
		const char* pszApiName;
		const char* pszDisplayName;
	};

	static const SteamStatRow g_PlayerStats[] =
	{
		{ "plr_kill",        "Players Killed" },
		{ "trp_kill",        "Players Killed with TRIPMINE" },
		{ "sqk_kill",        "Players Killed with SNARK" },
		{ "snp_kill",        "Players Killed with SNIPER RIFLE" },
		{ "matches_played",  "Matches Played" },
		{ "uw_kill",         "Players Killed While Underwater" },
		{ "ml_kill",         "Players Killed With Melee Weapons" },
	};
}

CLeaderboardPanel::CLeaderboardPanel(VPANEL parent)
	: BaseClass(nullptr, "LeaderboardPanel")
	, m_pList(nullptr)
	, m_bStatsReady(false)
{
	SetParent(parent);
	SetSize(500, 300);
	SetTitle("Service Record", true);
	SetVisible(false);
	SetSizeable(false);

	m_pList = new ListPanel(this, "StatsList");

	m_pList->AddColumnHeader(0, "stat", "STAT", 330, 0);
	m_pList->AddColumnHeader(1, "value", "VALUE", 100, 0);

	m_pList->SetAutoResize(ListPanel::PIN_TOPLEFT, ListPanel::AUTORESIZE_DOWN, 0, 0, 0, 0);
	m_pList->SetBounds(10, 30, 480, 240);

	AddActionSignalTarget(this);
}

void CLeaderboardPanel::Activate()
{
	BaseClass::Activate();

	MoveToFront();
	RequestFocus();
	MoveToCenterOfScreen();

	RefreshStats();
}

void CLeaderboardPanel::OnClose()
{
	SetVisible(false);
}

void CLeaderboardPanel::RefreshStats()
{
	if (!SteamUserStats() || !SteamUser())
	{
		m_pList->RemoveAll();

		KeyValues* failRow = new KeyValues("data");
		failRow->SetString("stat", "Steam unavailable");
		failRow->SetString("value", "");
		m_pList->AddItem(failRow, 0, false, false);

		return;
	}

	m_bStatsReady = false;

	m_pList->RemoveAll();

	KeyValues* row = new KeyValues("data");
	row->SetString("stat", "Loading Steam stats...");
	row->SetString("value", "");
	m_pList->AddItem(row, 0, false, false);

	CSteamID steamID = SteamUser()->GetSteamID();

	SteamAPICall_t hSteamAPICall = SteamUserStats()->RequestUserStats(steamID);

	if (hSteamAPICall == k_uAPICallInvalid)
	{
		m_pList->RemoveAll();

		KeyValues* failRow = new KeyValues("data");
		failRow->SetString("stat", "Failed to request Steam stats");
		failRow->SetString("value", "");
		m_pList->AddItem(failRow, 0, false, false);

		return;
	}

	m_CallResultUserStatsReceived.Set(
		hSteamAPICall,
		this,
		&CLeaderboardPanel::OnUserStatsReceived
	);
}

void CLeaderboardPanel::OnUserStatsReceived(UserStatsReceived_t* pResult, bool bIOFailure)
{
	if (bIOFailure || !pResult)
	{
		m_bStatsReady = false;

		m_pList->RemoveAll();

		KeyValues* row = new KeyValues("data");
		row->SetString("stat", "Steam stats request failed");
		row->SetString("value", "");
		m_pList->AddItem(row, 0, false, false);

		return;
	}

	Msg("ServiceRecord: UserStatsReceived result = %d, gameID = %llu\n",
		pResult->m_eResult,
		pResult->m_nGameID);

	if (pResult->m_eResult != k_EResultOK)
	{
		m_bStatsReady = false;

		m_pList->RemoveAll();

		KeyValues* row = new KeyValues("data");
		row->SetString("stat", "Steam stats unavailable");
		row->SetInt("value", static_cast<int>(pResult->m_eResult));
		m_pList->AddItem(row, 0, false, false);

		return;
	}

	m_bStatsReady = true;
	PopulateStatsTable();
}
void CLeaderboardPanel::PopulateStatsTable()
{
	if (!SteamUserStats() || !m_bStatsReady)
		return;

	m_pList->RemoveAll();

	for (int i = 0; i < ARRAYSIZE(g_PlayerStats); ++i)
	{
		const SteamStatRow& stat = g_PlayerStats[i];

		int value = 0;
		bool bGotStat = SteamUserStats()->GetStat(stat.pszApiName, &value);

		KeyValues* row = new KeyValues("data");

		if (bGotStat)
		{
			row->SetString("stat", stat.pszDisplayName);
			row->SetInt("value", value);
		}
		else
		{
			row->SetString("stat", stat.pszDisplayName);
			row->SetString("value", "N/A");
		}

		m_pList->AddItem(row, 0, false, false);
	}

	m_pList->InvalidateLayout();
}

#endif
