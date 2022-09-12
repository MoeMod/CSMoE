/*
laserfistex.cpp
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
    class CLaserFistEX : public LinkWeaponTemplate< CLaserFistEX,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_AK47>::template type,
            TPrecacheEvent,
            TReloadDefault,
            TGetDamageDefault,
            TPrimaryAttackRifle
    >
    {
    public:
    static constexpr const auto &DefaultReloadTime = 2.2s;
    static constexpr const char *V_Model = "models/v_laserfistex.mdl";
    static constexpr const char *P_Model = "models/p_laserfistex.mdl";
    static constexpr const char *W_Model = "models/w_laserfistex.mdl";
    static constexpr const char *EventFile = "events/laserfistex.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_laserfistex";
    static constexpr int MaxClip = 500;
    enum
    {
        ANIM_IDLE1 = 0,
        ANIM_SHOOTA_EMPTY_LOOP,
        ANIM_SHOOTA_EMPTY_END,
        ANIM_SHOOTA_LOOP,
        ANIM_SHOOTA_END,
        ANIM_SHOOTB_READY,
        ANIM_SHOOTB_LOOP,
        ANIM_SHOOTB_SHOOT,
        ANIM_RELOAD,
        ANIM_DRAW,
	};

#define LASERFISTEX_STATEA_CHARGE_AMMO 70
#define LASERFISTEX_STATEB_CHARGE_AMMO 80

    static constexpr float PlayerMaxSpeed = 260;
    static constexpr float ArmorRatioModifier = 1.2f;

    static constexpr const auto &SpreadCalcNotOnGround = 0.3*A;
    static constexpr const auto &SpreadCalcDefault = 0.03*A;

	static constexpr const auto& CycleTime = 0.1s;
    static constexpr int DamageDefault = 3;
    static constexpr int DamageZB = 37;
    static constexpr int DamageZBS = 120;
   
    static constexpr float AccuracyDefault = 0.2;
    static constexpr float AccuracyMax = 0.8;

    static constexpr auto BulletType = BULLET_PLAYER_9MM;
    /*static constexpr*/ int Penetration = 1;
    KnockbackData KnockBack = { 350.0f, 250.0f, 300.0f, 100.0f, 0.5f };

    int m_iHoleIndex;
    int m_iExpIndex;
    int m_iHitIndex;
    int m_iChargeState;
    time_point_t m_flFinishTime;

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "laserfist";
        p->iMaxAmmo1 = 500;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = 500;
        p->iSlot = 0;
        p->iPosition = 1;
        p->iId = m_iId = WEAPON_LASERFISTEX;
        p->iFlags = 0;
        p->iWeight = AK47_WEIGHT;

        return 1;
    }

    float GetArmorRatioModifier() override { return 1.65; }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_laserfist" , 200 }; }
