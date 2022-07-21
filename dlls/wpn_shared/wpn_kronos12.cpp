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
#include "wpn_kronos12.h"
#include "weapons/KnifeAttack.h"
#include "shake.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_kronos12, CKronos12)

void CKronos12::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_kronos12");

	Precache();
	m_iId = WEAPON_KRONOS12;
	SET_MODEL(ENT(pev), "models/w_kronos12.mdl");
	m_iDefaultAmmo = 30;
	m_flNextThink = invalid_time_point;
	FallInit();
}

void CKronos12::Precache(void)
{
	PRECACHE_MODEL("models/v_kronos12.mdl");
	PRECACHE_MODEL("models/w_kronos12.mdl");
	PRECACHE_MODEL("models/p_kronos12.mdl");
	PRECACHE_MODEL("sprites/smoke.spr");

	PRECACHE_SOUND("weapons/kronos12-1.wav");

	m_iShellId = m_iShell = PRECACHE_MODEL("models/shotgunshell.mdl");

	m_usFireKronos12 = PRECACHE_EVENT(1, "events/kronos12.sc");
}

int CKronos12::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "buckshotk";
	p->iMaxAmmo1 = 60;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 30;
	p->iSlot = 0;
	p->iPosition = 5;
	p->iId = m_iId = WEAPON_KRONOS12;
	p->iFlags = 0;
	p->iWeight = M3_WEIGHT;

	return 1;
}

BOOL CKronos12::Deploy(void)
{
	m_flNextThink = invalid_time_point;
	return DefaultDeploy("models/v_kronos12.mdl", "models/p_kronos12.mdl", m_iWeaponState & WPNSTATE_KRONOS12_SCAN_ACTIVATE ? ANIM_DRAW2 : ANIM_DRAW, "shotgun", UseDecrement() != FALSE);
}

void CKronos12::SecondaryAttack(void)
{
	if (m_iWeaponState & WPNSTATE_KRONOS12_ZOOM_ACTIVATE)
	{
#ifndef CLIENT_DLL
		m_pEnemylist.clear();
#endif
		SendWeaponAnim(ANIM_SCAN_DEACTIVATE, UseDecrement() != FALSE);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.875s;
		m_iWeaponState = 0;
		m_flNextThink = invalid_time_point;
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;
		pev->iuser1 = 0;
		return;
	}

	if (m_iWeaponState & WPNSTATE_KRONOS12_SCAN_ACTIVATE)
	{
		SendWeaponAnim(ANIM_ZOOM, UseDecrement() != FALSE);
		m_flNextThink = gpGlobals->time + 0.87s;
		m_pPlayer->m_flNextAttack = 0.87s;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.875s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.875s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.87s;
	}

}

bool CKronos12::PrimaryAttack_CheckTargetAvailable(CBaseEntity* a2, Vector vecAngleDirection)
{
	const float flRadius = 4096;

	if (!a2->IsPlayer() && a2->Classify() != CLASS_PLAYER_ALLY)
		return false;

	if (!a2->IsAlive())
		return false;

#ifndef CLIENT_DLL
	if (g_pGameRules->PlayerRelationship(m_pPlayer, a2) == GR_TEAMMATE)
		return false;
#endif

	if (a2->pev == m_pPlayer->pev)
		return false;

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDelta = (a2->Center() - vecSrc).Normalize();

	UTIL_MakeVectors(vecAngleDirection);

	if (DotProduct(gpGlobals->v_forward, vecDelta) < 0.7)
		return false;

	TraceResult tr;
	UTIL_TraceLine(m_pPlayer->pev->origin, a2->pev->origin, missile, m_pPlayer->edict(), &tr);

	if (tr.flFraction < 1.0)
	{
		if (!tr.pHit)
			return false;

		if (tr.pHit != a2->edict())
			return false;
	}

	return true;
}

