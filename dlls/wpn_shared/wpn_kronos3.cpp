/*
kronos3.cpp
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
#include "weapons/KnifeAttack.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif
    class CKronos3 : public LinkWeaponTemplate< CKronos3,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_P90>::template type,
            TPrecacheEvent,
            TGetDamageDefault,
            TPrimaryAttackRifle
    >
    {
    public:

    static constexpr const char *V_Model = "models/v_kronos3.mdl";
    static constexpr const char *P_Model = "models/p_kronos3.mdl";
    static constexpr const char *W_Model = "models/w_kronos3.mdl";
    static constexpr const char *EventFile = "events/kronos3.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_kronos3";
    static constexpr const char *AnimExtension = "carbine";
    static constexpr int MaxClip = 50;
	enum
	{
        ANIM_IDLE,
        ANIM_IDLE2,
        ANIM_IDLE3,
        ANIM_RELOAD,
        ANIM_RELOAD2,
        ANIM_DRAW,
        ANIM_DRAW2,
        ANIM_SHOOT1,
        ANIM_SHOOT2,
        ANIM_SHOOT3,   
        ANIM_SCAN_ACTIVATE,
        ANIM_ZOOM,
        ANIM_SCAN_DEACTIVATE,
	};
    static constexpr float PlayerMaxSpeed = 230;
    static constexpr float ArmorRatioModifier = 1.5f;
	static constexpr float PrimaryAttackWalkingMiniumSpeed = 195;
    static constexpr const auto &SpreadCalcNotOnGround = 0.3*A;
    static constexpr const auto &SpreadCalcWalking = 0.115*A;
    static constexpr const auto &SpreadCalcDefault = 0.045*A;

    static constexpr const auto& CycleTime1 = 0.085s;
    static constexpr const auto& CycleTime2 = 0.076s;
    duration_t CycleTime = CycleTime1;
    static constexpr int DamageDefault = 40;
    static constexpr int DamageZB = 240;
    static constexpr int DamageZBS = 240;

    static constexpr float AccuracyDefault = 0.25;
    static constexpr float AccuracyMax = 0.9;
    static constexpr auto BulletType = BULLET_PLAYER_57MM;
    /*static constexpr*/ int Penetration = 2;
    KnockbackData KnockBack = { 250.0f, 200.0f, 250.0f, 90.0f, 0.65f };

#define WPNSTATE_KRONOS3_SCAN_ACTIVATE	1
#define WPNSTATE_KRONOS3_ZOOM_ACTIVATE	2
#define KRONOS3_CHARGE_AMMO	4

#ifndef CLIENT_DLL
    std::vector<EHANDLE> m_pEnemylist;
#endif
    time_point_t m_flNextThink;
    bool m_bHit;

    int GetItemInfo(ItemInfo* p) override
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "57mmk";
        p->iMaxAmmo1 = MAX_AMMO_57MM;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
        p->iMaxClip = P90_MAX_CLIP;
        p->iSlot = 0;
        p->iPosition = 8;
        p->iId = m_iId = WEAPON_KRONOS3;
        p->iFlags = 0;
        p->iWeight = P90_WEIGHT;

        return 1;
    }

    void Precache(void) override
    {
        //PRECACHE_SOUND("weapons/kronos3_takeaim.wav");
        //PRECACHE_SOUND("weapons/kronos3_beep.wav");
        PRECACHE_SOUND("weapons/kronos3-1.wav");
        PRECACHE_MODEL("sprites/muzzleflash3.spr");
        Base::Precache();
    }

    BOOL Deploy(void) override
    {
        m_flAccuracy = 0.2;
        m_iShotsFired = 0;

        return DefaultDeploy(V_Model, P_Model, m_iWeaponState & WPNSTATE_KRONOS3_SCAN_ACTIVATE ? ANIM_DRAW2 : ANIM_DRAW, "m249", UseDecrement() != FALSE);
    }


