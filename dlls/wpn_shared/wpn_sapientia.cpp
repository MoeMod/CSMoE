/*
sapientia.cpp
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
    class CSapientia : public LinkWeaponTemplate< CSapientia,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_DEAGLE>::template type,
            TPrecacheEvent,
            TDeployDefault,
			TReloadDefault,
            TPrimaryAttackRifle,
			TWeaponIdleDefault,
            TFirePistol,
			TGetDamageDefault
    >
    {
    public:
	static constexpr const auto& DefaultReloadTime = 2.3s;
    static constexpr const char *V_Model = "models/v_sapientia.mdl";
    static constexpr const char *P_Model = "models/p_sapientia.mdl";
    static constexpr const char *W_Model = "models/w_sapientia.mdl";
    static constexpr const char *EventFile = "events/sapientia.sc";
    static constexpr InventorySlotType ItemSlot = PISTOL_SLOT;
    static constexpr const char *ClassName = "weapon_sapientia";
    static constexpr const char *AnimExtension = "onehanded";
    static constexpr int MaxClip = 7;

	enum
	{
		ANIM_IDLE1 = 0,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_SHOOT_EMPTY,
		ANIM_RELOAD,
		ANIM_DRAW,
	};
    static constexpr float PlayerMaxSpeed = 250;
    static constexpr float ArmorRatioModifier = 1.5f;
    static constexpr const auto &SpreadCalcNotOnGround = 1.4*(1-A);
    static constexpr const auto &SpreadCalcWalking = 0.25*(1-A);
    static constexpr const auto& SpreadCalcDucking = 0.1*(1-A);
    static constexpr const auto &SpreadCalcDefault = 0.125*(1-A);

	static constexpr const auto& CycleTime = 0.2s;
    static constexpr int DamageDefault = 48;
    static constexpr int DamageZB = 76;
    static constexpr int DamageZBS = 150;

    static constexpr float AccuracyDefault = 0.9;
    /*static constexpr*/ int Penetration = 1;
    KnockbackData KnockBack = { 400.0f, 300.0f, 350.0f, 150.0f, 0.6f };

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_50aeholy" , 40 }; }
#endif
	float GetArmorRatioModifier() override { return 1.5; }
	WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED | ACCURACY_DUCK, 8, 3, 0 }; }

	int m_iExpIndex;

	int GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "50aeholy";
		p->iMaxAmmo1 = 35;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = 7;
		p->iSlot = 1;
		p->iPosition = 2;
		p->iId = m_iId = WEAPON_SAPIENTIA;
		p->iFlags = 0;
		p->iWeight = DEAGLE_WEIGHT;

		return 1;
	}

	void Precache(void)
	{
		m_iExpIndex = PRECACHE_MODEL("sprites/holybomb_exp.spr");
		PRECACHE_SOUND("weapons/holywater_explosion.wav");

		Base::Precache();
	}

    void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
		m_iShotsFired++;

		if (m_iShotsFired > 1)
			return;

		if (m_flLastFire != invalid_time_point)
		{
			m_flAccuracy -= (0.1 - ((gpGlobals->time - m_flLastFire) / 1s)) * 0.3;

			if (m_flAccuracy > 0.9)
				m_flAccuracy = 0.9;
			else if (m_flAccuracy < 0.5)
				m_flAccuracy = 0.5;
		}

		m_flLastFire = gpGlobals->time;

		if (m_iClip <= 0)
		{
			if (m_fFireOnEmpty)
			{
				PlayEmptySound();
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
			}
			return;
		}

		m_iClip--;
		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;

		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecDir;

		Vector vecUp, vecRight, vecForward;
		vecForward = gpGlobals->v_forward;
		vecUp = gpGlobals->v_up;
		vecRight = gpGlobals->v_right;

		vecDir = m_pPlayer->FireBullets3(vecSrc, vecForward, flSpread, 4096, 2, BULLET_PLAYER_50AE, GetDamage(), 0.8, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);
#ifndef CLIENT_DLL
		BuffToLineTargets(vecSrc, vecForward, vecRight, vecUp, vecDir, 8192, m_pPlayer);
#endif
		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif
		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0.0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, int(m_pPlayer->pev->punchangle.x * 100), int(m_pPlayer->pev->punchangle.y * 100), m_iClip != 0, FALSE);
		
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
		m_pPlayer->pev->punchangle.x -= 2;
    }

	float GetBuffDamage() const
	{
		float flDamage = 1.0;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 30.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 85.0f;
#endif
		return flDamage;
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

				EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/holywater_explosion.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

				MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
				WRITE_BYTE(TE_TEMPSPRITE);
				WRITE_COORD(tr.vecEndPos.x);
				WRITE_COORD(tr.vecEndPos.y);
				WRITE_COORD(tr.vecEndPos.z + 10.0);
				WRITE_SHORT(m_iExpIndex);
				WRITE_BYTE(4);
				WRITE_BYTE(255);
				WRITE_BYTE(30);
				MESSAGE_END();

				if (pEntity->IsPlayer())
				{
					CBasePlayer* pInflictorPlayer = static_cast<CBasePlayer*>(pEntity);

					if (pInflictorPlayer->m_bIsZombie && !pInflictorPlayer->m_iHealthDecreaseCount)
					{
						pInflictorPlayer->m_pBuffAttacker = m_pPlayer->pev;
						pInflictorPlayer->m_iHealthDecreaseCount = 5;
						pInflictorPlayer->m_flHealthDecreaseAmount = 20;
						pInflictorPlayer->m_flHealthDecreaseInterval = 1.0s;
						pInflictorPlayer->m_flHealthDecreaseStartTime = gpGlobals->time + 1.0s;

						MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL, pInflictorPlayer->pev);
						WRITE_BYTE(4);
						WRITE_SHORT(pInflictorPlayer->entindex());
						WRITE_BYTE(8);
						MESSAGE_END();

					}
				}
				else
				{
					pEntity->m_pBuffAttacker = m_pPlayer->pev;
					pEntity->m_iHealthDecreaseCount = 5;
					pEntity->m_flHealthDecreaseAmount = 20;
					pEntity->m_flHealthDecreaseInterval = 1.0s;
					pEntity->m_flHealthDecreaseStartTime = gpGlobals->time + 1.0s;

					MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL, pEntity->pev);
					WRITE_BYTE(4);
					WRITE_SHORT(pEntity->entindex());
					WRITE_BYTE(8);
					MESSAGE_END();

				}
			}
		}
		return HitResult;
	}
#endif
};
LINK_ENTITY_TO_CLASS(weapon_sapientia, CSapientia)
}