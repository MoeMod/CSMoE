/*
poisongun.cpp
Copyright (C) 2019 Moemod Haoyuan

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

#include "weapons/WeaponTemplate.hpp"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif
    class CPoisonGun : public LinkWeaponTemplate< CPoisonGun,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_FAMAS>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TGetDamageDefault
    >
    {
    public:

    static constexpr const char *V_Model = "models/v_poisongun.mdl";
    static constexpr const char *P_Model = "models/p_poisongun.mdl";
    static constexpr const char *W_Model = "models/w_poisongun.mdl";
    static constexpr const char *EventFile = "events/poisongun.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_poisongun";
    static constexpr const char *AnimExtension = "m249";
    static constexpr int MaxClip = 200;
	enum
	{
		ANIM_IDLE1 = 0,
		ANIM_SHOOT1,
		ANIM_SHOOT_END,
		ANIM_RELOAD,
		ANIM_DRAW,
	};
    static constexpr float PlayerMaxSpeed = 240;
    static constexpr float ArmorRatioModifier = 1.2f;

    static constexpr int DamageDefault = 22;
    static constexpr int DamageZB = 42;
    static constexpr int DamageZBS = 42;
    KnockbackData KnockBack = { 100.0f, 80.0f, 80.0f, 80.0f, 0.3f };

	time_point_t m_flNextFireTime;
	time_point_t m_flNextSoundTime;

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_poisongun" , 60 }; }
#endif
	float GetArmorRatioModifier() override { return 1.2; }
	WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 6, 3, 140 }; }

	void Spawn() override
	{
		m_flNextFireTime = m_flNextSoundTime = invalid_time_point;
		Base::Spawn();
	}

    void Precache(void) override
    {
		PRECACHE_SOUND("weapons/poisongun-1.wav");
		PRECACHE_SOUND("weapons/poisongun-2.wav");

		PRECACHE_MODEL("sprites/ef_smoke_poison.spr");

        Base::Precache();
    }

    int GetItemInfo(ItemInfo* p) override
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "poisongun";
        p->iMaxAmmo1 = 200;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = 200;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_POISONGUN;
        p->iFlags = 0;
        p->iWeight = M249_WEIGHT;

        return 1;
    }

	BOOL Deploy(void)
	{
		m_flAccuracy = 0;
		m_iShotsFired = 0;
		return Base::Deploy();
	}

	bool CheckFiringOnEmpty()
	{
		if (m_iClip > 0)
			return false;

		if (m_fFireOnEmpty)
		{
			PlayEmptySound();

			if (m_pPlayer->pev->weaponanim == ANIM_SHOOT1)
			{
				SendWeaponAnim(ANIM_SHOOT_END, UseDecrement() != FALSE);

				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/poisongun-2.wav", VOL_NORM, 0.52, 0, 94 + RANDOM_LONG(0, 15));

				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 10s;
			}

			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
		}

#ifndef CLIENT_DLL
		if (TheBots)
			TheBots->OnEvent(EVENT_WEAPON_FIRED, m_pPlayer);
#endif

		return true;
	}

	bool CheckUnderwater()
	{
		if (!IsPlayerUnderwater())
			return false;

		PlayEmptySound();
		SetCycleTime(0.1s);

		return true;
	}

	void FireRemaining(int& shotsFired, time_point_t& shootTime, BOOL isGlock18)
	{
		if (!m_iClip || shotsFired >= 1)
		{
			shotsFired = 1;
			shootTime = invalid_time_point;

			return;
		}

		m_iClip--;

		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecEnd = PoisonGunAttack(3, vecSrc, gpGlobals->v_forward, GetSpread(), 395, GetDamage(), GetDamageB(), 8, 0.88, m_pPlayer->pev);

		if (gpGlobals->time >= m_flNextSoundTime)
		{
			m_flNextSoundTime = gpGlobals->time + 0.9s;

			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/poisongun-1.wav", VOL_NORM, 0.52, 0, 94 + RANDOM_LONG(0, 15));
		}

		int flags;

#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecEnd.x, vecEnd.y, int(m_pPlayer->pev->punchangle.x * 100), int(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);

		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		if (++shotsFired >= 1)
			shootTime = invalid_time_point;
		else
			shootTime = gpGlobals->time + 0.06s;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
	}

	float GetSpread(void)
	{
		return 0.065;
	}

	bool IsPlayerUnderwater()
	{
		return m_pPlayer->pev->waterlevel == 3;
	}

	void PrimaryAttack(void) override
	{
		m_iFamasShotsFired = 0;

		if (CheckUnderwater())
			return;

		m_bDelayFire = true;

		if (CheckFiringOnEmpty())
			return;

		WeaponAnimation();
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 10s;

		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

		Vector vecDir = gpGlobals->v_forward;
		if (gpGlobals->time >= m_flNextFireTime)
		{
			m_iClip--;
			m_iShotsFired++;

			m_flNextFireTime = gpGlobals->time + 0.1s;
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
			Vector vecSrc = m_pPlayer->GetGunPosition();
			vecDir = PoisonGunAttack(3, vecSrc, gpGlobals->v_forward, GetSpread(), 395, GetDamage(), GetDamageB(), 8, 0.88, m_pPlayer->pev);
		}

		if (gpGlobals->time >= m_flNextSoundTime)
		{
			m_flNextSoundTime = gpGlobals->time + 0.9s;

			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/poisongun-1.wav", VOL_NORM, 0.52, 0, 94 + RANDOM_LONG(0, 15));
		}

		int flags;

#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, int(m_pPlayer->pev->punchangle.x * 100), int(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.1s;
#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", 0, 0);
#endif
		if (FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			KickBack(0.2, 0.0, 0.05, 0.0, 0.5, 0.0, 0);
		else if (m_pPlayer->pev->velocity.Length2D() > 0)
			KickBack(0.1, 0.0, 0.05, 0.0, 0.5, 0.0, 0);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			KickBack(0.1, 0.0, 0.05, 0.0, 0.5, 0.0, 0);
		else
			KickBack(0.1, 0.0, 0.05, 0.0, 0.5, 0.0, 0);

		m_iFamasShotsFired++;
		m_flFamasShoot = gpGlobals->time + 0.06s;
	}

	void SetCycleTime(duration_t flCycleTime)
	{
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
	}

	void WeaponAnimation(void)
	{
		if (m_pPlayer->pev->weaponanim != ANIM_SHOOT1)
			SendWeaponAnim(ANIM_SHOOT1, UseDecrement() != FALSE);
	}

	void Reload(void)
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			return;

		if (DefaultReload(200, ANIM_RELOAD, 4.8s))
		{
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
			m_flAccuracy = 0.0f;
			m_bDelayFire = false;
			m_iShotsFired = 0;
			m_flNextFireTime = invalid_time_point;
			m_flNextSoundTime = invalid_time_point;
		}
	}

	void WeaponIdle(void)
	{
		ResetEmptySound();

		m_flNextFireTime = invalid_time_point;
		m_flNextSoundTime = invalid_time_point;

		m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		if (m_pPlayer->pev->weaponanim == ANIM_SHOOT1)
		{
			SendWeaponAnim(ANIM_SHOOT_END, UseDecrement() != FALSE);

			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/poisongun-2.wav", VOL_NORM, 0.52, 0, 94 + RANDOM_LONG(0, 15));

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 10s;
		}

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
		SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);
	}


	float GetDamageB() const
	{
		float flDamage = 5.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
		{
			flDamage = 30.0f;
		}
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 30.0f;
#endif
		return flDamage;
	}

	Vector PoisonGunAttack(int iNumIterations, Vector vecSrc, Vector vecDir, float flSpread, float flRange, float flDamage, float flPoisonDamage, int iPoisonDamageCount, float flRangeModifier, entvars_t* pevAttacker)
	{
#ifndef CLIENT_DLL
		Vector vecResult;

		for (int i = iNumIterations; i >= 1; i--)
		{
			vecResult = PoisonAttack(vecSrc, vecDir, flSpread, flRange, flDamage, flPoisonDamage, iPoisonDamageCount, flRangeModifier, pevAttacker);
		}

		return vecResult;
#else
		return g_vecZero;
#endif
	}

#ifndef CLIENT_DLL
	Vector PoisonAttack(Vector vecSrc, Vector vecDir, float flSpread, float flRange, float flDamage, float flPoisonDamage, int iPoisonDamageCount, float flRangeModifier, entvars_t* pevAttacker)
	{
		Vector vecEnd;
		TraceResult tr;

		ClearMultiDamage();
		gMultiDamage.type = DMG_BULLET | DMG_NEVERGIB;

		vecEnd = vecSrc + vecDir * flRange + gpGlobals->v_right * RANDOM_FLOAT(-1, 1) * flSpread + gpGlobals->v_up * RANDOM_FLOAT(-1, 1) * flSpread;

		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pev), &tr);

		if (tr.iHitgroup == HITGROUP_HEAD)
			tr.iHitgroup = HITGROUP_CHEST;

		if (tr.flFraction < 1.0f)
		{
			CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

			float flCurrentDamage = pow(flRangeModifier, tr.flFraction * flRange / 500.0f) * flDamage;

			if (pEntity->IsPlayer() || pEntity->Classify() == CLASS_PLAYER_ALLY)
			{
				if (pEntity->IsPlayer())
				{
					CBasePlayer* pPlayer = (CBasePlayer*)pEntity;
					if (pPlayer->m_bIsZombie)
					{
						pPlayer->m_pBuffAttacker = pevAttacker;
						pPlayer->m_flHealthDecreaseAmount = flPoisonDamage;
						pPlayer->m_iHealthDecreaseCount = iPoisonDamageCount;
						pPlayer->m_flHealthDecreaseInterval = 1.0s;
					}		
				}
				else
				{
					pEntity->m_pBuffAttacker = pevAttacker;
					pEntity->m_flHealthDecreaseAmount = flPoisonDamage;
					pEntity->m_iHealthDecreaseCount = iPoisonDamageCount;
					pEntity->m_flHealthDecreaseInterval = 1.0s;
				}
			}

			if (g_pGameRules->PlayerRelationship(m_pPlayer, pEntity) != GR_TEAMMATE)
			{
				pEntity->TraceAttack(pevAttacker, flCurrentDamage, vecDir, &tr, DMG_BULLET);
			}

			ApplyMultiDamage(pev, pevAttacker);
		}

		return tr.vecEndPos;
	}
#endif
};
LINK_ENTITY_TO_CLASS(weapon_poisongun, CPoisonGun)
}