/*
skull3.cpp
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
    class CSKULL3 : public LinkWeaponTemplate< CSKULL3,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_MP5N>::template type,
            TPrecacheEvent,
            TReloadDefault,
            TGetDamageDefault,
            TSecondaryAttackZoom,
            TPrimaryAttackRifle,
            TFireRifle,
            TRecoilKickBack,
            TWeaponIdleDefault
    >
    {
    public:

    static constexpr const auto& DefaultReloadTime = 2.3s;
    static constexpr int ZoomFOV = 64;
    static constexpr const char *V_Model = "models/v_skull3.mdl";
    static constexpr const char *P_Model = "models/p_skull3.mdl";
    static constexpr const char *W_Model = "models/w_skull3.mdl";
    static constexpr const char *EventFile = "events/skull3.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_skull3";
    static constexpr const char *AnimExtension = "mp5";
    static constexpr int MaxClip = 35;

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
    static constexpr float PlayerMaxSpeed = 245;
    static constexpr float ArmorRatioModifier = 1.5f;

    static constexpr const auto& SpreadCalcNotOnGround = 0.375 * A + 0.07;
    static constexpr const auto& SpreadCalcDefault = 0.025 * A;

    static constexpr const auto& CycleTime = 0.085s;

    static constexpr int DamageDefault = 29;
    static constexpr int DamageZB = 37;
    static constexpr int DamageZBS = 37;

    static constexpr const auto& AccuracyCalc = (N * N / 190.0) + 0.475;
    static constexpr float AccuracyMax = 1.4f;
    static constexpr float AccuracyDefault = 0.0;
    static constexpr float RangeModifier = 0.95;
    static constexpr auto BulletType = BULLET_PLAYER_45ACP;
    static constexpr int Penetration = 1;
    static constexpr int Distance = 8192;

    KickBackData KickBackWalking = { 0.6f, 0.3f, 0.25f, 0.035f, 3.5f, 2.25f, 10 };
    KickBackData KickBackNotOnGround = { 1.0f, 0.525f, 0.4f, 0.05f, 4.6f, 2.35f, 9 };
    KickBackData KickBackDucking = { 0.25f, 0.175f, 0.125f, 0.02f, 2.25f, 1.2f, 10 };
    KickBackData KickBackDefault = { 0.275f, 0.32f, 0.17f, 0.05f, 2.4f, 1.85f, 9 };

    KnockbackData KnockBack = { 300.0f, 250.0f, 300.0f, 110.0f, 0.7f };

    int m_iChargeShotsFired;
    int m_iExpIndex;

    void Spawn()
    {
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
        p->iMaxClip = 35;
        p->iSlot = 0;
        p->iPosition = 14;
        p->iId = m_iId = WEAPON_SKULL3;
        p->iFlags = 0;
        p->iWeight = MP5NAVY_WEIGHT;

        return 1;
    }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_45acpaz" , 75 }; }
#endif
    float GetArmorRatioModifier() override { return 1.4; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR, 6, 3, 0 }; }

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
                    if (!Q_stricmp(pItem->pszName(), "weapon_skull3d"))
                    {
                        m_pLink = pItem;

                        m_pLink->m_iSwing = 1;
                        m_iSwing = 0;
                    }

                    pItem = pItem->m_pNext;
                }
            }
            if (!m_pLink)
                pPlayer->GiveNamedItem("weapon_skull3d");

        }
    }

    ~CSKULL3()
    {
        CBaseEntity* pOther = m_pLink;
        if (pOther && pOther->m_pLink == this)
        {
            pOther->m_pLink = m_pLink = nullptr;
            pOther->SUB_Remove();
        }
    }
#endif


    BOOL Deploy() override
    {
        BOOL bResult;

        m_iShotsFired = 0;
        m_bDelayFire = false;
        m_flAccuracy = 0.0;

        if (m_iSwing)
        {
            bResult = DefaultDeploy("models/v_skull3.mdl", "models/p_skull3dual.mdl", ANIM_DRAW4, m_pPlayer->m_bIsFemale ? "dualpistols" : "dualpistols_2", UseDecrement() != FALSE);
            m_pPlayer->m_flNextAttack = 3.0s;
        }
        else
        {
            bResult = DefaultDeploy("models/v_skull3.mdl", "models/p_skull3.mdl", ANIM_DRAW, "carbine", UseDecrement() != FALSE);
        }

        return bResult;
    }
    

    void ItemPostFrame()
    {
#ifndef CLIENT_DLL
        m_iSwing = 0;
        if (m_pLink)
            m_pLink->m_iSwing = 1;
#endif
        return CBasePlayerWeapon::ItemPostFrame();
    }


};
LINK_ENTITY_TO_CLASS(weapon_skull3, CSKULL3)
}