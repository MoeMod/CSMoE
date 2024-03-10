/*
soulreaper.cpp
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
#ifndef CLIENT_DLL
#include "gamemode/mod_zb1.h"
#endif
#include "weapons/WeaponTemplate.hpp"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

#ifndef CLIENT_DLL
    class CSoulReaper;
    class CSoulReaperBModeProj : public CBaseEntity
    {
    public:
        float GetArmorRatioModifier() { return 0.9; }
        int GetWeaponsId(void) { return WEAPON_SOULREAPER; }

        float GetBModeProjExploDamage() const
        {
            float flDamage = 22.0f;
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flDamage = 5800.0f;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = 14000.0f;
            return flDamage;
        }

        float GetBModeProjExploKnockbackSide() const
        {
            if (g_pModRunning->DamageTrack() == DT_ZB)
                return BModeProjExploKnockbackSideZombie;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                return BModeProjExploKnockbackSideScenario;
            return BModeProjExploKnockbackSide;
        }

        float GetBModeProjExploKnockbackUp() const
        {
            if (g_pModRunning->DamageTrack() == DT_ZB)
                return BModeProjExploKnockbackUpZombie;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                return BModeProjExploKnockbackUpScenario;
            return BModeProjExploKnockbackUp;
        }

        duration_t GetBModeProjExploKnockbackDuration() const
        {        
            if (g_pModRunning->DamageTrack() == DT_ZB)
                return BModeProjExploKnockbackDurationZombie;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                return BModeProjExploKnockbackDurationScenario;
            return BModeProjExploKnockbackDuration;
        }

        void Spawn() override
        {
            Precache();

            SetThink(&CSoulReaperBModeProj::ProjThink);

            SET_MODEL(this->edict(), "sprites/ef_soulreaper_sphere.spr");

            pev->rendermode = kRenderTransTexture;
            pev->renderamt = 255;
            pev->framerate = 30.0;
            pev->scale = 0.02;

            pev->solid = SOLID_NOT;
            pev->movetype = MOVETYPE_NOCLIP;

            pev->nextthink = gpGlobals->time + 0.1s;

            m_flMaxFrames = float(MODEL_FRAMES(pev->modelindex) - 1);

            UTIL_SetOrigin(pev, pev->origin);
            UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));
        }

        void Precache() override
        {
            PRECACHE_MODEL("sprites/ef_soulreaper_sphere.spr");

            PRECACHE_MODEL("sprites/ef_soulreaper_sphere02.spr");   //???

            m_iSprExplo = PRECACHE_MODEL("sprites/ef_soulreaper_exp.spr");
        }

        void EXPORT OnTouch(CBaseEntity* pOther)
        {
            pev->velocity = g_vecZero;

            SetTouch(nullptr);

            RadiusDamage();
        }

        void FindTarget()
        {
            if (m_iType == 1)
                return;

            if (m_hTarget)
                return;

            Vector vecSrc, vecEnd;

            vecSrc = pev->origin;
            CBaseEntity* pEntity = nullptr;

            TraceResult tr;
            float Dis = BModeFindTargetRadius * 39.37;
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

                if (flDelta < Dis)
                {
                    m_hTarget.Set(pEntity->edict());
                    Dis = flDelta;
                }

            }

            if (m_hTarget)
            {
                m_iType = 2;    //chasemode

                pev->solid = SOLID_NOT;
                pev->movetype = MOVETYPE_NOCLIP;
            }


        }

        void Launch(Vector vecSrc, Vector vecDir, float flSpeed)
        {
            pev->solid = SOLID_BBOX;
            pev->movetype = MOVETYPE_FLYMISSILE;

            m_iType = 1;
            pev->scale = 0.05;
            pev->origin = vecSrc;
            pev->velocity = vecDir * flSpeed;

            UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

            SetTouch(&CSoulReaperBModeProj::OnTouch);
        }

        void RadiusDamage()
        {
            Vector vecSrc = pev->origin;

            MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
            WRITE_BYTE(TE_TEMPSPRITE);
            WRITE_COORD(vecSrc.x);
            WRITE_COORD(vecSrc.y);
            WRITE_COORD(vecSrc.z);
            WRITE_SHORT(m_iSprExplo);
            WRITE_BYTE(15);
            WRITE_BYTE(255);
            WRITE_BYTE(30);
            MESSAGE_END();

            UTIL_EMIT_SOUND(this->edict(), CHAN_ITEM, "weapons/lightningsmg-2_exp.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

            TraceResult tr;
            CBaseEntity* pEntity = NULL;
            Vector vecEnd, vecDir;

            float flRadius = BModeProjExploRadius * 39.37;
            float flDamage = GetBModeProjExploDamage();
         
            while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != NULL)
            {
                if (pEntity->pev->takedamage == DAMAGE_NO)
                    continue;

                if (pEntity->pev->health <= 0)
                    continue;

                if (ENT(pEntity->pev) == pev->owner)
                    continue;

                if (g_pGameRules->PlayerRelationship(m_pOwner, pEntity) == GR_TEAMMATE)
                    continue;

                vecEnd = pEntity->BodyTarget(vecSrc);

                UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, edict(), &tr);

                if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
                    continue;

                //Damage dir
                vecDir = (vecEnd - vecSrc).Normalize();

                ClearMultiDamage();
                pEntity->TraceAttack(m_pOwner->pev, flDamage, vecDir, &tr, DMG_BULLET);
                ApplyMultiDamage(pev, m_pOwner->pev);

                //knockback dir
                vecDir = (m_pOwner->GetGunPosition() - vecEnd).Normalize();

                if (pEntity->IsPlayer())
                {
                    CBasePlayer* player = dynamic_cast<CBasePlayer*>(pEntity);
      
                    player->m_flPushBoost = GetBModeProjExploKnockbackSide();
                    player->m_flPushBoostZ = GetBModeProjExploKnockbackUp();
                    player->m_vecPushDirection = vecDir;
                    player->m_flPushDuration = gpGlobals->time + GetBModeProjExploKnockbackDuration();
                }
            }
            

            SetThink(nullptr);
            UTIL_Remove(this);

        }

        void EXPORT ProjThink()
        {
            pev->nextthink = gpGlobals->time + 0.01s;

            if (m_pOwner && (!m_pOwner->IsAlive() || m_pOwner->m_bIsZombie))
            {
                RemoveProj();
                return;
            }

            FindTarget();

            if (m_hTarget)
            {
                Vector vecBody;
                Vector vecIdeal = m_hTarget->BodyTarget(vecBody) - pev->origin;

                pev->velocity = vecIdeal.Normalize() * BModeProjSpeed;

                if (vecIdeal.Length() <= BModeProjExpDist * 39.37)
                {
                    RadiusDamage();
                    return;
                }
            }

            pev->frame += pev->framerate * gpGlobals->frametime / 1s;

            if (pev->frame > m_flMaxFrames)
                pev->frame = fmod(pev->frame, m_flMaxFrames);

            if (m_iType == 0)
            {
                UTIL_MakeVectors(m_pOwner->pev->v_angle);

                Vector vecSrc;
                Vector vecForward = gpGlobals->v_forward;
                Vector vecRight = gpGlobals->v_right;
                vecForward.z = vecRight.z = 0;

                vecSrc = m_pOwner->BodyTarget(vecSrc);
                vecSrc -= vecForward * BModeProjOffset;

                vecRight = vecRight * BModeProjRadius;
                Vector crossProduct = CrossProduct(vecForward, vecRight);

                Vector vecOffset = crossProduct * sin(pev->fuser1 * M_PI / 180) + vecRight * cos(pev->fuser1 * M_PI / 180);

                pev->origin = vecSrc + vecOffset;
              
                pev->fuser1 += 1;
            }
        }

        void RemoveProj()
        {
            SetThink(nullptr);
            UTIL_Remove(this);
        }

        void Init(Vector vecVelocity, int type, CBasePlayer* pOwner, CSoulReaper* pWeapon)
        {
            m_iType = type;

            m_pOwner = pOwner;
            m_pWeapon = pWeapon;

            if (m_iType == 1)
            {
               

                pev->velocity = std::move(vecVelocity);
            }
            else
            {
                m_vecCenter.z += 60;
                m_flRadius = 120.0;
            }
        }

        KnockbackData GetKnockBackData() override { return { 0.0f, 0.0f, 0.0f, 0.0f, 0.6f }; }

        static constexpr const float BModeFindTargetRadius = 5.0f;
        
        static constexpr const float BModeProjExpDist = 1.0f;
        static constexpr const float BModeProjExploRadius = 3.5f;
        static constexpr const float BModeProjSpeed = 1900.0f;

        static constexpr const float BModeProjExploKnockbackSide = 0.0f;
        static constexpr const float BModeProjExploKnockbackUp = 0.0f;
        static constexpr const float BModeProjExploKnockbackSideZombie = 1500.0f;
        static constexpr const float BModeProjExploKnockbackUpZombie = 100.0f;
        static constexpr const float BModeProjExploKnockbackSideScenario = 200.0f;
        static constexpr const float BModeProjExploKnockbackUpScenario = 100.0f;

        static constexpr const auto BModeProjExploKnockbackDuration = 0.0s;
        static constexpr const auto BModeProjExploKnockbackDurationZombie = 0.5s;
        static constexpr const auto BModeProjExploKnockbackDurationScenario = 0.5s;

        static constexpr float BModeProjOffset = 32.0;
        static constexpr float BModeProjRadius = 10.0;

        Vector m_vecCenter;
        float m_flRadius;
        float m_flRadiusDamage;
        float m_flMaxFrames;
        Vector m_vecNormal;
        int m_iSprExplo;
        int m_iType;
        EHANDLE m_hTarget;
        CSoulReaper *m_pWeapon;
        CBasePlayer *m_pOwner;

    };
    LINK_ENTITY_TO_CLASS(soulreaper_bmodeproj, CSoulReaperBModeProj)
#endif

    class CSoulReaper : public LinkWeaponTemplate< CSoulReaper,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_AK47>::template type,
            TPrecacheEvent,
            TGetDamageDefault
    >
    {
    public:
  
    static constexpr const char *V_Model = "models/v_soulreaper.mdl";
    static constexpr const char *P_Model = "models/p_soulreaper.mdl";
    static constexpr const char *W_Model = "models/w_soulreaper.mdl";
    static constexpr const char *EventFile = "events/soulreaper.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_soulreaper";
    static constexpr int MaxClip = 80;
    enum
    {
        ANIM_IDLEA = 0,
        ANIM_SHOOTA,
        ANIM_RELOADA,
        ANIM_DRAWA,
        ANIM_SHOOTBA,
        ANIM_SHOOTBC,
        ANIM_IDLEC,
        ANIM_SHOOTC,
        ANIM_RELOADC,
        ANIM_CHANGECA,
	};

    enum soulreaper_bodygroup_e
    {
        GROUP_WEAPON1,
        GROUP_WEAPON2,
        GROUP_WEAPON3,
        GROUP_WEAPON4,
        GROUP_STONE,
        GROUP_HANDS,
    };

    static constexpr const auto AModeReloadTime = 2.0s;
    static constexpr const auto AModeCycleTime = 0.185s;

    static constexpr const auto deployAttackTime = 0.1s;
    static constexpr const auto deployIdleTime = 1.0s;

    static constexpr const float AModeAccuracy = 0.1f;
    static constexpr const float AModeAccuracyDiv = 240.0f;
    static constexpr const float AModeAccuracyAdd = 0.3f;
    static constexpr const float AModeAccuracyLimit = 0.8f;
    static constexpr const float AModeSpreadJump = 0.2f;
    static constexpr const float AModeSpreadStand = 0.01f;
    static constexpr const float AModeDistance = 8192.0f;
    static constexpr const int AModePenetration = 3;
    static constexpr const float AModeRangeMod = 0.97f;
    static constexpr const float AModeFanAttackMeter = 9.0f;
    static constexpr const float AModeFanAttackAngle = 120.0f;

    static constexpr const auto BModeToAModeDelay = 0.2s;
    static constexpr const auto BModeToBModeDelay = 0.6s;
    static constexpr const auto BModeToCModeDelay = 1.0s;

    static constexpr const int BModeMaxEnergy = 60;
    static constexpr const int BModeChargeEnergy = 2;
    static constexpr const int BModeMakeProjEnergy = 20;

    static constexpr const int CModeChargeMax = 100;
    static constexpr const int CModeChargeAmount = 2;
    static constexpr const int CModeChargeAmountZombie = 2;
    static constexpr const int CModeChargeAmountScenario = 2;

    static constexpr const int CModeChargeAmountWhenKill = 4;
    static constexpr const int CModeChargeAmountWhenKillZombie = 4;
    static constexpr const int CModeChargeAmountWhenKillScenario = 1;
    static constexpr const int CModeChargeAmountWhenAssist = 2;
    static constexpr const int CModeChargeAmountWhenAssistZombie = 2;
    static constexpr const int CModeChargeAmountWhenAssistScenario = 1;

    static constexpr const auto CModeChargeInterval = 0.1s;//0.67s;
    static constexpr const auto CModeChargeIntervalZombie = 0.67s;
    static constexpr const auto CModeChargeIntervalScenario = 0.67s;
    static constexpr const auto CModeChargeDelay = 1.0s;
    static constexpr const auto CModeConsumeInterval = 0.054s;
    static constexpr const int CModeCanChangeGauge = 30;
    static constexpr const int CModeChangeUseGauge = 20;

    static constexpr const auto CModeReloadTime = 2.0s;
    static constexpr const auto CModeCycleTime = 1.15s;

    static constexpr const float CModeActivatedAttackRadius = 6.0f;
    static constexpr const float CModeActivatedAttackKnockbackSide = 0.0f;
    static constexpr const float CModeActivatedAttackKnockbackUp = 0.0f;
    static constexpr const float CModeActivatedAttackKnockbackSideZombie = 1700.0f;
    static constexpr const float CModeActivatedAttackKnockbackUpZombie = 100.0f;
    static constexpr const float CModeActivatedAttackKnockbackSideScenario = 200.0f;
    static constexpr const float CModeActivatedAttackKnockbackUpScenario = 100.0f;
    static constexpr const auto CModeActivatedAttackKnockbackDuration = 0.25s;

    static constexpr const float CModeDistance = 51.0f;
    static constexpr const float CModeRadius = 4.0f;
    static constexpr const auto CModeFixedMoveTime = 2.0s;
    static constexpr const float CModeFixedMoveSpeed = 50.0f;
    static constexpr const float CModeFixedMoveGravity = 2.0f;
    static constexpr const auto CModePullingDuration = 0.0s;
    static constexpr const auto CModePullingDurationZombie = 0.3s;
    static constexpr const auto CModePullingDurationScenario = 0.3s;

    static constexpr const auto BModeFireProjCycleTime = 0.85s;
    static constexpr const float BModeProjSpeed = 1900.0f;

   /* static constexpr const int KillEvasionShotCount = 99999;
    static constexpr const int KillEvasionShotCountZombie = 20;
    static constexpr const int KillEvasionShotCountScenario = 20;
    static constexpr const auto KillEvasionWeaponCooltime = 99999s;
    static constexpr const auto KillEvasionWeaponCooltimeZombie = 80.0s;
    static constexpr const auto KillEvasionWeaponCooltimeScenario = 60.0s;
    static constexpr const auto KillEvasionPlayerCooltime = 99999s;
    static constexpr const auto KillEvasionPlayerCooltimeZombie = 80.0s;
    static constexpr const auto KillEvasionPlayerCooltimeScenario = 60.0s;*/
    static constexpr const auto KillEvasionGodTime = 2.0s;

    static constexpr const float KillEvasionAttackRadius = 7.0f;
    static constexpr const float KillEvasionKnockbackSide = 0.0f;
    static constexpr const float KillEvasionKnockbackSideZombie = 1700.0f;
    static constexpr const float KillEvasionKnockbackSideScenario = 500.0f;
    static constexpr const float KillEvasionKnockbackUp = 0.0f;
    static constexpr const float KillEvasionKnockbackUpZombie = 150.0f;
    static constexpr const float KillEvasionKnockbackUpScenario = 100.0f;
    static constexpr const auto KillEvasionFixedMoveDuration = 2.5s;
    static constexpr const float KillEvasionFixedMoveSpeed = 50.0f;
    static constexpr const float KillEvasionFixedMoveGravity = 2.0f;
    static constexpr const float KillEvasionAttackHeal = 0.0f;
    static constexpr const float KillEvasionAttackHealZombie = 300.0f;
    static constexpr const float KillEvasionAttackHealScenario = 200.0f;
    static constexpr const float KillEvasionAttackHealMax = 0.0f;
    static constexpr const float KillEvasionAttackHealMaxZombie = 1200.0f;
    static constexpr const float KillEvasionAttackHealMaxScenario = 2000.0f;

    static constexpr float PlayerMaxSpeed = 245;
    static constexpr float ArmorRatioModifier = 1.6f;

    static constexpr int DamageDefault = 26;
    static constexpr int DamageZB = 590;
    static constexpr int DamageZBS = 2300;

    static constexpr const int BModeProjSize = 4;
   
    KnockbackData KnockBack = { 650.0f, 600.0f, 700.0f, 600.0f, 0.4f };

    KnockbackData GetKnockBackData() override
    {
        return { 650.0f, 600.0f, 700.0f, 600.0f, 0.4f };
    }

    int m_iExpIndex;
    int m_iHitIndex[2];
    int m_iChargeShotFired;
    int m_iSkillShotFired;
    int m_iEF_Amode;
    int m_iEF_Cmode;
    int m_iEF_Skill;
    int m_iModelIndex;
    int m_iBModeProjAmmo;
    time_point_t m_flNextShootCAttack;
    time_point_t m_flTimeNextCharge;
    time_point_t m_flTimeNextCharge2;
    time_point_t m_flTimeCheckCharge;
    time_point_t m_flWeaponBuffRecordTime;
    EHANDLE m_hProjs[BModeProjSize];

    void Spawn()
    { 
        m_iSkillShotFired = 0;
        m_iChargeShotFired = 0;
        pev->iuser2 = pev->iuser1 = 0;
        pev->iuser1 = 10;
        ResetChargeTimer();
        Base::Spawn();
    }

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "soulreaper";
        p->iMaxAmmo1 = 200;
        p->pszAmmo2 = "soulreaper_c";
        p->iMaxAmmo2 = CModeChargeMax;
        p->iMaxClip = MaxClip;
        p->iSlot = 0;
        p->iPosition = 1;
        p->iId = m_iId = WEAPON_SOULREAPER;
        p->iFlags = 0;
        p->iWeight = 20;

        return 1;
    }

    int ExtractAmmo(CBasePlayerWeapon* pWeapon) override
    {
        if (pev->iuser1)
        {
            if (m_pPlayer)
                m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = pev->iuser1;
            pev->iuser1 = 0;
        }
        return CBasePlayerWeapon::ExtractAmmo(pWeapon);
    }

    float GetArmorRatioModifier() override { return 1.6; }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_soulreaper" , 200 }; }
