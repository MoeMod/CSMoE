/*
halogun.cpp
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
    class CHaloGunProjectile : public CBaseAnimating
    {
    public:
        CHaloGunProjectile()
        {
            m_flRemoveTime = invalid_time_point;
        }

        float GetArmorRatioModifier() { return 1.99; }
        int GetWeaponsId(void) { return WEAPON_HALOGUN; }

        float GetDirectDamage() const
        {
            float flDamage = 20.0f;
            if (g_pModRunning->DamageTrack() == DT_ZB)
            {
                if (enhancedzombie.value)
                    flDamage = 8000;
                else
                    flDamage = 4000;
            }
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = 40000.0f;
            return flDamage;
        }

        float GetRadiusDamage() const
        {
            float flDamage = 20.0f;
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flDamage = 200.0f;
           
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = 200.0f;
            return flDamage;
        }

        void Spawn() override
        {
            Precache();

            SetThink(&CHaloGunProjectile::OnThink);
            SET_MODEL(this->edict(), "models/ef_halogun_projectile.mdl");

            pev->rendermode = kRenderTransAdd;
            pev->renderfx = kRenderFxNone;
            pev->renderamt = 255;
            pev->solid = SOLID_TRIGGER;
            pev->movetype = MOVETYPE_NOCLIP;

            pev->frame = 0;
            pev->sequence = 0;
            pev->avelocity.z = 500;

            ResetSequenceInfo();

            pev->effects |= EF_NOCULL;

            m_flRemoveTime = gpGlobals->time + 2.0s;
            UTIL_SetSize(pev, g_vecZero, g_vecZero);

            pev->nextthink = gpGlobals->time + 0.1s;

            MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
            WRITE_BYTE(TE_BEAMFOLLOW);
            WRITE_SHORT(entindex());
            WRITE_SHORT(m_iTrail);
            WRITE_BYTE(2);
            WRITE_BYTE(60);
            WRITE_BYTE(255);
            WRITE_BYTE(255);
            WRITE_BYTE(255);
            WRITE_BYTE(255);
            MESSAGE_END();
        }

        void Precache() override
        {
            PRECACHE_MODEL("models/ef_halogun_projectile.mdl");
            m_iTrail = PRECACHE_MODEL("sprites/ef_halogun_projectile.spr");
        }

        void RadiusDamage()
        {
            CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
            CBasePlayer* pAttackPlayer = nullptr;
            if (pAttacker && pAttacker->IsPlayer())
                pAttackPlayer = static_cast<CBasePlayer*>(pAttacker);

            if (!pAttackPlayer)
                return;

            TraceResult tr;
            CBaseEntity* pEntity = NULL;
            float flDamage;

            while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, 110.0)) != NULL)
            {
                if (pEntity->Classify() != CLASS_PLAYER_ALLY && !pEntity->IsPlayer())
                    continue;

                CBasePlayer* target = static_cast<CBasePlayer*>(pEntity);

                if (g_pGameRules->PlayerRelationship(pAttackPlayer, pEntity) == GR_TEAMMATE)
                    continue;

                flDamage = GetDirectDamage();

                if (std::find(m_TargetVector.begin(), m_TargetVector.end(), pEntity->entindex()) != m_TargetVector.end())
                    flDamage = GetRadiusDamage();
                else
                {
                    Vector vecDelta = pev->origin - pEntity->pev->origin;
                    float yaw = g_engfuncs.pfnVecToYaw(vecDelta);
                    g_engfuncs.pfnWalkMove(pEntity->edict(), yaw, vecDelta.Length(), WALKMOVE_NORMAL);

                    m_TargetVector.push_back(pEntity->entindex());
                }
                   
                pEntity->m_LastHitGroup = HITGROUP_CHEST;
                pEntity->TakeDamage(pev, pAttackPlayer->pev, flDamage, DMG_BULLET | DMG_NOMORALE);

                Vector vecDirection = (pEntity->pev->origin - pAttackPlayer->pev->origin).Normalize();
                if(!(pEntity->pev->flags & FL_ONGROUND))
                    UTIL_AddKickRateBoost(pEntity, vecDirection.Make2D(), 1.0, 300.0, 0);
                else
                    UTIL_AddKickRateBoost(pEntity, vecDirection.Make2D(), 1.0, 2000.0, 0);
                
            }
        }

        void EXPORT OnThink()
        {
            pev->nextthink = gpGlobals->time + 0.1s;

            if (m_flRemoveTime <= gpGlobals->time)
            {
                Remove();
                return;
            }

            RadiusDamage();
        }

        KnockbackData GetKnockBackData() override { return { -1.0f, -1.0f, -1.0f, -1.0f, 1.0f }; }

        int m_iTrail;
        std::vector<int> m_TargetVector;
        time_point_t m_flRemoveTime;

    protected:
        void Remove()
        {
            SetThink(nullptr);
            pev->effects |= EF_NODRAW;
            return UTIL_Remove(this);
        }
    };

    LINK_ENTITY_TO_CLASS(halogun_projectile, CHaloGunProjectile)
#endif

    class CHaloGun : public LinkWeaponTemplate< CHaloGun,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_AK47>::template type,
            TPrecacheEvent,
            TReloadDefault,
            TGetDamageDefault,
            TFireRifle,
            TRecoilKickBack,
            TPrimaryAttackRifle
    >
    {
    public:
    static constexpr const auto &DefaultReloadTime = 2.0s;
    static constexpr const char *V_Model = "models/v_halogun.mdl";
    static constexpr const char *P_Model = "models/p_halogun.mdl";
    static constexpr const char *W_Model = "models/w_halogun.mdl";
    static constexpr const char *EventFile = "events/halogun.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_halogun";
    static constexpr int MaxClip = 100;
    enum
    {
        ANIM_IDLE = 0,
        ANIM_RELOAD,
        ANIM_DRAW,
        ANIM_SHOOTA,
        ANIM_SHOOTB_START,
        ANIM_SHOOTB_LOOP,
        ANIM_SHOOTB_END,
        ANIM_SHOOTB_CHARGING_LOOP,
        ANIM_SHOOTB_CHARGING_END,
        ANIM_SHOOTC,
        ANIM_SHOOTB_CHANGE,
	};

    static constexpr float PlayerMaxSpeed = 240;
    static constexpr float ArmorRatioModifier = 1.5f;

    static constexpr float PrimaryAttackWalkingMiniumSpeed = 0;
    static constexpr const auto& SpreadCalcNotOnGround = 0.4 * A + 0.04;
    static constexpr const auto& SpreadCalcWalking = 0.03 * A;
    static constexpr const auto& SpreadCalcDefault = 0.0275 * A;

	static constexpr const auto& CycleTime = 0.13s;
    static constexpr int DamageDefault = 25;
    static constexpr int DamageZB = 370;
    static constexpr int DamageZBS = 1690;
    static constexpr const auto& AccuracyCalc = (N * N * N / 220.0) + 0.35;
    static constexpr float AccuracyDefault = 0.2;
    static constexpr float AccuracyMax = 1.25;
    static constexpr float RangeModifier = 0.98;

    static constexpr auto BulletType = BULLET_PLAYER_556MM;

    KickBackData KickBackWalking = { 1.5, 0.45, 0.225, 0.05, 6.5, 2.5, 7 };
    KickBackData KickBackNotOnGround = { 2.0, 1.0, 0.4, 0.35, 9.0, 6.0, 5 };
    KickBackData KickBackDucking = { 0.48, 0.16, 0.2, 0.011, 2.0, 1.65, 9 };
    KickBackData KickBackDefault = { 0.49, 0.18, 0.2, 0.013, 2.4, 1.74, 8 };

    /*static constexpr*/ int Penetration = 3;
    KnockbackData KnockBack = { 600.0f, 550.0f, 650.0f, 550.0f, 0.4f };

    KnockbackData GetKnockBackData() override
    {
        if (m_iChargeState)
            return { 600.0f, 550.0f, 650.0f, 550.0f, -1.0f };
        else
            return { 600.0f, 550.0f, 650.0f, 550.0f, 0.4f };
    }


