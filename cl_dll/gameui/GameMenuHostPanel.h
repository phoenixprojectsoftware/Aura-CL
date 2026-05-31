#pragma once

#include <vgui_controls/Panel.h>

#include <vgui/KeyCode.h>
#include <vgui/MouseCode.h>
#include <vgui/IInput.h>
#include <vgui/IInputInternal.h>

class CGameMenuHostPanel : public vgui2::Panel
{
	DECLARE_CLASS_SIMPLE(CGameMenuHostPanel, vgui2::Panel);

public:
	CGameMenuHostPanel(vgui2::VPANEL parent);
	~CGameMenuHostPanel() override;

	void Activate();
	void Deactivate();

	void OnTick() override;
	void PerformLayout() override;
	void Paint() override;

	void OnCursorMoved(int x, int y) override;
	void OnMousePressed(vgui2::MouseCode code) override;
	void OnMouseReleased(vgui2::MouseCode code) override;
	void OnMouseWheeled(int delta) override;

	void OnKeyCodePressed(vgui2::KeyCode code) override;
	void OnKeyCodeReleased(vgui2::KeyCode code) override;
	void OnKeyTyped(wchar_t unichar) override;

private:
	bool UpdateSize();
};
