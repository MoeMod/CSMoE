/*
buffaug.cpp
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
    class CBuffAug : public LinkWeaponTemplate< CBuffAug,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_AUG>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TGetDamageDefault,
            TReloadDefault,
            TSecondaryAttackZoom,
            TFireRifle,
            TWeaponIdleDefault
    >
    {
    public:
    static constexpr const auto& DefaultReloadTime = 1.7s;
    static constexpr int ZoomFOV = 55;
    static constexpr const char *V_Model = "models/v_buffaug.mdl";
    static constexpr const char *P_Model = "models/p_buffaug.mdl";
    static constexpr const char *W_Model = "models/w_buffaug.mdl";
    static constexpr const char *EventFile = "events/buffaug.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_buffaug";
    static constexpr const char *AnimExtension = "rifle";
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
    static constexpr float PlayerMaxSpeed = 240;
    static constexpr float ArmorRatioModifier = 1.6f;

    static constexpr int DamageDefault = 38;
    static constexpr int DamageZB = 150;
    static constexpr int DamageZBS = 330;
    static constexpr const auto &AccuracyCalc = (N * N * N / 220.0) + 0.3;
    static constexpr float AccuracyDefault = 0.15;
    static constexpr float AccuracyMax = 1.0;
    static constexpr float RangeModifier = 0.97;
    static constexpr auto BulletType = BULLET_PLAYER_556MM;
    /*static constexpr*/ int Penetration = 1;
    KnockbackData KnockBack = { 350.0f, 250.0f, 300.0f, 100.0f, 0.5f };

    KickBackData KickBackWalking = { 1.5, 0.45, 0.225, 0.05, 6.5, 2.5, 7 };
    KickBackData KickBackNotOnGround = { 2.0, 1.0, 0.5, 0.35, 9.0, 6.0, 5 };
    KickBackData KickBackDucking = { 0.9, 0.35, 0.15, 0.025, 5.5, 1.5, 9 };
    KickBackData KickBackDefault = { 1.0, 0.375, 0.175, 0.0375, 5.75, 1.75, 8 };

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "buffaug_bullet";
        p->iMaxAmmo1 = 90;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = 50;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_BUFFAUG;
        p->iFlags = 0;
        p->iWeight = AUG_WEIGHT;

        return 1;
    }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_buffaug_bullet" , 200 }; }
#endif
    float GetArmorRatioModifier() override { return 1.6; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 3, 3, 140 }; }

    void Precache()
    {
        PRECACHE_MODEL("sprites/muzzleflash64.spr");
        PRECACHE_MODEL("sprites/ef_buffaugsmoke.spr");

        Base::Precache();
    }

    void PrimaryAttack() override
    {
        if (m_pPlayer->pev->fov == 90)
        {      
            if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
                Fire(0.02 + (0.2) * m_flAccuracy, 0.077s, FALSE);
            else if (m_pPlayer->pev->velocity.Length2D() > 140)
                Fire(0.02 + (0.01) * m_flAccuracy, 0.077s, FALSE);
            else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
                Fire((0.01) * m_flAccuracy, 0.077s, FALSE);
            else
                Fire((0.02) * m_flAccuracy, 0.077s, FALSE);
        }
        else
        {
            if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
                BuffAugZoomedFire(0.01 + (0.1) * m_flAccuracy, 0.077s, FALSE);
            else if (m_pPlayer->pev->velocity.Length2D() > 140)
                BuffAugZoomedFire(0.01 + (0.01) * m_flAccuracy, 0.077s, FALSE);
            else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
                BuffAugZoomedFire((0.003) * m_flAccuracy, 0.077s, FALSE);
            else
                BuffAugZoomedFire((0.001) * m_flAccuracy, 0.077s, FALSE);   
        }
    }


    void BuffAugZoomedFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_bDelayFire = true;
        m_iShotsFired++;
        m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 210.0) + 0.2;

        if (m_flAccuracy > 0.7)
            m_flAccuracy = 0.7;

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

        StopIdleSound();

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_556MM, GetDamage(), 0.97, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

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
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.9s;

        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            KickBack(0.3, 0.2, 0.2, 0.02, 2.9, 1.9, 7);
        else if (m_pPlayer->pev->velocity.Length2D() > 0)
            KickBack(0.2, 0.1, 0.1, 0.01, 1.9, 0.9, 9);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            KickBack(0.16, 0.06, 0.06, 0.006, 1.7, 0.7, 10);
        else
            KickBack(0.18, 0.08, 0.08, 0.008, 1.8, 0.8, 10);
    }

    void Recoil()
    {
        StopIdleSound();

        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            KickBack(0.32, 0.22, 0.22, 0.022, 3.1, 2.1, 7);
        else if (m_pPlayer->pev->velocity.Length2D() > 0)
            KickBack(0.22, 0.12, 0.12, 0.012, 2.1, 1.1, 9);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            KickBack(0.18, 0.08, 0.08, 0.008, 1.9, 0.9, 10);
        else
            KickBack(0.2, 0.1, 0.1, 0.01, 2.0, 1.0, 10);
    }

    void StopIdleSound(void)
    {
#ifndef CLIENT_DLL
        StopSound("weapons/buffaug_idle.wav");
        StopSound("weapons/buffaug_draw.wav");
#endif
    }

    void StopSound(const char* s)
    {
#ifndef CLIENT_DLL
        MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
        WRITE_BYTE(79);
        WRITE_SHORT(m_pPlayer->entindex());
        WRITE_SHORT(CHAN_AUTO);
        WRITE_STRING(s);
        MESSAGE_END();
#endif
    }

    void Holster(int skiplocal)
    {
        StopIdleSound();

        CBasePlayerWeapon::Holster(skiplocal);
    }

};
LINK_ENTITY_TO_CLASS(weapon_buffaug, CBuffAug)
}