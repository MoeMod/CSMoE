/*
buffng7.cpp
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

#ifndef CLIENT_DLL
#define BUFFNG7_DEBUFF_TIME 10

    class CBuffNG7Debuff : public CBaseEntity
    {
    public:
        KnockbackData GetKnockBackData() override { return { 400.0f, 400.0f, 400.0f, 400.0f, 0.6f }; }
        float GetArmorRatioModifier() { return 1.0; }
        int GetWeaponsId(void) { return WEAPON_BUFFNG7; }

        CBuffNG7Debuff()
        {
            m_iHitTime = 0;
        }
    
        void Spawn() override
        {
            Precache();

            SetThink(&CBuffNG7Debuff::OnThink);

            pev->solid = SOLID_NOT;
            pev->movetype = MOVETYPE_NOCLIP;

            pev->nextthink = gpGlobals->time + 0.1s;
        }

        void Precache() override
        {
            m_iSparkIndex = PRECACHE_MODEL("sprites/ef_buffng7_debuff.spr");
     
            //PRECACHE_SOUND("weapons/buffng7_exp1.wav");
        }

        bool CanAttack()
        {
            CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
            CBasePlayer* pAttackPlayer = nullptr;
            if (pAttacker && pAttacker->IsPlayer())
                pAttackPlayer = static_cast<CBasePlayer*>(pAttacker);

            if (!pAttackPlayer)
                return false;

            if (pAttackPlayer->m_bIsZombie)
                return false;

            if (!pAttackPlayer->IsAlive())
                return false;

            if (!m_pTarget)
                return false;

            if (!m_pTarget->IsAlive())
                return false;

            if (m_iHitTime >= BUFFNG7_DEBUFF_TIME)
                return false;
        }

        void EXPORT OnThink()
        {
            m_iHitTime++;

            pev->nextthink = gpGlobals->time + 1.0s;

            if (!CanAttack())
            {
                SetThink(nullptr);
                UTIL_Remove(this);
            }

            m_pTarget->TakeDamage(pev, VARS(pev->owner), GetDamage(), DMG_BULLET);

            if (m_pTarget->IsAlive())
            {
                MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
                WRITE_BYTE(TE_TEMPSPRITE);
                WRITE_COORD(m_pTarget->pev->origin.x);
                WRITE_COORD(m_pTarget->pev->origin.y);
                WRITE_COORD(m_pTarget->pev->origin.z);
                WRITE_SHORT(m_iSparkIndex);
                WRITE_BYTE(2);
                WRITE_BYTE(255);
                WRITE_BYTE(30);
                MESSAGE_END();
            }         
        }

        float GetDamage()
        {
            float flDamage = 10;
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flDamage = 250;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = 550;
            return flDamage;
        }

        CBaseEntity* m_pTarget;
        int m_iSparkIndex;
        int m_iHitTime;
    };

LINK_ENTITY_TO_CLASS(buffng7_debuff, CBuffNG7Debuff)

    class CBuffNG7ForceField : public CBaseAnimating
    {
    public:
        CBuffNG7ForceField()
        {
            m_flRemoveTime = invalid_time_point;
        }

        KnockbackData GetKnockBackData() override { return { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f }; }
        float GetArmorRatioModifier() { return 1.0; }
        int GetWeaponsId(void) { return WEAPON_BUFFNG7; }

        void Spawn() override
        {
            Precache();

            SetThink(&CBuffNG7ForceField::OnThink);
            SetTouch(&CBuffNG7ForceField::OnTouch);
            SET_MODEL(this->edict(), "models/buffng7_forcefield.mdl");

            pev->rendermode = kRenderNormal;
            pev->renderfx = kRenderFxNone;
            pev->renderamt = 255;
            pev->solid = SOLID_TRIGGER;
            pev->movetype = MOVETYPE_TOSS;
            pev->effects |= EF_NOCULL;

            pev->gravity = 1.0;
            pev->frame = 0;
            pev->sequence = 1;

            ResetSequenceInfo();

            UTIL_SetSize(pev, g_vecZero, g_vecZero);

            m_flRemoveTime = gpGlobals->time + 10.0s;

            pev->nextthink = gpGlobals->time + 0.1s;
        }

        void Precache() override
        {
            m_iExpIndex = PRECACHE_MODEL("sprites/ef_buffng7_magazine_exp.spr");
            PRECACHE_MODEL("models/buffng7_forcefield.mdl");
            PRECACHE_SOUND("weapons/buffng7_exp1.wav");
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
            float flDamage = GetDamage();
            while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, 70.0)) != NULL)
            {
                if (pEntity->pev->takedamage == DAMAGE_NO)
                    continue;

                if (pEntity->edict() == pev->owner)
                    continue;

                if (UTIL_IsWallBetweenEntity(pEntity, this))
                    continue;

                pEntity->TakeDamage(pev, pAttackPlayer->pev, flDamage, DMG_EXPLOSION);
            }
        }

        void EXPORT OnTouch(CBaseEntity* pOther)
        {
            if (!pOther)
                return;

            if (pOther->edict() == pev->owner)
                return;

            entvars_t* pevOwner = VARS(pev->owner);

        }

        void EXPORT OnThink()
        {
            pev->nextthink = gpGlobals->time + 0.1s;

            if (pev->sequence == 1 && pev->flags & FL_ONGROUND)
            {
                /*if (g_pModRunning->DamageTrack() == DT_NONE)
                {
                    Remove();
                    return;
                }*/

                pev->solid = SOLID_NOT;
                pev->movetype = MOVETYPE_NONE;

                pev->sequence = 0;
                pev->frame = 0;
                pev->body = 1;

                ResetSequenceInfo();
                
                EMIT_SOUND_DYN(this->edict(), CHAN_ITEM, "weapons/buffng7_exp1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

                MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
                WRITE_BYTE(TE_TEMPSPRITE);
                WRITE_COORD(pev->origin.x);
                WRITE_COORD(pev->origin.y);
                WRITE_COORD(pev->origin.z);
                WRITE_SHORT(m_iExpIndex);
                WRITE_BYTE(4);
                WRITE_BYTE(255);
                WRITE_BYTE(30);
                MESSAGE_END();

                RadiusDamage();

                m_flRemoveTime = gpGlobals->time + GetRemoveTime();
                return;
            }

            if (!pev->sequence)
            {
                pev->nextthink = gpGlobals->time + 0.01s;

                Field_Blocking();
            }

            if (m_flRemoveTime <= gpGlobals->time)
            {
                Remove();
                return;
            }
        }

        void Field_Blocking()
        {
            //From SME
            float LENGTH = 220.0;
            float TALL = 50.0;
            float WIDE = 90.0;
            float flKnockBack = 500.0;
            UTIL_MakeVectors(this->pev->angles);

            Vector vecFor, vecRight, vecUp;

            vecFor = gpGlobals->v_forward;
            vecRight = gpGlobals->v_right;
            vecUp = gpGlobals->v_up;

            float w2 = LENGTH * 0.5;
            float t2 = TALL * 0.5;

            Vector vecSrc = pev->origin;
            Vector vecSrc2 = vecSrc + vecUp * t2;

            // Only check for circular ranged targets
            float flRange = sqrt(w2 * w2 + t2 * t2);


            Vector vecEntityVel, vecEnd, vecConflict;
            float fMultiplier, fProduct;

            CBaseEntity* pEntity = NULL;
            while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc2, flRange)) != NULL)
            {
                if (!pEntity->IsAlive())
                    continue;

                if (pEntity->edict() == pev->owner)
                    continue;

                vecEntityVel = pEntity->pev->velocity;
                // #Non-Conflict situation 1 - non velocity
                if (vecEntityVel.Length() <= 0.01)
                    continue;


                vecEnd = pEntity->pev->origin;

                Field_GetConflictPoint(vecEnd, vecEntityVel, vecSrc, vecFor, fMultiplier, vecConflict);

                // #Non-Conflict situation 3 - cannot be conflict in positive multiplier
                if (fMultiplier <= 0.01)
                    continue;

                // #Non-Conflict situation 2 - Pararell direction
                fProduct = Field_GetInnerProduct(vecFor, vecEntityVel);
                if (abs(fProduct) <= 0.01)
                    continue;

                // #Non-Conflict situation 4 - out of ranged (plane)
                if (!Field_CheckPlaneDistance(vecSrc, vecUp, vecConflict, w2, WIDE))
                    continue;

                // #Non-Conflict situation 5 - out of ranged (vert)
                if (Field_GetVerticalDistance(vecEnd, vecFor, vecSrc) > TALL)
                    continue;

                // Get cosin
                fProduct /= vecFor.Length();
                fProduct /= vecEntityVel.Length();

                if (fProduct > 0.01) // back
                    vecEntityVel = vecFor * -flKnockBack;
                else // forward
                    vecEntityVel = vecFor *  flKnockBack;

                pEntity->pev->velocity = vecEntityVel;
            }
        }

        float Field_GetInnerProduct(Vector a, Vector b)
        {
            float fReturn = a[0];
            fReturn *= b[0];

            float fTemp = a[1];
            fTemp *= b[1];

            fReturn += fTemp;

            fTemp = a[2];
            fTemp *= b[2];

            fReturn += fTemp;

            return fReturn;
        }

        void Field_GetConflictPoint(Vector& vStart, Vector &vDir, Vector& s_vPoint, Vector& vHor, float& fMul, Vector &vRes)
        {
            fMul = 0.0;

            // Temporary values will be used for calculation
            float fTemp1 = 0.0, fTemp2 = 0.0, fTemp3 = 0.0;

            for (int i = 0; i < 3; i++)
            {
                if (vHor[i])
                {
                    fTemp2 = vStart[i];
                    fTemp2 -= s_vPoint[i];
                    fTemp2 *= vHor[i];
                    fTemp1 += fTemp2;
                }
            }

            for (int i = 0; i < 3; i++)
            {
                if (vHor[i])
                {
                    fTemp3 = vHor[i];
                    fTemp3 *= vDir[i];
                    fTemp2 += fTemp3;
                }
            }
            fTemp2 *= -1.0;

            if (abs(fTemp2) <= 0.01)
            {
                //server_print("Error: Invalid value %f (9)", fTemp2);
                return;
            }

            fMul = fTemp1 / fTemp2;

            vRes = vStart + fMul * vDir;
        }


        bool Field_CheckPlaneDistance(Vector vecSrc, Vector vecUp, Vector vecConflict, float fWidth2, float fTall)
        {
            float fTemp1 = 0.0, fTemp2 = 0.0, fTemp3 = 0.0;

            Vector vecSrc2;
            vecSrc2 = vecUp * fTall / 2;
            vecSrc2 += vecSrc;
            vecSrc2 -= vecConflict;

            fTemp1 = vecSrc2.Length();

            if (fTemp1 <= 0.01)
            {
                //server_print("Warning: abnormal tempdist result: %f", fTemp1);
                return false;
            }

            Vector vecSrc3;
            vecSrc3 = vecUp * fTall;

            fTemp1 = Field_GetInnerProduct(vecSrc2, vecSrc3);
            fTemp1 = abs(fTemp1);

            if (fTemp1 <= 0.01) // vert
            {
                if (vecSrc2.Length() > fWidth2)
                    return false;

                return true;
            }

            fTemp2 = vecSrc3.Length();
            fTemp1 /= fTemp2; // res

            fTemp2 = fTall;
            fTemp2 /= 2.0;

            if (fTemp1 > fTemp2)
                return false;


            fTemp2 = vecSrc2.Length();
            fTemp2 *= fTemp2;

            fTemp3 = fTemp1;
            fTemp3 *= fTemp3;

            if (fTemp2 < fTemp3)
            {
                //server_print("Warning: abnormal root variables: %f %f", fTemp2, fTemp3);
                return false;
            }

            fTemp2 -= fTemp3;
            fTemp2 = sqrt(fTemp2);

            if (fTemp2 > fWidth2)
                return false;

            return true;
        }

        float Field_GetVerticalDistance(Vector vecEnd, Vector vecDir, Vector vecSrc)
        {
            /*2.0 * (vecSrc[0] * vecDir[0] - vecEnd[0] * vecDir[0]
            + vecSrc[1] * vecDir[1] - vecEnd[1] * vecDir[1]
            + vecSrc[2] * vecDir[2] - vecEnd[2] * vecDir[2])
            / (1 + vecDir[0] * vecDir[0] + vecDir[1] * vecDir[1] + vecDir[2] + vecDir[2]);*/

            float fTemp1 = 0.0, fTemp2 = 0.0, fTemp3 = 0.0;

            fTemp1 = vecSrc[0];
            fTemp1 *= vecDir[0];

            fTemp2 = vecEnd[0];
            fTemp2 *= vecDir[0];

            fTemp1 -= fTemp2;

            fTemp2 = vecSrc[1];
            fTemp2 *= vecDir[1];

            fTemp3 = vecEnd[1];
            fTemp3 *= vecDir[1];

            fTemp2 -= fTemp3;
            fTemp1 += fTemp2;

            fTemp2 = vecSrc[2];
            fTemp2 *= vecDir[2];

            fTemp3 = vecEnd[2];
            fTemp3 *= vecDir[2];

            fTemp2 -= fTemp3;
            fTemp1 += fTemp2;

            fTemp2 = 1.0;

            fTemp3 = vecDir[0];
            fTemp3 *= vecDir[0];
            fTemp2 += fTemp3;

            fTemp3 = vecDir[1];
            fTemp3 *= vecDir[1];
            fTemp2 += fTemp3;

            fTemp3 = vecDir[2];
            fTemp3 *= vecDir[2];
            fTemp2 += fTemp3;

            fTemp1 /= fTemp2;
            fTemp1 *= 2.0;

            fTemp1 = abs(fTemp1);

            return fTemp1;
        }

        int m_iExpIndex;
        time_point_t m_flRemoveTime;

    private:
        float GetDamage()
        {
            float flDamage = 5;
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flDamage = 100;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = 100;
            return flDamage;
        }

        duration_t GetRemoveTime()
        {
            duration_t flLife = 10s;
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flLife = 10s;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flLife = 12s;
            return flLife;
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

    LINK_ENTITY_TO_CLASS(buffng7_forcefield, CBuffNG7ForceField)
#endif

    class CBUFFNG7 : public LinkWeaponTemplate< CBUFFNG7,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_M249>::template type,
            TPrecacheEvent,
            TGetDamageDefault
    >
    {
    public:

    static constexpr const char *V_Model = "models/v_buffng7.mdl";
    static constexpr const char *P_Model = "models/p_buffng7.mdl";
    static constexpr const char *W_Model = "models/w_buffng7.mdl";
    static constexpr const char *EventFile = "events/buffng7.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_buffng7";
    static constexpr const char *AnimExtension = "m249";
    static constexpr int MaxClip = 150;
	enum
	{
		ANIM_IDLE1 = 0,
        ANIM_SHOOT1,
        ANIM_SHOOT2,
        ANIM_RELOAD,
        ANIM_DRAW,
        ANIM_B_MODE,
        ANIM_A_MODE,
        ANIM_IDLE_B,
        ANIM_SHOOT1_B,
        ANIM_SHOOT2_B,
        ANIM_RELOAD_B,
        ANIM_DRAW_B,
	};

#define WPNSTATE_BUFFNG7_MODEB	1
#define WPNSTATE_BUFFNG7_SHOOTB_READY	2
#define FIELD_TARGET_SHOTS_FIRED    50
#define DEBUFF_SHOTS_FIRED    25
#define DEBUFF_INTERVAL 0.8s
#define DEBUFF_RADIUS 5 * 39.37
#define DEBUFF_DURATION_TIME 12s

    static constexpr float PlayerMaxSpeed = 280;
    static constexpr float ArmorRatioModifier = 1.5f;
	static constexpr float PrimaryAttackWalkingMiniumSpeed = 195;

    static constexpr int DamageDefault = 31;
    static constexpr int DamageZB = 285;
    static constexpr int DamageZBS = 570;

    static constexpr float AccuracyDefault = 0.2;
    static constexpr float AccuracyMax = 1.0;
    static constexpr float RangeModifier = 0.97;
    static constexpr auto BulletType = BULLET_PLAYER_762MM;
    /*static constexpr*/ int Penetration = 7;
    KnockbackData KnockBack = { 340.0f, 240.0f, 290.0f, 105.0f, 0.6f };

    int m_iDebuffBeamIndex;
    int m_iState;  
    int m_iExpIndex;
    int m_iChargeShotsFired;
    int m_iTargetShotsFired;
    int m_iFieldShostFired;
#ifndef CLIENT_DLL
    bool m_bEnableDebuff;
    int m_iDebuffShotsFired;
    std::vector<int> m_TargetVector;
    time_point_t m_flDebuffTime;
    time_point_t m_flNextGiveDebuffTime;

#endif
    void Spawn() override
    {
        m_iState = 0;
        m_iTargetShotsFired = 1;
#ifndef CLIENT_DLL
        m_bEnableDebuff = false;
        m_iDebuffShotsFired = 0;
        m_flDebuffTime = m_flNextGiveDebuffTime = invalid_time_point;
#endif
        Base::Spawn();
    }

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "buffng7";
        p->iMaxAmmo1 = 200;
        p->pszAmmo2 = "BuffNG7Field";
        p->iMaxAmmo2 = 1;
        p->iMaxClip = 150;
        p->iSlot = 0;
        p->iPosition = 1;
        p->iId = m_iId = WEAPON_BUFFNG7;
        p->iFlags = 0;
        p->iWeight = M249_WEIGHT;

        return 1;
    }

    float GetArmorRatioModifier() override { return 1.5; }
#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_buffng7" , 60 }; }
#endif
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 4, 3, 195 }; }


    float GetMaxSpeed(void) override
    {
        if (m_iWeaponState & WPNSTATE_BUFFNG7_MODEB)
            return 210;
        return 280;
    }


    void Precache(void)
    {
        PRECACHE_MODEL("models/buffng7_barrier.mdl");
      
        PRECACHE_MODEL("models/p_buffng7_b.mdl");

        PRECACHE_MODEL("sprites/ef_buffng7_bmode.spr");
        PRECACHE_MODEL("sprites/ef_buffng7_bmode2.spr");
        PRECACHE_MODEL("sprites/ef_buffng7_bmode3.spr");
        PRECACHE_MODEL("sprites/ef_buffng7_charging_beam.spr");
        PRECACHE_MODEL("sprites/ef_buffng7_charging_shot.spr");
        m_iExpIndex = PRECACHE_MODEL("sprites/ef_buffng7_charging_shot_exp.spr");

        PRECACHE_MODEL("sprites/ef_buffng7_shield.spr");
        PRECACHE_MODEL("sprites/ef_buffng7_spark_a.spr");
        PRECACHE_MODEL("sprites/ef_buffng7_spark_b.spr");
        PRECACHE_MODEL("sprites/muzzleflash155.spr");
        m_iDebuffBeamIndex = PRECACHE_MODEL("sprites/ef_buffng7_electric_field_beam.spr");

        PRECACHE_SOUND("weapons/buffng7_b_idle.wav");
        PRECACHE_SOUND("weapons/buffng7_beep.wav");
        //PRECACHE_SOUND("weapons/buffng7_beep2.wav");???

        PRECACHE_SOUND("weapons/buffng7_exp2.wav");
        PRECACHE_SOUND("weapons/buffng7_b_idle.wav");

        PRECACHE_SOUND("weapons/buffng7-1.wav");
        PRECACHE_SOUND("weapons/buffng7-2.wav");
        PRECACHE_SOUND("weapons/buffng7-3.wav");

        Base::Precache();
    }

    BOOL Deploy(void)
    {
        m_flAccuracy = 0.2;
        m_iShotsFired = 0;
        m_iChargeShotsFired = 0;

        if (m_iWeaponState & WPNSTATE_BUFFNG7_MODEB)
            return DefaultDeploy(V_Model, "models/p_buffng7_b.mdl", ANIM_DRAW_B, "m249", UseDecrement() != FALSE);
        else
            return DefaultDeploy(V_Model, P_Model, ANIM_DRAW, "m249", UseDecrement() != FALSE);
    }

    void PrimaryAttack()
    {
        if (m_iWeaponState & WPNSTATE_BUFFNG7_MODEB)
            BUFFNG7FireB();
        else
        {
            if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
                BUFFNG7FireA(m_flAccuracy * 0.2 + 0.05, 0.099s, FALSE);
            else if (m_pPlayer->pev->velocity.Length2D() > 195)
                BUFFNG7FireA(m_flAccuracy * 0.03 + 0.035, 0.099s, FALSE);
            else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
                BUFFNG7FireA(m_flAccuracy * 0.02 + 0.01, 0.099s, FALSE);
            else
                BUFFNG7FireA(m_flAccuracy * 0.0 + 0.021, 0.099s, FALSE);
        }
    }

    void GetFireInfo(float& flDamage, float& flFireDistance, float& flFireSpread, int& cShots, duration_t& flCycleTime)
    {
        flFireSpread = 0.03;
        cShots = 7;
        flCycleTime = 0.26s;
        flFireDistance = 3048;
        flDamage = 30;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = 96;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = 228;
#endif
    }

    void BUFFNG7FireA(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_bDelayFire = true;
        m_iShotsFired++;
        m_flAccuracy = (m_iShotsFired * m_iShotsFired * m_iShotsFired) / 190.0 + 0.3;

        if (m_flAccuracy > 1.0)
            m_flAccuracy = 1.0;

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

        if (!m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType])
        {
            m_iFieldShostFired++;
            if (m_iFieldShostFired >= FIELD_TARGET_SHOTS_FIRED)
            {
                m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 1;

#ifndef CLIENT_DLL
                MESSAGE_BEGIN(MSG_ONE, gmsgAmmoPickup, NULL, m_pPlayer->pev);
                WRITE_BYTE(CBasePlayer::GetAmmoIndex("BuffNG7Field"));
                WRITE_BYTE(1);
                MESSAGE_END();
#endif
                m_iFieldShostFired = 0;
            }
        }

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponVolume = BRIGHT_GUN_FLASH;

        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, Penetration, BULLET_PLAYER_762MM, GetDamage(), RangeModifier, m_pPlayer->pev, false, m_pPlayer->random_seed);
        
        int flags = 0;
