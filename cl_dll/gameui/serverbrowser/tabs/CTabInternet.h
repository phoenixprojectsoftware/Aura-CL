// Ported from Source, but modified for GoldSrc

#ifndef SERVER_BROWSER_TAB_INTERNET_H
#define SERVER_BROWSER_TAB_INTERNET_H

#include "gameui/serverbrowser/CBaseTab.h"

class CTabInternet : public CBaseTab
{
public:
	DECLARE_CLASS_SIMPLE(CTabInternet, CBaseTab);

public:
	CTabInternet(vgui2::Panel* parent, const char* panelName = "InternetGames", EPageType eType = eInternetServer);
	~CTabInternet();

	// property page handlers
	virtual void OnPageShow();

	// returns true if the game list supports the specified ui elements
	virtual bool SupportsItem(CBaseTab::InterfaceItem_e item);

	// gets a new server list
	MESSAGE_FUNC(GetNewServerList, "GetNewServerList");

	// serverlist refresh responses
	virtual void ServerResponded(HServerListRequest hReq, int iServer);
	virtual void ServerFailedToRespond(HServerListRequest hReq, int iServer);
	virtual void RefreshComplete(HServerListRequest hReq, EMatchMakingServerResponse response);
	MESSAGE_FUNC_INT(OnRefreshServer, "RefreshServer", serverID);

	virtual int GetRegionCodeToFilter();
	virtual bool CheckTagFilter(gameserveritem_t& server);

protected:
	// vgui overrides
	virtual void PerformLayout();
	virtual void OnTick();

	virtual const char* GetStringNoUnfilteredServers() { return "#ServerBrowser_NoInternetGames"; }
	virtual const char* GetStringNoUnfilteredServersOnMaster() { return "#ServerBrowser_MasterServerHasNoServersListed"; }
	virtual const char* GetStringNoServersResponded() { return "#ServerBrowser_NoInternetGamesResponded"; }

private:
	// Called once per frame to see if sorting needs to occur again
	void CheckRedoSort();
	// opens context menu (user right clicked on a server)
	MESSAGE_FUNC_INT(OnOpenContextMenu, "OpenContextMenu", itemID);

	struct regions_s
	{
		CUtlSymbol name;
		unsigned char code;
	};

	CUtlVector<struct regions_s> m_Regions;	// list of the different regions you can query for

	float				m_fLastSort;	// Time of last re-sort
	bool				m_bDirty;	// Has the list been modified, thereby needing re-sort
	bool				m_bRequireUpdate;	// checks whether we need an update upon opening

	// error cases for if no servers are listed
	bool				m_bAnyServersRetrievedFromMaster;
	bool				m_bAnyServersRespondedToQuery;
	bool				m_bNoServersListedOnMaster;

	bool m_bOfflineMode;
};

#endif