#ifndef CLIENT_DLL
    void FireBullet3CallBack(int iPenetration, Vector vecDir, TraceResult* pTrace)
    {
        if (!m_bHit)
        {
            if (CBaseEntity* pEntity = gMultiDamage.pEntity)
            {
                if (pEntity->IsPlayer() || pEntity->Classify() == CLASS_PLAYER_ALLY)
                {
                    if (g_pGameRules->PlayerRelationship(m_pPlayer, pEntity) != GR_TEAMMATE)
                    {
                        pev->iuser1++;

                        MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
                        WRITE_BYTE(1);
                        WRITE_BYTE(pev->iuser1);
                        MESSAGE_END();

                        m_bHit = true;
                    }
                }
            }
        }

    }
#endif

    void PrimaryAttack() override
    {
        if (m_iWeaponState & WPNSTATE_KRONOS3_ZOOM_ACTIVATE)
            CycleTime = CycleTime2;
        else
            CycleTime = CycleTime1;
        return Base::PrimaryAttack();
    }

    void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_bDelayFire = true;
        m_iShotsFired++;
        m_flAccuracy = (m_iShotsFired * m_iShotsFired * m_iShotsFired) / 175.0 + 0.45;

        if (m_flAccuracy > 1)
            m_flAccuracy = 1;

        if (m_iClip <= 0)
        {
            if (m_fFireOnEmpty)
            {
                PlayEmptySound();

                m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
            }

#ifndef CLIENT_DLL
            if (TheBots)
                TheBots->OnEvent(EVENT_WEAPON_FIRED_ON_EMPTY, m_pPlayer);
#endif

            return;
        }

        m_iClip--;
        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponVolume = BRIGHT_GUN_FLASH;

        m_bHit = m_pPlayer->m_iFOV != 90 ? true : false;

        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecDir;
#ifndef CLIENT_DLL
        float flDamage = GetDamage();
        if (m_iWeaponState & WPNSTATE_KRONOS3_ZOOM_ACTIVATE && !m_pEnemylist.empty())
        {
            PrimaryAttack_InstantDamage();
            SendWeaponAnim(ANIM_SHOOT3, UseDecrement() != FALSE);
            EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/kronos3-1.wav", VOL_NORM, ATTN_NORM, 0, 94 + RANDOM_LONG(0, 15));
        }
        else
        {
            vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_57MM, flDamage, 0.885, m_pPlayer->pev, false, m_pPlayer->random_seed);
            PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), m_iWeaponState & WPNSTATE_KRONOS3_ZOOM_ACTIVATE, m_iWeaponState & WPNSTATE_KRONOS3_SCAN_ACTIVATE);
        }
           
        m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", 0, 0);
