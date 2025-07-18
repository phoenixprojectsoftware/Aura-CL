// Ported from Source, but modified for GoldSrc

#include <tier1/interface.h>
#include <vgui/IInputInternal.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/ToggleButton.h>
#include "tier1/KeyValues.h"
#include "../ServerListSorter.h"
#include "../CServerContextMenu.h"
#include "../CServerBrowser.h"
#include "../../gameui_viewport.h"
#include "CTabHistory.h"

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CTabHistory::CTabHistory(vgui2::Panel* parent) :
	BaseClass(parent, "HistoryGames", eHistoryServer)
{
	m_bRefreshOnListReload = false;
	m_pServerList->AddColumnHeader(9, "LastPlayed", "#ServerBrowser_LastPlayed", 100);
	m_pServerList->SetSortFunc(9, LastPlayedCompare);
	m_pServerList->SetSortColumn(9);

#ifndef _STEAMWORKS
		m_pServerList->SetEmptyListText("#ServerBrowser_OfflineMode");
		m_pConnect->SetEnabled(false);
		m_pRefreshAll->SetEnabled(false);
		m_pRefreshQuick->SetEnabled(false);
		m_pAddServer->SetEnabled(false);
		m_pFilter->SetEnabled(false);
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CTabHistory::~CTabHistory()
{
}

//-----------------------------------------------------------------------------
// Purpose: loads favorites list from disk
//-----------------------------------------------------------------------------
void CTabHistory::LoadHistoryList()
{
#ifdef _STEAMWORKS
		// set empty message
		m_pServerList->SetEmptyListText("#ServerBrowser_NoServersPlayed");
#endif

	if (m_bRefreshOnListReload)
	{
		m_bRefreshOnListReload = false;
		StartRefresh();
	}
}


//-----------------------------------------------------------------------------
// Purpose: returns true if the game list supports the specified ui elements
//-----------------------------------------------------------------------------
bool CTabHistory::SupportsItem(InterfaceItem_e item)
{
	switch (item)
	{
	case FILTERS:
		return true;

	case ADDSERVER:
	case GETNEWLIST:
	default:
		return false;
	}
}


//-----------------------------------------------------------------------------
// Purpose: called when the current refresh list is complete
//-----------------------------------------------------------------------------
void CTabHistory::RefreshComplete(HServerListRequest hReq, EMatchMakingServerResponse response)
{
	SetRefreshing(false);
	m_pServerList->SetEmptyListText("#ServerBrowser_NoServersPlayed");
	m_pServerList->SortList();
}

//-----------------------------------------------------------------------------
// Purpose: opens context menu (user right clicked on a server)
//-----------------------------------------------------------------------------
void CTabHistory::OnOpenContextMenu(int itemID)
{
	CServerContextMenu* menu = CGameUIViewport::Get()->GetServerBrowser()->GetContextMenu(m_pServerList);
	if (m_pServerList->GetSelectedItemsCount())
	{
		// get the server
		int serverID = m_pServerList->GetItemUserData(m_pServerList->GetSelectedItem(0));

		// Activate context menu
		menu->ShowMenu(this, serverID, true, true, true, true);
		menu->AddMenuItem("RemoveServer", "#ServerBrowser_RemoveServerFromHistory", new KeyValues("RemoveFromHistory"), this);
	}
	else
	{
		// no selected rows, so don't display default stuff in menu
		menu->ShowMenu(this, (uint32)-1, false, false, false, false);
	}
}


//-----------------------------------------------------------------------------
// Purpose: removes a server from the favorites
//-----------------------------------------------------------------------------
void CTabHistory::OnRemoveFromHistory()
{
	if (!SteamMatchmakingServers() || !SteamMatchmaking())
		return;

	// iterate the selection
	for (int i = m_pServerList->GetSelectedItemsCount() - 1; i >= 0; i--)
	{
		int itemID = m_pServerList->GetSelectedItem(i);
		int serverID = m_pServerList->GetItemData(itemID)->userData;

		gameserveritem_t* pServer = SteamMatchmakingServers()->GetServerDetails(m_hRequest, serverID);
		if (pServer)
			SteamMatchmaking()->RemoveFavoriteGame(pServer->m_nAppID, pServer->m_NetAdr.GetIP(), pServer->m_NetAdr.GetConnectionPort(), pServer->m_NetAdr.GetQueryPort(), k_unFavoriteFlagHistory);
	}

	UpdateStatus();
	InvalidateLayout();
	Repaint();
}