/*
chainsawex.cpp
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
#include "weapons/WeaponTemplate.hpp"


#ifndef CLIENT_DLL
namespace sv {
    class CChainsawEX_Scythe : public CBaseAnimating
    {
    public:
        CChainsawEX_Scythe()
        {
            m_flRemoveTime = invalid_time_point;
            m_flDamageTime = invalid_time_point;
        }

        KnockbackData GetKnockBackData() override { return { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f }; }
        float GetArmorRatioModifier() { return 1.8f; }
        int GetWeaponsId(void) { return WEAPON_CHAINSAWEX; }

        void Spawn() override
        {
            Precache();

            SetThink(&CChainsawEX_Scythe::ScytheThink);
            SetTouch(&CChainsawEX_Scythe::ScytheTouch);
            SET_MODEL(this->edict(), "models/ef_chainsawex.mdl");

            pev->rendermode = kRenderNormal;
            pev->renderfx = kRenderFxNone;
            pev->renderamt = 255;
            pev->solid = SOLID_TRIGGER;
            pev->movetype = MOVETYPE_FLY;

            pev->frame = 0;
            pev->sequence = 0;

            ResetSequenceInfo();

            UTIL_SetSize(pev, { -6, -6, -2 }, { 6, 6, 2 });

            m_flRemoveTime = gpGlobals->time + 7.0s;
            m_flDamageTime = gpGlobals->time + 0.1s;

            pev->nextthink = gpGlobals->time + 0.1s;
        }

        void Precache() override
        {
            PRECACHE_MODEL("models/ef_chainsawex.mdl");
        }

        void RadiusDamage()
        {
            CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
            CBasePlayer* pAttackPlayer = nullptr;
            if (pAttacker && pAttacker->IsPlayer())
                pAttackPlayer = static_cast<CBasePlayer*>(pAttacker);

            if (!pAttackPlayer)
                return;

            if (pAttackPlayer->m_bIsZombie)
                return;

            TraceResult tr;
            CBaseEntity* pEntity = NULL;

            Vector vecEnd;
            int bInWater = (UTIL_PointContents(pev->origin) == CONTENTS_WATER);
            float flDamage = GetScytheDamage();
            while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, 60.0)) != NULL)
            {
                if (pEntity->pev->takedamage == DAMAGE_NO)
                    continue;

                if (pEntity->edict() == pev->owner)
                    continue;

                if (bIsWallBetweenEntity(pEntity, pAttackPlayer))
                    continue;

                pEntity->m_LastHitGroup = HITGROUP_CHEST;
                pEntity->TakeDamage(pev, pAttackPlayer->pev, flDamage, DMG_BULLET);
            }
        }

        void EXPORT ScytheTouch(CBaseEntity* pOther)
        {
            if (!pOther)
                return;

            if (pOther->edict() == pev->owner)
                return;

            float flDamage = GetDirectDamage();

            entvars_t* pevOwner = VARS(pev->owner);

            if (!pOther->IsDormant())
            {
                if (pOther->pev->takedamage != DAMAGE_NO)
                {
                    TraceResult tr = UTIL_GetGlobalTrace();

                    ClearMultiDamage();
                    tr.iHitgroup = HITGROUP_CHEST;
                    pOther->TraceAttack(pevOwner, flDamage, pev->velocity, &tr, DMG_BULLET | DMG_SLASH);
                    ApplyMultiDamage(pev, pevOwner);
                }
            }

            pev->solid = SOLID_NOT;
            pev->movetype = MOVETYPE_NONE;
            pev->velocity = g_vecZero;
            pev->nextthink = gpGlobals->time + 0.1s;
        }

        void EXPORT ScytheThink()
        {
            pev->nextthink = gpGlobals->time + 0.1s;
            if (m_flRemoveTime <= gpGlobals->time)
            {
                Remove();
                return;
            }

            if (m_flDamageTime < gpGlobals->time)
            {
                RadiusDamage();
                m_flDamageTime = gpGlobals->time + 0.5s;
            }
        }

        time_point_t m_flRemoveTime;
        time_point_t m_flDamageTime;

    private:
        float GetDirectDamage()
        {
            float flDamage = 75;
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flDamage = 850;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = 2050;
            return flDamage;
        }

        float GetScytheDamage()
        {
            float flDamage = 65;
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flDamage = 350;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = 800;
            return flDamage;
        }

    protected:
        void Remove()
        {
            SetThink(nullptr);
            SetTouch(nullptr);
            pev->effects |= EF_NODRAW; // 0x80u
            return UTIL_Remove(this);
        }
    };

    LINK_ENTITY_TO_CLASS(chainsawex_scythe, CChainsawEX_Scythe)
}
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

#define WPNSTATE_CHAINSAW_SLASH	(1<<1)

    class CChainsawEX : public LinkWeaponTemplate< CChainsawEX,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_M249>::template type,
            TPrecacheEvent     
    >
    {
    public:
    static constexpr const char *V_Model = "models/v_chainsawex.mdl";
    static constexpr const char *P_Model = "models/p_chainsawex.mdl";
    static constexpr const char *W_Model = "models/w_chainsawex.mdl";
    static constexpr const char *EventFile = "events/chainsawex.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_chainsawex";
    static constexpr const char *AnimExtension = "chainsaw";
    static constexpr int MaxClip = 200;
    KnockbackData KnockBack = { 290.0f, 85.0f, 50.0f, 150.0f, 0.25f };
	enum
	{
        ANIM_IDLE1,
        ANIM_DRAW,
        ANIM_DRAW_EMPTY,
        ANIM_ATTACK1_START,
        ANIM_ATTACK1_LOOP,
        ANIM_ATTACK1_END,
        ANIM_RELOAD,
        ANIM_SLASH1,
        ANIM_SLASH2,
        ANIM_SLASH3,
        ANIM_SLASH4,
        ANIM_EMPTY,
        ANIM_SKILL_START,
        ANIM_SKILL_LOOP,
        ANIM_CHARGING_LOOP,
        ANIM_SKILL_END,
	};

    int m_iChargeState;
    time_point_t m_flTimeCheckCharge;
    time_point_t m_flAttackLoop;
   

    KnockbackData GetKnockBackData() override
    {
        if (m_iWeaponState & WPNSTATE_CHAINSAW_SLASH)
            return { 290.0f, 85.0f, 50.0f, 150.0f, 0.9f };
        else
            return { 290.0f, 85.0f, 50.0f, 150.0f, 0.25f };
    }

    void Precache(void)
    {
        PRECACHE_SOUND("weapons/chainsawex_idle.wav");
        PRECACHE_SOUND("weapons/chainsawex_attack1_end.wav");
        PRECACHE_SOUND("weapons/chainsawex_slash1.wav");
        PRECACHE_SOUND("weapons/chainsawex_slash2.wav");
        PRECACHE_SOUND("weapons/chainsawex_slash3.wav");
        PRECACHE_SOUND("weapons/chainsawex_slash4.wav");
        PRECACHE_SOUND("weapons/chainsawex-2.wav");
        PRECACHE_SOUND("weapons/chainsawex_charge.wav");
        PRECACHE_SOUND("weapons/chainsawex_charge_end.wav");
        PRECACHE_SOUND("weapons/chainsawex_hit1.wav");
        PRECACHE_SOUND("weapons/chainsawex-1_start.wav");
        PRECACHE_SOUND("weapons/chainsawex_wall.wav");

        PRECACHE_MODEL("sprites/ef_chainattack.spr");
        PRECACHE_MODEL("sprites/ef_chaincharging.spr");
        Base::Precache();
    }


    int GetItemInfo(ItemInfo* p) override
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "chainsawoil";
        p->iMaxAmmo1 = 200;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = 200;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_CHAINSAWEX;
        p->iFlags = 0;
        p->iWeight = M249_WEIGHT;
        return 1;
    }

    bool HasSecondaryAttack() override { return true; }
#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_chainsaw" , 300 }; }
#endif
    float GetArmorRatioModifier() override { return 1.8; }
    WeaponCrosshairData GetCrosshairData() override { return { 0, 6, 3, 0 }; }


    float GetPrimaryAttackDamage() const
    {
        int flDamage = 38;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = 165;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = 500;
#endif
        return flDamage;
    }

    float GetSecondaryAttackDamage() const
    {
        float flDamage = 110.0f;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = 2250.0f;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = 6500.0f;
#endif
        return flDamage;
    }


    BOOL Deploy(void) override
    {
        m_iWeaponState &= ~WPNSTATE_CHAINSAW_SLASH;
        m_iShotsFired = 0;
        m_flAttackLoop = invalid_time_point;
        m_iChargeState = 0;
        m_flTimeCheckCharge = gpGlobals->time + 999999s;

        return DefaultDeploy("models/v_chainsawex.mdl", "models/p_chainsawex.mdl", ANIM_DRAW, "chainsaw", UseDecrement() != FALSE, 1.11s);
    }

    void Reload(void)
    {
        PLAYBACK_EVENT_FULL(FEV_HOSTONLY, ENT(m_pPlayer->pev), m_usFire, 0.0, g_vecZero, g_vecZero, 0.0, 0.0, 0, 1, TRUE, TRUE);

        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            return;

        if (DefaultReload(200, ANIM_RELOAD, 3.0s))
        {
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0s;
            m_iWeaponState &= ~WPNSTATE_CHAINSAW_SLASH;
            m_bDelayFire = false;
            m_iShotsFired = 0;
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
        }
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

    void Attack1(void)
    {
#ifndef CLIENT_DLL

        Vector vecSrc, vecEnd;
        TraceResult tr;

        UTIL_MakeVectors(m_pPlayer->pev->v_angle);

        vecSrc = m_pPlayer->GetGunPosition();
        vecEnd = vecSrc + gpGlobals->v_forward * 150.0;

        UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

        float flDamage = GetPrimaryAttackDamage();

        if (tr.flFraction >= 1.0)
        {
            UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(m_pPlayer->pev), &tr);

            if (tr.flFraction >= 1.0)
                return;

            CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);

            if (!pHit || pHit->IsBSPModel())
            {
                FindHullIntersection(vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, ENT(m_pPlayer->pev));
            }

            vecEnd = tr.vecEndPos;
        }

        if (tr.flFraction < 1.0)
        {
            CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

            ClearMultiDamage();

            pEntity->TraceAttack(m_pPlayer->pev, flDamage, gpGlobals->v_forward, &tr, DMG_BULLET | DMG_NEVERGIB);

            ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

            EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/chainsawex_hit1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
        }
#endif
    }

    void Slash(void)
    {
#ifndef CLIENT_DLL
        int classify;
        Vector vecSrc, vecEnd;
        TraceResult tr;
        CBaseEntity* pEntity = NULL;

        UTIL_MakeVectors(m_pPlayer->pev->v_angle);

        vecSrc = m_pPlayer->GetGunPosition();

        Vector2D vecAngleCheckDir = gpGlobals->v_forward.Make2D().Normalize();
        float angCheck = cos(160.0 * 0.5 * M_PI / 180.0);
        float flDamage;

        flDamage = GetSecondaryAttackDamage();
      
        while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, 125.0)) != NULL)
        {
            if (pEntity == m_pPlayer)
                continue;

            if (pEntity->IsDormant())
                continue;

            if (pEntity->pev->takedamage == DAMAGE_NO)
                continue;

            vecEnd = pEntity->BodyTarget(vecSrc);

            if (DotProduct((vecEnd.Make2D() - vecSrc.Make2D()).Normalize(), vecAngleCheckDir) < angCheck)
                continue;

            UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

            if (tr.flFraction < 1.0 && tr.pHit != ENT(pEntity->pev))
                continue;

            classify = pEntity->Classify();

            if (classify && classify != CLASS_MACHINE)
                EMIT_SOUND_DYN(ENT(pEntity->pev), CHAN_WEAPON, "weapons/chainsawex_wall.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

            ClearMultiDamage();

            pEntity->TraceAttack(m_pPlayer->pev, flDamage, (tr.vecEndPos - vecSrc).Normalize(), &tr, DMG_BULLET | DMG_NEVERGIB);

            ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

            if (!pEntity->IsPlayer())
                continue;

            if (pEntity->m_iTeam == m_pPlayer->m_iTeam)
                continue;

            if (((CBasePlayer*)pEntity)->m_bIsZombie)
            {
                AddKickRate(((CBasePlayer*)pEntity), m_pPlayer->pev->origin, 3200, 120);
            }
        }
#endif
    }

    void AddKickRate(CBasePlayer* pPlayer, Vector vecSrc, float amount, float boost)
    {
        //from AE
        Vector vecDir = (pPlayer->pev->origin - vecSrc).Normalize();

        pPlayer->pev->velocity.x += vecDir.x * amount;
        pPlayer->pev->velocity.y += vecDir.y * amount;
        pPlayer->pev->velocity.z += boost;
    }

    void PrimaryAttack(void)
    {
        m_iChargeState = 0;

        if (m_iClip <= 0)
        {
#ifndef CLIENT_DLL
            if (TheBots)
                TheBots->OnEvent(EVENT_WEAPON_FIRED_ON_EMPTY, m_pPlayer, NULL);
#endif

            if (m_flAttackLoop != invalid_time_point)
            {
                SendWeaponAnim(ANIM_ATTACK1_END, UseDecrement() != FALSE);

                m_flNextPrimaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;

                m_flAttackLoop = invalid_time_point;
            }

            return;
        }

        int flags;
#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        if (m_flAttackLoop == invalid_time_point)
        {
            SendWeaponAnim(ANIM_ATTACK1_START, UseDecrement() != FALSE);

            EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/chainsawex-1_start.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);

            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5s;
            m_flAttackLoop = gpGlobals->time + 0.5s;

            return;
        }

        if (m_flAttackLoop > gpGlobals->time)
            return;

        m_iShotsFired++;
        m_iWeaponState &= ~WPNSTATE_CHAINSAW_SLASH;
        m_bDelayFire = true;
        m_iClip--;
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
        SendWeaponAnim(ANIM_ATTACK1_LOOP, UseDecrement() != FALSE);

        m_flTimeWeaponIdle = m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.063s;

        Attack1();
        PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFire, 0.0, g_vecZero, g_vecZero, 0.0, 0.0, int(m_pPlayer->pev->punchangle.x * 100.0), 0, FALSE, TRUE);
#ifndef CLIENT_DLL
        if (!m_iClip && !m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", 0, 0);
#endif
        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            KickBackCustom(1.0, 0.4, 0.2, 0.15, 3.0, 2.0, 0);
        else if (m_pPlayer->pev->velocity.Length2D() > 0.0)
            KickBackCustom(0.3, 0.075, 0.05, 0.02, 2.5, 1.5, 0);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            KickBackCustom(0.175, 0.04, 0.03, 0.01, 1.5, 1.0, 0);
        else
            KickBackCustom(0.2, 0.05, 0.035, 0.015, 2.0, 1.25, 0);
    }

    void SecondaryAttack(void)
    {
        if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] && !m_iClip)
        {
            OnAttack2Released();
            return;
        }
           
        if (m_iChargeState)
        {
            if (gpGlobals->time > m_flTimeCheckCharge)
            {
                if (m_iChargeState > 1)
                {
                    SendWeaponAnim(ANIM_CHARGING_LOOP);

                    EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/chainsawex_charge_end.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);

                    m_iChargeState = 3;
                    m_flTimeCheckCharge = gpGlobals->time + 999999s;
                    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3s;
                }
                else
                {
                    SendWeaponAnim(ANIM_SKILL_LOOP);

                    m_iChargeState = 2;
                    m_flTimeCheckCharge = gpGlobals->time + 1.7s - 0.53s;
                    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.7s - 0.53s;

                    PLAYBACK_EVENT_FULL(FEV_HOSTONLY, ENT(m_pPlayer->pev), m_usFire, 0.0, g_vecZero, g_vecZero, 0.0, 0.0, 0, 1, FALSE, TRUE);

                }
            }
        }
        else
        {
            SendWeaponAnim(ANIM_SKILL_START);

            EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/chainsawex_charge.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);

            m_flTimeCheckCharge = gpGlobals->time + 0.53s;
            m_iChargeState = 1;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.53s;
            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 99999s;
        }


    }

    void OnAttack2Released()
    {
        //turn off
        PLAYBACK_EVENT_FULL(FEV_HOSTONLY, ENT(m_pPlayer->pev), m_usFire, 0.0, g_vecZero, g_vecZero, 0.0, 0.0, 0, 1, TRUE, TRUE);


        if (m_iChargeState >= 3)
        {
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_ATTACK2);
#endif
            SendWeaponAnim(ANIM_SKILL_END, UseDecrement() != FALSE);
            //create entity

            EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/chainsawex-2.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);
        
            if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
            {
                m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= 10;
                if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
                    m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = 0;
            }
            else
            {
                if (m_iClip)
                {
                    m_iClip -= 10;
                    if (m_iClip <= 0)
                        m_iClip = 0;               
                }
            }

#ifndef CLIENT_DLL
            if (!m_iClip && !m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
                m_pPlayer->SetSuitUpdate("!HEV_AMO0", 0, 0);

            Vector vecAngle = m_pPlayer->pev->v_angle;
            vecAngle[0] = 360.0 - vecAngle[0];
            UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
            Vector vecSrc = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 16 + gpGlobals->v_right * 8 + gpGlobals->v_up * -8;
            CChainsawEX_Scythe* pEnt = static_cast<CChainsawEX_Scythe*>(CBaseEntity::Create("chainsawex_scythe", vecSrc, vecAngle, ENT(m_pPlayer->pev)));

            if (pEnt)
            {
                pEnt->m_iTeam = m_pPlayer->m_iTeam;
                pEnt->pev->velocity = gpGlobals->v_forward * 1000;
            }
#endif
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
            m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.15s;
            m_flAttackLoop = invalid_time_point;
            m_iChargeState = 0;
            return;
        }
        m_iChargeState = 0;

        m_iWeaponState |= WPNSTATE_CHAINSAW_SLASH;
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK2);
#endif

        if (m_iClip <= 0)
        {
            SendWeaponAnim(ANIM_SLASH4 - (RANDOM_LONG(0, 1) != 0), UseDecrement() != FALSE);

            if (RANDOM_LONG(0, 1))
                EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/chainsawex_slash3.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
            else
                EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/chainsawex_slash4.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
        }
        else
        {
            SendWeaponAnim(ANIM_SLASH2 - (RANDOM_LONG(0, 1) != 0), UseDecrement() != FALSE);

            if (RANDOM_LONG(0, 1))
                EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/chainsawex_slash1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
            else
                EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/chainsawex_slash2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
        }


        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.15s;
        m_flAttackLoop = invalid_time_point;

        Slash();

        int flags;
#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFire, 0.0, g_vecZero, g_vecZero, 0.0, 0.0, int(m_pPlayer->pev->punchangle.x * 100.0), int(m_pPlayer->pev->punchangle.y * 100.0), TRUE, FALSE);
#ifndef CLIENT_DLL
        if (!m_iClip && !m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", 0, 0);
#endif
    }

    void WeaponIdle(void)
    {
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

        if (m_iChargeState && !(m_pPlayer->pev->button & IN_ATTACK2))
        {
            OnAttack2Released();
        }

        if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
            return;

        if (m_flAttackLoop != invalid_time_point)
        {
            SendWeaponAnim(ANIM_ATTACK1_END, UseDecrement() != FALSE);

            EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/chainsawex_attack1_end.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

            m_flNextPrimaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
            m_flAttackLoop = invalid_time_point;
        }
        else if (m_iClip > 0)
        {
            SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);

            EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/chainsawex_idle.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.7s;
        }
        else
        {
            SendWeaponAnim(ANIM_EMPTY, UseDecrement() != FALSE);

            EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/chainsawex_idle.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.7s;
        }
    }
    		 	
};
LINK_ENTITY_TO_CLASS(weapon_chainsawex, CChainsawEX)
}