#endif

        if (!m_iWeaponState)
        {
            if (pev->iuser1 >= KRONOS3_CHARGE_AMMO)
            {
                SendWeaponAnim(ANIM_SCAN_ACTIVATE, UseDecrement() != FALSE);
                m_iWeaponState |= WPNSTATE_KRONOS3_SCAN_ACTIVATE;
                pev->iuser1 = 0;
            }
        }

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.6s;

        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            KickBack(0.7, 0.4, 0.21, 0.03, 4.0, 2.25, 6);
        else if (m_pPlayer->pev->velocity.Length2D() > 0)
            KickBack(0.7, 0.4, 0.21, 0.03, 4.0, 2.25, 7);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            KickBack(0.45, 0.225, 0.15, 0.01, 2.05, 1.85, 8);
        else
            KickBack(0.5, 0.25, 0.125, 0.01, 3.0, 2.0, 8);
    }


    void SecondaryAttack(void)
    {
        if (m_iWeaponState & WPNSTATE_KRONOS3_ZOOM_ACTIVATE)
        {
#ifndef CLIENT_DLL
            m_pEnemylist.clear();
#endif
            SendWeaponAnim(ANIM_SCAN_DEACTIVATE, UseDecrement() != FALSE);
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.875s;
            m_iWeaponState = 0;
            m_flNextThink = invalid_time_point;
            m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;
            pev->iuser1 = 0;
            return;
        }

        if (m_iWeaponState & WPNSTATE_KRONOS3_SCAN_ACTIVATE)
        {
            SendWeaponAnim(ANIM_ZOOM, UseDecrement() != FALSE);
            m_flNextThink = gpGlobals->time + 0.87s;
            m_pPlayer->m_flNextAttack = 0.87s;
            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.875s;
            m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.875s;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.87s;
        }

    }


    bool PrimaryAttack_CheckTargetAvailable(CBaseEntity* a2, Vector vecAngleDirection)
    {
        const float flRadius = 8192;

        if (!a2->IsPlayer() && a2->Classify() != CLASS_PLAYER_ALLY)
            return false;

        if (!a2->IsAlive())
            return false;

#ifndef CLIENT_DLL
        if (g_pGameRules->PlayerRelationship(m_pPlayer, a2) == GR_TEAMMATE)
            return false;
#endif

        if (a2->pev == m_pPlayer->pev)
            return false;

        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecDelta = (a2->Center() - vecSrc).Normalize();

        UTIL_MakeVectors(vecAngleDirection);

        if (DotProduct(gpGlobals->v_forward, vecDelta) < 0.7)
            return false;

        TraceResult tr;
        UTIL_TraceLine(m_pPlayer->pev->origin, a2->pev->origin, missile, m_pPlayer->edict(), &tr);

        if (tr.flFraction < 1.0)
        {
            if (!tr.pHit)
                return false;

            if (tr.pHit != a2->edict())
                return false;
        }

        return true;
    }

    void PrimaryAttack_FindTargets()
    {
        const float flRadius = 8192;

#ifndef CLIENT_DLL
        m_pEnemylist.erase(std::remove_if(m_pEnemylist.begin(), m_pEnemylist.end(), [=](CBaseEntity* pEntity) {

            if (!pEntity->IsAlive() || bIsWallBetweenEntity(pEntity, m_pPlayer))
                return true;

            UTIL_MakeVectors(m_pPlayer->pev->v_angle);

            Vector vecSrc = m_pPlayer->GetGunPosition();
            Vector vecDelta = (pEntity->Center() - vecSrc).Normalize();

            if (DotProduct(gpGlobals->v_forward, vecDelta) < 0.7)
                return true;

            return false;
            }), m_pEnemylist.end());


        CBaseEntity* pEntity = NULL;
        while ((pEntity = UTIL_FindEntityInSphere(pEntity, m_pPlayer->pev->origin, flRadius)) != nullptr)
        {
            EHANDLE eh;
            eh.Set(pEntity->edict());

            if (std::find(m_pEnemylist.begin(), m_pEnemylist.end(), eh) != m_pEnemylist.end())
                continue;

            if (PrimaryAttack_CheckTargetAvailable(pEntity, m_pPlayer->pev->v_angle))
            {
                m_pEnemylist.push_back(eh);
            }
        }
#endif
    }

    void PrimaryAttack_InstantDamage()
    {
#ifndef CLIENT_DLL
        for (CBaseEntity* pEntity : m_pEnemylist)
        {
            if (!pEntity)
                continue;

            Vector vecDirection = (pEntity->pev->origin - m_pPlayer->pev->origin).Normalize();
            Vector vecSrc = m_pPlayer->GetGunPosition();

            TraceResult tr;
            UTIL_TraceLine(vecSrc, vecDirection * 8192, missile, ENT(m_pPlayer->pev), &tr);

            float flDamage = GetDamageB() / m_pEnemylist.size();

            float flCurrentDistance = tr.flFraction * 8192;
            flDamage *= pow(0.885, flCurrentDistance / 500);

            ClearMultiDamage();
            pEntity->TraceAttack(m_pPlayer->pev, flDamage, vecDirection, &tr, DMG_BULLET);
            ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

            MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, m_pPlayer->pev->origin);
            WRITE_BYTE(TE_BEAMPOINTS);
            WRITE_COORD(vecSrc.x);
            WRITE_COORD(vecSrc.y);
            WRITE_COORD(vecSrc.z);
            WRITE_COORD(pEntity->pev->origin.x);
            WRITE_COORD(pEntity->pev->origin.y);
            WRITE_COORD(pEntity->pev->origin.z);
            WRITE_SHORT(g_sModelIndexSmokeBeam);
            WRITE_BYTE(0);
            WRITE_BYTE(0);
            WRITE_BYTE(10);//Life
            WRITE_BYTE(7);//Width
            WRITE_BYTE(0);//Noise
            WRITE_BYTE(255);
            WRITE_BYTE(255);
            WRITE_BYTE(255);
            WRITE_BYTE(20);
            WRITE_BYTE(0);
            MESSAGE_END();
        }
