// Ported from Source, but modified for GoldSrc

#ifndef DIALOGADDSERVER_H
#define DIALOGADDSERVER_H
#ifdef _WIN32
#pragma once
#endif

class CAddServerGameList;
class CBaseTab;

//-----------------------------------------------------------------------------
// Purpose: Dialog which lets the user add a server by IP address
//-----------------------------------------------------------------------------
class CDialogAddServer : public vgui2::Frame, public ISteamMatchmakingPingResponse
{
	DECLARE_CLASS_SIMPLE(CDialogAddServer, vgui2::Frame);
	friend class CAddServerGameList;

public:
	CDialogAddServer(vgui2::Panel* parent, CBaseTab* gameList);
	~CDialogAddServer();

	void ServerResponded(gameserveritem_t& server);
	void ServerFailedToRespond();

	void ApplySchemeSettings(vgui2::IScheme* pScheme);

	MESSAGE_FUNC(OnItemSelected, "ItemSelected");
private:
	virtual void OnCommand(const char* command);

	void GrabIPAddress(const char* szAddress, unsigned int* nIP, uint16* nQuery, uint16* nPort);
	void GetMostCommonQueryPorts(CUtlVector<uint16>& ports);
	void OnOK();

	void TestServers();
	MESSAGE_FUNC(OnTextChanged, "TextChanged");

private:
	CBaseTab* m_pGameList;

	vgui2::Button* m_pTestServersButton;
	vgui2::Button* m_pAddServerButton;
	vgui2::Button* m_pAddSelectedServerButton;

	vgui2::PropertySheet* m_pTabPanel;
	vgui2::TextEntry* m_pTextEntry;
	vgui2::ListPanel* m_pDiscoveredGames;
	int m_OriginalHeight;
	CUtlVector<gameserveritem_t> m_Servers;
	CUtlVector<HServerQuery> m_Queries;
};


#endif // DIALOGADDSERVER_H