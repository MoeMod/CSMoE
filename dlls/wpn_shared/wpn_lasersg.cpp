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
#include "wpn_lasersg.h"
#include <array>


#ifndef CLIENT_DLL
#include "effects.h"
#include "customentity.h"

#endif


#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif
	class CBeam;
	std::array<CBeam*, 20> phs17b; // EHANDLE ?
	static constexpr const char* Beam_SPR = "sprites/laserbeam.spr"; //
LINK_ENTITY_TO_CLASS(weapon_lasersg, CLaserSg)

void CLaserSg::Spawn(void)
{
	Precache();
	m_iId = WEAPON_LASERSG;
	SET_MODEL(ENT(pev), "models/w_lasersg.mdl");
	m_iDefaultAmmo2 = 25;
	m_iDefaultAmmo = 40;
	pev->skin = 0;

	m_tSecondaryAttackStart = invalid_time_point;
	InitChargeState();
	SendChargeState();

	FallInit();
}
int CLaserSg::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "lasersg_buckshot";
	p->iMaxAmmo1 = 180;
	p->pszAmmo2 = "lasersgshot";
	p->iMaxAmmo2 = 25;
	p->iMaxClip = 40;
	p->iSlot = 0;
	p->iPosition = 12;
	p->iId = m_iId = WEAPON_XM1014;
	p->iFlags = 0;
	p->iWeight = M3_WEIGHT;

	return 1;
}


void CLaserSg::Precache(void)
{
	PRECACHE_MODEL("models/v_lasersg.mdl");
	PRECACHE_MODEL("models/w_lasersg.mdl");
	PRECACHE_MODEL("models/p_lasersg.mdl");

	PRECACHE_MODEL("sprites/muzzleflash232.spr");
	PRECACHE_MODEL("sprites/muzzleflash233.spr");
	PRECACHE_MODEL("sprites/muzzleflash234.spr");
	PRECACHE_MODEL("sprites/muzzleflash235.spr");
	PRECACHE_MODEL("sprites/muzzleflash236.spr");
	PRECACHE_MODEL("sprites/ef_stormgianttw.spr");
	

	PRECACHE_SOUND("weapons/lasersg-1.wav");
	PRECACHE_SOUND("weapons/lasersg-2.wav");
	PRECACHE_SOUND("weapons/lasersg-2_end.wav");

	m_iShellId = m_iShell = PRECACHE_MODEL("models/shotgunshell.mdl");

	m_usFireLaserSg = PRECACHE_EVENT(1, "events/lasersg.sc");
}

BOOL CLaserSg::Deploy(void)
{
	ResetChargeTimer();

#ifndef CLIENT_DLL
	ItemPreFrame();
#endif

	m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iWeaponState;
	m_tSecondaryAttackStart = invalid_time_point;

	return DefaultDeploy("models/v_lasersg.mdl", "models/p_lasersg.mdl", 2, "shotgun", UseDecrement() != FALSE);
}

void CLaserSg::InitChargeState(void)
{
	m_iWeaponState = 0;
	m_flTimeNextCharge = invalid_time_point;

	pev->skin = 0;
}
void CLaserSg::ResetChargeTimer(void)
{
	if (m_iWeaponState != 25 && m_flTimeNextCharge == invalid_time_point)
		m_flTimeNextCharge = gpGlobals->time + 0.5s;
	if (this->pev->iuser1)
		this->pev->iuser1 = 0;
}

void CLaserSg::SendChargeState(void)
{
	if (!m_pPlayer)
		return;
}
int CLaserSg::ExtractAmmo(CBasePlayerWeapon* pWeapon)
{
	if (m_iDefaultAmmo2)
	{
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iDefaultAmmo2;
		m_iDefaultAmmo2 = 0;
	}
	return CBasePlayerWeapon::ExtractAmmo(pWeapon);
}
void CLaserSg::Holster(int skiplocal)
{
	ResetChargeTimer();
	m_tSecondaryAttackStart = invalid_time_point;

	CBasePlayerWeapon::Holster();
}
float CLaserSg::GetDamage() const
{
	float flDamage = 12.0;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 130.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 310.0f;
#endif
	return flDamage;
}

void CLaserSg::PrimaryAttack(void)
{

	ResetChargeTimer();

	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15s;
		return;
	}

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
		}

		return;
	}

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	m_iClip--;
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
#ifndef CLIENT_DLL
	m_pPlayer->FireBullets(7, m_pPlayer->GetGunPosition(), gpGlobals->v_forward, Vector(0.04, 0.04, 0), 3048, BULLET_PLAYER_BUCKSHOT, 0, GetDamage());
#endif

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif
	if(this->pev->iuser1)
		this->pev->iuser1 = 0;

	PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFireLaserSg, 0, g_vecZero, g_vecZero, m_vVecAiming.x, m_vVecAiming.y, 7, m_vVecAiming.x * 100, m_iClip != 0, FALSE);


