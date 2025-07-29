/***+
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
// weather.cpp
//
// Implementation of weather effects into goldsrc
// Code by Overfloater
//

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "pm_shared.h"

#include <string.h>
#include <stdio.h>

#include "pmtrace.h"
#include "r_efx.h"
#include "event_api.h"
#include "event_args.h"
#include "in_defs.h"
#include "pm_defs.h"
#include "weather.h"

#include "studio_util.h"
#include "r_studioint.h"
#include "triangleapi.h"

#include "studio.h"
#include "StudioModelRenderer.h"

#define IMPACT_LIFE			1.0f
#define IMPACT_FADE_TIME	0.5f

#define SYSTEM_SIZE			1024
#define MAX_HEIGHT			1024

#define PARTICLE_FADEIN		1.0f
#define PARTICLE_MINLIGHT	0.05f

extern int RecursiveLightPoint( model_t* pmodel, mnode_t *node, const vec3_t &start, const vec3_t &end, vec3_t &color, int surfstructsize );
extern engine_studio_api_t IEngineStudio;

// Class declaration
CWeather gWeather;

int __MsgFunc_Weather( const char *pszName, int iSize, void *pbuf )
{
	return gWeather.MsgFunc_Weather(pszName, iSize, pbuf );
}

/*
====================
Init

====================
*/
void CWeather::Init( void )
{
	glActiveTexture	= (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");

	HOOK_MESSAGE( Weather );
}

/*
====================
VidInit

====================
*/
void CWeather::VidInit( void )
{
	Clear();
}

/*
====================
Clear

====================
*/
void CWeather::Clear( void )
{
	// Clear previous particles
	memset(m_particlesArray, 0, sizeof(m_particlesArray));

	// Link up the particles
	for(int i = 1; i < MAX_WEATHER_PARTICLES; i++)
	{
		m_particlesArray[i-1].pnext = &m_particlesArray[i];
		m_particlesArray[i].pprev = &m_particlesArray[i-1];
	}

	// Set free particles pointer
	m_pFreeParticles = m_particlesArray;
	m_pParticleChain = NULL;
	m_pImpactChain = NULL;

	// Free any wind effects
	if(m_pWindChain)
	{
		windgust_t* pwindchain = m_pWindChain;
		while(pwindchain)
		{
			windgust_t* pfree = pwindchain;
			pwindchain = pfree->pnext;
			delete pfree;
		}

		m_pWindChain = NULL;
	}

	// Reset these
	m_weatherType = WEATHER_OFF;

	m_particleFrequency = 0;
	m_numParticles = 0;
	m_numSpawned = 0;
	m_spawnTime = 0;
	m_gravity = 0;
	m_spawnHeight = 0;
	m_frameCount = 0;

	m_windX = 0;
	m_windY = 0;
	m_minWindX = 0;
	m_minWindY = 0;

	m_nextWindTime = 0;
	m_pParticleSprite = NULL;
	m_pImpactSprite = NULL;

	ReleaseDrawChains();
}

/*
====================
Think

====================
*/
void CWeather::Think( void )
{
	if(m_weatherType == WEATHER_OFF)
		return;

	if(IEngineStudio.IsHardware() != 1)
		return;

	IEngineStudio.GetTimes( &m_frameCount, &m_time, &m_oldTime );

	double life = m_time - m_spawnTime;
	if(life < 0)
		life = 0;

	double freq = 1/(float)m_particleFrequency;
	unsigned long itimesspawn = life/freq;
	m_frametime =  m_time - m_oldTime;

	// Spawn any particles if needed
	if(itimesspawn > m_numSpawned)
	{
		int inumspawn = itimesspawn - m_numSpawned;

		// Create new particles
		for(int j = 0; j < inumspawn; j++)
			CreateParticle();

		// Add to counter
		m_numSpawned += inumspawn;
	}

	// Update wind if any
	if(m_windX || m_windY)
	{
		if(!m_nextWindTime || m_nextWindTime < m_time)
		{
			float xVariation = m_windX * 0.75;
			float yVariation = m_windY * 0.75;

			windgust_t* pnew = new windgust_t;
			memset(pnew, 0, sizeof(windgust_t));

			pnew->starttime = m_time;
			pnew->duration = gEngfuncs.pfnRandomFloat(2.5, 8.5);
			pnew->windx = m_windX + gEngfuncs.pfnRandomFloat(-xVariation, xVariation);
			pnew->windy = m_windY + gEngfuncs.pfnRandomFloat(-yVariation, yVariation);
			m_nextWindTime = m_time + gEngfuncs.pfnRandomFloat(2, 8);

			// Add this to the chain
			if(m_pWindChain)
				m_pWindChain->pprev = pnew;

			pnew->pnext = m_pWindChain;
			m_pWindChain = pnew;
		}
	}

	// Update gusts
	windgust_t* pwindgust = m_pWindChain;
	while(pwindgust)
	{
		// Check if we're active
		if(pwindgust->starttime < m_time)
		{
			// See if wind gust is dead
			if(pwindgust->starttime + pwindgust->duration <= m_time)
			{
				windgust_t* pfree = pwindgust;
				pwindgust = pfree->pnext;

				if(pfree->pprev) 
					pfree->pprev->pnext = pfree->pnext;
				else 
					m_pWindChain = pfree->pnext;

				if(pfree->pnext) 
					pfree->pnext->pprev = pfree->pprev;

				delete pfree;
				continue;
			}
			else
			{
				// Calculate current force
				float windForce;
				float halfTime = pwindgust->duration / 2.0f;
				float windMid = pwindgust->starttime + halfTime;
				if(windMid >= m_time)
				{
					float windForceBase = (m_time - pwindgust->starttime);
					windForce = windForceBase / halfTime;
				}
				else
				{
					float windForceBase = (m_time - windMid) / halfTime;
					windForce = 1.0 - windForceBase;
				}

				float windForceSinus = fabs(sin(windForce));
				pwindgust->curx = windForceSinus * pwindgust->windx;
				pwindgust->cury = windForceSinus * pwindgust->windy;
			}
		}

		windgust_t* pnext = pwindgust->pnext;
		pwindgust = pnext;
	}

	// Only simulate if we have anything to process
	if(!m_pImpactChain && !m_pParticleChain)
		return;

	// Update standard particles
	w_particle_t *pparticle = m_pParticleChain;
	while(pparticle)
	{
		if(!UpdateParticle(pparticle, m_pParticleSprite))
		{
			w_particle_t *pfree = pparticle;
			pparticle = pfree->pnext;

			FreeParticle(&m_pParticleChain, pfree);
			continue;
		}

		w_particle_t *pnext = pparticle->pnext;
		pparticle = pnext;
	}

	// Update impact particles
	pparticle = m_pImpactChain;
	while(pparticle)
	{
		if(!UpdateParticle(pparticle, m_pImpactSprite))
		{
			w_particle_t *pfree = pparticle;
			pparticle = pfree->pnext;

			FreeParticle(&m_pImpactChain, pfree);
			continue;
		}

		w_particle_t *pnext = pparticle->pnext;
		pparticle = pnext;
	}
}

/*
====================
CreateParticle

====================
*/
void CWeather::CreateParticle( void )
{
	vec3_t testOrigin;
	vec3_t randomOffset;

	cl_entity_t *pplayer = gEngfuncs.GetLocalPlayer();
	randomOffset[0] = gEngfuncs.pfnRandomLong(-SYSTEM_SIZE, SYSTEM_SIZE);
	randomOffset[1] = gEngfuncs.pfnRandomLong(-SYSTEM_SIZE, SYSTEM_SIZE);
	randomOffset[2] = min(m_spawnHeight, pplayer->origin[2] + MAX_HEIGHT);

	// Test if we hit sky from the spawn origin
	testOrigin[0] = pplayer->origin[0]+randomOffset[0];
	testOrigin[1] = pplayer->origin[1]+randomOffset[1];
	testOrigin[2] = m_spawnHeight+16;

	// Spawn in a parabole
	float fraction = max(0.75, min(1, (randomOffset.Length2D()/SYSTEM_SIZE)));
	randomOffset[2] = (1-(fraction))*(randomOffset[2]-pplayer->origin[2]);

	static pmtrace_t tr;
	gEngfuncs.pEventAPI->EV_SetTraceHull(2);
	gEngfuncs.pEventAPI->EV_PlayerTrace(pplayer->origin+randomOffset, testOrigin, PM_WORLD_ONLY, -1, &tr);

	if(tr.fraction != 1.0 && gEngfuncs.PM_PointContents(tr.endpos, NULL) != CONTENTS_SKY)
		return;

	// Create the particle
	w_particle_t* pparticle = AllocParticle(&m_pParticleChain);
	if(!pparticle)
		return;

	VectorAdd(pplayer->origin, randomOffset, pparticle->origin);
	pparticle->gravity = m_gravity;
	pparticle->alpha = 1.0;
	pparticle->wind = true;
	if(m_weatherType == WEATHER_SNOW)
		pparticle->scale = gEngfuncs.pfnRandomFloat(0.006, 0.04);
	else
		pparticle->scale = gEngfuncs.pfnRandomFloat(0.008, 0.08);
	pparticle->life = -1;
	pparticle->fadein = PARTICLE_FADEIN;
	pparticle->spawntime = m_time;

	// Weather particles don't animate, they pick random sprite frames
	pparticle->framerate = 0;
	pparticle->fps = 0;
	pparticle->frame = gEngfuncs.pfnRandomLong(0, m_pParticleSprite->numframes-1);
	
	switch(m_weatherType)
	{
	case WEATHER_SNOW:
		{
			vec3_t lightEnd;
			lightEnd = pparticle->origin;
			lightEnd.z -= MAX_HEIGHT*2;

			model_t* pmodel = IEngineStudio.GetModelByIndex( 1 );
			if(m_surfStructSize == 0)
				m_surfStructSize = DetermineSurfaceStructSize();

			RecursiveLightPoint( pmodel, pmodel->nodes, pparticle->origin, lightEnd, pparticle->color, m_surfStructSize );

			for(int i = 0; i < 3; i++)
			{
				if(pparticle->color[i] < PARTICLE_MINLIGHT)
					pparticle->color[i] = PARTICLE_MINLIGHT;
			}

			VectorCopy(pparticle->origin, pparticle->last_check);
		}
		break;
	case WEATHER_RAIN:
		{
			// Rain is fullbright
			pparticle->color = Legacy_Vector(1, 1, 1);
		}
		break;
	}
}

/*
====================
CreateImpactParticle

====================
*/
void CWeather::CreateImpactParticle( vec3_t& origin, vec3_t& normal, float scale, int frame )
{
	w_particle_t* pparticle = AllocParticle(&m_pImpactChain);
	if(!pparticle)
		return;

	if(m_weatherType == WEATHER_SNOW)
	{
		VectorCopy(origin, pparticle->origin);
		pparticle->fadeout = IMPACT_FADE_TIME;
		pparticle->life = IMPACT_LIFE;
		pparticle->framerate = 1.0;
		pparticle->fps = 1.0;
		pparticle->scale = scale;
		pparticle->frame = frame;
	}
	else
	{
		// Keep sprite from clipping
		mspriteframe_t* pframe = GetSpriteFrame( m_pImpactSprite, 0 );
	
		pparticle->framerate = gEngfuncs.pfnRandomFloat(0.8, 1.2);
		pparticle->fps = 30;
		pparticle->fadeout = 0;
		pparticle->life = m_pImpactSprite->numframes / (pparticle->fps * pparticle->framerate);
		pparticle->scale = gEngfuncs.pfnRandomFloat(0.1, 0.25);
		pparticle->frame = 0;

		VectorMA(origin, fabs(pframe->down) * pparticle->scale, normal, pparticle->origin);
	}

	VectorCopy(normal, pparticle->normal);
	pparticle->gravity = 0;
	pparticle->alpha = 1.0;
	pparticle->wind = false;
	pparticle->spawntime = m_time;

	switch(m_weatherType)
	{
	case WEATHER_SNOW:
		{
			vec3_t lightEnd;
			lightEnd = pparticle->origin;
			lightEnd.z -= MAX_HEIGHT*2;

			model_t* pmodel = IEngineStudio.GetModelByIndex( 1 );
			if(m_surfStructSize == 0)
				m_surfStructSize = DetermineSurfaceStructSize();

			RecursiveLightPoint( pmodel, pmodel->nodes, origin, lightEnd, pparticle->color, m_surfStructSize);

			for(int i = 0; i < 3; i++)
			{
				if(pparticle->color[i] < PARTICLE_MINLIGHT)
					pparticle->color[i] = PARTICLE_MINLIGHT;
			}

			VectorCopy(origin, pparticle->last_check);
		}
		break;
	case WEATHER_RAIN:
		{
			// Rain is fullbright
			pparticle->color = Legacy_Vector(1, 1, 1);
		}
		break;
	}
}

/*
====================
UpdateParticle

====================
*/
bool CWeather::UpdateParticle( w_particle_t* particle, model_t* pmodel )
{
	// Kill the particle if the time is exceeded
	if( particle->life != -1 && particle->spawntime+particle->life < m_time )
		return false;

	// Skip collision testing if possible
	if( particle->gravity )
	{
		// Add in gravity first
		static vec3_t testVelocity;
		VectorMA( particle->velocity, m_frametime*m_gravity, Legacy_Vector(0, 0, -800), testVelocity );

		// Now add in any constant wind
		if(particle->wind)
		{
			if(m_minWindX)
				testVelocity.x += m_minWindX * m_frametime * particle->scale/0.08;

			if(m_minWindY)
				testVelocity.y += m_minWindY * m_frametime * particle->scale/0.08;
		}

		// Add any wind gusts
		if(m_pWindChain)
		{
			windgust_t* pwindgust = m_pWindChain;
			while(pwindgust)
			{
				testVelocity.x += pwindgust->curx * m_frametime * particle->scale/0.08;
				testVelocity.y += pwindgust->cury * m_frametime * particle->scale/0.08;

				windgust_t* pnext = pwindgust->pnext;
				pwindgust = pnext;
			}
		}

		// Calculate the test origin
		static vec3_t testOrigin;
		VectorMA( particle->origin, m_frametime, testVelocity, testOrigin );

		// Do collision testing
		int testContents = gEngfuncs.PM_PointContents( testOrigin, NULL );
		if( testContents != CONTENTS_EMPTY )
		{
			// Kill immediately if we hit sky
			if( testContents == CONTENTS_SKY )
				return false;
			
			static pmtrace_t tr;
			gEngfuncs.pEventAPI->EV_PlayerTrace( particle->origin, testOrigin, PM_WORLD_ONLY, -1, &tr );
			if(tr.fraction != 1.0)
			{
				CreateImpactParticle( tr.endpos, tr.plane.normal, particle->scale, particle->frame );
				return false;
			}
		}

		// Just update the origin and the velocity
		VectorCopy( testOrigin, particle->origin );
		VectorCopy( testVelocity, particle->velocity );
	}

	// Fade it out if needed
	if( particle->fadeout && particle->life != -1 )
	{
		double timeLeft = (particle->spawntime + particle->life) - m_time;
		double flfraction = timeLeft / particle->fadeout;
		flfraction = max(flfraction, 0);
		flfraction = min(flfraction, 1);

		particle->alpha = flfraction;
	}
	
	// Fade in if needed
	if( particle->fadein )
	{
		double lifeTime = m_time - particle->spawntime;
		double flfraction = lifeTime / particle->fadein;
		flfraction = max(flfraction, 0);
		flfraction = min(flfraction, 1);

		particle->alpha = flfraction;
		if(particle->alpha == 1.0)
			particle->fadein = 0;
	}

	// Get light values if needed
	if(m_weatherType != WEATHER_RAIN)
	{
		if( (particle->last_check - particle->origin).Length2D() > PARTICLE_LIGHTCHECK_DIST )
		{
			vec3_t lightEnd;
			lightEnd = particle->origin;
			lightEnd.z -= MAX_HEIGHT*2;

			model_t* pmodel = IEngineStudio.GetModelByIndex( 1 );
			if(m_surfStructSize == 0)
				m_surfStructSize = DetermineSurfaceStructSize();

			RecursiveLightPoint( pmodel, pmodel->nodes, particle->origin, lightEnd, particle->color, m_surfStructSize );

			for(int i = 0; i < 3; i++)
			{
				if(particle->color[i] < PARTICLE_MINLIGHT)
					particle->color[i] = PARTICLE_MINLIGHT;
			}

			VectorCopy( particle->origin, particle->last_check );
		}
	}

	// Update frames if we are animated
	if(pmodel->numframes > 1 && particle->framerate > 0 && particle->fps > 0)
	{
		float delta = gEngfuncs.GetClientTime() - particle->spawntime;
		int frame = floor(particle->fps * particle->framerate * delta);
		particle->frame = frame % pmodel->numframes;
	}

	return true;
}

/*
====================
AllocParticle

====================
*/
w_particle_t* CWeather::AllocParticle( w_particle_t** pchain )
{
	if(!m_pFreeParticles)
		return NULL;

	w_particle_t *pparticle = m_pFreeParticles;
	m_pFreeParticles = pparticle->pnext;
	memset(pparticle, 0, sizeof(w_particle_t));

	// Add system into pointer array
	if((*pchain))
	{
		(*pchain)->pprev = pparticle;
		pparticle->pnext = (*pchain);
	}

	(*pchain) = pparticle;
	return pparticle;
}

/*
====================
FreeParticle

====================
*/
void CWeather::FreeParticle( w_particle_t** pchain, w_particle_t* pparticle )
{
	if(pparticle->pprev) 
		pparticle->pprev->pnext = pparticle->pnext;
	else 
		(*pchain) = pparticle->pnext;

	if(pparticle->pnext) 
		pparticle->pnext->pprev = pparticle->pprev;

	pparticle->pnext = m_pFreeParticles;
	m_pFreeParticles = pparticle;
}

/*
====================
DrawParticles

====================
*/
void CWeather::DrawParticles( void )
{
	if(m_weatherType == WEATHER_OFF)
		return;

	if(!m_pImpactChain && !m_pParticleChain)
		return;

	if(IEngineStudio.IsHardware() != 1)
		return;

	// Push texture state
	glPushAttrib(GL_TEXTURE_BIT);

	// Disable textures on all units except first
	glActiveTexture(GL_TEXTURE1);
	glDisable(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE2);
	glDisable(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE3);
	glDisable(GL_TEXTURE_2D);

	// Set the active texture unit
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);

	// Set up texture state
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PRIMARY_COLOR_ARB);
	glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);

	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_FALSE);

	glEnable(GL_BLEND);
	glBlendFunc( GL_SRC_ALPHA, GL_ONE );

	// Get view info
	IEngineStudio.GetViewInfo( m_renderOrigin, m_viewUp, m_viewRight, m_viewForward );

	// Draw normal particles
	if( m_pParticleChain )
	{
		// First construct the chains
		w_particle_t *pparticle = m_pParticleChain;
		while(pparticle)
		{
			pparticle->pdrawchain = m_pParticleDrawChains[pparticle->frame];
			m_pParticleDrawChains[pparticle->frame] = pparticle;

			pparticle = pparticle->pnext;
		}

		if(m_weatherType == WEATHER_RAIN)
			glCullFace(GL_BACK);
		else
			glCullFace(GL_FRONT);

		// Now go through each of them
		for(int i = 0; i < m_pParticleSprite->numframes; i++)
		{
			if(!m_pParticleDrawChains[i])
				continue;

			// Bind sprite
			mspriteframe_t* pframe = GetSpriteFrame( m_pParticleSprite, i );
			glBindTexture( GL_TEXTURE_2D, pframe->gl_texturenum );

			// Go through the chain and draw each
			pparticle = m_pParticleDrawChains[i];
			if(m_weatherType == WEATHER_SNOW)
			{
				while(pparticle)
				{
					DrawTiledParticle(pparticle, pframe);
					pparticle = pparticle->pdrawchain;
				}
			}
			else
			{
				while(pparticle)
				{
					DrawTracerParticle(pparticle, pframe);
					pparticle = pparticle->pdrawchain;
				}
			}
			
			// Clear the chain
			m_pParticleDrawChains[i] = NULL;
		}

		if(m_weatherType == WEATHER_RAIN)
			glCullFace(GL_FRONT);
	}

	// Draw impact particles
	if( m_pImpactChain )
	{
		w_particle_t *pparticle = m_pImpactChain;
		while(pparticle)
		{
			pparticle->pdrawchain = m_pImpactDrawChains[pparticle->frame];
			m_pImpactDrawChains[pparticle->frame] =  pparticle;

			pparticle = pparticle->pnext;
		}

		// Now go through each of them
		for(int i = 0; i < m_pImpactSprite->numframes; i++)
		{
			if(!m_pImpactDrawChains[i])
				continue;

			// Bind sprite
			mspriteframe_t* pframe = GetSpriteFrame( m_pImpactSprite, i );
			glBindTexture( GL_TEXTURE_2D, pframe->gl_texturenum );

			// Go through the chain and draw each
			pparticle = m_pImpactDrawChains[i];
			while(pparticle)
			{
				DrawTiledParticle(pparticle, pframe);
				pparticle = pparticle->pdrawchain;
			}
			
			// Clear the chain
			m_pImpactDrawChains[i] = NULL;
		}
	}

	// Restore states
	glDisable(GL_BLEND);
	glPopAttrib();
}

