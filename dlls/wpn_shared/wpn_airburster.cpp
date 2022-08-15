/*
airburster.cpp
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
    class CAirBurster : public LinkWeaponTemplate< CAirBurster,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_M249>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TGetDamageDefault
    >
    {
    public:

    static constexpr const char *V_Model = "models/v_airburster.mdl";
    static constexpr const char *P_Model = "models/p_airburster.mdl";
    static constexpr const char *W_Model = "models/w_airburster.mdl";
    static constexpr const char *EventFile = "events/airburster.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_airburster";
    static constexpr const char *AnimExtension = "m249";
    static constexpr int MaxClip = 100;
	enum
	{
		ANIM_IDLE1 = 0,
		ANIM_SHOOT1,
		ANIM_SHOOT1_END,
		ANIM_RELOAD,
		ANIM_DRAW,
        ANIM_SHOOT2,
	};
    static constexpr float PlayerMaxSpeed = 230;
    static constexpr float ArmorRatioModifier = 1.2f;

    static constexpr int DamageDefault = 17;
    static constexpr int DamageZB = 58;
    static constexpr int DamageZBS = 140;
    KnockbackData KnockBack = { 700.0f, 600.0f, 500.0f, 400.0f, 0.85f };

    time_point_t m_flNextSoundTime;
    unsigned short m_usModelIndexExplosion;
    int m_iTickCounter;

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_airgas" , 60 }; }
#endif
	float GetArmorRatioModifier() override { return 1.2; }
	WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 6, 3, 140 }; }

	void Spawn() override
	{
		m_flNextSoundTime = invalid_time_point;
		m_iTickCounter = 0;
		Base::Spawn();
	}

    void Precache(void) override
    {
        PRECACHE_SOUND("weapons/airburster_shoot.wav");
        PRECACHE_SOUND("weapons/airburster_shoot2.wav");
        PRECACHE_SOUND("weapons/airburster_shoot_end.wav");
        m_usModelIndexExplosion = PRECACHE_MODEL("sprites/ef_airexplosion.spr");
        PRECACHE_MODEL("sprites/ef_aircyclone.spr");
		PRECACHE_MODEL("sprites/muzzleflash35.spr");

        Base::Precache();
    }

    int GetItemInfo(ItemInfo* p) override
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "airgas";
        p->iMaxAmmo1 = 200;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = 100;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_AIRBURSTER;
        p->iFlags = 0;
        p->iWeight = M249_WEIGHT;

        return 1;
    }

	BOOL Deploy(void)
	{
		m_flAccuracy = 0;
		m_iShotsFired = 0;
		m_iTickCounter = 0;
		return Base::Deploy();
	}

	void ItemPostFrame(void) override
	{
		CBasePlayerWeapon::ItemPostFrame();

		if (m_iTickCounter)
		{
			if (m_iTickCounter % 10 == 0)
			{
				m_iTickCounter = 0;

				if (m_pPlayer->pev->weaponanim == ANIM_SHOOT1)
					PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usFire, 0.0, g_vecZero, g_vecZero, gpGlobals->v_forward.x, gpGlobals->v_forward.y, int(m_pPlayer->pev->punchangle.x * 100), int(m_pPlayer->pev->punchangle.y * 100), 385, FALSE);
			}
		}

		if (++m_iTickCounter > 99999999)
			m_iTickCounter = 0;
	}

	void PrimaryAttack(void) override
	{
		Fire(m_flAccuracy * 0.0 + 0.0, 0.09s, FALSE);
	}

	void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
	{
		if (m_pPlayer->pev->waterlevel == 3)
		{
			PlayEmptySound();

			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
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
					SendWeaponAnim(ANIM_SHOOT1_END, UseDecrement() != FALSE);

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

			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/airburster_shoot.wav", VOL_NORM, ATTN_NORM, 0, 94 + RANDOM_LONG(0, 15));
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

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
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

	void SecondaryAttack(void) override
	{
		if (m_pPlayer->pev->waterlevel == 3)
		{
			PlayEmptySound();

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.1s;
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.2s;

			return;
		}

		if (m_iClip <= 0)
		{
			Reload();

			return;
		}

		float flDamage = GetDamageB();
		float flKickRate = GetKickRate();

		m_iClip -= 10;

		if (m_iClip <= 0)
			m_iClip = 0;

		m_iShotsFired++;
		m_bDelayFire = true;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.1s;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.2s;

		SendWeaponAnim(ANIM_SHOOT2, UseDecrement() != FALSE);
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/airburster_shoot2.wav", VOL_NORM, 0.52, 0, 94 + RANDOM_LONG(0, 15));

#ifndef CLIENT_DLL
		Vector vecForward = Vector(gpGlobals->v_forward.x, gpGlobals->v_forward.y, 0.0).Normalize();

		TraceResult tr;
		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecStart = vecSrc - gpGlobals->v_up * 18;
		Vector vecEnd = vecStart + vecForward * 40;

		UTIL_TraceLine(vecStart, vecEnd, ignore_monsters, m_pPlayer->edict(), &tr);

		if (tr.flFraction < 1.0f)
		{
			CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);

			if (pHit && pHit->IsPlayer())
			{
				vecEnd = vecStart + (vecEnd - vecStart).Normalize() * tr.flFraction;
			}
		}

		CBaseEntity* pEntity = NULL;

		while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecEnd, 2.8 * 39.37f)) != NULL)
		{
			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			if (pEntity->pev->health <= 0.0 || pEntity == m_pPlayer)
				continue;

			if (pEntity->edict() != pev->owner && (g_pGameRules->PlayerRelationship(m_pPlayer, pEntity) == GR_TEAMMATE))
				continue;

			Vector rgflDamageDir = { 0.0, 0.0, 0.0 };
			float flCurDamage = UTIL_CalcDamage(vecEnd, pEntity->pev->origin, flDamage, 2.8 * 39.37f, rgflDamageDir);

			if (flCurDamage == 0.0f)
				continue;

			pEntity->m_LastHitGroup = HITGROUP_CHEST;
			pEntity->TakeDamage(pev, VARS(pev->owner), flCurDamage, DMG_EXPLOSION);

			if ((pEntity->IsPlayer() || pEntity->Classify() == CLASS_PLAYER_ALLY))
			{
				if (pEntity->IsPlayer())
				{
					CBasePlayer* pInflictorPlayer = static_cast<CBasePlayer*>(pEntity);

					if (pInflictorPlayer->m_bIsZombie && !(pInflictorPlayer->m_iBuff & BUFF_IGNORE_KNOCKBACK))
					{
						pInflictorPlayer->pev->velocity += rgflDamageDir * flKickRate;
						pInflictorPlayer->pev->velocity.z = max(pEntity->pev->velocity.z, 30);
					}
					
				}
				else
				{
					pEntity->pev->velocity += rgflDamageDir * flKickRate;
					pEntity->pev->velocity.z = max(pEntity->pev->velocity.z, 30);
				}
			}
		}

		MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, vecEnd);
		WRITE_BYTE(TE_EXPLOSION);
		WRITE_COORD(vecEnd.x);
		WRITE_COORD(vecEnd.y);
		WRITE_COORD(vecEnd.z);
		WRITE_SHORT(m_usModelIndexExplosion);
		WRITE_BYTE(int(2 - int(2.8 * 39.37f) * -0.3));
		WRITE_BYTE(45);
		WRITE_BYTE(TE_EXPLFLAG_NOSOUND);
		MESSAGE_END();
#endif
	}

	float GetDamageB() const
	{
		float flDamage = 40.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 140.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 320.0f;
#endif
		return flDamage;
	}

	float GetKickRate() const
	{
		float flRadius = 300.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flRadius = 1600.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flRadius = 1000.0f;
#endif
		return flRadius;
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
			SendWeaponAnim(ANIM_SHOOT1_END, UseDecrement() != FALSE);

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.06s;
		}

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
		SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);
	}

};
LINK_ENTITY_TO_CLASS(weapon_airburster, CAirBurster)
}