#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, static_cast<int>(m_pPlayer->pev->punchangle.x * 100), static_cast<int>(m_pPlayer->pev->punchangle.y * 100), FALSE, m_iWeaponState & WPNSTATE_BUFFNG7_MODEB);

        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", 0, 0);
#endif
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;

        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            KickBack(1.15, 0.36, 0.09, 0.165, 4.2, 4.25, 5);
        else if (m_pPlayer->pev->velocity.Length2D() > 0)
            KickBack(0.95, 0.31, 0.14, 0.06, 2.45, 3.75, 1);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            KickBack(0.55, 0.16, 0.06, 0.0275, 1.95, 2.75, 6);
        else
            KickBack(0.6, 0.21, 0.11, 0.03, 2.2, 3.0, 6);
    }

    void BUFFNG7FireB()
    {
        if (m_pPlayer->pev->waterlevel == 3)
        {
            PlayEmptySound();
            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15s;
            return;
        }

        if (m_iClip <= 0)
        {
            Reload();

            if (m_iClip == 0)
                PlayEmptySound();

            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0s;
            return;
        }

        m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

        if (!(m_iWeaponState & WPNSTATE_BUFFNG7_SHOOTB_READY))
        {
            m_iChargeShotsFired++;

            if (m_iState < 8 && m_iChargeShotsFired > m_iTargetShotsFired)
            {
                m_iState++;
                m_iChargeShotsFired = 0;
                pev->skin = m_iState;
            }

            if (m_iState == 8)
            {
                m_iWeaponState |= WPNSTATE_BUFFNG7_SHOOTB_READY;

                EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/buffng7_beep.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

#ifndef CLIENT_DLL
                MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
                WRITE_BYTE(32);
                WRITE_BYTE(0);
                WRITE_BYTE(m_iWeaponState);
                MESSAGE_END();
#endif
                m_iChargeShotsFired = 0;
            }
        }
       
        m_iClip -= 3;
        if (m_iClip <= 0)
            m_iClip = 0;

        if (!m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType])
        {
            m_iFieldShostFired += 3;
            if (m_iFieldShostFired >= FIELD_TARGET_SHOTS_FIRED)
            {
                m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 1;

#ifndef CLIENT_DLL
                MESSAGE_BEGIN(MSG_ONE, gmsgAmmoPickup, NULL, m_pPlayer->pev);
                WRITE_BYTE(CBasePlayer::GetAmmoIndex("BuffNG7Field"));
                WRITE_BYTE(1);
                MESSAGE_END();
#endif
                m_iFieldShostFired = 0;
            }
        }

        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

        float flDamage, flDistance, flSpread;
        duration_t flCycleTime;
        int cShots;

        GetFireInfo(flDamage, flDistance, flSpread, cShots, flCycleTime);

