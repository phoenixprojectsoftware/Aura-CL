// Ported from Source, but modified for GoldSrc

#include <convar.h>
#include "../../hud.h"

#include <steamworks/steam_api.h>

#include <vgui/IVGui.h>
#include <vgui/ISurface.h>
#include "CServerBrowser.h"
#include "CServerContextMenu.h"
#include "dialogs/CDialogGameInfo.h"
#include "tabs/CTabInternet.h"
#include "tabs/CTabHistory.h"
#include "tabs/CTabFriends.h"
#include "tabs/CTabLan.h"
#include "tabs/CTabFavorites.h"
#include "../gameui_viewport.h"
#include "../../client_vgui.h"
#include "IBaseUI.h"

CON_COMMAND(gameui_serverbrowser, "Opens Server Browser")
{
	// Since this command is called from game menu using "engine gameui_serverbrowser"
	// GameUI will hide itself and show the game.
	// We need to show it again and after that activate CServerBrowser
	// Otherwise it may be hidden by the dev console
	CGameUIViewport::Get()->GetServerBrowser()->OpenBrowser();
	g_pBaseUI->ActivateGameUI();
}

/*
	This is a replica of Source Engine's Server Browser
	except its stripped down version, originally made for Zombie Panic!
	and now for Cross Product Multiplayer.

	This was created to fix ServerBrowser.dll limiting us to AppIDs like
	70 or 10. Yeah. Because everyone plays Half-Life. And we didn't
	pay like 80 quid for our own mod App ID, like, at all.

	Well guess what VALVe. People have figured out how to get around it.
	And they made it open source.

	Fucking mother fuckers.

	Mic drop.

	HipHopPaRappa out.
*/

CServerBrowser::CServerBrowser(vgui2::Panel* parent)
	: Frame(parent, "CServerBrowser")
{
	SetTitle("#ServerBrowser_Title", true);

	m_pSavedData = NULL;
	m_pFilterData = NULL;
	m_pFavorites = NULL;
	m_pHistory = NULL;
	m_pInternetGames = NULL;

	LoadUserData();

	SetMinimumSize(640, 384);

	m_pGameList = m_pInternetGames;

	m_pContextMenu = new CServerContextMenu(this);

	// property sheet
	m_pTabPanel = new vgui2::PropertySheet(this, "GameTabs");
	m_pTabPanel->SetTabWidth(72);
	m_pTabPanel->AddActionSignalTarget(this);

	m_pStatusLabel = new vgui2::Label(this, "StatusLabel", "");

	LoadControlSettingsAndUserConfig("servers/DialogServerBrowser.res");

	m_pStatusLabel->SetText("");

	vgui2::ivgui()->AddTickSignal(GetVPanel());
}

bool CServerBrowser::JoinGame(uint32 unGameIP, uint16 usGamePort)
{
	CDialogGameInfo* gameDialog = OpenGameInfoDialog(unGameIP, usGamePort, usGamePort);
	gameDialog->Connect();
	CloseAllGameInfoDialogs();
	return false;
}

bool CServerBrowser::JoinGame(uint64 ulSteamIDFriend)
{
	if (OpenGameInfoDialog(m_pGameList, ulSteamIDFriend))
	{
		CDialogGameInfo* pDialogGameInfo = GetDialogGameInfoForFriend(ulSteamIDFriend);
		pDialogGameInfo->Connect();
		CloseAllGameInfoDialogs();
		return true;
	}
	return false;
}

bool CServerBrowser::CanOpenGameInfoDialog(uint64 ulSteamIDFriend)
{
	// activate an already-existing dialog
	CDialogGameInfo* pDialogGameInfo = GetDialogGameInfoForFriend(ulSteamIDFriend);
	if (pDialogGameInfo)
	{
		pDialogGameInfo->Activate();
		return true;
	}

	// none yet, create a new dialog
	FriendGameInfo_t fgi;
	if (SteamFriends()->GetFriendGamePlayed(ulSteamIDFriend, &fgi))
	{
		CDialogGameInfo* pDialogGameInfo = OpenGameInfoDialog(fgi.m_unGameIP, fgi.m_usGamePort, fgi.m_usGamePort);
		pDialogGameInfo->SetFriend(ulSteamIDFriend);
		return true;
	}
	return false;
}

