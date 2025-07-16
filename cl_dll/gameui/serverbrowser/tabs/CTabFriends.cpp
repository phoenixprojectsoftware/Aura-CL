// Ported from Source, but modified for GoldSrc

#include <vgui/IInputInternal.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/ToggleButton.h>
#include "tier1/KeyValues.h"
#include "../ServerListSorter.h"
#include "../CServerContextMenu.h"
#include "../CServerBrowser.h"
#include "../../gameui_viewport.h"
#include "CTabFriends.h"


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CTabFriends::CTabFriends(vgui2::Panel* parent) :
	BaseClass(parent, "FriendsGames", eFriendsServer)
{
	m_iServerRefreshCount = 0;

	// Offline Mode
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
CTabFriends::~CTabFriends()
{
}


//-----------------------------------------------------------------------------
// Purpose: returns true if the game list supports the specified ui elements
//-----------------------------------------------------------------------------
bool CTabFriends::SupportsItem(InterfaceItem_e item)
{
	switch (item)
	{
	case FILTERS:
		return true;

	case GETNEWLIST:
	default:
		return false;
	}
}


//-----------------------------------------------------------------------------
// Purpose: called when the current refresh list is complete
//-----------------------------------------------------------------------------
void CTabFriends::RefreshComplete(HServerListRequest hReq, EMatchMakingServerResponse response)
{
	SetRefreshing(false);
	m_pServerList->SortList();
	m_iServerRefreshCount = 0;

	// set empty message
	m_pServerList->SetEmptyListText("#ServerBrowser_NoFriendsServers");
}

//-----------------------------------------------------------------------------
// Purpose: opens context menu (user right clicked on a server)
//-----------------------------------------------------------------------------
void CTabFriends::OnOpenContextMenu(int itemID)
{
	if (!m_pServerList->GetSelectedItemsCount())
		return;

	// get the server
	int serverID = m_pServerList->GetItemData(m_pServerList->GetSelectedItem(0))->userData;

	// Activate context menu
	CServerContextMenu* menu = CGameUIViewport::Get()->GetServerBrowser()->GetContextMenu(m_pServerList);
	menu->ShowMenu(this, serverID, true, true, true, true);
}