/*
flamethrower.cpp
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
    class CFlameThrower : public LinkWeaponTemplate< CFlameThrower,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_M249>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TGetDamageDefault
    >
    {
    public:

    static constexpr const char *V_Model = "models/v_flamethrower.mdl";
    static constexpr const char *P_Model = "models/p_flamethrower.mdl";
    static constexpr const char *W_Model = "models/w_flamethrower.mdl";
    static constexpr const char *EventFile = "events/flamethrower.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_flamethrower";
    static constexpr const char *AnimExtension = "m249";
    static constexpr int MaxClip = 100;
	enum
	{
		ANIM_IDLE1 = 0,
		ANIM_SHOOT1,
		ANIM_SHOOT_END,
		ANIM_RELOAD,
		ANIM_DRAW,
	};
    static constexpr float PlayerMaxSpeed = 230;
    static constexpr float ArmorRatioModifier = 1.2f;

    static constexpr int DamageDefault = 21;
    static constexpr int DamageZB = 54;
    static constexpr int DamageZBS = 99;
    KnockbackData KnockBack = { 100.0f, 80.0f, 80.0f, 80.0f, 0.88f };

	time_point_t m_flNextSoundTime;

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_nitrogengas" , 300 }; }
#endif
	float GetArmorRatioModifier() override { return 1.2; }
	WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 6, 3, 140 }; }

	void Spawn() override
	{
		m_flNextSoundTime = invalid_time_point;
		Base::Spawn();
	}

    void Precache(void) override
    {
		PRECACHE_SOUND("weapons/flamegun-1.wav");
		PRECACHE_SOUND("weapons/flamegun-2.wav");

		PRECACHE_MODEL("sprites/flame_puff01.spr");

        Base::Precache();
    }

    int GetItemInfo(ItemInfo* p) override
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "nitrogengas";
        p->iMaxAmmo1 = 200;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = 100;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_FLAMETHROWER;
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

	void PrimaryAttack(void) override
	{
		if (m_pPlayer->pev->waterlevel == 3)
		{
			PlayEmptySound();

			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.09s;
			return;
		}

		m_bDelayFire = true;
		m_flAccuracy = (m_iShotsFired * m_iShotsFired * m_iShotsFired) / 175.0 + 0.0;

		if (m_flAccuracy > 0.0)
			m_flAccuracy = 0.0;

		if (m_iClip <= 0)
		{
			if (m_fFireOnEmpty)
			{
				PlayEmptySound();

				if (m_pPlayer->pev->weaponanim == ANIM_SHOOT1)
				{
					SendWeaponAnim(ANIM_SHOOT_END, UseDecrement() != FALSE);

					EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/flamegun-2.wav", VOL_NORM, ATTN_NORM, 0, 94 + RANDOM_LONG(0, 15));

					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.2s;
				}

				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
			}

#ifndef CLIENT_DLL
			if (TheBots)
				TheBots->OnEvent(EVENT_WEAPON_FIRED_ON_EMPTY, m_pPlayer);
#endif

			return;
		}
		if (m_pPlayer->pev->weaponanim != ANIM_SHOOT1)
		{
			SendWeaponAnim(ANIM_SHOOT1, UseDecrement() != FALSE);
	}

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.06s;

		Vector vecSrc = m_pPlayer->GetGunPosition();
		float flDamage = GetDamage();

		m_iClip--;
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
								pPlayer->m_flHealthDecreaseAmount = GetDamageB();
								pPlayer->m_iHealthDecreaseCount = 5;
								pPlayer->m_flHealthDecreaseInterval = 1.0s;
							}
						}
						else
						{
							pEntity->m_pBuffAttacker = m_pPlayer->pev;
							pEntity->m_flHealthDecreaseAmount = GetDamageB();
							pEntity->m_iHealthDecreaseCount = 5;
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

		int flags;

#ifndef CLIENT_WEAPONS
		flags = 0;
#else
		flags = FEV_NOTHOST;
#endif

		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0.0, g_vecZero, g_vecZero, 0.0f, 0.0f, int(m_pPlayer->pev->punchangle.x * 100), int(m_pPlayer->pev->punchangle.y * 100), 385, FALSE);

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

	void Reload(void)
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			return;

		if (DefaultReload(100, ANIM_RELOAD, 4.8s))
		{
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
			m_flAccuracy = 0.0f;
			m_bDelayFire = false;
			m_iShotsFired = 0;

			m_flNextSoundTime = invalid_time_point;
		}
	}

	void WeaponIdle(void)
	{
		ResetEmptySound();

		m_flNextSoundTime = invalid_time_point;

		m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		if (m_pPlayer->pev->weaponanim == ANIM_SHOOT1)
		{
			SendWeaponAnim(ANIM_SHOOT_END, UseDecrement() != FALSE);

			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/flamegun-2.wav", VOL_NORM, 0.52, 0, 94 + RANDOM_LONG(0, 15));

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 10s;
		}

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
		SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);
	}


	float GetDamageB() const
	{
		float flDamage = 1.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
		{
			flDamage = 40.0f;
		}
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 30.0f;
#endif
		return flDamage;
	}

};
LINK_ENTITY_TO_CLASS(weapon_flamethrower, CFlameThrower)
}