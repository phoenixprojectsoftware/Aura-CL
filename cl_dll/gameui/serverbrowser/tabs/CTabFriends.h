// Ported from Source, but modified for GoldSrc

#ifndef SERVER_BROWSER_TAB_FRIENDS_H
#define SERVER_BROWSER_TAB_FRIENDS_H

#include "gameui/serverbrowser/CBaseTab.h"

class CTabFriends : public CBaseTab
{
	DECLARE_CLASS_SIMPLE(CTabFriends, CBaseTab);

public:
	CTabFriends(vgui2::Panel* parent);
	~CTabFriends();

	// IGameList handlers
	// returns true if the game list supports the specified ui elements
	virtual bool SupportsItem(InterfaceItem_e item);

	// called when the current refresh list is complete
	virtual void RefreshComplete(HServerListRequest hReq, EMatchMakingServerResponse response);

private:
	// context menu message handlers
	MESSAGE_FUNC_INT(OnOpenContextMenu, "OpenContextMenu", itemID);

	int m_iServerRefreshCount;	// number of servers refreshed
};

#endif