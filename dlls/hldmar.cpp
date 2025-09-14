/****
*
* Copyright (c) 2021-2025 The Phoenix Project Software. Some Rights Reserved.
*
* AURA
*
* hldmar: kind of like the original MP5.
*
*
****/

#ifndef _HALO
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "weapon_hierarchy.h"
#include "nodes.h"
#include "player.h"
#include "soundent.h"
#include "gamerules.h"
//++ BulliT
#ifdef AGSTATS
#include "agstats.h"
#endif

enum hldmar_e
{
	MP5_LONGIDLE = 0,
	MP5_IDLE1,
	MP5_LAUNCH,
	MP5_RELOAD,
	MP5_DEPLOY,
	MP5_FIRE1,
	MP5_FIRE2,
	MP5_FIRE3,
};

LINK_ENTITY_TO_CLASS(weapon_hldmar, CHLDMAR);

int CHLDMAR::SecondaryAmmoIndex(void)
{
	return m_iSecondaryAmmoType;
}

void CHLDMAR::Spawn()
{
#ifndef CLIENT_DLL
	if (SGBOW == AgGametype())
	{
		//Spawn shotgun instead.
		CBaseEntity* pNewWeapon = CBaseEntity::Create("weapon_shotgun", g_pGameRules->VecWeaponRespawnSpot(this), pev->angles, pev->owner);
		return;
	}
#endif
	pev->classname = MAKE_STRING("weapon_hldmar"); // hack to allow for old namez
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmAR.mdl");
	m_iId = WEAPON_HLDMAR;

	m_iDefaultAmmo = HLDMAR_DEFAULT_GIVE;

	FallInit(); //get read to fall down.
}

void CHLDMAR::Precache(void)
{
	PRECACHE_MODEL("models/weapons/hldmar/v_hldmar.mdl");
	PRECACHE_MODEL("models/w_9mmAR.mdl");
	PRECACHE_MODEL("models/p_9mmAR.mdl");

	m_iShell = PRECACHE_MODEL("models/shell.mdl");// brass shellTE_MODEL

	PRECACHE_MODEL("models/grenade.mdl");	// grenade

	PRECACHE_MODEL("models/w_9mmARclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");

	PRECACHE_SOUND("items/clipinsert1.wav");
	PRECACHE_SOUND("items/cliprelease1.wav");

	PRECACHE_SOUND("weapons/hldmar_fire01.wav");
	PRECACHE_SOUND("weapons/hldmar_fire02.wav");
	PRECACHE_SOUND("weapons/hldmar_fire03.wav");
	PRECACHE_SOUND("weapons/hldmar_fire04.wav");

	PRECACHE_SOUND("weapons/hldmar_grenade01.wav");
	PRECACHE_SOUND("weapons/glauncher2.wav");

	PRECACHE_SOUND("weapons/357_cock.wav");

	m_usMP5 = PRECACHE_EVENT(1, "events/hldmar.sc");
	m_usMP52 = PRECACHE_EVENT(1, "events/hldmar2.sc");
}

int CHLDMAR::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "9mm";
	p->iMaxAmmo1 = _9MM_MAX_CARRY;
	p->pszAmmo2 = "ARgrenades";
	p->iMaxAmmo2 = M203_GRENADE_MAX_CARRY;
	p->iMaxClip = HLDMAR_MAX_CLIP;
	p->iSlot = WPN_AUTO_SLOT;
	p->iPosition = WPN_HLDMAR_POS;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_HLDMAR;
	p->iWeight = HLDMAR_WEIGHT;

	return 1;
}

int CHLDMAR::AddToPlayer(CBasePlayer* pPlayer)
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

BOOL CHLDMAR::Deploy()
{
	return DefaultDeploy("models/weapons/hldmar/v_hldmar.mdl", "models/p_9mmAR.mdl", MP5_DEPLOY, "mp5");
}

void CHLDMAR::PrimaryAttack()
{
	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = 0.15;
		return;
	}

	if (m_iClip <= 0)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = 0.15;
		return;
	}

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	m_iClip--;

	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	// player "shoot" animation
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

#ifndef CLIENT_DLL
	Vector vecOffset = Vector(0, -0.08, 0);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);
	Vector vecDir;
#else
	Legacy_Vector vecOffset = Legacy_Vector(0, -0.08, 0);
	Legacy_Vector vecSrc = m_pPlayer->GetGunPosition();
	Legacy_Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);
	Legacy_Vector vecDir;
#endif

	vecDir = m_pPlayer->FireBulletsPlayer(1, vecSrc, vecAiming, VECTOR_CONE_3DEGREES, 8192, BULLET_PLAYER_MP5, 2, 0, m_pPlayer->pev, m_pPlayer->random_seed);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(0, m_pPlayer->edict(), m_usMP5, 0.0, (float*)&g_vecZero, (float*)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0);

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	m_flNextPrimaryAttack = GetNextAttackDelay(0.1);

	if (m_flNextPrimaryAttack < UTIL_WeaponTimeBase())
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
}

void CHLDMAR::SecondaryAttack(void)
{
	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = 0.15;
		return;
	}

	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 0)
	{
		PlayEmptySound();
		return;
	}

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	m_pPlayer->m_iExtraSoundTypes = bits_SOUND_DANGER;
	m_pPlayer->m_flStopExtraSoundTime = UTIL_WeaponTimeBase() + 0.2;

	m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;

	// player "shoot" animation
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

#ifndef CLIENT_WEAPONS
	SendWeaponAnim(MP5_LAUNCH);

	if (RANDOM_LONG(0, 1))
	{
		// play this sound through BODY channel so we can hear it if player didn't stop firing MP3
		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/glauncher.wav", 0.8, ATTN_NORM);
	}
	else
	{
		// play this sound through BODY channel so we can hear it if player didn't stop firing MP3
		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/glauncher2.wav", 0.8, ATTN_NORM);
	}
#endif


	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	// we don't add in player velocity anymore.
	CGrenade::ShootContact(m_pPlayer->pev,
		m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16,
		gpGlobals->v_forward * 800);

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT(0, m_pPlayer->edict(), m_usMP52);

	m_flNextPrimaryAttack = GetNextAttackDelay(1);
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5;// idle pretty soon after shooting.

	if (!m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType])
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

#ifndef CLIENT_WEAPONS
	m_pPlayer->pev->punchangle.x -= 10;
#endif
}

void CHLDMAR::Reload(void)
{
	if (m_pPlayer->ammo_9mm <= 0)
		return;

	DefaultReload(HLDMAR_MAX_CLIP, MP5_RELOAD, 1.5);
}

void CHLDMAR::WeaponIdle(void)
{
	ResetEmptySound();

	m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	int iAnim;
	switch (RANDOM_LONG(0, 1))
	{
	case 0:
		iAnim = MP5_LONGIDLE;
		break;

	default:
	case 1:
		iAnim = MP5_IDLE1;
		break;
	}

	SendWeaponAnim(iAnim);

	m_flTimeWeaponIdle = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15); // how long till we do this again.
}

// no ammo because we'll use the 9mmAR's ammo.
#endif