/*
====================
GetSpriteFrame

====================
*/
mspriteframe_t* CWeather::GetSpriteFrame ( model_t *sprite, int frame )
{
	mspriteframe_t *pspriteframe;
	msprite_t *psprite = (msprite_t*)sprite->cache.data;

	if ((frame >= psprite->numframes) || (frame < 0))
	{
		gEngfuncs.Con_Printf ("R_GetSpriteFrame: no such frame %d\n", frame);
		frame = 0;
	}

	if (psprite->frames[frame].type == SPR_SINGLE)
	{
		pspriteframe = psprite->frames[frame].frameptr;
	}
	else
	{
		mspritegroup_t *pspritegroup = (mspritegroup_t *)psprite->frames[frame].frameptr;
		float *pintervals = pspritegroup->intervals;
		float fullinterval = pintervals[pspritegroup->numframes-1];

		// when loading in Mod_LoadSpriteGroup, we guaranteed all interval values
		// are positive, so we don't have to worry about division by 0
		float targettime = m_time - ((int)(m_time / fullinterval)) * fullinterval;

		int i = 0;
		for (; i < (pspritegroup->numframes-1); i++)
		{
			if (pintervals[i] > targettime)
				break;
		}

		pspriteframe = pspritegroup->frames[i];
	}

	return pspriteframe;
}

