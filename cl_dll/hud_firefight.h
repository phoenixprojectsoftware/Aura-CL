/****
*
* Copyright (c) 2021-2026 The Phoenix Project Software. Some Rights Reserved.
*
* AURA
*
* FIREFIGHT HUD
*
*
****/

#pragma once

class CHudFirefight : public CHudBase
{
public:
	int Init() override;
	int VidInit() override;
	int Draw(float flTime) override;
	void Reset() override;

	void SetTargets(int iCount, const int* pEntityIndices);

	int MsgFunc_FFTargets(const char* pszName, int iSize, void* pbuf);

private:
	bool ProjectTargetToScreen(const float* pOrigin, float& x, float& y, bool& bOffscreen);
	void DrawWaypoint(float x, float y, float flDistanceMeters, bool bOffscreen, int iTargetNumber);

private:
	int m_iTargetCount = 0;
	int m_iTargetEntities[3] =
	{
		0,
		0,
		0
	};
};