#ifndef CLIENT_DLL
        m_pPlayer->FireBullets(cShots, m_pPlayer->GetGunPosition(), gpGlobals->v_forward, Vector(flSpread, flSpread, 0), flDistance, BULLET_PLAYER_BUCKSHOT, 0, flDamage);
#endif

        int flags;
#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFire, 0, g_vecZero, g_vecZero, flSpread, flSpread, 0, cShots, FALSE, m_iWeaponState & WPNSTATE_BUFFNG7_MODEB);

#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
        m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
        m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;

        if (m_pPlayer->pev->velocity.Length2D() > 0)
            m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 3, 3);
        else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 5, 5);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 1.5, 1.5);
        else
            m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 2, 2);
    }

#ifndef CLIENT_DLL
    void FireBullet3CallBack(int iPenetration, Vector vecDir, TraceResult* tr)
    {    
        Vector vecEnd = tr->vecEndPos - vecDir * 5;
        int iOriginalPenetration;

        if (m_iWeaponState & WPNSTATE_BUFFNG7_SHOOTB_READY)
        {
            iOriginalPenetration = 1;
        }
        else
        {
            if (!(m_iWeaponState & WPNSTATE_BUFFNG7_MODEB))
                return;

            iOriginalPenetration = 7;
        }

        if (iOriginalPenetration == iPenetration)
        {
            if (m_iWeaponState & WPNSTATE_BUFFNG7_SHOOTB_READY)
            {
                float flRadius = 2 * 39.37;
                bool bInWater = UTIL_PointContents(tr->vecEndPos) == CONTENTS_WATER;

                EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/buffng7_exp2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

                MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
                WRITE_BYTE(TE_TEMPSPRITE);
                WRITE_COORD(vecEnd.x);
                WRITE_COORD(vecEnd.y);
                WRITE_COORD(vecEnd.z);
                WRITE_SHORT(m_iExpIndex);
                WRITE_BYTE(6 - int(flRadius * -0.1));
                WRITE_BYTE(255);
                WRITE_BYTE(30);
                MESSAGE_END();

                CBaseEntity* pEntity = NULL;
                while ((pEntity = UTIL_FindEntityInSphere(pEntity, tr->vecEndPos, flRadius)) != NULL)
                {
                    if (pEntity->pev->takedamage == DAMAGE_NO)
                        continue;

                    if (bInWater && pEntity->pev->waterlevel == 0)
                        continue;

                    if (!bInWater && pEntity->pev->waterlevel == 3)
                        continue;

                    if (pEntity == m_pPlayer)
                        continue;

                    pEntity->TakeDamage(m_pPlayer->pev, m_pPlayer->pev, GetDamageC(), DMG_BULLET);
                }
            }
            else
            {
                if (CBaseEntity* pEntity = gMultiDamage.pEntity)
                {
                    if (pEntity->IsPlayer() || pEntity->Classify() == CLASS_PLAYER_ALLY)
                    {
                        if (!m_bEnableDebuff)
                        {
                            m_iDebuffShotsFired++;
                            if (m_iDebuffShotsFired >= DEBUFF_SHOTS_FIRED)
                            {
                                m_TargetVector.clear();
                                m_flDebuffTime = gpGlobals->time + DEBUFF_DURATION_TIME;
                                m_flNextGiveDebuffTime = gpGlobals->time + DEBUFF_INTERVAL;
                                m_bEnableDebuff = true;
                                m_iDebuffShotsFired = 0;
                            }
                        }
                    }
                }
            }
        }
    }
