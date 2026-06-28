/****
*
* Copyright (c) 2021-2026 The Phoenix Project Software. Some Rights Reserved.
*
* AURA
*
* HX-40 THUMPER - M203 GRENADE LAUNCHER WEAPON
*
*
****/

#ifndef _HALO

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "weapon_hierarchy.h"
#include "gamerules.h"

#include "../cl_dll/cl_gametype.h"

#define THUMPER_ZOOM_LEVEL 50

LINK_ENTITY_TO_CLASS(weapon_thumper, CThumper);

void CThumper::Spawn()
{
	Precache();

	m_iId = WEAPON_THUMPER;
	SET_MODEL(ENT(pev), "models/w_railgun.mdl");

	m_iDefaultAmmo = THUMPER_DEFAULT_GIVE;

	FallInit();
}

void CThumper::Precache()
{
	PRECACHE_MODEL("models/weapons/thumper/v_rock2.mdl");
	PRECACHE_MODEL("models/w_railgun.mdl");
	PRECACHE_MODEL("models/weapons/thumper/p_rock2.mdl");

	PRECACHE_SOUND("weapons/hx40/hx40_fire.wav");

	PRECACHE_MODEL("models/grenade.mdl");

	m_usThumper = PRECACHE_EVENT(1, "events/thumper.sc");
}

int CThumper::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "ARgrenades";
	p->iMaxAmmo1 = M203_GRENADE_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = WEAPON_NOCLIP;
	p->iMaxClip = THUMPER_MAX_CLIP;
	p->iSlot = WPN_FOREIGN_SLOT;
	p->iPosition = WPN_THUMPER_POS;
	p->iId = WEAPON_THUMPER;
	p->iFlags = 0;
	p->iWeight = THUMPER_WEIGHT;

	return 1;
}

BOOL CThumper::Deploy()
{
	return DefaultDeploy("models/weapons/thumper/v_rock2.mdl", "models/weapons/thumper/p_rock2.mdl", THUMPER_IDLE1, "mp5");
}

void CThumper::Holster(int skiplocal)
{
	SendWeaponAnim(THUMPER_HOLSTER);

	if (m_bInZoom)
		SecondaryAttack();

	m_fInReload = false;
	m_pPlayer->m_flNextAttack = gpGlobals->time + 0.5;
	m_flTimeWeaponIdle = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10.0, 15.0);
}

void CThumper::PrimaryAttack()
{
	CBasePlayer* pPlayer = (CBasePlayer*)m_pPlayer;

	if (INSTAGIB != g_iGameType)
	{
		if (m_pPlayer->pev->waterlevel == 3)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = 0.15;
			return;
		}
	}

	if (m_fInReload)
		return;

	if (m_iClip <= 0)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = gpGlobals->time + 0.35f;
		return;
	}

	m_iClip--;

	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	Legacy_Vector vecSrc = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 38 + gpGlobals->v_right * 6 - gpGlobals->v_up * 4;

	Legacy_Vector vecVelocity = gpGlobals->v_forward * 2000;

#ifndef CLIENT_DLL
	CGrenade* pGrenade = CGrenade::ShootContact(m_pPlayer->pev, vecSrc, vecVelocity);

	if (pGrenade)
		pGrenade->m_bThumperGrenade = true;
#endif

	PLAYBACK_EVENT_FULL(0, pPlayer->edict(), m_usThumper, 0.0, (float*)&g_vecZero, (float*)&g_vecZero, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);

	SendWeaponAnim(THUMPER_FIRE1);

	m_flNextPrimaryAttack = gpGlobals->time + 1.0f;
	m_flTimeWeaponIdle = gpGlobals->time + 1.5f;
}

void CThumper::SecondaryAttack()
{
	m_bInZoom = !m_bInZoom;

	ToggleZoom();

	m_flNextSecondaryAttack = gpGlobals->time + 0.6f;
}

void CThumper::ToggleZoom()
{
	if (m_pPlayer->pev->fov == 0)
	{
		m_pPlayer->pev->fov = THUMPER_ZOOM_LEVEL;
		m_pPlayer->m_iFOV = THUMPER_ZOOM_LEVEL;

		m_bInZoom = true;
	}
	else
	{
		m_pPlayer->pev->fov = 0;
		m_pPlayer->m_iFOV = 0;

		m_bInZoom = false;
	}
}

void CThumper::Reload()
{
	if (m_iClip >= THUMPER_MAX_CLIP)
		return;

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;

	if (m_bInZoom)
		SecondaryAttack();

	DefaultReload(1, THUMPER_RELOAD1, 1.70);
}

void CThumper::WeaponIdle()
{
	ResetEmptySound();

	m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	if (m_flTimeWeaponIdle > gpGlobals->time)
		return;

	SendWeaponAnim(THUMPER_IDLE1);
	m_flTimeWeaponIdle = gpGlobals->time + RANDOM_FLOAT(10.0f, 15.0f);
}
#endif //_HALO