void CKronos12::PrimaryAttack_FindTargets()
{
	const float flRadius = 4096;

#ifndef CLIENT_DLL
	m_pEnemylist.erase(std::remove_if(m_pEnemylist.begin(), m_pEnemylist.end(), [=](CBaseEntity* pEntity) {

		if (!pEntity->IsAlive() || bIsWallBetweenEntity(pEntity, m_pPlayer))
			return true;

		UTIL_MakeVectors(m_pPlayer->pev->v_angle);

		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecDelta = (pEntity->Center() - vecSrc).Normalize();

		if (DotProduct(gpGlobals->v_forward, vecDelta) < 0.7)
			return true;

		return false;
		}), m_pEnemylist.end());


	CBaseEntity* pEntity = NULL;
	while ((pEntity = UTIL_FindEntityInSphere(pEntity, m_pPlayer->pev->origin, flRadius)) != nullptr)
	{
		EHANDLE eh;
		eh.Set(pEntity->edict());

		if (std::find(m_pEnemylist.begin(), m_pEnemylist.end(), eh) != m_pEnemylist.end())
			continue;

		if (PrimaryAttack_CheckTargetAvailable(pEntity, m_pPlayer->pev->v_angle))
		{
			m_pEnemylist.push_back(eh);
		}
	}
#endif
}

void CKronos12::PrimaryAttack_InstantDamage()
{
#ifndef CLIENT_DLL
	for (CBaseEntity* pEntity : m_pEnemylist)
	{
		if (!pEntity)
			continue;

		Vector vecDirection = (pEntity->pev->origin - m_pPlayer->pev->origin).Normalize();
		Vector vecSrc = m_pPlayer->GetGunPosition();

		TraceResult tr;
		UTIL_TraceLine(vecSrc, vecDirection * 8192, missile, ENT(m_pPlayer->pev), &tr);

		float flDamage = GetDamage_PrimaryAttack_Instant() * (1 - tr.flFraction);

		ClearMultiDamage();
		pEntity->TraceAttack(m_pPlayer->pev, flDamage, vecDirection, &tr, DMG_BULLET);
		ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, m_pPlayer->pev->origin);
		WRITE_BYTE(TE_BEAMPOINTS);
		WRITE_COORD(vecSrc.x);
		WRITE_COORD(vecSrc.y);
		WRITE_COORD(vecSrc.z);
		WRITE_COORD(pEntity->pev->origin.x);
		WRITE_COORD(pEntity->pev->origin.y);
		WRITE_COORD(pEntity->pev->origin.z);
		WRITE_SHORT(MODEL_INDEX("sprites/smoke.spr"));
		WRITE_BYTE(0);
		WRITE_BYTE(0);
		WRITE_BYTE(10);//Life
		WRITE_BYTE(7);//Width
		WRITE_BYTE(0);//Noise
		WRITE_BYTE(255);
		WRITE_BYTE(255);
		WRITE_BYTE(255);
		WRITE_BYTE(20);
		WRITE_BYTE(0);
		MESSAGE_END();
	}
#endif
}

void CKronos12::ItemPostFrame(void)
{
	if (m_flNextThink != invalid_time_point && m_flNextThink < gpGlobals->time)
	{
		if (m_pPlayer->m_iFOV != 90)
		{
#ifndef CLIENT_DLL
			m_pEnemylist.clear();
#endif
			SendWeaponAnim(ANIM_SCAN_DEACTIVATE, UseDecrement() != FALSE);
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.875s;
			m_iWeaponState = 0;
			m_flNextThink = invalid_time_point;
			m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;
			pev->iuser1 = 0;
		}
		else
		{
#ifndef CLIENT_DLL
			MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->edict());
			WRITE_BYTE(20);
			WRITE_SHORT(5);
			MESSAGE_END();

			if(!m_pPlayer->m_bNightVisionOn)
				UTIL_ScreenFade(m_pPlayer, Vector(80, 80, 0), 5s, 5s, 80, FFADE_MODULATE);
#endif
			m_iWeaponState |= WPNSTATE_KRONOS12_ZOOM_ACTIVATE;
			m_flNextThink = gpGlobals->time + 5.0s;
			m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 65;

			
		}
	}

	if (m_iWeaponState & WPNSTATE_KRONOS12_ZOOM_ACTIVATE)
	{
		PrimaryAttack_FindTargets();
	}

	return CBasePlayerWeapon::ItemPostFrame();
}

