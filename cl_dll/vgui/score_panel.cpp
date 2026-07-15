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

#include <vgui/IInput.h>
#include <vgui/IInputInternal.h>
#include <vgui/ISurface.h>
#include <vgui/IScheme.h>

#include <vgui_controls/ImageList.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Menu.h>
#include <vgui_controls/SectionedListPanel.h>

#include "bridge.h"
#include "score_panel.h"
#include "steam_avatar.h"
#include "viewport_panel_names.h"

#include "../hud.h"
#include "../voice_status.h"
#include "../steam_id.h"
#include "../color_tags.h"

#include <steamworks/steam_api.h>

extern int iTeamColors[5][3];
extern int iNumberOfTeamColors;

char gServerName[128];

static std::uint64_t Steam2ToSteamID64(
	const std::string& steam2ID)
{
	if (steam2ID.empty())
		return 0;

	unsigned int authServer = 0;
	unsigned long accountNumber = 0;

	// OpenAG's AuthID handler removes the "STEAM_" prefix, leaving:
	//
	//     0:Y:Z
	//
	// The first value is the Steam2 universe and is not needed for the
	// normal public individual-account conversion.
	if (sscanf(
		steam2ID.c_str(),
		"%*u:%u:%lu",
		&authServer,
		&accountNumber) != 2)
	{
		return 0;
	}

	if (authServer > 1)
		return 0;

	const std::uint64_t accountID =
		static_cast<std::uint64_t>(accountNumber) * 2ULL +
		static_cast<std::uint64_t>(authServer);

	return 76561197960265728ULL + accountID;
}

extern "C"
{
	void IN_ActivateMouse();
	void IN_DeactivateMouse();
}

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
	m_pServerNameLabel(nullptr),
	m_pMapLabel(nullptr),
	m_pPlayerCountLabel(nullptr),
	m_pPlayerList(nullptr),
	m_pPlayerMenu(nullptr),
	m_bMousePointerEnabled(false),
	m_iSelectedClient(0),
	m_iMuteMenuItem(-1),
	m_pImageList(nullptr),
	m_hHeaderFont(vgui2::INVALID_FONT),
	m_hPlayerFont(vgui2::INVALID_FONT),
	m_flNextUpdateTime(0.0)
{
	SetTitle(" ", true);
	SetTitleBarVisible(false);

	SetSizeable(false);
	SetMoveable(false);
	SetCloseButtonVisible(false);
	SetDeleteSelfOnClose(false);

	SetProportional(true);

	if (parent)
	{
		SetScheme(parent->GetScheme());
	}

	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);

	m_pServerNameLabel =
		new vgui2::Label(
			this,
			"ServerNameLabel",
			GAME_NAME);

	m_pServerNameLabel->SetContentAlignment(
		vgui2::Label::a_center);

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

	m_pPlayerList->SetClickable(true);
	m_pPlayerList->SetMouseInputEnabled(true);

	m_pImageList = new vgui2::ImageList(true);

	m_pPlayerList->SetImageList(m_pImageList, true);

	m_pPlayerMenu = new vgui2::Menu(
		this,
		"PlayerMenu");
	m_pPlayerMenu->SetVisible(false);

	m_iMuteMenuItem =
		m_pPlayerMenu->AddMenuItem(
			"MutePlayer",
			"Mute player",
			"TogglePlayerMute",
			this);

	for (int clientIndex = 0;
		clientIndex <= SCOREBOARD_MAX_PLAYERS;
		++clientIndex)
	{
		m_pAvatars[clientIndex] = nullptr;
	}

	for (int clientIndex = 1;
		clientIndex <= SCOREBOARD_MAX_PLAYERS;
		++clientIndex)
	{
		CSteamAvatarImage* avatar =
			new CSteamAvatarImage();

		avatar->SetSize(
			vgui2::scheme()->GetProportionalScaledValue(18),
			vgui2::scheme()->GetProportionalScaledValue(18));

		m_pAvatars[clientIndex] = avatar;

		m_pPlayerList->SetLineSpacingOverride(
			vgui2::scheme()->GetProportionalScaledValue(21));

		m_pImageList->SetImageAtIndex(
			clientIndex,
			avatar);
	}

	LoadControlSettings(
		"ui/resource/ScorePanel.res");

	// Force VGUI to run ApplySchemeSettings() and PerformLayout().
	InvalidateLayout(true, true);

	CreateSections();

	SetVisible(false);
}

