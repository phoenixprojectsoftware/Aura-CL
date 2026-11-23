/****
*
* Copyright (c) 2021-2026 The Phoenix Project Software. Some Rights Reserved.
*
* AURA
*
* KING OF THE HILL HUD
*
*
****/

#pragma once

class CHudKOTH : public CHudBase
{
public:
	int Init() override;
	int VidInit() override;
	int Draw(float flTime) override;
	void Reset() override;

	void SetHill(const char* pszName, const float* origin);

private:
	bool ProjectHillToScreen(float& x, float& y, bool& bOffscreen);
	void DrawWaypoint(float x, float y, float distanceMeters, bool bOffscreen);
	void DrawHillName();

private:
	bool m_bHasHill = false;
	char m_szHillName[64] = {};
	vec3_t m_vHillOrigin = {};
};
