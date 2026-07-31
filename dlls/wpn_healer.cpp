/****
*
* Copyright (c) 2021-2026 The Phoenix Project Software. Some Rights Reserved.
*
* AURA
*
* healer weapon - heal a teammate or friendly npc
*
*
****/

#ifndef _HALO

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "weapon_hierarchy.h"
#include "monsters.h"
#include "player.h"
#include "effects.h"
#include "decals.h"
#include "gamerules.h"

LINK_ENTITY_TO_CLASS(weapon_healer, CHealer);

void CHealer::Spawn(void)
{
	Precache();

	SET_MODEL(ENT(pev), "models/w_medkit.mdl"); // we typically won't find this in the world
	m_iId = WEAPON_HEALER;
	m_iDefaultAmmo = HEALER_MAX_CARRY;

	FallInit();
}

void CHealer::Precache(void)
{
	PRECACHE_MODEL("models/v_medkit.mdl");
	PRECACHE_MODEL("models/p_medkit.mdl");
	PRECACHE_MODEL("models/w_medkit.mdl");

	PRECACHE_SOUND("weapons/healer/health1.wav");
	PRECACHE_SOUND("weapons/healer/revive.wav");
	PRECACHE_SOUND("items/medshotno1.wav");
}

int CHealer::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "medcharge";
	p->iMaxAmmo1 = HEALER_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = WPN_MELEE_SLOT;
	p->iPosition = WPN_HEALER_POS;
	p->iFlags = 0;
	p->iId = WEAPON_HEALER;
	p->iWeight = -5;

	return 1;
}

int CHealer::AddToPlayer(CBasePlayer* pPlayer)
{
	if (CBasePlayerWeapon::AddToPlayer(pPlayer))
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev);
		WRITE_BYTE(m_iId);
		MESSAGE_END();

		return TRUE;
	}

	return FALSE;
}

BOOL CHealer::Deploy(void)
{
	return DefaultDeploy("models/v_medkit.mdl", "models/p_medkit.mdl", HEALER_DRAW, "crowbar");
}

void CHealer::Holster(int skiplocal)
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5f;
	SendWeaponAnim(HEALER_HOLSTER);
}

CBaseEntity* CHealer::FindMedkitTarget(float flRange)
{
	if (!m_pPlayer) return NULL;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle);

	Legacy_Vector vecSrc = m_pPlayer->GetGunPosition();
	Legacy_Vector vecEnd = vecSrc + gpGlobals->v_forward * flRange;

	TraceResult tr;
	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

	if (tr.flFraction >= 1.0f || FNullEnt(tr.pHit))
		return NULL;

	return CBaseEntity::Instance(tr.pHit);
}

#ifndef CLIENT_DLL
bool CHealer::IsFriendlyPlayer(CBaseEntity* pEntity)
{
	if (!pEntity || !pEntity->IsPlayer())
		return false;

	CBasePlayer* pOther = (CBasePlayer*)pEntity;

	if (pOther->IsTeammate(pEntity))
		return true;

	return false;
}

bool CHealer::IsFriendlyMonster(CBaseMonster* pMonster)
{
	if (!pMonster)
		return false;

	if (pMonster->IRelationship(m_pPlayer) < R_NO)
		return true;

	return false;
}

bool CHealer::CanHealTarget(CBaseEntity* pTarget)
{
	if (!pTarget)
		return false;

	if (pTarget->pev->deadflag != DEAD_NO)
		return false;
	if (pTarget->pev->health <= 0)
		return false;

	if (pTarget->IsPlayer())
	{
		if (!IsFriendlyPlayer(pTarget))
			return false;

		if (pTarget->pev->health >= pTarget->pev->max_health)
			return false;

		return true;
	}

	CBaseMonster* pMonster = pTarget->MyMonsterPointer();
	if (pMonster)
	{
		if (!IsFriendlyMonster(pMonster))
			return false;

		if (pMonster->pev->health >= pMonster->pev->max_health)
			return false;

		return true;
	}

	return false;
}

