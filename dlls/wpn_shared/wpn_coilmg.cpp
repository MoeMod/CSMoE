/*
coilmg.cpp
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
    class CCoilMG : public LinkWeaponTemplate< CCoilMG,
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

    static constexpr const auto &DefaultReloadTime = 4.0s;
    static constexpr const char *V_Model = "models/v_coilmg.mdl";
    static constexpr const char *P_Model = "models/p_coilmg.mdl";
    static constexpr const char *W_Model = "models/w_coilmg.mdl";
    static constexpr const char *EventFile = "events/coilmg.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_coilmg";
    static constexpr const char *AnimExtension = "m249";
    static constexpr int MaxClip = 100;

	enum
	{
		ANIM_IDLE1 = 0,
		ANIM_RELOAD,
		ANIM_DRAW,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_SHOOT3,
	};
    static constexpr float PlayerMaxSpeed = 210;
    static constexpr float ArmorRatioModifier = 1.5f;
	static constexpr float PrimaryAttackWalkingMiniumSpeed = 140;
    static constexpr const auto &SpreadCalcNotOnGround = 0.3*A + 0.04;
    static constexpr const auto &SpreadCalcWalking = 0.03*A + 0.035;
    static constexpr const auto &SpreadCalcDefault = 0.06*A;
	static constexpr const auto& SpreadCalcDucking = A*0.02 + 0.01;

	static constexpr const auto& CycleTime = 0.0955s;
    static constexpr int DamageDefault = 36;
    static constexpr int DamageZB = 52;
    static constexpr int DamageZBS = 55;

    static constexpr float AccuracyDefault = 0.22;
    static constexpr float AccuracyMax = 0.9;
    static constexpr float RangeModifier = 0.94;
    static constexpr auto BulletType = BULLET_PLAYER_762MM;
    /*static constexpr*/ int Penetration = 1;
    KnockbackData KnockBack = { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f };

	int m_iExpIndex;

	void Precache(void)
	{
		PRECACHE_SOUND("weapons/coilmg_exp1.wav");
		m_iExpIndex = PRECACHE_MODEL("sprites/ef_coilmg.spr");
		PRECACHE_MODEL("sprites/muzzleflash19.spr");
		Base::Precache();
	}

    int GetItemInfo(ItemInfo* p) override
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "coil";
        p->iMaxAmmo1 = 200;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = 100;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_COILMG;
        p->iFlags = 0;
        p->iWeight = M249_WEIGHT;

        return 1;
    }

	BOOL Deploy()
	{
		m_flAccuracy = 0.22;
		m_iShotsFired = 0;

		return DefaultDeploy("models/v_coilmg.mdl", "models/p_coilmg.mdl", ANIM_DRAW, "m249", UseDecrement() != FALSE);
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

		Vector vecUp, vecRight, vecForward;
		vecForward = gpGlobals->v_forward;
		vecUp = gpGlobals->v_up;
		vecRight = gpGlobals->v_right;

		float flDamage = GetDamage();

		vecDir = m_pPlayer->FireBullets3(vecSrc, vecForward, flSpread, 8192, 2, BULLET_PLAYER_762MM, flDamage, 0.98, m_pPlayer->pev, false, m_pPlayer->random_seed);
#ifndef CLIENT_DLL
		BuffToLineTargets(vecSrc, vecForward, vecRight, vecUp, vecDir, 8192, m_pPlayer);
#endif

		int flags;

#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0.0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, int(m_pPlayer->pev->punchangle.x * 100), int(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", 0, 0);
#endif
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.6s;

		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			KickBack(1.5, 0.55, 0.3, 0.3, 6, 5, 5);
		else if (m_pPlayer->pev->velocity.Length2D() > 0)
			KickBack(1.1, 0.3, 0.2, 0.06, 4.0, 2.5, 8);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			KickBack(0.6, 0.1, 0.1, 0.018, 3, 1.2, 9);
		else
			KickBack(0.7, 0.2, 0.15, 0.02, 3.2, 2.25, 8);
    }

#ifndef CLIENT_DLL
	int BuffToLineTargets(Vector vecSrc, Vector vecDirShooting, Vector vecRight, Vector vecUp, Vector vecSpread, float flDistance, CBasePlayer* pPlayer)
	{
		int HitResult = HIT_NONE;

		TraceResult tr;

		Vector vecDir, vecEnd;

		vecDir = vecDirShooting + vecSpread[0] * vecRight + vecSpread[1] * vecUp;
		vecEnd = vecSrc + vecDir * flDistance;

		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pPlayer->pev), &tr);

		if (tr.flFraction < 1.0)
		{
			HitResult = HIT_WALL;
			CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

			if (pEntity && (pEntity->IsPlayer() || pEntity->Classify() == CLASS_PLAYER_ALLY))
			{

				HitResult = HIT_PLAYER;
		
				if (pEntity->IsPlayer())
				{
					if (g_pGameRules->PlayerRelationship(pPlayer, pEntity) != GR_TEAMMATE)
					{
						MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
						WRITE_BYTE(TE_TEMPSPRITE);
						WRITE_COORD(tr.vecEndPos.x);
						WRITE_COORD(tr.vecEndPos.y);
						WRITE_COORD(tr.vecEndPos.z);
						WRITE_SHORT(m_iExpIndex);
						WRITE_BYTE(10);
						WRITE_BYTE(255);
						WRITE_BYTE(8);
						MESSAGE_END();

						EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/coilmg_exp1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
					}
				}
				else
				{
					MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
					WRITE_BYTE(TE_TEMPSPRITE);
					WRITE_COORD(tr.vecEndPos.x);
					WRITE_COORD(tr.vecEndPos.y);
					WRITE_COORD(tr.vecEndPos.z);
					WRITE_SHORT(m_iExpIndex);
					WRITE_BYTE(10);
					WRITE_BYTE(255);
					WRITE_BYTE(8);
					MESSAGE_END();

					EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/coilmg_exp1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

				}

				bool bInWater = UTIL_PointContents(tr.vecEndPos) == CONTENTS_WATER;
				pEntity = NULL;

				while ((pEntity = UTIL_FindEntityInSphere(pEntity, tr.vecEndPos, 90.0)) != NULL)
				{
					if (pEntity->pev->takedamage == DAMAGE_NO)
						continue;

					if (bInWater && pEntity->pev->waterlevel == 0)
						continue;

					if (!bInWater && pEntity->pev->waterlevel == 3)
						continue;

					if (pEntity->pev == m_pPlayer->pev)
						continue;

					pEntity->TakeDamage(m_pPlayer->pev, m_pPlayer->pev, UTIL_CalcDamage(tr.vecEndPos, pEntity->pev->origin, GetDamage(), 90, NULL), DMG_EXPLOSION);
				}

			}
		}
		return HitResult;
	}
#endif

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_coil" , 200 }; }
#endif
    float GetArmorRatioModifier() override { return 1.5; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 6, 3, 140 }; }
    		 	
};
LINK_ENTITY_TO_CLASS(weapon_coilmg, CCoilMG)
}