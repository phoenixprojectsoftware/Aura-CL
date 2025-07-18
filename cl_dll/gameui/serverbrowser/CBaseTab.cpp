// Ported from Source, but modified for GoldSrc

#include <vgui/IVGui.h>
#include <vgui/ILocalize.h>
#include <vgui_controls/PropertySheet.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/ImageList.h>
#include <vgui_controls/ToggleButton.h>
#include <vgui_controls/CheckButton.h>
#include <vgui_controls/ComboBox.h>
#include "ServerListSorter.h"
#include "CBaseTab.h"
#include "CServerBrowser.h"
#include "../gameui_viewport.h"
#include <time.h>


CServerListPanel::CServerListPanel(CBaseTab* pOuter, const char* pName) :
	BaseClass(pOuter, pName)
{
	m_pOuter = pOuter;
}


void CServerListPanel::OnKeyCodeTyped(vgui2::KeyCode code)
{
	// Let the outer class handle it.
	if (code == vgui2::KEY_ENTER && m_pOuter->OnGameListEnterPressed())
		return;

	BaseClass::OnKeyCodeTyped(code);
}


#define FILTER_ALLSERVERS          0
#define FILTER_SECURESERVERSONLY   1
#define FILTER_INSECURESERVERSONLY 2


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CBaseTab::CBaseTab(vgui2::Panel* parent, const char* name, EPageType eType, const char* pCustomResFilename)
	: PropertyPage(parent, name), m_pCustomResFilename(pCustomResFilename),
	m_hRequest(NULL)
{
	SetSize(624, 278);
	m_szMapFilter[0] = 0;
	m_iPingFilter = 0;
	// Default to Zombie Panic! AppID if this just shits itself.
	m_uLimitToAppID = SteamUtils() ? SteamUtils()->GetAppID() : AURA_APPID; // no magic number for AppID please. thanks.
	m_iServerRefreshCount = 0;
	m_bFilterNoFullServers = false;
	m_bFilterNoEmptyServers = false;
	m_bFilterNoPasswordedServers = false;
	m_bFilterHasAssociatedSteamAccount = false;
	m_iSecureFilter = FILTER_ALLSERVERS;
	m_hFont = NULL;
	m_eMatchMakingType = eType;
	SetDefLessFunc(m_mapServers);
	SetDefLessFunc(m_mapServerIP);
	SetDefLessFunc(m_mapGamesFilterItem);

	m_SteamCallResultOnFavoritesMsg.Set(0, this, &CBaseTab::OnFavoritesMsg);

	// get the 'all' text
	wchar_t* all = g_pVGuiLocalize->Find("ServerBrowser_All");
	Q_UnicodeToUTF8(all, m_szComboAllText, sizeof(m_szComboAllText));

	// Init UI
	m_pConnect = new vgui2::Button(this, "ConnectButton", "#ServerBrowser_Connect");
	m_pConnect->SetEnabled(false);
	m_pRefreshAll = new vgui2::Button(this, "RefreshButton", "#ServerBrowser_Refresh");
	m_pRefreshQuick = new vgui2::Button(this, "RefreshQuickButton", "#ServerBrowser_RefreshQuick");
	m_pAddServer = new vgui2::Button(this, "AddServerButton", "#ServerBrowser_AddServer");
	m_pAddCurrentServer = new vgui2::Button(this, "AddCurrentServerButton", "#ServerBrowser_AddCurrentServer");
	m_pServerList = new CServerListPanel(this, "gamelist");
	m_pServerList->SetAllowUserModificationOfColumns(true);

	m_pAddToFavoritesButton = new vgui2::Button(this, "AddToFavoritesButton", "");
	m_pAddToFavoritesButton->SetEnabled(false);
	m_pAddToFavoritesButton->SetVisible(false);

	// Add the column headers
	m_pServerList->AddColumnHeader(0, "Password", "#ServerBrowser_Password", 16, vgui2::ListPanel::COLUMN_FIXEDSIZE | vgui2::ListPanel::COLUMN_IMAGE);
	m_pServerList->AddColumnHeader(1, "Bots", "#ServerBrowser_Bots", 16, vgui2::ListPanel::COLUMN_FIXEDSIZE | vgui2::ListPanel::COLUMN_HIDDEN);
	m_pServerList->AddColumnHeader(2, "Secure", "#ServerBrowser_Secure", 16, vgui2::ListPanel::COLUMN_FIXEDSIZE | vgui2::ListPanel::COLUMN_IMAGE);
	m_pServerList->AddColumnHeader(3, "Name", "#ServerBrowser_Servers", 100, vgui2::ListPanel::COLUMN_RESIZEWITHWINDOW | vgui2::ListPanel::COLUMN_UNHIDABLE);
	m_pServerList->AddColumnHeader(4, "IPAddr", "#ServerBrowser_IPAddress", 64, vgui2::ListPanel::COLUMN_HIDDEN);
	m_pServerList->AddColumnHeader(5, "GameDesc", "#ServerBrowser_Version", 200,
		112,	// minwidth
		300,	// maxwidth
		0		// flags
	);
	m_pServerList->AddColumnHeader(6, "Players", "#ServerBrowser_Players", 85, vgui2::ListPanel::COLUMN_FIXEDSIZE);
	m_pServerList->AddColumnHeader(7, "Map", "#ServerBrowser_Map", 90,
		90,		// minwidth
		300,	// maxwidth
		0		// flags
	);
	m_pServerList->AddColumnHeader(8, "Ping", "#ServerBrowser_Latency", 90, vgui2::ListPanel::COLUMN_FIXEDSIZE);

	m_pServerList->SetColumnHeaderTooltip(0, "#ServerBrowser_PasswordColumn_Tooltip");
	m_pServerList->SetColumnHeaderTooltip(1, "#ServerBrowser_BotColumn_Tooltip");
	m_pServerList->SetColumnHeaderTooltip(2, "#ServerBrowser_SecureColumn_Tooltip");

	// setup fast sort functions
	m_pServerList->SetSortFunc(0, PasswordCompare);
	m_pServerList->SetSortFunc(1, BotsCompare);
	m_pServerList->SetSortFunc(2, SecureCompare);
	m_pServerList->SetSortFunc(3, ServerNameCompare);
	m_pServerList->SetSortFunc(4, IPAddressCompare);
	m_pServerList->SetSortFunc(5, GameCompare);
	m_pServerList->SetSortFunc(6, PlayersCompare);
	m_pServerList->SetSortFunc(7, MapCompare);
	m_pServerList->SetSortFunc(8, PingCompare);

	// Sort by ping time by default
	m_pServerList->SetSortColumn(8);

	CreateFilters();
	LoadFilterSettings();

	m_bAutoSelectFirstItemInGameList = false;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CBaseTab::~CBaseTab()
{
	if (m_hRequest)
	{
		SteamMatchmakingServers()->ReleaseRequest(m_hRequest);
		m_hRequest = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBaseTab::GetInvalidServerListID()
{
	return m_pServerList->InvalidItemID();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseTab::PerformLayout()
{
	BaseClass::PerformLayout();

	if (m_pServerList->GetSelectedItemsCount() < 1)
	{
		m_pConnect->SetEnabled(false);
	}
	else
	{
		m_pConnect->SetEnabled(true);
	}


	if (SupportsItem(CBaseTab::GETNEWLIST))
	{
		m_pRefreshQuick->SetVisible(true);
		m_pRefreshAll->SetText("#ServerBrowser_RefreshAll");
	}
	else
	{
		m_pRefreshQuick->SetVisible(false);
		m_pRefreshAll->SetText("#ServerBrowser_Refresh");
	}

	if (SupportsItem(CBaseTab::ADDSERVER))
	{
		m_pFilterString->SetWide(90); // shrink the filter label to fix the add current server button
		m_pAddServer->SetVisible(true);
	}
	else
	{
		m_pAddServer->SetVisible(false);
	}

	if (SupportsItem(CBaseTab::ADDCURRENTSERVER))
	{
		m_pAddCurrentServer->SetVisible(true);
	}
	else
	{
		m_pAddCurrentServer->SetVisible(false);
	}

	if (IsRefreshing())
	{
		m_pRefreshAll->SetText("#ServerBrowser_StopRefreshingList");
	}

	if (m_pServerList->GetItemCount() > 0)
	{
		m_pRefreshQuick->SetEnabled(true);
	}
	else
	{
		m_pRefreshQuick->SetEnabled(false);
	}

	if (!SteamMatchmakingServers() || !SteamMatchmaking())
	{
		m_pAddCurrentServer->SetVisible(false);
		m_pRefreshQuick->SetEnabled(false);
		m_pAddServer->SetEnabled(false);
		m_pConnect->SetEnabled(false);
		m_pRefreshAll->SetEnabled(false);
		m_pAddToFavoritesButton->SetEnabled(false);
		m_pServerList->SetEmptyListText("#ServerBrowser_SteamRunning");
	}

	Repaint();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseTab::ApplySchemeSettings(vgui2::IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	OnButtonToggled(m_pFilter, false);

	// load the password icon
	vgui2::ImageList* imageList = new vgui2::ImageList(false);
	imageList->AddImage(vgui2::scheme()->GetImage("servers/icon_password", false));
	imageList->AddImage(vgui2::scheme()->GetImage("servers/icon_bots", false));
	imageList->AddImage(vgui2::scheme()->GetImage("servers/icon_robotron", false));
	imageList->AddImage(vgui2::scheme()->GetImage("servers/icon_secure_deny", false));

	int passwordColumnImage = imageList->AddImage(vgui2::scheme()->GetImage("servers/icon_password_column", false));
	int botColumnImage = imageList->AddImage(vgui2::scheme()->GetImage("servers/icon_bots_column", false));
	int secureColumnImage = imageList->AddImage(vgui2::scheme()->GetImage("servers/icon_robotron_column", false));

	m_pServerList->SetImageList(imageList, true);
	m_hFont = pScheme->GetFont("ListSmall", IsProportional());
	if (!m_hFont)
		m_hFont = pScheme->GetFont("DefaultSmall", IsProportional());

	m_pServerList->SetFont(m_hFont);

	m_pServerList->SetColumnHeaderImage(0, passwordColumnImage);
	m_pServerList->SetColumnHeaderImage(1, botColumnImage);
	m_pServerList->SetColumnHeaderImage(2, secureColumnImage);
}

//-----------------------------------------------------------------------------
// Purpose: gets information about specified server
//-----------------------------------------------------------------------------
gameserveritem_t* CBaseTab::GetServer(unsigned int serverID)
{
	if (!SteamMatchmakingServers())
		return NULL;

	if (serverID >= 0)
	{
		return SteamMatchmakingServers()->GetServerDetails(m_hRequest, serverID);
	}
	else
	{
		Assert(!"Unable to return a useful entry");
		return NULL; // bugbug Alfred: temp Favorites/History objects won't return a good value here...
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseTab::CreateFilters()
{
	m_bFiltersVisible = false;
	m_pFilter = new vgui2::ToggleButton(this, "Filter", "#ServerBrowser_Filter");
	m_pFilterString = new vgui2::Label(this, "FilterString", "");

	// filter controls
	m_pLocationFilter = new vgui2::ComboBox(this, "LocationFilter", 6, false);
	m_pLocationFilter->AddItem("", NULL);

	m_pMapFilter = new vgui2::TextEntry(this, "MapFilter");
	m_pPingFilter = new vgui2::ComboBox(this, "PingFilter", 6, false);
	m_pPingFilter->AddItem("#ServerBrowser_All", NULL);
	m_pPingFilter->AddItem("#ServerBrowser_LessThan50", NULL);
	m_pPingFilter->AddItem("#ServerBrowser_LessThan100", NULL);
	m_pPingFilter->AddItem("#ServerBrowser_LessThan150", NULL);
	m_pPingFilter->AddItem("#ServerBrowser_LessThan250", NULL);
	m_pPingFilter->AddItem("#ServerBrowser_LessThan350", NULL);
	m_pPingFilter->AddItem("#ServerBrowser_LessThan600", NULL);

	m_pSecureFilter = new vgui2::ComboBox(this, "SecureFilter", 3, false);
	m_pSecureFilter->AddItem("#ServerBrowser_All", NULL);
	m_pSecureFilter->AddItem("#ServerBrowser_SecureOnly", NULL);
	m_pSecureFilter->AddItem("#ServerBrowser_InsecureOnly", NULL);

	m_pNoEmptyServersFilterCheck = new vgui2::CheckButton(this, "ServerEmptyFilterCheck", "");
	m_pNoFullServersFilterCheck = new vgui2::CheckButton(this, "ServerFullFilterCheck", "");
	m_pNoPasswordFilterCheck = new vgui2::CheckButton(this, "NoPasswordFilterCheck", "");
	m_pValidSteamAccountFilterCheck = new vgui2::CheckButton(this, "ValidSteamAccountFilterCheck", "");
}


//-----------------------------------------------------------------------------
// Purpose: loads filter settings from the keyvalues
//-----------------------------------------------------------------------------
void CBaseTab::LoadFilterSettings()
{
	KeyValues* filter = CGameUIViewport::Get()->GetServerBrowser()->GetFilterSaveData(GetName());

	Q_strncpy(m_szMapFilter, filter->GetString("map"), sizeof(m_szMapFilter));
	m_iPingFilter = filter->GetInt("ping");
	m_bFilterNoFullServers = filter->GetInt("NoFull");
	m_bFilterNoEmptyServers = filter->GetInt("NoEmpty");
	m_bFilterNoPasswordedServers = filter->GetInt("NoPassword");
	m_bFilterHasAssociatedSteamAccount = filter->GetInt("HasAssociatedSteamAccount");

	int secureFilter = filter->GetInt("Secure");
	m_pSecureFilter->ActivateItem(secureFilter);

	// apply to the controls
	m_pMapFilter->SetText(m_szMapFilter);
	m_pLocationFilter->ActivateItem(filter->GetInt("location"));

	if (m_iPingFilter)
	{
		char buf[32];
		Q_snprintf(buf, sizeof(buf), "< %d", m_iPingFilter);
		m_pPingFilter->SetText(buf);
	}

	m_pNoFullServersFilterCheck->SetSelected(m_bFilterNoFullServers);
	m_pNoEmptyServersFilterCheck->SetSelected(m_bFilterNoEmptyServers);
	m_pNoPasswordFilterCheck->SetSelected(m_bFilterNoPasswordedServers);
	m_pValidSteamAccountFilterCheck->SetSelected(m_bFilterHasAssociatedSteamAccount);

	OnLoadFilter(filter);
	UpdateFilterSettings();
}

void CBaseTab::OnThink()
{
	BaseClass::OnThink();
}


//-----------------------------------------------------------------------------
// Purpose: Handles incoming server refresh data
//			updates the server browser with the refreshed information from the server itself
//-----------------------------------------------------------------------------
void CBaseTab::ServerResponded(gameserveritem_t& server)
{
	int nIndex = -1; // start at -1 and work backwards to find the next free slot for this adhoc query
	while (m_mapServers.Find(nIndex) != m_mapServers.InvalidIndex())
		nIndex--;
	ServerResponded(nIndex, &server);
}


//-----------------------------------------------------------------------------
// Purpose: Callback for ISteamMatchmakingServerListResponse
//-----------------------------------------------------------------------------
void CBaseTab::ServerResponded(HServerListRequest hReq, int iServer)
{
	gameserveritem_t* pServerItem = SteamMatchmakingServers()->GetServerDetails(hReq, iServer);
	if (!pServerItem)
	{
		Assert(!"Missing server response");
		return;
	}
	ServerResponded(iServer, pServerItem);
}


//-----------------------------------------------------------------------------
// Purpose: Handles incoming server refresh data
//			updates the server browser with the refreshed information from the server itself
//-----------------------------------------------------------------------------
void CBaseTab::ServerResponded(int iServer, gameserveritem_t* pServerItem)
{
	int iServerMap = m_mapServers.Find(iServer);
	if (iServerMap == m_mapServers.InvalidIndex())
	{
		servernetadr_t netAdr(pServerItem->m_NetAdr);
		int iServerIP = m_mapServerIP.Find(netAdr);
		if (iServerIP != m_mapServerIP.InvalidIndex())
		{
			// if we already had this entry under another index remove the old entry
			int iServerMap = m_mapServers.Find(m_mapServerIP[iServerIP]);
			if (iServerMap != m_mapServers.InvalidIndex())
			{
				ServerData_t& server = m_mapServers[iServerMap];
				if (m_pServerList->IsValidItemID(server.m_iListID))
					m_pServerList->RemoveItem(server.m_iListID);
				m_mapServers.RemoveAt(iServerMap);
			}
			m_mapServerIP.RemoveAt(iServerIP);
		}

		ServerData_t serverFind;
		serverFind.m_iListID = -1;
		serverFind.m_bDoNotRefresh = false;
		iServerMap = m_mapServers.Insert(iServer, serverFind);
		m_mapServerIP.Insert(netAdr, iServer);
	}

	ServerData_t* pServer = &m_mapServers[iServerMap];
	pServer->m_iServerID = iServer;
	Assert(pServerItem->m_NetAdr.GetIP() != 0);

	// check filters
	bool removeItem = false;
	if (!CheckPrimaryFilters(*pServerItem))
	{
		// server has been filtered at a primary level
		// remove from lists
		pServer->m_bDoNotRefresh = true;

		// remove from UI list
		removeItem = true;
	}
	else if (!CheckSecondaryFilters(*pServerItem))
	{
		// we still ping this server in the future; however it is removed from UI list
		removeItem = true;
	}

	if (removeItem)
	{
		if (m_pServerList->IsValidItemID(pServer->m_iListID))
		{
			m_pServerList->RemoveItem(pServer->m_iListID);
			pServer->m_iListID = GetInvalidServerListID();
		}
		return;
	}

	// update UI
	KeyValues* kv;
	if (m_pServerList->IsValidItemID(pServer->m_iListID))
	{
		// we're updating an existing entry
		kv = m_pServerList->GetItem(pServer->m_iListID);
		m_pServerList->SetUserData(pServer->m_iListID, pServer->m_iServerID);
	}
	else
	{
		// new entry
		kv = new KeyValues("Server");
	}

	kv->SetString("name", pServerItem->GetName());
	kv->SetString("map", pServerItem->m_szMap);
	kv->SetString("GameDir", pServerItem->m_szGameDir);
	kv->SetString("GameDesc", pServerItem->m_szGameDescription);
	kv->SetInt("password", pServerItem->m_bPassword ? 1 : 0);

	if (pServerItem->m_nBotPlayers > 0)
		kv->SetInt("bots", pServerItem->m_nBotPlayers);
	else
		kv->SetString("bots", "");

	if (pServerItem->m_bSecure)
	{
		// show the denied icon if banned from secure servers, the secure icon otherwise
		// kv->SetInt("secure", CGameUIViewport::Get()->IsVACBanned() ? 4 : 3);
	}
	else
	{
		kv->SetInt("secure", 0);
	}

	kv->SetString("IPAddr", pServerItem->m_NetAdr.GetConnectionAddressString());

	int nAdjustedForBotsPlayers = max(0, pServerItem->m_nPlayers);
	int nAdjustedForBotsMaxPlayers = max(0, pServerItem->m_nMaxPlayers);

	char buf[32];
	Q_snprintf(buf, sizeof(buf), "%d / %d", nAdjustedForBotsPlayers, nAdjustedForBotsMaxPlayers);
	kv->SetString("Players", buf);

	kv->SetInt("Ping", pServerItem->m_nPing);

	kv->SetString("Tags", pServerItem->m_szGameTags);

	if (pServerItem->m_ulTimeLastPlayed)
	{
		// construct a time string for last played time
		struct tm* now;
		now = localtime((time_t*)&pServerItem->m_ulTimeLastPlayed);

		if (now)
		{
			char buf[64];
			strftime(buf, sizeof(buf), "%a %d %b %I:%M%p", now);
			Q_strlower(buf + strlen(buf) - 4);
			kv->SetString("LastPlayed", buf);
		}
	}

	if (pServer->m_bDoNotRefresh)
	{
		// clear out the vars
		kv->SetString("Ping", "");
		kv->SetWString("GameDesc", g_pVGuiLocalize->Find("#ServerBrowser_NotResponding"));
		kv->SetString("Players", "");
		kv->SetString("map", "");
	}

	kv->SetInt("server_ip", pServerItem->m_NetAdr.GetIP());
	kv->SetInt("server_qport", pServerItem->m_NetAdr.GetQueryPort());
	kv->SetInt("server_port", pServerItem->m_NetAdr.GetConnectionPort());
	kv->SetInt("server_id", iServer);

	if (!m_pServerList->IsValidItemID(pServer->m_iListID))
	{
		// new server, add to list
		pServer->m_iListID = m_pServerList->AddItem(kv, pServer->m_iServerID, false, false);
		if (m_bAutoSelectFirstItemInGameList && m_pServerList->GetItemCount() == 1)
		{
			m_pServerList->AddSelectedItem(pServer->m_iListID);
		}

		kv->deleteThis();
	}
	else
	{
		// tell the list that we've changed the data
		m_pServerList->ApplyItemChanges(pServer->m_iListID);
		m_pServerList->SetItemVisible(pServer->m_iListID, true);
	}

	UpdateStatus();
	m_iServerRefreshCount++;
}

void CBaseTab::UpdateServerPlayerCount(gameserveritem_t& server)
{
	for (int i = 0; i < m_pServerList->GetItemCount(); i++)
	{
		KeyValues* kvItem = m_pServerList->GetItem(i);
		if ((uint32)kvItem->GetInt("server_ip") == server.m_NetAdr.GetIP() &&
			(uint16)kvItem->GetInt("server_qport") == server.m_NetAdr.GetQueryPort() &&
			(uint16)kvItem->GetInt("server_port") == server.m_NetAdr.GetConnectionPort())
		{
			int nAdjustedForBotsPlayers = max(0, server.m_nPlayers);
			int nAdjustedForBotsMaxPlayers = max(0, server.m_nMaxPlayers);

			char buf[32];
			Q_snprintf(buf, sizeof(buf), "%d / %d", nAdjustedForBotsPlayers, nAdjustedForBotsMaxPlayers);
			kvItem->SetString("Players", buf);
			return;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Handles filter dropdown being toggled
//-----------------------------------------------------------------------------
void CBaseTab::OnButtonToggled(Panel* panel, int state)
{
	if (panel == m_pFilter)
	{
		int wide, tall;
		GetSize(wide, tall);
		SetSize(624, 278);

		if (m_pCustomResFilename)
		{
			m_bFiltersVisible = false;
		}
		else
		{
			if (m_pFilter->IsSelected())
			{
				// drop down
				m_bFiltersVisible = true;
			}
			else
			{
				// hide filter area
				m_bFiltersVisible = false;
			}
		}

		UpdateDerivedLayouts();
		m_pFilter->SetSelected(m_bFiltersVisible);

		if (m_hFont)
		{
			SETUP_PANEL(m_pServerList);
			m_pServerList->SetFont(m_hFont);
		}

		SetSize(wide, tall);

		InvalidateLayout();
	}
	else if (panel == m_pNoFullServersFilterCheck || panel == m_pNoEmptyServersFilterCheck || panel == m_pNoPasswordFilterCheck || panel == m_pValidSteamAccountFilterCheck)
	{
		// treat changing these buttons like any other filter has changed
		OnTextChanged(panel, "");
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseTab::UpdateDerivedLayouts(void)
{
	char rgchControlSettings[MAX_PATH];
	if (m_pCustomResFilename)
	{
		Q_snprintf(rgchControlSettings, sizeof(rgchControlSettings), "%s", m_pCustomResFilename);
	}
	else
	{
		if (m_pFilter->IsSelected())
		{
			// drop down
			Q_snprintf(rgchControlSettings, sizeof(rgchControlSettings), "servers/%sPage_Filters.res", "InternetGames");
		}
		else
		{
			// hide filter area
			Q_snprintf(rgchControlSettings, sizeof(rgchControlSettings), "servers/%sPage.res", "InternetGames");
		}
	}

	LoadControlSettings(rgchControlSettings);
}

//-----------------------------------------------------------------------------
// Purpose: Called when the game dir combo box is changed
//-----------------------------------------------------------------------------
void CBaseTab::OnTextChanged(Panel* panel, const char* text)
{
	if (!Q_stricmp(text, m_szComboAllText))
	{
		vgui2::ComboBox* box = dynamic_cast<vgui2::ComboBox*>(panel);
		if (box)
		{
			box->SetText("");
			text = "";
		}
	}

	// get filter settings from controls
	UpdateFilterSettings();

	// apply settings
	ApplyGameFilters();

	if (m_bFiltersVisible && (panel == m_pLocationFilter) && CGameUIViewport::Get()->GetServerBrowser()->IsVisible())
	{
		// if they changed games and/or region then cancel the refresh because the old list they are getting
		// will be for the wrong game, so stop and start a refresh
		StopRefresh();
		GetNewServerList();
	}
}

//-----------------------------------------------------------------------------
// Purpose: applies only the game filter to the current list
//-----------------------------------------------------------------------------
void CBaseTab::ApplyGameFilters()
{
	if (!SteamMatchmakingServers())
		return;
	// loop through all the servers checking filters
	FOR_EACH_MAP_FAST(m_mapServers, i)
	{
		ServerData_t& server = m_mapServers[i];
		gameserveritem_t* pServer = SteamMatchmakingServers()->GetServerDetails(m_hRequest, server.m_iServerID);
		if (!pServer)
			continue;

		if (!CheckPrimaryFilters(*pServer) || !CheckSecondaryFilters(*pServer))
		{
			// server failed filtering, remove it
			server.m_bDoNotRefresh = true;
			if (m_pServerList->IsValidItemID(server.m_iListID))
			{
				// don't remove the server from list, just hide since this is a lot faster
				m_pServerList->SetItemVisible(server.m_iListID, false);
			}
		}
		else if (BShowServer(server))
		{
			// server passed filters, so it can be refreshed again
			server.m_bDoNotRefresh = false;
			gameserveritem_t* pServer = SteamMatchmakingServers()->GetServerDetails(m_hRequest, server.m_iServerID);

			// re-add item to list
			if (!m_pServerList->IsValidItemID(server.m_iListID))
			{
				KeyValues* kv = new KeyValues("Server");
				kv->SetString("name", pServer->GetName());
				kv->SetString("map", pServer->m_szMap);
				kv->SetString("GameDir", pServer->m_szGameDir);
				kv->SetString("GameTags", pServer->m_szGameTags);
				if (pServer->m_szGameDescription[0])
				{
					kv->SetString("GameDesc", pServer->m_szGameDescription);
				}
				else
				{
					kv->SetWString("GameDesc", g_pVGuiLocalize->Find("#ServerBrowser_PendingPing"));
				}

				int nAdjustedForBotsPlayers = max(0, pServer->m_nPlayers - pServer->m_nBotPlayers);
				int nAdjustedForBotsMaxPlayers = max(0, pServer->m_nMaxPlayers - pServer->m_nBotPlayers);

				char buf[256];
				Q_snprintf(buf, sizeof(buf), "%d / %d", nAdjustedForBotsPlayers, nAdjustedForBotsMaxPlayers);
				kv->SetString("Players", buf);
				kv->SetInt("Ping", pServer->m_nPing);
				kv->SetInt("password", pServer->m_bPassword ? 1 : 0);
				if (pServer->m_nBotPlayers > 0)
					kv->SetInt("bots", pServer->m_nBotPlayers);
				else
					kv->SetString("bots", "");

				server.m_iListID = m_pServerList->AddItem(kv, server.m_iServerID, false, false);
				kv->deleteThis();
			}

			// make sure the server is visible
			m_pServerList->SetItemVisible(server.m_iListID, true);
		}
	}

	UpdateStatus();
	m_pServerList->SortList();
	InvalidateLayout();
	Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: Resets UI server count
//-----------------------------------------------------------------------------
void CBaseTab::UpdateStatus()
{
	if (m_pServerList->GetItemCount() > 1)
	{
		wchar_t header[256];
		wchar_t count[128];

		V_snwprintf(count, Q_ARRAYSIZE(count), L"%d", m_pServerList->GetItemCount());
		g_pVGuiLocalize->ConstructString(header, sizeof(header), g_pVGuiLocalize->Find("#ServerBrowser_ServersCount"), 1, count);
		m_pServerList->SetColumnHeaderText(3, header);
	}
	else
	{
		m_pServerList->SetColumnHeaderText(3, g_pVGuiLocalize->Find("#ServerBrowser_Servers"));
	}
}

//-----------------------------------------------------------------------------
// Purpose: gets filter settings from controls
//-----------------------------------------------------------------------------
void CBaseTab::UpdateFilterSettings()
{
	// map
	m_pMapFilter->GetText(m_szMapFilter, sizeof(m_szMapFilter) - 1);
	Q_strlower(m_szMapFilter);

	// ping
	char buf[256];
	m_pPingFilter->GetText(buf, sizeof(buf));
	if (buf[0])
	{
		m_iPingFilter = atoi(buf + 2);
	}
	else
	{
		m_iPingFilter = 0;
	}

	// players
	m_bFilterNoFullServers = m_pNoFullServersFilterCheck->IsSelected();
	m_bFilterNoEmptyServers = m_pNoEmptyServersFilterCheck->IsSelected();
	m_bFilterNoPasswordedServers = m_pNoPasswordFilterCheck->IsSelected();
	m_bFilterHasAssociatedSteamAccount = m_pValidSteamAccountFilterCheck->IsSelected();
	m_iSecureFilter = m_pSecureFilter->GetActiveItem();

	m_vecServerFilters.RemoveAll();

	// update master filter string text
	if (m_bFilterNoEmptyServers)
	{
		m_vecServerFilters.AddToTail(MatchMakingKeyValuePair_t("empty", "1"));
	}
	if (m_bFilterNoFullServers)
	{
		m_vecServerFilters.AddToTail(MatchMakingKeyValuePair_t("full", "1"));
	}
	if (m_iSecureFilter == FILTER_SECURESERVERSONLY)
	{
		m_vecServerFilters.AddToTail(MatchMakingKeyValuePair_t("secure", "1"));
	}
	int regCode = GetRegionCodeToFilter();
	if (regCode > 0)
	{
		char szRegCode[32];
		Q_snprintf(szRegCode, sizeof(szRegCode), "%i", regCode);
		m_vecServerFilters.AddToTail(MatchMakingKeyValuePair_t("region", szRegCode));
	}

	// copy filter settings into filter file
	KeyValues* filter = CGameUIViewport::Get()->GetServerBrowser()->GetFilterSaveData(GetName());

	filter->SetString("map", m_szMapFilter);
	filter->SetInt("ping", m_iPingFilter);

	if (m_pLocationFilter->GetItemCount() > 1)
	{
		// only save this if there are options to choose from
		filter->SetInt("location", m_pLocationFilter->GetActiveItem());
	}

	filter->SetInt("NoFull", m_bFilterNoFullServers);
	filter->SetInt("NoEmpty", m_bFilterNoEmptyServers);
	filter->SetInt("NoPassword", m_bFilterNoPasswordedServers);
	filter->SetInt("HasAssociatedSteamAccount", m_bFilterHasAssociatedSteamAccount);
	filter->SetInt("Secure", m_iSecureFilter);

	OnSaveFilter(filter);

	RecalculateFilterString();
}


//-----------------------------------------------------------------------------
// Purpose: allow derived classes access to the saved filter string
//-----------------------------------------------------------------------------
void CBaseTab::OnSaveFilter(KeyValues* filter)
{
}

//-----------------------------------------------------------------------------
// Purpose: allow derived classes access to the saved filter string
//-----------------------------------------------------------------------------
void CBaseTab::OnLoadFilter(KeyValues* filter)
{
}

//-----------------------------------------------------------------------------
// Purpose: reconstructs the filter description string from the current filter settings
//-----------------------------------------------------------------------------
void CBaseTab::RecalculateFilterString()
{
	wchar_t unicode[2048], tempUnicode[128], spacerUnicode[8];
	unicode[0] = 0;
	int iTempUnicodeSize = sizeof(tempUnicode);

	Q_UTF8ToUnicode("; ", spacerUnicode, sizeof(spacerUnicode));

	if (m_iSecureFilter == FILTER_SECURESERVERSONLY)
	{
		wcscat(unicode, g_pVGuiLocalize->Find("#ServerBrowser_FilterDescSecureOnly"));
		wcscat(unicode, spacerUnicode);
	}
	else if (m_iSecureFilter == FILTER_INSECURESERVERSONLY)
	{
		wcscat(unicode, g_pVGuiLocalize->Find("#ServerBrowser_FilterDescInsecureOnly"));
		wcscat(unicode, spacerUnicode);
	}

	if (m_pLocationFilter->GetActiveItem() > 0)
	{
		m_pLocationFilter->GetText(tempUnicode, sizeof(tempUnicode));
		wcscat(unicode, tempUnicode);
		wcscat(unicode, spacerUnicode);
	}

	if (m_iPingFilter)
	{
		char tmpBuf[16];
		_snprintf(tmpBuf, sizeof(tmpBuf), "%d", m_iPingFilter);

		wcscat(unicode, g_pVGuiLocalize->Find("#ServerBrowser_FilterDescLatency"));
		Q_UTF8ToUnicode(" < ", tempUnicode, iTempUnicodeSize);
		wcscat(unicode, tempUnicode);
		Q_UTF8ToUnicode(tmpBuf, tempUnicode, iTempUnicodeSize);
		wcscat(unicode, tempUnicode);
		wcscat(unicode, spacerUnicode);
	}

	if (m_bFilterNoFullServers)
	{
		wcscat(unicode, g_pVGuiLocalize->Find("#ServerBrowser_FilterDescNotFull"));
		wcscat(unicode, spacerUnicode);
	}

	if (m_bFilterNoEmptyServers)
	{
		wcscat(unicode, g_pVGuiLocalize->Find("#ServerBrowser_FilterDescNotEmpty"));
		wcscat(unicode, spacerUnicode);
	}

	if (m_bFilterNoPasswordedServers)
	{
		wcscat(unicode, g_pVGuiLocalize->Find("#ServerBrowser_FilterDescNoPassword"));
		wcscat(unicode, spacerUnicode);
	}

	if (m_bFilterHasAssociatedSteamAccount)
	{
		wcscat(unicode, g_pVGuiLocalize->Find("#ServerBrowser_FilterDescValidSteamAccount"));
		wcscat(unicode, spacerUnicode);
	}

	if (m_szMapFilter[0])
	{
		Q_UTF8ToUnicode(m_szMapFilter, tempUnicode, iTempUnicodeSize);
		wcscat(unicode, tempUnicode);
	}

	m_pFilterString->SetText(unicode);
}

//-----------------------------------------------------------------------------
// Purpose: Checks to see if the server passes the primary filters
//			if the server fails the filters, it will not be refreshed again
//-----------------------------------------------------------------------------
bool CBaseTab::CheckPrimaryFilters(gameserveritem_t& server)
{
#if defined( TERROR )
	if (GetFilterAppID().AppID() == 238430/*1400*/ && server.m_nAppID == (int)GetFilterAppID().AppID())
	{
		return true;
	}
#endif
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Checks to see if a server passes the secondary set of filters
//			server will be continued to be pinged if it fails the filter, since
//			the relvent server data is dynamic
//-----------------------------------------------------------------------------
bool CBaseTab::CheckSecondaryFilters(gameserveritem_t& server)
{
	if (m_bFilterNoEmptyServers && (server.m_nPlayers - server.m_nBotPlayers) < 1)
	{
		return false;
	}

	if (m_bFilterNoFullServers && server.m_nPlayers >= server.m_nMaxPlayers)
	{
		return false;
	}

	if (m_iPingFilter && server.m_nPing > m_iPingFilter)
	{
		return false;
	}

	if (m_bFilterNoPasswordedServers && server.m_bPassword)
	{
		return false;
	}

	if (m_bFilterHasAssociatedSteamAccount && !server.m_steamID.IsValid())
	{
		return false;
	}

	if (m_iSecureFilter == FILTER_SECURESERVERSONLY && !server.m_bSecure)
	{
		return false;
	}

	if (m_iSecureFilter == FILTER_INSECURESERVERSONLY && server.m_bSecure)
	{
		return false;
	}

	// compare the first few characters of the filter name
	int count = Q_strlen(m_szMapFilter);
	if (count && Q_strnicmp(server.m_szMap, m_szMapFilter, count))
	{
		return false;
	}

	return CheckTagFilter(server);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
uint32 CBaseTab::GetServerFilters(MatchMakingKeyValuePair_t** pFilters)
{
	*pFilters = m_vecServerFilters.Base();
	return m_vecServerFilters.Count();
}


//-----------------------------------------------------------------------------
// Purpose: call to let the UI now whether the game list is currently refreshing
//-----------------------------------------------------------------------------
void CBaseTab::SetRefreshing(bool state)
{
	if (state)
	{
		CGameUIViewport::Get()->GetServerBrowser()->UpdateStatusText("#ServerBrowser_RefreshingServerList");

		// clear message in panel
		m_pServerList->SetEmptyListText("");
		m_pRefreshAll->SetText("#ServerBrowser_StopRefreshingList");
		m_pRefreshAll->SetCommand("stoprefresh");
		m_pRefreshQuick->SetEnabled(false);
	}
	else
	{
		CGameUIViewport::Get()->GetServerBrowser()->UpdateStatusText("");
		if (SupportsItem(CBaseTab::GETNEWLIST))
		{
			m_pRefreshAll->SetText("#ServerBrowser_RefreshAll");
		}
		else
		{
			m_pRefreshAll->SetText("#ServerBrowser_Refresh");
		}
		m_pRefreshAll->SetCommand("GetNewList");

		// 'refresh quick' button is only enabled if there are servers in the list
		if (m_pServerList->GetItemCount() > 0)
		{
			m_pRefreshQuick->SetEnabled(true);
		}
		else
		{
			m_pRefreshQuick->SetEnabled(false);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseTab::OnCommand(const char* command)
{
	if (!Q_stricmp(command, "Connect"))
	{
		OnBeginConnect();
	}
	else if (!Q_stricmp(command, "stoprefresh"))
	{
		// cancel the existing refresh
		StopRefresh();
	}
	else if (!Q_stricmp(command, "refresh"))
	{
		if (SteamMatchmakingServers())
			SteamMatchmakingServers()->RefreshQuery(m_hRequest);
		SetRefreshing(true);
		m_iServerRefreshCount = 0;
	}
	else if (!Q_stricmp(command, "GetNewList"))
	{
		GetNewServerList();
	}
	else
	{
		BaseClass::OnCommand(command);
	}
}

//-----------------------------------------------------------------------------
// Purpose: called when a row gets selected in the list
//-----------------------------------------------------------------------------
void CBaseTab::OnItemSelected()
{
	if (m_pServerList->GetSelectedItemsCount() < 1)
	{
		m_pConnect->SetEnabled(false);
	}
	else
	{
		m_pConnect->SetEnabled(true);
	}
}

//-----------------------------------------------------------------------------
// Purpose: refreshes server list on F5
//-----------------------------------------------------------------------------
void CBaseTab::OnKeyCodePressed(vgui2::KeyCode code)
{
	if (code == vgui2::KEY_F5)
	{
		StartRefresh();
	}
	else
	{
		BaseClass::OnKeyCodePressed(code);
	}
}


//-----------------------------------------------------------------------------
// Purpose: Handle enter pressed in the games list page. Return true
// to intercept the message instead of passing it on through vgui.
//-----------------------------------------------------------------------------
bool CBaseTab::OnGameListEnterPressed()
{
	return false;
}


//-----------------------------------------------------------------------------
// Purpose: Get the # items selected in the game list.
//-----------------------------------------------------------------------------
int CBaseTab::GetSelectedItemsCount()
{
	return m_pServerList->GetSelectedItemsCount();
}

bool CBaseTab::FilterFind(std::string s1, std::string s2)
{
	if (s1.find(s2) != std::string::npos)
		return true;
	return false;
}

bool CBaseTab::HasGameTag(const std::string& str, const std::string& find)
{
	std::size_t found = str.rfind(find);
	if (found != std::string::npos)
		return true;
	return false;
}


//-----------------------------------------------------------------------------
// Purpose: adds a server to the favorites
//-----------------------------------------------------------------------------
void CBaseTab::OnAddToFavorites()
{
	if (!SteamMatchmakingServers())
		return;

	// loop through all the selected favorites
	for (int i = 0; i < m_pServerList->GetSelectedItemsCount(); i++)
	{
		int serverID = m_pServerList->GetItemUserData(m_pServerList->GetSelectedItem(i));

		gameserveritem_t* pServer = SteamMatchmakingServers()->GetServerDetails(m_hRequest, serverID);
		if (pServer)
		{
			// add to favorites list
			CGameUIViewport::Get()->GetServerBrowser()->AddServerToFavorites(*pServer);
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseTab::ServerFailedToRespond(HServerListRequest hReq, int iServer)
{
	ServerResponded(hReq, iServer);
}


//-----------------------------------------------------------------------------
// Purpose: removes the server from the UI list
//-----------------------------------------------------------------------------
void CBaseTab::RemoveServer(ServerData_t& server)
{
	if (m_pServerList->IsValidItemID(server.m_iListID))
	{
		// don't remove the server from list, just hide since this is a lot faster
		m_pServerList->SetItemVisible(server.m_iListID, false);

		// find the row in the list and kill
		//	m_pServerList->RemoveItem(server.listEntryID);
		//	server.listEntryID = GetInvalidServerListID();
	}

	UpdateStatus();
}


//-----------------------------------------------------------------------------
// Purpose: refreshes a single server
//-----------------------------------------------------------------------------
void CBaseTab::OnRefreshServer(int serverID)
{
	if (!SteamMatchmakingServers())
		return;

	// walk the list of selected servers refreshing them
	for (int i = 0; i < m_pServerList->GetSelectedItemsCount(); i++)
	{
		int serverID = m_pServerList->GetItemUserData(m_pServerList->GetSelectedItem(i));

		// refresh this server
		SteamMatchmakingServers()->RefreshServer(m_hRequest, serverID);
	}

	SetRefreshing(IsRefreshing());
}


//-----------------------------------------------------------------------------
// Purpose: starts the servers refreshing
//-----------------------------------------------------------------------------
void CBaseTab::StartRefresh()
{
	if (!SteamMatchmakingServers())
		return;

	ClearServerList();
	MatchMakingKeyValuePair_t* pFilters;
	int nFilters = GetServerFilters(&pFilters);

	if (m_hRequest)
	{
		SteamMatchmakingServers()->ReleaseRequest(m_hRequest);
		m_hRequest = NULL;
	}

	switch (m_eMatchMakingType)
	{
	case eFavoritesServer:
		m_hRequest = SteamMatchmakingServers()->RequestFavoritesServerList(m_uLimitToAppID, &pFilters, nFilters, this);
		break;
	case eHistoryServer:
		m_hRequest = SteamMatchmakingServers()->RequestHistoryServerList(m_uLimitToAppID, &pFilters, nFilters, this);
		break;
	case eInternetServer:
		m_hRequest = SteamMatchmakingServers()->RequestInternetServerList(m_uLimitToAppID, &pFilters, nFilters, this);
		break;
	case eFriendsServer:
		m_hRequest =SteamMatchmakingServers()->RequestFriendsServerList(m_uLimitToAppID, &pFilters, nFilters, this);
		break;
	case eLANServer:
		m_hRequest = SteamMatchmakingServers()->RequestLANServerList(m_uLimitToAppID, this);
		break;
	default:
		Assert(!"Unknown server type");
		break;
	}

	SetRefreshing(true);

	m_iServerRefreshCount = 0;
}


//-----------------------------------------------------------------------------
// Purpose: Remove all the servers we currently have
//-----------------------------------------------------------------------------
void CBaseTab::ClearServerList()
{
	m_mapServers.RemoveAll();
	m_mapServerIP.RemoveAll();
	m_pServerList->RemoveAll();
}


//-----------------------------------------------------------------------------
// Purpose: get a new list of servers from the backend
//-----------------------------------------------------------------------------
void CBaseTab::GetNewServerList()
{
	StartRefresh();
}


//-----------------------------------------------------------------------------
// Purpose: stops current refresh/GetNewServerList()
//-----------------------------------------------------------------------------
void CBaseTab::StopRefresh()
{
	// clear update states
	m_iServerRefreshCount = 0;

	// Stop the server list refreshing
	if (SteamMatchmakingServers())
		SteamMatchmakingServers()->CancelQuery(m_hRequest);
	// update UI
	RefreshComplete(m_hRequest, eServerResponded);
}

//-----------------------------------------------------------------------------
// Purpose: returns true if the list is currently refreshing servers
//-----------------------------------------------------------------------------
bool CBaseTab::IsRefreshing()
{
	return SteamMatchmakingServers() && SteamMatchmakingServers()->IsRefreshing(m_hRequest);
}

//-----------------------------------------------------------------------------
// Purpose: Activates the page, starts refresh
//-----------------------------------------------------------------------------
void CBaseTab::OnPageShow()
{
	StartRefresh();
}

//-----------------------------------------------------------------------------
// Purpose: Called on page hide, stops any refresh
//-----------------------------------------------------------------------------
void CBaseTab::OnPageHide()
{
	StopRefresh();
}

//-----------------------------------------------------------------------------
// Purpose: initiates server connection
//-----------------------------------------------------------------------------
void CBaseTab::OnBeginConnect()
{
	if (!m_pServerList->GetSelectedItemsCount())
		return;

	// get the server
	int serverID = m_pServerList->GetItemUserData(m_pServerList->GetSelectedItem(0));

	// Stop the current refresh
	StopRefresh();

	// join the game
	CGameUIViewport::Get()->GetServerBrowser()->JoinGame(serverID);
}

//-----------------------------------------------------------------------------
// Purpose: Displays the current game info without connecting
//-----------------------------------------------------------------------------
void CBaseTab::OnViewGameInfo()
{
	if (!m_pServerList->GetSelectedItemsCount())
		return;

	// get the server
	int serverID = m_pServerList->GetItemUserData(m_pServerList->GetSelectedItem(0));

	// Stop the current refresh
	StopRefresh();

	// join the game
	CGameUIViewport::Get()->GetServerBrowser()->OpenGameInfoDialog(this, serverID);
}


//-----------------------------------------------------------------------------
// Purpose: Refresh if our favorites list changed
//-----------------------------------------------------------------------------
void CBaseTab::OnFavoritesMsg(FavoritesListChanged_t* pFavListChanged, bool bIOFailure)
{
	if (!pFavListChanged->m_nIP) // a zero for IP means the whole list was reloaded and we need to reload
	{
		switch (m_eMatchMakingType)
		{
		case eInternetServer:
		case eLANServer:
		case eFriendsServer:
			return;
		case eFavoritesServer:
		case eHistoryServer:
			// check containing property sheet to see if the page is visible.
			// if not, don't bother initiating a server list grab right now -
			// it will happen when the dialog is activated later.
			if (reinterpret_cast<vgui2::PropertySheet*>(GetParent())->GetActivePage() == this &&
				GetParent()->IsVisible() && CGameUIViewport::Get()->GetServerBrowser()->IsVisible())
			{
				GetNewServerList();
			}
			return;
		default:
			Assert(!"unknown matchmaking type");
		}
		return;
	}

	switch (m_eMatchMakingType)
	{
	case eInternetServer:
	case eLANServer:
	case eFriendsServer:
		break;
	case eFavoritesServer:
	case eHistoryServer:
	{
		servernetadr_t ipfind;
		ipfind.SetIP(pFavListChanged->m_nIP);
		ipfind.SetConnectionPort(pFavListChanged->m_nConnPort);
		int iIPServer = m_mapServerIP.Find(ipfind);
		if (iIPServer == m_mapServerIP.InvalidIndex())
		{
			if (pFavListChanged->m_bAdd)
			{
				if (SteamMatchmakingServers())
					SteamMatchmakingServers()->PingServer(pFavListChanged->m_nIP, pFavListChanged->m_nQueryPort, this);
			}
			// ignore deletes of fav's we didn't have
		}
		else
		{
			if (pFavListChanged->m_bAdd)
			{
				if (m_mapServerIP[iIPServer] > 0)
					ServerResponded(m_hRequest, m_mapServerIP[iIPServer]);
			}
			else
			{
				int iServer = m_mapServers.Find(m_mapServerIP[iIPServer]);
				ServerData_t& server = m_mapServers[iServer];
				RemoveServer(server);
			}
		}
	}
	break;
	default:
		Assert(!"unknown matchmaking type");
	};
}