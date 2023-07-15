/*
leapstrikegun.cpp
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
#include "weapons/RadiusDamage.hpp"
#include "weapons/KnifeAttack.h"
#include "wpn_shared/wpn_leapstrikegun.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

    LINK_ENTITY_TO_CLASS(weapon_leapstrikegun, CLeapStrikeGun)

    void CLeapStrikeGun::Precache(void)
    {
        m_iFieldIndex = PRECACHE_MODEL("models/bufffiveseven_field.mdl");
        m_iChargingShotIndex[0] = PRECACHE_MODEL("models/ef_halogun_chargingshot.mdl");
        m_iChargingShotIndex[1] = PRECACHE_MODEL("models/ef_halogun_chargingshot2.mdl");
        m_iExpIndex = PRECACHE_MODEL("sprites/bufffiveseven_exp.spr");
       
        PRECACHE_MODEL("models/v_leapstrikegun_parts.mdl");
        PRECACHE_MODEL("sprites/muzzleflash195.spr");
        PRECACHE_MODEL("sprites/muzzleflash196.spr");
        PRECACHE_MODEL("sprites/muzzleflash197.spr");
        PRECACHE_MODEL("sprites/muzzleflash198.spr");

        PRECACHE_SOUND("weapons/bufffiveseven_blue_skill_start.wav");
        PRECACHE_SOUND("weapons/bufffiveseven_red_skill_start.wav");
        PRECACHE_SOUND("weapons/bufffiveseven-1_exp.wav");
        Base::Precache();
    }

    void CLeapStrikeGun::Spawn()
    {
        m_iDefaultAmmo2 = 0;
        m_flTimeNextCharge = gpGlobals->time;
        m_flTimeFlying = invalid_time_point;
        Base::Spawn();
    }

    int CLeapStrikeGun::ExtractAmmo(CBasePlayerWeapon* pWeapon)
    {
        if (m_iDefaultAmmo2)
        {
            m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iDefaultAmmo2;
            m_iDefaultAmmo2 = 0;
        }
        return CBasePlayerWeapon::ExtractAmmo(pWeapon);
    }

#ifndef CLIENT_DLL
    void CLeapStrikeGun::FireBullet3CallBack(int iPenetration, Vector vecDir, TraceResult* tr)
    {
        Vector vecEnd = tr->vecEndPos - vecDir * 10;
        int iOriginalPenetration = 4;

        if (iOriginalPenetration == iPenetration)
        {
            MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY, vecEnd);
            WRITE_BYTE(TE_TEMPSPRITE);
            WRITE_COORD(vecEnd.x);
            WRITE_COORD(vecEnd.y);
            WRITE_COORD(vecEnd.z);
            WRITE_SHORT(m_iExpIndex);
            WRITE_BYTE(6);
            WRITE_BYTE(255);
            WRITE_BYTE(30);
            MESSAGE_END();

            EMIT_SOUND_DYN(this->edict(), CHAN_ITEM, "weapons/bufffiveseven-1_exp.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);

            CBaseEntity* pEntity = NULL;
            while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecEnd, 3 * 39.37)) != NULL)
            {
                if (pEntity->IsDormant())
                    continue;

                if (pEntity->pev->takedamage == DAMAGE_NO)
                    continue;

                if (pEntity->pev == m_pPlayer->pev)
                    continue;

                pEntity->TraceAttack(m_pPlayer->pev, GetPrimaryAttackDamage(), vecDir, tr, DMG_BULLET | DMG_NEVERGIB);

            }
        }
    }
#endif

    BOOL CLeapStrikeGun::Deploy()
    {
        m_iShotsFired = 0;
        m_flNextReload = invalid_time_point;

        if (!m_iWeaponState)
        {
            m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] += (gpGlobals->time - m_flTimeNextCharge) / 0.1s;
            m_flTimeNextCharge = gpGlobals->time + 0.1s;
            if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] >= 100)
            {
                m_iWeaponState = 1;
                m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 100;
            }
        }
           
        return DefaultDeploy(V_Model, P_Model, m_iWeaponState ? ANIM_DRAWB : ANIM_DRAWA , "onehanded", UseDecrement() != FALSE, 0.1s);
    }


    void CLeapStrikeGun::Reload()
    {
        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            return;

        if (DefaultReload(50, m_iWeaponState ? ANIM_RELOADB : ANIM_RELOADA, 1.9s, 1))
        {
#ifndef CLIENT_DLL         
            m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
            m_flNextReload = gpGlobals->time + 1.9s;
        }
    }

    void CLeapStrikeGun::RadiusAttack2()
    {
#ifndef CLIENT_DLL
        UTIL_MakeVectors(m_pPlayer->pev->v_angle);
        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecAngle = m_pPlayer->pev->angles;
        vecAngle.x = 0;

        struct
        {
            float RadiusDamageRadius;
            int RadiusDamageBitsDamageType;
            bool RadiusDamageCanHeadshot;
            float RadiusDamageAmount;
        } Data;

        UTIL_TempModel({ 0, 0, -8 }, vecAngle, g_vecZero, m_iFieldIndex, 2, 0, 10, 0, 0, 255, kRenderTransAdd, m_pPlayer, false, 0, 0, 256, 0x20000000);

        Data.RadiusDamageRadius = GetRadius(1);
        Data.RadiusDamageBitsDamageType = DMG_BULLET;
        Data.RadiusDamageCanHeadshot = false;
        Data.RadiusDamageAmount = GetRadiusDamage(1);

        std::vector<CBaseEntity*> out;
        RadiusDamage(Data, vecSrc, m_pPlayer, m_pPlayer, std::back_inserter(out));
        for (CBaseEntity* ent : out)
        {
            CBasePlayer* player = dynamic_ent_cast<CBasePlayer*>(ent);
            if (player && player->m_bIsZombie)
            {
                auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
                if (strategy)
                {
                    strategy->SetStunSpeedTime(0.2s, 100.0);
                    strategy->SetStunGravityTime(0.2s);
                }
            }
        }
#endif
    }

    void CLeapStrikeGun::RadiusAttack1()
    {
#ifndef CLIENT_DLL
        UTIL_MakeVectors(m_pPlayer->pev->v_angle);
        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecAngle = m_pPlayer->pev->angles;
        vecAngle.x = 0;

        struct
        {
            float RadiusDamageRadius;
            int RadiusDamageBitsDamageType;
            bool RadiusDamageCanHeadshot;
            float RadiusDamageAmount;
        } Data;

        Vector vecOrigin = m_pPlayer->pev->origin;
        vecOrigin.z -= 16;

        UTIL_TempModel(vecOrigin, g_vecZero, g_vecZero, m_iChargingShotIndex[0], 10, 0, 10, 0, 0, 255, kRenderNormal, NULL, false, 0, 0, 256, 0x20000000);

        Data.RadiusDamageRadius = GetRadius(1);
        Data.RadiusDamageBitsDamageType = DMG_BULLET;
        Data.RadiusDamageCanHeadshot = false;
        Data.RadiusDamageAmount = GetRadiusDamage(1);

        std::vector<CBaseEntity*> out;
        RadiusDamage(Data, vecSrc, m_pPlayer, m_pPlayer, std::back_inserter(out));
        for (CBaseEntity* ent : out)
        {
            CBasePlayer* player = dynamic_ent_cast<CBasePlayer*>(ent);
            if (player && player->m_bIsZombie)
            {
                Vector vecDir = (player->pev->origin - m_pPlayer->pev->origin);
                vecDir.z = 0;

                ApplyKnockbackData(player, player->Center() - vecSrc, { 3000, 3000, 3000, 3000, 1.0f });
                player->pev->velocity.z = +200.0f;
            }
        }
#endif
    }

    void CLeapStrikeGun::SecondaryAttack(void)
    {
        if (!m_iWeaponState)
            m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2s;

        if (m_iWeaponState == 3)
        {
            m_iWeaponState = 4;

            m_pPlayer->pev->velocity = g_vecZero;
            m_pPlayer->pev->velocity.z -= 600;
        }
        else if (m_iWeaponState == 1)
        {
            m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] -= 100;

            m_iWeaponState = 2;

            UTIL_MakeVectors(m_pPlayer->pev->v_angle);

            Vector vecForward = gpGlobals->v_forward * 175;
            vecForward.z += 800;
            m_pPlayer->pev->velocity = vecForward;
            RadiusAttack1();

            m_pPlayer->m_iBuff |= BUFF_LEAPSTRIKEGUN;
            m_pPlayer->m_iBuff |= BUFF_AVOID_INFECT;

            SendWeaponAnim(ANIM_SHOOT_SPECIALB, UseDecrement() != FALSE);

            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;

            EMIT_SOUND_DYN(this->edict(), CHAN_ITEM, "weapons/bufffiveseven_blue_skill_start.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);

            m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2s;
            m_flTimeFlying = gpGlobals->time + 10s;
        }
        else
        {
            //
        }
    }

    void CLeapStrikeGun::PrimaryAttack(void)
    {
        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            RestrictionPistoFire((1.2) * (1 - m_flAccuracy), 0.25s, TRUE);
        else if (m_pPlayer->pev->velocity.Length2D() > 0)
            RestrictionPistoFire((0.185) * (1 - m_flAccuracy), 0.25s, TRUE);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            RestrictionPistoFire((0.095) * (1 - m_flAccuracy), 0.25s, TRUE);
        else
            RestrictionPistoFire((0.3) * (1 - m_flAccuracy), 0.25s, TRUE);
    }


    void CLeapStrikeGun::RestrictionPistoFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        if (m_flLastFire != invalid_time_point)
        {
            m_flAccuracy -= (0.3 - ((gpGlobals->time - m_flLastFire) / 1s)) * 0.325;

            if (m_flAccuracy > 0.9)
                m_flAccuracy = 0.9;
            else if (m_flAccuracy < 0.55)
                m_flAccuracy = 0.55;
        }

        m_flLastFire = gpGlobals->time;

        if (m_iClip <= 0)
        {
            if (m_fFireOnEmpty)
            {
                PlayEmptySound();
                m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
            }
            return;
        }
       
        RadiusAttack2();
        m_iClip--;
        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;

        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 4, BULLET_PLAYER_9MM, GetPrimaryAttackDamage(), 0.8, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

        int flags;
#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), m_iWeaponState ? TRUE : FALSE, FALSE);
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
        m_pPlayer->pev->punchangle.x -= 2;
    }

    void CLeapStrikeGun::DropAttack()
    {
#ifndef CLIENT_DLL
        UTIL_MakeVectors(m_pPlayer->pev->v_angle);
        Vector vecSrc = m_pPlayer->GetGunPosition();

        struct
        {
            float RadiusDamageRadius;
            int RadiusDamageBitsDamageType;
            bool RadiusDamageCanHeadshot;
            float RadiusDamageAmount;
        } Data;

        Vector vecOrigin = m_pPlayer->pev->origin;
        vecOrigin.z -= 16;

        UTIL_TempModel(vecOrigin, g_vecZero, g_vecZero, m_iChargingShotIndex[0], 10, 0, 10, 0, 0, 255, kRenderNormal, NULL, false, 0, 0, 256, 0x20000000);

        Data.RadiusDamageRadius = GetRadius(2);
        Data.RadiusDamageBitsDamageType = DMG_BULLET;
        Data.RadiusDamageCanHeadshot = false;
        Data.RadiusDamageAmount = GetRadiusDamage(2);

        std::vector<CBaseEntity*> out;
        RadiusDamage(Data, vecSrc, m_pPlayer, m_pPlayer, std::back_inserter(out));
        for (CBaseEntity* ent : out)
        {
            CBasePlayer* player = dynamic_ent_cast<CBasePlayer*>(ent);
            if (player && player->m_bIsZombie)
            {
                Vector vecDir = (player->pev->origin - m_pPlayer->pev->origin);
                vecDir.z = 0;

                ApplyKnockbackData(player, player->Center() - vecSrc, { 3000, 3000, 3000, 3000, 1.0f });
                player->pev->velocity.z = +200.0f;
            }
        }

        EMIT_SOUND_DYN(this->edict(), CHAN_ITEM, "weapons/bufffiveseven_red_skill_start.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);
#endif
    }

    void CLeapStrikeGun::FlyThink()
    {
        if (m_flTimeFlying != invalid_time_point && m_flTimeFlying < gpGlobals->time)
        {
           
            SendWeaponAnim(ANIM_CHANGEBA, UseDecrement() != FALSE);

            m_flTimeFlying = invalid_time_point;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.7s;
            m_pPlayer->m_iBuff &= ~BUFF_LEAPSTRIKEGUN;
            m_pPlayer->m_iBuff &= ~BUFF_AVOID_INFECT;
            m_iWeaponState = 0;
            return;
        }

        if (FBitSet(m_pPlayer->pev->flags, FL_ONGROUND) && (m_iWeaponState == 3 || m_iWeaponState == 4))
        {
            if (m_iWeaponState == 4)
            {
                DropAttack();
                m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 30;
            }

            SendWeaponAnim(ANIM_CHANGEBA, UseDecrement() != FALSE);

            m_flTimeFlying = invalid_time_point;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.7s;
            m_pPlayer->m_iBuff &= ~BUFF_LEAPSTRIKEGUN;
            m_pPlayer->m_iBuff &= ~BUFF_AVOID_INFECT;
            m_iWeaponState = 0;
        }

        if (m_iWeaponState == 2)
        {
            if (m_pPlayer->pev->velocity.z < 0)
                m_iWeaponState = 3;
        }

        if (m_iWeaponState == 3)
        {
            UTIL_MakeVectors(m_pPlayer->pev->v_angle);
            m_pPlayer->pev->velocity = gpGlobals->v_forward * 250;

            if (m_pPlayer->pev->button & (IN_BACK | IN_MOVELEFT | IN_MOVERIGHT))
            {
                m_pPlayer->pev->velocity = gpGlobals->v_forward * 50;
            }

            m_pPlayer->pev->velocity[2] = max(m_pPlayer->pev->velocity[2], -10);
        }

    }

    void CLeapStrikeGun::ItemPostFrame()
    {
        FlyThink();

        if (m_fInReload && m_flNextReload != invalid_time_point && m_flNextReload < gpGlobals->time)
        {
            // complete the reload.
            int j = Q_min(50 - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);
#ifndef CLIENT_DLL
            j = m_pPlayer->m_pModStrategy->AdjustDefaultReloadClip(this, j);
#endif

            // Add them to the clip
            m_iClip += j;
            m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= j;
#ifndef CLIENT_DLL
            m_pPlayer->TabulateAmmo();
#endif
            m_fInReload = FALSE;
            m_flNextReload = invalid_time_point;

        }

        if (!m_iWeaponState)
        {
            if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] < 100)
            {
                if (m_flTimeNextCharge <= gpGlobals->time)
                {
                    m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]++;
                    if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] >= 100)
                    {
                        if (m_pPlayer->pev->weaponanim == ANIM_IDLEA)
                        {
                            SendWeaponAnim(ANIM_CHANGEAB, UseDecrement() != FALSE);

                            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.7s;
                        }
                       
                        m_iWeaponState = 1;
                        m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 100;
                    }
                       

                    m_flTimeNextCharge = gpGlobals->time + 0.1s;
                }
            }
            else
                m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 100;
        }
        
        
        if (m_fInReload)
            return;

        return CBasePlayerWeapon::ItemPostFrame();
    }

    void CLeapStrikeGun::WeaponIdle()
    {
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

        if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
            return;

        SendWeaponAnim(m_iWeaponState ? ANIM_IDLEB : ANIM_IDLEA, UseDecrement() != FALSE);

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
        
    }


}