#endif
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR, 4, 3, 0 }; }

#ifndef CLIENT_DLL
    void SetSubModel()
    {
        pev->modelindex = m_iModelIndex;

        int iSubModel;

        if (m_iWeaponState == 2)
            iSubModel = 1;
        else
            iSubModel = 0;

        if (GetBodygroup(GROUP_STONE) != iSubModel)
        {
            SetBodygroup(GROUP_STONE, iSubModel);
        }

        int hands = 0;

        if (m_pPlayer && m_pPlayer->m_bIsFemale)
            hands = 1;

        SetBodygroup(GROUP_HANDS, hands);

        pev->modelindex = 0;
    }
#endif

    void Precache(void) override
    {
        m_iModelIndex = PRECACHE_MODEL(V_Model);

        m_iEF_Amode = PRECACHE_MODEL("models/ef_soulreaper_Amode.mdl");
        m_iEF_Cmode = PRECACHE_MODEL("models/ef_soulreaper_Cmode.mdl");
        m_iEF_Skill = PRECACHE_MODEL("models/ef_soulreaper_skill.mdl");

        PRECACHE_MODEL("models/ef_soulreaper_shoot_c.mdl");
        PRECACHE_MODEL("models/ef_soulreaper_shoot_c02.mdl");
     
        PRECACHE_MODEL("sprites/ef_soulreaper_amode.spr");
        PRECACHE_MODEL("sprites/ef_soulreaper_bmode.spr");
        PRECACHE_MODEL("sprites/ef_soulreaper_charging.spr");
        PRECACHE_MODEL("sprites/ef_soulreaper_cmode.spr");
        PRECACHE_MODEL("sprites/ef_soulreaper_sphere_c.spr");
        PRECACHE_MODEL("sprites/ef_soulreaper_blue01.spr");
        PRECACHE_MODEL("sprites/ef_soulreaper_blue02.spr");
        PRECACHE_MODEL("sprites/ef_soulreaper_exp.spr");
        PRECACHE_MODEL("sprites/ef_soulreaper_green01.spr");
        PRECACHE_MODEL("sprites/ef_soulreaper_green02.spr");
        PRECACHE_MODEL("sprites/ef_soulreaper_laser.spr");
        PRECACHE_MODEL("sprites/ef_soulreaper_shoot_c.spr");
        PRECACHE_MODEL("sprites/ef_soulreaper_skill.spr");
        PRECACHE_MODEL("sprites/ef_soulreaper_sphere.spr");
        PRECACHE_MODEL("sprites/ef_soulreaper_sphere02.spr");
        PRECACHE_MODEL("sprites/ef_soulreaper_p.spr");
        
        PRECACHE_MODEL("sprites/muzzleflash356.spr");
        PRECACHE_MODEL("sprites/muzzleflash357.spr");
        PRECACHE_MODEL("sprites/muzzleflash358.spr");
        PRECACHE_MODEL("sprites/muzzleflash359.spr");
        PRECACHE_MODEL("sprites/muzzleflash360.spr");
        PRECACHE_MODEL("sprites/muzzleflash361.spr");
        PRECACHE_MODEL("sprites/muzzleflash362.spr");
        return Base::Precache();
    }

    float GetAModeFanAttackDamage() const
    {
        float flDamage = 6.0f;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = 240.0f;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = 1024.0f;
#endif
        return flDamage;
    }

    float GetCModeActivatedAttackDamage() const
    {
        float flDamage = 10.0f;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = 1200.0f;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = 2400.0f;
#endif
        return flDamage;
    }

    float GetCModeDamage() const
    {
        float flDamage = 32.0f;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = 11263.0f;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = 30400.0f;
#endif
        return flDamage;
    }

    float GetKillEvasionDamage() const
    {
        float flDamage = 0.0f;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = 5200.0f;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = 16000.0f;
#endif
        return flDamage;
    }

    duration_t GetChargeTime()
    {
        duration_t flTime = CModeChargeInterval;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flTime = CModeChargeIntervalZombie;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flTime = CModeChargeIntervalScenario;
#endif
        return flTime;
    }

    void ResetChargeTimer()
    {
        if (m_iWeaponState != 2 && pev->iuser1 != CModeChargeMax && m_flTimeNextCharge == invalid_time_point)
            m_flTimeNextCharge = gpGlobals->time + GetChargeTime();
    }

    void ChargeAmmo()
    {
        if (m_flTimeNextCharge != invalid_time_point && m_flTimeNextCharge <= gpGlobals->time)
        {
            if (m_iWeaponState == 2)
            {
                pev->iuser1 -= int((gpGlobals->time - m_flTimeNextCharge) / CModeConsumeInterval + 1.0);

                m_flTimeNextCharge = gpGlobals->time + CModeConsumeInterval;

                if (pev->iuser1 <= 0)
                {
                    pev->skin = 0;
       
                    SendWeaponAnim(ANIM_CHANGECA, UseDecrement() != FALSE);

                    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.7s;
                
                    m_iWeaponState = 0;
                }

                m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = pev->iuser1;
            }
            else
            {
                pev->iuser1 += int((gpGlobals->time - m_flTimeNextCharge) / GetChargeTime()) * CModeChargeAmount + 1.0;

                m_flTimeNextCharge = gpGlobals->time + GetChargeTime();

                if (pev->iuser1 >= CModeCanChangeGauge)
                {
                    m_iWeaponState = 1;
                    pev->skin = 2;
                }
                   
                if (pev->iuser1 >= CModeChargeMax)
                    pev->iuser1 = CModeChargeMax;

                if (pev->iuser1 == CModeChargeMax)
                    m_flTimeNextCharge = invalid_time_point;

                m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = pev->iuser1;
            }
        }
        
    }

    void Holster(int skiplocal)
    {
        ResetChargeTimer();
        StopSound();
        RemoveProj();
        m_pPlayer->m_flWeaponBuffHoldTime[WEAPON_BUFF_SOULREAPER] += gpGlobals->time - m_flWeaponBuffRecordTime;
        CBasePlayerWeapon::Holster();
    }

    void RemoveProj()
    {
#ifndef CLIENT_DLL
        for (int i = 0; i < BModeProjSize; i++)
        {
            if (m_hProjs[i].Get() != NULL)
            {
                ((CSoulReaperBModeProj*)((CBaseEntity*)m_hProjs[i]))->RemoveProj();
            }
        }
#endif
    }

    BOOL Deploy(void) override
    {
        m_iShotsFired = 0;
        m_flAccuracy = AModeAccuracy;

        if (DefaultDeploy(V_Model, P_Model, ANIM_DRAWA, "ak47", UseDecrement() != FALSE, deployAttackTime, deployIdleTime))
        {
            if (pev->iuser1 >= CModeCanChangeGauge)
            {
                m_iWeaponState = 1;
                pev->skin = 2;
            }
            else
            {
                m_iWeaponState = 0;
                pev->skin = 0;
            }

#ifndef CLIENT_DLL
            MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
            WRITE_BYTE(55);
            WRITE_BYTE(pev->iuser2);
            WRITE_BYTE(BModeMaxEnergy);
            MESSAGE_END();

            SetSubModel();
#endif
            RemoveProj();

            m_iBModeProjAmmo = floor(float(pev->iuser2) / float(BModeMakeProjEnergy));

            if (m_iBModeProjAmmo > 0)
            {
                for (int i = 0; i < m_iBModeProjAmmo; i++)
                {
                    CreateProj();
                }

                SetProjAngle();
            }

           
            m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = pev->iuser1;

            ResetChargeTimer();

            return TRUE;
        }

        return FALSE;
    }

    void PrimaryAttack(void) override
    {
        if (m_iWeaponState == 2)
        {
            SoulReaperFireC(CModeCycleTime);
        }
        else
        {
            if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
                SoulReaperFire(AModeSpreadJump * m_flAccuracy, AModeCycleTime, FALSE);
            else
                SoulReaperFire(AModeSpreadStand * m_flAccuracy, AModeCycleTime, FALSE);
        }
     
    }

    void StopSound()
    {
#ifndef CLIENT_DLL
        MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->edict());
        WRITE_BYTE(79);
        WRITE_SHORT(m_pPlayer->entindex());
        WRITE_SHORT(CHAN_ITEM);
        WRITE_STRING("weapons/soulreaper-2.wav");
        MESSAGE_END();
