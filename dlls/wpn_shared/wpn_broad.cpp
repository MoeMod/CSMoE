/*
broad.cpp
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
    class CBroad : public LinkWeaponTemplate< CBroad,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_M249>::template type,
            TPrecacheEvent,
            TReloadDefault,
            TGetDamageDefault,
            TPrimaryAttackRifle,
            TFireRifle,
            TWeaponIdleDefault
    >
    {
    public:

    static constexpr const auto &DefaultReloadTime = 4.7s;
    static constexpr const char *V_Model = "models/v_broad.mdl";
    static constexpr const char *P_Model = "models/p_broad.mdl";
    static constexpr const char *W_Model = "models/w_broad.mdl";
    static constexpr const char *EventFile = "events/broad.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_broad";
    static constexpr const char *AnimExtension = "m249";
    static constexpr int MaxClip = 200;

#define WPNSTATE_BROAD_CHARGE (1<<1)

	enum
	{
		ANIM_IDLE1 = 0,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_RELOAD,
		ANIM_DRAW,
	};
    static constexpr float PlayerMaxSpeed = 220;
    static constexpr float ArmorRatioModifier = 1.6f;
	static constexpr float PrimaryAttackWalkingMiniumSpeed = 195;
    static constexpr const auto &SpreadCalcNotOnGround = 0.05*A + 0.05;
    static constexpr const auto &SpreadCalcWalking = 0.042*A + 0.03;
    static constexpr const auto &SpreadCalcDefault = 0.03*A + 0.025;
	static constexpr const auto& SpreadCalcDucking = A*0 + 0.021;

	static constexpr const auto& CycleTime = 0.12s;
    static constexpr int DamageDefault = 38;
    static constexpr int DamageZB = 55;
    static constexpr int DamageZBS = 80;

    static constexpr float AccuracyDefault = 0.24;
    static constexpr float AccuracyMax = 0.9;
    static constexpr float RangeModifier = 0.94;
    static constexpr auto BulletType = BULLET_PLAYER_762MM;
    /*static constexpr*/ int Penetration = 1;
    KnockbackData KnockBack = { 370.0f, 270.0f, 320.0f, 120.0f, 0.6f };

	int m_iChargeShotsFired;
	int m_iTargetShotsFired;
	int m_iExpIndex;
	int m_iExpIndex2;

	void Precache(void)
	{
		m_iExpIndex = PRECACHE_MODEL("sprites/ef_broad.spr");
		m_iExpIndex2 = PRECACHE_MODEL("sprites/ef_broad2.spr");
		Base::Precache();
	}

    int GetItemInfo(ItemInfo* p) override
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "762NatoBox";
        p->iMaxAmmo1 = 200;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = 200;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_BROAD;
        p->iFlags = 0;
        p->iWeight = M249_WEIGHT;

        return 1;
    }

	BOOL Deploy()
	{
		m_iWeaponState &= ~WPNSTATE_BROAD_CHARGE;
		m_flAccuracy = 0.24;
		m_iShotsFired = 0;

		m_iChargeShotsFired = 0;
		m_iTargetShotsFired = RANDOM_LONG(7, 11);

		return DefaultDeploy("models/v_broad.mdl", "models/p_broad.mdl", ANIM_DRAW, "m249", UseDecrement() != FALSE);
	}

    void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
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
		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
		m_pPlayer->m_iWeaponVolume = BRIGHT_GUN_FLASH;

		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecDir;

#ifndef CLIENT_DLL
		if (m_iWeaponState & WPNSTATE_BROAD_CHARGE)
		{
			float flDamage = GetDamageB();
			float flRadius = GetRadius();

			vecDir = BroadExplosion(vecSrc, gpGlobals->v_forward, 0.0, 8192, flDamage, flRadius, m_pPlayer->pev, m_pPlayer->random_seed);
		}
		else
#endif
		{
			float flDamage = GetDamage();

			vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_762MM, flDamage, 0.97, m_pPlayer->pev, false, m_pPlayer->random_seed);
					
			Vector vecDirction = gpGlobals->v_forward + vecDir[0] * gpGlobals->v_right + vecDir[1] * gpGlobals->v_up;
