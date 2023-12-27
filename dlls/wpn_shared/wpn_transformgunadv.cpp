/*
transformgunadv.cpp
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
#ifndef CLIENT_DLL
#include "effects.h"
#include "customentity.h"
#include "monsters.h"
#endif


#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif
    class CTransformGunADV : public LinkWeaponTemplate< CTransformGunADV,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_AK47>::template type,
            TPrecacheEvent,
            TGetDamageDefault,
            TPrimaryAttackRifle,
            TFireRifle,
            TRecoilKickBack
    >
    {
    public:
    static constexpr const char *V_Model = "models/v_transformgunadv.mdl";
    static constexpr const char *P_Model = "models/p_transformgunadv.mdl";
    static constexpr const char *W_Model = "models/w_transformgunadv.mdl";
    static constexpr const char *EventFile = "events/transformgunadv.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_transformgunadv";
    static constexpr const char *AnimExtension = "rifle";
    static constexpr int MaxClip = 60;
	enum
	{
		ANIM_IDLE = 0,
        ANIM_SHOOT1,
        ANIM_SHOOT2,
        ANIM_RELOAD,
        ANIM_DRAW,
        ANIM_SHOOT3,
        ANIM_SHOOT4,
        ANIM_SHOOT5,
        ANIM_IDLE1,
        ANIM_SHOOT11,
        ANIM_SHOOT12,
        ANIM_RELOAD1,
        ANIM_DRAW1,
        ANIM_SHOOT13,
        ANIM_SHOOT14,
        ANIM_SHOOT15,
        ANIM_IDLE2,
        ANIM_SHOOT21,
        ANIM_SHOOT22,
        ANIM_RELOAD2,
        ANIM_DRAW2,
        ANIM_SHOOT23,
        ANIM_SHOOT24,
        ANIM_SHOOT25,
        ANIM_RELOAD01,
        ANIM_RELOAD10,
        ANIM_RELOAD12,
        ANIM_RELOAD20,
        ANIM_RELOAD21,
	};

    enum slash_e
    {
        ANIM_SLASH11,
        ANIM_SLASH12,
        ANIM_SLASH13,
        ANIM_SLASH14,
        ANIM_SLASH21,
        ANIM_SLASH22,
        ANIM_SLASH23,
        ANIM_SLASH24,
        ANIM_SLASH31,
        ANIM_SLASH32,
        ANIM_SLASH33,
        ANIM_SLASH34,
    };

    enum transformgunadv_bodygroup_e
    {
        GROUP_WEAPON1,
        GROUP_GLOW,
        GROUP_HANDS,
        GROUP_WEAPON2,
    };

    enum SpecialAttackMode {
        SpecialAttackMode_1 = 1,
        SpecialAttackMode_2,
        SpecialAttackMode_3,
        SpecialAttackMode_4,
        SpecialAttackMode_5,
    };

    static constexpr float PlayerMaxSpeed = 235;
    static constexpr float ArmorRatioModifier = 1.6f;
	static constexpr float PrimaryAttackWalkingMiniumSpeed = 140;
    static constexpr const auto &SpreadCalcNotOnGround = 0.4*A + 0.04;
    static constexpr const auto &SpreadCalcWalking = 0.0275*A;
    static constexpr const auto &SpreadCalcDefault = 0.03*A;

	static constexpr const auto& CycleTime = 0.1s;
    static constexpr int DamageDefault = 34;
    static constexpr int DamageZB = 139;
    static constexpr int DamageZBS = 169;
    static constexpr const auto &AccuracyCalc = (N * N * N / 220.0) + 0.3;
    static constexpr float AccuracyDefault = 0.2;
    static constexpr float AccuracyMax = 1.0;
    static constexpr float RangeModifier = 0.98;
    static constexpr auto BulletType = BULLET_PLAYER_556MM;
    /*static constexpr*/ int Penetration = 4;

    KnockbackData KnockBack = { 350.0f, 250.0f, 300.0f, 100.0f, 0.5f };


    bool m_bHit;
    int m_iMode;
    int m_iAnim1;
    int m_iAnim2;
    int m_iModelIndex;
    int m_iSlashFx;
    int m_iSlashIndex[3];
    int m_iVmodelFx;
    int m_iDefaultAmmo2;
    time_point_t m_flNextReload;

    time_point_t m_flNextSpecialAttack1;
    time_point_t m_flNextSpecialAttack2;
    time_point_t m_flNextSpecialAttack3;
    time_point_t m_flNextSpecialAttack4;
    time_point_t m_flNextSpecialAttack5;

    int m_iShotsRequired;
    int m_iChargeShotsFired;

    KnockbackData GetKnockBackData() override
    {
        if(pev->iuser1)
            return { 350.0f, 250.0f, 300.0f, 100.0f, 0.3f };
        else
            return { 390.0f, 300.0f, 340.0f, 140.0f, 0.4f };
    }

    void Spawn() override
    {
        m_iChargeShotsFired = 0;
        m_iDefaultAmmo2 = 3;
        Base::Spawn();
        m_iId = WEAPON_TRANSFORMGUNADV;
    }

    int GetItemInfo(ItemInfo* p) override
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "556Nato";
        p->iMaxAmmo1 = MAX_AMMO_762NATO;
        p->pszAmmo2 = "transformgun_b";
        p->iMaxAmmo2 = 3;
        p->iMaxClip = 60;
        p->iSlot = 0;
        p->iPosition = 1;
        p->iId = m_iId = WEAPON_TRANSFORMGUNADV;
        p->iFlags = 0;
        p->iWeight = AK47_WEIGHT;

        return 1;
    }

    int ExtractAmmo(CBasePlayerWeapon* pWeapon)
    {
        if (m_iDefaultAmmo2)
        {
            m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iDefaultAmmo2;
            m_iDefaultAmmo2 = 0;
        }
        return CBasePlayerWeapon::ExtractAmmo(pWeapon);
    }

    float GetArmorRatioModifier() override { return 1.6; }
