/*
heavyzg.cpp
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
    class CHeavyZG : public LinkWeaponTemplate< CHeavyZG,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_GALIL>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TReloadDefault,
            TPrimaryAttackRifle,
            TWeaponIdleDefault
    >
    {
    public:

    static constexpr const auto &DefaultReloadTime = 2.45s;
    static constexpr const char *V_Model = "models/v_heavyzg.mdl";
    static constexpr const char *P_Model = "models/p_heavyzg.mdl";
    static constexpr const char *W_Model = "models/w_heavyzg.mdl";
    static constexpr const char *EventFile = "events/heavyzg.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_heavyzg";
    static constexpr const char *AnimExtension = "rifle";
    static constexpr int MaxClip = 35;
	enum
	{
		ANIM_IDLE1 = 0,
        ANIM_IDLE2,
        ANIM_RELOAD,
        ANIM_DRAW,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
        ANIM_SHOOT3,	
	};
    static constexpr float PlayerMaxSpeed = 236;
    static constexpr float ArmorRatioModifier = 1.55f;
	static constexpr float PrimaryAttackWalkingMiniumSpeed = 140;
    static constexpr const auto &SpreadCalcNotOnGround = 0.4*A + 0.04;
    static constexpr const auto &SpreadCalcWalking = 0.04*A + 0.07;
    static constexpr const auto &SpreadCalcDefault = 0.03*A;

	static constexpr const auto& CycleTime = 0.0925s;
    static constexpr float AccuracyDefault = 0.2;

    KnockbackData KnockBack = { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f };


    private:
        duration_t m_flTimeWeaponIdle2;

    public:
    void Precache() override
    {
        Base::Precache();
        PRECACHE_SOUND("weapons/heavyzg_dryfire.wav");
    }

    void Spawn(void) override
    {
        Base::Spawn();
        m_flTimeWeaponIdle2 = 0s;
    }

    BOOL Deploy(void) override
    {
        m_flTimeWeaponIdle2 = 0s;
        return Base::Deploy();
    }

    int GetItemInfo(ItemInfo* p) override
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "556LVB";
        p->iMaxAmmo1 = 90;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = 35;
        p->iSlot = 0;
        p->iPosition = 17;
        p->iId = m_iId = WEAPON_GALIL;
        p->iFlags = 0;
        p->iWeight = GALIL_WEIGHT;

        return 1;
    }

    float GetDamage() const
    {
        float flDamage = 36;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = 37;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = 50;
#endif
        return flDamage;
    }

    void WeaponIdle(void) override
    {
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

        if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
            return;

        if (m_flTimeWeaponIdle2 < 10s)
        {
            m_flTimeWeaponIdle2 = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 4.8s;
            SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);
        }
        else
        {
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 8.4s;
            SendWeaponAnim(ANIM_IDLE2, UseDecrement() != FALSE);
            m_flTimeWeaponIdle2 = 0s;
        }
    }

    void StopIdleSound(void)
    {
#ifndef CLIENT_DLL
        int player = m_pPlayer->entindex();

        MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->edict());
        WRITE_BYTE(79);
        WRITE_SHORT(player);
        WRITE_SHORT(CHAN_AUTO);
        WRITE_STRING("weapons/heavyzg_idle2.wav");
        MESSAGE_END();

        MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->edict());
        WRITE_BYTE(79);
        WRITE_SHORT(player);
        WRITE_SHORT(CHAN_AUTO);
        WRITE_STRING("weapons/heavyzg_draw.wav");
        MESSAGE_END();

        MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->edict());
        WRITE_BYTE(79);
        WRITE_SHORT(player);
        WRITE_SHORT(CHAN_AUTO);
        WRITE_STRING("weapons/heavyzg_reload.wav");
        MESSAGE_END();
#endif
    }

    BOOL PlayEmptySound(void)
    {
        if (m_iPlayEmptySound)
            EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/heavyzg_dryfire.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);

        return FALSE;
    }

    void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_bDelayFire = true;
        m_iShotsFired++;
        m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 200) + 0.35;

        if (m_flAccuracy > 1.25)
            m_flAccuracy = 1.25;

        if (m_iClip <= 0)
        {
            if (m_fFireOnEmpty)
            {
                PlayEmptySound();
                m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
            }

#ifndef CLIENT_DLL
            if (TheBots != NULL)
            {
                TheBots->OnEvent(EVENT_WEAPON_FIRED_ON_EMPTY, m_pPlayer);
            }
#endif

            return;
        }

        m_iClip--;
        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
        Vector vecSrc = m_pPlayer->GetGunPosition();

        Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_556MM, GetDamage(), 0.98, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

        FireEvent(vecDir);

        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.9s;
        m_flTimeWeaponIdle2 = 0s;

        if (m_pPlayer->pev->velocity.Length2D() > 0)
            KickBack(1.3, 0.45, 0.325, 0.05, 4.5, 2.5, 7);
        else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            KickBack(1.5, 0.75, 0.35, 0.25, 7.0, 4.0, 5);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            KickBack(0.7, 0.325, 0.25, 0.015, 3.75, 2.5, 7);
        else
            KickBack(0.85, 0.375, 0.25, 0.02, 4.75, 2.0, 8);
    }

    void FireEvent(const Vector& vecDir)
    {
        int flags = 0;

#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif
        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, static_cast<int>(m_pPlayer->pev->punchangle.x * 100), static_cast<int>(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);
    }

    void Holster(int skiplocal) override
    {
        CBasePlayerWeapon::Holster(skiplocal);

        StopIdleSound();
    }

    void Reload(void)
    {
        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            return;

        if (DefaultReload(35, ANIM_RELOAD, 2.45s))
        {
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
            m_flAccuracy = 0.2;
            m_iShotsFired = 0;
            m_bDelayFire = false;
            m_flTimeWeaponIdle2 = 0s;
        }
    }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_556lvb" , 30 }; }
#endif
    		 	
};
LINK_ENTITY_TO_CLASS(weapon_heavyzg, CHeavyZG)
}