#ifndef CLIENT_DLL
			BuffToLineTargets(vecSrc, vecDirction);
#endif
		}

	
#ifndef CLIENT_DLL
		m_iChargeShotsFired++;

		if (m_iChargeShotsFired == m_iTargetShotsFired)
		{
			m_iWeaponState |= WPNSTATE_BROAD_CHARGE;

			m_iChargeShotsFired = 0;
			m_iTargetShotsFired = RANDOM_LONG(7, 11);
		}
		else
		{
			m_iWeaponState &= ~WPNSTATE_BROAD_CHARGE;
		}
#endif

		int flags;

#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

#ifdef CLIENT_DLL
		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0.0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, int(m_pPlayer->pev->punchangle.x * 100), int(m_pPlayer->pev->punchangle.y * 100), (m_iWeaponState & WPNSTATE_BROAD_CHARGE) ? TRUE : FALSE, FALSE);
#else
		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0.0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, int(m_pPlayer->pev->punchangle.x * 100), int(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);
#endif

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", 0, 0);
#endif
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.6s;

		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			KickBack(1.5, 0.55, 0.3, 0.3, 6.0, 5.0, 8);
		else if (m_pPlayer->pev->velocity.Length2D() > 0)
			KickBack(1.2, 0.35, 0.25, 0.06, 4.0, 2.5, 8);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			KickBack(0.77, 0.12, 0.12, 0.019, 3.0, 1.2, 9);
		else
			KickBack(0.81, 0.23, 0.2, 0.023, 3.2, 2.2, 9);
    }

