#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "triangleapi.h"

#include "vgui_TeamFortressViewport.h"

#include "hud_player_waypoints.h"

#include <cmath>

DECLARE_MESSAGE(
	m_PlayerWaypoints,
	PlayerTargets);

extern "C" float Distance(
	const float* v1,
	const float* v2);

static const float UNITS_TO_METERS =
0.0254f;

int CHudPlayerWaypoints::Init()
{
	gHUD.AddHudElem(this);

	HOOK_MESSAGE(PlayerTargets);

	m_iFlags |= HUD_ACTIVE;

	Reset();

	return 1;
}

int CHudPlayerWaypoints::VidInit()
{
	return 1;
}

void CHudPlayerWaypoints::Reset()
{
	m_Targets.clear();
}

int CHudPlayerWaypoints::MsgFunc_PlayerTargets(
	const char* pszName,
	int iSize,
	void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int iCount =
		READ_BYTE();

	if (iCount < 0)
		iCount = 0;

	if (iCount > MAX_PLAYERS)
		iCount = MAX_PLAYERS;

	m_Targets.clear();
	m_Targets.reserve(iCount);

	for (int i = 0;
		i < iCount;
		++i)
	{
		const int iPlayerIndex =
			READ_BYTE();

		if (iPlayerIndex <= 0 ||
			iPlayerIndex > MAX_PLAYERS)
		{
			continue;
		}

		m_Targets.push_back(
			iPlayerIndex);
	}

	return 1;
}

static void ClampPlayerWaypointToScreenEdge(
	float& x,
	float& y,
	float margin)
{
	const float flCentreX =
		ScreenWidth * 0.5f;

	const float flCentreY =
		ScreenHeight * 0.5f;

	float dx = x - flCentreX;
	float dy = y - flCentreY;

	if (fabsf(dx) < 0.001f &&
		fabsf(dy) < 0.001f)
	{
		dx = 0.0f;
		dy = 1.0f;
	}

	const float flHalfWidth =
		flCentreX - margin;

	const float flHalfHeight =
		flCentreY - margin;

	const float flScaleX =
		flHalfWidth / fabsf(dx);

	const float flScaleY =
		flHalfHeight / fabsf(dy);

	const float flScale =
		min(flScaleX, flScaleY);

	x = flCentreX + dx * flScale;
	y = flCentreY + dy * flScale;

	x = max(margin,
		min(x, ScreenWidth - margin));

	y = max(margin,
		min(y, ScreenHeight - margin));
}

bool CHudPlayerWaypoints::ProjectTargetToScreen(
	const float* pOrigin,
	float& x,
	float& y,
	bool& bOffscreen)
{
	if (!pOrigin)
		return false;

	bOffscreen = false;

	const float flMargin = 48.0f;

	float markerOrigin[3];

	markerOrigin[0] = pOrigin[0];
	markerOrigin[1] = pOrigin[1];
	markerOrigin[2] = pOrigin[2] + 36.0f;

	float screen[3];

	const int iBehind =
		gEngfuncs.pTriAPI->WorldToScreen(
			markerOrigin,
			screen);

	x = XPROJECT(screen[0]);
	y = YPROJECT(screen[1]);

	if (iBehind)
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

			x =
				(ScreenWidth * 0.5f) +
				DotProduct(toTarget, right);

			y =
				(ScreenHeight * 0.5f) -
				DotProduct(toTarget, up);
		}
		else
		{
			x = ScreenWidth * 0.5f;
			y = ScreenHeight - flMargin;
		}

		ClampPlayerWaypointToScreenEdge(
			x,
			y,
			flMargin);

		return true;
	}

	if (x < flMargin ||
		x > ScreenWidth - flMargin ||
		y < flMargin ||
		y > ScreenHeight - flMargin)
	{
		bOffscreen = true;

		ClampPlayerWaypointToScreenEdge(
			x,
			y,
			flMargin);
	}

	return true;
}

void CHudPlayerWaypoints::DrawWaypoint(
	int iPlayerIndex,
	float x,
	float y,
	float flDistanceMeters,
	bool bOffscreen)
{
	int r = 255;
	int g = 180;
	int b = 64;

	const char* pszPlayerName =
		g_PlayerInfoList[iPlayerIndex].name;

	if (!pszPlayerName ||
		!pszPlayerName[0])
	{
		pszPlayerName = "PLAYER";
	}

	FillRGBA(
		(int)x - 1,
		(int)y - 16,
		2,
		32,
		r,
		g,
		b,
		220);

	FillRGBA(
		(int)x - 16,
		(int)y - 1,
		32,
		2,
		r,
		g,
		b,
		220);

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

	char szText[128];

	snprintf(
		szText,
		sizeof(szText),
		"%s  %.0fm",
		pszPlayerName,
		flDistanceMeters);

	szText[
		sizeof(szText) - 1] = '\0';

	gHUD.DrawHudStringCentered(
		(int)x,
		(int)y + 22,
		szText,
		r,
		g,
		b);
}

int CHudPlayerWaypoints::Draw(
	float flTime)
{
	if (m_Targets.empty())
		return 1;

	cl_entity_t* pLocalPlayer =
		gEngfuncs.GetLocalPlayer();

	if (!pLocalPlayer)
		return 1;

	if (gViewPort)
	{
		gViewPort->GetAllPlayersInfo();
	}

	for (std::vector<int>::const_iterator it =
		m_Targets.begin();
		it != m_Targets.end();
		++it)
	{
		const int iPlayerIndex = *it;

		cl_entity_t* pTarget =
			gEngfuncs.GetEntityByIndex(
				iPlayerIndex);

		if (!pTarget ||
			!pTarget->model)
		{
			continue;
		}

		if (pTarget == pLocalPlayer)
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

		const float flDistanceUnits =
			Distance(
				pLocalPlayer->origin,
				pTarget->origin);

		const float flDistanceMeters =
			flDistanceUnits *
			UNITS_TO_METERS;

		DrawWaypoint(
			iPlayerIndex,
			x,
			y,
			flDistanceMeters,
			bOffscreen);
	}

	return 1;
}
