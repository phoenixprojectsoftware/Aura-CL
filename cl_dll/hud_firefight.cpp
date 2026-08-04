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

#include "hud.h"
#include "cl_util.h"
#include "triangleapi.h"
#include "cl_gametype.h"
#include "parsemsg.h"

#include "hud_firefight.h"

#include <math.h>
#include <string.h>

extern "C" float Distance(const float* v1, const float* v2);

#define UNITS_TO_METERS 0.0254f

DECLARE_MESSAGE(m_Firefight, FFTargets);

int CHudFirefight::Init()
{
	gHUD.AddHudElem(this);

	m_iFlags |= HUD_ACTIVE;

	HOOK_MESSAGE(FFTargets);

	Reset();

	return 1;
}

int CHudFirefight::VidInit()
{
	return 1;
}

void CHudFirefight::Reset()
{
	m_iTargetCount = 0;

	for (int i = 0; i < 3; ++i)
	{
		m_iTargetEntities[i] = 0;
	}
}

void CHudFirefight::SetTargets(
	int iCount,
	const int* pEntityIndices)
{
	Reset();

	if (!pEntityIndices ||
		iCount <= 0)
	{
		return;
	}

	m_iTargetCount =
		min(iCount, 3);

	for (int i = 0;
		i < m_iTargetCount;
		++i)
	{
		m_iTargetEntities[i] =
			pEntityIndices[i];
	}
}

