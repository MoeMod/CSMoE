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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "z4b_k1ases.h"


#ifndef CLIENT_DLL
#include "gamemode/zb2/deimostail.h"
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(z4b_k1ases, CK1ases)

const int K1ASE_MAXCLIP = 60;

void CK1ases::Spawn(void)
{
	pev->classname = MAKE_STRING("z4b_k1ases");

	Precache();
	m_iId = WEAPON_Z4B_K1ASES;
	SET_MODEL(ENT(pev), "models/w_k1ases.mdl");

	m_iDefaultAmmo = K1ASE_MAXCLIP;
	m_iDefaultAmmo2 = 20;
	FallInit();
}

void CK1ases::Precache(void)
{
	PRECACHE_MODEL("models/p_k1ases.mdl");
	PRECACHE_MODEL("models/v_k1ases.mdl");
	PRECACHE_MODEL("models/w_k1ases.mdl");

	PRECACHE_SOUND("weapons/k1ases-1.wav");
	PRECACHE_SOUND("weapons/deimos_skill_start.wav");

	m_iShell = PRECACHE_MODEL("models/rshell.mdl");
	m_usFireK1ases = PRECACHE_EVENT(1, "events/k1ases.sc");
}

int CK1ases::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "556Nato";
	p->iMaxAmmo1 = 90;
	p->pszAmmo2 = "DeimosTail";
	p->iMaxAmmo2 = 20;
	p->iMaxClip = K1ASE_MAXCLIP;
	p->iSlot = 0;
	p->iPosition = 17;
	p->iId = m_iId = WEAPON_Z4B_K1ASES;
	p->iFlags = 0;
	p->iWeight = GALIL_WEIGHT;

	return 1;
}

int CK1ases::ExtractAmmo(CBasePlayerWeapon* pWeapon)
{
	if (m_iDefaultAmmo2)
	{
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iDefaultAmmo2;
		m_iDefaultAmmo2 = 0;
	}
	return CBasePlayerWeapon::ExtractAmmo(pWeapon);
}

BOOL CK1ases::Deploy(void)
{
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;
	iShellOn = 1;

	return DefaultDeploy("models/v_k1ases.mdl", "models/p_k1ases.mdl", K1A_DRAW, "ak47", UseDecrement() != FALSE);
}
#ifndef CLIENT_DLL
void CK1ases::DelaySecondaryAttack(void)
{
	CDeimosTail* pTail = (CDeimosTail*)CBaseEntity::Create("deimostail", m_pPlayer->GetGunPosition(), m_pPlayer->pev->angles, m_pPlayer->edict());

	if (!pTail)
		return;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	pTail->m_bIsZombieSkill = false;
	pTail->pev->mins = { -1, -1, -1 };
	pTail->pev->maxs = { 1, 1, 1 };
	pTail->Initialize(3.0s);
	pTail->pev->velocity = gpGlobals->v_forward * 1000;
}

void CK1ases::SecondaryAttack(void)
{
	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
		}

		return;
	}
	m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;

	SendWeaponAnim(K1A_SKILL, UseDecrement() != FALSE);
	EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_VOICE, "zombi/deimos_skill_start.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);

	SetThink(&CK1ases::DelaySecondaryAttack);
	pev->nextthink = gpGlobals->time + 0.3s;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.0s;
}
#endif
void CK1ases::PrimaryAttack(void)
{
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15s;
		return;
	}

	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		K1asesFire(0.04 + (0.4) * m_flAccuracy, 0.0995s, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 140)
		K1asesFire(0.04 + (0.125) * m_flAccuracy, 0.0995s, FALSE);
	else
		K1asesFire((0.03) * m_flAccuracy, 0.0995s, FALSE);
}

void CK1ases::K1asesFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	m_bDelayFire = true;
	m_iShotsFired++;
	m_flAccuracy = ((float)(m_iShotsFired * m_iShotsFired * m_iShotsFired) / 200.0) + 0.35;

	if (m_flAccuracy > 1.75)
		m_flAccuracy = 1.75;

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
		}

		return;
	}

	m_iClip--;
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_556MM, GetDamage(), 0.98, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireK1ases, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.9s;

	if (m_pPlayer->pev->velocity.Length2D() > 0)
		KickBack(0.95, 0.4, 0.25, 0.03, 3.75, 2.75, 8);
	else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		KickBack(1.2, 0.575, 0.3, 0.04, 3.75, 3.5, 7);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		KickBack(0.35, 0.175, 0.15, 0.0175, 1.75, 1.5, 11);
	else
		KickBack(0.4, 0.25, 0.2, 0.025, 2.5, 1.7, 10);
}

void CK1ases::Reload(void)
{
	if (m_pPlayer->ammo_556nato <= 0)
		return;

	if (DefaultReload(K1ASE_MAXCLIP, K1A_RELOAD, 3.0s))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		m_flAccuracy = 0.2;
		m_iShotsFired = 0;
		m_bDelayFire = false;
	}
}

void CK1ases::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
	SendWeaponAnim(K1A_IDLE1, UseDecrement() != FALSE);
}

float CK1ases::GetDamage() const
{
	float flDamage = 45.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 70.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 70.0f;
#endif
	return flDamage;
}
}
