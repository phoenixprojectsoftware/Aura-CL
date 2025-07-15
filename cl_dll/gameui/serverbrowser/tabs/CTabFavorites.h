// Ported from Source, but modified for GoldSrc

#ifndef SERVER_BROWSER_TAB_FAVORITES_H
#define SERVER_BROWSER_TAB_FAVORITES_H

#include "gameui/serverbrowser/CBaseTab.h"

class CTabFavorites : public CBaseTab
{
	DECLARE_CLASS_SIMPLE(CTabFavorites, CBaseTab);

public:
	CTabFavorites(vgui2::Panel* parent);
	~CTabFavorites();

	// favorites list, loads/saves into keyvalues
	void LoadFavoritesList();

	// IGameList handlers
	// returns true if the game list supports the specified ui elements
	virtual bool SupportsItem(InterfaceItem_e item);

	// called when the current refresh list is complete
	virtual void RefreshComplete(HServerListRequest hReq, EMatchMakingServerResponse response);

	// passed from main server browser window instead of messages
	void OnConnectToGame();
	void OnDisconnectFromGame(void);

	void SetRefreshOnReload() { m_bRefreshOnListReload = true; }

private:
	// context menu message handlers
	MESSAGE_FUNC_INT(OnOpenContextMenu, "OpenContextMenu", itemID);
	MESSAGE_FUNC(OnRemoveFromFavorites, "RemoveFromFavorites");
	MESSAGE_FUNC(OnAddServerByName, "AddServerByName");

	void OnAddCurrentServer(void);

	void OnCommand(const char* command);

	bool m_bRefreshOnListReload;
};

#endif