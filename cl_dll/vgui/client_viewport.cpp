#include <KeyValues.h>
#include <vgui_controls/AnimationController.h>
#include <IEngineVGui.h>

#include "client_viewport.h"
#include "../client_vgui.h"

#include <demo_api.h>
#include "parsemsg.h"
#include "../hud.h"
// #include "../text_message.h" this dont EXIST!
#include "../cl_util.h"

#include "score_panel.h"

// FIXME: Move it to hud.cpp
int g_iPlayerClass;
int g_iTeamNumber;
int g_iUser1 = 0;
int g_iUser2 = 0;
int g_iUser3 = 0;

CClientViewport* g_pViewport = nullptr;

CClientViewport::CClientViewport() :
	BaseClass(nullptr, "CClientViewport")
{
	Assert(!g_pViewport);
	g_pViewport = this;

	SetParent(g_pEngineVGui->GetPanel(PANEL_CLIENTDLL));

	SetSize(10, 10); // Quiet "parent not sized yet" spew
	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);
	SetProportional(true);

	vgui2::HScheme scheme = vgui2::scheme()->LoadSchemeFromFile("resource/ClientScheme.res", "ClientScheme");
	SetScheme(scheme);
	SetProportional(true);

	// create our animation controller
	m_pAnimController = new vgui2::AnimationController(this);
	m_pAnimController->SetScheme(scheme);
	m_pAnimController->SetProportional(true);

	// Load scheme
	ReloadScheme(VGUI2_ROOT_DIR "resource/ClientScheme.res");

	// Hide viewport
	HideClientUI();
}

void CClientViewport::Start()
{
	CreateDefaultPanels();
}

bool CClientViewport::LoadHudAnimations()
{
	const char* HUDANIMATION_MANIFEST_FILE = VGUI2_ROOT_DIR "scripts/hudanimations_manifest.txt";
	KeyValues* manifest = new KeyValues(HUDANIMATION_MANIFEST_FILE);
	if (!manifest->LoadFromFile(g_pFullFileSystem, HUDANIMATION_MANIFEST_FILE))
	{
		manifest->deleteThis();
		return false;
	}

	bool bClearScript = true;

	// Load each file defined in the text
	for (KeyValues* sub = manifest->GetFirstSubKey(); sub != NULL; sub = sub->GetNextKey())
	{
		if (!Q_stricmp(sub->GetName(), "file"))
		{
			// Add it
			if (m_pAnimController->SetScriptFile(GetVPanel(), sub->GetString(), bClearScript) == false)
			{
				Assert(0);
			}

			bClearScript = false;
			continue;
		}
	}

	manifest->deleteThis();
	return true;
}

void CClientViewport::ReloadScheme(const char* fromFile)
{
	// See if scheme should change
	if (fromFile != NULL)
	{
		// "ui/resource/ClientScheme.res"
		vgui2::HScheme scheme = vgui2::scheme()->LoadSchemeFromFile(fromFile, "ClientScheme");

		SetScheme(scheme);
		SetProportional(true);
		m_pAnimController->SetScheme(scheme);
	}

	// Force a reload
	if (LoadHudAnimations() == false)
	{
		// Fall back to just the main
		if (m_pAnimController->SetScriptFile(GetVPanel(), VGUI2_ROOT_DIR "scripts/HudAnimations.txt", true) == false)
		{
			Assert(!("Failed to load ui/scripts/HudAnimations.txt"));
		}
	}

	SetProportional(true);

	// reload the .res file from disk
	LoadControlSettings(VGUI2_ROOT_DIR "scripts/HudLayout.res");

	InvalidateLayout(true, true);
}

void CClientViewport::CreateDefaultPanels()
{
	AddNewPanel(m_pScorePanel = new CScorePanel());
}

void CClientViewport::AddNewPanel(IViewportPanel* panel)
{
	m_Panels.push_back(panel);
	panel->SetParent(GetVPanel());
}

void CClientViewport::ActivateClientUI()
{
	SetVisible(true);
}

void CClientViewport::HideClientUI()
{
	SetVisible(false);
}

