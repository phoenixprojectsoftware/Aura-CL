/****
*
* Copyright (c) 2021-2026 The Phoenix Project Software. Some Rights Reserved.
*
* AURA
*
* VGUI2 scoreboard
*
*
****/

#include <KeyValues.h>

#include <vgui/IScheme.h>
#include <vgui_controls/SectionedListPanel.h>

#include "score_panel.h"
#include "viewport_panel_names.h"

#include "../hud.h"

CScorePanel::CScorePanel(vgui2::Panel* parent)
	: BaseClass(parent, PANEL_SCOREBOARD),
	m_pPlayerList(nullptr),
	m_flNextUpdateTime(0.0)
{
	SetTitle("CROSS PRODUCT MULTIPLAYER", true);

	SetSizeable(false);
	SetMoveable(false);
	SetCloseButtonVisible(false);
	SetDeleteSelfOnClose(false);

	SetProportional(true);

	// prototype is display-only
	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);

	m_pPlayerList = new vgui2::SectionedListPanel(
		this,
		"PlayerList");

	CreateSections();

	SetVisible(false);
}

void CScorePanel::CreateSections()
{
	m_pPlayerList->RemoveAllSections();
	m_pPlayerList->RemoveAll();

	const int nameWidth = vgui2::scheme()->GetProportionalScaledValue(280);
	const int scoreWidth = vgui2::scheme()->GetProportionalScaledValue(70);
	const int deathsWidth = scoreWidth;
	const int pingWidth = scoreWidth;
	
	//
	// Active players
	//
	m_pPlayerList->AddSection(SECTION_PLAYERS, "PLAYERS");
	m_pPlayerList->SetSectionAlwaysVisible(SECTION_PLAYERS, true);
	m_pPlayerList->AddColumnToSection(SECTION_PLAYERS, "name", "PLAYER", vgui2::SectionedListPanel::COLUMN_BRIGHT, nameWidth);
	m_pPlayerList->AddColumnToSection(SECTION_PLAYERS, "score", "SCORE", vgui2::SectionedListPanel::COLUMN_BRIGHT | vgui2::SectionedListPanel::COLUMN_CENTER, scoreWidth);
	m_pPlayerList->AddColumnToSection(SECTION_PLAYERS, "deaths", "DEATHS", vgui2::SectionedListPanel::COLUMN_BRIGHT | vgui2::SectionedListPanel::COLUMN_CENTER, deathsWidth);
	m_pPlayerList->AddColumnToSection(SECTION_PLAYERS, "ping", "PING", vgui2::SectionedListPanel::COLUMN_BRIGHT | vgui2::SectionedListPanel::COLUMN_CENTER, pingWidth);

	//
	// Spectators
	// 
	m_pPlayerList->AddSection(SECTION_SPECTATORS, "Spectators");
	m_pPlayerList->AddColumnToSection(SECTION_SPECTATORS, "name", "PLAYER", vgui2::SectionedListPanel::COLUMN_BRIGHT, nameWidth);
	m_pPlayerList->AddColumnToSection(SECTION_SPECTATORS, "score", "", vgui2::SectionedListPanel::COLUMN_CENTER, scoreWidth);
	m_pPlayerList->AddColumnToSection(SECTION_SPECTATORS, "deaths", "", vgui2::SectionedListPanel::COLUMN_CENTER, deathsWidth);
	m_pPlayerList->AddColumnToSection(SECTION_SPECTATORS, "ping", "PING", vgui2::SectionedListPanel::COLUMN_BRIGHT | vgui2::SectionedListPanel::COLUMN_CENTER, pingWidth);
}

void CScorePanel::UpdatePlayerList()
{
	m_pPlayerList->RemoveAll();

	for (int clientIndex = 1; clientIndex <= MAX_PLAYERS; ++clientIndex)
	{
		// ask the engine for the latest player details for this client slot
		gEngfuncs.pfnGetPlayerInfo(clientIndex, &g_PlayerInfoList[clientIndex]);

		hud_player_info_t& playerInfo = g_PlayerInfoList[clientIndex];
		extra_player_info_t& extraInfo = g_PlayerExtraInfo[clientIndex];

		// an empty name means that this client slot is unoccupied.
		if (playerInfo.name == nullptr || playerInfo.name[0] == '\0')
		{
			continue;
		}

		const bool isSpectator = g_IsSpectator[clientIndex] != 0 || extraInfo.teamnumber == 0;

		const int section = isSpectator ? SECTION_SPECTATORS : SECTION_PLAYERS;

		KeyValues* playerData = new KeyValues("Player");

		playerData->SetInt("client", clientIndex);

		playerData->SetString("name", playerInfo.name);

		if (isSpectator)
		{
			playerData->SetString("score", "");
			playerData->SetString("deaths", "");
		}
		else
		{
			playerData->SetInt("score", extraInfo.frags);
			playerData->SetInt("deaths", extraInfo.deaths);
		}

		playerData->SetInt("ping", playerInfo.ping);

		const int itemID = m_pPlayerList->AddItem(section, playerData);

		// highlight our own row.
		if (playerInfo.thisplayer)
		{
			m_pPlayerList->SetItemBgColor(itemID, Color(80, 80, 80, 160));
		}

		playerData->deleteThis();
	}

	m_pPlayerList->InvalidateLayout();
}

void CScorePanel::OnThink()
{
	BaseClass::OnThink();

	if (!IsVisible())
		return;

	const double currentTime = gEngfuncs.GetAbsoluteTime();

	if (currentTime <= m_flNextUpdateTime)
		return;

	UpdatePlayerList();

	m_flNextUpdateTime = currentTime + 0.5;
}

void CScorePanel::Reset()
{
	m_flNextUpdateTime = 0.0;

	if (m_pPlayerList)
		m_pPlayerList->RemoveAll();

	ShowPanel(false);
}

void CScorePanel::ShowPanel(bool state)
{
	if (state == IsVisible())
		return;

	SetVisible(state);

	if (state)
	{
		m_flNextUpdateTime = 0.0;

		UpdatePlayerList();

		MoveToFront();
	}
}

const char* CScorePanel::GetName()
{
	return PANEL_SCOREBOARD;
}

vgui2::VPANEL CScorePanel::GetVPanel()
{
	return BaseClass::GetVPanel();
}

bool CScorePanel::IsVisible()
{
	return BaseClass::IsVisible();
}

void CScorePanel::SetParent(vgui2::VPANEL parent)
{
	BaseClass::SetParent(parent);
}

void CScorePanel::PerformLayout()
{
	BaseClass::PerformLayout();

	int parentWide = 640;
	int parentTall = 480;

	if (GetParent())
		GetParent()->GetSize(parentWide, parentTall);

	const int wide = vgui2::scheme()->GetProportionalScaledValue(520);

	const int tall = vgui2::scheme()->GetProportionalScaledValue(340);

	SetBounds((parentWide - wide) / 2, (parentTall - tall) / 2, wide, tall);

	const int sideMargin = vgui2::scheme()->GetProportionalScaledValue(12);
	const int topMargin = vgui2::scheme()->GetProportionalScaledValue(34);
	const int bottomMargin = sideMargin;

	m_pPlayerList->SetBounds(sideMargin, topMargin, wide - sideMargin * 2, tall - topMargin - bottomMargin);
}
