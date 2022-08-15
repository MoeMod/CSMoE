/*
balrog3.cpp
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
    class CBalrog3 : public LinkWeaponTemplate< CBalrog3,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_MP5N>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TGetDamageDefault,
            TReloadDefault,
            TSecondaryAttackZoom,
            TRecoilKickBack,
            TWeaponIdleDefault
    >
    {
    public:

    static constexpr const auto& DefaultReloadTime = 2.63s;
    static constexpr int ZoomFOV = 65;
    static constexpr const char *V_Model = "models/v_balrog3.mdl";
    static constexpr const char *P_Model = "models/p_balrog3.mdl";
    static constexpr const char *W_Model = "models/w_balrog3.mdl";
    static constexpr const char *EventFile = "events/balrog3.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_balrog3";
    static constexpr const char *AnimExtension = "mp5";
    static constexpr int MaxClip = 30;

#define WPNSTATE_BALROG3_CHARGE 1
#define WPNSTATE_BALROG3_CHARGECLIP 15
	enum
	{
        ANIM_IDLE1,
        ANIM_RELOAD,
        ANIM_DRAW,
        ANIM_SHOOT1,
        ANIM_SHOOT2,
	};
    static constexpr float PlayerMaxSpeed = 250;
    static constexpr float ArmorRatioModifier = 1.5f;

    static constexpr const auto& CycleTime = 0.07s;
 
    static constexpr int DamageDefault = 27;
    static constexpr int DamageZB = 40;
    static constexpr int DamageZBS = 105;

    static constexpr float AccuracyDefault = 0.0;
    static constexpr float AccuracyMax = 0.7;
    static constexpr float RangeModifier = 0.8;

    KnockbackData KnockBack = { 250.0f, 200.0f, 250.0f, 90.0f, 0.7f };

    int m_iChargeShotsFired;
    int m_iExpIndex;

    void Spawn()
    {
        m_iChargeShotsFired = 0;
        Base::Spawn();
    }

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "9mmahe";
        p->iMaxAmmo1 = MAX_AMMO_9MM;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = 30;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_BALROG3;
        p->iFlags = 0;
        p->iWeight = MP5NAVY_WEIGHT;

        return 1;
    }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_9mmahe" , 20 }; }
#endif
    float GetArmorRatioModifier() override { return 1.0; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR, 6, 3, 0 }; }

    void Precache()
    {
        PRECACHE_MODEL("sprites/muzzleflash21.spr");
        PRECACHE_MODEL("sprites/muzzleflash26.spr");
        m_iExpIndex = PRECACHE_MODEL("sprites/balrog5stack.spr");
        
        Base::Precache();
    }

    BOOL Deploy() override
    {
        m_iChargeShotsFired = 0;
        m_iWeaponState &= ~WPNSTATE_BALROG3_CHARGE;
        m_flAccuracy = 0.0f;
        return Base::Deploy();
    }

    void PrimaryAttack(void)
    {
        if (m_iWeaponState & WPNSTATE_BALROG3_CHARGE)
        {
            if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
                Fire(0.23 * m_flAccuracy, 0.063s, FALSE);
            else
                Fire((0.05) * m_flAccuracy, 0.063s, FALSE);
        }
        else
        {
            if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
                Fire((0.2) * m_flAccuracy, 0.082s, FALSE);
            else
                Fire((0.04) * m_flAccuracy, 0.082s, FALSE);
        }

    }

    void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_bDelayFire = true;
        m_iShotsFired++;

        if (m_iWeaponState & WPNSTATE_BALROG3_CHARGE)
        {
            m_flAccuracy = ((m_iShotsFired * m_iShotsFired) / 220.0) + 0.47;
            if (m_flAccuracy > 0.7)
                m_flAccuracy = 0.7;
        }
               
        else
        {
            m_flAccuracy = ((m_iShotsFired * m_iShotsFired) / 220.0) + 0.4;
            if (m_flAccuracy > 0.75)
                m_flAccuracy = 0.75;
        }         
 
        if (m_iClip <= 0)
        {
            if (m_fFireOnEmpty)
            {
                PlayEmptySound();
                m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
            }
            return;
        }

        if (m_iWeaponState & WPNSTATE_BALROG3_CHARGE && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0)
            m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
        else
            m_iClip--;

        m_iChargeShotsFired++;
        if (!(m_iWeaponState & WPNSTATE_BALROG3_CHARGE))
        {
            if (m_iChargeShotsFired == WPNSTATE_BALROG3_CHARGECLIP)
                m_iWeaponState |= WPNSTATE_BALROG3_CHARGE;
        }


        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;

        Vector vecUp, vecRight, vecForward;
        vecForward = gpGlobals->v_forward;
        vecUp = gpGlobals->v_up;
        vecRight = gpGlobals->v_right;

        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecDir = m_pPlayer->FireBullets3(vecSrc, vecForward, flSpread, 8192, 1, BULLET_PLAYER_9MM, m_iWeaponState & WPNSTATE_BALROG3_CHARGE ? GetDamageB() : GetDamage(), 0.84, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

#ifndef CLIENT_DLL
        if(m_iWeaponState & WPNSTATE_BALROG3_CHARGE)
            Balrog3ExpToLineTargets(vecSrc, vecForward, vecRight, vecUp, vecDir, 8192, m_pPlayer);
#endif

        int flags;
#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), m_iWeaponState & WPNSTATE_BALROG3_CHARGE, FALSE);
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;

        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            KickBack(0.9, 0.475, 0.35, 0.0425, 5, 3, 6);
        else if (m_pPlayer->pev->velocity.Length2D() > 0)
            KickBack(0.5, 0.275, 0.2, 0.03, 3, 2, 10);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            KickBack(0.225, 0.15, 0.1, 0.015, 2, 1, 10);
        else
            KickBack(0.27, 0.177, 0.127, 0.023, 2.28, 1.27, 10);
    }

    void WeaponIdle() override
    {
        m_iChargeShotsFired = 0;
        m_iWeaponState &= ~WPNSTATE_BALROG3_CHARGE;
        Base::WeaponIdle();
    }

    void Reload() override
    {
        m_iWeaponState &= ~WPNSTATE_BALROG3_CHARGE;
        m_iChargeShotsFired = 0;
        Base::Reload();
    }

    float GetDamageB() const
    {
        float flDamage = 23.0f;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = 25.0f;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = 57.0f;
#endif
        return flDamage;
    }

#ifndef CLIENT_DLL
    int Balrog3ExpToLineTargets(Vector vecSrc, Vector vecDirShooting, Vector vecRight, Vector vecUp, Vector vecSpread, float flDistance, CBasePlayer* pPlayer)
    {
        int HitResult = HIT_NONE;

        TraceResult tr;

        Vector vecDir, vecEnd;

        vecDir = vecDirShooting + vecSpread[0] * vecRight + vecSpread[1] * vecUp;
        vecEnd = vecSrc + vecDir * flDistance;

        UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pPlayer->pev), &tr);

        if (tr.flFraction < 1.0)
        {
            HitResult = HIT_WALL;
            CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

            if (pEntity && (pEntity->IsPlayer() || pEntity->Classify() == CLASS_PLAYER_ALLY))
            {
                HitResult = HIT_PLAYER;

                MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
                WRITE_BYTE(TE_TEMPSPRITE);
                WRITE_COORD(tr.vecEndPos.x);
                WRITE_COORD(tr.vecEndPos.y);
                WRITE_COORD(tr.vecEndPos.z);
                WRITE_SHORT(m_iExpIndex);
                WRITE_BYTE(4);
                WRITE_BYTE(255);
                WRITE_BYTE(15);
                MESSAGE_END();
            }
        }
        return HitResult;
    }
#endif
};
LINK_ENTITY_TO_CLASS(weapon_balrog3, CBalrog3)
}