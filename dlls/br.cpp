/****
*
* Copyright (c) 2021-2025 The Phoenix Project Software. Some Rights Reserved.
*
* AURA
*
* br: the burst-fire precision weapon in ZAMNHLMP & Excession. great for swat!
*
*
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "weapon_hierarchy.h"
#include "gamerules.h"

LINK_ENTITY_TO_CLASS(weapon_br, CBattleRifle);
#ifdef _HALO
LINK_ENTITY_TO_CLASS(weapon_olr, CBattleRifle);
#endif
LINK_ENTITY_TO_CLASS(weapon_battlerifle, CBattleRifle);

void CBattleRifle::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_br");
	Precache();
	m_iId = WEAPON_BATTLERIFLE;
	SET_MODEL(ENT(pev), "models/weapons/br/w_br.mdl");
	m_iDefaultAmmo = BR_DEFAULT_GIVE;
	FallInit(); // get ready to fall down.
}

void CBattleRifle::Precache(void)
{
	PRECACHE_MODEL("models/weapons/br/v_br.mdl");
	PRECACHE_MODEL("models/weapons/br/w_br.mdl");
	PRECACHE_MODEL("models/weapons/br/p_br.mdl");

	PRECACHE_SOUND("weapons/olr1.wav");
	PRECACHE_SOUND("weapons/ol2.wav");

	m_usOLR = PRECACHE_EVENT(1, "events/olr.sc");
}

int CBattleRifle::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "brammo";
	p->iMaxAmmo1 = BR_MAX_CARRY;
	p->iMaxClip = BR_MAX_CLIP;
	p->iSlot = WPN_AUTO_SLOT;
	p->iPosition = WPN_BR_POS;
	p->iId = WEAPON_BATTLERIFLE;
	p->iFlags = 0;
	p->iWeight = 20;
	return 1;
}

void CBattleRifle::PrimaryAttack(void)
{
	if (m_iClip <= 0)
	{
		Reload();
		return;
	}

	m_iBurstShotsFired = 0;
	FireBurstShot();

	SetThink(&CBattleRifle::BurstThink);
	pev->nextthink = gpGlobals->time + 0.06f;

	m_flNextPrimaryAttack = gpGlobals->time + 0.32f;
}

void CBattleRifle::FireBurstShot(void)
{
	if (m_iClip <= 0)
		return;

	CBasePlayer* pPlayer = (CBasePlayer*)m_pPlayer;
	m_iClip--;

#ifndef CLIENT_DLL
	Vector vecSrc = pPlayer->GetGunPosition();
	Vector vecAiming = gpGlobals->v_forward;
	Vector vecSpread = VECTOR_CONE_2DEGREES;
#else
	Legacy_Vector vecSrc = pPlayer->GetGunPosition();
	Legacy_Vector vecAiming = gpGlobals->v_forward;
	Legacy_Vector vecSpread = VECTOR_CONE_2DEGREES;
#endif

	pPlayer->FireBullets(1, vecSrc, vecAiming, vecSpread,
		8192, BULLET_PLAYER_OLR, 0, 26);

	PLAYBACK_EVENT_FULL(FEV_NOTHOST, pPlayer->edict(), m_usOLR,
		0.0, (float*)&g_vecZero, (float*)&g_vecZero,
		vecSpread.x, vecSpread.y,
		m_iClip, 0, pev->body, 0);
}

void CBattleRifle::BurstThink(void)
{
	m_iBurstShotsFired++;

	if (m_iBurstShotsFired < 3 && m_iClip > 0)
	{
		FireBurstShot();
		pev->nextthink = gpGlobals->time + 0.06f;
	}
	else
	{
		// burst finished
		SetThink(&CBattleRifle::WeaponIdle);
		pev->nextthink = gpGlobals->time + 0.1f;
	}
}

BOOL CBattleRifle::Deploy(void)
{
	return DefaultDeploy("models/weapons/br/v_br.mdl", "models/weapons/br/p_br.mdl", OLR_DEPLOY, "olr");
}

void CBattleRifle::Reload(void)
{
	if (m_pPlayer->ammo_br <= 0)
		return;

	DefaultReload(BR_MAX_CLIP, OLR_RELOAD, 1.5);
}

void CBattleRifle::WeaponIdle(void)
{
	ResetEmptySound();

	m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	int iAnim;
	switch (RANDOM_LONG(0, 1))
	{
	case 0:
		iAnim = OLR_LONGIDLE;
		break;
	default:
	case 1:
		iAnim = OLR_IDLE1;
		break;
	}

	SendWeaponAnim(iAnim);

	m_flTimeWeaponIdle = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
}

class CAmmoBR85 : public CBasePlayerAmmo
{
	void Spawn(void) { Precache(); SET_MODEL(ENT(pev), "models/w_9mmARclip.mdl"); CBasePlayerAmmo::Spawn(); }
	void Precache(void) { PRECACHE_MODEL("models/w_9mmARclip.mdl"); PRECACHE_SOUND("items/9mmclip1.wav"); }
	BOOL AddAmmo(CBaseEntity* pOther) { return (pOther->GiveAmmo(36, "brammo", 180) != -1); }
};
LINK_ENTITY_TO_CLASS(ammo_br, CAmmoBR85);