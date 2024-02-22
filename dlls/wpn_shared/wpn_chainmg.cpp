/*
chainmg.cpp
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
#include "weapons/knifeattack.h"
#include "wpn_chainmg.h"


#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif
   
    LINK_ENTITY_TO_CLASS(weapon_chainmg, CChainMG)

    void CChainMG::Spawn()
    {
        Base::Spawn();
    }

    void CChainMG::Precache(void)
    {
        PRECACHE_MODEL("sprites/muzzleflash364.spr");
        PRECACHE_MODEL("sprites/muzzleflash365.spr");

        m_iSkillModelIndex[0] = PRECACHE_MODEL("sprites/ef_chainsr_skill.spr");
        m_iSkillModelIndex[1] = PRECACHE_MODEL("sprites/ef_chainsr_skill2.spr");

        Base::Precache();
    }

    void CChainMG::WeaponIdle(void)
    {
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

        if (m_iLastAnim == ANIM_SHOOT)
        {       
            SendWeaponAnim(ANIM_FIRE_AFTER, UseDecrement() != FALSE);
            m_iLastAnim = ANIM_FIRE_AFTER;
            
            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.03s;
            int flags;
#ifdef CLIENT_WEAPONS
            flags = FEV_NOTHOST;
#else
            flags = 0;
#endif
            PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, 2.0, 0.0, 0, 0, FALSE, FALSE);    //FireAfter
        }

        if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
            return;

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;

       
        SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);
        m_iLastAnim = ANIM_IDLE1;
    }

    void CChainMG::Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        if (m_iClip <= 0)
        {
            SendWeaponAnim(ANIM_FIRE_AFTER, UseDecrement() != FALSE);
            m_iLastAnim = ANIM_FIRE_AFTER;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.03s;
            int flags;
#ifdef CLIENT_WEAPONS
            flags = FEV_NOTHOST;
#else
            flags = 0;
#endif
            PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, 2.0, 0.0, 0, 0, FALSE, FALSE);    //FireAfter


            if (m_fFireOnEmpty)
            {
                m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
            }

#ifndef CLIENT_DLL
            if (TheBots != NULL)
            {
                TheBots->OnEvent(EVENT_WEAPON_FIRED_ON_EMPTY, m_pPlayer);
            }
#endif

            return;
        }

        m_flAccuracy = (m_iShotsFired * m_iShotsFired * m_iShotsFired) / AccuracyDiv + AccuracyAdd;

        if (m_flAccuracy > AccuracyLimit)
            m_flAccuracy = AccuracyLimit;

        m_iShotsFired++;
        m_iClip--;
        m_bDelayFire = true;

        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, Penetration, BULLET_PLAYER_762MM, GetDamage(), RangeMod, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

        int flags;
#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
        m_iLastAnim = ANIM_SHOOT;

#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            KickBack(1.0, 0.4, 0.2, 0.15, 3.0, 2.0, 0);
        else if (m_pPlayer->pev->velocity.Length2D() > 0)
            KickBack(0.3, 0.075, 0.05, 0.02, 2.5, 1.5, 0);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            KickBack(0.175, 0.04, 0.03, 0.01, 1.5, 1.0, 0);
        else
            KickBack(0.2, 0.05, 0.035, 0.015, 2.0, 1.25, 0);
    }

    void CChainMG::SecondaryAttack(void)
    {
        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);

        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
     
        CreateEnt(ShadowAT4SpawnAngle);
        CreateEnt(-ShadowAT4SpawnAngle);
#endif
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.7s;
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
    }

#ifndef CLIENT_DLL
    void CChainMG::CreateEnt(float fYaw)
    {
        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
  
        float sinA = sin(fYaw * M_PI / 180);
        float cosA = cos(fYaw * M_PI / 180);
        Vector vecSpawnDir = gpGlobals->v_forward * cosA + gpGlobals->v_right * sinA;
        vecSpawnDir.z = 0;

        Vector vecSpawnSrc = m_pPlayer->pev->origin + vecSpawnDir * ShadowAT4SpawnDist * 39.37;
        vecSpawnSrc.z -= m_pPlayer->pev->flags & FL_DUCKING ? 16 : 32;

        sinA = sin(-fYaw * M_PI / 180);
        cosA = cos(-fYaw * M_PI / 180);
        Vector vecDir = gpGlobals->v_forward * cosA + gpGlobals->v_right * sinA;
        vecDir.z = 0;
        Vector vecAngle;
        VectorAngles(vecDir, vecAngle);

        Vector vecForward = gpGlobals->v_forward;
  
        MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, vecSpawnSrc);
        WRITE_BYTE(TE_TEMPSPRITE);
        WRITE_COORD(vecSpawnSrc.x);
        WRITE_COORD(vecSpawnSrc.y);
        WRITE_COORD(vecSpawnSrc.z);
        WRITE_SHORT(m_iSkillModelIndex[0]);
        WRITE_BYTE(15);
        WRITE_BYTE(255);
        WRITE_BYTE(30);
        MESSAGE_END();

        MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, vecSpawnSrc);
        WRITE_BYTE(TE_TEMPSPRITE);
        WRITE_COORD(vecSpawnSrc.x);
        WRITE_COORD(vecSpawnSrc.y);
        WRITE_COORD(vecSpawnSrc.z);
        WRITE_SHORT(m_iSkillModelIndex[1]);
        WRITE_BYTE(15);
        WRITE_BYTE(255);
        WRITE_BYTE(10);
        MESSAGE_END();

        UTIL_EMIT_SOUND(this->edict(), CHAN_ITEM, "weapons/chainsr_smoke.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

        TraceResult tr;
        Vector vecStart = m_pPlayer->GetGunPosition();

        UTIL_TraceLine(vecStart, vecStart + vecForward * 8192, dont_ignore_monsters, m_pPlayer->edict(), &tr);


        CChainMG_ShadowAT4* pShadow = static_cast<CChainMG_ShadowAT4*>(CBaseEntity::Create("chainmg_shadowat4", vecSpawnSrc, vecAngle, ENT(m_pPlayer->pev)));
        if (pShadow)
        {
            pShadow->Init(m_pPlayer, this, tr.vecEndPos, m_pPlayer->pev->flags & FL_DUCKING ? 3 : 1);
        }
    }
#endif

    void CChainMG::WpnDeathCallBack(CBaseEntity* pVictim, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
    {
#ifndef CLIENT_DLL
        ShadowSearchForEnemy(pVictim, pVictim->pev->origin, 0, flDamage, pVictim->m_LastHitGroup);
#endif

    }

    void CChainMG::ShadowSearchForEnemy(CBaseEntity* pIgnored, Vector vecOrigin, int iType, float flDamage, int iLastHitGroup)
    {
#ifndef CLIENT_DLL
        float flMinDistance = ShadowShootRadius * 39.37;

        CBaseEntity* iIdDetected = NULL;
        CBaseEntity* pEntity = NULL;

        while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecOrigin, flMinDistance)) != NULL)
        {
            if (pEntity->IsDormant())
                continue;

            if (pEntity->Classify() != CLASS_PLAYER_ALLY && !pEntity->IsPlayer())
            {
                if (!FClassnameIs(pEntity->pev, "monster_entity"))
                    continue;
            }

            if (pEntity->pev->takedamage == DAMAGE_NO)
                continue;

            if (pEntity == m_pPlayer)
                continue;

            if (pEntity == pIgnored)
                continue;

            //¶ÓÓÑ²»´¥·¢¼ì²â
            if (g_pGameRules->PlayerRelationship(m_pPlayer, pEntity) == GR_TEAMMATE)
                continue;

            if (UTIL_IsWallBetweenEntity(pEntity, pIgnored))
                continue;

            //iIdDetected = pEntity;

            float flDistance = (pEntity->pev->origin - vecOrigin).Length();
            if (flMinDistance > flDistance)
            {
                flMinDistance = flDistance;
                iIdDetected = pEntity;
            }
        }

        Vector vecDir = (vecOrigin - (iIdDetected ? (iIdDetected->pev->origin) : vecOrigin));
        Vector vecTemp;
        Vector vecScale;
        if (vecDir.Length())
        {
            vecDir = vecDir.Normalize();
            vecTemp = vecDir;

            VectorScale(vecTemp, -32, vecScale);
            VectorAdd(vecOrigin, vecScale, vecOrigin);
        }

        MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, vecOrigin);
        WRITE_BYTE(TE_TEMPSPRITE);
        WRITE_COORD(vecOrigin.x);
        WRITE_COORD(vecOrigin.y);
        WRITE_COORD(vecOrigin.z);
        WRITE_SHORT(m_iSkillModelIndex[0]);
        WRITE_BYTE(15);
        WRITE_BYTE(255);
        WRITE_BYTE(30);
        MESSAGE_END();

        MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
        WRITE_BYTE(TE_TEMPSPRITE);
        WRITE_COORD(vecOrigin.x);
        WRITE_COORD(vecOrigin.y);
        WRITE_COORD(vecOrigin.z);
        WRITE_SHORT(m_iSkillModelIndex[1]);
        WRITE_BYTE(15);
        WRITE_BYTE(255);
        WRITE_BYTE(10);
        MESSAGE_END();

        UTIL_EMIT_SOUND(this->edict(), CHAN_ITEM, "weapons/chainsr_smoke.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

        CChainMG_ShadowChainMG* pShadow = static_cast<CChainMG_ShadowChainMG*>(CBaseEntity::Create("chainmg_shadowchainmg", vecOrigin, g_vecZero, ENT(m_pPlayer->pev)));
        if (pShadow)
        {
            pShadow->Init(m_pPlayer, this, iIdDetected, iLastHitGroup);
        }
  
        //if (iIdDetected)
        //	ClientPrint(m_pPlayer->pev, HUD_PRINTTALK, "Nearest Enemy Searched : %s [Distance:%s m]", STRING(iIdDetected->pev->netname), std::to_string((flMinDistance / 39.37)).c_str());
#endif
    }
 
}