#endif
    }

    void CreateProj()
    {
#ifndef CLIENT_DLL
        int iCount = floor(float(pev->iuser2) / float(BModeMakeProjEnergy));

        int index;
        for (index = 0; index < iCount + 1; index++)
        {
            if (!m_hProjs[index].Get())
                break;
        }

        if (index < iCount)
        {
            //create ent
            CSoulReaperBModeProj* pEnt = static_cast<CSoulReaperBModeProj*>(CBaseEntity::Create("soulreaper_bmodeproj", g_vecZero, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev)));
            if (pEnt)
            {
                m_hProjs[index] = pEnt;
                pEnt->Init(g_vecZero, 0, m_pPlayer, this);
            }
        }
#endif
    }

    void SoulReaperFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_bDelayFire = true;
        m_iShotsFired++;
        m_flAccuracy = ((m_iShotsFired * m_iShotsFired) / AModeAccuracyDiv) + AModeAccuracyAdd;

        if (m_flAccuracy > AModeAccuracyLimit)
            m_flAccuracy = AModeAccuracyLimit;

        if (m_iClip <= 0)
        {
            if (m_fFireOnEmpty)
            {
                PlayEmptySound();
                m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
            }

#ifndef CLIENT_DLL
            if (TheBots != NULL)
            {
                TheBots->OnEvent(EVENT_WEAPON_FIRED_ON_EMPTY, m_pPlayer);
            }
#endif

            return;
        }

        m_iClip--;

        pev->iuser2 += BModeChargeEnergy;
        pev->iuser2 = Clamp(pev->iuser2, 0, BModeMaxEnergy);

        if (pev->iuser2 % BModeMakeProjEnergy == 0)
        {
            CreateProj();
            SetProjAngle();
        }

        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
        Vector vecSrc = m_pPlayer->GetGunPosition();

        Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, AModeDistance, AModePenetration, BULLET_PLAYER_762MM, GetDamage(), AModeRangeMod, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

        int flags = 0;

