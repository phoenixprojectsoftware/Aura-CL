/****
*
* Copyright (c) 2021-2025 The Phoenix Project Software. Some Rights Reserved.
*
* AURA
*
* railgun: super cool laser thingy
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

enum railgun_e
{
	RAILGUN_IDLE = 0,
	RAILGUN_FIRE,
	RAILGUN_DRAW,
	RAILGUN_HOLSTER,
};

// LINK_ENTITY_TO_CLASS(weapon_railgun, CRailgun);

void CRailgun::Spawn()
{
	Precache();
	m_iId = WEAPON_RAILGUN;
	SET_MODEL(ENT(pev), "models/w_railgun.mdl");

	m_iDefaultAmmo = RAILGUN_DEFAULT_GIVE;

	FallInit();
}

void CRailgun::Precache(void)
{
	PRECACHE_MODEL("models/w_railgun.mdl");
	PRECACHE_MODEL("models/p_railgun.mdl");
	PRECACHE_MODEL("models/v_railgun.mdl");

	PRECACHE_SOUND("weapons/railgun.wav");

	UTIL_PrecacheOther("railgun_ray");

	m_usRailgun = PRECACHE_EVENT(1, "events/tf_rail.sc");
}

int CRailgun::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "uranium";
	p->iMaxAmmo1 = URANIUM_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = WPN_AUTO_SLOT;
	p->iPosition = WPN_RAILGUN_POS;
	p->iId = WEAPON_RAILGUN;
	p->iFlags = 0;
	p->iWeight = GAUSS_WEIGHT;

	return 1;
}

void CRailgun::Fire(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	{
		PlayEmptySound();
		return;
	}

	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound();
		return;
	}

	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
	m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;

#ifndef CLIENT_DLL
	Vector vecSrc = m_pPlayer->GetGunPosition() + gpGlobals->v_right * 4 - gpGlobals->v_up * 4 + gpGlobals->v_forward * 2;
	Vector vecDir = m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);
#else
	Legacy_Vector vecSrc = m_pPlayer->GetGunPosition() + gpGlobals->v_right * 4 - gpGlobals->v_up * 4 + gpGlobals->v_forward * 2;
	Legacy_Vector vecDir = m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);
#endif

#ifndef CLIENT_DLL
	CBaseEntity* pRay = CBaseEntity::Create("raingun_ray", vecSrc, m_pPlayer->pev->v_angle, m_pPlayer->edict());
	pRay->pev->velocity = pRay->pev->vuser1 = vecDir * 2000;
	pRay->pev->angles = UTIL_VecToAngles(pRay->pev->velocity);
#endif

	PLAYBACK_EVENT_FULL(0, edict(), m_usRailgun, 0.0, (float*)&g_vecZero, (float*)&g_vecZero, 0.0, 0.0, 0, 0, 0, 0);
}

void CRailgun::PrimaryAttack(void)
{
	if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase())
	{
		return;
	}

	Fire();

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;
}

void CRailgun::SecondaryAttack(void)
{
	if (m_flNextSecondaryAttack > UTIL_WeaponTimeBase())
	{
		return;
	}

	Fire();

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;

}

BOOL CRailgun::Deploy()
{
	return DefaultDeploy("models/v_railgun.mdl", "models/p_railgun.mdl", RAILGUN_DRAW, "hive");
}

void CRailgun::Holster(int skiplocal)
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	SendWeaponAnim(RAILGUN_HOLSTER);
}

void CRailgun::WeaponIdle(void)
{
	m_pPlayer->GetAutoaimVector(AUTOAIM_2DEGREES);
	ResetEmptySound();

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	SendWeaponAnim(RAILGUN_IDLE);
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_WeaponTimeBase() + 31.0 / 10.0;
}

#endif // _HALO
