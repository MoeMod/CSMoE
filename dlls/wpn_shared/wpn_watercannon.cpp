/*
watercannon.cpp
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
    class CWaterCannon : public LinkWeaponTemplate< CWaterCannon,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_M249>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TGetDamageDefault
    >
    {
    public:

    static constexpr const char *V_Model = "models/v_watercannon.mdl";
    static constexpr const char *P_Model = "models/p_watercannon.mdl";
    static constexpr const char *W_Model = "models/w_watercannon.mdl";
    static constexpr const char *EventFile = "events/watercannon.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_watercannon";
    static constexpr const char *AnimExtension = "m134";
    static constexpr int MaxClip = 100;
	enum
	{
		ANIM_IDLE1 = 0,
		ANIM_SHOOT_START,
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
    KnockbackData KnockBack = { 100.0f, 80.0f, 80.0f, 80.0f, 0.25f };

	time_point_t m_flNextSoundTime;
	time_point_t m_flAttackLoop;

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_liquefiedgas" , 300 }; }
#endif
	float GetArmorRatioModifier() override { return 1.2; }
	WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 6, 3, 140 }; }

	void Spawn() override
	{
		m_flAttackLoop = invalid_time_point;
		m_flNextSoundTime = invalid_time_point;
		Base::Spawn();
	}

    void Precache(void) override
    {
		PRECACHE_SOUND("weapons/watercannon_shoot_start.wav");
		PRECACHE_SOUND("weapons/watercannon_shoot1.wav");
		PRECACHE_SOUND("weapons/watercannon_shoot_end.wav");

		PRECACHE_MODEL("sprites/holybomb_exp.spr");

        Base::Precache();
    }

    int GetItemInfo(ItemInfo* p) override
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "liquefiedgas";
        p->iMaxAmmo1 = 200;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = 100;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_WATERCANNON;
        p->iFlags = 0;
        p->iWeight = M249_WEIGHT;

        return 1;
    }

	BOOL Deploy(void)
	{
		m_flAttackLoop = invalid_time_point;
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

				if (m_flAttackLoop != invalid_time_point)
				{
					SendWeaponAnim(ANIM_SHOOT_END, UseDecrement() != FALSE);

					EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/watercannon_shoot_end.wav", VOL_NORM, ATTN_NORM, 0, 94 + RANDOM_LONG(0, 15));
					m_flAttackLoop = invalid_time_point;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.73s;
				}

				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
			}

#ifndef CLIENT_DLL
			if (TheBots)
				TheBots->OnEvent(EVENT_WEAPON_FIRED_ON_EMPTY, m_pPlayer);
#endif

			return;
		}

		if (m_flAttackLoop == invalid_time_point)
		{
			SendWeaponAnim(ANIM_SHOOT_START, UseDecrement() != FALSE);

			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/watercannon_shoot_start.wav", VOL_NORM, ATTN_NORM, 0, 94 + RANDOM_LONG(0, 15));

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.23s;
			m_flAttackLoop = gpGlobals->time + 0.23s;
			return;
		}

		if (m_flAttackLoop > gpGlobals->time)
			return;


		if (m_pPlayer->pev->weaponanim != ANIM_SHOOT1)
		{
			SendWeaponAnim(ANIM_SHOOT1, UseDecrement() != FALSE);
		}

		Vector vecSrc = m_pPlayer->GetGunPosition();
		float flDamage = GetDamage();

		m_iClip--;
		m_iShotsFired++;
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		if (gpGlobals->time >= m_flNextSoundTime)
		{
			m_flNextSoundTime = gpGlobals->time + 0.5s;

			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/watercannon_shoot1.wav", VOL_NORM, ATTN_NORM, 0, 94);
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

		m_flTimeWeaponIdle = m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.09s;
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

		if (DefaultReload(100, ANIM_RELOAD, 4.0s))
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

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		if (m_flAttackLoop != invalid_time_point)
		{
			SendWeaponAnim(ANIM_SHOOT_END, UseDecrement() != FALSE);

			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/watercannon_shoot_end.wav", VOL_NORM, 0.52, 0, 94 + RANDOM_LONG(0, 15));

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
			m_flAttackLoop = invalid_time_point;
		}
		else
		{
			SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0s;
		}

	}

};
LINK_ENTITY_TO_CLASS(weapon_watercannon, CWaterCannon)
}