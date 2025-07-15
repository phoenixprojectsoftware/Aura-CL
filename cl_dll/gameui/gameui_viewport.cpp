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
	CGameUIViewport::Get()->OpenTestPanel();
}

CGameUIViewport::CGameUIViewport() : BaseClass(nullptr, "ClientGameUIViewport")
{
	Assert(!m_sInstance);
	m_sInstance = this;

	vgui2::VPANEL parent = g_pEngineVGui->GetPanel(PANEL_GAMEUIDLL);
	SetScheme(vgui2::scheme()->LoadSchemeFromFile(VGUI2_ROOT_DIR "resource/ClientSourceScheme.res", "ClientSourceScheme"));
	SetProportional(false);
	SetSize(0, 0);

	CGameUITestPanel* pTestPanel = new CGameUITestPanel(this);
	m_hTestPanel = pTestPanel;
}

CGameUIViewport::~CGameUIViewport()
{
	Assert(m_sInstance);
	m_sInstance = this;
}

void CGameUIViewport::PreventEscapeToShow(bool state)
{
	if (state)
	{
		m_bPreventEscape = true;
		m_iDelayedPreventEscapeFrame = 0;
	}
	else
	{
		// PreventEscapeToShow(false) may be called the same frame that esc was pressed
		// and CGameUIViewport::OnThink wont hide gameui
		// so the change is delayed by 1 frame
		m_bPreventEscape = false;
		m_iDelayedPreventEscapeFrame = 1;
	}
}

void CGameUIViewport::OpenTestPanel()
{
	GetDialog(m_hTestPanel)->Activate();
}

CServerBrowser* CGameUIViewport::GetServerBrowser()
{
	return GetDialog(m_hServerBrowser);
}

// TODO: VAC BANNED FUNC
