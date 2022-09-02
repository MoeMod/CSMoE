/*
balrog5.cpp
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
    class CBalrog5 : public LinkWeaponTemplate< CBalrog5,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_AUG>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TGetDamageDefault,
            TReloadDefault,
            TRecoilKickBack,
            TWeaponIdleDefault
    >
    {
    public:

    static constexpr const auto& DefaultReloadTime = 2.7s;
    static constexpr const char *V_Model = "models/v_balrog5.mdl";
    static constexpr const char *P_Model = "models/p_balrog5.mdl";
    static constexpr const char *W_Model = "models/w_balrog5.mdl";
    static constexpr const char *EventFile = "events/balrog5.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_balrog5";
    static constexpr const char *AnimExtension = "mp5";

    static constexpr int MaxClip = 40;

#define WPNSTATE_BALROG5_CHARGE 1
#define WPNSTATE_BALROG5_CHARGECLIP 3
	enum
	{
        ANIM_IDLE1,
        ANIM_SHOOT1,
        ANIM_SHOOT2,
        ANIM_SHOOT3,
        ANIM_RELOAD,
        ANIM_DRAW,
	};
    static constexpr float PlayerMaxSpeed = 230;
    static constexpr float ArmorRatioModifier = 1.4f;
    static constexpr const auto& CycleTime = 0.13s;
    static constexpr int DamageDefault = 2;
    static constexpr int DamageZB = 42;
    static constexpr int DamageZBS = 105;
    static constexpr float AccuracyDefault = 0.0;
    static constexpr float AccuracyMax = 1.5;
    static constexpr float RangeModifier = 0.97;

    KnockbackData KnockBack = { 400.0f, 290.0f, 350.0f, 180.0f, 0.7f };

    int m_iTargetIndex;
    int m_iChargeShotsFired;
    int m_iExpIndex;
    bool m_bHit;

    void Spawn()
    {
        m_iTargetIndex = 0;
        m_iChargeShotsFired = 0;
        Base::Spawn();
    }

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "556ahe";
        p->iMaxAmmo1 = 120;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = 40;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_BALROG5;
        p->iFlags = 0;
        p->iWeight = MP5NAVY_WEIGHT;

        return 1;
    }
    
	float GetMaxSpeed(void)
	{
        if (m_pPlayer->pev->fov != 90)
			return 210;

		return 230;
	}

    void SecondaryAttack()
    {
        if (m_pPlayer->m_iFOV != 90)
            m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;
        else
            m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 65;

        m_pPlayer->ResetMaxSpeed();
        m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;
    }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_556ahe" , 200 }; }
#endif
    float GetArmorRatioModifier() override { return 1.4; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 5, 3, 130 }; }

    void Precache()
    {
        PRECACHE_MODEL("sprites/muzzleflash20.spr");
        PRECACHE_MODEL("sprites/muzzleflash21.spr");
        m_iExpIndex = PRECACHE_MODEL("sprites/balrog5stack.spr");
        
        Base::Precache();
    }

    BOOL Deploy() override
    {
        m_iWeaponState = 0;
        m_iChargeShotsFired = 0;
        m_iTargetIndex = 0;
        m_flAccuracy = 0.0f;
        return Base::Deploy();
    }

    void PrimaryAttack(void)
    {
        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            Fire(m_flAccuracy * 0.4 + 0.035, CycleTime, FALSE);
        else if (m_pPlayer->pev->velocity.Length2D() > 130)
            Fire(m_flAccuracy * 0.05 + 0.035, CycleTime, FALSE);
        else if (m_pPlayer->pev->fov != 90)
            Fire(m_flAccuracy * 0.008, CycleTime, FALSE);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            Fire(m_flAccuracy * 0.018, CycleTime, FALSE);
        else
            Fire(m_flAccuracy * 0.02, CycleTime, FALSE);

    }

#ifndef CLIENT_DLL
    void FireBullet3CallBack(int iPenetration, Vector vecDir, TraceResult* tr)
    {
        Vector vecEnd = tr->vecEndPos - vecDir * 5;
        int iOriginalPenetration = 2;

        if (iOriginalPenetration == iPenetration)
        {
            if (CBaseEntity* pEntity = gMultiDamage.pEntity)
            {
                if (pEntity->IsPlayer() || pEntity->Classify() == CLASS_PLAYER_ALLY)
                {
                    if (m_iTargetIndex == pEntity->entindex())
                    {
                        m_iChargeShotsFired++;
                        if (m_iWeaponState < 5 && m_iChargeShotsFired >= WPNSTATE_BALROG5_CHARGECLIP)
                        {
                            m_iWeaponState++;
                            m_iChargeShotsFired = 0;
                        }

                        if (m_iWeaponState)
                        {
                            pEntity->TraceAttack(m_pPlayer->pev, GetDamageB(), vecDir, tr, DMG_BULLET | DMG_NEVERGIB);
                        }

                        if (m_iWeaponState == 5)
                        {
                            m_bHit = true;

                            MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
                            WRITE_BYTE(TE_TEMPSPRITE);
                            WRITE_COORD(vecEnd.x);
                            WRITE_COORD(vecEnd.y);
                            WRITE_COORD(vecEnd.z + 30);
                            WRITE_SHORT(m_iExpIndex);
                            WRITE_BYTE(5);
                            WRITE_BYTE(255);
                            WRITE_BYTE(15);
                            MESSAGE_END();
                        }
                    }
                    else
                    {
                        m_iTargetIndex = pEntity->entindex();
                        m_iWeaponState = 0;
                        m_iChargeShotsFired = 0;
                        
                        m_iTargetIndex = pEntity->entindex();
                        m_iChargeShotsFired++;
                        if (m_iWeaponState < 5 && m_iChargeShotsFired >= WPNSTATE_BALROG5_CHARGECLIP)
                        {
                            m_iWeaponState++;
                            m_iChargeShotsFired = 0;
                        }
                    }          
                }
            }
        }
    }
#endif


    void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_bDelayFire = true;
        m_iShotsFired++;

        m_flAccuracy = ((m_iShotsFired * m_iShotsFired) / 230.0) + 0.39;
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
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;

        Vector vecForward;
        vecForward = gpGlobals->v_forward;

        m_bHit = false;

        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecDir = m_pPlayer->FireBullets3(vecSrc, vecForward, flSpread, 8192, 2, BULLET_PLAYER_556MM, GetDamage(), 0.97, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);
#ifndef CLIENT_DLL
        PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), m_bHit, FALSE);
#endif      
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;

        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            KickBack(1.2, 0.5, 0.23, 0.15, 5.5, 3.5, 6);
        else if (m_pPlayer->pev->velocity.Length2D() > 0)
            KickBack(1.0, 0.45, 0.28, 0.045, 3.75, 3, 7);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            KickBack(0.225, 0.15, 0.1, 0.015, 2, 1, 10);
        else
            KickBack(0.65, 0.35, 0.25, 0.015, 3.5, 2.25, 7);
    }

    float GetDamageB() const
    {
        float flDamage;
        switch (m_iWeaponState)
        {
        case 1: flDamage = 1.0f; break;
        case 2: flDamage = 2.0f; break;
        case 3: flDamage = 3.0f; break;
        case 4: flDamage = 4.0f; break;
        case 5: flDamage = 5.0f; break;
        default: flDamage = 0.0f; break;
        }  
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
        {
            switch (m_iWeaponState)
            {
            case 1: flDamage = 1.0f; break;
            case 2: flDamage = 3.0f; break;
            case 3: flDamage = 5.0f; break;
            case 4: flDamage = 7.0f; break;
            case 5: flDamage = 9.0f; break;
            default: flDamage = 0.0f; break;
            }
        }
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
        {
            switch (m_iWeaponState)
            {
            case 1: flDamage = 1.0f; break;
            case 2: flDamage = 5.0f; break;
            case 3: flDamage = 9.0f; break;
            case 4: flDamage = 15.0f; break;
            case 5: flDamage = 30.0f; break;
            default: flDamage = 0.0f; break;
            }
        }
#endif
        return flDamage;
    }

};
LINK_ENTITY_TO_CLASS(weapon_balrog5, CBalrog5)
}