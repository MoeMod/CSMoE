/***
*	wpn_balrog7.cpp
*	Copyright (c) 2019, JustANoobQAQ.
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
#include "wpn_balrog7.h"
#include "weapons/WeaponTemplate.hpp"
#ifndef CLIENT_DLL
#include "effects.h"
#include "customentity.h"
#include "monsters.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

LINK_ENTITY_TO_CLASS(weapon_balrog7, CBalrog7)

void CBalrog7::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_balrog7");

	Precache();
	m_iId = WEAPON_BALROG7;
	SET_MODEL(ENT(pev), "models/w_balrog7.mdl");

	m_iDefaultAmmo = BALROG7_DEFAULT_GIVE;
	m_flAccuracy = 0.24;
	m_iShotsFired = 0;

	FallInit();
}

void CBalrog7::Precache(void)
{
	PRECACHE_MODEL("models/v_balrog7.mdl");
	PRECACHE_MODEL("models/w_balrog7.mdl");
	PRECACHE_MODEL("models/p_balrog7.mdl");

	PRECACHE_MODEL("sprites/muzzleflash20.spr");

	PRECACHE_SOUND("weapons/balrog7-1.wav");
	PRECACHE_SOUND("weapons/balrog7_exp.wav");

	m_iShell = PRECACHE_MODEL("models/rshell.mdl");
	m_usFireBalrog7 = PRECACHE_EVENT(1, "events/balrog7.sc");
	
}

int CBalrog7::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "762natoboxame";
	p->iMaxAmmo1 = 240;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = BALROG7_MAX_CLIP;
	p->iSlot = 0;
	p->iPosition = 4;
	p->iId = m_iId = WEAPON_BALROG7;
	p->iFlags = 0;
	p->iWeight = M249_WEIGHT;

	return 1;
}

BOOL CBalrog7::Deploy(void)
{
	m_flAccuracy = 0.24;
	m_iShotsFired = 0;
	iShellOn = 1;
	m_iTargetShotsFired = RANDOM_LONG(7, 11);

	return DefaultDeploy("models/v_balrog7.mdl", "models/p_balrog7.mdl", BALROG7_DRAW, "m249", UseDecrement() != FALSE);
}

void CBalrog7::PrimaryAttack(void)
{
	duration_t flCycleTime;

	if (m_pPlayer->pev->fov != 90)
		flCycleTime = 0.15s;
	else
		flCycleTime = 0.115s;

	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		Balrog7Fire(m_flAccuracy * 0.2 + 0.05, flCycleTime, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 195)
		Balrog7Fire(m_flAccuracy * 0.03 + 0.035, flCycleTime, FALSE);
	else if (m_pPlayer->pev->fov != 90)
		Balrog7Fire(m_flAccuracy * 0.01 + 0.02, flCycleTime, FALSE);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		Balrog7Fire(m_flAccuracy * 0.02 + 0.01, flCycleTime, FALSE);
	else
		Balrog7Fire(m_flAccuracy * 0.0 + 0.021, flCycleTime, FALSE);
}

void CBalrog7::SecondaryAttack(void)
{
	if (m_pPlayer->m_iFOV != 90)
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;
	else
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 65;

	m_pPlayer->ResetMaxSpeed();
	EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/zoom.wav", 0.2, 2.4, 0, PITCH_NORM);
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;
}

Vector CBalrog7::Get_ShootPosition(Vector vecSrc, Vector vecDirShooting, float flSpread, float flDistance, int random_seed)
{
	Vector vecRight = gpGlobals->v_right;
	Vector vecUp = gpGlobals->v_up;
	float x = UTIL_SharedRandomFloat(random_seed, -0.5, 0.5) + UTIL_SharedRandomFloat(random_seed + 1, -0.5, 0.5);
	float y = UTIL_SharedRandomFloat(random_seed + 2, -0.5, 0.5) + UTIL_SharedRandomFloat(random_seed + 3, -0.5, 0.5);

	TraceResult tr;
	Vector vecDir = vecDirShooting + x * flSpread * vecRight + y * flSpread * vecUp;
	Vector vecEnd = vecSrc + vecDir * flDistance;

	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

	return tr.vecEndPos;
}


void CBalrog7::ItemPostFrame()
{	
	if (!(this->m_pPlayer->pev->button & IN_ATTACK))
#ifndef CLIENT_DLL
		m_iChargeShotsFired = 0;
#endif
	return CBasePlayerWeapon::ItemPostFrame();
}

float CBalrog7::GetDamage()
{
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZBS)
		return 51.0f;
	if (g_pModRunning->DamageTrack() == DT_ZB)
		return 36.0f;
#endif
	return 36.0f;
}

#ifndef CLIENT_DLL
float CBalrog7::BalrogDamage()
{
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZBS)
		return 600.0f;
	if (g_pModRunning->DamageTrack() == DT_ZB)
		return 400.0f;
#endif
	return 105.0f;
}

void CBalrog7::RadiusDamage(Vector vecAiming, float flDamage)
{
	float flRadius = 85.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZBS)
		flRadius = 140.0f;
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flRadius = 125.0f;
#endif		
	const Vector vecSrc = vecAiming;
	entvars_t * const pevAttacker = VARS(pev->owner);
	entvars_t * const pevInflictor = this->pev;
	int bitsDamageType = DMG_BULLET;

	TraceResult tr;
	const float falloff = flRadius ? flDamage / flRadius : 1;
	const int bInWater = (UTIL_PointContents(vecSrc) == CONTENTS_WATER);

	CBaseEntity *pEntity = NULL;
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

void CBalrog7::Balrog7Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	m_bDelayFire = true;
	m_iShotsFired++;
	m_flAccuracy = (m_iShotsFired * m_iShotsFired * m_iShotsFired) / 190.0 + 0.3;

	if (m_flAccuracy > 0.9)
		m_flAccuracy = 0.9;

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

	float flRangeModifier;

	if (m_pPlayer->m_iFOV == 90)
		flRangeModifier = 0.97;
	else
		flRangeModifier = 0.98;

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_556MM, GetDamage(), flRangeModifier, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

#ifndef CLIENT_DLL
	m_iChargeShotsFired++;
	if (m_iChargeShotsFired == m_iTargetShotsFired)
	{
		m_iChargeShotsFired = 0;
		auto vecShootPosition = Get_ShootPosition(vecSrc, gpGlobals->v_forward, 0.0, 8192, m_pPlayer->random_seed);
		RadiusDamage(vecShootPosition, BalrogDamage());
		PLAYBACK_EVENT_FULL(0, m_pPlayer->edict(), m_usFireBalrog7, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, TRUE);
		m_iTargetShotsFired = RANDOM_LONG(7, 11);
		//RadiusDamage(vecAiming , flDamage);
	}
	else
		PLAYBACK_EVENT_FULL(0, m_pPlayer->edict(), m_usFireBalrog7, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);
#endif

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.6s;

	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		KickBack(1.5, 0.55, 0.3, 0.3, 6.0, 5.0, 5);
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
		KickBack(1.1, 0.3, 0.2, 0.06, 4.0, 2.5, 8);
	else if (m_pPlayer->pev->fov != 90)
		KickBack(0.6, 0.2, 0.08, 0.0, 2.0, 0.2, 10);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		KickBack(0.75, 0.1, 0.1, 0.018, 3.5, 1.2, 9);
	else
		KickBack(0.8, 0.2, 0.18, 0.02, 3.2, 2.25, 7);

}

void CBalrog7::Reload(void)
{
	
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;

	if (m_pPlayer->m_iFOV != 90)
		SecondaryAttack();

	if (DefaultReload(BALROG7_MAX_CLIP, BALROG7_RELOAD, 3.8s))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		m_flAccuracy = 0.24;
		m_bDelayFire = false;
		m_iShotsFired = 0;
		m_iTargetShotsFired = RANDOM_LONG(7, 11);
	}
}

void CBalrog7::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
	SendWeaponAnim(BALROG7_IDLE1, UseDecrement() != FALSE);
}

float CBalrog7::GetMaxSpeed(void)
{
	if (m_pPlayer->m_iFOV == 90)
		return M249_MAX_SPEED;

	return 200;
}
}