#endif
    }

    void Reload(void) override
    {
        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            return;

        int iClipSize = 50;
        int j = Q_min(iClipSize - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);

        if (!j)
            return;

        if (m_pPlayer->m_iFOV != 90)
        {
            m_iWeaponState = 0;
            m_flNextThink = invalid_time_point;
            m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;
            pev->iuser1 = 0;
        }


        if (DefaultReload(iClipSize, m_iWeaponState & WPNSTATE_KRONOS3_SCAN_ACTIVATE ? ANIM_RELOAD2 : ANIM_RELOAD, 3.0s))
        {

#ifndef CLIENT_DLL
            m_pEnemylist.clear();
            m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
            m_flAccuracy = 0.2;
            m_iShotsFired = 0;
            m_bDelayFire = false;
        }

    }


    void ItemPostFrame(void)
    {
        if (m_flNextThink != invalid_time_point && m_flNextThink < gpGlobals->time)
        {
            if (m_pPlayer->m_iFOV != 90)
            {
#ifndef CLIENT_DLL
                m_pEnemylist.clear();
#endif
                SendWeaponAnim(ANIM_SCAN_DEACTIVATE, UseDecrement() != FALSE);
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.875s;
                m_iWeaponState = 0;
                m_flNextThink = invalid_time_point;
                m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;
                pev->iuser1 = 0;
            }
            else
            {
#ifndef CLIENT_DLL
                MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->edict());
                WRITE_BYTE(20);
                WRITE_SHORT(5);
                MESSAGE_END();

                if (!m_pPlayer->m_bNightVisionOn)
                    UTIL_ScreenFade(m_pPlayer, Vector(80, 80, 0), 5s, 5s, 80, 0x0002);
#endif
                m_iWeaponState |= WPNSTATE_KRONOS3_ZOOM_ACTIVATE;
                m_flNextThink = gpGlobals->time + 5.0s;
                m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 65;


            }
        }

        if (m_iWeaponState & WPNSTATE_KRONOS3_ZOOM_ACTIVATE)
        {

        if ((m_pPlayer->pev->button & IN_RELOAD) && !m_fInReload && m_flNextPrimaryAttack < UTIL_WeaponTimeBase())
        {
            // reload when reload is pressed, or if no buttons are down and weapon is empty.
            Reload();
            return;
        }

            PrimaryAttack_FindTargets();
            if (m_flNextPrimaryAttack < UTIL_WeaponTimeBase())
                PrimaryAttack();
        }

        return CBasePlayerWeapon::ItemPostFrame();
    }


    void Holster(int skiplocal)
    {
#ifndef CLIENT_DLL
        m_pEnemylist.clear();
#endif
        if (m_iWeaponState & WPNSTATE_KRONOS3_ZOOM_ACTIVATE)
            m_iWeaponState = 0;
        CBasePlayerWeapon::Holster(skiplocal);

    }

    void WeaponIdle(void)
    {
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

        if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
        {
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0s;

            if (m_iWeaponState & WPNSTATE_KRONOS3_SCAN_ACTIVATE)
            {
                if (m_iWeaponState & WPNSTATE_KRONOS3_ZOOM_ACTIVATE)
                    SendWeaponAnim(ANIM_IDLE3, UseDecrement() != FALSE);
                else
                    SendWeaponAnim(ANIM_IDLE2, UseDecrement() != FALSE);
            }
            else
            {
                SendWeaponAnim(ANIM_IDLE, UseDecrement() != FALSE);
            }

        }
    }

    float GetDamageB() const
    {
        float flDamage = 42;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = 252;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = 252;
#endif
        return flDamage;
    }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_57mmk" , 25 }; }
#endif
    float GetArmorRatioModifier() override { return 1.5; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 7, 3, 170 }; }
    		 	
};
LINK_ENTITY_TO_CLASS(weapon_kronos3, CKronos3)
}