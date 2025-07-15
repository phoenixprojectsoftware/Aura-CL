// Ported from Source, but modified for GoldSrc

#ifndef SERVER_BROWSER_TAB_HISTORY_H
#define SERVER_BROWSER_TAB_HISTORY_H

#include "gameui/serverbrowser/CBaseTab.h"

class CTabHistory : public CBaseTab
{
	DECLARE_CLASS_SIMPLE(CTabHistory, CBaseTab);

public:
	CTabHistory(vgui2::Panel* parent);
	~CTabHistory();

	// favorites list, loads/saves into keyvalues
	void LoadHistoryList();


	// IGameList handlers
	// returns true if the game list supports the specified ui elements
	virtual bool SupportsItem(InterfaceItem_e item);

	// called when the current refresh list is complete
	virtual void RefreshComplete(HServerListRequest hReq, EMatchMakingServerResponse response);

	void SetRefreshOnReload() { m_bRefreshOnListReload = true; }

private:
	// context menu message handlers
	MESSAGE_FUNC_INT(OnOpenContextMenu, "OpenContextMenu", itemID);
	MESSAGE_FUNC(OnRemoveFromHistory, "RemoveFromHistory");

	bool m_bRefreshOnListReload;
};

#endif