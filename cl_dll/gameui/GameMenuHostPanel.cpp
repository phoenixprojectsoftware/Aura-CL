#include "../menu/GameMenu.h"

#include "GameMenuHostPanel.h"

#include <vgui/ISurface.h>
#include <vgui/IVGui.h>

using namespace vgui2;

CGameMenuHostPanel::CGameMenuHostPanel(VPANEL parent) : BaseClass(nullptr, "AuraRmlGameMenuHost")
{
	SetVisible(false);
	SetEnabled(true);

	SetPaintBackgroundEnabled(false);
	SetPaintBorderEnabled(false);

	SetMouseInputEnabled(true);
	SetKeyBoardInputEnabled(true);

	ivgui()->AddTickSignal(GetVPanel(), 0);
}

CGameMenuHostPanel::~CGameMenuHostPanel() {}

void CGameMenuHostPanel::Activate()
{
	if (!g_GameMenu.IsInitialized())
		g_GameMenu.Init();

	SetVisible(true);
	SetEnabled(true);
	SetPaintEnabled(true);
	SetMouseInputEnabled(true);
	SetKeyBoardInputEnabled(true);

	UpdateSize();

	MakePopup(false, true);
	SetZPos(-100);
	MoveToFront();
	RequestFocus();

	if (g_GameMenu.HasContext())
		g_GameMenu.Show();
}

void CGameMenuHostPanel::Deactivate()
{
	g_GameMenu.Hide();

	SetVisible(false);
	SetEnabled(false);
}

void CGameMenuHostPanel::OnTick()
{
	BaseClass::OnTick();

	if (!IsVisible())
		return;

	// If context was not ready during Activate(), keep trying until it is.
	if (!g_GameMenu.HasContext())
	{
		if (!UpdateSize())
			return;

		g_GameMenu.Show();
	}

	g_GameMenu.Update(0.0f);
}

void CGameMenuHostPanel::PerformLayout()
{
	BaseClass::PerformLayout();
	UpdateSize();
}

void CGameMenuHostPanel::Paint()
{
	BaseClass::Paint();

	static int s_iPaintCount = 0;

#ifdef _DEBUG
	if ((++s_iPaintCount % 120) == 0)
	{
		int wide = 0;
		int tall = 0;
		GetSize(wide, tall);

		Msg("CGameMenuHostPanel::Paint visible=%d, size=%d x %d\n", IsVisible() ? 1 : 0, wide, tall);
	}
#endif

	if (!IsVisible())
		return;

	g_GameMenu.Render();
}

bool CGameMenuHostPanel::UpdateSize()
{
	int wide = 0;
	int tall = 0;

	vgui2::surface()->GetScreenSize(wide, tall);

	if (wide <= 0 || tall <= 0)
	{
		Msg("CGameMenuHostPanel: screen size not ready yet (%d x %d)\n", wide, tall);
		return false;
	}

	SetBounds(0, 0, wide, tall);
	g_GameMenu.SetViewportSize(wide, tall);

	return true;
}

void CGameMenuHostPanel::OnCursorMoved(int x, int y)
{
	BaseClass::OnCursorMoved(x, y);
	g_GameMenu.OnMouseMove(x, y);
}

void CGameMenuHostPanel::OnMousePressed(vgui2::MouseCode code)
{
	BaseClass::OnMousePressed(code);

	RequestFocus();
	g_GameMenu.OnMousePressed(code);
}

void CGameMenuHostPanel::OnMouseReleased(vgui2::MouseCode code)
{
	BaseClass::OnMouseReleased(code);
	g_GameMenu.OnMouseReleased(code);
}

void CGameMenuHostPanel::OnMouseWheeled(int delta)
{
	BaseClass::OnMouseWheeled(delta);
	g_GameMenu.OnMouseWheeled(delta);
}

void CGameMenuHostPanel::OnKeyCodePressed(vgui2::KeyCode code)
{
	BaseClass::OnKeyCodePressed(code);

	if (code == vgui2::KEY_ESCAPE)
	{
		Deactivate();
		return;
	}

	g_GameMenu.OnKeyCodePressed(code);
}

void CGameMenuHostPanel::OnKeyCodeReleased(vgui2::KeyCode code)
{
	BaseClass::OnKeyCodeReleased(code);
	g_GameMenu.OnKeyCodeReleased(code);
}

void CGameMenuHostPanel::OnKeyTyped(wchar_t unichar)
{
	BaseClass::OnKeyTyped(unichar);
	g_GameMenu.OnTextInput(unichar);
}
