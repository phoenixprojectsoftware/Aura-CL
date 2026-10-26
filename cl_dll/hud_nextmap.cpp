#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

DECLARE_MESSAGE(m_NextMap, Nextmap);

int CHudNextMap::Init()
{
	HOOK_MESSAGE(Nextmap);

	m_iFlags = 0;

	next_map[0] = '\0';
	next_mode[0] = '\0';

	gHUD.AddHudElem(this);
	return 0;
}

int CHudNextMap::VidInit()
{
	m_iFlags &= ~HUD_ACTIVE;

	return 1;
}

int CHudNextMap::Draw(float time)
{
	if (gHUD.m_flTime >= draw_until) {
		m_iFlags &= ~HUD_ACTIVE;
		return 0;
	}

	int r, g, b;
	UnpackRGB(r, g, b, gHUD.m_iDefaultHUDColor);

	char szMap[96];
	snprintf(szMap, sizeof(szMap), "The next map is %s", next_map);

	gHUD.DrawHudStringCentered(ScreenWidth / 2, gHUD.m_scrinfo.iCharHeight * 5, szMap, r, g, b);

	if (next_mode[0])
	{
		char szMode[96];

		snprintf(szMode, sizeof(szMode), "on %s", next_mode);

		gHUD.DrawHudStringCentered(ScreenWidth / 2, gHUD.m_scrinfo.iCharHeight * 6, szMode, 0, 255, 0);
	}

	return 0;
}

int CHudNextMap::MsgFunc_Nextmap(const char* name, int size, void* buf)
{
	BEGIN_READ(buf, size);

	const char* pszMap = READ_STRING();
	strncpy(next_map, pszMap ? pszMap : "", ARRAYSIZE(next_map) - 1);
	next_map[ARRAYSIZE(next_map) - 1] = '\0';

	const char* pszMode = READ_STRING();
	strncpy(next_mode, pszMode ? pszMode : "", ARRAYSIZE(next_mode) - 1);
	next_mode[ARRAYSIZE(next_mode) - 1] = '\0';

	draw_until = gHUD.m_flTime + 10.0f;

	m_iFlags |= HUD_ACTIVE;

	return 1;
}
