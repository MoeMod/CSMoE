/*
tmpdragon.cpp
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
    class CTMPDragon : public LinkWeaponTemplate< CTMPDragon,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_TMP>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TReloadDefault,
            TPrimaryAttackRifle,
            TFirePistol,
			TRecoilKickBack,
            TWeaponIdleDefault
    >
    {
    public:
    static constexpr const auto &DefaultReloadTime = 2.12s;
    static constexpr const char *V_Model = "models/v_tmpdragon.mdl";
    static constexpr const char *P_Model = "models/p_tmpdragon.mdl";
    static constexpr const char *W_Model = "models/w_tmpdragon.mdl";
    static constexpr const char *EventFile = "events/tmp.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_tmpdragon";
    static constexpr const char *AnimExtension = "onehanded";
    static constexpr int MaxClip = 30;
	enum
	{
		ANIM_IDLE1,
		ANIM_RELOAD,
		ANIM_DRAW,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_SHOOT3
	};
    static constexpr float PlayerMaxSpeed = 250;
    static constexpr const auto &SpreadCalcNotOnGround = 0.25*A;
    static constexpr const auto &SpreadCalcDefault = 0.03*A;

	static constexpr const auto& CycleTime = 0.07s;
    static constexpr float AccuracyDefault = 0.2;
    /*static constexpr*/ int Penetration = 1;
    KnockbackData KnockBack = { 250.0f, 200.0f, 250.0f, 90.0f, 0.7f };

    void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
		m_bDelayFire = true;
		m_iShotsFired++;
		m_flAccuracy = ((float)(m_iShotsFired * m_iShotsFired * m_iShotsFired) / 200) + 0.55;

		if (m_flAccuracy > 1.4)
			m_flAccuracy = 1.4;

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
		Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 1, BULLET_PLAYER_9MM, GetDamage(), 0.8, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), 5, FALSE);
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;

		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			KickBack(1.1, 0.5, 0.35, 0.045, 4.5, 3.5, 6);
		else if (m_pPlayer->pev->velocity.Length2D() > 0)
			KickBack(0.8, 0.4, 0.2, 0.03, 3, 2.5, 7);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			KickBack(0.7, 0.35, 0.125, 0.025, 2.5, 2, 10);
		else
			KickBack(0.725, 0.375, 0.15, 0.025, 2.75, 2.25, 9);
    }
    		 
	float GetDamage() const
	{
		float flDamage = 20.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
		{
			flDamage = RANDOM_LONG(0,20) ? 37.0f : 114514;
		}
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 37.0f;
#endif
		return flDamage;
	}
};
LINK_ENTITY_TO_CLASS(weapon_tmpdragon, CTMPDragon)
}