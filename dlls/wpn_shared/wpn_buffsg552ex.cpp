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
#include "wpn_buffsg552ex.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif
#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

enum buffsg552ex_e
{
	BUFFSG552EX_IDLE1,
	BUFFSG552EX_RELOAD,
	BUFFSG552EX_DRAW,
	BUFFSG552EX_SHOOT1,
	BUFFSG552EX_SHOOT2,
	BUFFSG552EX_SHOOT3,
};

LINK_ENTITY_TO_CLASS(weapon_buffsg552ex, CBuffsg552ex)

void CBuffsg552ex::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_buffsg552ex");

	Precache();
	m_iId = WEAPON_BUFFSG552EX;
	SET_MODEL(ENT(pev), "models/w_buffsg552ex.mdl");

	m_iDefaultAmmo = 50;
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;
	m_iShotsCount = 0;

	FallInit();
}

void CBuffsg552ex::Precache(void)
{
	PRECACHE_MODEL("models/v_buffsg552ex.mdl");
	PRECACHE_MODEL("models/w_buffsg552ex.mdl");
	PRECACHE_MODEL("models/p_buffsg552ex.mdl");
	PRECACHE_MODEL("sprites/ef_buffsg552ex_slash1.spr");
	PRECACHE_MODEL("sprites/ef_buffsg552ex_slash2.spr");
	PRECACHE_MODEL("sprites/ef_buffsg552ex_slash3.spr");
	PRECACHE_MODEL("sprites/ef_buffsg552ex_slash_big.spr");
	PRECACHE_MODEL("sprites/muzzleflash205.spr");
	PRECACHE_MODEL("sprites/muzzleflash206.spr");

	PRECACHE_SOUND("weapons/buffsg552ex_shoot1.wav");
	PRECACHE_SOUND("weapons/buffsg552ex_idle.wav");

	m_iShell = PRECACHE_MODEL("models/rshell.mdl");
	m_usFireBuffsg552ex = PRECACHE_EVENT(1, "events/buffsg552ex.sc");
}

int CBuffsg552ex::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "556Nato";
	p->iMaxAmmo1 = MAX_AMMO_556NATO;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 50;
	p->iSlot = 0;
	p->iPosition = 10;
	p->iId = m_iId = WEAPON_BUFFSG552EX;
	p->iFlags = 0;
	p->iWeight = SG552_WEIGHT;

	return 1;
}

BOOL CBuffsg552ex::Deploy(void)
{
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;
	iShellOn = 1;
	return DefaultDeploy("models/v_buffsg552ex.mdl", "models/p_buffsg552ex.mdl", BUFFSG552EX_DRAW, "mp5", UseDecrement() != FALSE);
}

void CBuffsg552ex::SecondaryAttack(void)
{
#ifndef CLIENT_DLL
	if (IsSignal())
	{
		if (m_pPlayer->m_iFOV != 90)
		{
			m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_buffsg552ex.mdl");
			m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;
			m_iShotsCount = 0;
		}
		else
			m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 75;
	}
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;
#endif
}


void CBuffsg552ex::PrimaryAttack(void)
{
	if (m_pPlayer->pev->fov != 90)
		Buffsg552exFireB((0.02) * m_flAccuracy, 0.235s, FALSE);
	else
	{
		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			Buffsg552exFire(0.035 + (0.45) * m_flAccuracy, 0.0825s, FALSE);
		else if (m_pPlayer->pev->velocity.Length2D() > 140)
			Buffsg552exFire(0.035 + (0.075) * m_flAccuracy, 0.0825s, FALSE);
		else
			Buffsg552exFire((0.02) * m_flAccuracy, 0.0825s, FALSE);
	}
}

float CBuffsg552ex::GetDamage()
{
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZBS)
		return 423.0f;
	if (g_pModRunning->DamageTrack() == DT_ZB)
		return 120.0f;
#endif
	return 28.0f;
}

float CBuffsg552ex::GetDamageB()
{
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZBS)
		return 396.0f;
	if (g_pModRunning->DamageTrack() == DT_ZB)
		return 100.0f;
#endif
	return 24.0f;
}


float CBuffsg552ex::SlashDamage()
{
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZBS)
		return 300.0f;
	if (g_pModRunning->DamageTrack() == DT_ZB)
		return 686.0f;
#endif
	return 51.0f;
}

#ifndef CLIENT_DLL
void CBuffsg552ex::RadiusDamage(Vector vecAiming, float flDamage)
{
	float flRadius = 75.0f;
	if (g_pModRunning->DamageTrack() == DT_ZBS)
		flRadius = 75.0f;
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flRadius = 75.0f;	
	const Vector vecSrc = vecAiming;
	entvars_t* const pevAttacker = VARS(pev->owner);
	entvars_t* const pevInflictor = this->pev;
	int bitsDamageType = DMG_BULLET;

	TraceResult tr;
	const float falloff = flRadius ? flDamage / flRadius : 1;
	const int bInWater = (UTIL_PointContents(vecSrc) == CONTENTS_WATER);

	CBaseEntity* pEntity = NULL;
	while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != NULL)
	{
		if (pEntity->pev->takedamage != DAMAGE_NO)
		{
			if (bInWater && !pEntity->pev->waterlevel)
				continue;

			if (!bInWater && pEntity->pev->waterlevel == 3)
				continue;

			if (pEntity->IsBSPModel())
				continue;

			if (pEntity->pev == pevAttacker)
				continue;

			Vector vecSpot = pEntity->BodyTarget(vecSrc);
			UTIL_TraceLine(vecSrc, vecSpot, missile, ENT(pevInflictor), &tr);

			if (tr.flFraction == 1.0f || tr.pHit == pEntity->edict())
			{
				if (tr.fStartSolid)
				{
					tr.vecEndPos = vecSrc;
					tr.flFraction = 0;
				}
				float flAdjustedDamage = flDamage - (vecSrc - pEntity->pev->origin).Length() * falloff;
				flAdjustedDamage = Q_max(0, flAdjustedDamage);

				if (tr.flFraction == 1.0f)
				{
					pEntity->TakeDamage(pevInflictor, pevAttacker, flAdjustedDamage, bitsDamageType);
				}
				else
				{
					tr.iHitgroup = HITGROUP_CHEST;
					ClearMultiDamage();
					pEntity->TraceAttack(pevInflictor, flAdjustedDamage, (tr.vecEndPos - vecSrc).Normalize(), &tr, bitsDamageType);
					ApplyMultiDamage(pevInflictor, pevAttacker);
				}
			}
		}
	}
}
#endif

