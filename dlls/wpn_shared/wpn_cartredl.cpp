/*
cartredl.cpp
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
    class CCartRedL : public LinkWeaponTemplate< CCartRedL,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_MP5N>::template type,
            TPrecacheEvent,
            TReloadDefault,
            TGetDamageDefault,
            TPrimaryAttackRifle,
            TFireRifle,
            TRecoilKickBack,
            TWeaponIdleDefault
    >
    {
    public:

    static constexpr const auto& DefaultReloadTime = 3.5s;
    static constexpr const char *V_Model = "models/v_cartred.mdl";
    static constexpr const char *P_Model = "models/p_cartred.mdl";
    static constexpr const char *W_Model = "models/w_cartred.mdl";
    static constexpr const char *EventFile = "events/cartredl.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_cartredl";
    static constexpr const char *AnimExtension = "mp5";
    static constexpr int MaxClip = 30;

    enum
	{
        ANIM_IDLE1,
        ANIM_RELOAD,
        ANIM_DRAW,
        ANIM_SHOOT1,
        ANIM_SHOOT2,
        ANIM_SHOOT3,
        ANIM_CHANGE_H,
        ANIM_IDLE_H,
        ANIM_RELOAD_H,
        ANIM_DRAW_H,
        ANIM_SHOOT1_H,
        ANIM_SHOOT2_H,
        ANIM_SHOOT3_H,
        ANIM_CHANGE_L,
	};
    static constexpr float PlayerMaxSpeed = 225;
    static constexpr float ArmorRatioModifier = 1.3f;

    static constexpr float PrimaryAttackWalkingMiniumSpeed = 140;
    static constexpr const auto& SpreadCalcNotOnGround = 0.4 * A + 0.04;
    static constexpr const auto& SpreadCalcWalking = 0.04 * A + 0.02;
    static constexpr const auto& SpreadCalcDefault = 0.03 * A;


    static constexpr const auto& CycleTime = 0.09s;

    static constexpr int DamageDefault = 26;
    static constexpr int DamageZB = 53;
    static constexpr int DamageZBS = 53;

    static constexpr const auto& AccuracyCalc = (N * N * N / 200.0) + 0.4;
    static constexpr float AccuracyMax = 0.45;
    static constexpr float AccuracyDefault = 0.2;
    static constexpr float RangeModifier = 0.94;
    static constexpr auto BulletType = BULLET_PLAYER_556MM;
    static constexpr int Penetration = 2;
    static constexpr int Distance = 8192;

    KickBackData KickBackWalking = { 0.5f, 0.11, 0.085f, 0.02f, 1.0f, 0.8f, 10 };
    KickBackData KickBackNotOnGround = { 1.0f, 0.5f, 0.4f, 0.3f, 8.5f, 6.125f, 1 };
    KickBackData KickBackDucking = { 0.2f, 0.2f, 0.02f, 0.01f, 0.8, 0.5f, 4 };
    KickBackData KickBackDefault = { 0.5f, 0.11f, 0.085f, 0.02f, 1.0f, 0.8f, 4 };

    KnockbackData KnockBack = { 450.0f, 400.0f, 400.0f, 200.0f, 0.5f };

    void Spawn()
    {
         Base::Spawn();
         m_pLink = nullptr;
         m_iSwing = 0;
    }

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "556Nato";
        p->iMaxAmmo1 = 90;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = 30;
        p->iSlot = 0;
        p->iPosition = 10;
        p->iId = m_iId = WEAPON_CARTREDL;
        p->iFlags = 0;
        p->iWeight = M4A1_WEIGHT;

        return 1;
    }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_M4A1); }
#endif
    float GetArmorRatioModifier() override { return 1.3; }
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
                    if (!Q_stricmp(pItem->pszName(), "weapon_cartredh"))
                    {
                        m_pLink = pItem;

                        m_pLink->m_iSwing = 1;
                        m_iSwing = 0;
                    }

                    pItem = pItem->m_pNext;
                }
            }
            if (!m_pLink)
                pPlayer->GiveNamedItem("weapon_cartredh");

        }
    }

    ~CCartRedL()
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
        m_flAccuracy = 0.2;

        if (m_iSwing)
        {
            bResult = DefaultDeploy(V_Model, P_Model, ANIM_CHANGE_H, "carbine", UseDecrement() != FALSE);
            m_pPlayer->m_flNextAttack = 5.0s;
        }
        else
        {
            bResult = DefaultDeploy(V_Model, P_Model, ANIM_DRAW, "mp5", UseDecrement() != FALSE);
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
LINK_ENTITY_TO_CLASS(weapon_cartredl, CCartRedL)
}