/*
====================
DrawParticles

====================
*/
void CWeather::DrawTiledParticle( w_particle_t* pparticle, mspriteframe_t* pframe )
{
	// Set color
	glColor4f(pparticle->color.x, pparticle->color.y, pparticle->color.z, pparticle->alpha);

	glBegin( GL_QUADS );
	vec3_t vertexPos = pparticle->origin - m_viewForward * 0.01 + m_viewUp * pframe->up * pparticle->scale;
	vertexPos = vertexPos + m_viewRight * pframe->left * pparticle->scale;
	glTexCoord2f( 0, 0 );
	glVertex3fv( vertexPos );

	vertexPos = pparticle->origin - m_viewForward * 0.01 + m_viewUp * pframe->up * pparticle->scale;
	vertexPos = vertexPos + m_viewRight * pframe->right * pparticle->scale;
	glTexCoord2f( 1, 0 );
	glVertex3fv( vertexPos );

	vertexPos = pparticle->origin - m_viewForward * 0.01 + m_viewUp * pframe->down * pparticle->scale;
	vertexPos = vertexPos + m_viewRight * pframe->right * pparticle->scale;
	glTexCoord2f( 1, 1 );
	glVertex3fv( vertexPos );

	vertexPos = pparticle->origin - m_viewForward * 0.01 + m_viewUp * pframe->down * pparticle->scale;
	vertexPos = vertexPos + m_viewRight * pframe->left * pparticle->scale;
	glTexCoord2f( 0, 1 );
	glVertex3fv( vertexPos );
	glEnd();
}

