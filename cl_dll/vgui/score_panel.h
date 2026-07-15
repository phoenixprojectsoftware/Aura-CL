#ifndef VGUI_SCORE_PANEL_H
#define VGUI_SCORE_PANEL_H

#include <vgui_controls/Frame.h>

#include "IViewportPanel.h"

class CSteamAvatarImage;

namespace vgui2
{
	class ImageList;
	class Label;
	class Menu;
	class SectionedListPanel;
}

enum
{
	SCOREBOARD_MAX_PLAYERS = 32
};

class CScorePanel : public vgui2::Frame, public IViewportPanel
{
	DECLARE_CLASS_SIMPLE(CScorePanel, vgui2::Frame);

public:
	explicit CScorePanel(vgui2::Panel* parent);
	~CScorePanel() override;

	// vgui2::Panel overrides.
	void OnThink() override;

	// IViewportPanel implementation.
	const char* GetName() override;
	void Reset() override;
	void ShowPanel(bool state) override;
	vgui2::VPANEL GetVPanel() override;
	bool IsVisible() override;
	void SetParent(vgui2::VPANEL parent) override;

	MESSAGE_FUNC_INT(OnItemContextMenu, "ItemContextMenu", itemID);

	void EnableMousePointer(bool enable);

	bool IsMousePointerEnabled() const
	{
		return m_bMousePointerEnabled;
	}

protected:
	void PerformLayout() override;

private:
	enum BoardSectionID
	{
		SECTION_PLAYERS = 1,

		SECTION_TEAM_BASE = 100,

		SECTION_SPECTATORS = 1000
	};

	void CreateSections();
	void AddPlayerColumns(int sectionID, const char* sectionName, bool showStatHeadings);
	void ApplyTeamSectionColor(int sectionID, int teamNumber);
	void UpdateHeader();
	void UpdatePlayerList();
	void UpdatePlayerAvatar(int clientIndex, std::uint64_t steamID);

	int GetSectionForPlayer(int clientIndex) const;
	int FindTeamIndex(const char* teamName) const;

	static bool ScoreSort(vgui2::SectionedListPanel* list, int itemID1, int itemID2);

	void OpenPlayerMenu(int itemID);
	void ToggleSelectedPlayerMute();

	void OnCommand(const char* command) override;

	vgui2::Label* m_pMapLabel;
	vgui2::Label* m_pPlayerCountLabel;
	vgui2::SectionedListPanel* m_pPlayerList;
	vgui2::Menu* m_pPlayerMenu;

	bool m_bMousePointerEnabled;
	int m_iSelectedClient;
	int m_iMuteMenuItem;

	vgui2::ImageList* m_pImageList;
	CSteamAvatarImage* m_pAvatars[SCOREBOARD_MAX_PLAYERS + 1];

	double m_flNextUpdateTime;
};

#endif // VGUI_SCORE_PANEL_H
