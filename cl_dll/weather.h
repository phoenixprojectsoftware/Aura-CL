//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

//
// weather.cpp
//
// Implementation of weather effects into goldsrc
// Code by Overfloater
//

#if !defined ( WEATHER_H )
#define WEATHER_H
#if defined( _WIN32 )
#pragma once
#include "../common/winsani_in.h"
#include <Windows.h>
#include "../common/winsani_out.h"
#endif

#include "gl/gl.h"
#include "glext.h"
#include "com_model.h"

#define MAX_WEATHER_PARTICLES		8192

#define SURF_DRAWTILED				0x20
#define	SURF_DRAWSKY				4

#define SPLASH_FRAMES_NUM			14
#define SPLASH_FRAME_X				64
#define SPLASH_FRAME_Y				64

#define SNOW_GRAVITY		0.025f
#define RAIN_GRAVITY		0.6f

#define PARTICLE_LIGHTCHECK_DIST	8

enum weather_e
{
	WEATHER_OFF = 0,
	WEATHER_SNOW,
	WEATHER_RAIN
};

struct w_particle_t
{
	double life;
	double spawntime;

	float gravity;
	float alpha;
	float fadeout;
	float fadein;
	float scale;
	float framerate;
	float fps;
	int frame;
	
	bool wind;

	vec3_t velocity;
	vec3_t origin;
	vec3_t normal;
	vec3_t color;
	vec3_t last_check;

	w_particle_t *pnext;
	w_particle_t *pprev;

	w_particle_t *pdrawchain;
};

struct windgust_t
{
	float starttime;
	float duration;
	float windx;
	float windy;

	float curx;
	float cury;

	windgust_t* pnext;
	windgust_t* pprev;
};

extern void Sys_Error ( char *fmt, ... );

/*
====================
CWeather

====================
*/
class CWeather
{
public:
	void Init( void );
	void VidInit( void );
	void Clear( void );
	
	void Think( void );
	void DrawParticles( void );

	int MsgFunc_Weather( const char *pszName, int iSize, void *pBuf );

private:
	void CreateParticle( void );
	void CreateImpactParticle( vec3_t& origin, vec3_t& normal, float scale, int frame );
	bool UpdateParticle( w_particle_t* particle, model_t* pmodel );

	w_particle_t* AllocParticle( w_particle_t** pchain );
	void FreeParticle( w_particle_t** pchain, w_particle_t* pparticle );

	void DrawTiledParticle( w_particle_t* pparticle, mspriteframe_t *pframe );
	void DrawTracerParticle( w_particle_t* pparticle, mspriteframe_t *pframe );
	mspriteframe_t* GetSpriteFrame ( model_t *sprite, int frame );

	void ReleaseDrawChains( void );
	void AllocDrawChains( void );
	
	int DetermineSurfaceStructSize( void );

private:
	w_particle_t* m_pParticleChain;
	w_particle_t* m_pImpactChain;

	w_particle_t* m_pFreeParticles;
	w_particle_t m_particlesArray[MAX_WEATHER_PARTICLES];

	windgust_t* m_pWindChain;

	w_particle_t** m_pParticleDrawChains;
	w_particle_t** m_pImpactDrawChains;

private:
	float m_windX;
	float m_windY;
	float m_minWindX;
	float m_minWindY;

	float m_nextWindTime;

	float m_gravity;
	float m_spawnHeight;
	float m_particleFrequency;

	int m_weatherType;
	int m_frameCount;

	double m_spawnTime;
	double m_time;
	double m_oldTime;
	double m_frametime;

	vec3_t m_renderOrigin;
	vec3_t m_viewUp;
	vec3_t m_viewRight;
	vec3_t m_viewForward;

	unsigned long m_numSpawned;
	unsigned long m_numParticles;

	int m_surfStructSize;

private:
	model_t* m_pParticleSprite;
	model_t* m_pImpactSprite;

	PFNGLACTIVETEXTUREPROC glActiveTexture;
};

extern CWeather gWeather;
#endif // WEATHER_H