#endif

    void CreateForceField()
    {
        m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;
        SetThink(NULL);
#ifndef CLIENT_DLL
        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

        Vector vecSrc = m_pPlayer->GetGunPosition() + gpGlobals->v_forward;

        Vector vecAngles = UTIL_VecToAngles(gpGlobals->v_forward);

        CBuffNG7ForceField* pField = (CBuffNG7ForceField*)CBaseEntity::Create("buffng7_forcefield", vecSrc, vecAngles, m_pPlayer->edict());
        if(pField)
        {
            pField->pev->velocity = gpGlobals->v_forward * 300.0;
        }
        m_TargetVector.clear();
        m_flDebuffTime = gpGlobals->time + DEBUFF_DURATION_TIME;
        m_flNextGiveDebuffTime = gpGlobals->time + DEBUFF_INTERVAL;
        m_bEnableDebuff = true;
        m_iDebuffShotsFired = 0;
#endif
        
    }

    void ItemPostFrame() override
    {

#ifndef CLIENT_DLL   
        if (m_flDebuffTime != invalid_time_point && m_flDebuffTime < gpGlobals->time)
        {
            m_TargetVector.clear();
            m_bEnableDebuff = false;
            m_flNextGiveDebuffTime = m_flDebuffTime = invalid_time_point;
        }

        if (m_bEnableDebuff)
        {
            if (m_flNextGiveDebuffTime != invalid_time_point && m_flNextGiveDebuffTime < gpGlobals->time)
            {
                m_flNextGiveDebuffTime = gpGlobals->time + DEBUFF_INTERVAL;
                CBaseEntity* pEntity = NULL;
                while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, DEBUFF_RADIUS)) != NULL)
                {
                    if (std::find(m_TargetVector.begin(), m_TargetVector.end(), pEntity->entindex()) != m_TargetVector.end())
                        continue;

                    if (g_pGameRules->PlayerRelationship(m_pPlayer, pEntity) == GR_TEAMMATE)
                        continue;

                    if (pEntity->pev->takedamage == DAMAGE_NO)
                        continue;

                    if (pEntity->edict() == pev->owner)
                        continue;

                    if (!pEntity->IsAlive())
                        continue;

                    MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
                    WRITE_BYTE(TE_BEAMENTS);
                    WRITE_SHORT(m_pPlayer->entindex() | 0x1000);
                    WRITE_SHORT(pEntity->entindex());
                    WRITE_SHORT(m_iDebuffBeamIndex);
                    WRITE_BYTE(0); // framerate
                    WRITE_BYTE(30); // framerate
                    WRITE_BYTE(5); // life
                    WRITE_BYTE(20);  // width
                    WRITE_BYTE(25);   // noise
                    WRITE_BYTE(255);
                    WRITE_BYTE(255);
                    WRITE_BYTE(255);
                    WRITE_BYTE(255);	// brightness
                    WRITE_BYTE(25);		// speed
                    MESSAGE_END();

                    m_TargetVector.push_back(pEntity->entindex());

                    CBuffNG7Debuff* pDebuff = (CBuffNG7Debuff*)CBaseEntity::Create("buffng7_debuff", g_vecZero, g_vecZero, m_pPlayer->edict());
                    if (pDebuff)
                    {
                        pDebuff->m_pTarget = pEntity;
                    }
                }
            }
            
        }

