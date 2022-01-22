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
#include "wpn_y21s1jetgunmd.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_y21s1jetgunmd, CY21s1Jetgunmd)

const int y21s1jetgunmd_MAXCLIP = 28;

void CY21s1Jetgunmd::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_y21s1jetgunmd");

	Precache();
	m_iId = WEAPON_ELITE;
	SET_MODEL(ENT(pev), "models/w_y21s1jetgunmd.mdl");

	m_iDefaultAmmo = 100;
	m_iDefaultAmmo2 = m_iChargeShotsFired = 0;

	m_tPrimaryAttackStart = invalid_time_point;
	m_tDashTime = invalid_time_point;
	m_tDashStart = invalid_time_point;

	FallInit();
}

void CY21s1Jetgunmd::Precache(void)
{
	PRECACHE_MODEL("models/v_y21s1jetgunmd.mdl");
	PRECACHE_MODEL("models/p_y21s1jetgunmd.mdl");
	PRECACHE_MODEL("models/w_y21s1jetgunmd.mdl");

	PRECACHE_SOUND("weapons/jetgunmd-1.wav");
	PRECACHE_SOUND("weapons/jetgunmd-1_end.wav");
	PRECACHE_SOUND("weapons/jetgunmd-2.wav");
	PRECACHE_SOUND("weapons/jetgunma_beep.wav");
	PRECACHE_SOUND("weapons/jetgunmb_dash.wav");

	PRECACHE_MODEL("sprites/ef_jetgunhit.spr");
	PRECACHE_MODEL("sprites/muzzleflash162.spr");
	PRECACHE_MODEL("sprites/muzzleflash167.spr");
	PRECACHE_MODEL("sprites/muzzleflash168.spr");

	m_usFireY21s1Jetgunmd = PRECACHE_EVENT(1, "events/y21s1jetgunmd.sc");
}

int CY21s1Jetgunmd::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "EnergyCells";
	p->iMaxAmmo1 = 200;
	p->pszAmmo2 = "EnergyBattery";
	p->iMaxAmmo2 = 2;
	p->iMaxClip = 100;
	p->iSlot = 1;
	p->iPosition = 5;
	p->iId = m_iId = WEAPON_ELITE;
	p->iFlags = 0;
	p->iWeight = ELITE_WEIGHT;

	return 1;
}

int CY21s1Jetgunmd::ExtractAmmo(CBasePlayerWeapon* pWeapon)
{
	if (m_iDefaultAmmo2)
	{
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iDefaultAmmo2;
		m_iDefaultAmmo2 = 0;
	}
	return CBasePlayerWeapon::ExtractAmmo(pWeapon);
}

BOOL CY21s1Jetgunmd::Deploy(void)
{
	m_tPrimaryAttackStart = invalid_time_point;
	m_tDashTime = invalid_time_point;
	m_tDashStart = invalid_time_point;

	return DefaultDeploy("models/v_y21s1jetgunmd.mdl", "models/p_y21s1jetgunmd.mdl", ANIM_DRAW, m_pPlayer->GetAnimExtDualPistols(), UseDecrement() != FALSE);
}


void CY21s1Jetgunmd::Holster(int skiplocal)
{
	m_tPrimaryAttackStart = invalid_time_point;
	m_tDashTime = invalid_time_point;
	m_tDashStart = invalid_time_point;

	return CBasePlayerWeapon::Holster();
}

void CY21s1Jetgunmd::PrimaryAttack(void)
{
	++m_iShotsFired;
	m_bDelayFire = 1;
	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2s;
		}
		return; // sth ignored
	}

	if (m_tPrimaryAttackStart == invalid_time_point)
	{
		SendWeaponAnim(ANIM_SHOOT_START, UseDecrement() != FALSE); // 3
		m_tPrimaryAttackStart = gpGlobals->time + 0.13s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 99999.0s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 99999.0s;
	}
	else
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME; // 600
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH; // 512

		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.9s;

		--m_iClip;

		m_iChargeShotsFired++;

		if (m_iChargeShotsFired >= 60)
		{
			m_iChargeShotsFired = 0;

			if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] < DashCount())
			{
				m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]++;

				EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/jetgunma_beep.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

#ifndef CLIENT_DLL
				MESSAGE_BEGIN(MSG_ONE, gmsgAmmoPickup, NULL, m_pPlayer->pev);
				WRITE_BYTE(CBasePlayer::GetAmmoIndex("EnergyBattery"));
				WRITE_BYTE(1);
				MESSAGE_END();