/*
====================
DrawParticles

====================
*/
void CWeather::DrawTracerParticle( w_particle_t* pparticle, mspriteframe_t* pframe )
{
	vec3_t up;
	VectorScale(m_viewUp, 1.5, up);

	vec3_t right;
	VectorScale(m_viewRight, 1.5, right);

	const float minDistance = 128;
	const float maxDistance = SYSTEM_SIZE;
	float viewDistance = (pparticle->origin - m_renderOrigin).Length();
	float scaleDistAdj = 1.0 - (maxDistance - viewDistance) / (maxDistance - minDistance);
	if(scaleDistAdj < 0)
		scaleDistAdj = 0;
	else if (scaleDistAdj > 1.0)
		scaleDistAdj = 1.0;

	float width = pframe->width * (pparticle->scale + scaleDistAdj * pparticle->scale * 8.0);
	float height = pframe->height * (pparticle->scale + scaleDistAdj * pparticle->scale * 8.0);

	vec3_t start = pparticle->origin;
	vec3_t end = start + pparticle->velocity.Normalize() * height;

	vec3_t screenStart, screenEnd;
	bool culledStart = gEngfuncs.pTriAPI->WorldToScreen(start, screenStart) == 1 ? true : false;
	bool culledEnd = gEngfuncs.pTriAPI->WorldToScreen(end, screenEnd) == 1 ? true : false;

	if(!culledStart || !culledEnd)
	{
		Legacy_Vector tmp;
		VectorSubtract(screenStart, screenEnd, tmp);

		tmp[0] *= width;
		tmp[1] *= width;
		tmp[2] = 0;
		tmp.Normalize();

		Legacy_Vector normal;
		VectorScale(up, tmp[0], normal);
		VectorMA(normal, -tmp[1], right, normal);

		// Set color
		glColor4f(pparticle->color.x, pparticle->color.y, pparticle->color.z, pparticle->alpha);

		// Draw the quad
		glBegin(GL_QUADS);
		VectorAdd(start, normal, tmp);
		glTexCoord2f(0, 0);
		glVertex3fv(tmp);

		VectorAdd(end, normal, tmp);
		glTexCoord2f(0, 1);
		glVertex3fv(tmp);

		VectorSubtract(end, normal, tmp);
		glTexCoord2f(1, 1);
		glVertex3fv(tmp);

		VectorSubtract(start, normal, tmp);
		glTexCoord2f(1, 0);
		glVertex3fv(tmp);
		glEnd();
	}
}

