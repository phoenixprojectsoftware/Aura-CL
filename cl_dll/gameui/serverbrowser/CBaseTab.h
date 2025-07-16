// Ported from Source, but modified for GoldSrc

#ifndef SERVER_BROWSER_BASE_TAB_H
#define SERVER_BROWSER_BASE_TAB_H
#include <vgui_controls/ListPanel.h>
#include <vgui_controls/PropertyPage.h>
#include <steamworks/steam_api.h>
#include "tier1/utlmap.h"

class CBaseTab;
class CServerListPanel : public vgui2::ListPanel
{
public:
	DECLARE_CLASS_SIMPLE(CServerListPanel, vgui2::ListPanel);

	CServerListPanel(CBaseTab* pOuter, const char* pName);

	virtual void OnKeyCodeTyped(vgui2::KeyCode code);

private:
	CBaseTab* m_pOuter;
};

class CBaseTab : public vgui2::PropertyPage, public ISteamMatchmakingServerListResponse, public ISteamMatchmakingPingResponse
{
	DECLARE_CLASS_SIMPLE(CBaseTab, vgui2::PropertyPage);

public:
	enum EPageType
	{
		eInternetServer,
		eLANServer,
		eFriendsServer,
		eFavoritesServer,
		eHistoryServer
	};

	enum InterfaceItem_e
	{
		FILTERS,
		GETNEWLIST,
		ADDSERVER,
		ADDCURRENTSERVER,
	};

	struct ServerData_t
	{
		ServerData_t()
		{
			m_iListID = -1;
			m_iServerID = -1;
			m_bDoNotRefresh = true;
		}
		int			m_iListID;		// the VGUI2 list panel index for displaying this server
		int			m_iServerID;	// the matchmaking interface index for this server
		bool		m_bDoNotRefresh;	// Don't clear this item if true.
		bool operator==(const ServerData_t& rhs) const { return rhs.m_iServerID == m_iServerID; }
	};

public:
	CBaseTab(vgui2::Panel* parent, const char* name, EPageType eType, const char* pCustomResFilename = NULL);
	~CBaseTab();

	virtual void PerformLayout();
	virtual void ApplySchemeSettings(vgui2::IScheme* pScheme);

	uint32 GetServerFilters(MatchMakingKeyValuePair_t** pFilters);

	virtual void SetRefreshing(bool state);

	// loads filter settings from disk
	virtual void LoadFilterSettings();

	virtual void OnThink();

	// Called by CGameList when the enter key is pressed.
	// This is overridden in the add server dialog - since there is no Connect button, the message
	// never gets handled, but we want to add a server when they dbl-click or press enter.
	virtual bool OnGameListEnterPressed();

	int GetSelectedItemsCount();

	// Ported from UI2ServerBrowser
	bool FilterFind(std::string s1, std::string s2);
	bool HasGameTag(const std::string& str, const std::string& find);

	// adds a server to the favorites
	MESSAGE_FUNC(OnAddToFavorites, "AddToFavorites");

	virtual void UpdateDerivedLayouts(void);

	void OnFavoritesMsg(FavoritesListChanged_t* pCallback, bool bIOFailure);
	CCallResult<CBaseTab, FavoritesListChanged_t> m_SteamCallResultOnFavoritesMsg;

	// returns true if the game list supports the specified ui elements
	virtual bool SupportsItem(InterfaceItem_e item) = 0;

	// starts the servers refreshing
	virtual void StartRefresh();

	// gets a new server list
	virtual void GetNewServerList();

	// stops current refresh/GetNewServerList()
	virtual void StopRefresh();

	// returns true if the list is currently refreshing servers
	virtual bool IsRefreshing();

	// gets information about specified server
	virtual gameserveritem_t* GetServer(unsigned int serverID);

	// invalid server index
	virtual int GetInvalidServerListID();

protected:
	virtual void OnCommand(const char* command);
	virtual void OnKeyCodePressed(vgui2::KeyCode code);
	virtual int GetRegionCodeToFilter() { return -1; }

	MESSAGE_FUNC(OnItemSelected, "ItemSelected");

	// applies games filters to current list
	void ApplyGameFilters();
	// updates server count UI
	void UpdateStatus();

