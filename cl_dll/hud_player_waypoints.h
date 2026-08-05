#pragma once

#include <vector>

class CHudPlayerWaypoints : public CHudBase
{
public:
	int Init() override;
	int VidInit() override;
	int Draw(float flTime) override;
	void Reset() override;

	int MsgFunc_PlayerTargets(const char* pszName, int iSize, void* pbuf);

private:
	bool ProjectTargetToScreen(const float* pOrigin, float& x, float& y, bool& bOffscreen);
	void DrawWaypoint(int iPlayerIndex, float x, float y, float flDistanceMeters, bool bOffscreen);

private:
	std::vector<int> m_Targets;
};
