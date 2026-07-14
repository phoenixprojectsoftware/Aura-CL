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

#include <vgui_controls/Label.h>
#include <vgui_controls/SectionedListPanel.h>

#include "score_panel.h"
#include "viewport_panel_names.h"

#include "../hud.h"

extern int iTeamColors[5][3];
extern int iNumberOfTeamColors;

static void GetDisplayMapName(
	const char* levelPath,
	char* output,
	std::size_t outputSize)
{
	if (!output || outputSize == 0)
		return;

	output[0] = '\0';

	if (!levelPath || levelPath[0] == '\0')
		return;

	const char* mapName = levelPath;

	const char* slash =
		strrchr(levelPath, '/');

	const char* backslash =
		strrchr(levelPath, '\\');

	if (slash && slash + 1 > mapName)
		mapName = slash + 1;

	if (backslash && backslash + 1 > mapName)
		mapName = backslash + 1;

	strncpy(
		output,
		mapName,
		outputSize - 1);

	output[outputSize - 1] = '\0';

	char* extension =
		strrchr(output, '.');

	if (extension &&
		stricmp(extension, ".bsp") == 0)
	{
		*extension = '\0';
	}
}

CScorePanel::CScorePanel(vgui2::Panel* parent)
	: BaseClass(parent, PANEL_SCOREBOARD),
	m_pMapLabel(nullptr),
	m_pPlayerCountLabel(nullptr),
	m_pPlayerList(nullptr),
	m_flNextUpdateTime(0.0)
{
	SetTitle(" ", true);

	SetSizeable(false);
	SetMoveable(false);
	SetCloseButtonVisible(false);
	SetDeleteSelfOnClose(false);

	SetProportional(true);

	// prototype is display-only
	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);

	m_pMapLabel = new vgui2::Label(
		this,
		"MapLabel",
		"");

	m_pMapLabel->SetContentAlignment(
		vgui2::Label::a_west);

	m_pPlayerCountLabel = new vgui2::Label(
		this,
		"PlayerCountLabel",
		"");

	m_pPlayerCountLabel->SetContentAlignment(
		vgui2::Label::a_east);

	m_pPlayerList = new vgui2::SectionedListPanel(
		this,
		"PlayerList");

	CreateSections();

	SetVisible(false);
}

bool CScorePanel::ScoreSort(vgui2::SectionedListPanel* list, int itemID1, int itemID2)
{
	if (!list)
		return false;

	KeyValues* player1 = list->GetItemData(itemID1);
	KeyValues* player2 = list->GetItemData(itemID2);

	if (!player1 || !player2)
		return false;

	const int score1 = player1->GetInt("score_value");
	const int score2 = player2->GetInt("score_value");

	// higher score appears first
	if (score1 != score2)
		return score1 > score2;

	const int deaths1 = player1->GetInt("deaths_value");
	const int deaths2 = player2->GetInt("deaths_value");

	// when scores match, fewer deaths appears first
	if (deaths1 != deaths2)
		return deaths1 < deaths2;

	const char* name1 = player1->GetString("name");
	const char* name2 = player2->GetString("name");

	// final fallback = alphabetical by name
	return stricmp(name1, name2) < 0;
}

void CScorePanel::CreateSections()
{
	m_pPlayerList->RemoveAll();
	m_pPlayerList->RemoveAllSections();

	if (gHUD.m_Teamplay)
	{
		for (int teamIndex = 1;
			teamIndex <= MAX_TEAMS;
			++teamIndex)
		{
			const team_info_t& team =
				g_TeamInfo[teamIndex];

			if (team.name[0] == '\0')
				continue;

			if (team.players <= 0)
				continue;

			const int sectionID =
				SECTION_TEAM_BASE + teamIndex;

			// Aura displays section names through column headings,
			// rather than the AddSection name argument.
			m_pPlayerList->AddSection(
				sectionID,
				"",
				&CScorePanel::ScoreSort);

			m_pPlayerList->SetSectionAlwaysVisible(
				sectionID,
				true);

			AddPlayerColumns(
				sectionID,
				team.name,
				true);

			ApplyTeamSectionColor(sectionID, team.teamnumber);
		}
	}
	else
	{
		m_pPlayerList->AddSection(
			SECTION_PLAYERS,
			"",
			&CScorePanel::ScoreSort);

		m_pPlayerList->SetSectionAlwaysVisible(
			SECTION_PLAYERS,
			true);

		AddPlayerColumns(
			SECTION_PLAYERS,
			"Players",
			true);
	}

	m_pPlayerList->AddSection(
		SECTION_SPECTATORS,
		"",
		&CScorePanel::ScoreSort);

	AddPlayerColumns(
		SECTION_SPECTATORS,
		"Spectators",
		false);
}