static void ClampFirefightWaypointToScreenEdge(
	float& x,
	float& y,
	float margin)
{
	const float cx =
		ScreenWidth * 0.5f;

	const float cy =
		ScreenHeight * 0.5f;

	float dx = x - cx;
	float dy = y - cy;

	if (fabsf(dx) < 0.001f &&
		fabsf(dy) < 0.001f)
	{
		dx = 0.0f;
		dy = 1.0f;
	}

	const float halfW =
		cx - margin;

	const float halfH =
		cy - margin;

	const float scaleX =
		halfW / fabsf(dx);

	const float scaleY =
		halfH / fabsf(dy);

	const float scale =
		min(scaleX, scaleY);

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

bool CHudFirefight::ProjectTargetToScreen(
	const float* pOrigin,
	float& x,
	float& y,
	bool& bOffscreen)
{
	bOffscreen = false;

	if (!pOrigin)
		return false;

	const float margin = 48.0f;

	float markerOrigin[3];

	markerOrigin[0] =
		pOrigin[0];

	markerOrigin[1] =
		pOrigin[1];

	markerOrigin[2] =
		pOrigin[2] + 48.0f;

	float screen[3];

	const int behind =
		gEngfuncs.pTriAPI->WorldToScreen(
			markerOrigin,
			screen);

	x = XPROJECT(screen[0]);
	y = YPROJECT(screen[1]);

	if (behind)
	{
		bOffscreen = true;

		cl_entity_t* pLocalPlayer =
			gEngfuncs.GetLocalPlayer();

		if (pLocalPlayer)
		{
			float viewAngles[3];
			gEngfuncs.GetViewAngles(
				viewAngles);

			float forward[3];
			float right[3];
			float up[3];

			AngleVectors(
				viewAngles,
				forward,
				right,
				up);

			float toTarget[3];

			toTarget[0] =
				markerOrigin[0] -
				pLocalPlayer->origin[0];

			toTarget[1] =
				markerOrigin[1] -
				pLocalPlayer->origin[1];

			toTarget[2] =
				markerOrigin[2] -
				pLocalPlayer->origin[2];

			const float side =
				DotProduct(
					toTarget,
					right);

			const float vertical =
				DotProduct(
					toTarget,
					up);

			x =
				(ScreenWidth * 0.5f) +
				side;

			y =
				(ScreenHeight * 0.5f) -
				vertical;
		}
		else
		{
			x =
				ScreenWidth * 0.5f;

			y =
				ScreenHeight - margin;
		}

		ClampFirefightWaypointToScreenEdge(
			x,
			y,
			margin);

		return true;
	}

	if (x < margin ||
		x > ScreenWidth - margin ||
		y < margin ||
		y > ScreenHeight - margin)
	{
		bOffscreen = true;

		ClampFirefightWaypointToScreenEdge(
			x,
			y,
			margin);
	}

	return true;
}

void CHudFirefight::DrawWaypoint(
	float x,
	float y,
	float flDistanceMeters,
	bool bOffscreen,
	int iTargetNumber)
{
	int r = 255;
	int g = 80;
	int b = 80;

	FillRGBA(
		(int)x - 1,
		(int)y - 18,
		2,
		8,
		r,
		g,
		b,
		220);

	FillRGBA(
		(int)x - 1,
		(int)y + 10,
		2,
		8,
		r,
		g,
		b,
		220);

	FillRGBA(
		(int)x - 18,
		(int)y - 1,
		8,
		2,
		r,
		g,
		b,
		220);

	FillRGBA(
		(int)x + 10,
		(int)y - 1,
		8,
		2,
		r,
		g,
		b,
		220);

	FillRGBA(
		(int)x - 2,
		(int)y - 2,
		4,
		4,
		r,
		g,
		b,
		230);

	if (bOffscreen)
	{
		FillRGBA(
			(int)x - 5,
			(int)y - 5,
			10,
			2,
			255,
			255,
			255,
			200);

		FillRGBA(
			(int)x - 5,
			(int)y + 3,
			10,
			2,
			255,
			255,
			255,
			200);
	}

	char szText[64];

	if (m_iTargetCount > 1)
	{
		snprintf(
			szText,
			sizeof(szText),
			"ENEMY %d  %.0fm",
			iTargetNumber + 1,
			flDistanceMeters);
	}
	else
	{
		snprintf(
			szText,
			sizeof(szText),
			"LAST ENEMY  %.0fm",
			flDistanceMeters);
	}

	szText[
		sizeof(szText) - 1] = '\0';

	gHUD.DrawHudStringCentered(
		(int)x,
		(int)y + 24,
		szText,
		r,
		g,
		b);
}

int CHudFirefight::Draw(float flTime)
{
	const int iGameType =
		gHUD.GetGameType();

	if (iGameType != FIREFIGHT &&
		iGameType != FIESTAFIGHT)
	{
		return 1;
	}

	if (m_iTargetCount <= 0)
		return 1;

	cl_entity_t* pLocalPlayer =
		gEngfuncs.GetLocalPlayer();

	if (!pLocalPlayer)
		return 1;

	for (int i = 0;
		i < m_iTargetCount;
		++i)
	{
		const int iEntityIndex =
			m_iTargetEntities[i];

		if (iEntityIndex <= 0)
			continue;

		cl_entity_t* pTarget =
			gEngfuncs.GetEntityByIndex(
				iEntityIndex);

		if (!pTarget)
			continue;

		// A stale entity slot may temporarily have no model.
		if (!pTarget->model)
			continue;

		float x = 0.0f;
		float y = 0.0f;
		bool bOffscreen = false;

		if (!ProjectTargetToScreen(
			pTarget->origin,
			x,
			y,
			bOffscreen))
		{
			continue;
		}

		const float flUnits =
			Distance(
				pLocalPlayer->origin,
				pTarget->origin);

		const float flMeters =
			flUnits *
			UNITS_TO_METERS;

		DrawWaypoint(
			x,
			y,
			flMeters,
			bOffscreen,
			i);
	}

	return 1;
}

int CHudFirefight::MsgFunc_FFTargets(
	const char* pszName,
	int iSize,
	void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int iCount =
		READ_BYTE();

	if (iCount < 0)
		iCount = 0;

	if (iCount > 3)
		iCount = 3;

	int iTargets[3] =
	{
		0,
		0,
		0
	};

	for (int i = 0;
		i < iCount;
		++i)
	{
		iTargets[i] =
			READ_SHORT();
	}

	SetTargets(
		iCount,
		iTargets);

	return 1;
}