#endif
			}
		}

		KickBack(0.49, 0.18, 0.2, 0.013, 1.0, 1.4, 8);

		PrimaryAttack_FindTargets();
		PrimaryAttack_InstantDamage();
		PrimaryAttack_FieldDamage();
	}

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.09s;
}


void CY21s1Jetgunmd::PrimaryAttack_FindTargets()
{
	m_pFieldEnemyList.clear();
	m_pEnemyList.clear();
#ifndef CLIENT_DLL
	CBaseEntity* pEntity = NULL;
	while ((pEntity = UTIL_FindEntityInSphere(pEntity, m_pPlayer->pev->origin, 345)) != nullptr)
	{
		if (PrimaryAttack_CheckTargetAvailable(pEntity, m_pPlayer->pev->v_angle, TRUE))
		{
			EHANDLE eh;
			eh.Set(pEntity->edict());
			m_pEnemyList.push_back(eh);
		}
	}

	pEntity = NULL;
	while ((pEntity = UTIL_FindEntityInSphere(pEntity, m_pPlayer->pev->origin, 183)) != nullptr)
	{
		if (PrimaryAttack_CheckTargetAvailable(pEntity, m_pPlayer->pev->v_angle, FALSE))
		{
			EHANDLE eh;
			eh.Set(pEntity->edict());
			m_pFieldEnemyList.push_back(eh);
		}
	}

#endif
}

bool CY21s1Jetgunmd::PrimaryAttack_CheckTargetAvailable(CBaseEntity* a2, Vector vecAngleDirection, bool bFireAttack)
{
	const float flRadius = 345;

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

	if (bFireAttack)
	{
		UTIL_MakeVectors(vecAngleDirection);

		if (DotProduct(gpGlobals->v_forward, vecDelta) < 0.88)
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
	}
	else
	{
		TraceResult tr;
		UTIL_TraceLine(m_pPlayer->pev->origin, a2->pev->origin, missile, m_pPlayer->edict(), &tr);

		if (tr.flFraction < 1.0)
		{
			if (!tr.pHit)
				return false;

			if (tr.pHit != a2->edict())
				return false;
		}
	}

	return true;
}

void CY21s1Jetgunmd::PrimaryAttack_InstantDamage()
{
	if (m_pEnemyList.empty())
		return;

	for (CBaseEntity* pEntity : m_pEnemyList)
	{
		if (!pEntity)
			continue;
		Vector vecDirection = (pEntity->pev->origin - m_pPlayer->pev->origin).Normalize();

		TraceResult tr;
		UTIL_TraceLine(m_pPlayer->pev->origin, pEntity->pev->origin, missile, ENT(m_pPlayer->pev), &tr);

		ClearMultiDamage();
		pEntity->TraceAttack(m_pPlayer->pev, GetDamage_PrimaryAttack_Instant(), vecDirection, &tr, DMG_BULLET);
		ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);
	}
}


void CY21s1Jetgunmd::PrimaryAttack_FieldDamage()
{
	if (m_pFieldEnemyList.empty())
		return;

	//std::random_shuffle(phs9_10_11.begin(), phs9_10_11.end());
	for (CBaseEntity* pEntity : m_pFieldEnemyList)
	{
		if (!pEntity)
			continue;
		Vector vecDirection = (pEntity->pev->origin - m_pPlayer->pev->origin).Normalize();

		TraceResult tr;
		UTIL_TraceLine(m_pPlayer->pev->origin, pEntity->pev->origin, missile, ENT(m_pPlayer->pev), &tr);

		ClearMultiDamage();
		pEntity->TraceAttack(m_pPlayer->pev, GetDamage_PrimaryAttack_Field(), vecDirection, &tr, DMG_BULLET);
		ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

#ifndef CLIENT_DLL
			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pEntity->pev->origin);
			WRITE_BYTE(TE_EXPLOSION);
			WRITE_COORD(pEntity->pev->origin.x);
			WRITE_COORD(pEntity->pev->origin.y);
			WRITE_COORD(pEntity->pev->origin.z);
			WRITE_SHORT(MODEL_INDEX("sprites/ef_jetgunhit.spr"));
			WRITE_BYTE(5);
			WRITE_BYTE(30);
			WRITE_BYTE(TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NOSOUND);
			MESSAGE_END();
#endif
	}
}