/*
====================
ReleaseDrawChains

====================
*/
void CWeather::ReleaseDrawChains( void )
{
	if(m_pParticleDrawChains)
	{
		delete[] m_pParticleDrawChains;
		m_pParticleDrawChains = NULL;
	}

	if(m_pImpactDrawChains)
	{
		delete[] m_pImpactDrawChains;
		m_pImpactDrawChains = NULL;
	}
}

/*
====================
AllocDrawChains

====================
*/
void CWeather::AllocDrawChains( void )
{
	if(!m_pParticleSprite || m_pParticleSprite->numframes <= 0)
	{
		gEngfuncs.Con_Printf("%s - Invalid or missing sprite for particle effect.\n", __FUNCTION__);
		Clear();
		return;
	}

	if(!m_pImpactSprite || m_pImpactSprite->numframes <= 0)
	{
		gEngfuncs.Con_Printf("%s - Invalid or missing sprite for impact effect.\n", __FUNCTION__);
		Clear();
		return;
	}

	// For the main particles
	m_pParticleDrawChains = new w_particle_t*[m_pParticleSprite->numframes];
	memset(m_pParticleDrawChains, 0, sizeof(w_particle_t*)*m_pParticleSprite->numframes);

	// For the impact particles
	m_pImpactDrawChains = new w_particle_t*[m_pImpactSprite->numframes];
	memset(m_pImpactDrawChains, 0, sizeof(w_particle_t*)*m_pImpactSprite->numframes);
}