#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif
        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, static_cast<int>(m_pPlayer->pev->punchangle.x * 100), static_cast<int>(m_pPlayer->pev->punchangle.y * 100), pev->iuser2 % BModeMakeProjEnergy != 0, FALSE);

        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;  

#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

        Vector vecOrigin = m_pPlayer->pev->origin;
        UTIL_MakeVectors(m_pPlayer->pev->v_angle);

        Vector vecForward = gpGlobals->v_forward;

        vecForward.z = 0;
        Vector vecAngle = UTIL_VecToAngles(vecForward);

        if (m_pPlayer->pev->flags & FL_DUCKING)
            vecOrigin.z -= 8.0;
        else
            vecOrigin.z -= 16.0;

        UTIL_TempModel(vecOrigin, vecAngle, g_vecZero, m_iEF_Amode, 7, 0, 10, false, 0, 255, kRenderTransTexture, NULL, false, 0, 10, 0, 0x20000000);

        MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
        WRITE_BYTE(55);
        WRITE_BYTE(pev->iuser2);
        WRITE_BYTE(BModeMaxEnergy);
        MESSAGE_END();
#endif

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + flCycleTime;

        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            KickBack(1.3, 0.55, 0.4, 0.05, 4.75, 3.75, 5);
        else if (m_pPlayer->pev->velocity.Length2D() > 0)
            KickBack(0.9, 0.45, 0.25, 0.035, 3.5, 2.75, 7);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            KickBack(0.75, 0.4, 0.175, 0.03, 2.75, 2.5, 10);
        else
            KickBack(0.775, 0.425, 0.2, 0.03, 3.0, 2.75, 9);
    }

    void SoulReaperFireC(duration_t flCycleTime)
    {
        if (pev->iuser1 <= 0)
            return;

        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

        Vector vecSrc, vecEnd, vecDir;
        vecSrc = m_pPlayer->GetGunPosition();

#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);

        CBaseEntity* pList[100];
        TraceResult tr;

        UTIL_MakeVectors(m_pPlayer->pev->v_angle);
        Vector vecForward = gpGlobals->v_forward;
        vecEnd = vecSrc + vecForward * 39.37 * CModeDistance;

        UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

        const float flWidth = min(39.37 * CModeDistance, (tr.vecEndPos - vecSrc).Length());
        const float flLength = CModeRadius * 39.37;
        const float flHeight = CModeRadius * 39.37;
    
        int iCount = UTIL_EntitiesInRotatedBox(pList, 100, vecSrc, m_pPlayer->pev->v_angle, flLength, flWidth, flHeight, FL_CLIENT | FL_MONSTER);

        for (int i = 0; i < iCount; i++)
        {
            CBaseEntity* pEntity = pList[i];

            if (pEntity == m_pPlayer)
                continue;

            if (pEntity->IsDormant())
                continue;

            if (pEntity->pev->takedamage == DAMAGE_NO)
                continue;

            vecEnd = pEntity->BodyTarget(vecSrc);

            UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

            if (tr.flFraction < 1.0 && tr.pHit != ENT(pEntity->pev))
                continue;

            pEntity->TakeDamage(pev, VARS(pev->owner), GetCModeDamage(), DMG_BULLET | DMG_NEVERGIB);

            if (pEntity->IsPlayer())
            {
                CBasePlayer* player = (CBasePlayer*)pEntity;

                if (player->m_bIsZombie)
                {
                    auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
                    if (strategy)
                    {
                        strategy->SetStunSpeedTime(CModeFixedMoveTime, CModeFixedMoveSpeed);
                        strategy->SetStunGravityTime(CModeFixedMoveTime, CModeFixedMoveGravity);
                    }
                }

                vecDir = (vecEnd - vecSrc).Normalize();

                Vector crossProduct = CrossProduct(vecDir, vecForward);

                if (crossProduct.z < 0) {
                    // 目标在玩家的左侧，给目标往右的速度
                    vecDir = -gpGlobals->v_right;


                    player->m_flPushBoost = 650.0;
                    player->m_flPushBoostZ = 50.0;
                    player->m_vecPushDirection = vecDir;
                    player->m_flPushDuration = gpGlobals->time + CModePullingDurationZombie;
                }
                else if (crossProduct.z > 0) {
                    // 目标在玩家的右侧，给目标往左的速度
                    vecDir = gpGlobals->v_right;

                    player->m_flPushBoost = 650.0;
                    player->m_flPushBoostZ = 50.0;
                    player->m_vecPushDirection = vecDir;
                    player->m_flPushDuration = gpGlobals->time + CModePullingDurationZombie;
                }

                
            }
        }

#endif

        UTIL_MakeVectors(m_pPlayer->pev->v_angle);

        vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, 0.0, 8192, 0, BULLET_PLAYER_556MM, 0, 0, m_pPlayer->pev, false, m_pPlayer->random_seed);

        int flags = 0;

