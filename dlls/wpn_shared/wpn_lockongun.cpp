/*
lockongun.cpp
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
#include "gamemode/mods.h"
#endif

#include "weapons/WeaponTemplate.hpp"
#include "weapons/KnifeAttack.h"


#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif
    class CLockOnGun : public LinkWeaponTemplate< CLockOnGun,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_AUG>::template type,
            TPrecacheEvent,
            TGetDamageDefault
    >
    {
    public:

    static constexpr const char *V_Model = "models/v_lockongun.mdl";
    static constexpr const char *P_Model = "models/p_lockongun.mdl";
    static constexpr const char *W_Model = "models/w_lockongun.mdl";
    static constexpr const char *EventFile = "events/lockongun.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_lockongun";
    static constexpr const char *AnimExtension = "rifle";
    static constexpr int MaxClip = 50;

#define WPNSTATE_LOCKONGUN_SCOPE_ON 1 
#define WPNSTATE_LOCKONGUN_ZOOM_IN 2
#define LOCK_EXPIRE_TIME 15s
#define LOCK_MAX_TARGET 10

	enum
	{
        ANIM_IDLE1 = 0,
        ANIM_IDLE_ON,
        ANIM_SHOOTA,
        ANIM_SHOOTA_ON,
        ANIM_RELOAD,
        ANIM_RELOAD_ON,
        ANIM_DRAW,
        ANIM_DRAW_ON,
        ANIM_SCOPE_ON,
        ANIM_ZOOM_IN,
        ANIM_ZOOM_IDLE,
        ANIM_ZOOM_OUT,
        ANIM_SHOOTB,
        ANIM_SHOOTB_ON,
	};
    static constexpr float PlayerMaxSpeed = 250;

    static constexpr int DamageDefault = 32;
    static constexpr int DamageZB = 33;
    static constexpr int DamageZBS = 38;

    KnockbackData KnockBack = { 350.0f, 250.0f, 300.0f, 100.0f, 0.5f };

    time_point_t m_flTimeHolster;
    time_point_t m_flTimeNextCharge;
    time_point_t m_flNextThink;
    time_point_t m_flNextReload;


    int m_iSlot;
    int m_iExpIndex;
    int m_iDefaultAmmo2;
   

#ifndef CLIENT_DLL
    time_point_t m_flNextPlayEvent;
    time_point_t m_flNextFire;
    bool m_bSlot[10];
    struct Enemylist
    {
        CBaseEntity* Entity;
        int Hitgroup;
        int Slot;
        time_point_t ExpireTime;
    };
    std::vector<Enemylist> m_pEnemylist;
#endif

    void Spawn() override
    {
        m_iDefaultAmmo2 = 10;
        m_flTimeHolster = gpGlobals->time;
#ifndef CLIENT_DLL
        for (int i = 0; i < 10; i++)
        {
            m_bSlot[i] = false;
        }
#endif
        Base::Spawn();
    }

    void Precache()
    {
        PRECACHE_MODEL("sprites/muzzleflash102.spr");
        PRECACHE_MODEL("sprites/ef_lockongun_reload_left.spr");
        PRECACHE_MODEL("sprites/ef_lockongun_reload.spr");

        PRECACHE_MODEL("sprites/ef_lockongun_smoker_left.spr");
        PRECACHE_MODEL("sprites/ef_lockongun_smoker.spr");
        PRECACHE_MODEL("sprites/ef_lockongun_smokel_left.spr");
        PRECACHE_MODEL("sprites/ef_lockongun_smokel.spr");

        PRECACHE_SOUND("weapons/lockongun_lockon_beep.wav");
        PRECACHE_SOUND("weapons/lockongun_shootb.wav");

        Base::Precache();
    }

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "lockongun";
        p->iMaxAmmo1 = 100;
        p->pszAmmo2 = "lockongun2";
        p->iMaxAmmo2 = 255;
        p->iMaxClip = 50;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_LOCKONGUN;
        p->iFlags = 0;
        p->iWeight = AUG_WEIGHT;

        return 1;
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

    BOOL CanHolster(void) override
    {
#ifndef CLIENT_DLL
        if (m_flNextFire != invalid_time_point)
            return FALSE;
        else
            return TRUE;
#endif
        return FALSE;
    }

    BOOL CanDrop() override
    {
#ifndef CLIENT_DLL
        if (m_flNextFire != invalid_time_point)
            return FALSE;
        else
            return TRUE;
#endif
        return FALSE;
    }

    void Holster(int skiplocal)
    {
        SetThink(nullptr);
        m_flTimeHolster = gpGlobals->time;
#ifndef CLIENT_DLL
        m_pEnemylist.clear();
        ClearSlot();
        m_flNextFire = invalid_time_point;
#endif
        m_iWeaponState &= ~WPNSTATE_LOCKONGUN_ZOOM_IN;
        return CBasePlayerWeapon::Holster(skiplocal);
    }


#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_lockongun" , 350 }; }
#endif
    float GetArmorRatioModifier() override { return 1.75; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 4, 3, 140 }; }

    BOOL Deploy(void)
    {
        m_flNextReload = invalid_time_point;
#ifndef CLIENT_DLL
        m_flNextFire = invalid_time_point;
#endif
        if (m_flTimeHolster != invalid_time_point)
        {
            m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] += (gpGlobals->time - m_flTimeHolster) / 0.5s;
        }

        ResetStateLED();

        return DefaultDeploy("models/v_lockongun.mdl", "models/p_lockongun.mdl", m_iWeaponState & WPNSTATE_LOCKONGUN_SCOPE_ON ? ANIM_DRAW_ON : ANIM_DRAW, "mp5", UseDecrement() != FALSE);
    }

    void ResetStateLED()
    {
        if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] >= 10)
        {
            m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 10;
            m_iWeaponState |= WPNSTATE_LOCKONGUN_SCOPE_ON;
        }
        else
            m_iWeaponState &= ~WPNSTATE_LOCKONGUN_SCOPE_ON;
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

            if (m_iWeaponState & WPNSTATE_LOCKONGUN_SCOPE_ON && pev->iuser1 && pev->iuser1 != 10)
            {
                SendWeaponAnim(ANIM_SCOPE_ON, UseDecrement() != FALSE);
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.7s;
            }
        }

        if (!(m_iWeaponState & WPNSTATE_LOCKONGUN_ZOOM_IN))
        {
            if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] < 10)
            {
                if (m_flTimeNextCharge <= gpGlobals->time)
                {
                    m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]++;

#ifndef CLIENT_DLL
                    MESSAGE_BEGIN(MSG_ONE, gmsgAmmoPickup, NULL, m_pPlayer->pev);
                    WRITE_BYTE(CBasePlayer::GetAmmoIndex("lockongun2"));
                    WRITE_BYTE(1);
                    MESSAGE_END();
#endif

                    if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] >= 10)
                    {
                        if (!m_fInReload)
                        {
                            SendWeaponAnim(ANIM_SCOPE_ON, UseDecrement() != FALSE);
                            pev->iuser1 = 0;
                        }
                            

                        m_iWeaponState |= WPNSTATE_LOCKONGUN_SCOPE_ON;

                        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.7s;

                    }
                    else
                        m_flTimeNextCharge = gpGlobals->time + 0.5s;
                }
            }
        }

        if (m_flNextThink != invalid_time_point && m_flNextThink < gpGlobals->time)
        {
            if (m_pPlayer->pev->fov != 90)
            {
                SendWeaponAnim(ANIM_ZOOM_OUT, UseDecrement() != FALSE);
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.7s;
                m_iWeaponState = 0;
                m_flNextThink = invalid_time_point;
                m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;
            }
            else
            {
                m_iWeaponState |= WPNSTATE_LOCKONGUN_ZOOM_IN;

                m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 89;
                m_flNextThink = invalid_time_point;
            }
        }
#ifndef CLIENT_DLL
        if (m_flNextPlayEvent != invalid_time_point && m_flNextPlayEvent < gpGlobals->time)
        {
            m_flNextPlayEvent = invalid_time_point;

            PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, 0.0, 0.0, 2, 0, FALSE, FALSE);

        }

        if (m_flNextFire != invalid_time_point && m_flNextFire < gpGlobals->time)
        {
            m_flNextFire = gpGlobals->time + 0.02s;     

            MissileFire();
            if (!m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType])
            {
                m_pEnemylist.clear();
                ClearSlot();
                m_flNextFire = invalid_time_point;
            }  
        }
#endif
        if (m_iWeaponState & WPNSTATE_LOCKONGUN_ZOOM_IN)
        {
#ifndef CLIENT_DLL
            CheckEnemyList();
#endif
        }

        if (m_fInReload)
            return;
        return CBasePlayerWeapon::ItemPostFrame();
    }

    void PrimaryAttack() override
    {
        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            Fire(0.035 + (0.4) * m_flAccuracy, 0.0814s, FALSE);
        else if (m_pPlayer->pev->velocity.Length2D() > 140)
            Fire(0.0325 + (0.045) * m_flAccuracy, 0.0814s, FALSE);
        else
            Fire((0.02) * m_flAccuracy, 0.0814s, FALSE);

    }

    void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_bDelayFire = true;
        m_iShotsFired++;
        m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 210) + 0.3;

        if (m_flAccuracy > 1.2)
            m_flAccuracy = 1.2;

        if (m_iClip <= 0)
        {

            if (m_iWeaponState & WPNSTATE_LOCKONGUN_ZOOM_IN)
            {
#ifndef CLIENT_DLL
                Vector vecSrc = m_pPlayer->GetGunPosition();

                Vector vecUp, vecRight, vecForward;
                vecForward = gpGlobals->v_forward;
                vecUp = gpGlobals->v_up;
                vecRight = gpGlobals->v_right;

                int HitResult = GetLineTarget(vecSrc, vecForward, vecRight, vecUp, g_vecZero, 8192, m_pPlayer);
#endif
                SendWeaponAnim(ANIM_ZOOM_IDLE, UseDecrement() != FALSE);
                
                m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.03s;
                return;
            }
               
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
        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
        Vector vecSrc = m_pPlayer->GetGunPosition();
        
        Vector vecUp, vecRight, vecForward;
        vecForward = gpGlobals->v_forward;
        vecUp = gpGlobals->v_up;
        vecRight = gpGlobals->v_right;

        Vector vecDir;

#ifndef CLIENT_DLL
        if (m_iWeaponState & WPNSTATE_LOCKONGUN_ZOOM_IN)
        {
            vecDir = FireBullets3(vecSrc, vecForward, flSpread, 8192, 3, BULLET_PLAYER_556MM, GetDamage(), 0.97, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);
        }
        else
            vecDir = m_pPlayer->FireBullets3(vecSrc, vecForward, flSpread, 8192, 3, BULLET_PLAYER_556MM, GetDamage(), 0.97, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);
#else
        vecDir = m_pPlayer->FireBullets3(vecSrc, vecForward, flSpread, 8192, 3, BULLET_PLAYER_556MM, GetDamage(), 0.97, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);
#endif
        

#if 0
        if(m_iWeaponState & WPNSTATE_LOCKONGUN_ZOOM_IN)
            int HitResult = GetLineTarget(vecSrc, vecForward, vecRight, vecUp, vecDir, 8192, m_pPlayer);
#endif

        int flags;
#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), m_iWeaponState & WPNSTATE_LOCKONGUN_ZOOM_IN, m_iWeaponState & WPNSTATE_LOCKONGUN_SCOPE_ON);

        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.03s;

        if (m_pPlayer->pev->velocity.Length2D() > 0)
            KickBack(0.875, 0.425, 0.275, 0.0435, 3.5, 2.5, 8);
        else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            KickBack(1.1, 0.47, 0.215, 0.135, 5.2, 3.3, 6);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            KickBack(0.5, 0.24, 0.12, 0.01, 3.15, 2.0, 7);
        else
            KickBack(0.595, 0.32, 0.225, 0.0115, 3.25, 2.1, 8);
        
    }
#ifndef CLIENT_DLL
    void MissileFire()
    {
        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
        Vector vecUp, vecRight, vecForward;

        int iClip2 = m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType];
        vecForward = gpGlobals->v_forward;
        vecUp = gpGlobals->v_up;
        vecRight = gpGlobals->v_right;

        Vector vecSrc = m_pPlayer->GetGunPosition() + vecForward * 5;

        for (Enemylist &Enemy : m_pEnemylist)
        {
            if (Enemy.Slot == -1)
                continue;

            Vector vecStart = vecSrc;
            if (Enemy.Slot % 2)
            {
                vecStart -= vecRight * 20;

                int mid = (Enemy.Slot - 1);

                if(mid >= 4)
                    vecStart += vecUp * (mid - 4) * 4;
                else
                    vecStart -= vecUp * (4 - mid) * 4;
            }             
            else
            {
                vecStart += vecRight * 20;

                int mid = Enemy.Slot;

                if (mid >= 4)
                    vecStart += vecUp * (mid - 4) * 4;
                else
                    vecStart -= vecUp * (4 - mid) * 4;
            }
                

            CLockOnGunMissile* pMissile = CLockOnGunMissile::Create(0, vecStart, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev), Enemy.Entity, Enemy.Hitgroup, m_pPlayer->m_iTeam);

            if (pMissile)
            {
                pMissile->pev->velocity = gpGlobals->v_forward * 1000;
            }

            m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;
            Enemy.Slot = -1;
            break;
        }

        if (iClip2 == m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType])
        {
            for (int i = 0; i < 10; i++)
            {
                Vector vecStart = vecSrc;
                if (!m_bSlot[i])
                {
                    if (i % 2)
                    {
                        vecStart -= vecRight * 20;

                        int mid = (i - 1);

                        if (mid >= 4)
                            vecStart += vecUp * (mid - 4) * 4;
                        else
                            vecStart -= vecUp * (4 - mid) * 4;

                    }

                    else
                    {
                        vecStart += vecRight * 20;

                        int mid = i;

                        if (mid >= 4)
                            vecStart += vecUp * (mid - 4) * 4;
                        else
                            vecStart -= vecUp * (4 - mid) * 4;
                    }

                    CLockOnGunMissile* pMissile = CLockOnGunMissile::Create(0, vecStart, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev), nullptr, -1, m_pPlayer->m_iTeam);

                    if (pMissile)
                    {
                        pMissile->pev->velocity = gpGlobals->v_forward * 1000;
                    }
                    m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;
                    m_bSlot[i] = true;
                    break;
                }
            }
        }

    }

    void ClearSlot()
    {
        //clear all
        MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
        WRITE_BYTE(22);
        MESSAGE_END();

        for (int i = 0; i < 10; i++)
        {
            m_bSlot[i] = false;
        }
    }

    void CheckEnemyList()
    {
        m_pEnemylist.erase(std::remove_if(m_pEnemylist.begin(), m_pEnemylist.end(), [=](Enemylist strEnemy) {

            if (!strEnemy.Entity)
            {
                MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
                WRITE_BYTE(21);
                WRITE_BYTE(0); //time
                WRITE_BYTE(m_bSlot[strEnemy.Slot]); //slot
                WRITE_BYTE(-1);
                MESSAGE_END();

                m_bSlot[strEnemy.Slot] = false;
                return true;
            }
               

            if (!strEnemy.Entity->IsAlive())
            {
                MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
                WRITE_BYTE(21);
                WRITE_BYTE(0); //time
                WRITE_BYTE(strEnemy.Slot); //slot
                WRITE_BYTE(-1);
                MESSAGE_END();

                m_bSlot[strEnemy.Slot] = false;
                return true;
            }

            if (strEnemy.ExpireTime < gpGlobals->time)
            {
                MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
                WRITE_BYTE(21);
                WRITE_BYTE(0); //time
                WRITE_BYTE(strEnemy.Slot); //slot
                WRITE_BYTE(-1);
                MESSAGE_END();

                m_bSlot[strEnemy.Slot] = false;
                return true;
            }

            return false;
            }), m_pEnemylist.end());
    }

    int GetLineTarget(Vector vecSrc, Vector vecDirShooting, Vector vecRight, Vector vecUp, Vector vecSpread, float flDistance, CBaseEntity* pEntity)
    {
        CBasePlayer* pPlayer = (CBasePlayer*)pEntity;
        if (!pPlayer)
            return 0;

        int HitResult = HIT_NONE;

        TraceResult tr;

        Vector vecDir, vecEnd;

        vecDir = vecDirShooting + vecSpread[0] * vecRight + vecSpread[1] * vecUp;
        vecEnd = vecSrc + vecDir * flDistance;

        UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pPlayer->pev), &tr);

        if (tr.flFraction < 1.0)
        {
            HitResult = HIT_WALL;
            CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

            if (pEntity && (pEntity->IsPlayer() || pEntity->Classify() == CLASS_PLAYER_ALLY))
            {
                if (pEntity->IsPlayer())
                {
                    if (g_pGameRules->PlayerRelationship(pPlayer, pEntity) != GR_TEAMMATE)
                    {
                        EMIT_SOUND_DYN(this->edict(), CHAN_WEAPON, "weapons/lockongun_lockon_beep.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);

                        EHANDLE eh;
                        eh.Set(pEntity->edict());

                        tr.iHitgroup = g_pModRunning->GetAdjustedTraceAttackHitgroup(this, m_pPlayer->pev, 0, vecDir, &tr, DMG_BULLET);

                        int HitGroup = tr.iHitgroup;
                        if (HitGroup == HITGROUP_RIGHTARM)
                            HitGroup = HITGROUP_LEFTARM;

                        if (HitGroup == HITGROUP_LEFTLEG || HitGroup == HITGROUP_RIGHTLEG)
                            HitGroup = HITGROUP_RIGHTARM;

                        if (HitGroup >= HITGROUP_SHIELD)
                            HitGroup = HITGROUP_GENERIC;

                       // m_vecVoteMaps
                        if (m_pEnemylist.size() >= 10)
                        {
                            MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
                            WRITE_BYTE(21);
                            WRITE_BYTE(0); //time
                            WRITE_BYTE(m_pEnemylist.at(0).Slot); //slot
                            WRITE_BYTE(-1);
                            MESSAGE_END();

                            m_bSlot[m_pEnemylist.at(0).Slot] = false;

                            m_pEnemylist.erase(m_pEnemylist.begin());                           
                        }
                       
                        int iSlot;
                        for (iSlot = 0; iSlot < 10; iSlot++)
                        {
                            if (!m_bSlot[iSlot])
                            {
                                m_bSlot[iSlot] = true;
                                break;
                            }
                        }

                        Enemylist strEnemy = {pEntity, tr.iHitgroup, iSlot, gpGlobals->time + LOCK_EXPIRE_TIME };
                        m_pEnemylist.push_back(strEnemy);

                        MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
                        WRITE_BYTE(21);
                        WRITE_BYTE(LOCK_EXPIRE_TIME / 1s); //time
                        WRITE_BYTE(iSlot); //slot
                        WRITE_BYTE(HitGroup);
                        MESSAGE_END();

                        HitResult = HIT_PLAYER;
                    }
                        
                }
                else
                {
                    EMIT_SOUND_DYN(this->edict(), CHAN_WEAPON, "weapons/lockongun_lockon_beep.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);


                    HitResult = HIT_PLAYER;
                }
            }
        }
        return HitResult;
    }

#endif
    void SecondaryAttack(void)
    {
        if (m_iWeaponState & WPNSTATE_LOCKONGUN_ZOOM_IN)
        {
            m_iWeaponState = 0;
            SendWeaponAnim(ANIM_SHOOTB, UseDecrement() != FALSE);
 
            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 2.7s;
            m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.7s;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.7s;
            m_flNextThink = invalid_time_point;

            m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;
            m_flTimeNextCharge = gpGlobals->time + 3.0s;
#ifndef CLIENT_DLL
            m_flNextFire = gpGlobals->time + 1.0s;
            m_flNextPlayEvent = gpGlobals->time + 0.56s;
#endif
            return;
        }

        if (m_iWeaponState & WPNSTATE_LOCKONGUN_SCOPE_ON)
        {
            SendWeaponAnim(ANIM_ZOOM_IN, UseDecrement() != FALSE);
            m_flNextThink = gpGlobals->time + 0.6s;
            m_pPlayer->m_flNextAttack = 0.7s;
            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.7s;
            m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.7s;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.7s;
        }
    }

#ifndef CLIENT_DLL
    Vector FireBullets3(Vector vecSrc, Vector vecDirShooting, float vecSpread, float flDistance, int iPenetration,
            int iBulletType, int iDamage, float flRangeModifier, entvars_t* pevAttacker, bool bPistol,
            int shared_rand)
    {
        int iOriginalPenetration = iPenetration;
        int iPenetrationPower;
        float flPenetrationDistance;
        int iCurrentDamage = iDamage;
        float flCurrentDistance;

        TraceResult tr, tr2;
        Vector vecRight, vecUp;

        bool bHitMetal = false;
        int iSparksAmount = 1;

        vecRight = gpGlobals->v_right;
        vecUp = gpGlobals->v_up;

        iPenetrationPower = 35;
        flPenetrationDistance = 4000;
        
        if (!pevAttacker) {
            // the default attacker is ourselves
            pevAttacker = pev;
        }

        gMultiDamage.type = (DMG_BULLET | DMG_NEVERGIB);

        float x, y, z;

        if (IsPlayer()) {
            // Use player's random seed.
            // get circular gaussian spread
            x = UTIL_SharedRandomFloat(shared_rand, -0.5, 0.5) + UTIL_SharedRandomFloat(shared_rand + 1, -0.5, 0.5);
            y = UTIL_SharedRandomFloat(shared_rand + 2, -0.5, 0.5) + UTIL_SharedRandomFloat(shared_rand + 3, -0.5, 0.5);
        }
        else {
            do {
                x = RANDOM_FLOAT(-0.5, 0.5) + RANDOM_FLOAT(-0.5, 0.5);
                y = RANDOM_FLOAT(-0.5, 0.5) + RANDOM_FLOAT(-0.5, 0.5);
                z = x * x + y * y;
            } while (z > 1);
        }

        Vector vecDir, vecEnd;
        Vector vecOldSrc, vecNewSrc;

        vecDir = vecDirShooting + x * vecSpread * vecRight + y * vecSpread * vecUp;
        vecEnd = vecSrc + vecDir * flDistance;


        float flDamageModifier = 0.5;

        while (iPenetration != 0) {
            ClearMultiDamage();
            UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pev), &tr);

            if (TheBots != NULL && tr.flFraction != 1.0f) {
                TheBots->OnEvent(EVENT_BULLET_IMPACT, this, (CBaseEntity*)&tr.vecEndPos);
            }

            char cTextureType = UTIL_TextureHit(&tr, vecSrc, vecEnd);
            bool bSparks = false;

            switch (cTextureType) {
            case CHAR_TEX_METAL:
                bHitMetal = true;
                bSparks = true;

                iPenetrationPower *= 0.15;
                flDamageModifier = 0.2;
                break;
            case CHAR_TEX_CONCRETE:
                iPenetrationPower *= 0.25;
                break;
            case CHAR_TEX_GRATE:
                bHitMetal = true;
                bSparks = true;

                iPenetrationPower *= 0.5;
                flDamageModifier = 0.4;
                break;
            case CHAR_TEX_VENT:
                bHitMetal = true;
                bSparks = true;

                iPenetrationPower *= 0.5;
                flDamageModifier = 0.45;
                break;
            case CHAR_TEX_TILE:
                iPenetrationPower *= 0.65;
                flDamageModifier = 0.3;
                break;
            case CHAR_TEX_COMPUTER:
                bHitMetal = true;
                bSparks = true;

                iPenetrationPower *= 0.4;
                flDamageModifier = 0.45;
                break;
            case CHAR_TEX_WOOD:
                flDamageModifier = 0.6;
                break;
            default:
                break;
            }
            if (tr.flFraction != 1.0f) {
                CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

                iPenetration--;

                flCurrentDistance = tr.flFraction * flDistance;
                iCurrentDamage *= pow(flRangeModifier, flCurrentDistance / 500);

                if (flCurrentDistance > flPenetrationDistance) {
                    iPenetration = 0;
                }

                if (tr.iHitgroup == HITGROUP_SHIELD) {
                    EMIT_SOUND(pEntity->edict(), CHAN_VOICE,
                        (RANDOM_LONG(0, 1) == 1) ? "weapons/ric_metal-1.wav" : "weapons/ric_metal-2.wav", VOL_NORM,
                        ATTN_NORM);
                    UTIL_Sparks(tr.vecEndPos);

                    pEntity->pev->punchangle.x = iCurrentDamage * RANDOM_FLOAT(-0.15, 0.15);
                    pEntity->pev->punchangle.z = iCurrentDamage * RANDOM_FLOAT(-0.15, 0.15);

                    if (pEntity->pev->punchangle.x < 4) {
                        pEntity->pev->punchangle.x = -4;
                    }

                    if (pEntity->pev->punchangle.z < -5) {
                        pEntity->pev->punchangle.z = -5;
                    }
                    else if (pEntity->pev->punchangle.z > 5) {
                        pEntity->pev->punchangle.z = 5;
                    }

                    break;
                }

                float flDistanceModifier;
                if (VARS(tr.pHit)->solid != SOLID_BSP || !iPenetration) {
                    iPenetrationPower = 42;
                    flDamageModifier = 0.75;
                    flDistanceModifier = 0.75;
                }
                else
                    flDistanceModifier = 0.5;

                if (!Q_strcmp(STRING(pEntity->pev->classname), "zombiepile")) {
                    EMIT_SOUND(pEntity->edict(), CHAN_VOICE,
                        (RANDOM_LONG(0, 1) == 1) ? "weapons/ric_metal-1.wav" : "weapons/ric_metal-2.wav", VOL_NORM,
                        ATTN_NORM);
                    UTIL_Sparks(tr.vecEndPos);
                    iPenetration = 0;
                    flDistanceModifier = 0.0;
                }

                DecalGunshot(&tr, iBulletType, (!bPistol && RANDOM_LONG(0, 3)), pev, bHitMetal);

#ifdef TRACE_BULLETS
                Vector vecEndPos2 = tr.vecEndPos - (vecDir * 3);
                MESSAGE_BEGIN(MSG_ALL, SVC_TEMPENTITY);
                WRITE_BYTE(TE_LINE);
                WRITE_COORD(vecEndPos2.x);
                WRITE_COORD(vecEndPos2.y);
                WRITE_COORD(vecEndPos2.z);

                WRITE_COORD(tr.vecEndPos.x);
                WRITE_COORD(tr.vecEndPos.y);
                WRITE_COORD(tr.vecEndPos.z);
                WRITE_SHORT(300);
                WRITE_BYTE(0);
                WRITE_BYTE(0);
                WRITE_BYTE(255);
                MESSAGE_END();
#endif


                vecSrc = tr.vecEndPos + (vecDir * iPenetrationPower);
                flDistance = (flDistance - flCurrentDistance) * flDistanceModifier;
                vecEnd = vecSrc + (vecDir * flDistance);

                pEntity->TraceAttack(pevAttacker, iCurrentDamage, vecDir, &tr, (DMG_BULLET | DMG_NEVERGIB));

                if (iPenetration == iOriginalPenetration - 1)
                {
                    if (pEntity && (pEntity->IsPlayer() || pEntity->Classify() == CLASS_PLAYER_ALLY))
                    {
                        if (pEntity->IsPlayer())
                        {
                            if (g_pGameRules->PlayerRelationship(m_pPlayer, pEntity) != GR_TEAMMATE)
                            {
                                EMIT_SOUND_DYN(this->edict(), CHAN_WEAPON, "weapons/lockongun_lockon_beep.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);

                                EHANDLE eh;
                                eh.Set(pEntity->edict());

                                int HitGroup = tr.iHitgroup;
                                if (HitGroup == HITGROUP_RIGHTARM)
                                    HitGroup = HITGROUP_LEFTARM;

                                if (HitGroup == HITGROUP_LEFTLEG || HitGroup == HITGROUP_RIGHTLEG)
                                    HitGroup = HITGROUP_RIGHTARM;

                                if (HitGroup >= HITGROUP_SHIELD)
                                    HitGroup = HITGROUP_GENERIC;

                                // m_vecVoteMaps
                                if (m_pEnemylist.size() >= 10)
                                {
                                    MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
                                    WRITE_BYTE(21);
                                    WRITE_BYTE(0); //time
                                    WRITE_BYTE(m_pEnemylist.at(0).Slot); //slot
                                    WRITE_BYTE(-1);
                                    MESSAGE_END();

                                    m_bSlot[m_pEnemylist.at(0).Slot] = false;

                                    m_pEnemylist.erase(m_pEnemylist.begin());
                                }

                                int iSlot;
                                for (iSlot = 0; iSlot < 10; iSlot++)
                                {
                                    if (!m_bSlot[iSlot])
                                    {
                                        m_bSlot[iSlot] = true;
                                        break;
                                    }
                                }

                                Enemylist strEnemy = { pEntity, tr.iHitgroup, iSlot, gpGlobals->time + LOCK_EXPIRE_TIME };
                                m_pEnemylist.push_back(strEnemy);

                                MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
                                WRITE_BYTE(21);
                                WRITE_BYTE(LOCK_EXPIRE_TIME / 1s); //time
                                WRITE_BYTE(iSlot); //slot
                                WRITE_BYTE(HitGroup);
                                MESSAGE_END();

                            }

                        }
                        else
                        {
                            EMIT_SOUND_DYN(this->edict(), CHAN_WEAPON, "weapons/lockongun_lockon_beep.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);

                        }
                    }
                }

                iCurrentDamage *= flDamageModifier;
            }
            else
                iPenetration = 0;

            ApplyMultiDamage(pev, pevAttacker);
        }

        return Vector(x * vecSpread, y * vecSpread, 0);
    }
#endif

    void Reload(void)
    {
        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            return;
   
        if (m_iWeaponState & WPNSTATE_LOCKONGUN_ZOOM_IN)
            return;

        if (DefaultReload(50, m_iWeaponState & WPNSTATE_LOCKONGUN_SCOPE_ON ? ANIM_RELOAD_ON : ANIM_RELOAD, 1.9s, 1))
        {
#ifndef CLIENT_DLL         
            m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
            pev->iuser1 = m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType];
            m_flNextReload = gpGlobals->time + 1.9s;

            PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0.65, g_vecZero, g_vecZero, 0.0, 0.0, 3, 0, FALSE, FALSE);
        }
    }

    void WeaponIdle() override
    {
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

        if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
        {
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0s;
            if (m_iWeaponState & WPNSTATE_LOCKONGUN_SCOPE_ON)
            {
                if (m_iWeaponState & WPNSTATE_LOCKONGUN_ZOOM_IN)
                    SendWeaponAnim(ANIM_ZOOM_IDLE, UseDecrement() != FALSE);
                else
                    SendWeaponAnim(ANIM_IDLE_ON, UseDecrement() != FALSE);
            }
            else
            {
                SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);
            }
        }
    }
};
LINK_ENTITY_TO_CLASS(weapon_lockongun, CLockOnGun)

}