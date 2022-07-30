/*
janusmk5.cpp
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
    class CJANUSMK5 : public LinkWeaponTemplate< CJANUSMK5,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_AUG>::template type,
            TPrecacheEvent,
            TGetDamageDefault
    >
    {
    public:

    static constexpr const char *V_Model = "models/v_janusmk5.mdl";
    static constexpr const char *P_Model = "models/p_janusmk5.mdl";
    static constexpr const char *W_Model = "models/w_janusmk5.mdl";
    static constexpr const char *EventFile = "events/janusmk5.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_janusmk5";
    static constexpr const char *AnimExtension = "rifle";
    static constexpr int MaxClip = 30;
	enum
	{
        ANIM_IDLE1 = 0,
        ANIM_RELOAD1,
        ANIM_DRAW1,
        ANIM_SHOOT1,
        ANIM_SHOOT1_SIGNAL,
        ANIM_CHANGE1,
        ANIM_IDLE2,
        ANIM_DRAW2,
        ANIM_SHOOT2_1,
        ANIM_SHOOT2_2,
        ANIM_SHOOT2_3,
        ANIM_CHANGE2,
        ANIM_SIGNAL,
        ANIM_RELOAD_SIGNAL,
        ANIM_DRAW_SIGNAL,
	};
    static constexpr float PlayerMaxSpeed = 230;

    static constexpr int DamageDefault = 32;
    static constexpr int DamageZB = 33;
    static constexpr int DamageZBS = 38;

    KnockbackData KnockBack = { 550.0f, 350.0f, 350.0f, 450.0f, 0.65f };

    time_point_t m_flNextReset;
    int m_iShotsRequired;

#define WPNSTATE_JANUSMK5_SIGNAL 1
#define WPNSTATE_JANUSMK5_USING 2

    void Spawn() override
    {
        m_flNextReset = invalid_time_point;
        m_iShotsRequired = GetShotsRequired();
        Base::Spawn();
    }

    int GetShotsRequired()
    {
        int iShotsRequired = 46;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            iShotsRequired = 57;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            iShotsRequired = 50;
#endif
        return iShotsRequired;
    }

    void Precache()
    {
        PRECACHE_MODEL("sprites/muzzleflash2.spr");
        PRECACHE_MODEL("sprites/muzzleflash25.spr");
        PRECACHE_SOUND("weapons/change1_ready.wav");
        Base::Precache();
    }

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "556Nato";
        p->iMaxAmmo1 = 90;
        p->pszAmmo2 = "InfiniteAmmo";
        p->iMaxAmmo2 = 255;
        p->iMaxClip = 30;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_JANUSMK5;
        p->iFlags = 0;
        p->iWeight = AUG_WEIGHT;

        return 1;
    }

    int ExtractAmmo(CBasePlayerWeapon* pWeapon)
    {
        m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 0;
        return CBasePlayerWeapon::ExtractAmmo(pWeapon);
    }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_AUG); }
#endif
    float GetArmorRatioModifier() override { return 1.45; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 4, 3, 140 }; }

    BOOL Deploy() override
    {
        m_flAccuracy = 0.2;
        m_iShotsFired = 0;

        int iAnim;
        if (m_iWeaponState == WPNSTATE_JANUSMK5_USING)
            iAnim = ANIM_DRAW2;
        else if (m_iWeaponState == WPNSTATE_JANUSMK5_SIGNAL)
            iAnim = ANIM_DRAW_SIGNAL;
        else
            iAnim = ANIM_DRAW1;

        return DefaultDeploy("models/v_janusmk5.mdl", "models/p_janusmk5.mdl", iAnim, "carbine", UseDecrement() != FALSE);
    }

    void PrimaryAttack() override
    {
        if (m_iWeaponState == WPNSTATE_JANUSMK5_USING)
            Fire(0.0715, 0.072s, FALSE);
        else
        {
            if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
                Fire(0.035 + (0.4) * m_flAccuracy, 0.09s, FALSE);
            else if (m_pPlayer->pev->velocity.Length2D() > 140)
                Fire(0.0325 + (0.045) * m_flAccuracy, 0.09s, FALSE);
            else
                Fire((0.02) * m_flAccuracy, 0.09s, FALSE);
        }
        
    }

    void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_bDelayFire = true;
        m_iShotsFired++;
        m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 210) + 0.3;

        if (m_flAccuracy > 1.2)
            m_flAccuracy = 1.2;

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


        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecDir;
        if (m_iWeaponState == WPNSTATE_JANUSMK5_USING)
        {
#ifndef CLIENT_DLL
            m_pPlayer->FireBullets(3, m_pPlayer->GetGunPosition(), gpGlobals->v_forward, Vector(0.0715, 0.0715, 0.0), 8192, BULLET_PLAYER_BUCKSHOT, 0, GetDamageB());
            PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, static_cast<int>(m_pPlayer->pev->punchangle.x * 100), static_cast<int>(m_pPlayer->pev->punchangle.y * 100), m_iWeaponState, FALSE);
#endif
        }
        else
        {
            vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_556LVB, GetDamage(), 0.97, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);
            m_iClip--;
        }
         
        vecDir = gpGlobals->v_forward + vecDir[0] * gpGlobals->v_right + vecDir[1] * gpGlobals->v_up;
        ChargeToLineTargets(vecSrc, vecDir);
#ifndef CLIENT_DLL
        if (m_iWeaponState != WPNSTATE_JANUSMK5_USING)
            PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, static_cast<int>(m_pPlayer->pev->punchangle.x * 100), static_cast<int>(m_pPlayer->pev->punchangle.y * 100), m_iWeaponState, FALSE);
#endif
        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.03s;

        if (m_iWeaponState == WPNSTATE_JANUSMK5_USING)
        {
            if (m_pPlayer->pev->velocity.Length2D() > 0)
                KickBack(0.9, 0.435, 0.25, 0.04, 3.25, 3.0, 8);
            else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
                KickBack(1.14, 0.475, 0.225, 0.145, 5.4, 3.5, 6);
            else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
                KickBack(0.45, 0.2, 0.12, 0.01, 3.0, 2.0, 7);
            else
                KickBack(0.595, 0.32, 0.215, 0.0105, 3.25, 2.0, 8);
        }
        else
        {
            if (m_pPlayer->pev->velocity.Length2D() > 0)
                KickBack(0.875, 0.425, 0.275, 0.0435, 3.5, 2.5, 8);
            else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
                KickBack(1.1, 0.47, 0.215, 0.135, 5.2, 3.3, 6);
            else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
                KickBack(0.5, 0.24, 0.12, 0.01, 3.15, 2.0, 7);
            else
                KickBack(0.595, 0.32, 0.225, 0.0115, 3.25, 2.1, 8);
        }
    }

    void ChargeToLineTargets(Vector vecSrc, Vector vecDir)
    {
        TraceResult tr;

        UTIL_TraceLine(vecSrc, vecSrc + vecDir * 8192.0, dont_ignore_monsters, m_pPlayer->edict(), &tr);

        if (tr.flFraction < 1.0 && !FNullEnt(tr.pHit))
        {
            CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

            if (pEntity && (pEntity->IsPlayer() || pEntity->Classify() == CLASS_PLAYER_ALLY))
            {
                if (!m_iWeaponState)
                {
                    pev->iuser1++;
                    if (pev->iuser1 >= m_iShotsRequired)
                    {
                        m_iWeaponState = WPNSTATE_JANUSMK5_SIGNAL;
                        m_flNextReset = gpGlobals->time + 9.0s;

                        EMIT_SOUND_DYN(edict(), CHAN_ITEM, "weapons/change1_ready.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);
                    }
                }
            }
        }
    }

    void ItemPostFrame(void) override
    {
        if (m_flNextReset != invalid_time_point && m_flNextReset < gpGlobals->time)
        {
            if (m_iWeaponState == WPNSTATE_JANUSMK5_SIGNAL)
            {
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0s;
                m_flNextReset = invalid_time_point;
            }
            if (m_iWeaponState == WPNSTATE_JANUSMK5_USING)
            {
                m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 0;
                SendWeaponAnim(ANIM_CHANGE2, UseDecrement() != FALSE);
                m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.7s;
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.7s;
                m_flNextReset = invalid_time_point;
            }
            pev->iuser1 = 0;
            m_iWeaponState = 0;
        }
        return CBasePlayerWeapon::ItemPostFrame();
    }

    void SecondaryAttack(void)
    {
        if (m_iWeaponState == WPNSTATE_JANUSMK5_SIGNAL)
        {
            m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 255;
            m_iWeaponState = WPNSTATE_JANUSMK5_USING;
            SendWeaponAnim(ANIM_CHANGE1, UseDecrement() != FALSE);
            m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.0s;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;

            m_flNextReset = gpGlobals->time + 13.0s;
        }
    }

    void Reload() override
    {
        if (m_pPlayer->ammo_556nato <= 0 || m_iWeaponState == WPNSTATE_JANUSMK5_USING)
            return;

        if (DefaultReload(MaxClip, m_iWeaponState == WPNSTATE_JANUSMK5_SIGNAL ? ANIM_RELOAD_SIGNAL : ANIM_RELOAD1, 3.0s))
        {
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
            m_flAccuracy = 0.2;
            m_iShotsFired = 0;
            m_bDelayFire = false;
        }
    }

    void WeaponIdle(void) override
    {
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

        if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
            return;

        int iAnim;
        if (m_iWeaponState == WPNSTATE_JANUSMK5_USING)
            iAnim = ANIM_IDLE2;
        else if (m_iWeaponState == WPNSTATE_JANUSMK5_SIGNAL)
            iAnim = ANIM_SIGNAL;
        else
            iAnim = ANIM_IDLE1;

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
        SendWeaponAnim(iAnim, UseDecrement() != FALSE);
    }

    int GetDamageB()
    {
        int iDamage = 31;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            iDamage = 32;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            iDamage = 56;
#endif
        return iDamage;
    }

};
LINK_ENTITY_TO_CLASS(weapon_janusmk5, CJANUSMK5)
}