BOOL CBuffsg552ex::IsSignal()
{
#ifndef CLIENT_DLL
	if (m_iShotsCount >= SIGNAL_SHOTS_COUNT)
	{
		pev->skin = 1;
		return TRUE;
	}
	else
	{
		pev->skin = 0;
		return FALSE;
	}
#endif
	return TRUE;
}
void CBuffsg552ex::Buffsg552exFireB(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	m_bDelayFire = true;
	m_iShotsFired++;
	m_flAccuracy = ((float)(m_iShotsFired * m_iShotsFired * m_iShotsFired) / 220.0) + 0.3;

	if (m_flAccuracy > 1)
		m_flAccuracy = 1;

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
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	Vector vecSrc = m_pPlayer->GetGunPosition();

#ifndef CLIENT_DLL
	int iEffect = 2;
	Vector vecDir = m_pPlayer->FireBullets4(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_556MM, GetDamageB(), 0.955, m_pPlayer->pev, FALSE, iEffect, m_pPlayer->random_seed, m_pPlayer);
	if (!iEffect)
	{
		CBaseEntity* pevAttacker = this->m_pPlayer;
		auto vecShootPosition = Get_ShootPosition(pevAttacker, vecSrc, vecDir);
		RadiusDamage(vecShootPosition, SlashDamage());
	}
	PLAYBACK_EVENT_FULL(0, m_pPlayer->edict(), m_usFireBuffsg552ex, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), TRUE, FALSE);
#endif
	
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
	if (m_pPlayer->pev->velocity.Length2D() > 0)
		KickBack(1.0, 0.45, 0.28, 0.04, 4.25, 2.5, 7);
	else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		KickBack(1.25, 0.45, 0.22, 0.18, 6.0, 4.0, 5);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		KickBack(0.6, 0.35, 0.2, 0.0125, 3.7, 2.0, 10);
	else
		KickBack(0.625, 0.375, 0.25, 0.0125, 4.0, 2.25, 9);
}

Vector CBuffsg552ex::Get_ShootPosition(CBaseEntity* pevAttacker, Vector Start, Vector Aiming)
{
	Vector end;

	end = pevAttacker->pev->v_angle;
	UTIL_MakeVectors(end);
	end = gpGlobals->v_forward;
	end = end * 8192.0;
	end = Start + end;

	TraceResult tr;
	UTIL_TraceLine(Start, end, dont_ignore_monsters, pevAttacker->edict(), &tr);
	end = tr.vecEndPos;

	return end;
}

void CBuffsg552ex::Buffsg552exFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	m_bDelayFire = true;
	m_iShotsFired++;
	m_flAccuracy = ((float)(m_iShotsFired * m_iShotsFired * m_iShotsFired) / 220.0) + 0.3;

	if (m_flAccuracy > 1)
		m_flAccuracy = 1;

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
		}

		return;
	}
	m_iShotsCount++;
	IsSignal();
	m_iClip--;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_556MM, 33, 0.955, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireBuffsg552ex, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
	if (m_pPlayer->pev->velocity.Length2D() > 0)
		KickBack(1.0, 0.45, 0.28, 0.04, 4.25, 2.5, 7);
	else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		KickBack(1.25, 0.45, 0.22, 0.18, 6.0, 4.0, 5);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		KickBack(0.6, 0.35, 0.2, 0.0125, 3.7, 2.0, 10);
	else
		KickBack(0.625, 0.375, 0.25, 0.0125, 4.0, 2.25, 9);
}

void CBuffsg552ex::Reload(void)
{
	if (m_pPlayer->ammo_556nato <= 0)
		return;

	if (DefaultReload(50, BUFFSG552EX_RELOAD, 2.03s))
	{
#ifndef CLIENT_DLL
		if ((int)CVAR_GET_FLOAT("mp_csgospecialeffect"))
		{
			m_pPlayer->m_flNextAttack = 1.43s;
			m_flTimeWeaponIdle = 2.03s + 0.5s;
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.03s;
		}
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif

		if (m_pPlayer->m_iFOV != 90)
			SecondaryAttack();

		m_flAccuracy = 0.2;
		m_iShotsFired = 0;
		m_bDelayFire = false;
	}
}

void CBuffsg552ex::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
	SendWeaponAnim(BUFFSG552EX_IDLE1, UseDecrement() != FALSE);
}

float CBuffsg552ex::GetMaxSpeed(void)
{
	if (m_pPlayer->m_iFOV == 90)
		return 235;

	return 210;
}


}
