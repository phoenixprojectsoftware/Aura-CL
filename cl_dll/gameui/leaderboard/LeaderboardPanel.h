#pragma once

#ifndef _HALO

#include <vgui_controls/Frame.h>
#include <vgui_controls/ListPanel.h>

#include <steamworks/steam_api.h>

class CLeaderboardPanel : public vgui2::Frame
{
	DECLARE_CLASS_SIMPLE(CLeaderboardPanel, vgui2::Frame);

public:
	CLeaderboardPanel(vgui2::VPANEL parent);

	void Activate() override;
	void OnClose() override;

private:
	void RefreshStats();
	void PopulateStatsTable();

	void OnUserStatsReceived(UserStatsReceived_t* pResult, bool bIOFailure);

private:
	vgui2::ListPanel* m_pList;

	bool m_bStatsReady;

	CCallResult<CLeaderboardPanel, UserStatsReceived_t> m_CallResultUserStatsReceived;
};

#endif