void CScorePanel::AddPlayerColumns(
	int sectionID,
	const char* sectionName,
	bool showStatHeadings)
{
	const int nameWidth =
		vgui2::scheme()->GetProportionalScaledValue(280);

	const int scoreWidth =
		vgui2::scheme()->GetProportionalScaledValue(70);

	const int deathsWidth =
		vgui2::scheme()->GetProportionalScaledValue(70);

	const int pingWidth =
		vgui2::scheme()->GetProportionalScaledValue(70);

	m_pPlayerList->AddColumnToSection(
		sectionID,
		"name",
		sectionName,
		vgui2::SectionedListPanel::COLUMN_BRIGHT,
		nameWidth);

	m_pPlayerList->AddColumnToSection(
		sectionID,
		"score",
		showStatHeadings ? "Score" : "",
		vgui2::SectionedListPanel::COLUMN_BRIGHT |
		vgui2::SectionedListPanel::COLUMN_CENTER,
		scoreWidth);

	m_pPlayerList->AddColumnToSection(
		sectionID,
		"deaths",
		showStatHeadings ? "Deaths" : "",
		vgui2::SectionedListPanel::COLUMN_BRIGHT |
		vgui2::SectionedListPanel::COLUMN_CENTER,
		deathsWidth);

	m_pPlayerList->AddColumnToSection(
		sectionID,
		"ping",
		showStatHeadings ? "Ping" : "",
		vgui2::SectionedListPanel::COLUMN_BRIGHT |
		vgui2::SectionedListPanel::COLUMN_CENTER,
		pingWidth);
}

void CScorePanel::ApplyTeamSectionColor(int sectionID, int teamNumber)
{
	if (iNumberOfTeamColors <= 0)
		return;

	int colorIndex = teamNumber % iNumberOfTeamColors;

	if (colorIndex < 0)
		colorIndex += iNumberOfTeamColors;

	m_pPlayerList->SetSectionFgColor(sectionID, Color(iTeamColors[colorIndex][0], iTeamColors[colorIndex][1], iTeamColors[colorIndex][2], 255));
}

int CScorePanel::FindTeamIndex(
	const char* teamName) const
{
	if (!teamName || teamName[0] == '\0')
		return 0;

	for (int teamIndex = 1;
		teamIndex <= MAX_TEAMS;
		++teamIndex)
	{
		if (g_TeamInfo[teamIndex].name[0] == '\0')
			continue;

		if (stricmp(
			teamName,
			g_TeamInfo[teamIndex].name) == 0)
		{
			return teamIndex;
		}
	}

	return 0;
}

int CScorePanel::GetSectionForPlayer(int clientIndex) const
{
	if (clientIndex < 1 || clientIndex > MAX_PLAYERS)
		return SECTION_SPECTATORS;

	const extra_player_info_t& extraInfo = g_PlayerExtraInfo[clientIndex];

	if (g_IsSpectator[clientIndex])
		return SECTION_SPECTATORS;

	if (!gHUD.m_Teamplay)
		return SECTION_PLAYERS;

	const int teamIndex = FindTeamIndex(extraInfo.teamname);

	if (teamIndex <= 0)
		return SECTION_SPECTATORS;

	return SECTION_TEAM_BASE + teamIndex;
}

