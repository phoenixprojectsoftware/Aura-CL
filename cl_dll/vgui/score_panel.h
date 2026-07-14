#pragma once

#include <vgui_controls/Frame.h>

#include "IViewportPanel.h"

namespace vgui2
{
	class Label;
}

class CScorePanel : public vgui2::Frame, public IViewportPanel
{
	DECLARE_CLASS_SIMPLE(CScorePanel, vgui2::Frame);

public:
	explicit CScorePanel(vgui2::Panel* parent);

	const char* GetName() override;
	void Reset() override;
	void ShowPanel(bool state) override;
	vgui2::VPANEL GetVPanel() override;
	bool IsVisible() override;
	void SetParent(vgui2::VPANEL parent) override;

protected:
	void PerformLayout() override;

private:
	vgui2::Label* m_pStatusLabel;
};
