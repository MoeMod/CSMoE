/*
skull3d.cpp
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
    class CSKULL3D : public LinkWeaponTemplate< CSKULL3D,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_P90>::template type,
            TPrecacheEvent
    >
    {
    public:

#define WPNSTATE_SKULL3D_LEFT	(1<<0)

    static constexpr const char *V_Model = "models/v_skull3.mdl";
    static constexpr const char *P_Model = "models/p_skull3dual.mdl";
    static constexpr const char *W_Model = "models/w_skull3.mdl";
    static constexpr const char *EventFile = "events/skull3d.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_skull3d";
    static constexpr const char *AnimExtension = "dualpistols";
    static constexpr int MaxClip = 75;
    KnockbackData KnockBack = { 300.0f, 250.0f, 300.0f, 110.0f, 0.7f };

	enum
	{
        ANIM_IDLE1,
        ANIM_RELOAD,
        ANIM_DRAW,
        ANIM_SHOOT1,
        ANIM_SHOOT2,
        ANIM_SHOOT3,
        ANIM_IDLE,
        ANIM_IDLE_LEFTEMPTY,
        ANIM_SHOOT_LEFT1,
        ANIM_SHOOT_LEFT2,
        ANIM_SHOOT_LEFTLAST,
        ANIM_SHOOT_RIGHT1,
        ANIM_SHOOT_RIGHT2,
        ANIM_SHOOT_RIGHTLAST,
        ANIM_RELOAD2,
        ANIM_DRAW2,
        ANIM_DRAW3,
        ANIM_DRAW4,
	};
    static constexpr float PlayerMaxSpeed = 175;
    static constexpr float ArmorRatioModifier = 1.5f;

    int m_usFireSKULL3DLeft;
    int m_usFireSKULL3DRight;

    void Spawn()
    {
        m_bDelayFire = false;
        Base::Spawn(); 
        m_pLink = nullptr;
        m_iSwing = 0;
    }

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "45acpaz";
        p->iMaxAmmo1 = 120;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = 75;
        p->iSlot = 0;
        p->iPosition = 15;
        p->iId = m_iId = WEAPON_SKULL3D;
        p->iFlags = 0;
        p->iWeight = MP5NAVY_WEIGHT;

        return 1;
    }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_45acpaz" , 75 }; }
#endif
    float GetArmorRatioModifier() override { return 1.5; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR, 6, 3, 0 }; }

    void Precache()
    {
        m_iShellId = PRECACHE_MODEL("models/pshell.mdl");

        m_usFireSKULL3DLeft = PRECACHE_EVENT(1, "events/skull3d_left.sc");
        m_usFireSKULL3DRight = PRECACHE_EVENT(1, "events/skull3d_right.sc");
        
        Base::Precache();
    }


#ifndef CLIENT_DLL
    void AttachToPlayer(CBasePlayer* pPlayer)
    {
        CBasePlayerWeapon::AttachToPlayer(pPlayer);

        // find the linked weapon
        while (!m_pLink)
        {
            for (int i = 0; i < MAX_ITEM_TYPES; ++i)
            {
                CBasePlayerItem* pItem = m_pPlayer->m_rgpPlayerItems[i];
                while (pItem != NULL)
                {
                    if (!Q_stricmp(pItem->pszName(), "weapon_skull3"))
                    {
                        m_pLink = pItem;

                        m_pLink->m_iSwing = 1;
                        m_iSwing = 0;
                    }

                    pItem = pItem->m_pNext;
                }
            }
            if (!m_pLink)
                pPlayer->GiveNamedItem("weapon_skull3");

        }
    }

    ~CSKULL3D()
    {
        CBaseEntity* pOther = m_pLink;
        if (pOther && pOther->m_pLink == this)
        {
            pOther->m_pLink = m_pLink = nullptr;
            pOther->SUB_Remove();
        }
    }
#endif

    void ItemPostFrame()
    {
#ifndef CLIENT_DLL
        m_iSwing = 0;
        if (m_pLink)
            m_pLink->m_iSwing = 1;
#endif
        return CBasePlayerWeapon::ItemPostFrame();
    }

    float GetMaxSpeed(void)
    {
        return 235.0;
    }


    BOOL Deploy() override
    {
        BOOL bResult;

        m_flAccuracy = 0.0;
        m_iShotsFired = 0;
        m_bDelayFire = false;

        if (!(m_iClip & 1))
            m_iWeaponState |= WPNSTATE_SKULL3D_LEFT;

        if (m_iSwing)
        {
            bResult = DefaultDeploy("models/v_skull3.mdl", "models/p_skull3.mdl", ANIM_DRAW3, "carbine", UseDecrement() != FALSE);
            m_pPlayer->m_flNextAttack = 3.0s;
        }
        else
        {
            bResult = DefaultDeploy("models/v_skull3.mdl", "models/p_skull3dual.mdl", ANIM_DRAW2, m_pPlayer->m_bIsFemale ? "dualpistols" : "dualpistols_2", UseDecrement() != FALSE);
        }

        return bResult;
    }

    void PrimaryAttack(void)
    {
        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            SKULL3dFire((0.375) * m_flAccuracy + 0.07, 0.07s, FALSE);
        else
            SKULL3dFire((0.0275) * m_flAccuracy, 0.07s, FALSE);

    }

    void SKULL3dFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_bDelayFire = true;
        m_iShotsFired++;
        m_flAccuracy = (m_iShotsFired * m_iShotsFired) / 190.0 + 0.6;

        if (m_flAccuracy > 1.5)
            m_flAccuracy = 1.5;

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

        unsigned short eventindex;
        if (m_iWeaponState & WPNSTATE_SKULL3D_LEFT)
        {
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
            m_iWeaponState &= ~WPNSTATE_SKULL3D_LEFT;
            eventindex = m_usFireSKULL3DLeft;
        }
        else
        {
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_ATTACK2);
#endif
            m_iWeaponState |= WPNSTATE_SKULL3D_LEFT;
            eventindex = m_usFireSKULL3DRight;
        }

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
        Vector vecDir = m_pPlayer->FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192.0, 5, BULLET_PLAYER_45ACP, GetDamage(), 0.93, m_pPlayer->pev, false, m_pPlayer->random_seed);

        int flags;

#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), eventindex, 0.0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, int(m_pPlayer->pev->punchangle.x * 100), int(m_pPlayer->pev->punchangle.y * 100), FALSE, m_iClip);

        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", 0, 0);
#endif
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;

        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            KickBack(1.1, 0.58, 0.43, 0.055, 5.0, 3.25, 9);
        else if (m_pPlayer->pev->velocity.Length2D() > 0)
            KickBack(0.7, 0.32, 0.275, 0.035, 4.25, 2.23, 9);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            KickBack(0.275, 0.18, 0.13, 0.02, 2.25, 1.25, 10);
        else
            KickBack(0.3, 0.22, 0.18, 0.025, 3.0, 1.85, 10);
    }


    float GetDamage(void)
    {
        float flDamage;

        flDamage = 36.0f;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = 48.0f;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = 74.0f;
#endif
        return flDamage;
        
    }

    void Reload(void)
    {
        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            return;

        if (DefaultReload(MaxClip, ANIM_RELOAD2, 3.3s))
        {
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
            m_flAccuracy = 0;
            m_iShotsFired = 0;
        }
    }

    void WeaponIdle(void)
    {
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

        if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
            return;

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0s;

        if (m_iClip)
        {
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60.0s;

            SendWeaponAnim(m_iClip == 1 ? ANIM_IDLE_LEFTEMPTY : ANIM_IDLE, UseDecrement() != FALSE);
        }
    }

};
LINK_ENTITY_TO_CLASS(weapon_skull3d, CSKULL3D)
}