#ifndef CLIENT_DLL
	void BuffToLineTargets(Vector vecSrc, Vector vecDir)
	{
		TraceResult tr;
		Vector vecEnd = vecSrc + vecDir * 8192.0;
		UTIL_TraceLine(vecSrc, vecSrc + vecDir * 8192.0, dont_ignore_monsters, m_pPlayer->edict(), &tr);

		if (tr.flFraction < 1.0 && !FNullEnt(tr.pHit))
		{
			CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

			if (pEntity && (pEntity->IsPlayer() || pEntity->Classify() == CLASS_PLAYER_ALLY))
			{
				if (pEntity->Classify() == CLASS_PLAYER_ALLY)
				{
					MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL, pEntity->pev);
					WRITE_BYTE(4);
					WRITE_SHORT(pEntity->entindex());
					WRITE_BYTE(8);
					MESSAGE_END();

					pEntity->pBuffAttacker = m_pPlayer->pev;
					pEntity->m_iHealthDecreaseCount = 5;
					pEntity->m_flHealthDecreaseAmount = 20;
				}
				else
				{
					CBasePlayer* pInflictorPlayer = static_cast<CBasePlayer*>(pEntity);

					if (pInflictorPlayer->m_bIsZombie && !pInflictorPlayer->m_iHealthDecreaseCount) // fire burn
					{
						MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL, pEntity->pev);
						WRITE_BYTE(4);
						WRITE_SHORT(pEntity->entindex());
						WRITE_BYTE(8);
						MESSAGE_END();

						pEntity->pBuffAttacker = m_pPlayer->pev;
						pEntity->m_iHealthDecreaseCount = 5;
						pEntity->m_flHealthDecreaseAmount = 20;
					}
				}
			}
		}
	}

	Vector BroadExplosion(Vector vecSrc, Vector vecDirShooting, float flSpread, float flDistance, float flDamage, float flRadius, entvars_t* pevAttacker, int random_seed)
	{
		ClearMultiDamage();
		gMultiDamage.type = DMG_BULLET | DMG_NEVERGIB;

		entvars_t* pevAttack = pevAttacker;

		if (pevAttacker == NULL)
			pevAttack = pev;

		Vector vecRight = gpGlobals->v_right;
		Vector vecUp = gpGlobals->v_up;

		float x, y, z;

		if (IsPlayer())
		{
			x = UTIL_SharedRandomFloat(random_seed, -0.5, 0.5) + UTIL_SharedRandomFloat(random_seed + 1, -0.5, 0.5);
			y = UTIL_SharedRandomFloat(random_seed + 2, -0.5, 0.5) + UTIL_SharedRandomFloat(random_seed + 3, -0.5, 0.5);
		}
		else
		{
			do
			{
				x = RANDOM_FLOAT(-0.5, 0.5) + RANDOM_FLOAT(-0.5, 0.5);
				y = RANDOM_FLOAT(-0.5, 0.5) + RANDOM_FLOAT(-0.5, 0.5);
				z = x * x + y * y;
			} while (z > 1);
		}

		TraceResult tr;
		Vector vecDir = vecDirShooting + x * flSpread * vecRight + y * flSpread * vecUp;
		Vector vecEnd = vecSrc + vecDir * flDistance;

		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pev), &tr);

		MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, tr.vecEndPos);
		WRITE_BYTE(TE_EXPLOSION);
		WRITE_COORD(tr.vecEndPos.x);
		WRITE_COORD(tr.vecEndPos.y);
		WRITE_COORD(tr.vecEndPos.z);
		WRITE_SHORT(m_iExpIndex2);
		WRITE_BYTE(2 - int(flRadius * -0.1));
		WRITE_BYTE(30);
		WRITE_BYTE(TE_EXPLFLAG_NONE);
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_TEMPSPRITE);
		WRITE_COORD(tr.vecEndPos.x);
		WRITE_COORD(tr.vecEndPos.y);
		WRITE_COORD(tr.vecEndPos.z + 10.0);
		WRITE_SHORT(m_iExpIndex);
		WRITE_BYTE(2 - int(flRadius * -0.1));
		WRITE_BYTE(255);
		WRITE_BYTE(30);
		MESSAGE_END();

		
		bool bInWater = UTIL_PointContents(tr.vecEndPos) == CONTENTS_WATER;
		CBaseEntity* pEntity = NULL;

		while ((pEntity = UTIL_FindEntityInSphere(pEntity, tr.vecEndPos, flRadius)) != NULL)
		{
			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			if (bInWater && pEntity->pev->waterlevel == 0)
				continue;

			if (!bInWater && pEntity->pev->waterlevel == 3)
				continue;

			if (pEntity->pev == pevAttack)
				continue;

			pEntity->TakeDamage(pevAttack, pevAttack, UTIL_CalcDamage(tr.vecEndPos, pEntity->pev->origin, flDamage, flRadius, NULL), DMG_EXPLOSION);

			if ((pEntity->IsPlayer() || pEntity->Classify() == CLASS_PLAYER_ALLY))
			{
				if (pEntity->Classify() == CLASS_PLAYER_ALLY)
				{
					MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL, pEntity->pev);
					WRITE_BYTE(4);
					WRITE_SHORT(pEntity->entindex());
					WRITE_BYTE(8);
					MESSAGE_END();

					pEntity->pBuffAttacker = m_pPlayer->pev;
					pEntity->m_iHealthDecreaseCount = 5;
					pEntity->m_flHealthDecreaseAmount = 20;
				}
				else
				{
					CBasePlayer* pInflictorPlayer = static_cast<CBasePlayer*>(pEntity);

					if (pInflictorPlayer->m_bIsZombie && !pInflictorPlayer->m_iHealthDecreaseCount) // fire burn
					{
						MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL, pEntity->pev);
						WRITE_BYTE(4);
						WRITE_SHORT(pEntity->entindex());
						WRITE_BYTE(8);
						MESSAGE_END();

						pEntity->pBuffAttacker = m_pPlayer->pev;
						pEntity->m_iHealthDecreaseCount = 5;
						pEntity->m_flHealthDecreaseAmount = 20;
					}
				}
			}
		
		}

		return tr.vecEndPos;
	}

	float GetDamageB() const
	{
		float flDamage = 90.0f;
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 350.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 450.0f;
		return flDamage;
	}

	float GetRadius() const
	{
		float flRadius = 80.0f;
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flRadius = 110.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flRadius = 130.0f;
		return flRadius;
	}

#endif

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_762natobox" , 60 }; }
#endif
    float GetArmorRatioModifier() override { return 1.5; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 6, 3, 140 }; }
    		 	
};
LINK_ENTITY_TO_CLASS(weapon_broad, CBroad)
}