/***
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
// battery.cpp
//
// implementation of CHudBattery class
//

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "event_api.h"

#include <string.h>
#include <stdio.h>

#if defined(CLIENT_DLL) && !defined(MAX_NORMAL_BATTERY)
#define MAX_NORMAL_BATTERY 100
#endif

DECLARE_MESSAGE(m_Battery, Battery)

int CHudBattery::Init(void)
{
	m_iBat = 0;
	m_fFade = 0;
	m_iFlags = 0;

	HOOK_MESSAGE(Battery);

	gHUD.AddHudElem(this);

	return 1;
};


int CHudBattery::VidInit(void)
{
	int HUD_suit_empty = gHUD.GetSpriteIndex( "suit_empty" );
	int HUD_suit_full = gHUD.GetSpriteIndex( "suit_full" );

	m_hSprite1 = m_hSprite2 = 0;  // delaying get sprite handles until we know the sprites are loaded
	m_prc1 = &gHUD.GetSpriteRect( HUD_suit_empty );
	m_prc2 = &gHUD.GetSpriteRect( HUD_suit_full );
	m_iHeight = m_prc2->bottom - m_prc1->top;
	m_fFade = 0;
	return 1;
};

void CHudBattery::RunShieldPrediction(float time)
{
	if (m_iBat >= MAX_NORMAL_BATTERY)
	{
		if (!m_bShieldMaxxed)
		{
			gEngfuncs.pEventAPI->EV_StopSound(0, CHAN_STATIC, "player/shield_lp.wav");
			m_bShieldMaxxed = true;
		}
	}
	m_bShieldMaxxed = false;

	if (m_iBat <= 0)
	{
		if (!m_bShieldEmpty && (time - m_flLastShieldSoundTime > 1.0f)) // prevent spamming the sound
		{
			gEngfuncs.pEventAPI->EV_PlaySound(0, vec3_origin, CHAN_AUTO, "player/shield_empty.wav", 0.85, ATTN_NORM, 0, PITCH_NORM);
			gEngfuncs.pEventAPI->EV_PlaySound(0, vec3_origin, CHAN_AUTO, "player/shield_depleted2.wav", 0.7, ATTN_NORM, 0, PITCH_NORM);
			m_bShieldEmpty = true;
			m_flLastShieldSoundTime = time; // update the last sound time
		}
	}
	else
	{
		if (m_bShieldEmpty)
		{
			gEngfuncs.pEventAPI->EV_StopSound(0, CHAN_AUTO, "player/shield_empty.wav");
			m_bShieldEmpty = false;
		}

		if (m_iBat <= 10)
		{
			if (!m_bShieldLow && (time - m_flLastShieldSoundTime > 1.0f))
			{
				gEngfuncs.pEventAPI->EV_PlaySound(0, vec3_origin, CHAN_AUTO, "player/shield_low.wav", 0.75, ATTN_NORM, 0, PITCH_NORM);
				m_bShieldLow = true;
				m_flLastShieldSoundTime = time; // update the last sound time
			}
		}
		else if (m_bShieldLow)
		{
			gEngfuncs.pEventAPI->EV_StopSound(0, CHAN_AUTO, "player/shield_low.wav");
			m_bShieldLow = false;
		}
	}
}

int CHudBattery:: MsgFunc_Battery(const char *pszName,  int iSize, void *pbuf )
{
	m_iFlags |= HUD_ACTIVE;
	
	BEGIN_READ( pbuf, iSize );
	int x = READ_SHORT();

#if defined( _TFC )
	int y = READ_SHORT();

	if ( x != m_iBat || y != m_iBatMax )
	{
		m_fFade = FADE_TIME;
		m_iBat = x;
		m_iBatMax = y;
	}
#else
	if ( x != m_iBat )
	{
		m_fFade = FADE_TIME;
		m_iBat = x;
	}
#endif

	return 1;
}


int CHudBattery::Draw(float flTime)
{
	if (gHUD.m_iHideHUDDisplay & HIDEHUD_HEALTH)
		return 1;

	int r, g, b, x, y, a;
	wrect_t rc;

	rc = *m_prc2;

#if defined( _TFC )
	float fScale = 0.0;

	if (m_iBatMax > 0)
		fScale = 1.0 / (float)m_iBatMax;

	rc.top += m_iHeight * ((float)(m_iBatMax - (min(m_iBatMax, m_iBat))) * fScale); // battery can go from 0 to m_iBatMax so * fScale goes from 0 to 1
#else
	rc.top += m_iHeight * ((float)(100 - (min(100, m_iBat))) * 0.01);	// battery can go from 0 to 100 so * 0.01 goes from 0 to 1
#endif

	RunShieldPrediction(flTime); // Run shield prediction, if applicable

	// DeanAMX: Flash the armour HUD on zero.

	UnpackRGB(r, g, b, RGB_DEFAULT);
	if (m_iBat > 25)
	{
		UnpackRGB(r, g, b, gHUD.m_iDefaultHUDColor);
	}
	else
	{
		r = 250;
		g = 0;
		b = 0;
	}

	float auraRegeneration = CVAR_GET_FLOAT("sv_aura_regeneration");

	if (auraRegeneration != 0)
	{
		if (m_iBat <= 10)
		{
			if (!Blinking)
			{
				Blinking = true;
			}

		a = (int)(fabs(sin(flTime * 10)) * 256.0);
	}
	else
	{
		Blinking = false;

		if (0 != m_fFade) // Has health changed? Flash the health #
		{
			if (m_fFade > FADE_TIME)
				m_fFade = FADE_TIME;

			m_fFade -= (gHUD.m_flTimeDelta * 20);
			if (m_fFade <= 0)
			{
				a = 128;
				m_fFade = 0;
			}

			// Fade the health number back to dim

			a = MIN_ALPHA + (m_fFade / FADE_TIME) * 128;
		}
		else
			a = MIN_ALPHA;
		}
	
	ScaleColors(r, g, b, a );
	}

	int iOffset = (m_prc1->bottom - m_prc1->top)/6;

	y = ScreenHeight - gHUD.m_iFontHeight - gHUD.m_iFontHeight / 2;
	y += (int)(gHUD.m_iFontHeight * 0.2f);
	int width = (m_prc1->right - m_prc1->left);
	// this used to just be ScreenWidth/5 but that caused real issues at higher resolutions. Instead, base it on the width of this sprite.
	x = 3 * width;

	// make sure we have the right sprite handles
	if ( !m_hSprite1 )
		m_hSprite1 = gHUD.GetSprite( gHUD.GetSpriteIndex( "suit_empty" ) );
	if ( !m_hSprite2 )
		m_hSprite2 = gHUD.GetSprite( gHUD.GetSpriteIndex( "suit_full" ) );

	SPR_Set(m_hSprite1, r, g, b );
	SPR_DrawAdditive( 0,  x, y - iOffset, m_prc1);

	if (rc.bottom > rc.top)
	{
		SPR_Set(m_hSprite2, r, g, b );
		SPR_DrawAdditive( 0, x, y - iOffset + (rc.top - m_prc2->top), &rc);
	}

	x += width;
	y += (int)(gHUD.m_iFontHeight * 0.2f);
	x = gHUD.DrawHudNumber(x, y, DHN_3DIGITS | DHN_DRAWZERO, m_iBat, r, g, b);

	return 1;
}