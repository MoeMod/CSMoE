/*
lightningsmg.cpp
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
    class CLightningSMG : public LinkWeaponTemplate< CLightningSMG,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_P90>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TReloadDefault,
            TGetDamageDefault,
			TPrimaryAttackRifle
    >
    {
    public:
	static constexpr const auto& DefaultDeployTime = 0.52s;
    static constexpr const auto &DefaultReloadTime = 5.0s;
    static constexpr const char *V_Model = "models/v_lightningsmg.mdl";
    static constexpr const char *P_Model = "models/p_lightningsmg_a.mdl";
    static constexpr const char *W_Model = "models/w_lightningsmg.mdl";
    static constexpr const char *EventFile = "events/lightningsmg.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_lightningsmg";
    static constexpr const char *AnimExtension = "mp5";
    static constexpr int MaxClip = 80;
	enum
	{
		ANIM_IDLE1 = 0,
		ANIM_RELOAD,
		ANIM_DRAW,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_SHOOT3,
		ANIM_CHARGE1,
		ANIM_CHARGE_FINISH,
		ANIM_CHARGE2,
		ANIM_CHARGE_SHOOT,
	};

    static constexpr const auto& SpreadCalcNotOnGround = 0.2 * A;
    static constexpr const auto& SpreadCalcDefault = 0.04 * A;

    static constexpr const auto& CycleTime = 0.07s;

    static constexpr float PlayerMaxSpeed = 255;
    static constexpr float ArmorRatioModifier = 1.2f;

    static constexpr int DamageDefault = 26;
    static constexpr int DamageZB = 180;
    static constexpr int DamageZBS = 350;

    static constexpr float AccuracyDefault = 0.0;
    static constexpr float AccuracyMax = 0.7;

    KnockbackData KnockBack = { 250.0f, 200.0f, 250.0f, 90.0f, 0.7f };


    time_point_t m_flTimeCheckCharge;
    int m_iChargeState;
    int m_iDefaultAmmo2;
    int m_iChargeShotsFired;
    int m_iTargetShotsFired;

	void Spawn()
	{
        m_iDefaultAmmo2 = 0;
        m_iChargeState = 0;
        m_flTimeCheckCharge = invalid_time_point;
        m_iChargeShotsFired = 0;
        m_iTargetShotsFired = 35;
        Base::Spawn();
	}

    void Precache(void)
    {
        Base::Precache();

		PRECACHE_MODEL("models/ef_lightningsmg.mdl");
		PRECACHE_MODEL("sprites/muzzleflash215.spr");
    }


    int GetItemInfo(ItemInfo* p) override
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "lightningsmg";
        p->iMaxAmmo1 = 90;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = 80;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_LIGHTNINGSMG;
        p->iFlags = 0;
        p->iWeight = M249_WEIGHT;
        return 1;
    }

    bool HasSecondaryAttack() override { return true; }
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_lightningsmg" , 80 }; }
#endif
    float GetArmorRatioModifier() override { return 0.9; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 6, 3, 0 }; }

	int ExtractAmmo(CBasePlayerWeapon* pWeapon) override
	{
		if (m_iDefaultAmmo2)
		{
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iDefaultAmmo2;
			m_iDefaultAmmo2 = 0;
		}
		return CBasePlayerWeapon::ExtractAmmo(pWeapon);
	}

    BOOL Deploy() override
    {
        m_flAccuracy = AccuracyDefault;
        m_bDelayFire = false;

        int iAnim = ANIM_DRAW + m_iWeaponState;

        return DefaultDeploy(V_Model, P_Model, iAnim, "carbine", UseDecrement() != FALSE);
    }

    void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_bDelayFire = true;
        m_iShotsFired++;

        m_flAccuracy = ((m_iShotsFired * m_iShotsFired) / 230.0) + 0.3;
        if (m_flAccuracy > AccuracyMax)
            m_flAccuracy = AccuracyMax;

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
        m_iChargeShotsFired++;

        if (m_iChargeShotsFired > m_iTargetShotsFired && m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] < 3)
        {
            m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]++;

#ifndef CLIENT_DLL
            MESSAGE_BEGIN(MSG_ONE, gmsgAmmoPickup, NULL, m_pPlayer->pev);
            WRITE_BYTE(CBasePlayer::GetAmmoIndex("lightningsmg2"));
            WRITE_BYTE(1);
            MESSAGE_END();
#endif
            m_iChargeShotsFired = 0;
        }

        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;

        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 1, BULLET_PLAYER_45ACP, GetDamage(), 0.88, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

        int flags;
#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5s;

        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            KickBack(1.0, 0.55, 0.4, 0.05, 5, 3.25, 9);
        else if (m_pPlayer->pev->velocity.Length2D() > 0)
            KickBack(0.6, 0.3, 0.25, 0.035, 3.5, 2.75, 10);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            KickBack(0.25, 0.175, 0.125, 0.02, 2.25, 1.2, 10);
        else
            KickBack(0.275, 0.2, 0.15, 0.02, 3, 1.75, 9);
    }

    void OnAttack2Released()
    {
        if (m_iChargeState > 1)
        {
#ifndef CLIENT_DLL
            UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

            Vector vecSrc = m_pPlayer->GetGunPosition();
            Vector vecDir = m_pPlayer->FireBullets3(vecSrc - gpGlobals->v_up * 2.0, gpGlobals->v_forward, 0.0, 8192.0, 0, BULLET_PLAYER_556MM, 0, 0.0, m_pPlayer->pev, false, m_pPlayer->random_seed);

            Vector vecProjDir = gpGlobals->v_forward + gpGlobals->v_right * vecDir.x + gpGlobals->v_up * vecDir.y;    
#endif
        }
        else
        {
#ifndef CLIENT_DLL
            UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

            Vector vecSrc = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 50.0;;

          
#endif

        }

        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

        m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;

        SendWeaponAnim(ANIM_CHARGE_SHOOT);

        UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/lightningsmg-2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM, 1);

        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.7s;
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;

        if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 0)
            m_pPlayer->pev->weaponmodel = MAKE_STRING(P_Model);

        m_iChargeState = 0;
    }

    void SecondaryAttack(void)
    {
        if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] <= 0)
            return;

        if (m_iChargeState)
        {
            if (gpGlobals->time > m_flTimeCheckCharge)
            {
                switch (m_iChargeState)
                {
                case 1:
                {
                    SendWeaponAnim(ANIM_CHARGE_FINISH);

                    m_iChargeState = 2;
                    m_flTimeCheckCharge = gpGlobals->time + 0.53s;
                    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0s;
                    break;
                }
                case 2:
                {
                    SendWeaponAnim(ANIM_CHARGE2);

                    UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/lightningsmg_idle3.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM, 1);

                    m_iChargeState = 3;
                    m_flTimeCheckCharge = gpGlobals->time + 2.0s;
                    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 9999.0s;
                    break;
                }
                case 3:
                {
                    UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/lightningsmg_idle3.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM, 1);

                    m_flTimeCheckCharge = gpGlobals->time + 2.0s;
                }

                default:
                    break;
                }
            }
        }
        else
        {
            SendWeaponAnim(ANIM_CHARGE1);

            UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/lightningsmg_idle2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM, 1);

            m_flTimeCheckCharge = gpGlobals->time + 0.5s;
            m_iChargeState = 1;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
        }
    }

    void WeaponIdle() override
    {
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

        if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
            return;

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
        SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);
    }

    void Reload() override
    {
        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            return;

        int iAnim = m_iWeaponState + ANIM_RELOAD;

        if (DefaultReload(MaxClip, iAnim, DefaultReloadTime))
        {
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
            m_flAccuracy = 0.0;
        }
    }

    		 	
};
LINK_ENTITY_TO_CLASS(weapon_lightningsmg, CLightningSMG)
}