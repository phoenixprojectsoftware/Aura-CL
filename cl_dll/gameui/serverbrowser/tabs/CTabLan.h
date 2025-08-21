// Ported from Source, but modified for GoldSrc

#ifndef SERVER_BROWSER_TAB_LAN_H
#define SERVER_BROWSER_TAB_LAN_H

#include "../CBaseTab.h"

class CTabLan : public CBaseTab
{
	DECLARE_CLASS_SIMPLE(CTabLan, CBaseTab);

public:
	CTabLan(vgui2::Panel* parent, bool bAutoRefresh = true, const char* pCustomResFilename = NULL);
	~CTabLan();

	// property page handlers
	virtual void OnPageShow();

	// IGameList handlers
	// returns true if the game list supports the specified ui elements
	virtual bool SupportsItem(InterfaceItem_e item);

	// Control which button are visible.
	void ManualShowButtons(bool bShowConnect, bool bShowRefreshAll, bool bShowFilter);

	virtual void StartRefresh();

	// stops current refresh/GetNewServerList()
	virtual void StopRefresh();


	// IServerRefreshResponse handlers
	// called when a server response has timed out
	virtual void ServerFailedToRespond(HServerListRequest hReq, int iServer);

	// called when the current refresh list is complete
	virtual void RefreshComplete(HServerListRequest hReq, EMatchMakingServerResponse response);

	// Tell the game list what to put in there when there are no games found.
	virtual void SetEmptyListText();

private:
	// vgui message handlers
	virtual void OnTick();

	// lan timeout checking
	virtual void CheckRetryRequest();

	// context menu message handlers
	MESSAGE_FUNC_INT(OnOpenContextMenu, "OpenContextMenu", itemID);

	// number of servers refreshed
	int m_iServerRefreshCount;

	// true if we're broadcasting for servers
	bool m_bRequesting;

	// time at which we last broadcasted
	double m_fRequestTime;

	bool m_bAutoRefresh;
};

#endif