#ifndef VGUI_SCORE_PANEL_H
#define VGUI_SCORE_PANEL_H

#include <vgui_controls/Frame.h>

#include "IViewportPanel.h"

namespace vgui2
{
	class SectionedListPanel;
}

class CScorePanel : public vgui2::Frame, public IViewportPanel
{
	DECLARE_CLASS_SIMPLE(CScorePanel, vgui2::Frame);

public:
	explicit CScorePanel(vgui2::Panel* parent);

	// vgui2::Panel overrides.
	void OnThink() override;

	// IViewportPanel implementation.
	const char* GetName() override;
	void Reset() override;
	void ShowPanel(bool state) override;
	vgui2::VPANEL GetVPanel() override;
	bool IsVisible() override;
	void SetParent(vgui2::VPANEL parent) override;

protected:
	void PerformLayout() override;

private:
	enum
	{
		SECTION_PLAYERS = 1,
		SECTION_SPECTATORS = 2
	};

	void CreateSections();
	void UpdatePlayerList();

	vgui2::SectionedListPanel* m_pPlayerList;

	double m_flNextUpdateTime;
};

#endif // VGUI_SCORE_PANEL_H