void CScorePanel::UpdateHeader()
{
	const char* serverName = gEngfuncs.pfnGetCvarString("hostname");
	if (serverName && serverName[0] != '\0')
		SetTitle(serverName, true);
	else
	{
		SetTitle("CROSS PRODUCT MULTIPLAYER", true);
	}

	char mapName[128];

	GetDisplayMapName(gEngfuncs.pfnGetLevelName(), mapName, sizeof(mapName));

	char mapText[160];

	if (mapName[0] != '\0')
	{
		snprintf(mapText, sizeof(mapText), "MAP: %s", mapName);
	}
	else
	{
		strncpy(mapText, "MAP: UNKNOWN", sizeof(mapText) - 1);

		mapText[sizeof(mapText) - 1] = '\0';
	}

	m_pMapLabel->SetText(mapText);

	int connectedPlayers = 0;

	for (int clientIndex = 1; clientIndex <= MAX_PLAYERS; ++clientIndex)
	{
		gEngfuncs.pfnGetPlayerInfo(clientIndex, &g_PlayerInfoList[clientIndex]);

		const hud_player_info_t& playerInfo = g_PlayerInfoList[clientIndex];

		if (playerInfo.name && playerInfo.name[0] != '\0')
			++connectedPlayers;

		int maxPlayers = gEngfuncs.GetMaxClients();

		char playerCountText[64];

		snprintf(playerCountText, sizeof(playerCountText), "%d / %d PLAYERS", connectedPlayers, maxPlayers);

		m_pPlayerCountLabel->SetText(playerCountText);
	}
}

void CScorePanel::UpdatePlayerList()
{
	UpdateHeader();

	CreateSections();

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

		const int section = GetSectionForPlayer(clientIndex);
		const bool isSpectator = section == SECTION_SPECTATORS;

		KeyValues* playerData = new KeyValues("Player");

		playerData->SetInt("client", clientIndex);

		playerData->SetString("name", playerInfo.name);
		playerData->SetInt("score_value", extraInfo.frags);
		playerData->SetInt("deaths_value", extraInfo.deaths);

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

		const int itemID =
			m_pPlayerList->AddItem(
				section,
				playerData);

		if (!isSpectator &&
			gHUD.m_Teamplay &&
			iNumberOfTeamColors > 0)
		{
			int colorIndex =
				extraInfo.teamnumber %
				iNumberOfTeamColors;

			if (colorIndex < 0)
				colorIndex += iNumberOfTeamColors;

			m_pPlayerList->SetItemFgColor(
				itemID,
				Color(
					iTeamColors[colorIndex][0],
					iTeamColors[colorIndex][1],
					iTeamColors[colorIndex][2],
					255));
		}

		// highlight our own row.
		if (playerInfo.thisplayer)
		{
			if (!isSpectator &&
				gHUD.m_Teamplay &&
				iNumberOfTeamColors > 0)
			{
				int colorIndex =
					extraInfo.teamnumber %
					iNumberOfTeamColors;

				if (colorIndex < 0)
					colorIndex += iNumberOfTeamColors;

				m_pPlayerList->SetItemBgColor(
					itemID,
					Color(
						iTeamColors[colorIndex][0],
						iTeamColors[colorIndex][1],
						iTeamColors[colorIndex][2],
						110));
			}
			else
			{
				m_pPlayerList->SetItemBgColor(
					itemID,
					Color(100, 100, 100, 110));
			}
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

	if (m_pMapLabel)
		m_pMapLabel->SetText("");

	if (m_pPlayerCountLabel)
		m_pPlayerCountLabel->SetText("");

	if (m_pPlayerList)
		m_pPlayerList->RemoveAll();

	SetTitle("CROSS PRODUCT MULTIPLAYER", true);
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

	const int sideMargin =
		vgui2::scheme()->GetProportionalScaledValue(12);

	const int titleBarBottom =
		vgui2::scheme()->GetProportionalScaledValue(34);

	const int headerTall =
		vgui2::scheme()->GetProportionalScaledValue(24);

	const int headerGap =
		vgui2::scheme()->GetProportionalScaledValue(4);

	const int bottomMargin =
		vgui2::scheme()->GetProportionalScaledValue(12);

	const int headerWide =
		wide - sideMargin * 2;

	m_pMapLabel->SetBounds(
		sideMargin,
		titleBarBottom,
		headerWide / 2,
		headerTall);

	m_pPlayerCountLabel->SetBounds(
		sideMargin + headerWide / 2,
		titleBarBottom,
		headerWide / 2,
		headerTall);

	const int listY =
		titleBarBottom +
		headerTall +
		headerGap;

	m_pPlayerList->SetBounds(
		sideMargin,
		listY,
		headerWide,
		tall - listY - bottomMargin);
}
