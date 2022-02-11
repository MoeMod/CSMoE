/***
*	Made by MoeMod & TmNine!~
*	Modification by JustANoobQAQ
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
#include "wpn_desperado.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

enum desperado_e
{
	IDLE_M = 0,
	RUN_START_M,
	RUN_IDLE_M,
	RUN_END_M,
	DRAW_M,
	SHOOT1_M,
	RELOAD_M,
	SWAB_M,
	IDLE_W,
	RUN_START_W,
	RUN_IDLE_W,
	RUN_END_W,
	DRAW_W,
	SHOOT1_W,
	RELOAD_W,
	SWAB_W
};

LINK_ENTITY_TO_CLASS(weapon_desperado, CDesperado)

void CDesperado::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_desperado");
	Precache();
	m_iId = WEAPON_ELITE;
	SET_MODEL(ENT(pev), "models/w_desperado.mdl");

	m_iDefaultAmmo = DESPERADO_MAX_CLIP;
	m_iShotsFired = 0;
	pev->iuser1 = 0;
	m_flAccuracy = 1.0;

	FallInit();

	ResetRunIdle();
	m_iClip2 = m_iBackupClip = m_iDefaultAmmo;

	
}

void CDesperado::Precache(void)
{
	PRECACHE_MODEL("models/v_desperado.mdl");
	PRECACHE_MODEL("models/w_desperado.mdl");
	PRECACHE_MODEL("models/p_desperado_m.mdl");
	PRECACHE_MODEL("models/p_desperado_w.mdl");

	PRECACHE_MODEL("sprites/muzzleflash59.spr");
	PRECACHE_MODEL("sprites/muzzleflash60.spr");

	PRECACHE_SOUND("weapons/dprd-1.wav");

	m_iShell = PRECACHE_MODEL("models/pshell.mdl");
	m_usFireDesperado = PRECACHE_EVENT(1, "events/desperado.sc");
}

int CDesperado::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "44magnum";
	p->iMaxAmmo1 = 200;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = DESPERADO_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 5;
	p->iId = m_iId = WEAPON_ELITE;
	p->iFlags = 0;
	p->iWeight = ELITE_WEIGHT;

	return 1;
}

BOOL CDesperado::Deploy(void)
{
	ResetRunIdle();
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.17s;
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.01s;

	m_flAccuracy = 1.0f;
	m_iShotsFired = 0;
	m_fMaxSpeed = 250.0f;

	if (pev->iuser1)
		return DefaultDeploy("models/v_desperado.mdl", "models/p_desperado_w.mdl", GetCorrectedAnim(DRAW_M), "onehanded", UseDecrement() != FALSE, 0.0s, 0.2s);
	else
		return DefaultDeploy("models/v_desperado.mdl", "models/p_desperado_m.mdl", GetCorrectedAnim(DRAW_M), "onehanded", UseDecrement() != FALSE, 0.0s, 0.2s);
}

void CDesperado::SecondaryAttack(void)
{
	if (m_pPlayer->pev->button & IN_ATTACK)
		return;

	if (pev->iuser1 != 1)
	{
		SendWeaponAnim(GetCorrectedAnim(SWAB_M), UseDecrement() != FALSE);

		int iClipDiff = iMaxClip() - m_iClip;

		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= iClipDiff;

		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] < 0)
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = 0;

		if (iClipDiff < m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		{
			m_iClip2 = iClipDiff + m_iClip;
		}
		else
		{
			m_iClip2 = m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] + m_iClip;
		}

		pev->iuser1 = 1;
		m_iClip = m_iBackupClip;
		m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_desperado_w.mdl");
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.17s;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.17s;

		ResetRunIdle();
		m_flAccuracy = 1.0;

		return;

	}

	if (IsEmpty())
		return;

	float flSpread = 0.0;

	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
	{
		flSpread = (1.0 - m_flAccuracy) * 0.15;
	}
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
	{
		flSpread = (1.0 - m_flAccuracy) * 0.12;
	}
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
	{
		flSpread = (1.0 - m_flAccuracy) * 0.1;
	}
	else
	{
		flSpread = (1.0 - m_flAccuracy) * 0.11;
	}

	if (m_flLastFire != invalid_time_point)
	{
		m_flAccuracy -= (0.3s - (gpGlobals->time - m_flLastFire)) /1s * 0.2;

		if (m_flAccuracy > 1.0)
			m_flAccuracy = 1.0;
		else if (m_flAccuracy < 0.5)
			m_flAccuracy = 0.5;
	}

	m_flLastFire = gpGlobals->time;

	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

	UTIL_MakeVectors(m_pPlayer->pev->punchangle + m_pPlayer->pev->v_angle);

	m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDirShooting = gpGlobals->v_forward;

	Vector vecDir = m_pPlayer->FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192, 6, BULLET_PLAYER_50AE, GetDamage(), 0.9, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);

	AddKickRateToLineTargets(vecSrc, vecDirShooting);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFireDesperado, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, pev->iuser1);

	m_iClip--;

	if (m_iClip <= 0)
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.6s;
	else
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.1s;

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase();
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.6s;

	m_pPlayer->pev->punchangle.x -= 0.33;

	ResetRunIdle();
}

void CDesperado::PrimaryAttack(void)
{
	if (m_pPlayer->pev->button & IN_ATTACK2)
		return;

	if (pev->iuser1)
	{
		SendWeaponAnim(GetCorrectedAnim(SWAB_M), UseDecrement() != FALSE);

		int iClipDiff = iMaxClip() - m_iClip;

		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= iClipDiff;

		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] < 0)
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = 0;

		if (iClipDiff < m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		{
			m_iBackupClip = iClipDiff + m_iClip;
		}
		else
		{
			m_iBackupClip = m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] + m_iClip;
		}

		pev->iuser1 = 0;
		m_iClip = m_iClip2;

		m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_desperado_m.mdl");
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.17s;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.17s;

		ResetRunIdle();
		m_flAccuracy = 1.0;

		return;
	}

	if (IsEmpty())
		return;

	float flSpread = 0.0f;
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
	{
		flSpread = (1.0 - m_flAccuracy) * 0.15;
	}
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
	{
		flSpread = (1.0 - m_flAccuracy) * 0.12;
	}
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
	{
		flSpread = (1.0 - m_flAccuracy) * 0.1;
	}
	else
	{
		flSpread = (1.0 - m_flAccuracy) * 0.11;
	}

	if (m_flLastFire != invalid_time_point)
	{
		m_flAccuracy -= (0.3 - ((gpGlobals->time - m_flLastFire) / 1s)) * 0.2;

		if (m_flAccuracy > 1.0)
			m_flAccuracy = 1.0;
		else if (m_flAccuracy < 0.5)
			m_flAccuracy = 0.5;
	}

	m_flLastFire = gpGlobals->time;

	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

	UTIL_MakeVectors(m_pPlayer->pev->punchangle + m_pPlayer->pev->v_angle);

	m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	Vector vecDir = m_pPlayer->FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192, 6, BULLET_PLAYER_50AE, GetDamage(), 0.9, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFireDesperado, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, pev->iuser1);

	m_iClip--;

	if (m_iClip <= 0)
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.6s;
	else
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1s;

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.0s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.6s;

	m_pPlayer->pev->punchangle.x -= 0.33;

	ResetRunIdle();
}

BOOL CDesperado::IsEmpty(void)
{
	if (m_iClip > 0)
		return FALSE;

	PlayEmptySound();

	SendWeaponAnim(GetCorrectedAnim(IDLE_M), UseDecrement() != FALSE);

	m_flNextSecondaryAttack = m_flNextPrimaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.2s;

	return TRUE;
}

void CDesperado::Reload(void)
{
	if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase())
		return;

	if (m_flNextSecondaryAttack > UTIL_WeaponTimeBase())
		return;

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;

	if (DefaultReload(iMaxClip(), GetCorrectedAnim(RELOAD_M), 0.47s))
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.67s;

#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif

		ResetRunIdle();
		m_flAccuracy = 1.0;
	}
}

void CDesperado::WeaponIdle(void)
{
	ResetEmptySound();

	if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
	{
		if (m_pPlayer->pev->button & IN_FORWARD)
		{
			if (m_bJustRunStart)
			{
				SendWeaponAnim(GetCorrectedAnim(RUN_START_M), UseDecrement() != FALSE);

				m_bJustRunStart = false;
				m_flNextRunIdle = UTIL_WeaponTimeBase();
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.33s;

				return;
			}
			else if (m_flNextRunIdle <= UTIL_WeaponTimeBase())
			{
				SendWeaponAnim(GetCorrectedAnim(RUN_IDLE_M), UseDecrement() != FALSE);

				m_flNextRunIdle = UTIL_WeaponTimeBase() + 0.57s;
			}

			m_flNextRunEnd = UTIL_WeaponTimeBase();
		}
		else
		{
			if (!m_bJustRunStart)
			{
				SendWeaponAnim(GetCorrectedAnim(RUN_END_M), UseDecrement() != FALSE);

				m_bJustRunStart = true;
				m_flNextRunEnd = UTIL_WeaponTimeBase();
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.3s;

				return;
			}
			else if (m_flNextRunEnd <= UTIL_WeaponTimeBase())
			{
				SendWeaponAnim(GetCorrectedAnim(IDLE_M), UseDecrement() != FALSE);

				m_flNextRunEnd = UTIL_WeaponTimeBase() + 3.0s;
			}

			m_flNextRunIdle = UTIL_WeaponTimeBase();
		}

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.1s;
	}
}

void CDesperado::ResetRunIdle()
{
	m_bJustRunStart = true;
	m_flNextRunEnd = UTIL_WeaponTimeBase();
	m_flNextRunIdle = UTIL_WeaponTimeBase();
}

int CDesperado::GetCorrectedAnim(int sequence)
{
	if (pev->iuser1 == 1)
		return sequence + 8;

	return sequence;
}

int CDesperado::GetDamage() const
{
	int iDamage = 40;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		iDamage = 95;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		iDamage = 95;
#endif
	return iDamage;
}

void CDesperado::AddKickRateToLineTargets(Vector vecSrc, Vector vecDir)
{
	TraceResult tr;

	UTIL_TraceLine(vecSrc, vecSrc + vecDir * 8192.0, dont_ignore_monsters, m_pPlayer->edict(), &tr);

	if (tr.flFraction < 1.0 && !FNullEnt(tr.pHit))
	{
		CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

		if (pEntity && pEntity->IsPlayer())
		{
			CBasePlayer* player = static_cast<CBasePlayer*>(pEntity);
			if (player->m_bIsZombie)
			{
				Vector2D dir = (pEntity->pev->origin - m_pPlayer->pev->origin).Make2D().Normalize();

				player->pev->velocity.x = dir.x * 300.0;
				player->pev->velocity.y = dir.y * 300.0;
				player->pev->velocity.z = 100.0;

				player->m_flVelocityModifier = 0.85;
			}
		}
	}
}

}
