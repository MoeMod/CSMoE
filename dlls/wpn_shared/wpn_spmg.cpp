/*
spmg.cpp
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
    class CSPMG : public LinkWeaponTemplate< CSPMG,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_M249>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TReloadDefault,
            TGetDamageDefault,
            TPrimaryAttackRifle,
            TFireRifle,
            TWeaponIdleDefault
    >
    {
    public:
    static constexpr const auto &DefaultReloadTime = 4.7s;
    static constexpr const char *V_Model = "models/v_spmg.mdl";
    static constexpr const char *P_Model = "models/p_spmg.mdl";
    static constexpr const char *W_Model = "models/w_spmg.mdl";
    static constexpr const char *EventFile = "events/spmg.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_spmg";
    static constexpr const char *AnimExtension = "m249";
    static constexpr int MaxClip = 150;
	enum
	{
		ANIM_IDLE1 = 0,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_RELOAD,
		ANIM_DRAW,
	};
    static constexpr float PlayerMaxSpeed = 225;
    static constexpr float ArmorRatioModifier = 1.5f;
	static constexpr float PrimaryAttackWalkingMiniumSpeed = 140;
    static constexpr const auto &SpreadCalcNotOnGround = 0.5*A + 0.0365;
    static constexpr const auto &SpreadCalcWalking = 0.0775*A + 0.045;
    static constexpr const auto &SpreadCalcDefault = 0.03*A;

	static constexpr const auto& CycleTime = 0.1s;
    static constexpr int DamageDefault = 34;
    static constexpr int DamageZB = 60;
    static constexpr int DamageZBS = 70;
    static constexpr const auto &AccuracyCalc = (N * N * N / 190.0) + 0.385;
    static constexpr float AccuracyDefault = 0.2;
    static constexpr float AccuracyMax = 0.92;
    static constexpr float RangeModifier = 0.97;
    static constexpr auto BulletType = BULLET_PLAYER_556MM;
    /*static constexpr*/ int Penetration = 2;
    KnockbackData KnockBack = { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f };

	KickBackData KickBackWalking = { 1.1f, 0.5f, 0.3f, 0.06f, 4.0f, 3.0f, 8 };
	KickBackData KickBackNotOnGround = { 1.8f, 0.65f, 0.45f, 0.125f, 5.0f, 3.5f, 8 };
	KickBackData KickBackDucking = { 0.75f, 0.325f, 0.25f, 0.025f, 3.5f, 2.5f, 9 };
	KickBackData KickBackDefault = { 0.8f, 0.35f, 0.3f, 0.03f, 3.85f, 2.72f, 7 };
    
    int m_iSecondaryClip;
	time_point_t m_flNextSoundTime;

	void Spawn()
	{
		m_flNextSoundTime = invalid_time_point;
		m_iSecondaryClip = 0;
		Base::Spawn();
	}

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "556NatoBox";
        p->iMaxAmmo1 = MAX_AMMO_556NATOBOX;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = 150;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_SPMG;
        p->iFlags = 0;
        p->iWeight = M249_WEIGHT;

        return 1;
    }

	void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
	{
		m_bDelayFire = true;
		m_iShotsFired++;
		m_flAccuracy = (m_iShotsFired * m_iShotsFired * m_iShotsFired) / 190.0 + 0.385;

		if (m_flAccuracy > 0.92)
			m_flAccuracy = 0.92;

		if (m_iClip <= 0)
		{
			if (m_fFireOnEmpty)
			{
				PlayEmptySound();

				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
			}

#ifndef CLIENT_DLL
			if (TheBots)
				TheBots->OnEvent(EVENT_WEAPON_FIRED_ON_EMPTY, m_pPlayer);
#endif

			return;
		}

		m_iClip--;
		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		if (m_iSecondaryClip < 100)
			m_iSecondaryClip++;

		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
		m_pPlayer->m_iWeaponVolume = BRIGHT_GUN_FLASH;

		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecDir;

		float flDamage = GetDamage();
		vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_556MM, flDamage, 0.97, m_pPlayer->pev, false, m_pPlayer->random_seed);

#ifndef CLIENT_DLL
		PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, static_cast<int>(m_pPlayer->pev->punchangle.x * 100), static_cast<int>(m_pPlayer->pev->punchangle.y * 100), m_iSecondaryClip, TRUE);
