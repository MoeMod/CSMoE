/*
herochainsaw.cpp
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

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

#define WPNSTATE_CHAINSAW_SLASH	(1<<1)

    class CHeroChainsaw : public LinkWeaponTemplate< CHeroChainsaw,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_M249>::template type,
            TPrecacheEvent     
    >
    {
    public:
    static constexpr const char *V_Model = "models/v_herochainsaw.mdl";
    static constexpr const char *P_Model = "models/p_herochainsaw.mdl";
    static constexpr const char *W_Model = "models/w_herochainsaw.mdl";
    static constexpr const char *EventFile = "events/herochainsaw.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_herochainsaw";
    static constexpr const char *AnimExtension = "herochainsaw";
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
        ANIM_SLASH1,
        ANIM_SLASH2,
        ANIM_SLASH3,
        ANIM_SLASH4,
        ANIM_EMPTY
	};

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
        Base::Precache();
     
        PRECACHE_SOUND("weapons/herochainsaw_idle.wav");
        PRECACHE_SOUND("weapons/herochainsaw_attack_end.wav");
        PRECACHE_SOUND("weapons/herochainsaw_slash1.wav");
        PRECACHE_SOUND("weapons/herochainsaw_slash2.wav");
    }

    BOOL CanDrop() override { return FALSE; }

    int GetItemInfo(ItemInfo* p) override
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "chainsawoil";
        p->iMaxAmmo1 = 1600;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = -1;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_HEROCHAINSAW;
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

    int ExtractAmmo(CBasePlayerWeapon* pWeapon) override
    {
        m_iClip = WEAPON_NOCLIP;
        m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = 1600;
        return CBasePlayerWeapon::ExtractAmmo(pWeapon);
    }


    float GetPrimaryAttackDamage() const
    {
        int flDamage = 38;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = 210;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = 72;
#endif
        return flDamage;
    }

    float GetSecondaryAttackDamage() const
    {
        float flDamage = 110.0f;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = 4600.0f;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = 500.0f;
#endif
        return flDamage;
    }


    BOOL Deploy(void) override
    {
        m_iWeaponState &= ~WPNSTATE_CHAINSAW_SLASH;
        m_iShotsFired = 0;
        m_flAttackLoop = invalid_time_point;

        return DefaultDeploy("models/v_herochainsaw.mdl", "models/p_herochainsaw.mdl", ANIM_DRAW, "chainsaw", UseDecrement() != FALSE);
    }

    void Reload(void)
    {
        return;
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
        vecEnd = vecSrc + gpGlobals->v_forward * 99.0;

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

            EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/chainsaw_hit1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
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
            {
                if (m_iClip <= 0)
                    EMIT_SOUND_DYN(ENT(pEntity->pev), CHAN_ITEM, "weapons/chainsaw_hit3.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
                else
                    EMIT_SOUND_DYN(ENT(pEntity->pev), CHAN_ITEM, "weapons/chainsaw_hit2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
            }

            ClearMultiDamage();

            pEntity->TraceAttack(m_pPlayer->pev, flDamage, (tr.vecEndPos - vecSrc).Normalize(), &tr, DMG_BULLET | DMG_NEVERGIB);

            ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

            if (!pEntity->IsPlayer())
                continue;

            if (pEntity->m_iTeam == m_pPlayer->m_iTeam)
                continue;

            if (((CBasePlayer*)pEntity)->m_bIsZombie)
            {
                AddKickRate(((CBasePlayer*)pEntity), m_pPlayer->pev->origin, 3200, 140);
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
        int flags;
#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        if (m_flAttackLoop == invalid_time_point)
        {
            SendWeaponAnim(ANIM_ATTACK1_START, UseDecrement() != FALSE);

            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5s;
            m_flAttackLoop = gpGlobals->time + 0.5s;

            PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFire, 0.0, g_vecZero,g_vecZero, 0.0, 0.0, int(m_pPlayer->pev->punchangle.x * 100.0), int(m_pPlayer->pev->punchangle.y * 100.0), FALSE, FALSE);

            return;
        }

        if (m_flAttackLoop > gpGlobals->time)
            return;

        m_iShotsFired++;
        m_iWeaponState &= ~WPNSTATE_CHAINSAW_SLASH;
        m_bDelayFire = true;
 
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
        SendWeaponAnim(ANIM_ATTACK1_LOOP, UseDecrement() != FALSE);

        m_flTimeWeaponIdle = m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.063s;

        Attack1();

        PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFire, 0.0, g_vecZero, g_vecZero, 0.0, 0.0, int(m_pPlayer->pev->punchangle.x * 100.0), int(m_pPlayer->pev->punchangle.y * 100.0), FALSE, TRUE);
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
        m_iWeaponState |= WPNSTATE_CHAINSAW_SLASH;
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK2);
#endif

        if (m_iClip <= 0)
        {
            SendWeaponAnim(ANIM_SLASH4 - (RANDOM_LONG(0, 1) != 0), UseDecrement() != FALSE);
           
        }
        else
        {
            SendWeaponAnim(ANIM_SLASH2 - (RANDOM_LONG(0, 1) != 0), UseDecrement() != FALSE);        
        }

        if(RANDOM_LONG(0, 1))
            EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/herochainsaw_slash1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
        else
            EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/herochainsaw_slash2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

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

        if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
            return;

        if (m_flAttackLoop != invalid_time_point)
        {
            SendWeaponAnim(ANIM_ATTACK1_END, UseDecrement() != FALSE);

            EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/herochainsaw_attack_end.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

            m_flNextPrimaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
            m_flAttackLoop = invalid_time_point;
        }
        else if (m_iClip > 0)
        {
            SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);

            EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/herochainsaw_idle.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.0s;
        }
        else
        {
            SendWeaponAnim(ANIM_EMPTY, UseDecrement() != FALSE);

            EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/herochainsaw_idle.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 4.0s;
        }
    }
    		 	
};
LINK_ENTITY_TO_CLASS(weapon_herochainsaw, CHeroChainsaw)
}