/*
====================
DetermineSurfaceStructSize

====================
*/
int CWeather::DetermineSurfaceStructSize( void )
{
	model_t* pworld = IEngineStudio.GetModelByIndex(1);
	assert(pworld);

	mplane_t* pplanes = pworld->planes;
	msurface_t* psurfaces = pworld->surfaces;

	// Try to find second texinfo ptr
	byte* psecondsurfbytedata = reinterpret_cast<byte*>(&psurfaces[1]);

	// Size of msurface_t with that stupid displaylist junk
	static const int MAXOFS = 108;

	int byteoffs = 0;
	while(byteoffs <= MAXOFS)
	{
		mplane_t **pplaneptr = reinterpret_cast<mplane_t**>(psecondsurfbytedata+byteoffs);
		
		int i = 0;
		for(; i < pworld->numplanes; i++)
		{
			if(&pplanes[i] == *pplaneptr)
				break;
		}

		if(i != pworld->numplanes)
		{
			break;
		}

		byteoffs++;
	}

	if(byteoffs >= MAXOFS)
	{
		gEngfuncs.Con_Printf("%s - Failed to determine msurface_t struct size.\n");
		return sizeof(msurface_t);
	}
	else
	{
		mplane_t** pfirstsurftexinfoptr = &psurfaces[0].plane;
		byte* psecondptr = reinterpret_cast<byte*>(psecondsurfbytedata) + byteoffs;
		byte* ptr = reinterpret_cast<byte*>(pfirstsurftexinfoptr);
		return ((unsigned int)psecondptr - (unsigned int)ptr);
	}
}

