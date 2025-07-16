// Ported from Source, but modified for GoldSrc

#include <vgui/IInputInternal.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/ToggleButton.h>
#include "tier1/KeyValues.h"
#include "../CServerContextMenu.h"
#include "../CServerBrowser.h"
#include "../../gameui_viewport.h"
#include "CTabInternet.h"


// How often to re-sort the server list
const float MINIMUM_SORT_TIME = 1.5f;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//			NOTE:	m_Servers can not use more than 96 sockets, else it will
//					cause internet explorer to Stop working under win98 SE!
//-----------------------------------------------------------------------------
CTabInternet::CTabInternet(vgui2::Panel* parent, const char* panelName, EPageType eType) :
	CBaseTab(parent, panelName, eType)
{
	m_fLastSort = 0.0f;
	m_bDirty = false;
	m_bRequireUpdate = true;
	m_bOfflineMode = !SteamAPI_IsSteamRunning();

	m_bAnyServersRetrievedFromMaster = false;
	m_bNoServersListedOnMaster = false;
	m_bAnyServersRespondedToQuery = false;

	m_pLocationFilter->DeleteAllItems();
	KeyValues* kv = new KeyValues("Regions");
	if (kv->LoadFromFile(g_pFullFileSystem, "servers/Regions.vdf", NULL))
	{
		// iterate the list loading all the servers
		for (KeyValues* srv = kv->GetFirstSubKey(); srv != NULL; srv = srv->GetNextKey())
		{
			struct regions_s region;

			region.name = srv->GetString("text");
			region.code = srv->GetInt("code");
			KeyValues* regionKV = new KeyValues("region", "code", region.code);
			m_pLocationFilter->AddItem(region.name.String(), regionKV);
			regionKV->deleteThis();
			m_Regions.AddToTail(region);
		}
	}
	else
	{
		Assert(!("Could not load file servers/Regions.vdf; server browser will not function."));
	}
	kv->deleteThis();

	LoadFilterSettings();
}


//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CTabInternet::~CTabInternet()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTabInternet::PerformLayout()
{
	if (!m_bOfflineMode && m_bRequireUpdate && CGameUIViewport::Get()->GetServerBrowser()->IsVisible())
	{
		PostMessage(this, new KeyValues("GetNewServerList"), 0.1f);
		m_bRequireUpdate = false;
	}

	if (m_bOfflineMode)
	{
		m_pServerList->SetEmptyListText("#ServerBrowser_OfflineMode");
		m_pConnect->SetEnabled(false);
		m_pRefreshAll->SetEnabled(false);
		m_pRefreshQuick->SetEnabled(false);
		m_pAddServer->SetEnabled(false);
		m_pFilter->SetEnabled(false);
	}

	BaseClass::PerformLayout();
	m_pLocationFilter->SetEnabled(true);
}

//-----------------------------------------------------------------------------
// Purpose: Activates the page, starts refresh if needed
//-----------------------------------------------------------------------------
void CTabInternet::OnPageShow()
{
	if (m_pServerList->GetItemCount() == 0 && CGameUIViewport::Get()->GetServerBrowser()->IsVisible())
		BaseClass::OnPageShow();
	// the "internet games" tab (unlike the other browser tabs)
	// does not automatically start a query when the user
	// navigates to this tab unless they have no servers listed.
}


//-----------------------------------------------------------------------------
// Purpose: Called every frame, maintains sockets and runs refreshes
//-----------------------------------------------------------------------------
void CTabInternet::OnTick()
{
	if (m_bOfflineMode)
	{
		BaseClass::OnTick();
		return;
	}

	BaseClass::OnTick();

	CheckRedoSort();
}