#define HALOGUN_MAXAMMO_3 150
#define HALOGUN_FIRETARGET 60
    int m_iDefaultAmmo2;
    int m_iClip2;
    int m_iClip3;
    int m_iChargingShotIndex[2];
    int m_iExpIndex;
    int m_iHitIndex[2];
    int m_iChargeState;
    int m_iChargeShotFired;
    int m_iSkillShotFired;
    time_point_t m_flNextShootCAttack;
    time_point_t m_flTimeNextCharge;
    time_point_t m_flTimeNextCharge2;
    time_point_t m_flTimeCheckCharge;

    void Spawn()
    { 
        m_iSkillShotFired = 0;
        m_iChargeShotFired = 0;
        m_iChargeState = m_iClip2 = m_iClip3 = 0;
        m_iDefaultAmmo2 = 10;
        ResetChargeTimer();
        Base::Spawn();
    }

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "halogun";
        p->iMaxAmmo1 = 200;
        p->pszAmmo2 = "halogun_b";
        p->iMaxAmmo2 = 50;
        p->iMaxClip = 100;
        p->iSlot = 0;
        p->iPosition = 1;
        p->iId = m_iId = WEAPON_HALOGUN;
        p->iFlags = 0;
        p->iWeight = M3_WEIGHT;

        return 1;
    }

    int ExtractAmmo(CBasePlayerWeapon* pWeapon) override
    {
        if (m_iDefaultAmmo2)
        {
            if (m_pPlayer)
                m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iDefaultAmmo2;
            m_iDefaultAmmo2 = 0;
        }
        return CBasePlayerWeapon::ExtractAmmo(pWeapon);
    }

    float GetArmorRatioModifier() override { return 1.5; }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_halogun" , 200 }; }
