/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
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
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"


#define	FRUIT_PRIMARY_VOLUME		450

enum fruit_e {
	FRUIT_IDLE = 0,
	FRUIT_FIDGET,
	FRUIT_PINPULL,
	FRUIT_THROW1,	// toss
	FRUIT_THROW2,	// medium
	FRUIT_THROW3,	// hard
	FRUIT_HOLSTER,
	FRUIT_DRAW
};


LINK_ENTITY_TO_CLASS(weapon_fruit, CFruitGrenade);


void CFruitGrenade::Spawn()
{
	Precache();
	m_iId = WEAPON_FRUIT;
	SET_MODEL(ENT(pev), "models/weapons/w_fruit.mdl");

#ifndef CLIENT_DLL
	pev->dmg = gSkillData.plrDmgHandGrenade;
#endif

	m_iDefaultAmmo = HANDGRENADE_DEFAULT_GIVE * 2;

	FallInit();// get ready to fall down.
}


void CFruitGrenade::Precache(void)
{
	PRECACHE_MODEL("models/weapons/w_fruit.mdl");
	PRECACHE_MODEL("models/weapons/v_fruit.mdl");
	PRECACHE_MODEL("models/weapons/p_fruit.mdl");
}

int CFruitGrenade::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "Fruit Grenade";
	p->iMaxAmmo1 = 25;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 4;
	p->iPosition = 7;
	p->iId = m_iId = WEAPON_FRUIT;
	p->iWeight = HANDGRENADE_WEIGHT * 3;
	p->iFlags = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;

	return 1;
}

void CFruitGrenade::SetBodygroups()
{
	if (!m_pPlayer || !m_pPlayer->pev->viewmodel)
		return;

	// edict_t* pViewModel = m_pPlayer->pev->viewmodel;

	const int GROUP_RIGHT_HAND = 1;
	const int GROUP_FRUIT = 2;

	const int SUB_RIGHT_HAND = 0;
	const int SUB_FRUIT = 5;

	SetBodygroup(GROUP_RIGHT_HAND, SUB_RIGHT_HAND);
	SetBodygroup(GROUP_FRUIT, SUB_FRUIT);
}

BOOL CFruitGrenade::Deploy()
{
	m_flReleaseThrow = -1;
	BOOL bResult = DefaultDeploy("models/weapons/v_fruit.mdl", "models/weapons/p_fruit.mdl", FRUIT_DRAW, "crowbar");

	if (bResult)
	{
		SetBodygroups();
	}

	return bResult;
}

BOOL CFruitGrenade::CanHolster(void)
{
	// can only holster hand grenades when not primed!
	return (m_flStartThrow == 0);
}

void CFruitGrenade::Holster(int skiplocal /* = 0 */)
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		SendWeaponAnim(FRUIT_HOLSTER);
	}
	else
	{
		// no more grenades!
		m_pPlayer->pev->weapons &= ~(1 << WEAPON_FRUIT);
		SetThink(&CFruitGrenade::DestroyItem);
		pev->nextthink = gpGlobals->time + 0.1;
	}

	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "common/null.wav", 1.0, ATTN_NORM);
}

void CFruitGrenade::PrimaryAttack()
{
	if (!m_flStartThrow && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0)
	{
		m_flStartThrow = gpGlobals->time;
		m_flReleaseThrow = 0;

		SendWeaponAnim(FRUIT_PINPULL);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
	}
}


void CFruitGrenade::WeaponIdle(void)
{
	if (m_flReleaseThrow == 0 && m_flStartThrow)
		m_flReleaseThrow = gpGlobals->time;

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_flStartThrow)
	{
		Legacy_Vector angThrow = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;

		if (angThrow.x < 0)
			angThrow.x = -10 + angThrow.x * ((90 - 10) / 90.0);
		else
			angThrow.x = -10 + angThrow.x * ((90 + 10) / 90.0);

		static float flMultiplier = 6.5f;
		float flVel = (90 - angThrow.x) * flMultiplier;
		if (flVel > 1000)
			flVel = 1000;

		UTIL_MakeVectors(angThrow);

		Legacy_Vector vecSrc = m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16;

		Legacy_Vector vecThrow = gpGlobals->v_forward * flVel + m_pPlayer->pev->velocity;

		// alway explode 3 seconds after the pin was pulled
		float time = m_flStartThrow - gpGlobals->time + 3.0;
		if (time < 0)
			time = 0;

		CGrenade::FruitTimed(m_pPlayer->pev, vecSrc, vecThrow, time);

		if (flVel < 500)
		{
			SendWeaponAnim(FRUIT_THROW1);
		}
		else if (flVel < 1000)
		{
			SendWeaponAnim(FRUIT_THROW2);
		}
		else
		{
			SendWeaponAnim(FRUIT_THROW3);
		}

		// player "shoot" animation
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

		m_flStartThrow = 0;
		m_flNextPrimaryAttack = GetNextAttackDelay(0.5);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;

		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;

		if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		{
			// just threw last grenade
			// set attack times in the future, and weapon idle in the future so we can see the whole throw
			// animation, weapon idle will automatically retire the weapon for us.
			m_flTimeWeaponIdle = m_flNextSecondaryAttack = m_flNextPrimaryAttack = GetNextAttackDelay(0.5);// ensure that the animation can finish playing
		}
		return;
	}
	else if (m_flReleaseThrow > 0)
	{
		// we've finished the throw, restart.
		m_flStartThrow = 0;

		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		{
			SendWeaponAnim(FRUIT_DRAW);
		}
		else
		{
			// RetireWeapon();
			return;
		}

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
		m_flReleaseThrow = -1;
		return;
	}

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		int iAnim;
		float flRand = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0, 1);
		if (flRand <= 0.75)
		{
			iAnim = FRUIT_IDLE;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);// how long till we do this again.
		}
		else
		{
			iAnim = FRUIT_FIDGET;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 75.0 / 30.0;
		}

		SendWeaponAnim(iAnim);
	}
}
