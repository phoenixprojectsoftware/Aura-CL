// Ported from Source, but modified for GoldSrc

#ifndef CREATE_SERVER_BROWSER_H
#define CREATE_SERVER_BROWSER_H

#include <vgui_controls/Frame.h>
#include <vgui_controls/TextEntry.h>
#include <vgui_controls/PropertySheet.h>
#include <FileSystem.h>
#include "CBaseTab.h"

class CDialogGameInfo;
class CServerContextMenu;
class CTabInternet;
class CTabFavorites;
class CTabHistory;
class CTabLan;
class CTabFriends;

class CServerBrowser : public vgui2::Frame
{
	DECLARE_CLASS_SIMPLE(CServerBrowser, vgui2::Frame);

public:
	CServerBrowser(vgui2::Panel* pParent);
	~CServerBrowser();

	// joins a specified game - game info dialog will only be opened if the server is fully or passworded
	bool JoinGame(uint32 unGameIP, uint16 usGamePort);
	bool JoinGame(uint64 ulSteamIDFriend);

	// opens a game info dialog to watch the specified server; associated with the friend 'userName'
	bool CanOpenGameInfoDialog(uint64 ulSteamIDFriend);
	CDialogGameInfo* OpenGameInfoDialog(CBaseTab* gameList, uint64 serverIndex);
	CDialogGameInfo* OpenGameInfoDialog(int serverIP, uint16 connPort, uint16 queryPort);

	// closes all the game info dialogs
	void CloseAllGameInfoDialogs();
	CDialogGameInfo* GetDialogGameInfoForFriend(uint64 ulSteamIDFriend);

	CServerContextMenu* GetContextMenu(vgui2::Panel* pParent);

	// called every frame
	virtual void OnTick();

	// methods
	void OpenBrowser();
	void Close();

	KeyValues* GetFilterSaveData(const char* filterSet);

	// Load or save our crap
	void LoadUserData();
	void SaveUserData();

	void RefreshCurrentPage();

	// updates status text at bottom of window
	void UpdateStatusText(const char* format, ...);
	void UpdateStatusText(wchar_t* unicode);

	// Adds a server to the list of favorites
	void AddServerToFavorites(gameserveritem_t& server);

	gameserveritem_t* GetServer(unsigned int serverID);

	virtual gameserveritem_t* GetCurrentConnectedServer()
	{
		return &m_CurrentConnection;
	}

protected:
	CBaseTab* m_pGameList;
	CServerContextMenu* m_pContextMenu;
	KeyValues* m_pFilterData;
	KeyValues* m_pSavedData;
	AppId_t m_uLimitToAppID;

	CTabInternet* m_pInternetGames;
	CTabFavorites* m_pFavorites;
	CTabHistory* m_pHistory;
	CTabLan* m_pLanGames;
	CTabFriends* m_pFriendsGames;

private:

	// current game list change
	MESSAGE_FUNC(OnGameListChanged, "PageChanged");
	void ReloadFilterSettings();

	// receives a specified game is active, so no other game types can be displayed in server list
	MESSAGE_FUNC_PARAMS(OnActiveGameName, "ActiveGameName", name);

	// notification that we connected / disconnected
	MESSAGE_FUNC_PARAMS(OnConnectToGame, "ConnectedToGame", kv);
	MESSAGE_FUNC(OnDisconnectFromGame, "DisconnectedFromGame");

	MESSAGE_FUNC_PARAMS(ShowServerBrowserPage, "ShowServerBrowserPage", kv);

	virtual bool GetDefaultScreenPosition(int& x, int& y, int& wide, int& tall);

	// list of all open game info dialogs
	CUtlVector<vgui2::DHANDLE<CDialogGameInfo>> m_GameInfoDialogs;

	// Status text
	vgui2::Label* m_pStatusLabel;

	// property sheet
	vgui2::PropertySheet* m_pTabPanel;

	// currently connected game
	bool m_bCurrentlyConnected;
	gameserveritem_t m_CurrentConnection;
};

#endif