#endif
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR, 4, 3, 0 }; }

    void Precache(void) override
    {
        PRECACHE_SOUND("weapons/halogun_fx.wav");
        PRECACHE_SOUND("weapons/halogun_idle.wav");
        PRECACHE_SOUND("weapons/halogun-1.wav");
        PRECACHE_SOUND("weapons/halogun-2.wav");
        PRECACHE_SOUND("weapons/halogun-2_end.wav");
        PRECACHE_SOUND("weapons/halogun-1_exp.wav");
        PRECACHE_SOUND("weapons/halogun-2_exp1.wav");
        PRECACHE_SOUND("weapons/halogun-2_exp2.wav");
        PRECACHE_SOUND("weapons/halogun-3.wav");

        m_iChargingShotIndex[0] = PRECACHE_MODEL("models/ef_halogun_chargingshot.mdl");
        m_iChargingShotIndex[1] = PRECACHE_MODEL("models/ef_halogun_chargingshot2.mdl");

        PRECACHE_MODEL("sprites/ef_halogun_projectile.spr");
        m_iExpIndex = PRECACHE_MODEL("sprites/ef_halogun_expA.spr");
        m_iHitIndex[0] = PRECACHE_MODEL("sprites/ef_halogun_shootA_hit.spr");
        m_iHitIndex[1] = PRECACHE_MODEL("sprites/ef_halogun_shootB_hit.spr");

        PRECACHE_MODEL("sprites/muzzleflash273.spr");
        PRECACHE_MODEL("sprites/muzzleflash274.spr");
        PRECACHE_MODEL("sprites/muzzleflash275.spr");
        PRECACHE_MODEL("sprites/muzzleflash276.spr");
        PRECACHE_MODEL("sprites/muzzleflash277.spr");
        PRECACHE_MODEL("sprites/muzzleflash278.spr");
        PRECACHE_MODEL("sprites/muzzleflash279.spr");
        PRECACHE_MODEL("sprites/muzzleflash280.spr");
        PRECACHE_MODEL("sprites/muzzleflash281.spr");
        PRECACHE_MODEL("sprites/muzzleflash282.spr");
        PRECACHE_MODEL("sprites/muzzleflash290.spr");
        PRECACHE_MODEL("sprites/muzzleflash291.spr");
        return Base::Precache();
    }

    float GetExplodeDamage()
    {
        float flDamage = 15.0f;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = 1500.0f;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = 6000.0f;
#endif
        return flDamage;
    }

    float GetDirectDamage()
    {
        float flDamage = 25.0f;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = 370.0f;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = 1690.0f;
#endif
        return flDamage;
    }

    float GetSecondaryAttackDamage() const
    {
        float flDamage = 10.0f;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = 500.0f;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = 2200.0f;
#endif
        return flDamage;
    }

    duration_t GetChargeTime()
    {
        duration_t flTime = 1.0s;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flTime = 0.4s;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flTime = 0.4s;
#endif
        return flTime;
    }

    void ResetChargeTimer()
    {
        m_flTimeNextCharge = gpGlobals->time + GetChargeTime();
        m_flTimeNextCharge2 = gpGlobals->time + 0.1s;
    }

    void ChargeAmmo()
    {
        if (m_flTimeNextCharge != invalid_time_point && m_flTimeNextCharge <= gpGlobals->time)
        {
            m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] += int((gpGlobals->time - m_flTimeNextCharge) / GetChargeTime() + 1.0);

            if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] >= 50)
                m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 50;

            m_flTimeNextCharge = gpGlobals->time + GetChargeTime();
        }

        if (m_iWeaponState < 3 && m_flTimeNextCharge2 != invalid_time_point && m_flTimeNextCharge2 <= gpGlobals->time)
        {
            m_iClip3 += int((gpGlobals->time - m_flTimeNextCharge2) / 0.1s + 1.0);

            m_flTimeNextCharge2 = gpGlobals->time + 0.1s;

            if (m_iClip3 >= HALOGUN_MAXAMMO_3)
            {
                m_iClip3 = 0;
                m_iWeaponState++;              
            }

#ifndef CLIENT_DLL
            MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
            WRITE_BYTE(37);
            WRITE_BYTE(m_iWeaponState);
            WRITE_BYTE(m_iClip3);
            WRITE_BYTE(HALOGUN_MAXAMMO_3);
            MESSAGE_END();
#endif
        }

        if (m_iWeaponState >= 3)
            m_flTimeNextCharge2 = invalid_time_point;
    }

    void Holster(int skiplocal)
    {
        ResetChargeTimer();
        StopSound();
        CBasePlayerWeapon::Holster();
    }

    BOOL Deploy(void) override
    {
        m_flAccuracy = AccuracyDefault;
        m_iChargeState = 0;
        pev->skin = 0;

        SetPModel();

        ChargeAmmo();

        if (DefaultDeploy(V_Model, P_Model, ANIM_DRAW, "m249", UseDecrement() != FALSE, 0.1s, 1.0s))
        {
            m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.1s;
            return TRUE;
        }

        return FALSE;
    }

    void StopSound()
    {
#ifndef CLIENT_DLL
        MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->edict());
        WRITE_BYTE(79);
        WRITE_SHORT(m_pPlayer->entindex());
        WRITE_SHORT(CHAN_ITEM);
        WRITE_STRING("weapons/halogun-2.wav");
        MESSAGE_END();