CDialogGameInfo* CServerBrowser::OpenGameInfoDialog(CBaseTab* gameList, uint64 serverIndex)
{
	gameserveritem_t* pServer = gameList->GetServer(serverIndex);
	if (!pServer)
		return NULL;

	CDialogGameInfo* gameDialog = new CDialogGameInfo(NULL, pServer->m_NetAdr.GetIP(), pServer->m_NetAdr.GetQueryPort(), pServer->m_NetAdr.GetConnectionPort());
	gameDialog->SetParent(GetVParent());
	gameDialog->AddActionSignalTarget(this);
	gameDialog->ShowModal(pServer->GetName());
	int i = m_GameInfoDialogs.AddToTail();
	m_GameInfoDialogs[i] = gameDialog;
	return gameDialog;
}

CDialogGameInfo* CServerBrowser::OpenGameInfoDialog(int serverIP, uint16 connPort, uint16 queryPort)
{
	CDialogGameInfo* gameDialog = new CDialogGameInfo(NULL, serverIP, queryPort, connPort);
	gameDialog->AddActionSignalTarget(this);
	gameDialog->SetParent(GetVParent());
	gameDialog->ShowModal("");
	int i = m_GameInfoDialogs.AddToTail();
	m_GameInfoDialogs[i] = gameDialog;
	return gameDialog;
}

void CServerBrowser::CloseAllGameInfoDialogs()
{
	for (int i = 0; i < m_GameInfoDialogs.Count(); i++)
	{
		vgui2::Panel* dlg = m_GameInfoDialogs[i];
		if (dlg)
		{
			vgui2::ivgui()->PostMessage(dlg->GetVPanel(), new KeyValues("Close"), NULL);
		}
	}
}

CDialogGameInfo* CServerBrowser::GetDialogGameInfoForFriend(uint64 ulSteamIDFriend)
{
	FOR_EACH_VEC(m_GameInfoDialogs, i)
	{
		CDialogGameInfo* pDlg = m_GameInfoDialogs[i];
		if (pDlg && pDlg->GetAssociatedFriend() == ulSteamIDFriend)
		{
			return pDlg;
		}
	}
	return nullptr;
}

CServerContextMenu* CServerBrowser::GetContextMenu(vgui2::Panel* pParent)
{
	// create a drop down for this object's states
	if (m_pContextMenu)
		delete m_pContextMenu;
	m_pContextMenu = new CServerContextMenu(this);
	m_pContextMenu->SetAutoDelete(false);

	if (!pParent)
		m_pContextMenu->SetParent(this);
	else
		m_pContextMenu->SetParent(pParent);

	m_pContextMenu->SetVisible(false);
	return m_pContextMenu;
}

//-----------------------------------------------------------------------------
// Purpose: Adds server to the history, saves as currently connected server
//-----------------------------------------------------------------------------
void CServerBrowser::OnConnectToGame(KeyValues* pMessageValues)
{
	int ip = pMessageValues->GetInt("ip");
	int connectionPort = pMessageValues->GetInt("connectionport");
	int queryPort = pMessageValues->GetInt("queryport");

	if (!ip || !queryPort)
		return;

	uint32 unIP = uint32(ip);

	memset(&m_CurrentConnection, 0, sizeof(gameserveritem_t));
	m_CurrentConnection.m_NetAdr.SetIP(unIP);
	m_CurrentConnection.m_NetAdr.SetQueryPort(queryPort);
	m_CurrentConnection.m_NetAdr.SetConnectionPort((unsigned short)connectionPort);

	if (m_pHistory && SteamMatchmaking())
	{
		SteamMatchmaking()->AddFavoriteGame(0, unIP, connectionPort, queryPort, k_unFavoriteFlagHistory, time(NULL));
		m_pHistory->SetRefreshOnReload();
	}

	// tell the game info dialogs, so they can cancel if we have connected
	// to a server they were auto-retrying
	for (int i = 0; i < m_GameInfoDialogs.Count(); i++)
	{
		vgui2::Panel* dlg = m_GameInfoDialogs[i];
		if (dlg)
		{
			KeyValues* kv = new KeyValues("ConnectedToGame", "ip", unIP, "connectionport", connectionPort);
			kv->SetInt("queryport", queryPort);
			vgui2::ivgui()->PostMessage(dlg->GetVPanel(), kv, NULL);
		}
	}

	// forward to favorites
	m_pFavorites->OnConnectToGame();

	m_bCurrentlyConnected = true;
}

