//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include <assert.h>
#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "com_model.h"
#include "studio.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "dlight.h"
#include "triangleapi.h"

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <math.h>

#include "studio_util.h"
#include "r_studioint.h"

#include "StudioModelRenderer.h"
#include "GameStudioModelRenderer.h"
#include "Exports.h"

//
// Override the StudioModelRender virtual member functions here to implement custom bone
// setup, blending, etc.
//

// Global engine <-> studio model rendering code interface
extern engine_studio_api_t IEngineStudio;

// The renderer object, created on the stack.
CGameStudioModelRenderer g_StudioRenderer;
/*
====================
CGameStudioModelRenderer

====================
*/
CGameStudioModelRenderer::CGameStudioModelRenderer( void )
{
}

////////////////////////////////////
// Hooks to class implementation
////////////////////////////////////

/*
====================
R_StudioDrawPlayer

====================
*/
int R_StudioDrawPlayer( int flags, entity_state_t *pplayer )
{
	return g_StudioRenderer.StudioDrawPlayer( flags, pplayer );
}

/*
====================
R_StudioDrawModel

====================
*/
static cl_entity_t legs;
int R_StudioDrawModel( int flags )
{
	return g_StudioRenderer.StudioDrawModel( flags );

	if (!CL_IsThirdPerson() && (m_pCurrentEntity == gEngfuncs.GetViewModel())) // check for first person and viewmodel
		2
	{
		3
			cl_entity_t * saveent = m_pCurrentEntity; // save the current drawing entity
		four
			cl_entity_t* player = gEngfuncs.GetLocalPlayer(); // find the local player
		five

			6
			vec3_t forward, origin, angles;
		7

			eight
			AngleVectors(player->angles, forward, NULL, NULL); // find the direction of the player's gaze
		nine
			origin = player->origin - forward * 20; // move the origin of the legs back a little so as not to see unnecessary parts of the model
		10
			angles = player->angles; // the angles of rotation of the legs are the same as for the player
		eleven

			12
			// copy the new vectors to the leg entity vectors
			13
			VectorCopy(origin, legs.origin);
		fourteen
			VectorCopy(origin, legs.curstate.origin);
		fifteen
			VectorCopy(angles, legs.angles);
		sixteen
			VectorCopy(angles, legs.curstate.angles);
		17

			eighteen
			legs.model = IEngineStudio.Mod_ForName("models / player_legs.mdl", 1); // assign the entity model. I don't remember whether it is necessary to pre-cache it on the server, if the game crashes, then make a pre-cache in ClientPrecache in client.cpp
		nineteen
			legs.curstate.framerate = player->curstate.framerate; // copy the frame rate
		twenty
			legs.curstate.sequence = player->curstate.gaitsequence; // copy the sequence. That is, the gate sequence. That is why ordinary sexences are not played.
		21

			22
			m_pPlayerInfo = IEngineStudio.PlayerInfo(player->index - 1); // get player info
		23
			StudioProcessGait(IEngineStudio.GetPlayerState(player->index - 1)); // calculate the gate
		24
			legs.angles[YAW] = legs.curstate.angles[YAW] = m_pPlayerInfo->gaityaw; // make the horizontal rotation angle equal to gaityaw
			m_pPlayerInfo = NULL; // nullify the player's info. I do not know whether it is necessary or not, but in all places where it is determined, it is then reset to zero

			StudioDrawCustomEntity(&legs); // draw the model

			m_pCurrentEntity = saveent; // return the original model. I also don’t remember if it was necessary. I rewrote the code 10 times, so I don't remember in which case there were glitches without this line
	}

}

/*
====================
R_StudioInit

====================
*/
void R_StudioInit( void )
{
	g_StudioRenderer.Init();
}

// The simple drawing interface we'll pass back to the engine
r_studio_interface_t studio =
{
	STUDIO_INTERFACE_VERSION,
	R_StudioDrawModel,
	R_StudioDrawPlayer,
};

/*
====================
HUD_GetStudioModelInterface

Export this function for the engine to use the studio renderer class to render objects.
====================
*/
int CL_DLLEXPORT HUD_GetStudioModelInterface( int version, struct r_studio_interface_s **ppinterface, struct engine_studio_api_s *pstudio )
{
//	RecClStudioInterface(version, ppinterface, pstudio);

	if ( version != STUDIO_INTERFACE_VERSION )
		return 0;

	// Point the engine to our callbacks
	*ppinterface = &studio;

	// Copy in engine helper functions
	memcpy( &IEngineStudio, pstudio, sizeof( IEngineStudio ) );

	// Initialize local variables, etc.
	R_StudioInit();

	// Success
	return 1;
}

int CGameStudioModelRenderer::StudioDrawCustomEntity(cl_entity_t* ent)
{
		alight_t lighting;
		vec3_t dir;

		m_pCurrentEntity = ent;
		IEngineStudio.GetTimes(&m_nFrameCount, &m_clTime, &m_clOldTime);
		IEngineStudio.GetViewInfo(m_vRenderOrigin, m_vUp, m_vRight, m_vNormal);
		IEngineStudio.GetAliasScale(&m_fSoftwareXScale, &m_fSoftwareYScale);

		m_pRenderModel = m_pCurrentEntity->model;
		m_pStudioHeader = (studiohdr_t*)IEngineStudio.Mod_Extradata(m_pRenderModel);
		IEngineStudio.StudioSetHeader(m_pStudioHeader);
		IEngineStudio.SetRenderModel(m_pRenderModel);

		StudioSetUpTransform(0);

		(*m_pModelsDrawn)++;
	(*m_pStudioModelCount)++; // render data cache cookie

		if (m_pStudioHeader->numbodyparts == 0)
			return 1;

		StudioSetupBones();
		StudioSaveBones();

		lighting.plightvec = dir;
		IEngineStudio.StudioDynamicLight(m_pCurrentEntity, &lighting);
		IEngineStudio.StudioEntityLight(&lighting);

		// model and frame independant
		IEngineStudio.StudioSetupLighting(&lighting);

		// get remap colors
		m_nTopColor = m_pCurrentEntity->curstate.colormap & 0xFF;
		m_nBottomColor = (m_pCurrentEntity->curstate.colormap & 0xFF00) >> 8;
		IEngineStudio.StudioSetRemapColors(m_nTopColor, m_nBottomColor);

		StudioRenderModel();

		return 1;
}