#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.3s;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.875s;

	m_tSecondaryAttackStart = invalid_time_point;

	if (m_pPlayer->pev->velocity.Length2D() > 0)
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomFloat(m_pPlayer->random_seed + 1, 3.0, 4.0);
	else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 5.0, 6.0);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 1.5, 2.0);
	else
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 2.0, 3.0);

	Attack_FindTargets(LaserSgPrimaryAttack);
	Attack_ApplyDamage(LaserSgPrimaryAttack);


}
void CLaserSg::SecondaryAttack(void)
{
	if (m_iWeaponState <= 0)
		return;

	m_bDelayFire = 1;
	if (m_tSecondaryAttackStart == invalid_time_point)
	{
		SendWeaponAnim(4, UseDecrement() != FALSE); // 3
		m_tSecondaryAttackStart = gpGlobals->time + 0.35s;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 99999.0s;
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
		if (!m_iWeaponState && m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.12s;

		if (m_iWeaponState < 0)
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iWeaponState = 0;
		else 
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = --m_iWeaponState;

		KickBack(0.49, 0.18, 0.2, 0.013, 1.0, 1.4, 8);

		Attack_FindTargets(LaserSgSecondaryAttack);
		Attack_ApplyDamage(LaserSgSecondaryAttack);
	}
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.12s;
}

void CLaserSg::Attack_FindTargets(bool bFireAttack)
{
	m_pEnemyList.clear();
	m_pFieldEnemyList.clear();
	if (bFireAttack)
	{

#ifndef CLIENT_DLL
		CBaseEntity* pEntity = NULL;
		while ((pEntity = UTIL_FindEntityInSphere(pEntity, m_pPlayer->pev->origin, 370)) != nullptr)
		{
			if (Attack_CheckTargetAvailable(pEntity, m_pPlayer->pev->v_angle, TRUE))
			{
				EHANDLE eh;
				eh.Set(pEntity->edict());
				m_pEnemyList.push_back(eh);
			}
		}
#endif
	}
	else
	{

#ifndef CLIENT_DLL
		CBaseEntity* pEntity = NULL;

		while ((pEntity = UTIL_FindEntityInSphere(pEntity, m_pPlayer->pev->origin, 320)) != nullptr)
		{
			if (Attack_CheckTargetAvailable(pEntity, m_pPlayer->pev->v_angle, FALSE))
			{
				EHANDLE eh;
				eh.Set(pEntity->edict());
				m_pFieldEnemyList.push_back(eh);
			}
		}
#endif

	}
}
bool CLaserSg::Attack_CheckTargetAvailable(CBaseEntity* a2, Vector vecAngleDirection, bool bFireAttack)
{
	const float flRadius = 370;

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

		if (DotProduct(gpGlobals->v_forward, vecDelta) < 0.33)
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
	}

	return true;
}
void CLaserSg::Attack_ApplyDamage(bool bFireAttack)
{
	if (bFireAttack)
	{
		if (m_pEnemyList.empty())
			return;
		for (CBaseEntity* pEntity : m_pEnemyList)
		{
			if (!pEntity)
				continue;

			//No HeadShot At All
#ifndef CLIENT_DLL
			pEntity->m_LastHitGroup = HITGROUP_CHEST;
#endif
			pEntity->TakeDamage(m_pPlayer->pev, m_pPlayer->pev, GetDamage_SecondaryAttack(), DMG_BULLET);
#ifndef CLIENT_DLL
			Vector vecDirection = (pEntity->pev->origin - m_pPlayer->pev->origin).Normalize();
			UTIL_AddKickRateBoost(pEntity, vecDirection.Make2D(), 1.0, 800.0, 0);
			//Normally 1200.0

			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pEntity->pev->origin);
			WRITE_BYTE(TE_EXPLOSION);
			WRITE_COORD(pEntity->pev->origin.x);
			WRITE_COORD(pEntity->pev->origin.y);
			WRITE_COORD(pEntity->pev->origin.z);
			WRITE_SHORT(MODEL_INDEX("sprites/ef_stormgianttw.spr"));
			WRITE_BYTE(5);
			WRITE_BYTE(30);
			WRITE_BYTE(TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NOSOUND);
			MESSAGE_END();
#endif
		}
	}
	else
	{
		size_t v8 = 0;
		if (m_pFieldEnemyList.empty())
			return;

		for (CBaseEntity* pEntity : m_pFieldEnemyList)
		{
			if (!pEntity)
				continue;

			Vector vecDirection = (pEntity->pev->origin - m_pPlayer->pev->origin).Normalize();
			TraceResult tr;
			UTIL_TraceLine(m_pPlayer->pev->origin, pEntity->pev->origin, missile, ENT(m_pPlayer->pev), &tr);

#ifndef CLIENT_DLL
			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, m_pPlayer->pev->origin);
			WRITE_BYTE(TE_BEAMENTPOINT);
			WRITE_SHORT(m_pPlayer->entindex() | 0x1000);
			WRITE_COORD(pEntity->pev->origin.x);
			WRITE_COORD(pEntity->pev->origin.y);
			WRITE_COORD(pEntity->pev->origin.z);
			WRITE_SHORT(MODEL_INDEX("sprites/ef_winggun_laserbeam.spr"));
			WRITE_BYTE(1);
			WRITE_BYTE(30);
			WRITE_BYTE(3);//Life
			WRITE_BYTE(10);//Width
			WRITE_BYTE(15);//Noise
			WRITE_BYTE(240);
			WRITE_BYTE(65);
			WRITE_BYTE(85);
			WRITE_BYTE(230);
			WRITE_BYTE(10);
			MESSAGE_END();
#endif
			++v8;

			ClearMultiDamage();
			pEntity->TraceAttack(m_pPlayer->pev, GetDamage_PrimaryAttack(), vecDirection, &tr, DMG_BULLET);
			ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);