#endif
    }

    void FireC()
    {
        m_flTimeNextCharge2 = gpGlobals->time + 0.1s;

        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

        Vector vecSrc = m_pPlayer->GetGunPosition() - gpGlobals->v_up * 2;
        Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, 0.0, 8192, 0, BULLET_PLAYER_556MM, 0, 0, m_pPlayer->pev, false, m_pPlayer->random_seed);
        vecDir = gpGlobals->v_forward + gpGlobals->v_right * vecDir.x + gpGlobals->v_up * vecDir.y;

#ifndef CLIENT_DLL
        MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
        WRITE_BYTE(37);
        WRITE_BYTE(m_iWeaponState);
        WRITE_BYTE(m_iClip3);
        WRITE_BYTE(HALOGUN_MAXAMMO_3);
        MESSAGE_END();

        m_pPlayer->SetAnimation(PLAYER_ATTACK1);

      
        CHaloGunProjectile* pEnt = static_cast<CHaloGunProjectile*>(CBaseEntity::Create("halogun_projectile", vecSrc, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev)));
        if (pEnt)
        {
            pEnt->pev->velocity = vecDir * 1000;
        }
#endif
        int flags = 0;

#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, static_cast<int>(m_pPlayer->pev->punchangle.x * 100), static_cast<int>(m_pPlayer->pev->punchangle.y * 100), TRUE, FALSE);

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;

        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.9s;
        m_flNextShootCAttack = gpGlobals->time + 0.9s;
    }

    void ItemPostFrame()
    {

        if (m_iWeaponState && m_pPlayer->pev->button & IN_ATTACK && m_pPlayer->pev->button & IN_ATTACK2 && m_flNextShootCAttack < gpGlobals->time)
        {
            m_iWeaponState--;
            m_iChargeState = 0;
            FireC();
            return;
        }

        ChargeAmmo();

        if (m_iChargeState && !(m_pPlayer->pev->button & IN_ATTACK2) && m_flNextSecondaryAttack <= 0.0s)
            OnAttack2Released();
            
        return CBasePlayerWeapon::ItemPostFrame();
    }

    void PrimaryAttack()
    {
        m_iChargeState = 0;

        Base::PrimaryAttack();
    }

    void FireEvent(const Vector& vecDir)
    {
        int flags = 0;

#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, static_cast<int>(m_pPlayer->pev->punchangle.x * 100), static_cast<int>(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);
    }

    void SetPModel()
    {
        int flags = 0;

#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, 2.0, 0.0, 0, m_iChargeState, FALSE, TRUE);
    }

    int AddToPlayer(CBasePlayer* pPlayer)
    {
#ifndef CLIENT_DLL
        if(pPlayer->m_iBuff & BUFF_HALOGUN_AVOID_DEATH)
        {
            MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, pPlayer->pev);
            WRITE_BYTE(38);
            WRITE_BYTE(0);
            WRITE_BYTE(1);
            MESSAGE_END();
        }