#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif
        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, 2.0, vecDir.y, 0, 0, TRUE, FALSE);

        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

       // UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/soulreaper-3.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
     
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + flCycleTime;
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
    }


    void Reload(void)
    {
        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            return;

        if (DefaultReload(MaxClip, m_iWeaponState == 2 ? ANIM_RELOADC : ANIM_RELOADA, AModeReloadTime))
        {
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
            m_flAccuracy = AModeAccuracy;
            m_iShotsFired = 0;
        }
    }

#ifndef CLIENT_DLL
    void FireBullet3CallBack(int iPenetration, Vector vecDir, TraceResult* pTrace)
    {
        int iOriginalPenetration = AModePenetration;

        if (iOriginalPenetration == iPenetration)
        {
            if (!(m_pPlayer->m_iBuff & BUFF_AVOID_DEATH_WITHCALLBACK))
            {
                if (CBaseEntity* pEntity = gMultiDamage.pEntity)
                {
                    if (pEntity->IsPlayer() || pEntity->Classify() == CLASS_PLAYER_ALLY)
                    {
                        if (g_pGameRules->PlayerRelationship(m_pPlayer, pEntity) != GR_TEAMMATE)
                        {
                            m_pPlayer->m_iWeaponBuffShotFired[WEAPON_BUFF_SOULREAPER]++;
                        }
                    }
                }
            }
        }
    }