	// ISteamMatchmakingServerListResponse callbacks
	virtual void ServerResponded(HServerListRequest hReq, int iServer);
	virtual void ServerResponded(int iServer, gameserveritem_t* pServerItem);
	virtual void UpdateServerPlayerCount(gameserveritem_t& server);
	virtual void ServerFailedToRespond(HServerListRequest hReq, int iServer);
	virtual void RefreshComplete(HServerListRequest hReq, EMatchMakingServerResponse response) = 0;

	// ISteamMatchmakingPingResponse callbacks
	virtual void ServerResponded(gameserveritem_t& server);
	virtual void ServerFailedToRespond() {}

	// Removes server from list
	void RemoveServer(ServerData_t& server);

	virtual bool BShowServer(ServerData_t& server) { return server.m_bDoNotRefresh; }
	void ClearServerList();

	// filtering methods
	// returns true if filters passed; false if failed
	virtual bool CheckPrimaryFilters(gameserveritem_t& server);
	virtual bool CheckSecondaryFilters(gameserveritem_t& server);
	virtual bool CheckTagFilter(gameserveritem_t& server) { return true; }

	virtual void OnSaveFilter(KeyValues* filter);
	virtual void OnLoadFilter(KeyValues* filter);
	virtual void UpdateFilterSettings();

	virtual void OnPageShow();
	virtual void OnPageHide();

	// called when Connect button is pressed
	MESSAGE_FUNC(OnBeginConnect, "ConnectToServer");
	// called to look at game info
	MESSAGE_FUNC(OnViewGameInfo, "ViewGameInfo");
	// refreshes a single server
	MESSAGE_FUNC_INT(OnRefreshServer, "RefreshServer", serverID);

	// If true, then we automatically select the first item that comes into the games list.
	bool m_bAutoSelectFirstItemInGameList;

	CServerListPanel* m_pServerList;
	vgui2::ComboBox* m_pLocationFilter;

	// command buttons
	vgui2::Button* m_pConnect;
	vgui2::Button* m_pRefreshAll;
	vgui2::Button* m_pRefreshQuick;
	vgui2::Button* m_pAddServer;
	vgui2::Button* m_pAddCurrentServer;
	vgui2::Button* m_pAddToFavoritesButton;
	vgui2::ToggleButton* m_pFilter;

	CUtlMap<uint64, int> m_mapGamesFilterItem;
	CUtlMap<int, ServerData_t> m_mapServers;
	CUtlMap<servernetadr_t, int> m_mapServerIP;
	CUtlVector<MatchMakingKeyValuePair_t> m_vecServerFilters;
	uint32 m_iServerRefreshCount;

	EPageType m_eMatchMakingType;
	HServerListRequest m_hRequest;

protected:
	virtual void CreateFilters();

	MESSAGE_FUNC_PTR_CHARPTR(OnTextChanged, "TextChanged", panel, text);
	MESSAGE_FUNC_PTR_INT(OnButtonToggled, "ButtonToggled", panel, state);

private:
	void RequestServersResponse(int iServer, EMatchMakingServerResponse response, bool bLastServer); // callback for matchmaking interface

	void RecalculateFilterString();

	// If set, it uses the specified resfile name instead of its default one.
	const char* m_pCustomResFilename;

	// filter controls
	vgui2::TextEntry* m_pMapFilter;
	vgui2::ComboBox* m_pPingFilter;
	vgui2::ComboBox* m_pSecureFilter;
	vgui2::CheckButton* m_pNoFullServersFilterCheck;
	vgui2::CheckButton* m_pNoEmptyServersFilterCheck;
	vgui2::CheckButton* m_pNoPasswordFilterCheck;
	vgui2::CheckButton* m_pValidSteamAccountFilterCheck;
	vgui2::Label* m_pFilterString;
	char m_szComboAllText[64];

	KeyValues* m_pFilters; // base filter data
	bool m_bFiltersVisible;	// true if filter section is currently visible
	vgui2::HFont m_hFont;

	// filter data
	char m_szMapFilter[32];
	int	m_iPingFilter;
	bool m_bFilterNoFullServers;
	bool m_bFilterNoEmptyServers;
	bool m_bFilterNoPasswordedServers;
	bool m_bFilterHasAssociatedSteamAccount;
	int m_iSecureFilter;

	// AppID Filter
	AppId_t m_uLimitToAppID;
};

#endif