#endif
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR, 4, 4, 0 }; }

    void Precache(void) override
    {
        PRECACHE_SOUND("weapons/laserfist_shootb_exp.wav");
        PRECACHE_SOUND("weapons/laserfist_idle.wav");

        PRECACHE_MODEL("sprites/ef_laserfistex_laserbeam.spr");
        PRECACHE_MODEL("sprites/ef_laserfistex_charge_left.spr");
        PRECACHE_MODEL("sprites/ef_laserfistex_charge.spr");
        PRECACHE_MODEL("sprites/ef_laserfistex_charge_left2.spr");
        PRECACHE_MODEL("sprites/ef_laserfistex_charge2.spr");
        m_iHoleIndex = PRECACHE_MODEL("models/ef_laserfistex.mdl");

        m_iExpIndex = PRECACHE_MODEL("sprites/ef_laserfist_laser_explosion.spr");
        PRECACHE_MODEL("sprites/ef_laserfistex_laser.spr");
        m_iHitIndex = PRECACHE_MODEL("sprites/ef_laserfist_laser_hit.spr");

        PRECACHE_MODEL("sprites/muzzleflash142.spr");
        PRECACHE_MODEL("sprites/muzzleflash143.spr");
        PRECACHE_MODEL("sprites/muzzleflash144.spr");
        PRECACHE_MODEL("sprites/muzzleflash86.spr");
        PRECACHE_MODEL("sprites/muzzleflash87.spr");
        PRECACHE_MODEL("sprites/muzzleflash91.spr");
        return Base::Precache();
    }


    float GetSecondaryAttackDamage() const
    {
        float flDamage = m_iWeaponState == 2 ? 140.0f : 110.0f;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = m_iWeaponState == 2 ? 4570.0f : 3600.0f;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = m_iWeaponState == 2 ? 18500.0f : 13500.0f;
#endif
        return flDamage;
    }

    BOOL Deploy(void) override
    {
        m_flAccuracy = AccuracyDefault;
        m_iChargeState = 0;
        m_pPlayer->m_bShieldDrawn = false;
   
        if (DefaultDeploy(V_Model, P_Model, ANIM_DRAW, m_pPlayer->m_bIsFemale ? "dualpistols" : "dualpistols_2", UseDecrement() != FALSE, 1.7s, 1.7s))
        {
            m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.7s;
            return TRUE;
        }
        return FALSE;
    }

    void FireRemaining(int& shotsFired, time_point_t& shootTime, BOOL isGlock18)
    {
#ifndef CLIENT_DLL
        if (m_iWeaponState == 1)
            pev->iuser1++;
        else if (!m_iWeaponState)
            pev->iuser1++;
#endif
 
        m_iClip--;
        shotsFired++;

        if (shotsFired != 8)
        {
            shootTime = gpGlobals->time + 0.0125s;
        }
        else
            shootTime = invalid_time_point;

        if (m_iClip < 0)
        {
            m_iClip = 0;
            return;
        }

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, m_fBurstSpread, 8192, 1, BULLET_PLAYER_9MM, GetDamage(), 0.995, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);
    
        int flags;

