/*
lightningsmg.cpp
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
#ifndef CLIENT_DLL
#include "gamemode/mod_zb1.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

#ifndef CLIENT_DLL
    class CLightningSMGElectroField : public CBaseAnimating
    {
    public:
        float GetArmorRatioModifier() { return 0.9; }
        int GetWeaponsId(void) { return WEAPON_LIGHTNINGSMG; }

        void Spawn() override
        {
            Precache();

            SetThink(&CLightningSMGElectroField::ElectroFieldThink);

            SET_MODEL(this->edict(), "models/ef_lightningsmg.mdl");

            pev->rendermode = kRenderTransAdd;
            pev->renderamt = 255;
            pev->framerate = 1.0;
            pev->solid = SOLID_NOT;
            pev->movetype = MOVETYPE_NOCLIP;
            pev->nextthink = gpGlobals->time + 0.1s;
 
            m_flRemoveTime = gpGlobals->time + 3.0s;
            m_flFieldFadeoutTime = invalid_time_point;
            UTIL_SetOrigin(pev, pev->origin);
            UTIL_SetSize(pev, Vector(-1.0, -1.0, -1.0), Vector(1.0, 1.0, 1.0));
        }

        void Precache() override
        {
            PRECACHE_MODEL("models/ef_lightningsmg.mdl");
  
        }

        void Attack()
        {
            TraceResult tr;
            float flDamage;
            CBaseEntity* pList[100];
            Vector vecStart;

            vecStart = pev->origin + Vector(0.0, 0.0, 1.0);

            const float flRange = FieldRadius * 39.37;
            int iCount = UTIL_EntitiesInBox(pList, 100, vecStart - Vector(flRange, flRange, flRange), vecStart + Vector(flRange, flRange, flRange), FL_CLIENT | FL_MONSTER);

            if (g_pModRunning->DamageTrack() == DT_ZBS)
            {
                flDamage = FieldTickDamage_Scenario;
            }
            else if (g_pModRunning->DamageTrack() == DT_ZB)
            {
                flDamage = FieldTickDamage_Zombie;
            }
            else
            {
                flDamage = FieldTickDamage;
            }

            CBasePlayer* pOwner = (CBasePlayer*)CBaseEntity::Instance(pev->owner);

            if (pOwner)
            {
                if (pOwner->IsPlayer() && pOwner->m_bIsZombie)
                    flDamage = 0.0;

                for (int i = 0; i < iCount; i++)
                {
                    CBaseEntity* pEntity = pList[i];

                    if (pEntity == this)
                        continue;

                    if (pEntity->pev->takedamage == DAMAGE_NO)
                        continue;

                    if (pEntity->edict() == pev->owner)
                        continue;

                    if (g_pGameRules->PlayerRelationship(pOwner, pEntity) == GR_TEAMMATE)
                        continue;

                    UTIL_TraceLine(vecStart, pEntity->BodyTarget(vecStart), dont_ignore_monsters, edict(), &tr);

                    if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
                        continue;

                    pEntity->TakeDamage(pev, VARS(pev->owner), flDamage, DMG_GENERIC);

                    if (pEntity->IsPlayer())
                    {
                        CBasePlayer* player = dynamic_cast<CBasePlayer*>(pEntity);

                        if (player->m_bIsZombie)
                        {
                            auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
                            if (strategy)
                            {
                                strategy->SetStunSpeedTime(FieldExpFixedMoveDuration, FieldFixedMoveSpeed);
                            }
                        }
                        else
                        {
                            player->m_tLockBuffTime = gpGlobals->time + FieldExpFixedMoveDuration;
                            player->m_flLockSpeed = FieldFixedMoveSpeed;
                            player->ResetMaxSpeed();                        
                        }
                    }
                }
            }
        }

        void EXPORT ElectroFieldThink()
        {
            pev->nextthink = gpGlobals->time + 0.1s;

            if (m_flFieldFadeoutTime != invalid_time_point)
            {
                duration_t interval = gpGlobals->time - m_flFieldFadeoutTime;

                if (interval > FieldFadeoutTime)
                {
                    m_flFieldFadeoutTime = invalid_time_point;
                    pev->nextthink = invalid_time_point;
                    SetThink(nullptr);
                    UTIL_Remove(this);
                }
                else
                {
                    pev->rendermode = kRenderTransAlpha;
                    pev->renderamt = (1.0 - interval / FieldFadeoutTime) * FieldAmt;
                }
            }

            if (m_flNextAttackTime < gpGlobals->time)
            {           
                Attack();

                m_flNextAttackTime = gpGlobals->time + FieldTick;
            }

            if (m_flRemoveTime != invalid_time_point && m_flRemoveTime < gpGlobals->time)
            {
                m_flFieldFadeoutTime = gpGlobals->time;
                m_flRemoveTime = invalid_time_point;
            }
        }

        KnockbackData GetKnockBackData() override { return { 0.0f, 0.0f, 0.0f, 0.0f, 0.6f }; }

       
        static constexpr float FieldTickDamage = 9.0;
        static constexpr float FieldTickDamage_Zombie = 300.0;
        static constexpr float FieldTickDamage_Scenario = 450.0;
        static constexpr auto FieldTick = 0.3s;
        static constexpr auto FieldExpFixedMoveDuration = 1.0s;
        static constexpr float FieldFixedMoveSpeed = 150.0;
        //static constexpr float FieldFixedMoveGravity = 0.0;
        static constexpr float FieldRadius = 6.0;
        static constexpr float FieldAmt = 100.0;
        static constexpr float FieldScale = 1.25;
        static constexpr auto FieldFadeoutTime = 1.25s;

        time_point_t m_flRemoveTime;
        time_point_t m_flNextAttackTime;
        time_point_t m_flFieldFadeoutTime;
    };
    LINK_ENTITY_TO_CLASS(lightningsmg_electro_field, CLightningSMGElectroField)

    class CLightningSMGElectroMissile : public CBaseAnimating
    {
    public:
        float GetArmorRatioModifier() { return 0.9; }
        int GetWeaponsId(void) { return WEAPON_LIGHTNINGSMG; }

        float GetMissileExpDamageRadius() const
        {
            if (g_pModRunning->DamageTrack() == DT_ZB)
                return MissileExpDamageRadiusZombie * 39.37;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                return MissileExpDamageRadiusScenario * 39.37;
            return MissileExpDamageRadius * 39.37;
        }

        float GetMissileChaseRadius() const
        {
            if (g_pModRunning->DamageTrack() == DT_ZB)
                return MissileChaseRadiusZombie * 39.37;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                return MissileChaseRadiusScenario * 39.37;
            return MissileChaseRadius * 39.37;
        }

        float GetMissileDamage() const
        {
            if (g_pModRunning->DamageTrack() == DT_ZB)
                return MissileDamageZombie;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                return MissileDamageScenario;
            return MissileDamage;
        }

        void Spawn() override
        {
            Precache();

            SetThink(&CLightningSMGElectroMissile::MissileThink);

            SET_MODEL(this->edict(), "sprites/ef_lightningsmg_missile.spr");

            pev->rendermode = kRenderTransAdd;
            pev->renderamt = GlobeAmt;
            pev->framerate = GlobeFramerate;
            pev->scale = GlobeScale;

            pev->solid = SOLID_NOT;
            pev->movetype = MOVETYPE_NOCLIP;
           
            pev->nextthink = gpGlobals->time + 0.1s;

            m_flMaxFrames = float(MODEL_FRAMES(pev->modelindex) - 1);
            m_flRemoveTime = gpGlobals->time + MissileLife;
            m_flFieldFadeoutTime = invalid_time_point;

            UTIL_SetOrigin(pev, pev->origin);
            UTIL_SetSize(pev, Vector(-1.0, -1.0, -1.0), Vector(1.0, 1.0, 1.0));
        }

        void Precache() override
        {
            PRECACHE_MODEL("sprites/ef_lightningsmg_missile.spr");

            m_iSprExplo = PRECACHE_MODEL("sprites/ef_lightningsmg_explo.spr");
        }

        void EXPORT OnTouch(CBaseEntity* pOther)
        {
            pev->velocity = g_vecZero;

            SetTouch(nullptr);

            RadiusDamage();
        }

        bool CheckEnt(CBaseEntity * pEnt)
        {
            CBaseEntity* pMissile = nullptr;
            while ((pMissile = UTIL_FindEntityByClassname(pMissile, "lightningsmg_electro_missile")) != NULL)
            {
                CLightningSMGElectroMissile* missile = dynamic_ent_cast<CLightningSMGElectroMissile*>(pMissile);
                if (missile == nullptr)
                    continue;

                if (missile->pev->iuser2 == pev->iuser2)
                {
                    //find same batch
                    if (!missile->m_hTarget)
                        return true;

                    if (missile->m_hTarget->entindex() == pEnt->entindex())
                        return false;
                }
            }

            return true;
        }

        void FindTarget()
        {
            if (m_hTarget)
                return;

            Vector vecSrc, vecEnd;

            vecSrc = pev->origin;
            CBaseEntity* pEntity = nullptr;
            
            TraceResult tr;
            float Dis = GetMissileChaseRadius();
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

                if (!CheckEnt(pEntity))
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

        void RadiusDamage()
        {
            Vector vecSrc = pev->origin;

            MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
            WRITE_BYTE(TE_TEMPSPRITE);
            WRITE_COORD(vecSrc.x);
            WRITE_COORD(vecSrc.y);
            WRITE_COORD(vecSrc.z);
            WRITE_SHORT(m_iSprExplo);
            WRITE_BYTE(GlobeExpScale * 10);
            WRITE_BYTE(GlobeExpAmt);
            WRITE_BYTE(30);
            MESSAGE_END();

            UTIL_EMIT_SOUND(this->edict(), CHAN_ITEM, "weapons/lightningsmg-2_exp.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

            TraceResult tr;
            CBaseEntity* pEntity = NULL;
            CBasePlayer* pOwner = (CBasePlayer*)CBaseEntity::Instance(pev->owner);

            float flRadius = GetMissileExpDamageRadius();
            float flDamage = GetMissileDamage();
            if (pOwner)
            {  
                while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != NULL)
                {
                    if (pEntity->pev->takedamage == DAMAGE_NO)
                        continue;

                    if (pEntity->pev->health <= 0)
                        continue;

                    if (ENT(pEntity->pev) == pev->owner)
                        continue;

                    if (g_pGameRules->PlayerRelationship(pOwner, pEntity) == GR_TEAMMATE)
                        continue;

                    UTIL_TraceLine(vecSrc, pEntity->BodyTarget(vecSrc), dont_ignore_monsters, edict(), &tr);

                    if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
                        continue;

                    pEntity->m_LastHitGroup = HITGROUP_CHEST;
                    pEntity->TakeDamage(pev, pOwner->pev, flDamage, DMG_GENERIC);

                    if (pEntity->IsPlayer())
                    {
                        CBasePlayer* player = dynamic_cast<CBasePlayer*>(pEntity);

                        if (player->m_bIsZombie)
                        {
                            auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
                            if (strategy)
                            {
                                strategy->SetStunSpeedTime(MissileFixedMoveDuration, MissileFixedMoveSpeed);
                            }
                        }
                        else
                        {
                            player->m_tLockBuffTime = gpGlobals->time + MissileFixedMoveDuration;
                            player->m_flLockSpeed = MissileFixedMoveSpeed;
                            player->ResetMaxSpeed();
                        }
                    }
                }
            }

            SetThink(nullptr);
            UTIL_Remove(this);

        }

        void EXPORT MissileThink()
        {
            pev->nextthink = gpGlobals->time + 0.01s;

            FindTarget();
            
            if (m_hTarget)
            {
                Vector vecBody;
                Vector vecIdeal = m_hTarget->BodyTarget(vecBody) - pev->origin;

                pev->velocity = vecIdeal.Normalize() * MissileChaseSpeed;

                if (vecIdeal.Length() <= MissileExpRadius * 39.37)
                {
                    RadiusDamage();
                    return;
                }
            }

            if (m_flFieldFadeoutTime != invalid_time_point)
            {
                duration_t interval = gpGlobals->time - m_flFieldFadeoutTime;

                if (interval > FieldFadeoutTime)
                {
                    m_flFieldFadeoutTime = invalid_time_point;
                    pev->nextthink = invalid_time_point;
                    SetThink(nullptr);
                    UTIL_Remove(this);
                }
                else
                {
                    pev->renderamt = (1.0 - interval / FieldFadeoutTime) * GlobeAmt;
                }
            }

            pev->frame += pev->framerate * gpGlobals->frametime / 1s;

            if (pev->frame > m_flMaxFrames)
                pev->frame = fmod(pev->frame, m_flMaxFrames);
       
            if (m_iType == 0)
            {
                Vector vecA = CrossProduct(m_vecNormal, {1, 0, 0} );
                if (vecA.IsZero())
                {
                    vecA = CrossProduct(m_vecNormal, { 0, 1, 0 });
                }

                Vector vecB = CrossProduct(m_vecNormal, vecA);
                vecB = vecB.Normalize();
                vecA = vecA.Normalize();

                pev->origin.x = m_vecCenter.x + m_flRadius * vecA.x * cos(pev->fuser1 * M_PI / 180.0) + m_flRadius * vecB.x * sin(pev->fuser1 * M_PI / 180.0);
                pev->origin.y = m_vecCenter.y + m_flRadius * vecA.y * cos(pev->fuser1 * M_PI / 180.0) + m_flRadius * vecB.y * sin(pev->fuser1 * M_PI / 180.0);
                pev->origin.z = m_vecCenter.z + m_flRadius * vecA.z * cos(pev->fuser1 * M_PI / 180.0) + m_flRadius * vecB.z * sin(pev->fuser1 * M_PI / 180.0);

                pev->fuser1 += 5.0;
            }

            if (m_flRemoveTime != invalid_time_point && m_flRemoveTime < gpGlobals->time)
            {
                m_flFieldFadeoutTime = gpGlobals->time;
                m_flRemoveTime = invalid_time_point;
            }
        }

        void Init(Vector vecVelocity, int type, Vector vecCenter)
        {
            m_iType = type;

            pev->fuser1 = RANDOM_LONG(-180, 180);

            if (m_iType == 1)
            {
                pev->solid = SOLID_BBOX;
                pev->movetype = MOVETYPE_FLYMISSILE;

                SetTouch(&CLightningSMGElectroMissile::OnTouch);

                pev->velocity = std::move(vecVelocity);
            }
            else
            {
                m_vecCenter = vecCenter;
                m_vecCenter.z += 60;
                m_flRadius = 120.0;
            }        
        }

        KnockbackData GetKnockBackData() override { return { 0.0f, 0.0f, 0.0f, 0.0f, 0.6f }; }

        static constexpr float GlobeAngularVel = 250.0;
        static constexpr float GlobeAmt = 200.0;
        static constexpr int GlobeExpAmt = 200;
        static constexpr float GlobeExpScale = 1.5;
        static constexpr float GlobeScale = 0.3;
        static constexpr float GlobeFramerate = 2.0;

        //static constexpr float MissileSize = 0.0;
        static constexpr auto MissileLife = 3.0s;  
        static constexpr float MissileChaseSpeed = 1000.0;
        static constexpr float MissileExpDamageRadius = 2.0;
        static constexpr float MissileExpDamageRadiusZombie = 3.0;
        static constexpr float MissileExpDamageRadiusScenario = 4.0;
        static constexpr float MissileExpRadius = 1.0;
        //static constexpr float MissileExpRadiusZombie = 1.0;
        //static constexpr float MissileExpRadiusScenario = 1.0;
        static constexpr float MissileChaseRadius = 3.0;
        static constexpr float MissileChaseRadiusZombie = 4.0;
        static constexpr float MissileChaseRadiusScenario = 5.0;
        static constexpr float MissleChaseAngleAccel = 360.0;

        static constexpr float MissileDamage = 25.0;
        static constexpr float MissileDamageZombie = 1800.0;
        static constexpr float MissileDamageScenario = 350.0;

        static constexpr float MissileFixedMoveSpeed = 150.0;
       // static constexpr float MissileFixedMoveGravity = 0.0;
        static constexpr auto MissileFixedMoveDuration = 1.0s;
        static constexpr float MissileShootPos = 15.0;
        static constexpr auto FieldFadeoutTime = 1.25s;

        time_point_t m_flRemoveTime;
        time_point_t m_flFieldFadeoutTime;
        Vector m_vecCenter;
        float m_flRadius;
        float m_flRadiusDamage;
        float m_flMaxFrames;
        Vector m_vecNormal;
        int m_iSprExplo;
        int m_iType;
        EHANDLE m_hTarget;

    };
    LINK_ENTITY_TO_CLASS(lightningsmg_electro_missile, CLightningSMGElectroMissile)
#endif

    class CLightningSMG : public LinkWeaponTemplate< CLightningSMG,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_P90>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TReloadDefault,
            TGetDamageDefault,
			TPrimaryAttackRifle
    >
    {
    public:
	static constexpr const auto& DefaultDeployTime = 0.53s;
    static constexpr const auto &DefaultReloadTime = 2.33s;
    static constexpr const char *V_Model = "models/v_lightningsmg.mdl";
    static constexpr const char *P_Model = "models/p_lightningsmg_a.mdl";
    static constexpr const char *W_Model = "models/w_lightningsmg.mdl";
    static constexpr const char *EventFile = "events/lightningsmg.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_lightningsmg";
    static constexpr const char *AnimExtension = "mp5";
    static constexpr int MaxClip = 80;
	enum
	{
		ANIM_IDLE1 = 0,
		ANIM_RELOAD,
		ANIM_DRAW,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_SHOOT3,
		ANIM_CHARGE1,
		ANIM_CHARGE_FINISH,
		ANIM_CHARGE2,
		ANIM_CHARGE_SHOOT,
	};

    enum transformgunadv_bodygroup_e
    {
        GROUP_WEAPON1,
        GROUP_WEAPON2,
        GROUP_EFFECT,
        GROUP_HANDS,
    };

    static constexpr const auto& SpreadCalcNotOnGround = 0.2 * A;
    static constexpr const auto& SpreadCalcDefault = 0.04 * A;

    static constexpr const auto& CycleTime = 0.07s;

    static constexpr float PlayerMaxSpeed = 255;
    static constexpr float ArmorRatioModifier = 1.2f;

    static constexpr int DamageDefault = 26;
    static constexpr int DamageZB = 180;
    static constexpr int DamageZBS = 350;

    static constexpr float AccuracyDefault = 0.0;
    static constexpr float AccuracyMax = 0.7;

    static constexpr int ChargingCount = 35;
    static constexpr int ChargingCountForBuffClass23s5 = 30;
    static constexpr int MaxChargeCount = 3;

    static constexpr auto SecondaryCycleTime = 0.7s;
    static constexpr auto SecondaryChargingTime = 0.5s;

    static constexpr float FieldExpDamage = 25.0;
    static constexpr float FieldExpDamage_Zombie = 1800.0;
    static constexpr float FieldExpDamage_Scenario = 350.0;
    static constexpr float FieldExpRadius = 3.0;
    static constexpr float FieldExpFixedMoveSpeed = 100.0;
    static constexpr float FieldExpFixedMoveGravity = 100.0;

    static constexpr int GlobeCount = 3;
  
 
    static constexpr float MissileSpeed = 1000.0;
    static constexpr auto MissileShootDelay = 0.3s;
    
    KnockbackData KnockBack = { 250.0f, 200.0f, 250.0f, 90.0f, 0.7f };


    time_point_t m_flTimeCheckCharge;
    int m_iModelIndex;
    int m_iChargeState;
    int m_iChargeShotsFired;

	void Spawn()
	{
        m_iChargeState = 0;
        m_flTimeCheckCharge = invalid_time_point;
        m_iChargeShotsFired = 0;
        m_iModelIndex = 0;
        Base::Spawn();
	}

    void Precache(void)
    {
        m_iModelIndex = PRECACHE_MODEL(V_Model);
        Base::Precache();

		PRECACHE_MODEL("models/ef_lightningsmg.mdl");
		PRECACHE_MODEL("sprites/muzzleflash215.spr");
    }


    int GetItemInfo(ItemInfo* p) override
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "lightningsmg";
        p->iMaxAmmo1 = 90;
        p->pszAmmo2 = "lightningsmg2";
        p->iMaxAmmo2 = -1;
        p->iMaxClip = 80;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_LIGHTNINGSMG;
        p->iFlags = 0;
        p->iWeight = M249_WEIGHT;
        return 1;
    }

    bool HasSecondaryAttack() override { return true; }
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_lightningsmg" , 80 }; }
#endif
    float GetArmorRatioModifier() override { return 0.9; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 6, 3, 0 }; }

    void SetSubModel()
    {
        pev->modelindex = m_iModelIndex;

        int iSubModel;

        if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] > 0)
            iSubModel = 2;
        else
            iSubModel = 0;

        if (GetBodygroup(GROUP_EFFECT) != iSubModel)
        {
            SetBodygroup(GROUP_EFFECT, iSubModel);
        }

        int hands = 0;

        if (m_pPlayer && m_pPlayer->m_bIsFemale)
            hands = 1;

        SetBodygroup(GROUP_HANDS, hands);

        pev->modelindex = 0;
    }

	int ExtractAmmo(CBasePlayerWeapon* pWeapon) override
	{
		if (pev->iuser2)
		{
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = pev->iuser2;
		}
        
		return CBasePlayerWeapon::ExtractAmmo(pWeapon);
	}

    BOOL Deploy() override
    {
        m_flAccuracy = AccuracyDefault;
        m_bDelayFire = false;
        pev->iuser1 = 0;

        if(DefaultDeploy(V_Model, P_Model, ANIM_DRAW, "mp5", UseDecrement() != FALSE))
        {
            SetSubModel();
            return TRUE;
        }

        return FALSE;
    }

    void Holster(int skiplocal)
    {
        SetThink(nullptr);
        CBasePlayerWeapon::Holster();
    }

    void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_bDelayFire = true;
        m_iShotsFired++;

        m_flAccuracy = ((m_iShotsFired * m_iShotsFired) / 230.0) + 0.3;
        if (m_flAccuracy > AccuracyMax)
            m_flAccuracy = AccuracyMax;

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
        m_iChargeShotsFired++;

        if (m_iChargeShotsFired > (m_pPlayer->m_iModelName == MODEL_BUFFCLASS23S5TR ? ChargingCountForBuffClass23s5 : ChargingCount) && pev->iuser2 < MaxChargeCount)
        {
            pev->iuser2++;
            
            SetSubModel();
#ifndef CLIENT_DLL
            MESSAGE_BEGIN(MSG_ONE, gmsgAmmoPickup, NULL, m_pPlayer->pev);
            WRITE_BYTE(CBasePlayer::GetAmmoIndex("lightningsmg2"));
            WRITE_BYTE(1);
            MESSAGE_END();
#endif
            m_iChargeShotsFired = 0;

            m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = pev->iuser2;
        }

        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;

        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 1, BULLET_PLAYER_45ACP, GetDamage(), 0.88, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

        int flags;
#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5s;

        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            KickBack(1.0, 0.55, 0.4, 0.05, 5, 3.25, 9);
        else if (m_pPlayer->pev->velocity.Length2D() > 0)
            KickBack(0.6, 0.3, 0.25, 0.035, 3.5, 2.75, 10);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            KickBack(0.25, 0.175, 0.125, 0.02, 2.25, 1.2, 10);
        else
            KickBack(0.275, 0.2, 0.15, 0.02, 3, 1.75, 9);
    }

    void FireMissile()
    {
        pev->iuser1++;

        pev->nextthink = gpGlobals->time + MissileShootDelay;

        if (pev->iuser1 > GlobeCount)
        {
            SetThink(nullptr);

            pev->iuser1 = 0;
            return;
        }

#ifndef CLIENT_DLL
        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecDir = m_pPlayer->FireBullets3(vecSrc - gpGlobals->v_up * 2.0, gpGlobals->v_forward, 0.0, 8192.0, 0, BULLET_PLAYER_556MM, 0, 0.0, m_pPlayer->pev, false, m_pPlayer->random_seed);

        Vector vecProjDir = gpGlobals->v_forward + gpGlobals->v_right * vecDir.x + gpGlobals->v_up * vecDir.y;

        CLightningSMGElectroMissile* pEnt = static_cast<CLightningSMGElectroMissile*>(CBaseEntity::Create("lightningsmg_electro_missile", vecSrc, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev)));
        if (pEnt)
        {
            pEnt->Init(vecProjDir * MissileSpeed, 1, g_vecZero);
        }
#endif

    }

    // 生成随机单位向量
    Vector generateUnitVector3D() {
        Vector vecOut;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<double> dis(0, 2 * M_PI);
        std::uniform_real_distribution<double> disTheta(-M_PI / 2, M_PI / 2);

        double phi = dis(gen);
        double theta = disTheta(gen);

        vecOut.x = std::cos(phi) * std::cos(theta);
        vecOut.y = std::sin(phi) * std::cos(theta);
        vecOut.z = std::sin(theta);

        return vecOut;
    }

    // 生成与给定向量垂直且夹角不太小的单位向量
    Vector generatePerpendicularUnitVector(Vector vecIn, double minAngle, double maxAngle) {
        Vector vecOut;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<double> dis(minAngle, maxAngle);

        double phi = std::atan2(vecIn.y, vecIn.x);
        double theta = std::asin(vecIn.z);

        double newPhi = phi + M_PI / 2; // 与给定向量垂直
        double newTheta = dis(gen);

        vecOut.x = std::cos(newPhi) * std::cos(newTheta);
        vecOut.y = std::sin(newPhi) * std::cos(newTheta);
        vecOut.z = std::sin(newTheta);

        return vecOut;
    }

    void OnAttack2Released()
    {
#ifndef CLIENT_DLL
        Vector vecFiledSrc = m_pPlayer->pev->origin;
        Vector vecAngle = m_pPlayer->pev->v_angle;
        vecAngle.x = vecAngle.z = 0;

        if (m_pPlayer->pev->flags & FL_DUCKING)
        {
            vecFiledSrc.z -= 16;
        }
        else
        {
            vecFiledSrc.z -= 32;
        }

        CLightningSMGElectroField* pEnt = static_cast<CLightningSMGElectroField*>(CBaseEntity::Create("lightningsmg_electro_field", vecFiledSrc, vecAngle, ENT(m_pPlayer->pev)));

#endif

        if (m_iChargeState > 1)
        {
            SetThink(&CLightningSMG::FireMissile);

            pev->nextthink = gpGlobals->time + 0.01s;
        }
        else
        {
#ifndef CLIENT_DLL
            Vector vecNormal[3];
            vecNormal[0] = generateUnitVector3D();
            vecNormal[1] = generatePerpendicularUnitVector(vecNormal[0], M_PI / 6, M_PI / 3);
            vecNormal[2] = generatePerpendicularUnitVector(vecNormal[0], 0, M_PI / 3);
  
            int randomID = RandomInt(0, 99);
            for (int i = 0; i < 3; i++)
            {
                Vector vecSrc = m_pPlayer->pev->origin;

                vecSrc.z += RANDOM_FLOAT(-75, 75);
                vecSrc.x += RANDOM_FLOAT(-75, 75);
                vecSrc.y += RANDOM_FLOAT(-75, 75);

                CLightningSMGElectroMissile* pEnt = static_cast<CLightningSMGElectroMissile*>(CBaseEntity::Create("lightningsmg_electro_missile", vecSrc, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev)));
                if (pEnt)
                {
                    pEnt->Init(g_vecZero, 0, vecFiledSrc);
                    pEnt->pev->fuser2 = i * 1.0;
                    pEnt->m_vecNormal = vecNormal[i];
                }
            }    
#endif

        }


        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

        pev->iuser2--;
        m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = pev->iuser2;

        SetSubModel();

        SendWeaponAnim(ANIM_CHARGE_SHOOT);

        UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/lightningsmg-2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM, 1);

        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.7s;
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;

        if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 0)
            m_pPlayer->pev->weaponmodel = MAKE_STRING(P_Model);

        m_iChargeState = 0;
    }

    void SecondaryAttack(void)
    {
        if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] <= 0)
            return;

        if (m_iChargeState)
        {
            if (gpGlobals->time > m_flTimeCheckCharge)
            {
                switch (m_iChargeState)
                {
                case 1:
                {
                    SendWeaponAnim(ANIM_CHARGE_FINISH);

                    m_iChargeState = 2;
                    m_flTimeCheckCharge = gpGlobals->time + 0.53s;
                    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0s;
                    break;
                }
                case 2:
                {
                    SendWeaponAnim(ANIM_CHARGE2);

                    UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/lightningsmg_idle3.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM, 1);

                    m_iChargeState = 3;
                    m_flTimeCheckCharge = gpGlobals->time + 2.0s;
                    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 9999.0s;
                    break;
                }
                case 3:
                {
                    UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/lightningsmg_idle3.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM, 1);

                    m_flTimeCheckCharge = gpGlobals->time + 2.0s;
                }

                default:
                    break;
                }
            }
        }
        else
        {
            SendWeaponAnim(ANIM_CHARGE1);

            UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/lightningsmg_idle2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM, 1);

            m_flTimeCheckCharge = gpGlobals->time + 0.5s;
            m_iChargeState = 1;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
        }
    }

    void WeaponIdle() override
    {
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

        if (m_iChargeState && !(m_pPlayer->pev->button & IN_ATTACK2))
        {
            OnAttack2Released();
        }

        if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
            return;

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
        SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);
    }

    void Reload() override
    {
        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            return;

        if (DefaultReload(MaxClip, ANIM_RELOAD, DefaultReloadTime))
        {
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
            m_flAccuracy = 0.0;
        }
    }

    		 	
};
LINK_ENTITY_TO_CLASS(weapon_lightningsmg, CLightningSMG)
}