CScorePanel::~CScorePanel()
{
	// m_pPlayerList owns m_pImageList.
	// m_pImageList owns the avatar image objects.
	m_pImageList = nullptr;

	for (int clientIndex = 0;
		clientIndex <= SCOREBOARD_MAX_PLAYERS;
		++clientIndex)
	{
		m_pAvatars[clientIndex] = nullptr;
	}
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

	if (m_hHeaderFont != vgui2::INVALID_FONT)
	{
		if (gHUD.m_Teamplay)
		{
			for (int teamIndex = 1;
				teamIndex <= MAX_TEAMS;
				++teamIndex)
			{
				const team_info_t& team =
					g_TeamInfo[teamIndex];

				if (team.name[0] == '\0' ||
					team.players <= 0)
				{
					continue;
				}

				m_pPlayerList->SetFontSection(
					SECTION_TEAM_BASE + teamIndex,
					m_hHeaderFont);
			}
		}
		else
		{
			m_pPlayerList->SetFontSection(
				SECTION_PLAYERS,
				m_hHeaderFont);
		}

		m_pPlayerList->SetFontSection(
			SECTION_SPECTATORS,
			m_hHeaderFont);
	}
}

void CScorePanel::AddPlayerColumns(
	int sectionID,
	const char* sectionName,
	bool showStatHeadings)
{
	const int nameWidth =
		vgui2::scheme()->GetProportionalScaledValue(208);

	const int scoreWidth =
		vgui2::scheme()->GetProportionalScaledValue(70);

	const int deathsWidth =
		vgui2::scheme()->GetProportionalScaledValue(70);

	const int pingWidth =
		vgui2::scheme()->GetProportionalScaledValue(70);

	const int statusWidth =
		vgui2::scheme()->GetProportionalScaledValue(48);

	const int avatarWidth =
		vgui2::scheme()->GetProportionalScaledValue(24);

	m_pPlayerList->AddColumnToSection(
		sectionID,
		"avatar",
		"",
		vgui2::SectionedListPanel::COLUMN_IMAGE |
			vgui2::SectionedListPanel::COLUMN_CENTER,
		avatarWidth);

	m_pPlayerList->AddColumnToSection(
		sectionID,
		"status",
		"",
		vgui2::SectionedListPanel::COLUMN_CENTER,
		statusWidth);

	m_pPlayerList->AddColumnToSection(
		sectionID,
		"name",
		sectionName,
		vgui2::SectionedListPanel::COLUMN_BRIGHT,
		nameWidth,
		m_hHeaderFont);

	m_pPlayerList->AddColumnToSection(
		sectionID,
		"score",
		showStatHeadings ? "Score" : "",
		vgui2::SectionedListPanel::COLUMN_BRIGHT |
		vgui2::SectionedListPanel::COLUMN_CENTER,
		scoreWidth,
		m_hHeaderFont);

	m_pPlayerList->AddColumnToSection(
		sectionID,
		"deaths",
		showStatHeadings ? "Deaths" : "",
		vgui2::SectionedListPanel::COLUMN_BRIGHT |
		vgui2::SectionedListPanel::COLUMN_CENTER,
		deathsWidth,
		m_hHeaderFont);

	m_pPlayerList->AddColumnToSection(
		sectionID,
		"ping",
		showStatHeadings ? "Ping" : "",
		vgui2::SectionedListPanel::COLUMN_BRIGHT |
		vgui2::SectionedListPanel::COLUMN_CENTER,
		pingWidth,
		m_hHeaderFont);
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
	const char* serverName = gServerName;

	if (m_pServerNameLabel)
	{
		if (serverName && serverName[0] != '\0')
		{
			m_pServerNameLabel->SetText(serverName);
		}
		else
			m_pServerNameLabel->SetText(GAME_NAME);
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
	}

	int maxPlayers = gEngfuncs.GetMaxClients();

	char playerCountText[64];

	snprintf(playerCountText, sizeof(playerCountText), "%d / %d PLAYERS", connectedPlayers, maxPlayers);

	m_pPlayerCountLabel->SetText(playerCountText);
}

