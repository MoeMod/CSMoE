/*
wpn_speargunex.cpp - Black Dragon speargunex from CSO
Copyright (C) 2019 Moemod Hyakuya

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "wpn_speargun.h"
#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_speargunex, CSpearGunEX)

enum speargunex_e
{
	SPEARGUNEX_IDLE1,
	SPEARGUNEX_SHOOT1,
	SPEARGUNEX_RELOAD,
	SPEARGUNEX_DRAW,
	SPEARGUNEX_DRAW_EMPTY,
	SPEARGUNEX_IDLE_EMPTY,
	SPEARGUNEX_CHARGE_START,
	SPEARGUNEX_CHARGE_LOOP,
	SPEARGUNEX_CHARGE_FINISH,
	SPEARGUNEX_CHARGE_FINISH_LOOP,
	SPEARGUNEX_CHARGE_SHOOT,
};

int CSpearGunEX::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "SpearEXAmmo";
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 150;
	p->iSlot = 0;
	p->iPosition = 1;
	p->iId = m_iId = WEAPON_SPEARGUNEX;
	p->iFlags = 0;
	p->iWeight = AK47_WEIGHT;

	return 1;
}

void CSpearGunEX::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_speargunex");

	Precache();

	m_iId = WEAPON_SPEARGUNEX;
	SET_MODEL(ENT(pev), "models/w_speargunex.mdl");

	FallInit();

	m_flAccuracy = AccuracyDefault;
	m_iShotsFired = 0;
	m_bDelayFire = false;

	m_iDefaultAmmo = 30;

	m_flTimeCheckCharge = invalid_time_point;
}

void CSpearGunEX::Precache(void)
{
	PRECACHE_MODEL("models/v_speargunex.mdl");
	PRECACHE_MODEL("models/p_speargunex.mdl");
	PRECACHE_MODEL("models/w_speargunex.mdl");
	PRECACHE_MODEL("sprites/muzzleflash355.spr");

	PRECACHE_SOUND("weapons/speargun_wood1.wav");
	PRECACHE_SOUND("weapons/speargun_wood2.wav");
	PRECACHE_SOUND("weapons/speargun_metal1.wav");
	PRECACHE_SOUND("weapons/speargun_metal2.wav");
	PRECACHE_SOUND("weapons/speargun_stone1.wav");
	PRECACHE_SOUND("weapons/speargun_stone2.wav");

	m_usFireSpearGunEX = PRECACHE_EVENT(1, "events/speargunex.sc");
}

BOOL CSpearGunEX::Deploy(void)
{
	m_flAccuracy = AccuracyDefault;
	m_iShotsFired = 0;
	m_bDelayFire = false;

	iShellOn = 0;

	if (DefaultDeploy("models/v_speargunex.mdl", "models/p_speargunex.mdl", m_iClip > 0 ? SPEARGUNEX_DRAW : SPEARGUNEX_DRAW_EMPTY, "carbine", UseDecrement() != FALSE))
	{
		pev->skin = 0;
		return TRUE;
	}
	return FALSE;

}

float CSpearGunEX::GetSpread(void)
{
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		return m_flAccuracy * SpreadMulJump + SpreadAddJump;
	else if (m_pPlayer->pev->velocity.Length2D() > 120)
		return m_flAccuracy * SpreadMulMove + SpreadAddMove;
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		return m_flAccuracy * SpreadMulDuck + SpreadAddDuck;
	else
		return m_flAccuracy * SpreadMul + AccuracyAdd;
}

void CSpearGunEX::SpearGunFire()
{
	if (m_iClip <= 0)
		return;

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;

	m_iClip--;

	m_pPlayer->pev->effects &= ~EF_MUZZLEFLASH;

	m_flTimeReloadSpear = gpGlobals->time + ShootTime;

#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	Vector vecSrc = m_pPlayer->GetGunPosition() - gpGlobals->v_up * 2;
	Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, GetSpread(), 8192, 0, BULLET_PLAYER_556MM, 0, 0, m_pPlayer->pev, false, m_pPlayer->random_seed);
	Vector vecSpearDir = gpGlobals->v_forward + gpGlobals->v_right * vecDir.x + gpGlobals->v_up * vecDir.y;


	Vector vecAngles = UTIL_VecToAngles(vecSpearDir);

	int iType;
	float flSpearSpeed;
	float flHitDamage;
	float flCrowdDamage;
	float flExpDamage;
	duration_t flPushDuration;
	float ExplodeForce;
	float flPushForce;
	float flPushBoostVel;
	float flCrowdForce;
	float flCrowdBoostVel;

	if (m_iWeaponState > 2)
	{
		iType = 2;
		flSpearSpeed = ChargingShotSpeed;
		flHitDamage = FullChargingHitDamage;
		flExpDamage = FullChargingExpDamage;
		flPushDuration = AmmoHookingTime;
		ExplodeForce = 0.0;
		flPushForce = MainKickSpeed;
		flPushBoostVel = MainKickSpeedZ;
		flCrowdForce = SubKickSpeed;
		flCrowdBoostVel = SubKickSpeedZ;

		if (g_pModRunning->DamageTrack() == DT_ZB)
		{
			flHitDamage = FullChargingHitDamageZombie;
			flExpDamage = FullChargingExpDamageZombie;
			ExplodeForce = 0.0;
			flPushForce = MainKickSpeed;
			flPushBoostVel = MainKickSpeedZ;
			flCrowdForce = SubKickSpeed;
			flCrowdBoostVel = SubKickSpeedZ;
		}
		if (g_pModRunning->DamageTrack() == DT_ZBS)
		{
			flHitDamage = FullChargingHitDamageScenario;
			flExpDamage = FullChargingExpDamageScenario;
			ExplodeForce = 0.0;
			flPushForce = MainKickSpeed;
			flPushBoostVel = MainKickSpeedZ;
			flCrowdForce = SubKickSpeed;
			flCrowdBoostVel = SubKickSpeedZ;
		}
	}
	else if (m_iWeaponState > 1)
	{
		iType = 1;
		flSpearSpeed = ChargingShotSpeed;
		flHitDamage = ChargingHitDamage;
		flExpDamage = ChargingExpDamage;
		flPushDuration = AmmoHookingTime;
		ExplodeForce = 0.0;
		flPushForce = MainKickSpeed;
		flPushBoostVel = MainKickSpeedZ;
		flCrowdForce = SubKickSpeed;
		flCrowdBoostVel = SubKickSpeedZ;

		if (g_pModRunning->DamageTrack() == DT_ZB)
		{
			flHitDamage = ChargingHitDamageZombie;
			flExpDamage = ChargingExpDamageZombie;
			flPushDuration = AmmoHookingTime;
			ExplodeForce = 0.0;
			flPushForce = MainKickSpeed;
			flPushBoostVel = MainKickSpeedZ;
			flCrowdForce = SubKickSpeed;
			flCrowdBoostVel = SubKickSpeedZ;
		}
		if (g_pModRunning->DamageTrack() == DT_ZBS)
		{
			flHitDamage = ChargingHitDamageScenario;
			flExpDamage = ChargingExpDamageScenario;
			flPushDuration = AmmoHookingTime;
			ExplodeForce = 0.0;
			flPushForce = MainKickSpeed;
			flPushBoostVel = MainKickSpeedZ;
			flCrowdForce = SubKickSpeed;
			flCrowdBoostVel = SubKickSpeedZ;
		}
	}
	else
	{
		iType = 0;
		flSpearSpeed = AmmoSpeed;
		flHitDamage = Damage;
		flExpDamage = ExpDamage;
		flPushDuration = AmmoHookingTime;
		ExplodeForce = 100.0;
		flPushForce = MainKickSpeed;
		flPushBoostVel = MainKickSpeedZ;
		flCrowdForce = SubKickSpeed;
		flCrowdBoostVel = SubKickSpeedZ;

		if (g_pModRunning->DamageTrack() == DT_ZB)
		{
			flHitDamage = DamageZombie;
			flExpDamage = ExpDamageZombie;
			flPushDuration = AmmoHookingTimeZombie;
			ExplodeForce = 350.0;
			flPushForce = MainKickSpeedZombie;
			flPushBoostVel = MainKickSpeedZZombie;
			flCrowdForce = SubKickSpeedZombie;
			flCrowdBoostVel = SubKickSpeedZZombie;
		}
		if (g_pModRunning->DamageTrack() == DT_ZBS)
		{
			flHitDamage = DamageScenario;
			flExpDamage = ExpDamageScenario;
			flPushDuration = AmmoHookingTimeScenario;
			ExplodeForce = 350.0;
			flPushForce = MainKickSpeedScenario;
			flPushBoostVel = MainKickSpeedZScenario;
			flCrowdForce = SubKickSpeedScenario;
			flCrowdBoostVel = SubKickSpeedZScenario;
		}
	}

	CSpearEX* pSpear = CSpearEX::Create(iType);

	if (pSpear)
	{
		pSpear->m_iTeam = m_pPlayer->m_iTeam;
		pSpear->pev->origin = vecSrc;
		pSpear->pev->angles = vecAngles;
		pSpear->pev->owner = m_pPlayer->edict();
		pSpear->m_vecVelocity = pSpear->pev->velocity = vecSpearDir * flSpearSpeed;
		pSpear->pev->avelocity.z = 5;
		pSpear->m_flTimeExplode = gpGlobals->time + AmmoLifeTime;
		pSpear->m_vecDirection = pSpear->pev->velocity.Normalize();

		pSpear->m_flPushDuration = flPushDuration;
		pSpear->m_flHitDamage = flHitDamage;
		pSpear->m_flExplodeDamage = flExpDamage;
		pSpear->m_flOwnerDamageModifier = 0.1;
		pSpear->m_flCrowdDamage = TouchDamage;	
		pSpear->m_flExplodeForce = ExplodeForce;
		pSpear->m_flPushForce = flPushForce;
		pSpear->m_flPushBoostVel = flPushBoostVel;
		pSpear->m_flCrowdForce = flCrowdForce;
		pSpear->m_flCrowdBoostVel = flCrowdBoostVel;
		pSpear->m_pOwner = m_pPlayer;

		if (iType == 0)
		{
			UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

			float AutoAimDegree = 30.0f;
			Vector vecEnd;
			vecSrc = m_pPlayer->GetGunPosition();
			Vector vecAngleCheckDir = gpGlobals->v_forward.Normalize();
			float angCheck = cos(AutoAimDegree * M_PI / 180.0);

			CBaseEntity* pEntity = nullptr;
			TraceResult tr;
			float Dis = 4096;
			while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, 4096)) != nullptr)
			{
				if (pEntity->edict() == pev->owner)
					continue;

				if (pEntity->IsDormant())
					continue;

				if (pEntity->pev->takedamage == DAMAGE_NO)
					continue;

				if (pEntity->Classify() != CLASS_PLAYER_ALLY && !pEntity->IsPlayer())
					continue;

				vecEnd = pEntity->BodyTarget(vecSrc);

				if (DotProduct((vecEnd - vecSrc).Normalize(), vecAngleCheckDir) < angCheck)
					continue;

				UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pev), &tr);

				if (tr.flFraction < 1.0 && tr.pHit != ENT(pEntity->pev))
					continue;

				float flDelta = (vecSrc - vecEnd).Length();

				if (flDelta < Dis)
				{
					pSpear->m_hTarget.Set(pEntity->edict());
					Dis = flDelta;
				}
			}
		}
		
	}
#endif


	int flags;

#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireSpearGunEX, 0, g_vecZero, g_vecZero, 0, 0, 0, m_iClip, m_iWeaponState <= 1, FALSE);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + CycleTime;

	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		KickBack(15.0, 10.0, 0.6, 0.35, 9.0, 6.0, 5);
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
		KickBack(15.0, 10.0, 0.225, 0.05, 6.5, 2.5, 7);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		KickBack(4.0, 3.0, 0.125, 0.02, 5.0, 1.35, 9);
	else
		KickBack(10.0, 7.0, 0.22, 0.038, 5.9, 1.9, 8);

	pev->skin = 0;
}

void CSpearGunEX::PrimaryAttack(void)
{
	SpearGunFire();
}

void CSpearGunEX::OnAttack2Released()
{
	if (m_iWeaponState > 2)
	{
		SpearGunFire();
	}
	else if (m_iWeaponState > 1)
	{
		SpearGunFire();
	}
	else
	{
		PrimaryAttack();
	}

	m_iWeaponState = 0;
}

void CSpearGunEX::SecondaryAttack(void)
{
	if (m_iWeaponState)
	{
		if (gpGlobals->time > m_flTimeCheckCharge)
		{
			switch (m_iWeaponState)
			{
			case 1:
			{
				SendWeaponAnim(SPEARGUNEX_CHARGE_LOOP);

				m_iWeaponState = 2;
				m_flTimeCheckCharge = gpGlobals->time + SecondaryChargingTime;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0s;
				break;
			}
			case 2:
			{
				SendWeaponAnim(SPEARGUNEX_CHARGE_FINISH);
				pev->skin = 1;

				UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/lightningar_idle3.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM, 1);

				m_iWeaponState = 3;
				m_flTimeCheckCharge = gpGlobals->time + 0.53s;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
				break;
			}
			case 3:
			{
				SendWeaponAnim(SPEARGUNEX_CHARGE_FINISH_LOOP);

				m_iWeaponState = 4;
				m_flTimeCheckCharge = gpGlobals->time + 0.53s;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 9999.0s;		
				break;
			}
			case 4:
			{
				UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/lightningar_idle3.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM, 1);

				m_flTimeCheckCharge = gpGlobals->time + 4.0s;
				break;
			}

			default:
				break;
			}
		}
	}
	else
	{

		SendWeaponAnim(SPEARGUNEX_CHARGE_START);

		m_flTimeCheckCharge = gpGlobals->time + SecondaryChargingMinTime;
		m_iWeaponState = 1;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
	}

}

void CSpearGunEX::ItemPostFrame(void)
{
	if (m_flTimeReloadSpear != invalid_time_point && gpGlobals->time > m_flTimeReloadSpear && m_iClip > 0)
	{
		SendWeaponAnim(SPEARGUNEX_RELOAD);

		m_flTimeReloadSpear = invalid_time_point;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + ReloadTime;
	}
	else
	{
		CBasePlayerWeapon::ItemPostFrame();
	}
}

void CSpearGunEX::WeaponIdle(void)
{
	ResetEmptySound();

	if (m_iWeaponState && !(m_pPlayer->pev->button & IN_ATTACK2))
	{
		OnAttack2Released();
	}

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	int iAnim;

	if (m_iClip <= 0)
		iAnim = SPEARGUNEX_IDLE_EMPTY;
	else
		iAnim = SPEARGUNEX_IDLE1;

	SendWeaponAnim(iAnim);

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0s;
}
}