#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, int(m_pPlayer->pev->punchangle.x * 100), int(m_pPlayer->pev->punchangle.y * 100), shotsFired % 2, FALSE);

        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
       
    }

    void ItemPostFrame()
    {
#ifndef CLIENT_DLL
        if (m_iWeaponState == 1)
        {
            if (pev->iuser1 >= LASERFISTEX_STATEB_CHARGE_AMMO)
            {
                pev->iuser1 = 0;
                m_iWeaponState = pev->skin = 2;
            }
        }
        else if (!m_iWeaponState && pev->iuser1 >= LASERFISTEX_STATEA_CHARGE_AMMO)
        {
            pev->iuser1 = 0;
            m_iWeaponState = pev->skin = 1;
        }
#endif

        if (m_iChargeState && !(m_pPlayer->pev->button & IN_ATTACK2) && m_flNextSecondaryAttack <= 0.0s)
            OnAttack2Released();
            
        return CBasePlayerWeapon::ItemPostFrame();
    }

    void SecondaryAttack()
    {
        if (!m_iWeaponState)
            return;

        if (!m_iChargeState)
        {    
            m_flFinishTime = gpGlobals->time + 1.3s;
            SendWeaponAnim(ANIM_SHOOTB_READY, UseDecrement() != FALSE);
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.3s;
            m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.3s;
            m_iChargeState = 1;

            PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0.833, g_vecZero, g_vecZero, 0.0, 2.0, 0, m_iWeaponState, FALSE, FALSE);

        }
        else if (gpGlobals->time > m_flFinishTime && m_iChargeState != 2)
        {
            m_iChargeState = 2;
            SendWeaponAnim(ANIM_SHOOTB_LOOP, UseDecrement() != FALSE);
            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 9999.0s;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 9999.0s;
        }
    }

    void OnAttack2Released()
    {
        TraceResult tr;

        SendWeaponAnim(ANIM_SHOOTB_SHOOT, UseDecrement() != FALSE);

        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
#ifndef CLIENT_DLL
        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecDir = gpGlobals->v_forward;
        Vector vecEnd = vecSrc + gpGlobals->v_forward * 8192.0;

        UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);
        
        vecEnd = tr.vecEndPos;

        if (tr.flFraction < 1.0)
        {
            CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

            MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY, vecEnd);
            WRITE_BYTE(TE_TEMPSPRITE);
            WRITE_COORD(vecEnd.x);
            WRITE_COORD(vecEnd.y);
            WRITE_COORD(vecEnd.z);
            WRITE_SHORT(m_iHitIndex);
            WRITE_BYTE(4);
            WRITE_BYTE(255);
            WRITE_BYTE(30);
            MESSAGE_END();

            ClearMultiDamage();

            pEntity->TraceAttack(m_pPlayer->pev, GetSecondaryAttackDamage(), gpGlobals->v_forward, &tr, DMG_BULLET | DMG_NEVERGIB);

            ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);
        }

        Vector vecPos = vecEnd - gpGlobals->v_forward * 10;
        MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY, vecPos);
        WRITE_BYTE(TE_TEMPSPRITE);
        WRITE_COORD(vecPos.x);
        WRITE_COORD(vecPos.y);
        WRITE_COORD(vecPos.z);
        WRITE_SHORT(m_iExpIndex);
        WRITE_BYTE(8);
        WRITE_BYTE(255);
        WRITE_BYTE(30);
        MESSAGE_END();

        EMIT_SOUND_DYN(this->edict(), CHAN_ITEM, "weapons/laserfist_shootb_exp.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);

        CBaseEntity* pEntity = NULL;
        while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecEnd, 3 * 39.37)) != NULL)
        {
            if (pEntity->IsDormant())
                continue;

            if (pEntity->pev->takedamage == DAMAGE_NO)
                continue;

            if (pEntity->pev == m_pPlayer->pev)
                continue;

            if (UTIL_IsWallBetweenEntity(pEntity, m_pPlayer))
                continue;

           if (pEntity->edict() != tr.pHit)
                pEntity->TakeDamage(m_pPlayer->pev, m_pPlayer->pev, GetSecondaryAttackDamage(), DMG_NEVERGIB | DMG_BULLET);

            Vector vecDirection = (pEntity->pev->origin - m_pPlayer->pev->origin).Normalize();
            UTIL_AddKickRateBoost(pEntity, vecDirection.Make2D(), 1.0, 6000.0, 500);
        }

        PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, 0, 0.0, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, TRUE);

        if (m_iWeaponState == 2)
        {
            pEntity = NULL;
            while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecEnd, 3 * 39.37)) != NULL)
            {
                if (pEntity->pev->takedamage != DAMAGE_NO)
                {
                    if (pEntity->IsBSPModel())
                        continue;

                    if (pEntity == m_pPlayer)
                        continue;

                    if (UTIL_IsWallBetweenEntity(pEntity, m_pPlayer))
                        continue;

                    CBasePlayer* player = dynamic_ent_cast<CBasePlayer*>(pEntity);
                    if (player && player->m_bIsZombie)
                    {
                        Vector vecDirection = (pEntity->pev->origin - m_pPlayer->pev->origin).Normalize();
                        UTIL_AddKickRateBoost(pEntity, vecDirection.Make2D(), player->m_flVelocityModifier, 3000.0, 200);
                    }
                }
            }

            UTIL_TempModel({ 0, 0, -32 }, g_vecZero, g_vecZero, m_iHoleIndex, 2, 1, 10, false, 0, 255, kRenderTransAdd, m_pPlayer, false, 0, 0, 255, 0);
        }
