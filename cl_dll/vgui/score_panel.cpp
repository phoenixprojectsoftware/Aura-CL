/****
*
* Copyright (c) 2021-2026 The Phoenix Project Software. Some Rights Reserved.
*
* AURA
*
* VGUI2 scoreboard
*
*
****/

#include <vgui/IScheme.h>
#include <vgui_controls/Label.h>

#include "score_panel.h"
#include "viewport_panel_names.h"

CScorePanel::CScorePanel(vgui2::Panel* parent) : BaseClass(parent, PANEL_SCOREBOARD), m_pStatusLabel(nullptr)
{
	SetTitle("CROSS PRODUCT MULTIPLAYER", true);

	SetSizeable(false);
	SetMoveable(false);
	SetCloseButtonVisible(false);
	SetDeleteSelfOnClose(false);

	SetProportional(true);

	// prototype scoreboard is display-only
	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);

	m_pStatusLabel = new vgui2::Label(this, "ScoreboardStatus", "VGUI2 scoreboard");

	m_pStatusLabel->SetContentAlignment(vgui2::Label::a_center);

	SetVisible(false);
}

const char* CScorePanel::GetName()
{
	return PANEL_SCOREBOARD;
}

void CScorePanel::Reset()
{
	ShowPanel(false);
}

void CScorePanel::ShowPanel(bool state)
{
	if (state == IsVisible())
		return;

	SetVisible(state);

	if (state)
	{
		MoveToFront();
		RequestFocus();
	}
}

vgui2::VPANEL CScorePanel::GetVPanel()
{
	return BaseClass::GetVPanel();
}

bool CScorePanel::IsVisible()
{
	return BaseClass::IsVisible();
}

void CScorePanel::SetParent(vgui2::VPANEL parent)
{
	BaseClass::SetParent(parent);
}

void CScorePanel::PerformLayout()
{
	BaseClass::PerformLayout();

	int parentWide = 640;
	int parentTall = 480;

	if (GetParent())
		GetParent()->GetSize(parentWide, parentTall);

	const int wide = vgui2::scheme()->GetProportionalScaledValue(520);
	const int tall = vgui2::scheme()->GetProportionalScaledValue(340);

	SetBounds((parentWide - wide) / 2, (parentTall - tall) / 2, wide, tall);

	const int margin = vgui2::scheme()->GetProportionalScaledValue(16);
	const int labelTall = vgui2::scheme()->GetProportionalScaledValue(30);

	m_pStatusLabel->SetBounds(margin, (tall - labelTall) / 2, wide - margin * 2, labelTall);
}
