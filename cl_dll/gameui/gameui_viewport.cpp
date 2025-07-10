/****
*
* Copyright © 2021-2025 The Phoenix Project Software. Some Rights Reserved.
*
* AURA
*
* gameui_viewport - GameUI viewport for the VGUI2 client interface. Based upon
* BugfixedHL-Rebased. Used as per the license agreement.
*
*
****/

#include <IEngineVGui.h>
#include <tier2/tier2.h>
#include "../hud.h"
#include "../cl_util.h"
#include "gameui_viewport.h"
#include "gameui_test_panel.h"

CON_COMMAND(gameui_cl_open_test_panel, "Opens a test panel for client GameUI")
{
	CGameUIViewport::Get()->OpenTestPanel();
}

CGameUIViewport::CGameUIViewport()
	: BaseClass(nullptr, "ClientGameUIViewport")
{
	Assert(!m_sInstance);
	m_sInstance = this;

	SetParent(g_pEngineVGui->GetPanel(PANEL_GAMEUIDLL));
	SetScheme(vgui2::scheme()->LoadSchemeFromFile(VGUI2_ROOT_DIR "resource/ClientSourceScheme.res", "ClientSourceScheme"));
	SetProportional(false);
	SetSize(0, 0);
}

CGameUIViewport::~CGameUIViewport()
{
	Assert(m_sInstance);
	m_sInstance = nullptr;
}

void CGameUIViewport::OpenTestPanel()
{
	GetDialog(m_hTestPanel)->Activate();
}