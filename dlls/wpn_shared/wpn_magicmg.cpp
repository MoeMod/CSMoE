/*
magicmg.cpp
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

    class CMagicMG : public LinkWeaponTemplate< CMagicMG,
    TGeneralData,
    BuildTGetItemInfoFromCSW<WEAPON_M249>::template type,
    TPrecacheEvent,
    TDeployDefault,
    TReloadDefault,
    TPrimaryAttackRifle,
    TWeaponIdleDefault
    >
    {
    public:

        static constexpr duration_t DefaultDeployTime = 1.03s;
        static constexpr const auto& DefaultReloadTime = 3.9s;
        static constexpr const char* V_Model = "models/v_magicmg.mdl";
        static constexpr const char* P_Model = "models/p_magicmg.mdl";
        static constexpr const char* W_Model = "models/w_magicmg.mdl";
        static constexpr const char* EventFile = "events/magicmg.sc";
        static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
        static constexpr const char* ClassName = "weapon_magicmg";
        static constexpr const char* AnimExtension = "m249";
        static constexpr int MaxClip = 100;
        enum
        {
            ANIM_IDLE1 = 0,
            ANIM_IDLE2,
            ANIM_RELOAD,
            ANIM_DRAW,
            ANIM_SHOOT1,
            ANIM_SHOOT2,
            ANIM_SHOOT3,
        };
        static constexpr float PlayerMaxSpeed = 230;
        static constexpr float PrimaryAttackWalkingMiniumSpeed = 140;
        static constexpr const auto& SpreadCalcNotOnGround = 0.4 * A + 0.04;
        static constexpr const auto& SpreadCalcWalking = 0.04 * A + 0.07;
        static constexpr const auto& SpreadCalcDefault = 0.03 * A;

        static constexpr const auto& CycleTime = 0.2s;
        static constexpr float AccuracyDefault = 0.2;

        KnockbackData KnockBack = { 500.0f, 250.0f, 450.0f, 300.0f, 0.3f };


        float GetArmorRatioModifier() override { return 1.6; }
#ifndef CLIENT_DLL
        WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_magicmg" , 500 }; }
#endif
        WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 6, 3, 140 }; }


    private:
        duration_t m_flTimeWeaponIdle2;
        int m_iDefaultAmmo2;

    public:
        void Precache() override
        {
            Base::Precache();

            PRECACHE_MODEL("sprites/muzzleflash101.spr");
            PRECACHE_MODEL("sprites/ef_magicmgdraw.spr");
            PRECACHE_MODEL("sprites/ef_magicmgidle.spr");
            PRECACHE_MODEL("sprites/ef_magicmgreloadstart.spr");
            PRECACHE_MODEL("sprites/ef_magicmgreloadcharge.spr");
            PRECACHE_MODEL("sprites/ef_magicmgshoot2.spr");
            PRECACHE_MODEL("sprites/ef_magicmgdraw_left.spr");
            PRECACHE_MODEL("sprites/ef_magicmgidle_left.spr");
            PRECACHE_MODEL("sprites/ef_magicmgreloadstart_left.spr");
            PRECACHE_MODEL("sprites/ef_magicmgreloadcharge_left.spr");
            PRECACHE_MODEL("sprites/ef_magicmgshoot2_left.spr");

            PRECACHE_SOUND("weapons/magicmg-1.wav");
            PRECACHE_SOUND("weapons/magicmg-2.wav");
        }

        void WeaponIdle(void) override; 
        void Holster(int skiplocal) override;
        void Reload(void) override;
        void DelaySeconadryAttack(void);
        void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
        void StopIdleSound(void);
        void SecondaryAttack(void) override;
        void ItemPostFrame() override;
        bool HasSecondaryAttack() override { return true; }

        void Spawn(void) override
        {
            Base::Spawn();
            m_flTimeWeaponIdle2 = 0s;
            m_iDefaultAmmo2 = 0;
        }

        BOOL Deploy(void) override
        {
            m_flTimeWeaponIdle2 = 0s;

            PLAYBACK_EVENT_FULL(FEV_HOSTONLY, m_pPlayer->edict(), m_usFire, 0.0, g_vecZero, g_vecZero, 0.0, 0.0, 0, 1, FALSE, FALSE);

            return Base::Deploy();
        }

        int ExtractAmmo(CBasePlayerWeapon* pWeapon) override
        {
            if (m_iDefaultAmmo2)
            {
                m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iDefaultAmmo2;
                m_iDefaultAmmo2 = 0;
            }
            return CBasePlayerWeapon::ExtractAmmo(pWeapon);
        }

        int GetItemInfo(ItemInfo* p) override
        {
            p->pszName = STRING(pev->classname);
            p->pszAmmo1 = "magicmg";
            p->iMaxAmmo1 = 200;
            p->pszAmmo2 = "magicmg_cannon";
            p->iMaxAmmo2 = 1;
            p->iMaxClip = MaxClip;
            p->iSlot = 0;
            p->iPosition = 17;
            p->iId = m_iId = WEAPON_MAGICMG;
            p->iFlags = 0;
            p->iWeight = M249_WEIGHT;

            return 1;
        }

        int GetShotsRequired()
        {
            int iShotsRequired = 60;
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                iShotsRequired = 30;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                iShotsRequired = 30;
#endif
            return iShotsRequired;
        }

        float GetTouchDamageA() const
        {
            float flDamage = 33.0f;
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flDamage = 256.0f;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = 770.0f;
#endif
            return flDamage;
        }

        float GetRadiusDamageA() const
        {
            float flDamage = 20.0f;
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flDamage = 150.0f;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = 70.0f;
#endif
            return flDamage;
        }

        float GetTouchDamageB() const
        {
            float flDamage = 150.0f;
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flDamage = 3900.0f;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = 10000.0f;
#endif
            return flDamage;
        }

        float GetRadiusDamageB() const
        {
            float flDamage = 100.0f;
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flDamage = 3000.0f;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = 2000.0f;
#endif
            return flDamage;
        }

        float GetHealthRate() const
        {
            float flRate = 0.1f;
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flRate = 0.3f;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flRate = 0.3f;
#endif
            return flRate;
        }
    };

    LINK_ENTITY_TO_CLASS(weapon_magicmg, CMagicMG)

#ifndef CLIENT_DLL
    class CMagicCannon : public CBaseEntity
    {
    public:
        float GetArmorRatioModifier() { return 1.6; }
        int GetWeaponsId(void) { return WEAPON_MAGICMG; }
        void Spawn() override
        {
            Precache();

            SetThink(&CMagicCannon::FollowThink);
            SetTouch(&CMagicCannon::OnTouch);
            SET_MODEL(this->edict(), "sprites/ef_magicmgmissile1.spr");

            pev->rendermode = kRenderTransAdd;
            pev->renderfx = kRenderFxNone;
            pev->renderamt = 200;
            pev->solid = SOLID_BBOX;
            pev->movetype = MOVETYPE_FLYMISSILE;
            pev->nextthink = gpGlobals->time + 0.1s;
            m_iType = 0;
            m_bHit = false;

            m_flMaxFrames = float(MODEL_FRAMES(pev->modelindex) - 1);

            UTIL_SetOrigin(pev, pev->origin);
            UTIL_SetSize(pev, Vector(-1.0, -1.0, -1.0), Vector(1.0, 1.0, 1.0));
        }

        void Precache() override
        {
            PRECACHE_MODEL("sprites/ef_magicmgmissile1.spr");
            PRECACHE_MODEL("sprites/ef_magicmgmissile2.spr");
 
            m_iSprExplo[0] = PRECACHE_MODEL("sprites/ef_magicmgexplo.spr");
            m_iSprExplo[1] = PRECACHE_MODEL("sprites/ef_magicmgexplo2.spr");
        }

        void EXPORT OnTouch(CBaseEntity* pOther)
        {
            if (!pOther)
                return;

            if (pOther->edict() == pev->owner)
                return;

            TraceResult tr = UTIL_GetGlobalTrace();
            entvars_t* pevOwner = VARS(pev->owner);


            if (!pOther->IsDormant())
            {
                if (pOther->pev->takedamage != DAMAGE_NO)
                {
                    if (g_pGameRules->PlayerRelationship(m_pOwner, pOther) != GR_TEAMMATE)
                    {
                        ClearMultiDamage();

                        if (m_iType)
                            pOther->m_LastHitGroup = HITGROUP_CHEST;

                        pOther->TraceAttack(pevOwner, m_flTouchDamage, pev->velocity.Normalize(), &tr, DMG_BULLET | DMG_NEVERGIB);

                        ApplyMultiDamage(pev, pevOwner);

                        if(pOther->IsPlayer() || pOther->Classify() == CLASS_PLAYER_ALLY)
                            m_bHit = true;
                    }             
                }
            }

            Explode(pOther);

            if (!m_iType && m_bHit)
            {
                m_pWeapon->pev->iuser1++;
            }

            SetThink(nullptr);
            SetTouch(nullptr);
            UTIL_Remove(this);
        }

        void Explode(CBaseEntity* pOther)
        {
            UTIL_EMIT_SOUND(this->edict(), CHAN_ITEM, m_iType ? "weapons/magicmg_2exp.wav" : "weapons/magicmg_1exp.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

            MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
            WRITE_BYTE(TE_TEMPSPRITE);
            WRITE_COORD(pev->origin.x);
            WRITE_COORD(pev->origin.y);
            WRITE_COORD(pev->origin.z);
            WRITE_SHORT(m_iType ? m_iSprExplo[1] : m_iSprExplo[0]);
            WRITE_BYTE(8);
            WRITE_BYTE(200);
            WRITE_BYTE(30);
            MESSAGE_END();

            CBaseEntity* pEntity = NULL;

            while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, m_iType ? 120 : 60)) != NULL)
            {
                if (pEntity->IsDormant())
                    continue;

                if (pEntity->pev->takedamage == DAMAGE_NO)
                    continue;

                if (pEntity == pOther)
                    continue;

                if (pEntity == m_pOwner)
                    continue;

                if (g_pGameRules->PlayerRelationship(m_pOwner, pEntity) == GR_TEAMMATE)
                    continue;

                if (pOther->IsPlayer() || pOther->Classify() == CLASS_PLAYER_ALLY)
                    m_bHit = true;

                TraceResult tr = UTIL_GetGlobalTrace();

                ClearMultiDamage();

                pEntity->m_LastHitGroup = HITGROUP_CHEST;
                pEntity->TraceAttack(m_pOwner->pev, m_flTouchDamage, pev->velocity.Normalize(), &tr, DMG_BULLET | DMG_NEVERGIB);

                ApplyMultiDamage(pev, m_pOwner->pev);
            }
        }

        void EXPORT FollowThink()
        {
            pev->nextthink = gpGlobals->time + 0.01s;

            pev->frame += pev->framerate * gpGlobals->frametime / 1s;

            if (pev->frame > m_flMaxFrames)
                pev->frame = fmod(pev->frame, m_flMaxFrames);

            if (m_iType && m_flRemoveTime < gpGlobals->time)
            {
                SetThink(nullptr);
                SetTouch(nullptr);
                UTIL_Remove(this);
            }

            if (!m_iType)
            {
                if (pev->iuser2)
                {
                    pev->angles[2] += float(pev->iuser1);

                    if (pev->angles[2] >= 360.0)
                        pev->angles[2] = 0.0;
                }
                else
                {
                    pev->angles[2] -= float(pev->iuser1);

                    if (pev->angles[2] <= 0.0)
                        pev->angles[2] = 360.0;
                }        
            }  
        }

        void Init(Vector vecVelocity, float flTouchDamage, float flExplodeDamage, int iType, CBasePlayer* pOwner, CMagicMG* pWeapon)
        {
            std::tie(m_flTouchDamage, m_flRadiusDamage, m_iType, m_pOwner, m_pWeapon) = std::make_tuple(flTouchDamage, flExplodeDamage, iType, pOwner, pWeapon);
            pev->velocity = std::move(vecVelocity);


            if (m_iType)
            {
                SET_MODEL(this->edict(), "sprites/ef_magicmgmissile2.spr");
                pev->framerate = 15;
                pev->scale = 0.8;
            }           
            else
            {
                SET_MODEL(this->edict(), "sprites/ef_magicmgmissile1.spr");
                pev->framerate = 1;
                pev->scale = 0.3;
            }
               
           
            m_flMaxFrames = float(MODEL_FRAMES(pev->modelindex) - 1);
            UTIL_SetSize(pev, Vector(-1.0, -1.0, -1.0), Vector(1.0, 1.0, 1.0));
        }

        KnockbackData GetKnockBackData() override { return  { 500.0f, 250.0f, 450.0f, 300.0f, 0.3f }; }

        CBasePlayer* m_pOwner;
        CMagicMG* m_pWeapon;
        time_point_t m_flRemoveTime;
        bool m_bHit;
        float m_flMaxFrames;
        float m_flTouchDamage;
        float m_flRadiusDamage;
        int m_iSprExplo[2];
        int m_iType;

    };

    LINK_ENTITY_TO_CLASS(magiccannon, CMagicCannon)
#endif

    void CMagicMG::WeaponIdle()
    {
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

        if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
            return;

        if (m_flTimeWeaponIdle2 < 5s)
        {
            m_flTimeWeaponIdle2 += m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
            SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);
        }
        else
        {
#ifndef CLIENT_DLL
            PLAYBACK_EVENT_FULL(FEV_HOSTONLY, m_pPlayer->edict(), m_usFire, 0.67, g_vecZero, g_vecZero, 0.0, 0.0, 0, 3, FALSE, FALSE);
#endif
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3s;
            SendWeaponAnim(ANIM_IDLE2, UseDecrement() != FALSE);
            m_flTimeWeaponIdle2 = 0s;
        }
    }

    void CMagicMG::StopIdleSound(void)
    {
#ifndef CLIENT_DLL
        int player = m_pPlayer->entindex();

        MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->edict());
        WRITE_BYTE(79);
        WRITE_SHORT(player);
        WRITE_SHORT(CHAN_AUTO);
        WRITE_STRING("weapons/magicmg_idle.wav");
        MESSAGE_END();

#endif
    }

    void CMagicMG::ItemPostFrame()
    {
        if (!m_iWeaponState)
        {
            if (pev->iuser1 >= GetShotsRequired())
            {
                m_iWeaponState++;
                m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iWeaponState;

                UTIL_EMIT_SOUND(this->edict(), CHAN_ITEM, "weapons/magicmg_alarm.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM, 1);

                pev->iuser1 = 0;
#ifndef CLIENT_DLL
                MESSAGE_BEGIN(MSG_ONE, gmsgAmmoPickup, NULL, m_pPlayer->pev);
                WRITE_BYTE(CBasePlayer::GetAmmoIndex("magicmg_cannon"));
                WRITE_BYTE(1);
                MESSAGE_END();
#endif
            }
        }

        return CBasePlayerWeapon::ItemPostFrame();
    }

    void CMagicMG::Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_iShotsFired++;
        m_flAccuracy = (m_iShotsFired * m_iShotsFired * m_iShotsFired) / 220.0 + 0.3;

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

        SetThink(nullptr);
        m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;
        m_iClip--;
       
        if (m_pPlayer->pev->effects & EF_MUZZLEFLASH)
            m_pPlayer->pev->effects &= ~EF_MUZZLEFLASH;

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecDir = m_pPlayer->FireBullets3(vecSrc - gpGlobals->v_up * 2.0, gpGlobals->v_forward, flSpread, 8192.0, 0, BULLET_PLAYER_556MM, 0, 0.0, m_pPlayer->pev, false, m_pPlayer->random_seed);

#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);

        Vector vecCannonDir = gpGlobals->v_forward + gpGlobals->v_right * vecDir.x + gpGlobals->v_up * vecDir.y;
        Vector vecAngles = UTIL_VecToAngles(vecCannonDir);

        CMagicCannon* pEnt = static_cast<CMagicCannon*>(CBaseEntity::Create("magiccannon", vecSrc, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev)));
        if (pEnt)
        {
            pEnt->Init(vecCannonDir * 1000, GetTouchDamageA(), GetTouchDamageA(), 0, m_pPlayer, this);
            pEnt->pev->iuser1 = RANDOM_LONG(5, 10);
            pEnt->pev->iuser2 = RANDOM_LONG(0, 1);
        }
#endif

        int flags;

#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0.0, g_vecZero, g_vecZero, 0.0, 0.0, 0, 0, FALSE, FALSE);

        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.03s;

#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", 0, 0);
#endif

        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            KickBack(1.2, 0.5, 0.23, 0.15, 5.5, 3.5, 6);
        else if (m_pPlayer->pev->velocity.Length2D() > 0)
            KickBack(1.0, 0.45, 0.28, 0.045, 3.75, 3.0, 7);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            KickBack(0.6, 0.3, 0.2, 0.0125, 3.25, 2.0, 7);
        else
            KickBack(0.65, 0.35, 0.25, 0.015, 3.5, 2.25, 7);
    }

    void CMagicMG::SecondaryAttack()
    {
        if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] <= 0)
        {
            m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.0s;
            return;
        }
           
#ifndef CLIENT_DLL
        if (m_pPlayer->m_iModelName == MODEL_BUFFCLASS19S3CT ||
            m_pPlayer->m_iModelName == MODEL_BUFFCLASS19S3TR)
            UTIL_SendAudio(m_pPlayer, "%!MRAD_MAGICMG", PITCH_NORM);
#endif

        PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0.0, g_vecZero, g_vecZero, 0.0, 0.0, 0, 0, TRUE, FALSE);

        SetThink(&CMagicMG::DelaySeconadryAttack);
        pev->nextthink = gpGlobals->time + 1.0s;

        m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.0s;
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.03s;
    }

    void CMagicMG::DelaySeconadryAttack()
    {
        SetThink(nullptr);

        m_iWeaponState--;
        m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iWeaponState;

        m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;

        if (m_pPlayer->pev->effects & EF_MUZZLEFLASH)
            m_pPlayer->pev->effects &= ~EF_MUZZLEFLASH;

#ifndef CLIENT_DLL
        m_pPlayer->pev->health += m_pPlayer->pev->max_health * GetHealthRate();
        m_pPlayer->pev->health = min(m_pPlayer->pev->health, m_pPlayer->pev->max_health);

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecDir = m_pPlayer->FireBullets3(vecSrc - gpGlobals->v_up * 2.0, gpGlobals->v_forward, 0.0, 8192.0, 0, BULLET_PLAYER_556MM, 0, 0.0, m_pPlayer->pev, false, m_pPlayer->random_seed);

        m_pPlayer->SetAnimation(PLAYER_ATTACK1);

        Vector vecCannonDir = gpGlobals->v_forward + gpGlobals->v_right * vecDir.x + gpGlobals->v_up * vecDir.y;
        Vector vecAngles = UTIL_VecToAngles(vecCannonDir);

        CMagicCannon* pEnt = static_cast<CMagicCannon*>(CBaseEntity::Create("magiccannon", vecSrc, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev)));
        if (pEnt)
        {
            pEnt->Init(vecCannonDir * 1000, GetTouchDamageB(), GetTouchDamageB(), 1, m_pPlayer, this);
            pEnt->m_flRemoveTime = gpGlobals->time + 3.0s;
        }
#endif

    }

    void CMagicMG::Holster(int skiplocal)
    {
        SetThink(nullptr);
        StopIdleSound();

        CBasePlayerWeapon::Holster(skiplocal);
    }

    void CMagicMG::Reload(void)
    {
        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            return;

        if (DefaultReload(MaxClip, ANIM_RELOAD, DefaultReloadTime))
        {
            PLAYBACK_EVENT_FULL(FEV_HOSTONLY, m_pPlayer->edict(), m_usFire, 0.5, g_vecZero, g_vecZero, 0.0, 0.0, 0, 2, FALSE, FALSE);

            PLAYBACK_EVENT_FULL(FEV_HOSTONLY, m_pPlayer->edict(), m_usFire, 2.46, g_vecZero, g_vecZero, 0.0, 0.0, 0, 4, FALSE, FALSE);
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
            m_flAccuracy = 0.2;
            m_iShotsFired = 0;
            m_bDelayFire = false;
            m_flTimeWeaponIdle2 = 0s;
        }
    }

}