#endif
        pev->iuser1 = 0;
        m_iChargeState = 0;
        m_iWeaponState = pev->skin = 0;
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.7s;
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.7s;
    }

    void WeaponIdle(void)
    {
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

        if (m_pPlayer->pev->weaponanim == ANIM_SHOOTA_LOOP)
        {
            SendWeaponAnim(ANIM_SHOOTA_END, UseDecrement() != FALSE);

            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
        }
        else if (m_pPlayer->pev->weaponanim == ANIM_SHOOTA_EMPTY_LOOP)
        {
            SendWeaponAnim(ANIM_SHOOTA_EMPTY_END, UseDecrement() != FALSE);

            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
        }

        if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
            return;

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;

        SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);
    }

    void KickBackCustom(float up_base, float lateral_base, float up_modifier, float lateral_modifier, float up_max, float lateral_max, int direction_change)
    {
        float up, lateral;
        int iRan;

        up = ((rand() % 10) * up_base) * 0.1;
        lateral = ((rand() % 10) * lateral_base) * 0.1;

        if (m_iShotsFired != 1)
        {
            up += (rand() % (m_iShotsFired * 2)) * up_modifier;
            lateral += (rand() % (m_iShotsFired * 2)) * lateral_modifier;
        }

        iRan = rand();

        if (!iRan || iRan % 2)
        {
            m_pPlayer->pev->punchangle.x += up;

            if (m_pPlayer->pev->punchangle.x > up_max)
                m_pPlayer->pev->punchangle.x = up_max;
        }
        else
        {
            m_pPlayer->pev->punchangle.x -= up;

            if (m_pPlayer->pev->punchangle.x < -up_max)
                m_pPlayer->pev->punchangle.x = -up_max;
        }

        if (rand() % 2 == 1)
        {
            m_pPlayer->pev->punchangle.y += lateral;

            if (m_pPlayer->pev->punchangle.y > lateral_max)
                m_pPlayer->pev->punchangle.y = lateral_max;
        }
        else
        {
            m_pPlayer->pev->punchangle.y -= lateral;

            if (m_pPlayer->pev->punchangle.y < -lateral_max)
                m_pPlayer->pev->punchangle.y = -lateral_max;
        }
    }

    void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
#ifndef CLIENT_DLL
        if (m_iWeaponState == 1)
            pev->iuser1++;
        else if(!m_iWeaponState)
            pev->iuser1++;
#endif
        if (m_iWeaponState && m_iChargeState)
            return;

        m_iFamasShotsFired = 0;

        m_bDelayFire = true;
        m_iShotsFired++;
        m_flAccuracy = ((float)(m_iShotsFired * m_iShotsFired) / 200.0) + 0.2;

        if (m_flAccuracy > 0.8)
            m_flAccuracy = 0.8;

        if(m_iClip > 0)
            m_fBurstSpread = flSpread;
        m_iFamasShotsFired++;
        m_flFamasShoot = gpGlobals->time + 0.0125s;

        SendWeaponAnim(m_iClip > 0 ? ANIM_SHOOTA_LOOP : ANIM_SHOOTA_EMPTY_LOOP, UseDecrement() != FALSE);

        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.1s;

         if (m_iClip <= 0)
            return;

        m_iClip--;
        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 1, BULLET_PLAYER_9MM, GetDamage(), 0.995, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

        int flags;
#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), m_iShotsFired % 2, FALSE);
      
#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif

        if (m_pPlayer->pev->velocity.Length2D() > 0)
            KickBackCustom(0.15, 0.1, 0.015, 0.01, 1.5, 5.5, 0);
        else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            KickBackCustom(0.3, 0.21, 0.03, 0.021, 3, 10, 0);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            KickBackCustom(0.08, 0.05, 0.008, 0.005, 0.8, 4.8, 0);
        else
            KickBackCustom(0.1, 0.07, 0.01, 0.007, 1, 5, 0);
    }

};
LINK_ENTITY_TO_CLASS(weapon_laserfistex, CLaserFistEX)
}