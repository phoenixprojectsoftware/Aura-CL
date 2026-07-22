#pragma once
#include <cstdint>
#include "versioninfo.h"

class CHudSettings : public CHudBase
{
	float draw_until;

	char gamemode[32];

	bool match_is_on;
	int8_t time_limit;
	int8_t frag_limit;
	bool friendly_fire;
	bool weapon_stay;

	char ag_version[8];
	char wallgauss[8];
	char headshot[8];
	char blast_radius[8];
	bool m_bOldBadVersion; // is the server's game version too old for this client?

	cvar_t* hud_settings;

public:
	virtual int Init();
	virtual int VidInit();
	virtual int Draw(float time);

	const char* GetGamemode() const;

	int MsgFunc_Settings(const char* name, int size, void* buf);

	bool IsOldServer() const { return m_bOldBadVersion; }
};
