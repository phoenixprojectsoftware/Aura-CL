// hud_radar.cpp
#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "hud_radar.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef DEG2RAD
    #define DEG2RAD( a ) ( (a) * (M_PI / 180.0f) )
#endif
CHudRadar gRadar;

int CHudRadar::Init()
{
    gHUD.AddHudElem(static_cast<CHudBase*>(this));
    m_iFlags |= HUD_ACTIVE;
    return 1;
}

int CHudRadar::VidInit()
{
    return 1;
}

int CHudRadar::Draw(float flTime)
{
   /* if (gEngfuncs.GetMaxClients() <= 1)
        return 1;*/

    gEngfuncs.Con_Printf("Radar draw called\n");

    int x = ScreenWidth - RADAR_RADIUS - 20;
    int y = RADAR_RADIUS + 20;

    FillRGBA(x - RADAR_RADIUS, y - RADAR_RADIUS, RADAR_RADIUS * 2, RADAR_RADIUS * 2, 0, 0, 0, 128);

    Vector localOrigin = gHUD.m_vecOrigin;
    Vector localAngles = gHUD.m_vecAngles;

    for (int i = 1; i <= gEngfuncs.GetMaxClients(); ++i)
    {
        if (i == gEngfuncs.GetLocalPlayer()->index)
            continue;

        if (!g_PlayerInfoList[i].name || !g_PlayerInfoList[i].name[0])
            continue;

        cl_entity_t* ent = gEngfuncs.GetEntityByIndex(i);
        if (!ent)
            continue;

        Vector radarPos = WorldToRadar(localOrigin, ent->origin, localAngles);
        float zDiff = ent->origin.z - localOrigin.z;

        bool isFriend = (g_PlayerExtraInfo[i].teamnumber == g_PlayerExtraInfo[gEngfuncs.GetLocalPlayer()->index].teamnumber);

        DrawZAxis(radarPos, isFriend);
    }

    return 1;
}

Vector CHudRadar::WorldToRadar(const Vector& playerOrigin, const Vector& targetOrigin, const Vector& viewAngles)
{
    Vector2D diff = targetOrigin.Make2D() - playerOrigin.Make2D();
    float yaw = viewAngles.y;
    float angle = atan2(diff.y, diff.x);
    float dist = min(diff.Length(), RADAR_RANGE);
    float flOffset = DEG2RAD(yaw) - angle;

    float radius = (dist / RADAR_RANGE) * RADAR_RADIUS;

    return Vector(radius * cos(flOffset), -radius * sin(flOffset), targetOrigin.z - playerOrigin.z);
}

void CHudRadar::DrawZAxis(const Vector& pos, bool isFriend)
{
    const float zThresh = 64.0f;
    int r = isFriend ? 255 : 255;
    int g = isFriend ? 255 : 0;
    int b = isFriend ? 0 : 0;

    int cx = ScreenWidth - RADAR_RADIUS - 20 + (int)pos.x;
    int cy = RADAR_RADIUS + 20 + (int)pos.y;

    if (pos.z > zThresh)
    {
        DrawDot(cx, cy - 2, r, g, b, 255);
    }
    else if (pos.z < -zThresh)
    {
        DrawDot(cx, cy + 2, r, g, b, 255);
    }
    else
    {
        DrawDot(cx, cy, r, g, b, 255);
    }
}

void CHudRadar::DrawDot(int x, int y, int r, int g, int b, int a)
{
    FillRGBA(x - 2, y - 2, 4, 4, r, g, b, a);
}
