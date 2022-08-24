/*
buffsg552.cpp
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
    class CBuffSG552 : public LinkWeaponTemplate< CBuffSG552,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_AUG>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TGetDamageDefault,
            TReloadDefault,
            TSecondaryAttackSniperZoom2,
            TWeaponIdleDefault
    >
    {
    public:
    static constexpr const auto& DefaultReloadTime = 2.03s;
    static constexpr int ZoomFOV1 = 40;
    static constexpr int ZoomFOV2 = 20;
    static constexpr const char *V_Model = "models/v_buffsg552.mdl";
    static constexpr const char *P_Model = "models/p_buffsg552.mdl";
    static constexpr const char *W_Model = "models/w_buffsg552.mdl";
    static constexpr const char *EventFile = "events/buffsg552.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_buffsg552";
    static constexpr const char *AnimExtension = "mp5";
    static constexpr int MaxClip = 50;
	enum
	{
        ANIM_IDLE1,
        ANIM_RELOAD,
        ANIM_DRAW,
        ANIM_SHOOT1,
        ANIM_SHOOT2,
        ANIM_SHOOT3
	};
    static constexpr float PlayerMaxSpeed = 235;
    static constexpr float ArmorRatioModifier = 1.6f;

    static constexpr int DamageDefault = 35;
    static constexpr int DamageZB = 120;
    static constexpr int DamageZBS = 200;
    static constexpr float AccuracyDefault = 0.19;
    static constexpr float AccuracyMax = 1.0;

    KnockbackData KnockBack = { 350.0f, 250.0f, 300.0f, 100.0f, 0.5f };

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "buffbullet";
        p->iMaxAmmo1 = 90;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = 50;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_BUFFSG552;
        p->iFlags = 0;
        p->iWeight = AUG_WEIGHT;

        return 1;
    }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_buffbullet" , 200 }; }
#endif
    float GetArmorRatioModifier() override { return 1.6; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 5, 3, 140 }; }

    float GetMaxSpeed(void)
    {
        if (m_pPlayer->m_iFOV == 90)
            return 235;

        return 175;
    }

    float GetDamageB()
    {
        float flDamage = 70;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = 300;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = 500;
#endif
        return flDamage;
    }

    void Precache()
    {
        PRECACHE_MODEL("sprites/muzzleflash51.spr");
        PRECACHE_MODEL("sprites/muzzleflash52.spr");
        PRECACHE_MODEL("sprites/muzzleflash53.spr");

        Base::Precache();
    }

    void PrimaryAttack() override
    {
        if (m_pPlayer->pev->fov != 90)
        {
            if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
                BuffSG552Fire(0.09 * (1 - m_flAccuracy), 0.25s, FALSE);
            else if (m_pPlayer->pev->velocity.Length2D() > 0)
                BuffSG552Fire(0.03, 0.25s, FALSE);
            else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
                BuffSG552Fire(0.01 * (1 - m_flAccuracy), 0.25s, FALSE);
            else
                BuffSG552Fire(0.02 * (1 - m_flAccuracy), 0.25s, FALSE);
        }
        else
        {
            if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
                BuffSG552Fire(0.45 * m_flAccuracy + 0.035, 0.09s, FALSE);
            else if (m_pPlayer->pev->velocity.Length2D() > 140)
                BuffSG552Fire(0.01 * m_flAccuracy + 0.035, 0.09s, FALSE);
            else
                BuffSG552Fire(0.02 * m_flAccuracy, 0.09s, FALSE);
        }
    }


    void BuffSG552Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_bDelayFire = true;
        m_iShotsFired++;

        if (m_pPlayer->m_iFOV != 90)
            m_flLastFire = gpGlobals->time;

        if (m_pPlayer->m_iFOV == 90)
            m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 210.0) + 0.3;
        else
            m_flAccuracy = ((gpGlobals->time - m_flLastFire) / 1s) * 0.5 + 1.0;

        if (m_flAccuracy > 1)
            m_flAccuracy = 1;

        m_flLastFire = gpGlobals->time;

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
        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_556MM, m_pPlayer->m_iFOV == 90 ? GetDamage() : GetDamageB(), 0.97, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

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

        if (m_pPlayer->m_iFOV == 90)
        {
            if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
                KickBack(1.1, 0.47, 0.295, 0.135, 5.2, 3.3, 8);
            else if (m_pPlayer->pev->velocity.Length2D() > 0)
                KickBack(0.65, 0.434, 0.265, 0.032, 3.5, 2.5, 8);
            else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
                KickBack(0.5, 0.24, 0.12, 0.01, 3.15, 2.0, 6);
            else
                KickBack(0.595, 0.32, 0.225, 0.0115, 3.25, 2.1, 7);
        }
        else
        {
            m_pPlayer->pev->punchangle.x -= (UTIL_SharedRandomFloat(m_pPlayer->random_seed + 4, 1.5, 3.0) + m_pPlayer->pev->punchangle.x * 0.25);
            m_pPlayer->pev->punchangle.y += UTIL_SharedRandomFloat(m_pPlayer->random_seed + 5, -0.8, 0.8);
        }
    }

    void StopIdleSound(void)
    {
#ifndef CLIENT_DLL
        MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
        WRITE_BYTE(79);
        WRITE_SHORT(m_pPlayer->entindex());
        WRITE_SHORT(CHAN_AUTO);
        WRITE_STRING("weapons/buffsg552_idle.wav");
        MESSAGE_END();
#endif
    }

    void Holster(int skiplocal)
    {
        StopIdleSound();

        CBasePlayerWeapon::Holster(skiplocal);
    }

};
LINK_ENTITY_TO_CLASS(weapon_buffsg552, CBuffSG552)
}