void CKronos12::PrimaryAttack(void)
{
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15s;
		return;
	}

	if (m_iClip <= 0)
	{
		PlayEmptySound();

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0s;
		return;
	}

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	m_iClip--;
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	if (m_iWeaponState & WPNSTATE_KRONOS12_ZOOM_ACTIVATE && !m_pEnemylist.empty())
		PrimaryAttack_InstantDamage();
	else
	{
		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
#ifndef CLIENT_DLL
		m_pPlayer->FireBullets(6, m_pPlayer->GetGunPosition(), gpGlobals->v_forward, Vector(0.0675, 0.0675, 0), 3000, BULLET_PLAYER_BUCKSHOT, 0, GetDamage());

		if (CBaseEntity* pEntity = gMultiDamage.pEntity)
		{
			if(!m_iWeaponState)
			{
				if (pEntity->IsPlayer() || pEntity->Classify() == CLASS_PLAYER_ALLY)
				{
					pev->iuser1++;

					MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
					WRITE_BYTE(1);
					WRITE_BYTE(pev->iuser1);
					MESSAGE_END();
				}
			}		
		}
#endif
	}

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(0, ENT(m_pPlayer->pev), m_usFireKronos12, 0, g_vecZero, g_vecZero, 0, 0, 0, 0, m_iWeaponState & WPNSTATE_KRONOS12_ZOOM_ACTIVATE, m_iWeaponState & WPNSTATE_KRONOS12_SCAN_ACTIVATE);

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif

	if (!m_iWeaponState)
	{
		if (pev->iuser1 >= KRONOS12_CHARGE_AMMO)
		{
			SendWeaponAnim(ANIM_SCAN_ACTIVATE, UseDecrement() != FALSE);
			m_iWeaponState |= WPNSTATE_KRONOS12_SCAN_ACTIVATE;
			pev->iuser1 = 0;
		}
	}

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.3s;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.9s;

	if (m_pPlayer->pev->velocity.Length2D() > 0)
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 3, 3);
	else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 5, 5);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 1.5, 1.5);
	else
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 2, 2);
}

void CKronos12::Holster(int skiplocal)
{
#ifndef CLIENT_DLL
	m_pEnemylist.clear();
#endif
	if (m_iWeaponState & WPNSTATE_KRONOS12_ZOOM_ACTIVATE)
		m_iWeaponState = 0;
	CBasePlayerWeapon::Holster(skiplocal);

}

void CKronos12::Reload(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;

	int iClipSize = 30;
	int j = Q_min(iClipSize - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);

	if (!j)
		return;

	if (m_pPlayer->m_iFOV != 90)
	{
		m_iWeaponState = 0;
		m_flNextThink = invalid_time_point;
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;
		pev->iuser1 = 0;
	}
	
	
	if (DefaultReload(30, m_iWeaponState & WPNSTATE_KRONOS12_SCAN_ACTIVATE ? ANIM_RELOAD2 : ANIM_RELOAD, 4.0s))
	{
		
#ifndef CLIENT_DLL
		m_pEnemylist.clear();
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
	}
		
}

void CKronos12::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0s;

		if (m_iWeaponState & WPNSTATE_KRONOS12_SCAN_ACTIVATE)
		{
			if(m_iWeaponState & WPNSTATE_KRONOS12_ZOOM_ACTIVATE)
				SendWeaponAnim(ANIM_IDLE3, UseDecrement() != FALSE);
			else
				SendWeaponAnim(ANIM_IDLE2, UseDecrement() != FALSE);
		}
		else
		{
			SendWeaponAnim(ANIM_IDLE, UseDecrement() != FALSE);
		}
		
	}
}

float CKronos12::GetDamage()
{
	float flDamage = 16;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 46;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 84;
#endif
	return flDamage;
}

float CKronos12::GetDamage_PrimaryAttack_Instant()
{
	float flDamage = 96;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 360;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 608;
#endif
	return flDamage;
}


}
