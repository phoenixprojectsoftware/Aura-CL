#ifndef VGUI_CLIENT_VIEWPORT_H
#define VGUI_CLIENT_VIEWPORT_H
#include <vgui_controls/EditablePanel.h>

class CScorePanel;

class CClientViewport : public vgui2::EditablePanel {
	DECLARE_CLASS_SIMPLE(CClientViewport, vgui2::EditablePanel);

public:
	CClientViewport();

	void Start();
	void VidInit();

	void ReloadScheme(const char* fromFile);

	void ActivateClientUI();
	void HideClientUI();

	bool IsScoreBoardVisible() const;
	void ShowScoreBoard();
	void HideScoreBoard();

	CScorePanel* GetScoreboard() const
	{
		return m_pScorePanel;
	}

protected:
	void PerformLayout() override;

private:
	CScorePanel* m_pScorePanel;
};

extern CClientViewport* g_pViewport;

#endif // VGUI_CLIENT_VIEWPORT_H
