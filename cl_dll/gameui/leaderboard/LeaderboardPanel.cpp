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
#include "../../achievement_manager.h"

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
		// pre 2.10.1
		{ PLR_KILL_STATS,        "Players Killed" },
		{ PLR_MATCH_STATS,  "Matches Finished" },
		{ PLR_UW_KILLS_STATS,         "Players Killed While Underwater" },
		{ PLR_MELEE_KILLS_STATS,         "Players Killed With Melee Weapons" },

		// post 2.10.1
		// WEAPONS
		// TIER 2
		{GLOCK_SHOTS, "GLOCK - shots fired"},
		{GLOCK_KILLS, "GLOCK- kills"},
		{MAGNUM_SHOTS, ".357 MAGNUM - shots fired"},
		{MAGNUM_KILLS, ".357 MAGNUM - kills"},
		{DEAGLE_SHOTS, "DESERT EAGLE - shots fired"},
		{DEAGLE_KILLS, "DESERT EAGLE - kills"},
		// TIER 3
		{MP5_SHOTS, "9mm ASSAULT RIFLE - shots fired"},
		{MP5_ALTFIRE, "9mm ASSAULT RIFLE - M203s fired"},
		{MP5_KILLS, "9mm ASSAULT RIFLE - kills"},
		{M203_KILLS, "M203 GRENADE - kills"},
		{OLR_SHOTS, "OPERATOR LIGHT RIFLE - shots fired"},
		{OLR_KILLS, "OPERATOR LIGHT RIFLE - kills"},
		{SHOTGUN_SHOTS, "SHOTGUN - shots fired"},
		{SHOTGUN_KILLS, "SHOTGUN - kills"},
		{XBOW_SHOTS, "CROSSBOW - bolts fired"},
		{XBOW_KILLS, "CROSSBOW - kills"},
		// TIER 4
		{RPG_SHOTS, "ROCKET LAUNCHER - rockets propelled"},
		{RPG_KILLS, "ROCKET LAUNCHER - kills"},
		{HX40_SHOTS, "HX-40 THUMPER - M203s fired"},
		{TAU_SHOTS, "TAU-X3 CANNON - shots fired"},
		{TAU_KILLS, "TAU-X3 CANNON - kills"},
		{GLUON_KILLS, "GLUON GUN - kills"},
		{HIVEHAND_SHOTS, "HIVEHAND - shots fired"},
		{HIVEHAND_KILLS, "HIVEHAND - kills"},
		// TIER 5
		{GRENADE_KILLS, "GRENADE - kills"},
		{SATCHEL_KILLS, "SATCHEL CHARGE - kills"},
		{TRIPMINE_SHOTS, "LASER TRIPMINES PLACED"},
		{TRIPMINE_KILLS, "LASER TRIPMINE - kills"},
		{SNARK_SHOTS, "SNARKS DROPPED"},
		{ SNARK_KILLS,        "SNARK - kills" },
		{PENGUIN_SHOTS, "PENGUINS DROPPED"},
		{PENGUIN_KILLS, "PENGUIN - kills"},
		// TIER 6
		{SAW_SHOTS, "M249 - shots fired"},
		{SAW_KILLS, "M249 - kills"},
		{DISPLACER_KILLS, "XV11382 DISPLACER CANNON - kills"},
		{SNIPER_SHOTS, "SNIPER RIFLE - shots fired"},
		{ SNIPER_KILLS,        "SNIPER RIFLE - kills" },
		// TIER 7
		{SPORE_SHOTS, "SPORE LAUNCHER - shots fired"},
		{SPORE_KILLS, "SPORE LAUNCHER - kills"},
		{SHOCK_SHOTS, "SHOCK RIFLE - shots fired"},
		{SHOCK_KILLS, "SHOCK RIFLE - kills"}
	};
}

CLeaderboardPanel::CLeaderboardPanel(VPANEL parent)
	: BaseClass(nullptr, "LeaderboardPanel")
	, m_pList(nullptr)
	, m_bStatsReady(false)
{
	SetParent(parent);
	SetSize(500, 300);
	SetTitle("SERVICE RECORD", true);
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
