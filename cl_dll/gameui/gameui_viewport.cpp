#include <tier1/interface.h>
#include <IEngineVGui.h>
#include <tier2/tier2.h>
#include "../hud.h"
#include "../client_vgui.h"
#include "gameui_viewport.h"
#include "gameui_test_panel.h"
#include <convar.h>

CON_COMMAND(gameui_opentest, "Opens test")
{
	if (!CGameUIViewport::Get())
		new CGameUIViewport();

	CGameUIViewport::Get()->OpenTestPanel();
}

#ifdef _DEBUG
CON_COMMAND(gameui_testnull, "")
{
	if (CGameUIViewport::Get())
		CGameUIViewport::Get()->OpenTestPanel();
	else
		Msg("CGameUIViewport is null!\n");
}

CON_COMMAND(gameui_error, "Cause an error")
{
	Error("This is a test error from gameui_viewport.cpp\n"
		"Please report this to the developers if you see this message.\n");
}
#endif
#if defined(_DEBUG) || defined(CLOSED_BETA)
CON_COMMAND(gameui_init, "Init GameUI viewport")
{
	if (!CGameUIViewport::Get())
	{
		Msg("Creating GameUIViewport...\n");
		new CGameUIViewport();
	}
}
#endif

CGameUIViewport::CGameUIViewport() : BaseClass(nullptr, "ClientGameUIViewport")
{
	Assert(!m_sInstance);
	m_sInstance = this;

	vgui2::VPANEL parent = g_pEngineVGui->GetPanel(PANEL_GAMEUIDLL);
	Assert(parent); // runtime check to ensure the parent panel is valid

	SetScheme(vgui2::scheme()->LoadSchemeFromFile(VGUI2_ROOT_DIR "resource/ClientSourceScheme.res", "ClientSourceScheme"));
	SetProportional(false);
	SetSize(0, 0);
	SetVisible(true);
}

CGameUIViewport::~CGameUIViewport()
{
	Assert(m_sInstance);
	m_sInstance = this;
}

void CGameUIViewport::OpenTestPanel()
{
	GetDialog(m_hTestPanel)->Activate();
}
