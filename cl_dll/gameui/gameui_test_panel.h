/****
*
* Copyright © 2021-2025 The Phoenix Project Software. Some Rights Reserved.
*
* AURA
*
* gameui_test_panel - Test VGUI2 panel.         Based upon
* BugfixedHL-Rebased. Used as per the license agreement.
*
*
****/

#ifndef GAMEUI_TEST_PANEL_H
#define GAMEUI_TEST_PANEL_H
#include <vgui_controls/Frame.h>
#include <vgui_controls/RichText.h>

class CGameUITestPanel : public vgui2::Frame
{
	DECLARE_CLASS_SIMPLE(CGameUITestPanel, vgui2::Frame);

public:
	CGameUITestPanel(vgui2::Panel* pParent);

	virtual void PerformLayout() override;
	virtual void Activate() override;

private:
	vgui2::RichText* m_pText;
};

#endif // GAMEUI_TEST_PANEL_H
