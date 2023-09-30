// Sabian Roberts for TPPS!! 25-09-2023
// used Chat GPT-3.5 for assistance as I am absolutely shit at C++
// and I don't have the time to learn it properly, unfortunately :'(

#include "hud.h"
#include "cl_util.h"
#include "cl_entity.h"
#include "triangleapi.h"
#include "vgui_TeamFortressViewport.h"
#include "vgui_SpectatorPanel.h"
#include "pmtrace.h"
#include "event_api.h"
#include "pm_defs.h"

#include "pm_shared.h"
#include "pm_defs.h"
#include "parsemsg.h"


// Let's define identifiers that aren't anywhere else first
#define MAX_TRACE_LENGTH 8192 //this is the max size of a map

// define PLAYER_ENEMY
#define PLAYER_ENEMY 1

void ChangeCrosshairColour(int r, int g, int b)
{
	gHUD.m_iCrosshairRed = r;
	gHUD.m_iCrosshairGreen = g;
	gHUD.m_iCrosshairBlue = b;
}

bool IsLookingAtEnemy()
{
	// get the player's view
	cl_entity_t* player = gEngfuncs.GetLocalPlayer();
	if (!player)
		return false;

	vec3_t viewAngles;
	gEngfuncs.GetViewAngles(viewAngles);

	// calc the forward angle
	vec3_t forward;
	AngleVectors(viewAngles, forward, nullptr, nullptr);

		// trace a line from the player's center
		pmtrace_t trace;
		physent_t ent;
	gEngfuncs.pEventAPI->EV_SetTraceHull(2); // Use a small hull for the trace
	gEngfuncs.pEventAPI->EV_PlayerTrace(player->origin, player->origin + forward * MAX_TRACE_LENGTH, PM_NORMAL, -1, &trace);

	// cl_entity_t* entity = GetEntityByIndex(entityIndex);


		/*if (trace.ent && trace.ent->curstate.iuser1 == PLAYER_ENEMY) // Assuming PLAYER_ENEMY is the enemy player type
		{
			return true;
		}

		return false;*/
}

void HUD_WeaponSelect(int iWeapon)
{
	// Check if the player is looking at an enemy
	if (IsLookingAtEnemy())
	{
		// Change the crosshair color to red when looking at an enemy
		ChangeCrosshairColour(255, 0, 0);
	}
	else
	{
		// Reset the crosshair color to default when not looking at an enemy
		ChangeCrosshairColour(255, 255, 255);
	}
}