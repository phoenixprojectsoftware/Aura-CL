#include <KeyValues.h>
#include <IEngineVGui.h>

#include <vgui/ISurface.h>
#include <vgui_controls/AnimationController.h>

#include "client_viewport.h"
#include "score_panel.h"

#include "bridge.h"

#include "../client_vgui.h"
#include "../hud.h"

CClientViewport* g_pViewport = nullptr;

CClientViewport::CClientViewport()
	: BaseClass(nullptr, "CClientViewport"),
	m_pScorePanel(nullptr)
{
	Assert(!g_pViewport);
	g_pViewport = this;

	SetParent(g_pEngineVGui->GetPanel(PANEL_CLIENTDLL));

	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);

	ReloadScheme("ui/resource/ClientScheme.res");
	InvalidateLayout(true, true);

	m_pScorePanel = new CScorePanel(this);

	PerformLayout();
	HideClientUI();
}

void CClientViewport::Start()
{
	// The scoreboard is created in the constructor.
	//
	// We retain this function because Start() is part of the normal
	// IClientVGUI lifecycle and may be useful for future panels.
}

void CClientViewport::VidInit()
{
	InvalidateLayout(true, true);

	if (m_pScorePanel)
		m_pScorePanel->Reset();
}

void CClientViewport::ReloadScheme(const char* fromFile)
{
	if (fromFile != nullptr)
	{
		vgui2::HScheme scheme =
			vgui2::scheme()->LoadSchemeFromFile(fromFile, "HudScheme");

		SetScheme(scheme);
		SetProportional(true);
	}
}

void CClientViewport::ActivateClientUI()
{
	SetVisible(true);
}

void CClientViewport::HideClientUI()
{
	HideScoreBoard();
	SetVisible(false);
}

bool CClientViewport::IsScoreBoardVisible() const
{
	return m_pScorePanel && m_pScorePanel->IsVisible();
}

void CClientViewport::ShowScoreBoard()
{
	// The parent viewport must be visible or its visible children will not draw.
	SetVisible(true);

	// remain passive until the first click activates interaction
	SetMouseInputEnabled(false);

	if (m_pScorePanel)
		m_pScorePanel->ShowPanel(true);
}

void CClientViewport::HideScoreBoard()
{
	// the intermission scoreboard remains on-screen until we leave
	if (gHUD.m_iIntermission)
		return;

	SetMouseInputEnabled(false);

	if (m_pScorePanel)
		m_pScorePanel->ShowPanel(false);
}

void CClientViewport::PerformLayout()
{
	BaseClass::PerformLayout();

	int wide = 640;
	int tall = 480;

	vgui2::surface()->GetScreenSize(wide, tall);

	// Previously the viewport was only 10x10 pixels. A full-screen parent is
	// required for centrally positioned HUD panels.
	SetBounds(0, 0, wide, tall);

	if (m_pScorePanel)
		m_pScorePanel->InvalidateLayout(true);
}

bool VGUI2ViewportAvailable()
{
	return g_pViewport != nullptr;
}

void ShowVGUI2ScoreBoard()
{
	if (g_pViewport)
		g_pViewport->ShowScoreBoard();
}

void HideVGUI2ScoreBoard()
{
	if (g_pViewport)
		g_pViewport->HideScoreBoard();
}

bool IsVGUI2ScoreBoardMouseActive()
{
	return g_pViewport && g_pViewport->GetScoreboard() && g_pViewport->GetScoreboard()->IsMousePointerEnabled();
}

bool IsVGUI2ScoreBoardVisible()
{
	return g_pViewport && g_pViewport->IsScoreBoardVisible();
}

void ActivateScoreBoardMouse()
{
	if (!g_pViewport ||
		!g_pViewport->IsScoreBoardVisible())
	{
		return;
	}

	g_pViewport->SetMouseInputEnabled(true);

	CScorePanel* scoreBoard =
		g_pViewport->GetScoreboard();

	if (scoreBoard)
		scoreBoard->EnableMousePointer(true);
}
