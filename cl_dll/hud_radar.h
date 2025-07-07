// hud_radar.h
#pragma once

#define RADAR_RADIUS 64
#define RADAR_RANGE 2048.0f

class CHudRadar : public CHudBase
{
public:
    int Init();
    int VidInit();
    int Draw(float flTime);

private:
    void DrawPlayerBlip(const Vector& delta, bool isFriend, float zDiff);
    Vector WorldToRadar(const Vector& playerOrigin, const Vector& targetOrigin, const Vector& viewAngles);
    void DrawZAxis(const Vector& pos, bool isFriend);
    void DrawDot(int x, int y, int r, int g, int b, int a);
};

extern CHudRadar gRadar;