#endif

    void SetProjAngle()
    {
#ifndef CLIENT_DLL
        int iProjCount = 0;
        for (int i = 0; i < BModeProjSize; i++)
        {
            if (m_hProjs[i].Get() != NULL)
            {
                if (((CSoulReaperBModeProj*)((CBaseEntity*)m_hProjs[i]))->m_iType == 0)
                    iProjCount++;
            }
        }

        if (!iProjCount)
            return;

        float flAngle = 360 / iProjCount;

        int j = 0;
        for (int i = 0; i < BModeProjSize; i++)
        {
            if (m_hProjs[i].Get() != NULL)
            {
                if (((CSoulReaperBModeProj*)((CBaseEntity*)m_hProjs[i]))->m_iType == 0)
                {
                    ((CSoulReaperBModeProj*)((CBaseEntity*)m_hProjs[i]))->pev->fuser1 = j * 360 / iProjCount;
                    j++;
                }

            }
        }
#endif
    }

    void ItemPostFrame()
    {
        if (m_pPlayer->m_iWeaponBuffShotFired[WEAPON_BUFF_SOULREAPER] >= g_iWeaponBuffTargetShotFired[WEAPON_BUFF_SOULREAPER])
        {
            //满足击中20发条件
            if (m_pPlayer->m_flWeaponBuffCoolTime[WEAPON_BUFF_SOULREAPER] - gpGlobals->time < g_flWeaponBuffNeedCoolTime[WEAPON_BUFF_SOULREAPER])
            {
                //满足在CD内,触发
                //没过180s,
                if (gpGlobals->time - m_flWeaponBuffRecordTime + m_pPlayer->m_flWeaponBuffHoldTime[WEAPON_BUFF_SOULREAPER] > g_flWeaponBuffNeedHoldTime[WEAPON_BUFF_SOULREAPER])
                {
                    //手持超过80秒
                    m_pPlayer->m_iBuff |= BUFF_AVOID_DEATH_WITHCALLBACK;
                }
                    
            }

           
        }

        if (m_iWeaponState == 1 && m_pPlayer->pev->button & IN_ATTACK && m_pPlayer->pev->button & IN_ATTACK2)
        {
            pev->skin = 1;
            m_iWeaponState = 2;
#ifndef CLIENT_DLL
            SetSubModel();

            Vector vecOrigin = g_vecZero;
            if (m_pPlayer->pev->flags & FL_DUCKING)
                vecOrigin.z -= 16.0;
            else
                vecOrigin.z -= 32.0;

            UTIL_TempModel(vecOrigin, g_vecZero, g_vecZero, m_iEF_Cmode, 9, 0, 10, false, 0, 255, kRenderNormal, m_pPlayer, false, 0, 10, 0, 0x20000000);

#endif
            m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = pev->iuser1 -= CModeChangeUseGauge;
            m_flTimeNextCharge = gpGlobals->time;

            SoulReaperFireC(CModeCycleTime);
        }

        ChargeAmmo();
            
        return CBasePlayerWeapon::ItemPostFrame();
    }

    int AddToPlayer(CBasePlayer* pPlayer)
    {
#ifndef CLIENT_DLL
        m_flWeaponBuffRecordTime = gpGlobals->time;
        if(pPlayer->m_iBuff & BUFF_AVOID_DEATH_WITHCALLBACK)
        {
            
        }
#endif
        return CBasePlayerWeapon::AddToPlayer(pPlayer);
    }



    void SecondaryAttack()
    {
        if (pev->iuser2 < BModeMakeProjEnergy)
        {
            m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2s;
            return;
        }
          
        pev->iuser2 -= BModeMakeProjEnergy;
        pev->iuser2 = Clamp(pev->iuser2, 0, BModeMaxEnergy);
    #ifndef CLIENT_DLL
        MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
        WRITE_BYTE(55);
        WRITE_BYTE(pev->iuser2);
        WRITE_BYTE(BModeMaxEnergy);
        MESSAGE_END();
    #endif

        int flags = 0;

    #ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
    #else
        flags = 0;
    #endif
        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

        Vector vecSrc = m_pPlayer->GetGunPosition();
#ifndef CLIENT_DLL
        for (int i = 0; i < BModeProjSize; i++)
        {
            if (m_hProjs[i].Get() != NULL)
            {
                if (((CSoulReaperBModeProj*)((CBaseEntity*)m_hProjs[i]))->m_iType == 0)
                {
                    ((CSoulReaperBModeProj*)((CBaseEntity*)m_hProjs[i]))->Launch(vecSrc, gpGlobals->v_forward, BModeProjSpeed);
                    break;
                }
            }
        }
#endif
        SetProjAngle();

        Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, 0.0, 8192, 0, BULLET_PLAYER_556MM, 0, 0, m_pPlayer->pev, false, m_pPlayer->random_seed);
        vecDir = gpGlobals->v_forward + gpGlobals->v_right * vecDir.x + gpGlobals->v_up * vecDir.y;

        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, static_cast<int>(m_pPlayer->pev->punchangle.x * 100), static_cast<int>(m_pPlayer->pev->punchangle.y * 100), m_iWeaponState != 0, TRUE);

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + BModeFireProjCycleTime;
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + BModeFireProjCycleTime;
    }



    void WpnDeathCallBack(CBaseEntity* pVictim, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override
    {
#ifndef CLIENT_DLL
        Vector vecOrigin = m_pPlayer->pev->origin;
        if (m_pPlayer->pev->flags & FL_DUCKING)
            vecOrigin.z -= 16.0;
        else
            vecOrigin.z -= 32.0;

        CBaseEntity* pEntity = NULL;
        while ((pEntity = UTIL_FindEntityInSphere(pEntity, m_pPlayer->pev->origin, 275.6)) != NULL)
        {
            if (pEntity->IsDormant())
                continue;

            if (pEntity->pev->takedamage == DAMAGE_NO)
                continue;

            if (pEntity->pev == m_pPlayer->pev)
                continue;

            if (UTIL_IsWallBetweenEntity(pEntity, m_pPlayer))
                continue;

            pEntity->TakeDamage(m_pPlayer->pev, m_pPlayer->pev, GetKillEvasionDamage(), DMG_NEVERGIB | DMG_BULLET);

            Vector vecDirection = (pEntity->pev->origin - m_pPlayer->pev->origin).Normalize();
            UTIL_AddKickRateBoost(pEntity, vecDirection.Make2D(), 1.0, 1500.0, 400);
        }

        MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
        WRITE_BYTE(38);
        WRITE_BYTE(0);
        WRITE_BYTE(0);
        MESSAGE_END();

        MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
        WRITE_BYTE(38);
        WRITE_BYTE(1);
        WRITE_BYTE(1);
        MESSAGE_END();

        UTIL_TempModel(vecOrigin, g_vecZero, g_vecZero, m_iEF_Skill, 15, 0, 10, 0, 0, 255, kRenderTransTexture, NULL, false, 0, 0, 256, 0x20000000);

#endif
        m_pPlayer->m_iBuff |= BUFF_AVOID_INFECT;
        m_pPlayer->m_tAvoidInfectBuffTime = gpGlobals->time + KillEvasionGodTime;
        EMIT_SOUND_DYN(this->edict(), CHAN_ITEM, "weapons/soulreaper-2_exp2.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);

    }

    void WeaponIdle(void)
    {
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

        if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
            return;

        if (m_pPlayer->pev->weaponanim == ANIM_CHANGECA)
        {
#ifndef CLIENT_DLL
            SetSubModel();
#endif
        }

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
        SendWeaponAnim(m_iWeaponState == 2 ? ANIM_IDLEC : ANIM_IDLEA, UseDecrement() != FALSE);
    }

};
LINK_ENTITY_TO_CLASS(weapon_soulreaper, CSoulReaper)


}