//-----------------------------------------------------------------------------
// Purpose: Handles incoming server refresh data
//			updates the server browser with the refreshed information from the server itself
//-----------------------------------------------------------------------------
void CTabInternet::ServerResponded(HServerListRequest hReq, int iServer)
{
	m_bDirty = true;
	BaseClass::ServerResponded(hReq, iServer);
	m_bAnyServersRespondedToQuery = true;
	m_bAnyServersRetrievedFromMaster = true;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTabInternet::ServerFailedToRespond(HServerListRequest hReq, int iServer)
{
	m_bDirty = true;
	gameserveritem_t* pServer = SteamMatchmakingServers()->GetServerDetails(hReq, iServer);
	Assert(pServer);

	if (pServer->m_bHadSuccessfulResponse)
	{
		// if it's had a successful response in the past, leave it on
		ServerResponded(hReq, iServer);
	}
	else
	{
		int iServerMap = m_mapServers.Find(iServer);
		if (iServerMap != m_mapServers.InvalidIndex())
			RemoveServer(m_mapServers[iServerMap]);
		// we've never had a good response from this server, remove it from the list
		m_iServerRefreshCount++;
	}
}


//-----------------------------------------------------------------------------
// Purpose: Called when server refresh has been completed
//-----------------------------------------------------------------------------
void CTabInternet::RefreshComplete(HServerListRequest hReq, EMatchMakingServerResponse response)
{
	SetRefreshing(false);
	UpdateFilterSettings();

	if (response != eServerFailedToRespond)
	{
		if (m_bAnyServersRespondedToQuery)
		{
			m_pServerList->SetEmptyListText(GetStringNoUnfilteredServers());
		}
		else if (response == eNoServersListedOnMasterServer)
		{
			m_pServerList->SetEmptyListText(GetStringNoUnfilteredServersOnMaster());
		}
		else
		{
			m_pServerList->SetEmptyListText(GetStringNoServersResponded());
		}
	}
	else
	{
		m_pServerList->SetEmptyListText("#ServerBrowser_MasterServerNotResponsive");
	}

	// perform last sort
	m_bDirty = false;
	m_fLastSort = Plat_FloatTime();
	if (IsVisible())
	{
		m_pServerList->SortList();
	}

	UpdateStatus();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTabInternet::GetNewServerList()
{
	BaseClass::GetNewServerList();
	UpdateStatus();

	m_bRequireUpdate = false;
	m_bAnyServersRetrievedFromMaster = false;
	m_bAnyServersRespondedToQuery = false;

	m_pServerList->DeleteAllItems();
}


//-----------------------------------------------------------------------------
// Purpose: returns true if the game list supports the specified ui elements
//-----------------------------------------------------------------------------
bool CTabInternet::SupportsItem(BaseClass::InterfaceItem_e item)
{
#if defined( CONTAGION )
	if (m_eMatchMakingType == eLobbyServers)
		return false;
#endif

	switch (item)
	{
	case FILTERS:
	case GETNEWLIST:
		return true;

	default:
		return false;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTabInternet::CheckRedoSort(void)
{
	float fCurTime;

	// No changes detected
	if (!m_bDirty)
		return;

	fCurTime = Plat_FloatTime();
	// Not time yet
	if (fCurTime - m_fLastSort < MINIMUM_SORT_TIME)
		return;

	// postpone sort if mouse button is down
	if (vgui2::input()->IsMouseDown(vgui2::MOUSE_LEFT) || vgui2::input()->IsMouseDown(vgui2::MOUSE_RIGHT))
	{
		// don't sort for at least another second
		m_fLastSort = fCurTime - MINIMUM_SORT_TIME + 1.0f;
		return;
	}

	// Reset timer
	m_bDirty = false;
	m_fLastSort = fCurTime;

	// Force sort to occur now!
	m_pServerList->SortList();
}


//-----------------------------------------------------------------------------
// Purpose: opens context menu (user right clicked on a server)
//-----------------------------------------------------------------------------
void CTabInternet::OnOpenContextMenu(int itemID)
{
	if (!m_pServerList->GetSelectedItemsCount())
		return;

	// get the server
	int serverID = m_pServerList->GetItemData(m_pServerList->GetSelectedItem(0))->userData;

	// Activate context menu
	CServerContextMenu* menu = CGameUIViewport::Get()->GetServerBrowser()->GetContextMenu(m_pServerList);
	menu->ShowMenu(this, serverID, true, true, true, true);
}

//-----------------------------------------------------------------------------
// Purpose: refreshes a single server
//-----------------------------------------------------------------------------
void CTabInternet::OnRefreshServer(int serverID)
{
	BaseClass::OnRefreshServer(serverID);

	CGameUIViewport::Get()->GetServerBrowser()->UpdateStatusText("#ServerBrowser_GettingNewServerList");
}


//-----------------------------------------------------------------------------
// Purpose: get the region code selected in the ui
// Output: returns the region code the user wants to filter by
//-----------------------------------------------------------------------------
int CTabInternet::GetRegionCodeToFilter()
{
	KeyValues* kv = m_pLocationFilter->GetActiveItemUserData();
	if (kv)
		return kv->GetInt("code");
	else
		return 255;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTabInternet::CheckTagFilter(gameserveritem_t& server)
{
#if defined( CONTAGION )
	// No Lobby Games
	if (HasGameTag(server.m_szGameTags, "lobbygame"))
		return false;

	// No Solo Games
	if (FilterFind(server.GetName(), "Solo Game"))
		return false;
#endif

	bool bRetVal = true;

	// Servers without tags go in the official games, servers with tags go in custom games
	bool bOfficialServer = !(server.m_szGameTags && server.m_szGameTags[0]);
	if (bOfficialServer)
		return true;

	return bRetVal;
}