#endif
        return CBasePlayerWeapon::AddToPlayer(pPlayer);
    }



    void SecondaryAttack()
    {
        if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] <= 0)
        {
            PlayEmptySound();
            m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2s;
            return;
        }
           
        ResetChargeTimer();

        if (m_iChargeState)
        {
            if (gpGlobals->time > m_flTimeCheckCharge)
            {
                switch (m_iChargeState)
                {
                case 1:
                {
                    SendWeaponAnim(ANIM_SHOOTB_LOOP);

                    m_iChargeState = 2;

                    SetPModel();

                    m_flTimeCheckCharge = gpGlobals->time + 0.15s;
                    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 9999.0s;

                    m_iClip2 = m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType];
                    
                    //SHOOTB
                    m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;
                    FireB();
                    break;
                }
                case 2:
                {
                    m_flTimeCheckCharge = gpGlobals->time + 0.15s;

                    m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;

                    if (m_iClip2 - m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] >= 10)
                    {
                        m_flTimeCheckCharge = gpGlobals->time + 0.7s;
                        pev->skin = 1;

                        SendWeaponAnim(ANIM_SHOOTB_CHANGE);

                        m_iChargeState = 3;

                        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.7s;
                    }
                    else
                    {
                        //SHOOTB                
                        FireB();
                    }

                    SetPModel();

                    break;
                }
                case 3:
                {
                    SendWeaponAnim(ANIM_SHOOTB_CHARGING_LOOP);

                    m_iChargeState = 4;
                    m_flTimeCheckCharge = gpGlobals->time + 0.15s;
                    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 9999.0s;

                    //SHOOTB_CHARGING
                    m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;
                    FireB();
                    SetPModel();
                    break;
                }
                case 4:
                {
                    m_flTimeCheckCharge = gpGlobals->time + 0.15s;
                    //SHOOTB_CHARGING
                    m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;
                    FireB();
                    SetPModel();             
                    break;
                }
                }

                if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] <= 0)
                {
                    OnAttack2Released();
                }
            }
        }
        else
        {
            SendWeaponAnim(ANIM_SHOOTB_START);

#ifndef CLIENT_DLL
            EMIT_SOUND_DYN(this->edict(), CHAN_WEAPON, "weapons/halogun-2.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);
#endif
            m_flTimeCheckCharge = gpGlobals->time + 0.37s;
            m_iChargeState = 1;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.37s;
            m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.37s;
        }
    }

    void FireB()
    {
#ifndef CLIENT_DLL
        Vector vecSrc, vecEnd;
        TraceResult tr;
        CBaseEntity* pEntity = NULL;

        UTIL_MakeVectors(m_pPlayer->pev->v_angle);

        vecSrc = m_pPlayer->GetGunPosition();

        Vector2D vecAngleCheckDir = gpGlobals->v_forward.Make2D().Normalize();
        float angCheck = cos(66.0 * 0.5 * M_PI / 180.0);
        float flDamage;

        flDamage = GetSecondaryAttackDamage();

        while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, 393.7)) != NULL)
        {

            if (pEntity->Classify() != CLASS_PLAYER_ALLY && !pEntity->IsPlayer())
                continue;

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

            ClearMultiDamage();

            pEntity->m_LastHitGroup = HITGROUP_CHEST;
            tr.iHitgroup = HITGROUP_CHEST;
            pEntity->TraceAttack(m_pPlayer->pev, flDamage, (tr.vecEndPos - vecSrc).Normalize(), &tr, DMG_BULLET | DMG_NEVERGIB);    

            ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

            if (g_pGameRules->PlayerRelationship(m_pPlayer, pEntity) == GR_TEAMMATE)
                continue;

            MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
            WRITE_BYTE(TE_TEMPSPRITE);
            WRITE_COORD(pEntity->pev->origin.x);
            WRITE_COORD(pEntity->pev->origin.y);
            WRITE_COORD(pEntity->pev->origin.z);
            WRITE_SHORT(m_iHitIndex[1]);
            WRITE_BYTE(4);
            WRITE_BYTE(255);
            WRITE_BYTE(30);
            MESSAGE_END();

            Vector vecDirection = (pEntity->pev->origin - m_pPlayer->pev->origin).Normalize();
            UTIL_AddKickRateBoost(pEntity, vecDirection.Make2D(), 1.0, 150.0, 0);

            if (((CBasePlayer*)pEntity)->m_bIsZombie)
            {
                CBasePlayer* player = (CBasePlayer*)pEntity;
    
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

    void OnAttack2Released()
    {

        SendWeaponAnim(m_iChargeState > 2 ? ANIM_SHOOTB_CHARGING_END : ANIM_SHOOTB_END, UseDecrement() != FALSE);

        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + (m_iChargeState > 2 ? 1.7s :  0.1s);
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.7s;

        ResetChargeTimer();

        StopSound();

        if (m_iChargeState <= 2)
        {
            EMIT_SOUND_DYN(this->edict(), CHAN_WEAPON, "weapons/halogun-2_end.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);
            m_iChargeState = 0;
            SetPModel();
            return;
        }
        
        EMIT_SOUND_DYN(this->edict(), CHAN_WEAPON, "weapons/halogun-2_exp1.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);


#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

        m_iChargeState = 0;

#ifndef CLIENT_DLL
        TraceResult tr;

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

        Vector vecSrc = m_pPlayer->GetGunPosition() - gpGlobals->v_up * 2;
        Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, 0, 8192.0, 0, BULLET_PLAYER_556MM, 0, 0.0, m_pPlayer->pev, false, m_pPlayer->random_seed);

        vecDir = gpGlobals->v_forward + gpGlobals->v_right * vecDir.x + gpGlobals->v_up * vecDir.y;

        Vector vecEnd = vecSrc + vecDir * 39.37 * 12;

        UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, m_pPlayer->edict(), &tr);

        vecEnd = tr.vecEndPos;
        vecEnd.z -= 8192;
        UTIL_TraceLine(tr.vecEndPos, vecEnd, dont_ignore_monsters, m_pPlayer->edict(), &tr);

        vecEnd = tr.vecEndPos;
 
        vecEnd.z += 32;

        CBaseEntity* pEntity = NULL;
        while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecEnd, 7 * 39.37)) != NULL)
        {
            if (pEntity->IsDormant())
                continue;

            if (pEntity->pev->takedamage == DAMAGE_NO)
                continue;

            if (pEntity->pev == m_pPlayer->pev)
                continue;

            if (UTIL_IsWallBetweenEntity(pEntity, m_pPlayer))
                continue;

            ClearMultiDamage();

            pEntity->m_LastHitGroup = HITGROUP_CHEST;
            tr.iHitgroup = HITGROUP_CHEST;
            pEntity->TraceAttack(m_pPlayer->pev, GetDirectDamage(), vecDir, &tr, DMG_BULLET | DMG_NEVERGIB);
           
            ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

            Vector vecDirection = (pEntity->pev->origin - m_pPlayer->pev->origin).Normalize();
            UTIL_AddKickRateBoost(pEntity, vecDirection.Make2D(), 1.0, 2000.0, 120);

        }

        UTIL_TempModel(vecEnd, g_vecZero, g_vecZero, m_iChargingShotIndex[0], 10, 0, 10, 0, 0, 255, kRenderNormal, NULL, false, 0, 0, 256, 0x20000000);
