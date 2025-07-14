/****
*
* Copyright © 2021-2025 The Phoenix Project Software. Some Rights Reserved.
*
* AURA
*
* baseviewport - Base viewport for the VGUI2 client interface. Based upon
* Source SDK 2013.
*
****/

#include <tier1/interface.h>
#include <IEngineVGui.h>
#include <tier2/tier2.h>
#include "../hud.h"

#include "../cl_util.h"
#undef VectorAdd
#undef VectorSubtract
#undef VectorCopy
#undef DotProduct
#undef VectorClear
#undef AngleVectors

#include "vgui2_client_impl.h"
#include "baseviewport.h"
#include "gameui_test_panel.h"

CON_COMMAND(gameui_opentest, "Opens a test panel for client GameUI")
{
		CGameUIViewport::Get()->OpenTestPanel();
}

CGameUIViewport* g_pViewport;

CGameUIViewport::CGameUIViewport() : BaseClass(nullptr, "ClientGameUIViewport")
{
	Assert(!m_sInstance);
	m_sInstance = this;

	SetParent(g_pEngineVGui->GetPanel(PANEL_GAMEUIDLL));
	SetScheme(vgui2::scheme()->LoadSchemeFromFile(VGUI2_CLIENTSOURCE_SCHEME_FILE, VGUI2_CLIENTSOURCE_SCHEME_TAG));
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
	if (!m_hTestPanel.Get())
	{
		m_hTestPanel = new CGameUITestPanel(this);
	}

	GetDialog(m_hTestPanel)->Activate();
}