void CY21s1Jetgunmd::SecondaryAttack(void)
{
	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] <= 0)
		return;

	m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;

	if (m_tDashStart == invalid_time_point)
	{
		SendWeaponAnim(ANIM_DASH_START, UseDecrement() != FALSE); // 3

		EMIT_SOUND_DYN(this->edict(), CHAN_WEAPON, "weapons/jetgunmb_dash.wav", VOL_NORM, ATTN_NORM, 0, 94);

		m_tDashStart = gpGlobals->time + 0.13s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 99999.0s;
	}

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0s;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.6s;

	m_tPrimaryAttackStart = invalid_time_point;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	Vector vecDir = gpGlobals->v_forward;
	if (vecDir[2] < 0.0)
		vecDir[2] = 0.0;

	m_pPlayer->pev->velocity = vecDir * 650;
	m_pPlayer->m_flVelocityModifier = 1.0;

	m_tDashTime = gpGlobals->time + 0.6s;
}

void CY21s1Jetgunmd::ItemPostFrame()
{
	if (m_tPrimaryAttackStart > invalid_time_point)
	{
		if (this->m_pPlayer->pev->button & IN_ATTACK && this->m_iClip > 0)
		{
			if (gpGlobals->time > m_tPrimaryAttackStart)
			{
				m_tPrimaryAttackStart = gpGlobals->time + 0.5s;

				int flags;
#ifdef CLIENT_WEAPONS
				flags = FEV_NOTHOST;
#else
				flags = 0;
#endif

				PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireY21s1Jetgunmd, 0, g_vecZero, g_vecZero, 2, 0, 2, 0, FALSE, FALSE);

				return CBasePlayerWeapon::ItemPostFrame();
			}
		}
		else
		{
			this->SendWeaponAnim(ANIM_SHOOT_END, UseDecrement() != FALSE); // 5

			EMIT_SOUND_DYN(this->edict(), CHAN_WEAPON, "weapons/jetgunmd-1_end.wav", VOL_NORM, ATTN_NORM, 0, 94);

			m_tPrimaryAttackStart = invalid_time_point;
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5s;	
		}
	}

	if (m_tDashTime != invalid_time_point)
	{
		if (m_tDashTime < gpGlobals->time)
		{
			this->SendWeaponAnim(ANIM_DASH_END, UseDecrement() != FALSE); // 4
			m_tDashTime = invalid_time_point;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5s;

			m_pPlayer->pev->velocity = { 0, 0, 0 };
		}
		else
		{
			this->SendWeaponAnim(ANIM_DASH_LOOP, UseDecrement() != FALSE); // 4
			m_flTimeWeaponIdle= UTIL_WeaponTimeBase() + 9999.0s;
			return CBasePlayerWeapon::ItemPostFrame();
		}
	}

	return CBasePlayerWeapon::ItemPostFrame();
}


void CY21s1Jetgunmd::Reload(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;

	if (DefaultReload(100, ANIM_RELOAD, 2.2s))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
	}
}

void CY21s1Jetgunmd::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60s;
	SendWeaponAnim(ANIM_IDLE, UseDecrement() != FALSE);
}

double CY21s1Jetgunmd::GetDamage_PrimaryAttack_Instant()
{
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZBS)
	{
		if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 2)
			return 124.0;
		else if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 1)
			return 104.0;
		else
			return 84.0;
	}
	if (g_pModRunning->DamageTrack() == DT_ZB)
	{
		if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 2)
			return 64.0;
		else if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 1)
			return 54.0;
		else
			return 44.0;
	}
#endif
	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 2)
		return 15.0;
	else if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 1)
		return 13.0;
	else
		return 11.0;
}


double CY21s1Jetgunmd::GetDamage_PrimaryAttack_Field()
{
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZBS)
	{
		if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 2)
			return 130.0;
		else if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 1)
			return 100.0;
		else
			return 70.0;
	}
	if (g_pModRunning->DamageTrack() == DT_ZB)
	{
		if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 2)
			return 55.0;
		else if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 1)
			return 45.0;
		else
			return 35.0;
	}
#endif
	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 2)
		return 13.0;
	else if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 1)
		return 11.0;
	else
		return 9.0;
}

int  CY21s1Jetgunmd::DashCount()
{
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZBS)
		return 1;
#endif
	return 2;
}


}