#endif
        pev->skin = 0;
        SetPModel();
    }


#ifndef CLIENT_DLL
    void FireBullet3CallBack(int iPenetration, Vector vecDir, TraceResult* pTrace)
    {
        int iOriginalPenetration = 3;

        if (iOriginalPenetration == iPenetration)
        {
            if (m_iChargeState > 2)
            {
               
               

            }
            else if (m_iChargeState > 0)
            {

            }
            else
            {
                Vector vecEnd = pTrace->vecEndPos - vecDir * 15;
                m_iChargeShotFired++;

                if (!(m_pPlayer->m_iBuff & BUFF_HALOGUN_AVOID_DEATH))
                {
                    if (CBaseEntity* pEntity = gMultiDamage.pEntity)
                    {
                        if (pEntity->IsPlayer() || pEntity->Classify() == CLASS_PLAYER_ALLY)
                        {
                            if (g_pGameRules->PlayerRelationship(m_pPlayer, pEntity) != GR_TEAMMATE)
                            {
                                m_iSkillShotFired++;
                            }
                        }
                    }
                }

                if (m_iSkillShotFired >= HALOGUN_FIRETARGET)
                {
                    m_pPlayer->m_iBuff |= BUFF_HALOGUN_AVOID_DEATH;

                    MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
                    WRITE_BYTE(38);
                    WRITE_BYTE(0);
                    WRITE_BYTE(1);
                    MESSAGE_END();

                    EMIT_SOUND_DYN(this->edict(), CHAN_ITEM, "weapons/halogun_fx.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);

                    m_iSkillShotFired = 0;
                }

                if (m_iChargeShotFired >= 7)
                {
                    m_iChargeShotFired = 0;

                    EMIT_SOUND_DYN(this->edict(), CHAN_ITEM, "weapons/halogun-1_exp.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);

                    if (CBaseEntity* pEntity = gMultiDamage.pEntity)
                    {
                        float flDirectDamage = GetDirectDamage();
                        pEntity->TraceAttack(m_pPlayer->pev, flDirectDamage, vecDir, pTrace, DMG_BULLET | DMG_NEVERGIB);
                    }

                    MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
                    WRITE_BYTE(TE_TEMPSPRITE);
                    WRITE_COORD(vecEnd.x);
                    WRITE_COORD(vecEnd.y);
                    WRITE_COORD(vecEnd.z);
                    WRITE_SHORT(m_iExpIndex);
                    WRITE_BYTE(8);
                    WRITE_BYTE(255);
                    WRITE_BYTE(30);
                    MESSAGE_END();

                    CBaseEntity* pEntity = NULL;
                    while ((pEntity = UTIL_FindEntityInSphere(pEntity, pTrace->vecEndPos, 78.74)) != NULL)
                    {
                        if (pEntity->IsDormant())
                            continue;

                        if (pEntity->pev->takedamage == DAMAGE_NO)
                            continue;

                        if (pEntity->pev == m_pPlayer->pev)
                            continue;

                        if (UTIL_IsWallBetweenEntity(pEntity, m_pPlayer))
                            continue;

                        pEntity->TakeDamage(m_pPlayer->pev, m_pPlayer->pev, GetExplodeDamage(), DMG_NEVERGIB | DMG_BULLET);
                    }
                }
                else
                {
                    MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
                    WRITE_BYTE(TE_TEMPSPRITE);
                    WRITE_COORD(vecEnd.x);
                    WRITE_COORD(vecEnd.y);
                    WRITE_COORD(vecEnd.z);
                    WRITE_SHORT(m_iHitIndex[0]);
                    WRITE_BYTE(3);
                    WRITE_BYTE(255);
                    WRITE_BYTE(60);
                    MESSAGE_END();

                }
            }
        }

    }
#endif

    void WpnDeathCallBack(CBaseEntity* pVictim, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override
    {
#ifndef CLIENT_DLL
        Vector vecOrigin = m_pPlayer->pev->origin;
        vecOrigin.z -= 16;

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

            pEntity->TakeDamage(m_pPlayer->pev, m_pPlayer->pev, GetExplodeDamage(), DMG_NEVERGIB | DMG_BULLET);

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

        UTIL_TempModel(vecOrigin, g_vecZero, g_vecZero, m_iChargingShotIndex[1], 10, 0, 10, 0, 0, 255, kRenderNormal, NULL, false, 0, 0, 256, 0x20000000);

#endif
        m_pPlayer->m_iBuff |= BUFF_AVOID_INFECT;
        m_pPlayer->m_tAvoidInfectBuffTime = gpGlobals->time + 2.0s;
        EMIT_SOUND_DYN(this->edict(), CHAN_ITEM, "weapons/halogun-2_exp2.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);
        
    }

    void WeaponIdle(void)
    {
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

        if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
            return;

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;

        SendWeaponAnim(ANIM_IDLE, UseDecrement() != FALSE);
    }

};
LINK_ENTITY_TO_CLASS(weapon_halogun, CHaloGun)


}