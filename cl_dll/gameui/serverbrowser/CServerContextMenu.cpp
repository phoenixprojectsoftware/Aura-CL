// Ported from Source to Goldsrc

#include <vgui/IVGui.h>
#include <vgui/IInputInternal.h>
#include <vgui/ISurface.h>
#include "tier1/KeyValues.h"
#include "CServerContextMenu.h"

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CServerContextMenu::CServerContextMenu(vgui2::Panel* parent) : Menu(parent, "ServerContextMenu")
{
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CServerContextMenu::~CServerContextMenu()
{
}

//-----------------------------------------------------------------------------
// Purpose: Activates the menu
//-----------------------------------------------------------------------------
void CServerContextMenu::ShowMenu(
	Panel* target,
	unsigned int serverID,
	bool showConnect,
	bool showViewGameInfo,
	bool showRefresh,
	bool showAddToFavorites)
{
	if (showConnect)
	{
		AddMenuItem("ConnectToServer", "#ServerBrowser_ConnectToServer", new KeyValues("ConnectToServer", "serverID", serverID), target);
	}

	if (showViewGameInfo)
	{
		AddMenuItem("ViewGameInfo", "#ServerBrowser_ViewServerInfo", new KeyValues("ViewGameInfo", "serverID", serverID), target);
	}

	if (showRefresh)
	{
		AddMenuItem("RefreshServer", "#ServerBrowser_RefreshServer", new KeyValues("RefreshServer", "serverID", serverID), target);
	}

	if (showAddToFavorites)
	{
		AddMenuItem("AddToFavorites", "#ServerBrowser_AddServerToFavorites", new KeyValues("AddToFavorites", "serverID", serverID), target);
	}

	int x, y, gx, gy;
	vgui2::input()->GetCursorPos(x, y);
	vgui2::ipanel()->GetPos(vgui2::surface()->GetEmbeddedPanel(), gx, gy);
	SetPos(x - gx, y - gy);
	SetVisible(true);
}