#ifndef CLIENT_DLL
			if (pEntity->IsPlayer())
			{
				CBasePlayer* player = (CBasePlayer*)pEntity;

				if (player->m_bIsZombie && !(player->m_iBuff & BUFF_IGNORE_KNOCKBACK))
				{
					ApplyKnockbackData(player, vecDirection, { 0, 0, 0, 0, 0.4f });
				}
			}

			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pEntity->pev->origin);
			WRITE_BYTE(TE_EXPLOSION);
			WRITE_COORD(pEntity->pev->origin.x);
			WRITE_COORD(pEntity->pev->origin.y);
			WRITE_COORD(pEntity->pev->origin.z);
			WRITE_SHORT(MODEL_INDEX("sprites/ef_stormgianttw.spr"));
			WRITE_BYTE(3);
			WRITE_BYTE(15);
			WRITE_BYTE(TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NOSOUND);
			MESSAGE_END();
#endif
		}
	}
}


#ifndef CLIENT_DLL
void CLaserSg::ItemPostFrame(void)
{
	if (!m_iWeaponState)
	{
		SendChargeState();
	}

	if (m_tSecondaryAttackStart > invalid_time_point)
	{
		if (this->m_pPlayer->pev->button & IN_ATTACK2 && m_iWeaponState > 0)
		{
			if (gpGlobals->time > m_tSecondaryAttackStart)
			{
				m_tSecondaryAttackStart = gpGlobals->time + 0.5s;

				int flags;
#ifdef CLIENT_WEAPONS
				flags = FEV_NOTHOST;
#else
				flags = 0;
#endif
				this->pev->iuser1 = 1;
				PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireLaserSg, 0, g_vecZero, g_vecZero, 2, 0, 2, 0, FALSE, TRUE);
				this->SendWeaponAnim(5, UseDecrement() != FALSE);
				EMIT_SOUND_DYN(this->edict(), CHAN_WEAPON, "weapons/lasersg-2.wav", VOL_NORM, ATTN_NORM, 0, 94);

				return CBasePlayerWeapon::ItemPostFrame();
			}
		}
		else
		{
			this->SendWeaponAnim(6, UseDecrement() != FALSE);
			this->pev->iuser1 = 0;

			EMIT_SOUND_DYN(this->edict(), CHAN_WEAPON, "weapons/lasersg-2_end.wav", VOL_NORM, ATTN_NORM, 0, 94);

			m_tSecondaryAttackStart = invalid_time_point;
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5s;
		}
	}
	if (m_iWeaponState == 25)
		return CBasePlayerWeapon::ItemPostFrame();

	if (m_fInReload > 0)
		return CBasePlayerWeapon::ItemPostFrame();

	if (m_flTimeNextCharge != invalid_time_point && m_flTimeNextCharge <= gpGlobals->time && !this->pev->iuser1)
	{
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iWeaponState += int((gpGlobals->time - m_flTimeNextCharge) / 0.5s + 1.0);

		if (m_iWeaponState >= 25)
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iWeaponState = 25;

		//SetChargeSkin();
		SendChargeState();

		m_flTimeNextCharge = gpGlobals->time + 0.5s;

		if (m_iWeaponState == 25)
			m_flTimeNextCharge = invalid_time_point;
	}

	return CBasePlayerWeapon::ItemPostFrame();
}
#endif
void CLaserSg::WeaponIdle(void)
{
	ResetEmptySound();
	ResetChargeTimer();
	m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
	SendWeaponAnim(0, UseDecrement() != FALSE);
}

void CLaserSg::Reload(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;
	m_flTimeNextCharge = invalid_time_point;

	if (DefaultReload(40, 1, 2.03s))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
	}
}

double CLaserSg::GetDamage_SecondaryAttack()
{
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZBS)
	{
		return 1000.0;//1300
	}
	if (g_pModRunning->DamageTrack() == DT_ZB)
	{
		return 400.0;//1100
	}
#endif
	return 10.0;
}
double CLaserSg::GetDamage_PrimaryAttack()
{
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZBS)
	{
		return 500.0;
	}
	if (g_pModRunning->DamageTrack() == DT_ZB)
	{
		return 300.0;//180
	}
#endif
	return 6.0;
}
}
