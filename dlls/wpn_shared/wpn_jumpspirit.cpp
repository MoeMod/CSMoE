/*
wpn_cannon.h - Black Dragon Cannon from CSO
Copyright (C) 2019 Moemod Hyakuya

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
#include "wpn_jumpspirit.h"
#ifndef CLIENT_DLL
#include "gamemode/mod_zb1.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

#ifndef CLIENT_DLL
    LINK_ENTITY_TO_CLASS(jumpspirit_bird, CJumpSpiritBird)

    void CJumpSpiritBird::UpdateOnRemove(void)
    {
        if (m_pWeapon)
            m_pWeapon->m_pBird = NULL;

        CBaseEntity::UpdateOnRemove();
    }

    void CJumpSpiritBird::RemoveBird()
    {
        SetThink(nullptr);
        UTIL_Remove(this);
    }

    void CJumpSpiritBird::Spawn(void)
    {       
        Precache();
        pev->classname = MAKE_STRING("jumpspirit_bird");
        pev->solid = SOLID_NOT;
        pev->movetype = MOVETYPE_NOCLIP;
        pev->takedamage = DAMAGE_NO;
        pev->rendermode = kRenderTransTexture;
        pev->renderamt = 255;

        SET_MODEL(ENT(pev), "models/ef_jumpspirit_summon.mdl");

        pev->frame = 0;
        pev->sequence = 0;

        ResetSequenceInfo();

        SetThink(&CJumpSpiritBird::BirdThink);
        pev->nextthink = gpGlobals->time + 0.1s;
        m_flNextAttack = gpGlobals->time;
    }

    void CJumpSpiritBird::Precache(void)
    {
        PRECACHE_MODEL("models/ef_jumpspirit_summon.mdl");
        m_iBeamIndex = PRECACHE_MODEL("sprites/ef_laserfist_laser.spr");

    }

    int CJumpSpiritBird::GetWeaponsId(void)
    {
        return WEAPON_JUMPSPIRIT;
    }

    void CJumpSpiritBird::Init(CBasePlayer* pOwner, CJumpSpirit* pWeapon)
    {
        m_pOwner = pOwner;
        m_pWeapon = pWeapon;
    }

    void CJumpSpiritBird::FindTarget()
    {   
        Vector vecSrc, vecEnd;

        vecSrc = pev->origin;
        CBaseEntity* pEntity = nullptr;
        m_vecTarget.clear();
        TraceResult tr;
        float Dis = (m_pWeapon->m_flTimeAModeFullCharge < gpGlobals->time ? SentryModeRadius : (SentryModeRadius + SentryModeRadiusMod)) * 39.37;
        while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, Dis)) != nullptr)
        {
            if (pEntity->edict() == pev->owner)
                continue;

            if (pEntity->IsDormant())
                continue;

            if (pEntity->pev->takedamage == DAMAGE_NO)
                continue;

            if (pEntity->Classify() != CLASS_PLAYER_ALLY && !pEntity->IsPlayer())
                continue;

            vecEnd = pEntity->BodyTarget(vecSrc);

            UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pev), &tr);

            if (tr.flFraction < 1.0 && tr.pHit != ENT(pEntity->pev))
                continue;

            float flDelta = (vecSrc - vecEnd).Length();

            EHANDLE eh;
            eh.Set(pEntity->edict());

            m_vecTarget.push_back(std::make_pair(flDelta, eh));

        }

        if (m_vecTarget.empty())
            return;

        std::sort(m_vecTarget.begin(), m_vecTarget.end(), [](const std::pair<float, CBaseEntity*>& a, const std::pair<float, CBaseEntity*>& b) {return a.first < b.first; });
    }

    void CJumpSpiritBird::BirdThink(void)
    {
        pev->nextthink = gpGlobals->time + 0.01s;

        if (!m_pOwner)
        {
            RemoveBird();
        }

        UTIL_SetOrigin(pev, m_pOwner->pev->origin + Vector(0, 0, 2));

        pev->angles = m_pOwner->pev->angles;
        pev->angles.x = 0;

        if (m_flNextAttack < gpGlobals->time)
        {          
            FindTarget();

            if (m_vecTarget.empty())
                return;

            for (int i = 0; i < m_vecTarget.size(); i++)
            {
                if (i >= SentryModeMaxTarget)
                    break;

                CBaseEntity* pTarget = m_vecTarget.at(i).second;

                if (!m_vecTarget.at(i).second.Get() || !pTarget)
                    continue;

                TraceResult tr;

                Vector vecEnd = pTarget->BodyTarget(pev->origin);

                UTIL_TraceLine(pev->origin, vecEnd, dont_ignore_monsters, pev->owner, &tr);

                ClearMultiDamage();

                pTarget->TraceAttack(m_pOwner->pev, pev->dmg, pev->velocity.Normalize(), &tr, DMG_BULLET | DMG_NEVERGIB);

                ApplyMultiDamage(pev, m_pOwner->pev);

                if (m_pWeapon->m_flTimeAModeFullCharge < gpGlobals->time)
                {
                    m_flNextAttack = gpGlobals->time + SentryModeInterval;
                }
                else
                {
                    m_flNextAttack = gpGlobals->time + SentryModeIntervalMod;
                }

                Vector vecStart = pev->origin;
                vecStart.z += 50;

                UTIL_BeamTracer(vecStart, vecEnd, m_iBeamIndex, 0, 0, 50, 0, 10, 255, 255, 255, 40, 20, 255, 0);
            }
        }
     
    }

    LINK_ENTITY_TO_CLASS(jumpspirit_field, CJumpSpiritField)

    void CJumpSpiritField::Spawn(void)
    {
        Precache();

        pev->movetype = MOVETYPE_FLY;
        pev->solid = SOLID_BBOX;
        pev->takedamage = DAMAGE_NO;

        SET_MODEL(ENT(pev), "models/ef_jumpspirit_field.mdl");
        UTIL_SetSize(pev, g_vecZero, g_vecZero);
        UTIL_SetOrigin(pev, pev->origin);

        pev->frame = 0;
        pev->sequence = 0;

        ResetSequenceInfo();


        SetThink(&CJumpSpiritField::FieldThink);
        //SetTouch(&CJumpSpiritField::FieldTouch);

        pev->nextthink = gpGlobals->time + 0.1s;
        m_fLaunchTime = gpGlobals->time;
    }

    void CJumpSpiritField::Precache(void)
    {
        PRECACHE_MODEL("models/ef_jumpspirit_projectile.mdl");

        m_iBeamIndex = PRECACHE_MODEL("sprites/ef_laserfist_laser.spr");
        m_iJumpFXIndex = PRECACHE_MODEL("models/ef_jumpspirit_jump.mdl");
    }

    int CJumpSpiritField::GetWeaponsId(void)
    {
        return WEAPON_JUMPSPIRIT;
    }

    void CJumpSpiritField::RemoveField()
    {
        SetThink(NULL);
        SetTouch(NULL);

        UTIL_Remove(this);
    }

    void CJumpSpiritField::FindTarget()
    {
        Vector vecSrc, vecEnd;

        vecSrc = pev->origin;
        CBaseEntity* pEntity = nullptr;

        TraceResult tr;
        m_vecTarget.clear();
        float Dis = (m_pWeapon->m_flTimeAModeFullCharge < gpGlobals->time ? SentryModeRadius : (SentryModeRadius + SentryModeRadiusMod)) * 39.37;
        while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, Dis)) != nullptr)
        {
            if (pEntity->edict() == pev->owner)
                continue;

            if (pEntity->IsDormant())
                continue;

            if (pEntity->pev->takedamage == DAMAGE_NO)
                continue;

            if (pEntity->Classify() != CLASS_PLAYER_ALLY && !pEntity->IsPlayer())
                continue;

            vecEnd = pEntity->BodyTarget(vecSrc);

            UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pev), &tr);

            if (tr.flFraction < 1.0 && tr.pHit != ENT(pEntity->pev))
                continue;

            float flDelta = (vecSrc - vecEnd).Length();

            EHANDLE eh;
            eh.Set(pEntity->edict());

            m_vecTarget.push_back(std::make_pair(flDelta, eh));

        }

        if (m_vecTarget.empty())
            return;

        std::sort(m_vecTarget.begin(), m_vecTarget.end(), [](const std::pair<float, CBaseEntity*>& a, const std::pair<float, CBaseEntity*>& b) {return a.first < b.first; });
      
    }

    bool CJumpSpiritField::CheckEntInBox()
    {
        bool bRemove = false;
        CBaseEntity* pList[100];
        TraceResult tr;
        Vector vecStart = pev->origin + Vector(0.0, 0.0, 1.0);

        const float flRange = BirdKnockbackRadius * 39.37;
        int iCount = UTIL_EntitiesInBox(pList, 100, vecStart - Vector(flRange, flRange, flRange), vecStart + Vector(flRange, flRange, flRange), FL_CLIENT | FL_MONSTER);

        for (int i = 0; i < iCount; i++)
        {
            CBaseEntity* pEntity = pList[i];

            if (pEntity == this)
                continue;

            if (pEntity->pev->takedamage == DAMAGE_NO)
                continue;

            if (pEntity->edict() == pev->owner)
            {
                if (GetJumpSpeed() > pEntity->pev->velocity.z)
                    pEntity->pev->velocity.z = GetJumpSpeed();
                else
                    pEntity->pev->velocity.z += GetJumpSpeed();
                bRemove = true;
                continue;
            }

            if (g_pGameRules->PlayerRelationship(m_pOwner, pEntity) == GR_TEAMMATE)
                continue;

            UTIL_TraceLine(vecStart, pEntity->BodyTarget(vecStart), dont_ignore_monsters, edict(), &tr);

            if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
                continue;

            if (pEntity->IsPlayer() || pEntity->Classify() == CLASS_PLAYER_ALLY)
                bRemove = true;

            ClearMultiDamage();

            pEntity->TraceAttack(m_pOwner->pev, pev->dmg, (pEntity->BodyTarget(vecStart) - pev->origin).Normalize(), &tr, DMG_BULLET | DMG_NEVERGIB);

            ApplyMultiDamage(pev, m_pOwner->pev);

            if (pEntity->IsPlayer())
            {
                CBasePlayer* player = dynamic_cast<CBasePlayer*>(pEntity);

                Vector vecDir = (player->pev->origin - pev->origin).Normalize();
                vecDir.z = 0;

                player->m_flPushBoost = BirdKnockbackSide;
                player->m_flPushBoostZ = BirdKnockbackUp;
                player->m_vecPushDirection = vecDir;
                player->m_flPushDuration = gpGlobals->time + BirdKnockbackDuration;
            }
        }

        return bRemove;
    }

    void CJumpSpiritField::FieldThink()
    {
        if (gpGlobals->time >= m_fLaunchTime + BirdActivateTime)
        {
            RemoveField();
            return;
        }

        pev->nextthink = gpGlobals->time + 0.1s;

        if (!m_pOwner)
        {
            RemoveField();
        }
 
        if (CheckEntInBox())
        {
            UTIL_TempModel(pev->origin, pev->angles, g_vecZero, m_iJumpFXIndex, 20, 0, 10, true, 16, 255, kRenderTransTexture, NULL, false, 0, 10, 256, 0x20000000);

            UTIL_EMIT_SOUND(this->edict(), CHAN_ITEM, "weapons/jumpspirit_jump1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM, 1);

            SetThink(NULL);
            SetTouch(NULL);

            UTIL_Remove(this);
        }
       
        if (m_flNextAttack < gpGlobals->time)
        {
            FindTarget();

            if (m_vecTarget.empty())
                return;

            for (int i = 0; i < m_vecTarget.size(); i++)
            {
                if (i >= SentryModeMaxTarget)
                    break;

                CBaseEntity* pTarget = m_vecTarget.at(i).second;

                if (!m_vecTarget.at(i).second.Get() || !pTarget)
                    continue;

                TraceResult tr;

                Vector vecEnd = pTarget->BodyTarget(pev->origin);

                UTIL_TraceLine(pev->origin, vecEnd, dont_ignore_monsters, pev->owner, &tr);

                ClearMultiDamage();

                pTarget->TraceAttack(m_pOwner->pev, pev->dmg, pev->velocity.Normalize(), &tr, DMG_BULLET | DMG_NEVERGIB);

                ApplyMultiDamage(pev, m_pOwner->pev);

                Vector vecStart = pev->origin;
                vecStart.z += 50;

                UTIL_BeamTracer(vecStart, vecEnd, m_iBeamIndex, 0, 0, 50, 0, 10, 255, 255, 255, 40, 20, 255, 0);

                if (m_pWeapon->m_flTimeAModeFullCharge < gpGlobals->time)
                {
                    m_flNextAttack = gpGlobals->time + SentryModeInterval;
                }
                else
                {
                    m_flNextAttack = gpGlobals->time + SentryModeIntervalMod;
                }
            }
        }
    }

    void CJumpSpiritField::Init(CBasePlayer* pOwner, CJumpSpirit* pWeapon)
    {
        m_pOwner = pOwner;
        m_pWeapon = pWeapon;
    }

    LINK_ENTITY_TO_CLASS(jumpspirit_projectile, CJumpSpiritProjectile)

    void CJumpSpiritProjectile::Spawn(void)
    {
        Precache();

        pev->movetype = MOVETYPE_FLY;
        pev->solid = SOLID_BBOX;

        SET_MODEL(ENT(pev), "models/ef_jumpspirit_projectile.mdl");
        UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));
        UTIL_SetOrigin(pev, pev->origin);

        pev->frame = 0;
        pev->sequence = 0;

        ResetSequenceInfo();


        SetThink(&CJumpSpiritProjectile::ProjectileThink);
        SetTouch(&CJumpSpiritProjectile::ProjectileTouch);

        pev->nextthink = gpGlobals->time + 0.1s;
        m_fLaunchTime = gpGlobals->time + BirdFlyTime;
  
    }

    void CJumpSpiritProjectile::Precache(void)
    {
        PRECACHE_MODEL("models/ef_jumpspirit_projectile.mdl");

        m_iJumpFXIndex = PRECACHE_MODEL("models/ef_jumpspirit_jump.mdl");
    }

    int CJumpSpiritProjectile::GetWeaponsId(void)
    {
        return WEAPON_JUMPSPIRIT;
    }

    void CJumpSpiritProjectile::ProjectileThink()
    {
        if (!IsInWorld() || m_fLaunchTime < gpGlobals->time)
        {
            SetThink(NULL);
            SetTouch(NULL);

            UTIL_Remove(this);
            return;
        }

        pev->nextthink = gpGlobals->time + 0.1s;
    }

    void CJumpSpiritProjectile::ProjectileTouch(CBaseEntity* pOther)
    {
        if (!pOther)
            return;

        TraceResult tr = UTIL_GetGlobalTrace();
        Vector vecDir;

        CBasePlayer* pOwner = (CBasePlayer*)GET_PRIVATE(pev->owner);
     
        if (!pOwner || FNullEnt(pOwner->edict()) || !pOwner->IsPlayer() || pOwner->m_bIsZombie)
        {
            SetTouch(NULL);
            SetThink(NULL);

            UTIL_Remove(this);
            return;
        }

        if (!pOther->IsDormant() && pOther->pev->takedamage != DAMAGE_NO)
        {     
            ClearMultiDamage();

            pOther->TraceAttack(pOwner->pev, pev->dmg, pev->velocity.Normalize(), &tr, DMG_BULLET | DMG_NEVERGIB);

            ApplyMultiDamage(pev, pOwner->pev);

            if (pOther->IsPlayer())
            {
                CBasePlayer* player = dynamic_cast<CBasePlayer*>(pOther);

                vecDir = (player->pev->origin - pOwner->GetGunPosition()).Normalize();
                vecDir.z = 0;
         
                player->m_flPushBoost = BirdKnockbackSide;
                player->m_flPushBoostZ = BirdKnockbackUp;
                player->m_vecPushDirection = vecDir;
                player->m_flPushDuration = gpGlobals->time + BirdKnockbackDuration;
            }
        }
 
        if (pOther->IsPlayer() || pOther->Classify() == CLASS_PLAYER_ALLY)
        {
            CBaseEntity* pEntity = NULL;
            Vector vecEnd;
            float flRadius = BirdKnockbackRadius * 39.37;
            while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, flRadius)) != NULL)
            {
                if (pEntity->edict() == pev->owner)
                    continue;

                if (pEntity->IsDormant())
                    continue;

                if (pEntity->pev->takedamage == DAMAGE_NO)
                    continue;

                if (pEntity->Classify() == CLASS_NONE)
                    continue;

                if (g_pGameRules->PlayerRelationship(pOwner, pEntity) == GR_TEAMMATE)
                    continue;

                vecEnd = pEntity->BodyTarget(pev->origin);

                UTIL_TraceLine(pev->origin, vecEnd, ignore_monsters, pev->owner, &tr);

                if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
                    continue;

                ClearMultiDamage();

                pOther->TraceAttack(pOwner->pev, pev->dmg, (vecEnd - pev->origin).Normalize(), &tr, DMG_BULLET | DMG_NEVERGIB);

                ApplyMultiDamage(pev, pOwner->pev);

                if (pOther->IsPlayer())
                {
                    CBasePlayer* player = dynamic_cast<CBasePlayer*>(pOther);

                    vecDir = (player->pev->origin - pOwner->GetGunPosition()).Normalize();
                    vecDir.z = 0;

                    player->m_flPushBoost = BirdKnockbackSide;
                    player->m_flPushBoostZ = BirdKnockbackUp;
                    player->m_vecPushDirection = vecDir;
                    player->m_flPushDuration = gpGlobals->time + BirdKnockbackDuration;
                }
            }

            UTIL_TempModel(pev->origin, pev->angles, g_vecZero, m_iJumpFXIndex, 20, 0, 10, true, 16, 255, kRenderTransTexture, NULL, false, 0, 10, 256, 0x20000000);
        }
        else
        {
            UTIL_MakeVectors(pev->angles);

            Vector vecForward = gpGlobals->v_forward;
            vecForward.z = 0;
            Vector vecAngle = UTIL_VecToAngles(vecForward);
            vecForward = pev->velocity.Normalize();
            CJumpSpiritField* pField = (CJumpSpiritField*)CBaseEntity::Create("jumpspirit_field", pev->origin - vecForward * 5, vecAngle, pev->owner);

            if (pField)
            {
                pField->pev->dmg = pev->dmg;
                pField->Init(m_pOwner, m_pWeapon);
            }
        }
       
        SetThink(nullptr);
        SetTouch(nullptr);
        UTIL_Remove(this);
    }

    void CJumpSpiritProjectile::Init(CBasePlayer* pOwner, CJumpSpirit* pWeapon)
    {
        m_pOwner = pOwner;
        m_pWeapon = pWeapon;
    }

#endif

    LINK_ENTITY_TO_CLASS(weapon_jumpspirit, CJumpSpirit)

    void CJumpSpirit::Spawn()
    {
        ResetChargeTimer();
        Base::Spawn();
    }

    void CJumpSpirit::Precache(void)
    {
        m_iJumpFXIndex = PRECACHE_MODEL("models/ef_jumpspirit_jump.mdl");
     
        PRECACHE_MODEL("sprites/ef_jumpspirit_body.spr");
        PRECACHE_MODEL("sprites/ef_jumpspirit_cristal.spr");
        PRECACHE_MODEL("sprites/ef_jumpspirit_hit.spr");
        PRECACHE_MODEL("sprites/ef_jumpspirit_jump.spr");
        PRECACHE_MODEL("sprites/ef_jumpspirit_star.spr");
        PRECACHE_MODEL("sprites/ef_jumpspirit_summon.spr");
        PRECACHE_MODEL("sprites/ef_jumpspirit_summon_field.spr");
        PRECACHE_MODEL("sprites/ef_jumpspirit_wind.spr");

        PRECACHE_MODEL("sprites/muzzleflash372.spr");
 
        return Base::Precache();
    }

    void CJumpSpirit::RemoveBird()
    {
#ifndef CLIENT_DLL
        if (m_pBird)
        {
            m_pBird->RemoveBird();
        }
#endif
    }

    void CJumpSpirit::Holster(int skiplocal)
    {
        RemoveBird();
        ResetChargeTimer();
        CBasePlayerWeapon::Holster();
    }

    void CJumpSpirit::UpdateOnRemove(void)
    {
#ifndef CLIENT_DLL
        if (m_pBird)
        {
            if (m_pBird->m_pWeapon)
                m_pBird->m_pWeapon = NULL;

            RemoveBird();
        }

        CBaseEntity::UpdateOnRemove();
#endif
    }

    int CJumpSpirit::ExtractAmmo(CBasePlayerWeapon* pWeapon)
    {
        if (pev->iuser1)
        {
            if (m_pPlayer)
                m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = pev->iuser1;
        }
        return CBasePlayerWeapon::ExtractAmmo(pWeapon);
    }

    BOOL CJumpSpirit::Deploy()
    {
        m_flAccuracy = 0.99;
        m_flNextReload = invalid_time_point;


        if (DefaultDeploy(V_Model, P_Model, ANIM_DRAW, "onehanded", UseDecrement() != FALSE, 0.1s, 1.0s))
        {

#ifndef CLIENT_DLL
            RemoveBird();

            m_pBird = static_cast<CJumpSpiritBird*>(CBaseEntity::Create("jumpspirit_bird", g_vecZero, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev)));
            if (m_pBird)
            {
                m_pBird->pev->dmg = GetSentryModeDamage();
                m_pBird->Init(m_pPlayer, this);
            }
#endif
            ResetChargeTimer();
            m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.25s;

            return TRUE;
        }
        return FALSE;
    }

    void CJumpSpirit::PrimaryAttack()
    {
        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            JumpSpiritFire(0.11 * m_flAccuracy, 0.22s, FALSE);
        else if (m_pPlayer->pev->velocity.Length2D() > 0)
            JumpSpiritFire(0.05 * m_flAccuracy, 0.22s, FALSE);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            JumpSpiritFire(0.02 * m_flAccuracy, 0.22s, FALSE);
        else
            JumpSpiritFire(0.025 * m_flAccuracy, 0.22s, FALSE);

        if (m_flTimeCheckAModeCharge < gpGlobals->time)
        {
            switch (m_iAModeState)
            {
            case 0:
            {
                m_iAModeState = 1;
                m_flTimeCheckAModeCharge = gpGlobals->time + AModeEnergyChargeInterval;
                break;
            }
            case 1:
            {
                m_flTimeAModeFullCharge = gpGlobals->time + AModeMaxEnergy;
                m_flTimeCheckAModeCharge = gpGlobals->time + AModeEnergyChargeInterval;
                break;
            }  
            default:
                break;
            }
        }


    }

    void CJumpSpirit::JumpSpiritFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        if (m_flLastFire != invalid_time_point)
        {
            m_flAccuracy -= (0.2 - ((gpGlobals->time - m_flLastFire) / 1s)) * 0.25;

            if (m_flAccuracy > 0.99)
                m_flAccuracy = 0.99;
            else if (m_flAccuracy < 0.8)
                m_flAccuracy = 0.8;
        }

        m_flLastFire = gpGlobals->time;

        if (m_iClip <= 0)
        {
            if (m_fFireOnEmpty)
            {
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

        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 4096, 2, BULLET_PLAYER_9MM, GetPrimaryAttackDamage(), 0.85, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

        int flags;
#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), 0, FALSE);
        m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
        m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + BModeDelayAfterAMode;
#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;

        m_pPlayer->pev->punchangle.x -= 1.2;
    }


    void CJumpSpirit::OnAttack2Released()
    {

        if (m_iWeaponState > 1)
        {
            ShootProjectile();

            m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + BModeDelayAfterBird;

            m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = pev->iuser1 -= BModeBirdUseEnergy;
        }
        else
        {
          
#ifndef CLIENT_DLL
            UTIL_MakeVectors(m_pPlayer->pev->v_angle);

            Vector vecForward = gpGlobals->v_forward;
            vecForward.z = 0;
            Vector vecAngle = UTIL_VecToAngles(vecForward);

            UTIL_TempModel( { 0, 0, -32 }, vecAngle, g_vecZero, m_iJumpFXIndex, 20, 0, 10, true, 16, 255, kRenderTransTexture, m_pPlayer, false, 0, 10, 256, 0x20000000);
#endif
            if (GetJumpSpeed() > m_pPlayer->pev->velocity.z)
                m_pPlayer->pev->velocity.z = GetJumpSpeed();
            else
                m_pPlayer->pev->velocity.z += GetJumpSpeed();

            m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + BModeDelayAfterJump;

            m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = pev->iuser1 -= BModeJumpUseEnergy;

            SendWeaponAnim(ANIM_JUMP);

            UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/jumpspirit_jump2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM, 1);
        }

        ResetChargeTimer();

        m_iWeaponState = 0;     
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
    }

    void CJumpSpirit::ShootProjectile()
    {
        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;

        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecDir = m_pPlayer->FireBullets3(vecSrc - gpGlobals->v_up * 2.0, gpGlobals->v_forward, 0, 8192.0, 0, BULLET_PLAYER_556MM, 0, 0.0, m_pPlayer->pev, false, m_pPlayer->random_seed);

        Vector vecProjDir = gpGlobals->v_forward + gpGlobals->v_right * vecDir.x + gpGlobals->v_up * vecDir.y;

        CJumpSpiritProjectile* pProjectile = (CJumpSpiritProjectile*)CBaseEntity::Create("jumpspirit_projectile",
            m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 16.0 + gpGlobals->v_right * 8.0 + gpGlobals->v_up * -8.0,
            m_pPlayer->pev->v_angle,
            m_pPlayer->edict());

        if (pProjectile)
        {
            pProjectile->pev->velocity = vecProjDir * BirdSpeed;
            pProjectile->pev->dmg = GetSentryModeDamage();
            pProjectile->Init(m_pPlayer, this);
        }
#endif

        int flags;
#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, 0.0, 0.0, 0, 0, FALSE, TRUE);

    }

    void CJumpSpirit::ResetChargeTimer()
    {
        if (pev->iuser1 != BModeMaxEnergy && m_flTimeNextCharge == invalid_time_point)
            m_flTimeNextCharge = gpGlobals->time + BModeEnergyChargeInterval;
    }

    void CJumpSpirit::ItemPostFrame()
    {

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

        if (m_flTimeNextCharge != invalid_time_point && m_flTimeNextCharge <= gpGlobals->time)
        {
            pev->iuser1 += int((gpGlobals->time - m_flTimeNextCharge) / BModeEnergyChargeInterval) + 1.0;

            m_flTimeNextCharge = gpGlobals->time + BModeEnergyChargeInterval;

            if (pev->iuser1 >= BModeMaxEnergy)
                pev->iuser1 = BModeMaxEnergy;

            if (pev->iuser1 == BModeMaxEnergy)
                m_flTimeNextCharge = invalid_time_point;

            m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = pev->iuser1;       
        }

        if (m_fInReload)
            return;

        return CBasePlayerWeapon::ItemPostFrame();
    }

    void CJumpSpirit::Reload()
    {
        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            return;

        if (DefaultReload(50, ANIM_RELOAD, 2.2s, 1))
        {
#ifndef CLIENT_DLL         
            m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
            m_flNextReload = gpGlobals->time + 2.2s;
            m_flAccuracy = 0.99;
        }
    }


    void CJumpSpirit::SecondaryAttack(void)
    {
        if (pev->iuser1 < BModeJumpUseEnergy)
            return;

        if (m_iWeaponState)
        {
            if (gpGlobals->time > m_flTimeCheckCharge)
            {
                switch (m_iWeaponState)
                {
                    case 1:
                    {
                        SendWeaponAnim(ANIM_HOLD_LOOP);

                        m_iWeaponState = 2;
                        m_flTimeCheckCharge = gpGlobals->time + 4.0s;
                        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 9999s;
                        break;
                    }
                    case 2:
                    {     
                        UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/jumpspirit_idle.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM, 1);

                        m_flTimeCheckCharge = gpGlobals->time + 2.0s;
                    
                        break;
                    }   
                }
            }
        }
        else
        {
            SendWeaponAnim(ANIM_HOLD_START);

            UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/jumpspirit_ready.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM, 1);

            m_flTimeCheckCharge = gpGlobals->time + BModeChargeTime;
            m_iWeaponState = 1;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
        }

    }

    void CJumpSpirit::WeaponIdle(void)
    {
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

        if (m_iAModeState && !(m_pPlayer->pev->button & IN_ATTACK))
        {
            m_iAModeState = 0;
        }

        if (m_iWeaponState && !(m_pPlayer->pev->button & IN_ATTACK2))
        {
            OnAttack2Released();
            return;
        }

        if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
            return;

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
        SendWeaponAnim(ANIM_IDLE, UseDecrement() != FALSE);

    }
}