//-----------------------------------------------------------------------------
// Purpose: Clears currently connected server
//-----------------------------------------------------------------------------
void CServerBrowser::OnDisconnectFromGame(void)
{
	m_bCurrentlyConnected = false;
	memset(&m_CurrentConnection, 0, sizeof(gameserveritem_t));

	// forward to favorites
	m_pFavorites->OnDisconnectFromGame();
}

//-----------------------------------------------------------------------------
// Purpose: Switches to specified page
//-----------------------------------------------------------------------------
void CServerBrowser::ShowServerBrowserPage(KeyValues* pMessageValues)
{
	// get the page # from message
	int iPage = pMessageValues->GetInt("page");
	if (iPage < 0 || iPage >= m_pTabPanel->GetNumPages())
	{
		Msg("Tried to activate invalid server browser tab %d\n", iPage);
		return;
	}
	// switch to that page
	m_pTabPanel->SetActivePage(m_pTabPanel->GetPage(iPage));
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CServerBrowser::OnGameListChanged()
{
	m_pGameList = dynamic_cast<CBaseTab*>(m_pTabPanel->GetActivePage());

	UpdateStatusText("");

	InvalidateLayout();
	Repaint();
}

void CServerBrowser::OpenBrowser()
{
	if (!m_pInternetGames)
	{
		m_pInternetGames = new CTabInternet(this);
		m_pFavorites = new CTabFavorites(this);
		m_pHistory = new CTabHistory(this);
		m_pLanGames = new CTabLan(this);
		m_pFriendsGames = new CTabFriends(this);

		m_pGameList = m_pInternetGames;

		m_pTabPanel->AddPage(m_pInternetGames, "#ServerBrowser_InternetTab");
		m_pTabPanel->AddPage(m_pFavorites, "#ServerBrowser_FavoritesTab");
		m_pTabPanel->AddPage(m_pHistory, "#ServerBrowser_HistoryTab");
		m_pTabPanel->AddPage(m_pLanGames, "#ServerBrowser_LanTab");
		m_pTabPanel->AddPage(m_pFriendsGames, "#ServerBrowser_FriendsTab");

		// load current tab
		const char* gameList = m_pSavedData->GetString("GameList");
		if (!Q_stricmp(gameList, "favorites"))
			m_pTabPanel->SetActivePage(m_pFavorites);
		else if (!Q_stricmp(gameList, "history"))
			m_pTabPanel->SetActivePage(m_pHistory);
		else if (!Q_stricmp(gameList, "lan"))
			m_pTabPanel->SetActivePage(m_pLanGames);
		else if (!Q_stricmp(gameList, "friends"))
			m_pTabPanel->SetActivePage(m_pFriendsGames);
		else
			m_pTabPanel->SetActivePage(m_pInternetGames);
	}

	Activate();
	m_pTabPanel->RequestFocus();
}

void CServerBrowser::OnActiveGameName(KeyValues* pKV)
{
	m_uLimitToAppID = pKV->GetUint64("appid", AURA_APPID);
	ReloadFilterSettings();
}

void CServerBrowser::ReloadFilterSettings()
{
	m_pInternetGames->LoadFilterSettings();
	m_pFavorites->LoadFilterSettings();
	m_pLanGames->LoadFilterSettings();
	m_pFriendsGames->LoadFilterSettings();
	m_pHistory->LoadFilterSettings();
}

bool CServerBrowser::GetDefaultScreenPosition(int& x, int& y, int& wide, int& tall)
{
	int wx, wy, ww, wt;
	vgui2::surface()->GetWorkspaceBounds(wx, wy, ww, wt);
	x = wx + (int)(ww * 0.05);
	y = wy + (int)(wt * 0.4);
	wide = (int)(ww * 0.5);
	tall = (int)(wt * 0.55);
	return true;
}

KeyValues* CServerBrowser::GetFilterSaveData(const char* filterSet)
{
	return m_pFilterData->FindKey(filterSet, true);
}

void CServerBrowser::OnTick()
{
	BaseClass::OnTick();
}

void CServerBrowser::LoadUserData()
{
	// free any old filters
	if (m_pSavedData)
		m_pSavedData->deleteThis();

	// Read our size
	int wide, tall;
	vgui2::surface()->GetScreenSize(wide, tall);

	// Half it
	wide -= 20;
	tall -= 20;

	m_pSavedData = new KeyValues("Filters");
	if (m_pSavedData->LoadFromFile(g_pFullFileSystem, "ServerBrowser.vdf", "CONFIG"))
	{
		// Grab our size
		SetSize(m_pSavedData->GetInt("size_wide", 640), m_pSavedData->GetInt("size_tall", 384));
		SetPos(m_pSavedData->GetInt("pos_x", 10), m_pSavedData->GetInt("pos_y", 10));
	}
	else
	{
		SetSize(wide, tall);
		SetPos(10, 10);
	}

	// If our size is larger than screen size, reset
	if (GetWide() > wide || GetTall() > tall)
	{
		SetSize(wide, tall);
		SetPos(10, 10);
	}

	KeyValues* filters = m_pSavedData->FindKey("Filters", false);
	if (filters)
	{
		m_pFilterData = filters->MakeCopy();
		m_pSavedData->RemoveSubKey(filters);
	}
	else
		m_pFilterData = new KeyValues("Filters");

	// reload all the page settings if necessary
	if (m_pHistory)
	{
		// history
		m_pHistory->LoadHistoryList();
		if (IsVisible() && m_pHistory->IsVisible())
			m_pHistory->StartRefresh();
	}

	if (m_pFavorites)
	{
		// favorites
		m_pFavorites->LoadFavoritesList();

		// filters
		ReloadFilterSettings();

		if (IsVisible() && m_pFavorites->IsVisible())
			m_pFavorites->StartRefresh();
	}

	InvalidateLayout();
	Repaint();
}

void CServerBrowser::UpdateStatusText(const char* format, ...)
{
	if (!m_pStatusLabel)
		return;

	if (format && strlen(format) > 0)
	{
		char str[1024];
		va_list argptr;
		va_start(argptr, format);
		_vsnprintf(str, sizeof(str), format, argptr);
		va_end(argptr);
		m_pStatusLabel->SetColorCodedText(str);
	}
	else
		m_pStatusLabel->SetText("");
}

void CServerBrowser::UpdateStatusText(wchar_t* unicode)
{
	if (!m_pStatusLabel) return;
	if (unicode && wcslen(unicode) > 0)
		m_pStatusLabel->SetText(unicode);
	else
		// clear
		m_pStatusLabel->SetText("");
}

void CServerBrowser::AddServerToFavorites(gameserveritem_t& server)
{
	if (SteamMatchmaking())
	{
		SteamMatchmaking()->AddFavoriteGame(
			server.m_nAppID,
			server.m_NetAdr.GetIP(),
			server.m_NetAdr.GetConnectionPort(),
			server.m_NetAdr.GetQueryPort(),
			k_unFavoriteFlagFavorite,
			time(NULL));
	}
}

gameserveritem_t* CServerBrowser::GetServer(unsigned int serverID)
{
	return nullptr;
}