#endif
        return CBasePlayerWeapon::ItemPostFrame();
    }

    void SecondaryAttack(void)
    {
        if (m_iWeaponState & WPNSTATE_BUFFNG7_SHOOTB_READY)
        {
            m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
            m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
            m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

            UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

            Vector vecSrc = m_pPlayer->GetGunPosition();
            Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, 0.03, 3000.0, 1, BULLET_PLAYER_556MM, GetDamageB(), 1.0, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

            int flags = 0;
#ifdef CLIENT_WEAPONS
            flags = FEV_NOTHOST;
#else
            flags = 0;
#endif

            PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, static_cast<int>(m_pPlayer->pev->punchangle.x * 100), static_cast<int>(m_pPlayer->pev->punchangle.y * 100), m_iWeaponState & WPNSTATE_BUFFNG7_SHOOTB_READY, m_iWeaponState & WPNSTATE_BUFFNG7_MODEB);

            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1s;
            m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1s;
            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1s;

            m_iWeaponState &= ~WPNSTATE_BUFFNG7_SHOOTB_READY;
#ifndef CLIENT_DLL
            MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
            WRITE_BYTE(32);
            WRITE_BYTE(0);
            WRITE_BYTE(m_iWeaponState);
            MESSAGE_END();
#endif
            m_iState = 0;
            pev->skin = m_iState;
            return;
        }

        if (m_iWeaponState & WPNSTATE_BUFFNG7_MODEB)
        {
            m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_buffng7.mdl");

            m_iWeaponState &= ~WPNSTATE_BUFFNG7_MODEB;
            SendWeaponAnim(ANIM_A_MODE, UseDecrement() != FALSE);
#ifndef CLIENT_DLL
            MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
            WRITE_BYTE(32);
            WRITE_BYTE(0);
            WRITE_BYTE(m_iWeaponState);
            MESSAGE_END();
#endif
        }
        else
        {
            m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_buffng7_b.mdl");

            m_iWeaponState |= WPNSTATE_BUFFNG7_MODEB;
            SendWeaponAnim(ANIM_B_MODE, UseDecrement() != FALSE);
#ifndef CLIENT_DLL
            MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
            WRITE_BYTE(32);
            WRITE_BYTE(0);
            WRITE_BYTE(m_iWeaponState);
            MESSAGE_END();
#endif
        }

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1s;
        m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1s;
        m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1s;
        m_pPlayer->ResetMaxSpeed();
    }

    void Reload(void)
    {
        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            return;

        if (DefaultReload(MaxClip, m_iWeaponState & WPNSTATE_BUFFNG7_MODEB ? ANIM_RELOAD_B : ANIM_RELOAD, 3.3s))
        {
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
            if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType])
            {
                SetThink(&CBUFFNG7::CreateForceField);
                pev->nextthink = gpGlobals->time + 0.5s;
            }

            m_flAccuracy = 0.2;
            m_iShotsFired = 0;
            m_bDelayFire = false;
        }
    }

    void Holster(int skiplocal)
    {
        SetThink(nullptr);

        return CBasePlayerWeapon::Holster(skiplocal);
    }


    void WeaponIdle(void)
    {
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

        if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
            return;

        m_iChargeShotsFired = 0;

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
        SendWeaponAnim(m_iWeaponState & WPNSTATE_BUFFNG7_MODEB ? ANIM_IDLE_B : ANIM_IDLE1, UseDecrement() != FALSE);
    }

    float GetDamageB() const
    {
        float flDamage = 100;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = 6500;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = 11000;
#endif
        return flDamage;
    }

    float GetDamageC() const
    {
        float flDamage = 50;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = 3500;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = 7500;
#endif
        return flDamage;
    }

};
LINK_ENTITY_TO_CLASS(weapon_buffng7, CBUFFNG7)
}