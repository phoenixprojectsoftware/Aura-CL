#pragma once

class CHudWatermark : public CHudBase
{
	float draw_until;
	bool refresh_draw_until;
	bool update_is_available;
	bool hasMusicPlayed;
	bool m_bDrawInfo;

public:
	virtual int Init();
	virtual int VidInit();
	virtual int Draw(float time);
	bool IsBetaApp();
	bool Neptune();

	std::string pszGamemode;
};