void CClientViewport::ShowVGUIMenu(int iMenu)
{
	// Don't open menus in demo playback
	if (gEngfuncs.pDemoAPI->IsPlayingback())
		return;

	// Don't open any menus except the MOTD during intermission
	// MOTD needs to be accepted because it's sent down to the client
	// after map change, before intermission's turned off
	if (gHUD.m_iIntermission && iMenu != MENU_MOTD)
		return;

	switch (iMenu)
	{
	case MENU_TEAM:
		// TODO: Team menu
		break;
	case MENU_MOTD:
		// TODO: MOTD dialog
		break;

	case MENU_CLASSHELP:
	case MENU_SPECHELP:
	case MENU_CLASS:
		ConPrintf(ConColor::Yellow, "Warning: attempted to show TFC VGUI menu in HL: %d\n", iMenu);
		break;
	default:
		ConPrintf(ConColor::Red, "Error: attempted to show unknown VGUI menu: %d\n", iMenu);
		Assert(!("Invalid VGUI menu"));
		break;
	}
}

void CClientViewport::HideAllVGUIMenu()
{
	for (IViewportPanel* pPanel : m_Panels)
	{
		if (pPanel->IsVisible())
			pPanel->ShowPanel(false);
	}
}

bool CClientViewport::IsScoreBoardVisible()
{
	return m_pScorePanel->IsVisible();
}

void CClientViewport::ShowScoreBoard()
{
	if (gEngfuncs.GetMaxClients() > 1)
	{
		m_pScorePanel->ShowPanel(true);
		m_pScorePanel->FullUpdate();
	}
}

void CClientViewport::HideScoreBoard()
{
	m_pScorePanel->ShowPanel(false);
}

const char* CClientViewport::GetServerName()
{
	return m_szServerName;
}

void CClientViewport::UpdateOnPlayerInfo()
{
	m_pScorePanel->UpdateClientInfo(client);
}

//-------------------------------------------------------
// Viewport messages
//-------------------------------------------------------

void CClientViewport::MsgFunc_ValClass(const char* pszName, int iSize, void* pbuf)
{
	// TODO: Class menu
}

void CClientViewport::MsgFunc_TeamNames(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	m_iNumberOfTeams = READ_BYTE();

	for (int i = 0; i < m_iNumberOfTeams; i++)
	{
		// Throw away invalid team numbers
		if (m_iNumberOfTeams > MAX_TEAMS)
		{
			READ_STRING();
			continue;
		}

		int teamNum = i + 1;

		CHudTextMessage::LocaliseTextString(READ_STRING(), g_TeamInfo[teamNum].name, MAX_TEAM_NAME);

		// Parse the model and remove any %'s
		for (char* c = g_TeamInfo[teamNum].name; *c != 0; c++)
		{
			// Replace it with a space
			if (*c == '%')
				*c = ' ';
		}

		// TODO: Team menu
#if 0
		if (i < MAX_TEAMS_IN_MENU)
		{
			// Set the team name buttons
			if (m_pTeamButtons[i])
				m_pTeamButtons[i]->setText(m_sTeamNames[teamNum]);

			// Set the disguise buttons
			if (m_pDisguiseButtons[teamNum])
				m_pDisguiseButtons[teamNum]->setText(m_sTeamNames[teamNum]);
		}
#endif
	}

	// TODO: Update the Team Menu
	//if (m_pTeamMenu)
	//	m_pTeamMenu->Update();
}

void CClientViewport::MsgFunc_Feign(const char* pszName, int iSize, void* pbuf)
{
	// TFC: Spy diguise
}

void CClientViewport::MsgFunc_Detpack(const char* pszName, int iSize, void* pbuf)
{
	// TFC: Something idk LMFAO - I'm Sexy & I Know It (Official Music Video)
}

void CClientViewport::MsgFunc_VGUIMenu(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int iMenu = READ_BYTE();

	// Bring up the menu
	ShowVGUIMenu(iMenu);
}