/*
====================
MsgFunc_Weather

====================
*/
int CWeather::MsgFunc_Weather( const char *pszName, int iSize, void *pBuf )
{
	BEGIN_READ( pBuf, iSize );

	// Always clear
	Clear();

	model_t* pmodel = IEngineStudio.GetModelByIndex( 1 );

	bool isActive = (READ_BYTE() == 1) ? true : false;
	if(!isActive)
		return 1;

	if(IEngineStudio.IsHardware() != 1)
	{
		gEngfuncs.Con_Printf("Weather effects are not available in Software or Direct3D mode.\n");
		return 1;
	}

	int iType = READ_BYTE();
	m_weatherType = (iType == 0) ? WEATHER_SNOW : WEATHER_RAIN;

	m_particleFrequency = READ_COORD()*10;
	m_spawnHeight = READ_COORD();

	m_windX = READ_SHORT();
	m_minWindX = READ_SHORT();

	m_windY = READ_SHORT();
	m_minWindY = READ_SHORT();

	const char* pstrModelName = READ_STRING();
	m_pParticleSprite = IEngineStudio.Mod_ForName(pstrModelName, FALSE);
	if(!m_pParticleSprite)
	{
		gEngfuncs.Con_Printf( "Invalid or missing particle sprite '%s' specified for weather entity.\n", pstrModelName );
		Clear();
		return 1;
	}

	pstrModelName = READ_STRING();
	m_pImpactSprite = IEngineStudio.Mod_ForName(pstrModelName, FALSE);
	if(!m_pImpactSprite)
	{
		gEngfuncs.Con_Printf( "Invalid or missing particle sprite '%s' specified for weather entity.\n", pstrModelName );
		Clear();
		return 1;
	}

	if( m_weatherType == WEATHER_SNOW )
		m_gravity = SNOW_GRAVITY;
	else
		m_gravity = RAIN_GRAVITY;

	IEngineStudio.GetTimes( &m_frameCount, &m_spawnTime, &m_oldTime );
	m_numSpawned = 0;

	// Allocate draw chains for each frame
	AllocDrawChains();

	return 1;
}

