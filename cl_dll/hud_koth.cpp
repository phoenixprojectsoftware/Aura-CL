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

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "triangleapi.h"
#include "cl_gametype.h"

#include "hud_koth.h"

#include <math.h>
#include <string.h>

extern "C" float Distance(const float* v1, const float* v2);

#define UNITS_TO_METERS 0.0254f

int CHudKOTH::Init()
{
	gHUD.AddHudElem(this);

	m_iFlags |= HUD_ACTIVE;

	Reset();

	return 1;
}

int CHudKOTH::VidInit()
{
	return 1;
}

void CHudKOTH::Reset()
{
#if 0
	m_bHasHill = false;
	m_szHillName[0] = '\0';
	Legacy_VectorClear(m_vHillOrigin);
#endif
}

void CHudKOTH::SetHill(const char* pszName, const float* origin)
{
	if (!pszName || !pszName[0] || !origin)
	{
		Reset();
		return;
	}

	strncpy(m_szHillName, pszName, sizeof(m_szHillName) - 1);
	m_szHillName[sizeof(m_szHillName) - 1] = '\0';

	VectorCopy(origin, m_vHillOrigin);

	m_bHasHill = true;
}

void CHudKOTH::DrawHillName()
{
	if (!m_bHasHill)
		return;

	int r, g, b;
	UnpackRGB(r, g, b, gHUD.m_iDefaultHUDColor);

	char text[128];
	snprintf(text, sizeof(text), "CURRENT HILL: %s", m_szHillName);

	const int y = gHUD.m_scrinfo.iCharHeight * 3;

	gHUD.DrawHudStringCentered(ScreenWidth / 2, y, text, r, g, b);
}

static void ClampWaypointToScreenEdge(float& x, float& y, float margin)
{
	const float cx = ScreenWidth * 0.5f;
	const float cy = ScreenHeight * 0.5f;

	float dx = x - cx;
	float dy = y - cy;

	if (fabsf(dx) < 0.001f && fabsf(dy) < 0.001f)
	{
		dx = 0.0f;
		dy = 1.0f;
	}

	const float halfW = cx - margin;
	const float halfH = cy - margin;

	float scaleX = halfW / fabsf(dx);
	float scaleY = halfH / fabsf(dy);
	float scale = min(scaleX, scaleY);

	x = cx + dx * scale;
	y = cy + dy * scale;

	if (x < margin)
		x = margin;
	else if (x > ScreenWidth - margin)
		x = ScreenWidth - margin;

	if (y < margin)
		y = margin;
	else if (y > ScreenHeight - margin)
		y = ScreenHeight - margin;
}

bool CHudKOTH::ProjectHillToScreen(float& x, float& y, bool& bOffscreen)
{
	bOffscreen = false;

	if (!m_bHasHill)
		return false;

	const float margin = 48.0f;

	float markerOrigin[3];
	markerOrigin[0] = m_vHillOrigin[0];
	markerOrigin[1] = m_vHillOrigin[1];
	markerOrigin[2] = m_vHillOrigin[2] + 48.0f;

	float screen[3];
	const int behind = gEngfuncs.pTriAPI->WorldToScreen(markerOrigin, screen);

	x = XPROJECT(screen[0]);
	y = YPROJECT(screen[1]);

	if (behind)
	{
		bOffscreen = true;

		cl_entity_t* localPlayer = gEngfuncs.GetLocalPlayer();

		if (localPlayer)
		{
			float viewAngles[3];
			gEngfuncs.GetViewAngles(viewAngles);

			float forward[3], right[3], up[3];
			AngleVectors(viewAngles, forward, right, up);

			float toHill[3];
			toHill[0] = markerOrigin[0] - localPlayer->origin[0];
			toHill[1] = markerOrigin[1] - localPlayer->origin[1];
			toHill[2] = markerOrigin[2] - localPlayer->origin[2];

			const float side = DotProduct(toHill, right);
			const float vertical = DotProduct(toHill, up);

			x = (ScreenWidth * 0.5f) + side;
			y = (ScreenHeight * 0.5f) - vertical;
		}
		else
		{
			x = ScreenWidth * 0.5f;
			y = ScreenHeight - margin;
		}

		ClampWaypointToScreenEdge(x, y, margin);
		return true;
	}

	if (x < margin || x > ScreenWidth - margin || y < margin || y > ScreenHeight - margin)
	{
		bOffscreen = true;
		ClampWaypointToScreenEdge(x, y, margin);
		return true;
	}

	return true;
}

void CHudKOTH::DrawWaypoint(float x, float y, float distanceMeters, bool bOffscreen)
{
	int r, g, b;
	UnpackRGB(r, g, b, gHUD.m_iDefaultHUDColor);

	// waypoint diamond
	FillRGBA((int)x - 1, (int)y - 18, 2, 8, r, g, b, 220);
	FillRGBA((int)x - 1, (int)y + 10, 2, 8, r, g, b, 220);
	FillRGBA((int)x - 18, (int)y - 1, 8, 2, r, g, b, 220);
	FillRGBA((int)x + 10, (int)y - 1, 8, 2, r, g, b, 220);

	// centre dot
	FillRGBA((int)x - 2, (int)y - 2, 4, 4, r, g, b, 230);

	if (bOffscreen)
	{
		// edge
		FillRGBA((int)x - 5, (int)y - 5, 10, 2, 255, 255, 255, 200);
		FillRGBA((int)x - 5, (int)y + 3, 10, 2, 255, 255, 255, 200);
	}

	char text[128];
	snprintf(text, sizeof(text), "%s  %.0fm", m_szHillName, distanceMeters);

	gHUD.DrawHudStringCentered((int)x, (int)y + 24, text, r, g, b);
}

int CHudKOTH::Draw(float flTime)
{
	if (g_iGameType != GameType::KING)
		return 1;

	if (!m_bHasHill)
		return 1;

	DrawHillName();

	float x = 0.0f;
	float y = 0.0f;
	bool bOffscreen = false;

	if (!ProjectHillToScreen(x, y, bOffscreen))
		return 1;

	cl_entity_t* localPlayer = gEngfuncs.GetLocalPlayer();

	if (!localPlayer)
		return 1;

	const float units = Distance(localPlayer->origin, m_vHillOrigin);
	const float meters = units * UNITS_TO_METERS;

	DrawWaypoint(x, y, meters, bOffscreen);

	return 1;
}
