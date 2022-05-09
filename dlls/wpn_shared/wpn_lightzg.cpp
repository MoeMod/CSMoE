/*
lightzg.cpp
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
    class CLightZG : public LinkWeaponTemplate< CLightZG,
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

    static constexpr const auto &DefaultReloadTime = 3.1s;
    static constexpr const char *V_Model = "models/v_lightzg.mdl";
    static constexpr const char *P_Model = "models/p_lightzg.mdl";
    static constexpr const char *W_Model = "models/w_lightzg.mdl";
    static constexpr const char *EventFile = "events/lightzg.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_lightzg";
    static constexpr const char *AnimExtension = "rifle";
    static constexpr int MaxClip = 30;
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
    static constexpr float PlayerMaxSpeed = 238;
    static constexpr float ArmorRatioModifier = 1.55f;
	static constexpr float PrimaryAttackWalkingMiniumSpeed = 140;
    static constexpr const auto &SpreadCalcNotOnGround = 0.3*A + 0.03;
    static constexpr const auto &SpreadCalcWalking = 0.07*A + 0.03;
    static constexpr const auto &SpreadCalcDefault = 0.02*A;

	static constexpr const auto& CycleTime = 0.0885s;
    static constexpr float AccuracyDefault = 0.2;

    KnockbackData KnockBack = { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f };


    private:
        duration_t m_flTimeWeaponIdle2;

    public:
    void Precache() override
    {
        Base::Precache();
        PRECACHE_SOUND("weapons/lightzg_dryfire.wav");

        PRECACHE_MODEL("sprites/muzzleflash22.spr");
        PRECACHE_MODEL("sprites/muzzleflash23.spr");

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
        p->iMaxClip = 30;
        p->iSlot = 0;
        p->iPosition = 17;
        p->iId = m_iId = WEAPON_GALIL;
        p->iFlags = 0;
        p->iWeight = GALIL_WEIGHT;

        return 1;
    }

    float GetDamage() const
    {
        float flDamage = 33;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = 35;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = 46;
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
            m_flTimeWeaponIdle2 = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 10s;
            SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);
        }
        else
        {
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 6.9s;
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
        WRITE_STRING("weapons/lightzg_idle2.wav");
        MESSAGE_END();

        MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->edict());
        WRITE_BYTE(79);
        WRITE_SHORT(player);
        WRITE_SHORT(CHAN_AUTO);
        WRITE_STRING("weapons/lightzg_draw.wav");
        MESSAGE_END();

        MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->edict());
        WRITE_BYTE(79);
        WRITE_SHORT(player);
        WRITE_SHORT(CHAN_AUTO);
        WRITE_STRING("weapons/lightzg_reload.wav");
        MESSAGE_END();
#endif
    }

    BOOL PlayEmptySound(void)
    {
        if (m_iPlayEmptySound)
            EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/lightzg_dryfire.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);

        return FALSE;
    }

    void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_bDelayFire = true;
        m_iShotsFired++;
        m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 215) + 0.3;

        if (m_flAccuracy > 1.0)
            m_flAccuracy = 1.0;

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

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.03s;
        m_flTimeWeaponIdle2 = 0s;

        if (m_pPlayer->pev->velocity.Length2D() > 0)
            KickBack(1.0, 0.45, 0.275, 0.05, 4.0, 2.5, 7);
        else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            KickBack(1.25, 0.45, 0.22, 0.18, 5.5, 4.0, 5);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            KickBack(0.575, 0.325, 0.2, 0.011, 3.25, 2.0, 7);
        else
            KickBack(0.645, 0.35, 0.25, 0.015, 3.5, 2.25, 7);
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

        if (DefaultReload(30, ANIM_RELOAD, 3.1s))
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
LINK_ENTITY_TO_CLASS(weapon_lightzg, CLightZG)
}