#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_M4A1); }
#endif
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 5, 3, 140 }; }

    int GetShotsRequired()
    {
        if (!m_iWeaponState)
            return 10;
        else if (m_iWeaponState == 1)
            return 20;
        else
            return 30;
    }


    void SetSubModel()
    {
        pev->modelindex = m_iModelIndex;

        int iSubModel;

        if (m_iWeaponState == 2)
            iSubModel = 2;
        else if (m_iWeaponState == 1)
            iSubModel = 1;
        else
            iSubModel = 0;

        if (GetBodygroup(GROUP_GLOW) != iSubModel)
        {
            SetBodygroup(GROUP_GLOW, iSubModel);
        }

        int hands = 0;

        if (m_pPlayer && m_pPlayer->m_bIsFemale)
            hands = 1;

        SetBodygroup(GROUP_HANDS, hands);

        pev->modelindex = 0;
    }

    void Precache() override
    {
        PRECACHE_MODEL("sprites/muzzleflash341.spr");
        PRECACHE_MODEL("sprites/muzzleflash342.spr");
        PRECACHE_MODEL("sprites/muzzleflash343.spr");
        m_iModelIndex = PRECACHE_MODEL(V_Model);
        m_iSlashFx = PRECACHE_MODEL("models/ef_transformgunadv.mdl");
        m_iVmodelFx = PRECACHE_MODEL("models/v_transformgunadvfx.mdl"); 
        m_iSlashIndex[0] = PRECACHE_MODEL("sprites/ef_slash_trans_up1.spr");
        m_iSlashIndex[1] = PRECACHE_MODEL("sprites/ef_slash_trans_up2.spr");
        m_iSlashIndex[2] = PRECACHE_MODEL("sprites/ef_slash_trans_up3.spr");
        PRECACHE_SOUND("weapons/transformgunadv_beep.wav");
        PRECACHE_SOUND("weapons/transformgunadv-1.wav");
        PRECACHE_SOUND("weapons/transformgunadv_shoot3.wav");
        PRECACHE_SOUND("weapons/transformgunadv_shoot3_1.wav");
        PRECACHE_SOUND("weapons/transformgunadv_shoot4.wav");
        PRECACHE_SOUND("weapons/transformgunadv_shoot4_1.wav");
        PRECACHE_SOUND("weapons/transformgunadv_shoot_end.wav");
        m_usFire = PRECACHE_EVENT(1, EventFile);

        return Base::Precache();
    }


    duration_t GetSpecialAttack()
    {
        return 0.18s;

        if (m_iWeaponState == 2)
            return 0.1s;
        else if (m_iWeaponState == 1)
            return 0.16s;
        else
            return 0.22s;
    }

    float GetDamageAmount()
    {
        if (m_iWeaponState == 2)
            return 10.0;
        else if (m_iWeaponState == 1)
            return 10.0;
        else
            return 13.0;
    }

    float GetRadius()
    {
        if (m_iWeaponState == 2)
            return 393.7;
        else if (m_iWeaponState == 1)
            return 393.7;
        else
            return 393.7;
    }

    float GetAngle()
    {
        if (m_iWeaponState == 2)
            return 360;
        else if (m_iWeaponState == 1)
            return 210;
        else
            return 120;
    }

    void GetKickRate(float& kickrate, float &upboost, float &stunspeed, duration_t &stuntime)
    {
        if (m_iWeaponState == 2)
        {
            kickrate = 1200;
            upboost = 200;
            stunspeed = 120;
            stuntime = 1.5s;
        }
        else if (m_iWeaponState == 1)
        {
            kickrate = 1000;
            upboost = 200;
            stunspeed = 150;
            stuntime = 1.5s;
        }
        else
        {
            kickrate = 900;
            upboost = 0;
            stunspeed = 180;
            stuntime = 1s;
        }
    }

    void SetChargeSkin(void)
    {
        if (!m_iWeaponState)
        {
            if (m_iChargeShotsFired >= 10)
                pev->skin = 5;
            else if (m_iChargeShotsFired >= 8)
                pev->skin = 4;
            else if (m_iChargeShotsFired >= 6)
                pev->skin = 3;
            else if (m_iChargeShotsFired >= 4)
                pev->skin = 2;
            else if (m_iChargeShotsFired >= 2)
                pev->skin = 1;
            else
                pev->skin = 0;
        }
        else if(m_iWeaponState == 1)
        {
            if (m_iChargeShotsFired >= 20)
                pev->skin = 5;
            else if (m_iChargeShotsFired >= 16)
                pev->skin = 4;
            else if (m_iChargeShotsFired >= 12)
                pev->skin = 3;
            else if (m_iChargeShotsFired >= 8)
                pev->skin = 2;
            else if (m_iChargeShotsFired >= 4)
                pev->skin = 1;
            else
                pev->skin = 0;
        }
        else
        {
            if (m_iChargeShotsFired >= 30)
                pev->skin = 5;
            else if (m_iChargeShotsFired >= 24)
                pev->skin = 4;
            else if (m_iChargeShotsFired >= 18)
                pev->skin = 3;
            else if (m_iChargeShotsFired >= 12)
                pev->skin = 2;
            else if (m_iChargeShotsFired >= 6)
                pev->skin = 1;
            else
                pev->skin = 0;
        }
    }

    void Holster(int skiplocal)
    {
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2s;
        m_iMode = 0;
        SetThink(nullptr);
        CBasePlayerWeapon::Holster();
    }

    BOOL Deploy(void) override
    {
        m_iMode = 0;
        m_flAccuracy = 0.2;
        m_iShotsFired = 0;
        m_flNextReload = invalid_time_point;

        if (!m_iWeaponState)
        {
            if (DefaultDeploy(V_Model, P_Model, ANIM_DRAW, AnimExtension, UseDecrement() != FALSE))
            {
                SetSubModel();
                return TRUE;
            }
        }
        else if (m_iWeaponState == 1)
        {
            if (DefaultDeploy(V_Model, P_Model, ANIM_DRAW1, AnimExtension, UseDecrement() != FALSE))
            {
                SetSubModel();
                return TRUE;
            }
        }
        else
        {
            if (DefaultDeploy(V_Model, P_Model, ANIM_DRAW2, AnimExtension, UseDecrement() != FALSE))
            {
                SetSubModel();
                return TRUE;
            }
        }
        return FALSE;
    }

    void PrimaryAttack() override
    {
        if (m_iClip > 0)
        {
            if (!m_iWeaponState)
                SendWeaponAnim(RANDOM_LONG(ANIM_SHOOT1, ANIM_SHOOT2), UseDecrement() != FALSE);
            else if (m_iWeaponState == 1)
                SendWeaponAnim(RANDOM_LONG(ANIM_SHOOT11, ANIM_SHOOT12), UseDecrement() != FALSE);
            else
                SendWeaponAnim(RANDOM_LONG(ANIM_SHOOT21, ANIM_SHOOT22), UseDecrement() != FALSE);
        }

        Base::PrimaryAttack();

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.2s;
    }

    void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_bDelayFire = true;
        m_iShotsFired++;

        if (m_flAccuracy > 1.25)
            m_flAccuracy = 1.25;

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
        m_bHit = false;
        pev->iuser1 = 0;
        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 4, BULLET_PLAYER_556MM, GetDamage(), 0.97, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

        PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, static_cast<int>(m_pPlayer->pev->punchangle.x * 100), static_cast<int>(m_pPlayer->pev->punchangle.y * 100), m_iWeaponState, FALSE);
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
                        m_iChargeShotsFired++;
                        m_bHit = true;
                        SetChargeSkin();

                        if (!m_iWeaponState)
                        {
                            if (m_iChargeShotsFired == GetShotsRequired())
                                EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/transformgunadv_beep.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
                        }
                        else if (m_iWeaponState == 1)
                        {
                            if (m_iChargeShotsFired == GetShotsRequired())
                                EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/transformgunadv_beep.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
                        }
                        else
                        {
                            if (m_iChargeShotsFired == GetShotsRequired())
                                EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/transformgunadv_beep.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
                        }     
                    }
                }
            }
        }
    }

    hit_result_t RadiusAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer, bool boost)
    {
        pev->iuser1 = 1;
        CBaseEntity* pEntity = NULL;
        hit_result_t result = HIT_NONE;
        TraceResult tr;
        UTIL_MakeVectors(pAttackPlayer->pev->v_angle);
        Vector vecEnd;
        Vector vecSrc = pAttackPlayer->GetGunPosition();

        Vector vecForward = gpGlobals->v_forward;
        Vector2D vecForward2D = vecForward.Make2D().Normalize();

        float c = cos(flAngleDegrees * 0.5 * M_PI / 180.0);

        while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != NULL)
        {
            if (pEntity == pAttackPlayer)
                continue;

            if (pEntity->IsDormant())
                continue;

            if (pEntity->pev->takedamage == DAMAGE_NO)
                continue;

            if (pEntity->IsPlayer() && bIsWallBetweenEntity(pEntity, pAttackPlayer))
                continue;

            if (!AngleCheck(pEntity, pAttackPlayer, vecSrc, vecForward, vecForward2D, flRadius, c, &tr))
                continue;

            CBasePlayer* player = dynamic_ent_cast<CBasePlayer*>(pEntity);

            ClearMultiDamage();
            pEntity->m_LastHitGroup = HITGROUP_CHEST;
            pEntity->TraceAttack(pAttackPlayer->pev, flDamage, (tr.vecEndPos - vecSrc).Normalize(), &tr, bitsDamageType);
            pEntity->m_LastHitGroup = HITGROUP_CHEST;
            ApplyMultiDamage(pevInflictor, pAttackPlayer->pev);

            if (boost)
            {
                float kickrate, upboost, stunspeed;
                duration_t stuntime;

                GetKickRate(kickrate, upboost, stunspeed, stuntime);

                if (player && player->m_bIsZombie)
                {
                    auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
                    if (strategy)
                    {
                        strategy->SetStunSpeedTime(stuntime, stunspeed);
                    }
                }

                Vector vecDirection = (pEntity->pev->origin - pAttackPlayer->pev->origin).Normalize();
                UTIL_AddKickRateBoost(pEntity, vecDirection.Make2D(), 1.0, kickrate, upboost);
            }

           

            result = HIT_PLAYER;
        }

        if (!result)
        {
            static int calced = 0;
            static float calcRes = 0.0f;

            if (!(calced & 1))
            {
                calced |= 1;
                calcRes = sqrt(512.0);
            }

            vecEnd = vecSrc + vecForward * (flRadius - calcRes);

            UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, human_hull, pAttackPlayer->edict(), &tr);

            if (tr.flFraction < 1.0f)
                result = HIT_WALL;
        }

        return result;
    }