bool CHealer::CanReviveTarget(CBaseMonster* pMonster)
{
	if (!pMonster)
		return false;

	// must be dead
	if (pMonster->pev->deadflag == DEAD_NO || pMonster->pev->deadflag == DEAD_DYING)
		return false;

	if (pMonster->pev->health > 0)
		return false;

	if (!IsFriendlyMonster(pMonster))
		return false;

	if (pMonster->pev->effects & EF_NODRAW)
		return false;

	if (pMonster->pev->iuser1 != 1)
		return false;

	if (pMonster->pev->iuser2 == 1)
		return false;

	return true;
}

void CHealer::HealTarget(CBaseEntity* pTarget, float flAmount)
{
	if (!pTarget)
		return;

	float flNewHealth = pTarget->pev->health + flAmount;
	if (flNewHealth > pTarget->pev->max_health)
		flNewHealth = pTarget->pev->max_health;

	pTarget->pev->health = flNewHealth;
}

bool CHealer::CheckReviveHullClear(const Legacy_Vector& vecOrigin, edict_t* pentIgnore)
{
	TraceResult tr;

	UTIL_TraceHull(vecOrigin, vecOrigin, dont_ignore_monsters, human_hull, pentIgnore, &tr);

	if (tr.fStartSolid || tr.fAllSolid)
		return false;

	return true;
}

bool CHealer::ReviveTarget(CBaseMonster* pCorpse)
{
	if (!pCorpse)
		return false;

	Legacy_Vector vecOrigin = pCorpse->pev->origin;
	Legacy_Vector vecAngles = pCorpse->pev->angles;
	string_t iszClassname = pCorpse->pev->noise1;

	if (!iszClassname || !STRING(iszClassname) || !STRING(iszClassname)[0])
		return false;

	if (!CheckReviveHullClear(vecOrigin, ENT(m_pPlayer->pev)))
		return false;

	CBaseEntity* pNewEnt = CBaseEntity::Create((char*)STRING(iszClassname), vecOrigin, vecAngles, ENT(m_pPlayer->pev));
	if (!pNewEnt)
		return false;

	CBaseMonster* pNewMonster = pNewEnt->MyMonsterPointer();
	if (!pNewMonster)
	{
		UTIL_Remove(pNewEnt);
		return false;
	}

	pNewMonster->pev->body = pCorpse->pev->body;
	pNewMonster->pev->skin = pCorpse->pev->skin;
	pNewMonster->pev->health = pNewMonster->pev->max_health * 0.5f;

	UTIL_Remove(pCorpse);
	return true;
}
#endif

void CHealer::PlayFailSound()
{
	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "items/medshotno1.wav", 1.0f, ATTN_NORM, 0, PITCH_NORM);
}

void CHealer::PlayHealSound()
{
	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/healer/health1.wav", 1.0f, ATTN_NORM, 0, PITCH_NORM);
}

void CHealer::PlayReviveSound()
{
	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/healer/revive.wav", 1.0f, ATTN_NORM, 0, PITCH_NORM);
}

void CHealer::PrimaryAttack(void)
{
	if (!m_pPlayer)
		return;

#ifndef CLIENT_DLL

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] < 5)
	{
		PlayFailSound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.3f;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3f;
		return;
	}

	CBaseEntity* pTarget = FindMedkitTarget(64.0f);

	if (!pTarget || !CanHealTarget(pTarget))
	{
		PlayFailSound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.3f;
		return;
	}

	HealTarget(pTarget, 15.0f);
	m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= 5;

	PlayHealSound();

#endif

	SendWeaponAnim(1);
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5f;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5f;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0f;
}

void CHealer::SecondaryAttack(void)
{
	if (!m_pPlayer)
		return;

#ifndef CLIENT_DLL

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] < 25)
	{
		PlayFailSound();
		goto fail;
	}

	CBaseEntity* pTarget = FindMedkitTarget(64.0f);
	if (!pTarget)
		goto fail;

	CBaseMonster* pMonster = pTarget->MyMonsterPointer();
	if (!pMonster || !CanReviveTarget(pMonster))
		goto fail;

	if (!ReviveTarget(pMonster))
		goto fail;

	m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= 25;
	PlayReviveSound();

	goto success;

fail:
	PlayFailSound();

success:
#endif

	SendWeaponAnim(2);
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0f;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0f;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5f;
}

void CHealer::WeaponIdle(void)
{
	ResetEmptySound();

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	SendWeaponAnim(0); // idle anim
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0f;
}

#endif // _HALO
