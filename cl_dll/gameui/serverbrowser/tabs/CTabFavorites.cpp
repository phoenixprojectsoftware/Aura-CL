// Ported from Source, but modified for GoldSrc

#include <vgui/IInputInternal.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/ToggleButton.h>
#include "tier1/KeyValues.h"
#include "../ServerListSorter.h"
#include "../CServerContextMenu.h"
#include "../CServerBrowser.h"
#include "../dialogs/CDialogAddServer.h"
#include "../../gameui_viewport.h"
#include "CTabFavorites.h"
#include <time.h>


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CTabFavorites::CTabFavorites(vgui2::Panel* parent) :
	BaseClass(parent, "FavoriteGames", eFavoritesServer)
{
	m_bRefreshOnListReload = false;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CTabFavorites::~CTabFavorites()
{
}

//-----------------------------------------------------------------------------
// Purpose: loads favorites list from disk
//-----------------------------------------------------------------------------
void CTabFavorites::LoadFavoritesList()
{
	if (SteamMatchmaking() && SteamMatchmaking()->GetFavoriteGameCount() == 0)
	{
		// set empty message
		m_pServerList->SetEmptyListText("#ServerBrowser_NoFavoriteServers");
	}
	else
	{
		m_pServerList->SetEmptyListText("#ServerBrowser_NoInternetGamesResponded");

	}

	if (m_bRefreshOnListReload)
	{
		m_bRefreshOnListReload = false;
		StartRefresh();
	}
}


//-----------------------------------------------------------------------------
// Purpose: returns true if the game list supports the specified ui elements
//-----------------------------------------------------------------------------
bool CTabFavorites::SupportsItem(InterfaceItem_e item)
{
	switch (item)
	{
	case FILTERS:
	case ADDSERVER:
		return true;

	case ADDCURRENTSERVER:
		return SteamAPI_IsSteamRunning() ? true : false; // the fuck

	case GETNEWLIST:
	default:
		return false;
	}
}


//-----------------------------------------------------------------------------
// Purpose: called when the current refresh list is complete
//-----------------------------------------------------------------------------
void CTabFavorites::RefreshComplete(HServerListRequest hReq, EMatchMakingServerResponse response)
{
	SetRefreshing(false);
	if (SteamMatchmaking() && SteamMatchmaking()->GetFavoriteGameCount() == 0)
	{
		// set empty message
		m_pServerList->SetEmptyListText("#ServerBrowser_NoFavoriteServers");
	}
	else
	{
		m_pServerList->SetEmptyListText("#ServerBrowser_NoInternetGamesResponded");

	}
	m_pServerList->SortList();
}

//-----------------------------------------------------------------------------
// Purpose: opens context menu (user right clicked on a server)
//-----------------------------------------------------------------------------
void CTabFavorites::OnOpenContextMenu(int itemID)
{
	CServerContextMenu* menu = CGameUIViewport::Get()->GetServerBrowser()->GetContextMenu(m_pServerList);
	if (m_pServerList->GetSelectedItemsCount())
	{
		// get the server
		int serverID = m_pServerList->GetItemUserData(m_pServerList->GetSelectedItem(0));

		// Activate context menu
		menu->ShowMenu(this, serverID, true, true, true, false);
		menu->AddMenuItem("RemoveServer", "#ServerBrowser_RemoveServerFromFavorites", new KeyValues("RemoveFromFavorites"), this);
	}
	else
	{
		// no selected rows, so don't display default stuff in menu
		menu->ShowMenu(this, (uint32)-1, false, false, false, false);
	}

	menu->AddMenuItem("AddServerByName", "#ServerBrowser_AddServerByIP", new KeyValues("AddServerByName"), this);
}


//-----------------------------------------------------------------------------
// Purpose: removes a server from the favorites
//-----------------------------------------------------------------------------
void CTabFavorites::OnRemoveFromFavorites()
{
	if (!SteamMatchmakingServers() || !SteamMatchmaking())
		return;

	// iterate the selection
	for (int iGame = 0; iGame < m_pServerList->GetSelectedItemsCount(); iGame++)
	{
		int itemID = m_pServerList->GetSelectedItem(iGame);
		int serverID = m_pServerList->GetItemData(itemID)->userData;

		gameserveritem_t* pServer = SteamMatchmakingServers()->GetServerDetails(m_hRequest, serverID);

		if (pServer)
		{
			SteamMatchmaking()->RemoveFavoriteGame(pServer->m_nAppID, pServer->m_NetAdr.GetIP(), pServer->m_NetAdr.GetConnectionPort(), pServer->m_NetAdr.GetQueryPort(), k_unFavoriteFlagFavorite);
		}
	}

	UpdateStatus();
	InvalidateLayout();
	Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: Adds a server by IP address
//-----------------------------------------------------------------------------
void CTabFavorites::OnAddServerByName()
{
	// open the add server dialog
	CDialogAddServer* dlg = new CDialogAddServer(CGameUIViewport::Get()->GetServerBrowser(), this);
	dlg->MoveToCenterOfScreen();
	dlg->DoModal();
}

//-----------------------------------------------------------------------------
// Purpose: Adds the currently connected server to the list
//-----------------------------------------------------------------------------
void CTabFavorites::OnAddCurrentServer()
{
	gameserveritem_t* pConnected = CGameUIViewport::Get()->GetServerBrowser()->GetCurrentConnectedServer();
	if (pConnected && SteamMatchmaking())
	{
		SteamMatchmaking()->AddFavoriteGame(pConnected->m_nAppID, pConnected->m_NetAdr.GetIP(), pConnected->m_NetAdr.GetConnectionPort(), pConnected->m_NetAdr.GetQueryPort(), k_unFavoriteFlagFavorite, time(NULL));
		m_bRefreshOnListReload = true;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Parse posted messages
//			 
//-----------------------------------------------------------------------------
void CTabFavorites::OnCommand(const char* command)
{
	if (!Q_stricmp(command, "AddServerByName"))
	{
		OnAddServerByName();
	}
	else if (!Q_stricmp(command, "AddCurrentServer"))
	{
		OnAddCurrentServer();
	}
	else
	{
		BaseClass::OnCommand(command);
	}
}

//-----------------------------------------------------------------------------
// Purpose: enables adding server
//-----------------------------------------------------------------------------
void CTabFavorites::OnConnectToGame()
{
	m_pAddCurrentServer->SetEnabled(true);
}

//-----------------------------------------------------------------------------
// Purpose: disables adding current server
//-----------------------------------------------------------------------------
void CTabFavorites::OnDisconnectFromGame(void)
{
	m_pAddCurrentServer->SetEnabled(false);
}