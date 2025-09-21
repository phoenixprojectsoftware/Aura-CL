/****
*
* Copyright (c) 2021-2025 The Phoenix Project Software. Some Rights Reserved.
*
* AURA
*
* Steam Leaderboards - NOT for Excession right now
*
*
****/

#ifndef _HALO

#pragma once

#include <vgui_controls/Frame.h>
#include <vgui_controls/ListPanel.h>
#include "../../leaderboard_integration.h"

class CLeaderboardPanel : public vgui2::Frame
{
	DECLARE_CLASS_SIMPLE(CLeaderboardPanel, vgui2::Frame);

public:
	CLeaderboardPanel(vgui2::VPANEL parent);
	~CLeaderboardPanel() override {}

	void Activate();
	void RefreshLeaderboard();

private:
	MESSAGE_FUNC(OnClose, "Close");

	void OnDownloadScores(LeaderboardScoresDownloaded_t* pResult, bool bIOFailure);

	vgui2::ListPanel* m_pList;

	CCallResult<CLeaderboardPanel, LeaderboardScoresDownloaded_t> m_SteamCallResultDownloadScores;
};
#endif