#endif

    void CreateFX()
    {
#ifndef CLIENT_DLL
        int iAmount = RANDOM_LONG(3, 7);

        float x, y, speed;

        for (int i = 0; i < iAmount; i++)
        {
            UTIL_MakeVectors(m_pPlayer->pev->v_angle);

            x = RANDOM_FLOAT(-1, 1) * 0.5;
            y = RANDOM_FLOAT(0, 1) * 0.5;

            Vector vecForward = gpGlobals->v_forward;
            Vector vecRight = gpGlobals->v_right;
            Vector vecUp = gpGlobals->v_up;
            vecForward = vecForward + vecRight * x + vecUp * y;

            Vector vecAngle = UTIL_VecToAngles(vecForward);
            vecAngle.z = RANDOM_LONG(0, 12) * 30;

            speed = RANDOM_LONG(500, 700);

            pev->modelindex = m_iModelIndex;

            UTIL_TempModel(m_pPlayer->pev->origin, vecAngle, vecForward * speed, m_iSlashFx, 5, 0, 10, 0, 0, 255, kRenderTransAdd, NULL, false, 0, 0, 255, 0x20000000, false, GetBodygroup(GROUP_GLOW));

            pev->modelindex = 0;
        }
      
#endif
    }

    void ItemPostFrame() override
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
            if (!m_iWeaponState)
            {
                if (m_iChargeShotsFired >= GetShotsRequired())
                    m_iWeaponState = 1;
            }
            else if(m_iWeaponState == 1)
            {
                if (m_iChargeShotsFired >= GetShotsRequired())
                    m_iWeaponState = 2;
                else
                    m_iWeaponState = 0;
            }
            else
            {
                if (m_iChargeShotsFired >= GetShotsRequired())
                    m_iWeaponState = 2;
                else
                    m_iWeaponState = 1;
            }

            m_iChargeShotsFired = 0;
            SetSubModel();
            SetChargeSkin();
            m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 3;
        }

        if (m_fInReload)
            return;

        switch (m_iMode)
        {
        case SpecialAttackMode_1:
        {

            m_flNextSpecialAttack1 = gpGlobals->time;
            m_flNextSpecialAttack2 = gpGlobals->time;
            m_flNextSpecialAttack3 = gpGlobals->time;
            m_flNextSpecialAttack4 = gpGlobals->time + GetSpecialAttack() * GetDamageAmount();
            m_flNextSpecialAttack5 = m_flNextSpecialAttack4 + 0.1s;
            m_iMode = SpecialAttackMode_2;
            m_iSwing = 0;

            if (m_iWeaponState == 2)
            {
                m_iAnim1 = ANIM_SHOOT23;
            }
            else if (m_iWeaponState == 1)
            {
                m_iAnim1 = ANIM_SHOOT13;
            }
            else
            {
                m_iAnim1 = ANIM_SHOOT3;
            }

            m_iAnim2 = ANIM_SLASH11;
            break;
        }
        case SpecialAttackMode_2:
        {
            if (gpGlobals->time > m_flNextSpecialAttack1)
            { 
                SendWeaponAnim(m_iAnim1, UseDecrement() != FALSE);

                m_iAnim1++;

                if (m_iWeaponState == 2)
                {
                    if (m_iAnim1 > ANIM_SHOOT24)
                        m_iAnim1 = ANIM_SHOOT23;
                }
                else if (m_iWeaponState == 1)
                {
                    if (m_iAnim1 > ANIM_SHOOT14)
                        m_iAnim1 = ANIM_SHOOT13;
                }
                else
                {
                    if (m_iAnim1 > ANIM_SHOOT4)
                        m_iAnim1 = ANIM_SHOOT3;
                }

                m_iAnim2 += 1;
                if (m_iAnim2 == ANIM_SLASH34)
                {
                    m_iAnim2 = ANIM_SLASH11;
                }
                m_flNextSpecialAttack1 = get_gametime() + 0.47s;
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5s;
            }

            if (gpGlobals->time > m_flNextSpecialAttack2)
            {
                float time;
                if (m_iAnim2 % 2)
                    time = 0.33;
                else
                    time = 0.16;

                pev->modelindex = m_iModelIndex;

                PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0.0, g_vecZero, g_vecZero, time, 0.0, m_iAnim2, GetBodygroup(GROUP_GLOW), m_pPlayer->m_bIsFemale, TRUE); // show additional vmodel
               
                pev->modelindex = 0;

                m_iAnim2 += 1;
                if (m_iAnim2 == ANIM_SLASH34)
                {
                    m_iAnim2 = ANIM_SLASH11;
                }
                m_flNextSpecialAttack2 = get_gametime() + time * 1s;

                CreateFX();
            }

            if (gpGlobals->time > m_flNextSpecialAttack3)
            {
                m_flNextSpecialAttack3 = gpGlobals->time + GetSpecialAttack();
                Q_strcpy(m_pPlayer->m_szAnimExtention, "m16a1ep");
#ifndef CLIENT_DLL
                switch (RadiusAttack(GetDamageB(), GetRadius(), GetAngle(), DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer, false))
                {
                case HIT_NONE:
                {
                    if ((m_iSwing++) % 2)
                        EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/transformgunadv_shoot3.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
                    else
                        EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/transformgunadv_shoot4.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
             
                    break;
                }
                case HIT_PLAYER:
                {

                    if ((m_iSwing++) % 2)
                        EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/transformgunadv_shoot3_1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
                    else
                        EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/transformgunadv_shoot4_1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

                    m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
                    break;
                }
                case HIT_WALL:
                {
                    if ((m_iSwing++) % 2)
                        EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/transformgunadv_shoot3.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
                    else
                        EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/transformgunadv_shoot4.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

                    break;
                }
                }

                m_pPlayer->SetAnimation(PLAYER_ATTACK2);
#endif
            }

            if (gpGlobals->time > m_flNextSpecialAttack4)
            {
                Q_strcpy(m_pPlayer->m_szAnimExtention, "m16a1ep");

                int iAnim = ANIM_SHOOT5;
                if (m_iWeaponState == 2)
                {
                    iAnim = ANIM_SHOOT25;
                }
                else if (m_iWeaponState == 1)
                    iAnim = ANIM_SHOOT15;
                SendWeaponAnim(iAnim, UseDecrement() != FALSE);
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1s;
                m_iMode = SpecialAttackMode_3;
            }

            break;
            //Q_strcpy(m_pPlayer->m_szAnimExtention, "rifle");
        }
        case SpecialAttackMode_3:
        {
            if (gpGlobals->time > m_flNextSpecialAttack5)
            {
#ifndef CLIENT_DLL
                RadiusAttack(GetDamageC(), GetRadius(), GetAngle(), DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer, true);

                EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/transformgunadv_shoot_end.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

                m_pPlayer->SetAnimation(PLAYER_ATTACK2);
#endif
                m_iMode = 0;
                m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1s;
                Q_strcpy(m_pPlayer->m_szAnimExtention, AnimExtension);
            }
            break;
        }
        }

        return CBasePlayerWeapon::ItemPostFrame();
    }

    void SecondaryAttack() override
    {
        if (!m_iMode)
        {
            if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] > 0)
            {
                m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;
                m_iMode = SpecialAttackMode_1;
                m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 99s;
                //m_flNextPrimaryAttack = m_flNextSecondaryAttack = duration_t::max(); // 0x7F7FFFFF;
            }
        }
    }

    void Reload(void) override
    {
        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            return;

        int iAnim;

        if (!m_iWeaponState)
        {
            if (m_iChargeShotsFired >= GetShotsRequired())
                iAnim = ANIM_RELOAD01;
            else
                iAnim = ANIM_RELOAD;
        }
        else if (m_iWeaponState == 1)
        {
            if (m_iChargeShotsFired >= GetShotsRequired())
                iAnim = ANIM_RELOAD12;
            else
                iAnim = ANIM_RELOAD10;
        }
        else
        {
            if (m_iChargeShotsFired >= GetShotsRequired())
                iAnim = ANIM_RELOAD2;
            else
                iAnim = ANIM_RELOAD21;    
        }
           

        if (DefaultReload(60, iAnim, 2.5s, 1))
        {
            m_flNextReload = gpGlobals->time + 2.5s;

#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
            m_flAccuracy = 0.2;
            m_iShotsFired = 0;
            m_bDelayFire = false;
        }
    }


    void WeaponIdle(void) override
    {
        ResetEmptySound();

        m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

        if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
            return;

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;

        if (!m_iWeaponState)
        {
            if (m_pPlayer->pev->weaponanim != ANIM_IDLE)
                SendWeaponAnim(ANIM_IDLE, UseDecrement() != FALSE);
        }
        else if (m_iWeaponState == 1)
        {
            if (m_pPlayer->pev->weaponanim != ANIM_IDLE1)
                SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);
        }
        else
        {
            if (m_pPlayer->pev->weaponanim != ANIM_IDLE2)
                SendWeaponAnim(ANIM_IDLE2, UseDecrement() != FALSE);
        }
    }

    float GetDamageA() const
    {
        float flDamage;
        if (m_iWeaponState == 2)
        {
            flDamage = 36.0;
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flDamage = 374.0f;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = 1448.0f;
#endif
        }
        else if (m_iWeaponState == 1)
        {
            flDamage = 31.0;
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flDamage = 330.0f;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = 1260.0f;
#endif
        }
        else
        {
            flDamage = 26.0;
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flDamage = 286.0f;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = 1038.0f;
#endif
        }
        return flDamage;
    }

    float GetDamageB() const
    {
        float flDamage;
        if (m_iWeaponState == 2)
        {
            flDamage = 6.0;
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flDamage = 1720.0f;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = 1448.0f;
#endif
        }
        else if (m_iWeaponState == 1)
        {
            flDamage = 5.0;
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flDamage = 1658.0f;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = 1756.0f;
#endif
        }
        else
        {
            flDamage = 4.0;
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flDamage = 1420.0f;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = 1610.0f;
#endif
        }
        return flDamage;
    }

    float GetDamageC() const
    {
        float flDamage;
        if (m_iWeaponState == 2)
        {
            flDamage = 20.0;
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flDamage = 3830.0f;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = 5950.0f;
#endif
        }
        else if (m_iWeaponState == 1)
        {
            flDamage = 12.0;
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flDamage = 2960.0f;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = 3840.0f;
#endif
        }
        else
        {
            flDamage = 10.0;
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flDamage = 2040.0f;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = 2380.0f;
#endif
        }
        return flDamage;
    }

};
LINK_ENTITY_TO_CLASS(weapon_transformgunadv, CTransformGunADV)
}