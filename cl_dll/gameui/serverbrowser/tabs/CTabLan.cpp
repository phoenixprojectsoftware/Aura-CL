// Ported from Source, but modified for GoldSrc

#include <vgui/IInputInternal.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/ToggleButton.h>
#include "tier1/KeyValues.h"
#include "gameui/serverbrowser/CServerContextMenu.h"
#include "gameui/serverbrowser/CServerBrowser.h"
#include "gameui/gameui_viewport.h"
#include "CTabLan.h"


const float BROADCAST_LIST_TIMEOUT = 0.4f;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CTabLan::CTabLan(vgui2::Panel* parent, bool bAutoRefresh, const char* pCustomResFilename) :
	BaseClass(parent, "LanGames", eLANServer, pCustomResFilename)
{
	m_iServerRefreshCount = 0;
	m_bRequesting = false;
	m_bAutoRefresh = bAutoRefresh;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CTabLan::~CTabLan()
{
}


//-----------------------------------------------------------------------------
// Purpose: Activates the page, starts refresh
//-----------------------------------------------------------------------------
void CTabLan::OnPageShow()
{
	if (m_bAutoRefresh)
		StartRefresh();
}


//-----------------------------------------------------------------------------
// Purpose: Called every frame
//-----------------------------------------------------------------------------
void CTabLan::OnTick()
{
	BaseClass::OnTick();
	CheckRetryRequest();
}

//-----------------------------------------------------------------------------
// Purpose: returns true if the game list supports the specified ui elements
//-----------------------------------------------------------------------------
bool CTabLan::SupportsItem(InterfaceItem_e item)
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
// Purpose: starts the servers refreshing
//-----------------------------------------------------------------------------
void CTabLan::StartRefresh()
{
	BaseClass::StartRefresh();
	m_fRequestTime = Plat_FloatTime();
}


//-----------------------------------------------------------------------------
// Purpose: Control which button are visible.
//-----------------------------------------------------------------------------
void CTabLan::ManualShowButtons(bool bShowConnect, bool bShowRefreshAll, bool bShowFilter)
{
	m_pConnect->SetVisible(bShowConnect);
	m_pRefreshAll->SetVisible(bShowRefreshAll);
	m_pFilter->SetVisible(bShowFilter);
}


//-----------------------------------------------------------------------------
// Purpose: stops current refresh/GetNewServerList()
//-----------------------------------------------------------------------------
void CTabLan::StopRefresh()
{
	BaseClass::StopRefresh();
	// clear update states
	m_bRequesting = false;
}

//-----------------------------------------------------------------------------
// Purpose: Check to see if we've finished looking for local servers
//-----------------------------------------------------------------------------
void CTabLan::CheckRetryRequest()
{
	if (!m_bRequesting)
		return;

	double curtime = Plat_FloatTime();
	if (curtime - m_fRequestTime <= BROADCAST_LIST_TIMEOUT)
	{
		return;
	}

	// time has elapsed, finish up
	m_bRequesting = false;
}

//-----------------------------------------------------------------------------
// Purpose: called when a server response has timed out, remove it
//-----------------------------------------------------------------------------
void CTabLan::ServerFailedToRespond(HServerListRequest hReq, int iServer)
{
	int iServerMap = m_mapServers.Find(iServer);
	if (iServerMap != m_mapServers.InvalidIndex())
		RemoveServer(m_mapServers[iServerMap]);
}

//-----------------------------------------------------------------------------
// Purpose: called when the current refresh list is complete
//-----------------------------------------------------------------------------
void CTabLan::RefreshComplete(HServerListRequest hReq, EMatchMakingServerResponse response)
{
	SetRefreshing(false);
	m_pServerList->SortList();
	m_iServerRefreshCount = 0;
	m_pServerList->SetEmptyListText("#ServerBrowser_NoLanServers");
	SetEmptyListText();
}

void CTabLan::SetEmptyListText()
{
	m_pServerList->SetEmptyListText("#ServerBrowser_NoLanServers");
}

//-----------------------------------------------------------------------------
// Purpose: opens context menu (user right clicked on a server)
//-----------------------------------------------------------------------------
void CTabLan::OnOpenContextMenu(int row)
{
	if (!m_pServerList->GetSelectedItemsCount())
		return;

	// get the server
	int serverID = m_pServerList->GetItemUserData(m_pServerList->GetSelectedItem(0));
	// Activate context menu
	CServerContextMenu* menu = CGameUIViewport::Get()->GetServerBrowser()->GetContextMenu(m_pServerList);
	menu->ShowMenu(this, serverID, true, true, true, false);
}