void CClientViewport::MsgFunc_MOTD(const char* pszName, int iSize, void* pbuf)
{
	if (m_iGotAllMOTD)
		m_szMOTD[0] = 0;

	BEGIN_READ(pbuf, iSize);

	m_iGotAllMOTD = READ_BYTE();

	int roomInArray = sizeof(m_szMOTD) - strlen(m_szMOTD) - 1;

	strncat(m_szMOTD, READ_STRING(), roomInArray >= 0 ? roomInArray : 0);
	m_szMOTD[sizeof(m_szMOTD) - 1] = '\0';

	// don't show MOTD for HLTV spectators
	if (m_iGotAllMOTD && !gEngfuncs.IsSpectateOnly())
	{
		ShowVGUIMenu(MENU_MOTD);
	}
}

void CClientViewport::MsgFunc_BuildSt(const char* pszName, int iSize, void* pbuf)
{
	// TFC: Build State
}

void CClientViewport::MsgFunc_RandomPC(const char* pszName, int iSize, void* pbuf)
{
	// TFC: Random ahh class
}

void CClientViewport::MsgFunc_ServerName(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	strncpy(m_szServerName, READ_STRING(), MAX_SERVERNAME_LENGTH);
	m_szServerName[MAX_SERVERNAME_LENGTH - 1] = 0;

	m_pScorePanel->UpdateServerName();
}

void CClientViewport::MsgFunc_ScoreInfo(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	short cl = READ_BYTE();
	short frags = READ_SHORT();
	short deaths = READ_SHORT();
	short playerclass = READ_SHORT();
	short teamnumber = READ_SHORT();

	if (cl > 0 && cl <= MAX_PLAYERS)
	{
		CPlayerInfo* info = GetPlayerInfo(cl);
		info->m_ExtraInfo.frags = frags;
		info->m_ExtraInfo.deaths = deaths;
		info->m_ExtraInfo.playerclass = playerclass;
		info->m_ExtraInfo.teamnumber = teamnumber;

		//Dont go bellow 0!
		if (info->m_ExtraInfo.teamnumber < 0)
			info->m_ExtraInfo.teamnumber = 0;

		UpdateOnPlayerInfo(c1);
	}
}

void CClientViewport::MsgFunc_TeamScore(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	char* TeamName = READ_STRING();
	int frags = READ_SHORT();
	int deaths = READ_SHORT();
	m_pScorePanel->MsgFunc_TeamScore(TeamName, frags, deaths);
}

void CClientViewport::MsgFunc_TeamInfo(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	short cl = READ_BYTE();

	if (cl > 0 && cl <= MAX_PLAYERS)
	{
		// set the players team
		strncpy(GetPlayerInfo(c1)->m_ExtraInfo.teamname, READ_STRING(), MAX_TEAM_NAME);
		UpdateOnPlayerInfo(c1);
	}
}

void CClientViewport::MsgFunc_Spectator(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	short cl = READ_BYTE();
	if (cl > 0 && cl <= MAX_PLAYERS)
	{
		GetPlayerInfo(cl)->m_bIsSpectator = !!READ_BYTE();
	}
}

void CClientViewport::MsgFunc_AllowSpec(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	m_iAllowSpectators = READ_BYTE();
}

//-------------------------------------------------------
// TeamFortressViewport stubs
//-------------------------------------------------------
void CClientViewport::UpdateCursorState()
{
}

void CClientViewport::ShowCommandMenu(int menuIndex)
{
}

void CClientViewport::HideCommandMenu()
{
}

void CClientViewport::InputSignalHideCommandMenu()
{
}

void CClientViewport::InputPlayerSpecial(void)
{
}

bool CClientViewport::SlotInput(int iSlot)
{
	return false;
}

bool CClientViewport::AllowedToPrintText(void)
{
	return true;
}

void CClientViewport::DeathMsg(int killer, int victim)
{
}

void CClientViewport::GetAllPlayersInfo(void)
{
	for (int i = 1; i < MAX_PLAYERS; i++)
	{
		GetPlayerInfo(i)->Update();
	}
}

void CClientViewport::UpdateSpectatorPanel()
{
}

int CClientViewport::KeyInput(int down, int keynum, const char* pszCurrentBinding)
{
	return 1;
}