/*
==================
RecursiveLightPoint

==================
*/
int RecursiveLightPoint( model_t* pmodel, mnode_t *node, const vec3_t &start, const vec3_t &end, vec3_t &color, int surfstructsize )
{
	float		front, back, frac;
	int			side;
	mplane_t	*plane;
	vec3_t		mid;
	int			s, t, ds, dt;
	int			i;
	mtexinfo_t	*tex;
	color24		*lightmap;

	if (node->contents < 0)
		return FALSE;		// didn't hit anything
	
	plane = node->plane;
	front = DotProduct (start, plane->normal) - plane->dist;
	back = DotProduct (end, plane->normal) - plane->dist;
	side = front < 0;
	
	if ( (back < 0) == side )
		return RecursiveLightPoint (pmodel, node->children[side], start, end, color, surfstructsize);
	
	frac = front / (front-back);
	mid[0] = start[0] + (end[0] - start[0])*frac;
	mid[1] = start[1] + (end[1] - start[1])*frac;
	mid[2] = start[2] + (end[2] - start[2])*frac;
	
// go down front side	
	int r = RecursiveLightPoint (pmodel, node->children[side], start, mid, color, surfstructsize);

	if (r) 
		return TRUE;
		
	if ((back < 0) == side)
		return FALSE;

	byte* pfirstsurfbyteptr = reinterpret_cast<byte*>(pmodel->surfaces);
	for (i = 0; i < node->numsurfaces; i++)
	{
		msurface_t	*surf = reinterpret_cast<msurface_t*>(pfirstsurfbyteptr + surfstructsize*(node->firstsurface+i));
		if (surf->flags & (SURF_DRAWTILED | SURF_DRAWSKY))
			continue;	// no lightmaps

		int index = node->firstsurface+i;
		tex = surf->texinfo;
		s = DotProduct(mid, tex->vecs[0])+tex->vecs[0][3];
		t = DotProduct(mid, tex->vecs[1])+tex->vecs[1][3];

		if (s < surf->texturemins[0] ||
		t < surf->texturemins[1])
			continue;
		
		ds = s - surf->texturemins[0];
		dt = t - surf->texturemins[1];
		
		if ( ds > surf->extents[0] || dt > surf->extents[1] )
			continue;

		if (!surf->samples)
			continue;

		ds >>= 4;
		dt >>= 4;

		lightmap = surf->samples;

		if (lightmap)
		{
			int surfindex = node->firstsurface+i;
			int size = ((surf->extents[1]>>4)+1)*((surf->extents[0]>>4)+1);
			lightmap += dt * ((surf->extents[0]>>4)+1) + ds;

			float flIntensity = (lightmap->r + lightmap->g + lightmap->b)/3;
			float flScale = flIntensity/50;

			if(flScale > 1.0) 
				flScale = 1.0;

			color[0] = (float)(lightmap->r * flScale)/255.0f;
			color[1] = (float)(lightmap->g * flScale)/255.0f;
			color[2] = (float)(lightmap->b * flScale)/255.0f;
		}	
		else
		{
			color[0] = color[1] = color[2] = 0.5;
		}
		return TRUE;
	}

// go down back side
	return RecursiveLightPoint (pmodel, node->children[!side], mid, end, color, surfstructsize);
}