#endif

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", 0, 0);
#endif
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.6s;

		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			KickBack(1.8f, 0.65f, 0.45f, 0.125f, 5.0f, 3.5f, 8);
		else if (m_pPlayer->pev->velocity.Length2D() > 0)
			KickBack(1.1f, 0.5f, 0.3f, 0.06f, 4.0f, 3.0f, 8);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			KickBack(0.75f, 0.325f, 0.25f, 0.025f, 3.5f, 2.5f, 9);
		else
			KickBack(0.8f, 0.35f, 0.3f, 0.03f, 3.85f, 2.72f, 7);
	}

	void SecondaryAttack(void) override
	{
		if (m_pPlayer->pev->waterlevel == 3)
		{
			PlayEmptySound();

			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2s;
			return;
		}

		m_bDelayFire = true;
		m_flAccuracy = (m_iShotsFired * m_iShotsFired * m_iShotsFired) / 175.0 + 0.0;

		if (m_flAccuracy > 0.0)
			m_flAccuracy = 0.0;

		if (m_iSecondaryClip <= 1)
		{
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2s;
			return;
		}
			
		m_iSecondaryClip -= 2;

		SendWeaponAnim(ANIM_SHOOT2, UseDecrement() != FALSE);

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.3s;

		Vector vecSrc = m_pPlayer->GetGunPosition();
		float flDamage = GetDamageB();

		m_iShotsFired++;
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		if (gpGlobals->time >= m_flNextSoundTime)
		{
			m_flNextSoundTime = gpGlobals->time + 0.9s;

			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/flamegun-1.wav", VOL_NORM, ATTN_NORM, 0, 94 + RANDOM_LONG(0, 15));
		}

#ifndef CLIENT_DLL
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);

		for (int i = 0; i < 4; i++)
		{
			Vector vecStart, vecEnd;
			TraceResult tr;
			auto pTargetSet = new std::set<CBaseEntity*>;

			switch (i)
			{
			case 0:
				vecStart = vecSrc - gpGlobals->v_up.Normalize() * 8 + gpGlobals->v_up.Normalize() * 15;
				vecEnd = vecStart + gpGlobals->v_forward * 385;

				break;

			case 1:
				vecStart = vecSrc - gpGlobals->v_up.Normalize() * 8 + (-gpGlobals->v_up).Normalize() * 15;
				vecEnd = vecStart + gpGlobals->v_forward * 385;

				break;

			case 2:
				vecStart = vecSrc - gpGlobals->v_up.Normalize() * 8 + (-gpGlobals->v_right).Normalize() * 15;
				vecEnd = vecStart + gpGlobals->v_forward * 385;

				break;

			case 3:
				vecStart = vecSrc - gpGlobals->v_up.Normalize() * 8 + gpGlobals->v_right.Normalize() * 15;
				vecEnd = vecStart + gpGlobals->v_forward * 385;

				break;
			}

			for (int i = 1; i <= 32; i++)
			{
				UTIL_TraceLine(vecStart, vecEnd, dont_ignore_monsters, m_pPlayer->edict(), &tr);

				if (tr.iHitgroup == HITGROUP_HEAD)
					tr.iHitgroup = HITGROUP_CHEST;

				if (tr.flFraction >= 1.0f)
					break;

				CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

				if (pEntity == m_pPlayer)
					break;

				if (pTargetSet->find(pEntity) == pTargetSet->end())
				{
					ClearMultiDamage();

					pEntity->TraceAttack(m_pPlayer->pev, flDamage, gpGlobals->v_forward, &tr, DMG_BULLET | DMG_NEVERGIB);

					ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

					pTargetSet->insert(pEntity);

					if (pEntity->IsPlayer() || pEntity->Classify() == CLASS_PLAYER_ALLY)
					{
						if (pEntity->IsPlayer())
						{
							CBasePlayer* pPlayer = (CBasePlayer*)pEntity;
							if (pPlayer->m_bIsZombie)
							{
								pPlayer->m_pBuffAttacker = m_pPlayer->pev;
								pPlayer->m_flHealthDecreaseAmount = GetDamageC();
								pPlayer->m_iHealthDecreaseCount = 8;
								pPlayer->m_flHealthDecreaseInterval = 1.0s;
							}
						}
						else
						{
							pEntity->m_pBuffAttacker = m_pPlayer->pev;
							pEntity->m_flHealthDecreaseAmount = GetDamageC();
							pEntity->m_iHealthDecreaseCount = 8;
							pEntity->m_flHealthDecreaseInterval = 1.0s;
						}
					}

				}

				if (!tr.pHit || !pEntity->IsPlayer())
					break;

				vecStart = vecStart + (vecEnd - vecStart) * tr.flFraction;
			}

			delete pTargetSet;
		}
#endif

#ifndef CLIENT_DLL
		PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0.0, g_vecZero, g_vecZero, 0.0f, 0.0f, int(m_pPlayer->pev->punchangle.x * 100), int(m_pPlayer->pev->punchangle.y * 100), m_iSecondaryClip, FALSE);
#endif
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.09s;
#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", 0, 0);
#endif
		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			KickBack(0.2, 0.0, 0.05, 0.0, 0.5, 0.0, 0);
		else if (m_pPlayer->pev->velocity.Length2D() > 0)
			KickBack(0.1, 0.0, 0.05, 0.0, 0.5, 0.0, 0);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			KickBack(0.1, 0.0, 0.05, 0.0, 0.5, 0.0, 0);
		else
			KickBack(0.1, 0.0, 0.05, 0.0, 0.5, 0.0, 0);
	}

	float GetDamageB() const
	{
		float flDamage = 35;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 65;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 153;
#endif
		return flDamage;
	}

	float GetDamageC() const
	{
		float flDamage = 5;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 25;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 75;
#endif
		return flDamage;
	}


#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_M249); }
#endif
    float GetArmorRatioModifier() override { return 1.5; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 6, 3, 140 }; }

};
LINK_ENTITY_TO_CLASS(weapon_spmg, CSPMG)
}