void CScorePanel::UpdatePlayerAvatar(
	int clientIndex,
	std::uint64_t steamID)
{
	if (clientIndex < 1 ||
		clientIndex > SCOREBOARD_MAX_PLAYERS)
	{
		return;
	}

	CSteamAvatarImage* avatar =
		m_pAvatars[clientIndex];

	if (!avatar)
	{
		return;
	}

	if (steamID == 0)
	{
		avatar->Clear();
		return;
	}

	if (avatar->GetSteamID() != steamID)
	{
		avatar->SetSteamID(steamID);
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
		if (playerInfo.name == nullptr ||
			playerInfo.name[0] == '\0')
		{
			if (clientIndex >= 1 &&
				clientIndex <= SCOREBOARD_MAX_PLAYERS &&
				m_pAvatars[clientIndex])
			{
				m_pAvatars[clientIndex]->Clear();
			}

			continue;
		}

		const std::string& steam2ID = steam_id::get_steam_id(clientIndex - 1);
		const std::uint64_t steamID = Steam2ToSteamID64(steam2ID);

#ifdef _DEBUG
		static bool printedSteamIDs[
			SCOREBOARD_MAX_PLAYERS + 1] = {};

			if (!printedSteamIDs[clientIndex] &&
				!steam2ID.empty())
			{
				gEngfuncs.Con_Printf(
					"Scoreboard slot %d: Steam2=%s, SteamID64=%llu\n",
					clientIndex,
					steam2ID.c_str(),
					static_cast<unsigned long long>(
						steamID));

				printedSteamIDs[clientIndex] = true;
			}
#endif

		UpdatePlayerAvatar(clientIndex, steamID);

		const int section = GetSectionForPlayer(clientIndex);
		const bool isSpectator = section == SECTION_SPECTATORS;

		KeyValues* playerData = new KeyValues("Player");

		playerData->SetInt("client", clientIndex);
			
		playerData->SetInt(
			"avatar",
			steamID != 0
			? clientIndex
			: 0);

		bool isMuted = false;
		if (!playerInfo.thisplayer && GetClientVoiceMgr())
		{
			isMuted = GetClientVoiceMgr()->IsPlayerBlocked(clientIndex);
		}

		playerData->SetString("status", isMuted ? "Muted" : "");

		char displayName[256];

		color_tags::strip_color_tags(displayName, playerInfo.name, sizeof(displayName));

		playerData->SetString("name", displayName);
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

		if (m_hPlayerFont != vgui2::INVALID_FONT)
		{
			m_pPlayerList->SetItemFont(
				itemID,
				m_hPlayerFont);
		}

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

		if (isMuted)
		{
			m_pPlayerList->SetItemFgColor(
				itemID,
				Color(
					150,
					150,
					150,
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

	for (int clientIndex = 1;
		clientIndex <= SCOREBOARD_MAX_PLAYERS;
		++clientIndex)
	{
		if (m_pAvatars[clientIndex])
		{
			m_pAvatars[clientIndex]->Clear();
		}
	}

	if (m_pServerNameLabel)
	{
		m_pServerNameLabel->SetText(GAME_NAME);
	}
	ShowPanel(false);
}

void CScorePanel::ShowPanel(bool state)
{
	if (state == IsVisible())
		return;

	if (state)
	{
		SetVisible(true);

		// Ensures ApplySchemeSettings() has run before sections and rows
		// are recreated with the cached font handles.
		InvalidateLayout(true, true);

		m_flNextUpdateTime = 0.0;
		m_iSelectedClient = 0;

		UpdatePlayerList();
		MoveToFront();
	}
	else
	{
		EnableMousePointer(false);

		m_iSelectedClient = 0;

		if (m_pPlayerMenu)
			m_pPlayerMenu->SetVisible(false);

		SetVisible(false);
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

void CScorePanel::ApplySchemeSettings(
	vgui2::IScheme* scheme)
{
	BaseClass::ApplySchemeSettings(scheme);

	if (!scheme)
		return;

	m_hHeaderFont =
		scheme->GetFont(
			"ScoreboardHeader",
			false);

	m_hPlayerFont =
		scheme->GetFont(
			"ScoreboardPlayer",
			false);

	if (m_hHeaderFont != vgui2::INVALID_FONT &&
		m_pPlayerList)
	{
		m_pPlayerList->SetHeaderFont(
			m_hHeaderFont);
	}

	if (m_hPlayerFont != vgui2::INVALID_FONT &&
		m_pPlayerList)
	{
		m_pPlayerList->SetRowFont(
			m_hPlayerFont);
	}

#ifdef _DEBUG
	gEngfuncs.Con_Printf(
		"Scoreboard fonts: header=%u player=%u scheme=%u\n",
		static_cast<unsigned int>(m_hHeaderFont),
		static_cast<unsigned int>(m_hPlayerFont),
		static_cast<unsigned int>(GetScheme()));
#endif

	InvalidateLayout();
}

void CScorePanel::PerformLayout()
{
	BaseClass::PerformLayout();

	int parentWide = 640;
	int parentTall = 480;

	if (GetParent())
	{
		GetParent()->GetSize(
			parentWide,
			parentTall);
	}

	const int wide =
		vgui2::scheme()->GetProportionalScaledValue(520);

	const int tall =
		vgui2::scheme()->GetProportionalScaledValue(380);

	SetBounds(
		(parentWide - wide) / 2,
		(parentTall - tall) / 2,
		wide,
		tall);

	const int sideMargin =
		vgui2::scheme()->GetProportionalScaledValue(12);

	const int topMargin =
		vgui2::scheme()->GetProportionalScaledValue(8);

	const int serverNameTall =
		vgui2::scheme()->GetProportionalScaledValue(26);

	const int serverNameGap =
		vgui2::scheme()->GetProportionalScaledValue(2);

	const int headerTall =
		vgui2::scheme()->GetProportionalScaledValue(20);

	const int headerGap =
		vgui2::scheme()->GetProportionalScaledValue(2);

	const int bottomMargin =
		vgui2::scheme()->GetProportionalScaledValue(8);

	const int contentWide =
		wide - sideMargin * 2;

	// Server hostname/title.
	if (m_pServerNameLabel)
	{
		m_pServerNameLabel->SetBounds(
			sideMargin,
			topMargin,
			contentWide,
			serverNameTall);
	}

	// Map and player-count row.
	const int headerY =
		topMargin +
		serverNameTall +
		serverNameGap;

	if (m_pMapLabel)
	{
		m_pMapLabel->SetBounds(
			sideMargin,
			headerY,
			contentWide / 2,
			headerTall);
	}

	if (m_pPlayerCountLabel)
	{
		m_pPlayerCountLabel->SetBounds(
			sideMargin + contentWide / 2,
			headerY,
			contentWide - contentWide / 2,
			headerTall);
	}

	// Team/player list below the header row.
	const int listY =
		headerY +
		headerTall +
		headerGap;

	const int listTall =
		tall -
		listY -
		bottomMargin;

	if (m_pPlayerList)
	{
		m_pPlayerList->SetBounds(
			sideMargin,
			listY,
			contentWide,
			listTall);
	}
}

void CScorePanel::EnableMousePointer(bool enable)
{
	if (enable && !IsVisible())
		return;

	SetMouseInputEnabled(enable);
	SetKeyBoardInputEnabled(false);

	if (m_pPlayerList)
		m_pPlayerList->SetMouseInputEnabled(enable);

	if (enable)
	{
		// Clear gameplay actions which may already be held before handing
		// the mouse to the scoreboard.
		gEngfuncs.pfnClientCmd("-attack\n");
		gEngfuncs.pfnClientCmd("-attack2\n");
		IN_DeactivateMouse();
		m_bMousePointerEnabled = true;

		int cursorX = 0;
		int cursorY = 0;

		vgui2::input()->GetCursorPos(cursorX, cursorY);

		vgui2::input()->SetCursorPos(cursorX, cursorY);
	}
	else
	{
		if (m_pPlayerMenu)
			m_pPlayerMenu->SetVisible(false);

		IN_ActivateMouse();
		m_bMousePointerEnabled = false;
	}
}

void CScorePanel::OnItemContextMenu(int itemID)
{
	EnableMousePointer(true);
	OpenPlayerMenu(itemID);
}

void CScorePanel::OpenPlayerMenu(int itemID)
{
	if (!m_pPlayerList ||
		!m_pPlayerMenu)
	{
		return;
	}

	KeyValues* playerData =
		m_pPlayerList->GetItemData(itemID);

	if (!playerData)
		return;

	const int clientIndex =
		playerData->GetInt("client", 0);

	if (clientIndex < 1 ||
		clientIndex > MAX_PLAYERS)
	{
		return;
	}

	hud_player_info_t& playerInfo =
		g_PlayerInfoList[clientIndex];

	if (!playerInfo.name ||
		playerInfo.name[0] == '\0')
	{
		return;
	}

	m_iSelectedClient = clientIndex;

	const bool thisPlayer =
		playerInfo.thisplayer != 0;

	const bool muted =
		GetClientVoiceMgr() &&
		GetClientVoiceMgr()->IsPlayerBlocked(
			clientIndex);

	m_pPlayerMenu->SetItemEnabled(
		m_iMuteMenuItem,
		!thisPlayer);

	m_pPlayerMenu->UpdateMenuItem(
		m_iMuteMenuItem,
		muted
		? "Unmute player"
		: "Mute player",
		new KeyValues(
			"Command",
			"command",
			"TogglePlayerMute"));

	m_pPlayerMenu->PositionRelativeToPanel(
		this,
		vgui2::Menu::CURSOR,
		0,
		true);
}

void CScorePanel::OnCommand(
	const char* command)
{
	if (command &&
		stricmp(
			command,
			"TogglePlayerMute") == 0)
	{
		ToggleSelectedPlayerMute();
		return;
	}

	BaseClass::OnCommand(command);
}

void CScorePanel::ToggleSelectedPlayerMute()
{
	if (m_iSelectedClient < 1 ||
		m_iSelectedClient > MAX_PLAYERS)
	{
		return;
	}

	hud_player_info_t& playerInfo =
		g_PlayerInfoList[m_iSelectedClient];

	// Refresh the slot in case the player disconnected or changed.
	gEngfuncs.pfnGetPlayerInfo(
		m_iSelectedClient,
		&playerInfo);

	if (!playerInfo.name ||
		playerInfo.name[0] == '\0' ||
		playerInfo.thisplayer)
	{
		return;
	}

	CVoiceStatus* voiceManager =
		GetClientVoiceMgr();

	if (!voiceManager)
		return;

	const bool currentlyMuted =
		voiceManager->IsPlayerBlocked(
			m_iSelectedClient);

	voiceManager->SetPlayerBlockedState(
		m_iSelectedClient,
		!currentlyMuted);

	m_iSelectedClient = 0;

	if (m_pPlayerMenu)
		m_pPlayerMenu->SetVisible(false);

	// Refresh immediately rather than waiting for the next half-second tick.
	UpdatePlayerList();
}
