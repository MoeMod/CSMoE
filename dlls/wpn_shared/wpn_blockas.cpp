/*
blockas.cpp
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
    class CBlockASMissile : public CGrenade
    {
    public:
        //float GetArmorRatioModifier() { return 1.0; }
        KnockbackData GetKnockBackData() override { return { 350.0f, 450.0f, 600.0f, 450.0f, 0.5f }; }
        int GetWeaponsId(void) { return WEAPON_BLOCKAS; }

        static CBlockASMissile* Create(Vector vecSrc, Vector vecDir, CBasePlayer* pPlayer)
        {
            CBlockASMissile* pMissile = (CBlockASMissile*)CBaseEntity::Create("blockas_missile", vecSrc, UTIL_VecToAngles(vecDir), pPlayer->edict());

            if (!pMissile)
                return NULL;

            Vector pos = vecSrc + vecDir * 35;

            MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
            WRITE_BYTE(TE_SPRITE);
            WRITE_COORD(pos.x);
            WRITE_COORD(pos.y);
            WRITE_COORD(pos.z);
            WRITE_SHORT(g_sModelIndexSmokePuff);
            WRITE_BYTE(20);
            WRITE_BYTE(128);
            MESSAGE_END();

            UTIL_BreakModel(pos, Vector(10, 10, 10), Vector(0, 0, 0), 10, pMissile->m_iShellModelIndex, 0x40, 10, 3);

            pMissile->pev->velocity = gpGlobals->v_forward * 300;
            pMissile->pev->dmg = pMissile->GetExplodeDamage();
            pMissile->m_iTeam = pPlayer->m_iTeam;

            return pMissile;
        }

        void Spawn(void) override
        {
            Precache();

            pev->movetype = MOVETYPE_TOSS;
            pev->solid = SOLID_BBOX;
            pev->gravity = 0.5;

            SET_MODEL(ENT(pev), "models/blockas2_straybullet.mdl");

            UTIL_SetSize(pev, g_vecZero, g_vecZero);
            UTIL_SetOrigin(pev, pev->origin);

            SetThink(&CBlockASMissile::IgniteThink);
            SetTouch(&CBlockASMissile::RocketTouch);

            pev->angles.x -= 30.0;
            UTIL_MakeVectors(pev->angles);
            pev->angles.x = -(pev->angles.x + 30.0);

            pev->nextthink = gpGlobals->time + 0.1s;
        }

        void Precache(void) override
        {
            PRECACHE_MODEL("models/blockas2_straybullet.mdl");

            m_iBeamModelIndex = PRECACHE_MODEL("sprites/smoke.spr");
            m_iShellModelIndex = PRECACHE_MODEL("models/block_shell.mdl");
        }

    public:
        void EXPORT IgniteThink()
        {
            if (!IsInWorld())
            {
                SetThink(NULL);
                SetTouch(NULL);

                UTIL_Remove(this);
                return;
            }

            MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
            WRITE_BYTE(TE_BEAMFOLLOW);
            WRITE_SHORT(entindex());
            WRITE_SHORT(m_iBeamModelIndex);
            WRITE_BYTE(30);
            WRITE_BYTE(5);
            WRITE_BYTE(224);
            WRITE_BYTE(224);
            WRITE_BYTE(255);
            WRITE_BYTE(200);
            MESSAGE_END();

            SetThink(&CBlockASMissile::FollowThink);

            pev->nextthink = gpGlobals->time + 0.1s;
        }

        void EXPORT FollowThink(void)
        {
            if (!IsInWorld())
            {
                MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
                WRITE_BYTE(TE_KILLBEAM);
                WRITE_SHORT(this->entindex());
                MESSAGE_END();

                SetThink(NULL);
                SetTouch(NULL);

                UTIL_Remove(this);
                return;
            }

            UTIL_MakeAimVectors(pev->angles);

            pev->angles = UTIL_VecToAngles(gpGlobals->v_forward);
            pev->nextthink = gpGlobals->time + 0.1s;
        }

        void EXPORT RocketTouch(CBaseEntity* pOther)
        {
            CBasePlayer* pOwner = (CBasePlayer*)CBaseEntity::Instance(pev->owner);

            if (pOther && pOwner && pOwner->IsAlive() && !pOther->IsDormant() && pOther->pev->takedamage != DAMAGE_NO)
                pOther->TakeDamage(pev, pOwner->pev, GetDirectDamage(), DMG_GENERIC);

            ExplodeTouch3(pOther, 5.7 * 39.37, CGrenade::EXPTYPE_AT4ROCKET, false);
        }

    private:
        float GetDirectDamage()
        {
            float flDamage = 70;
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flDamage = 490;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = 770;
#endif
            return flDamage;
        }

        float GetExplodeDamage()
        {
            float flDamage = 155;
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flDamage = 1085;	//3500
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = 1705;
#endif
            return flDamage;
        }

        int m_iBeamModelIndex;
        int m_iShellModelIndex;
    };

    LINK_ENTITY_TO_CLASS(blockas_missile, CBlockASMissile);
#endif

    class CBlockAS : public LinkWeaponTemplate< CBlockAS,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_M3>::template type,
            TPrecacheEvent,
            TGetDamageDefault
    >
    {
    public:
    static constexpr const char *V_Model = "models/v_blockas1.mdl";
    static constexpr const char *P_Model = "models/p_blockas1.mdl";
    static constexpr const char *W_Model = "models/w_blockas1.mdl";
    static constexpr const char *EventFile = "events/blockas.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_blockas";
    static constexpr const char *AnimExtension = "shotgun";
    static constexpr int MaxClip = 8;

    enum blockas1_e
    {
        BLOCKAS1_IDLE,
        BLOCKAS1_SHOOT1,
        BLOCKAS1_SHOOT2,
        BLOCKAS1_SHOOT3,
        BLOCKAS1_CHANGE1,
        BLOCKAS1_CHANGE2,
        BLOCKAS1_RELOAD_START,
        BLOCKAS1_RELOAD_LOOP,
        BLOCKAS1_RELOAD_END,
        BLOCKAS1_DRAW,
    };

    enum blockchange_e
    {
        BLOCKCHANGE_CHANGE
    };

    enum blockas2_e
    {
        BLOCKAS2_IDLE,
        BLOCKAS2_IDLE_EMPTY,
        BLOCKAS2_SHOOT_START,
        BLOCKAS2_SHOOT_END,
        BLOCKAS2_CHANGE1,
        BLOCKAS2_CHANGE1_EMPTY,
        BLOCKAS2_CHANGE2,
        BLOCKAS2_CHANGE2_EMPTY,
        BLOCKAS2_RELOAD,
        BLOCKAS2_DRAW,
        BLOCKAS2_DRAW_EMPTY
    };

    static constexpr float PlayerMaxSpeed = 230;
    static constexpr float ArmorRatioModifier = 1.1f;

    static constexpr int DamageDefault = 20;
    static constexpr int DamageZB = 28;
    static constexpr int DamageZBS = 60;


    static constexpr auto BulletType = BULLET_PLAYER_BUCKSHOT;
    /*static constexpr*/ int Penetration = 1;
    KnockbackData KnockBack = { 700.0f, 450.0f, 600.0f, 450.0f, 0.4f };

    time_point_t m_flRocketReloadTime;
    time_point_t m_flTimeWeaponIdle2;
    int m_iDefaultAmmo2;
    duration_t m_flPumpTime;

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "blockbuckshot";
        p->iMaxAmmo1 = 32;
        p->pszAmmo2 = "blockasmissile";
        p->iMaxAmmo2 = 10;
        p->iMaxClip = 8;
        p->iSlot = 0;
        p->iPosition = 1;
        p->iId = m_iId = WEAPON_BLOCKAS;
        p->iFlags = 0;
        p->iWeight = M4A1_WEIGHT;

        return 1;
    }

    float GetArmorRatioModifier() override { return 1.1; }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override 
    {
        if (m_iWeaponState == 3)
            return { "ammo_blockasmissile" , 200 };
        else
            return { "ammo_blockbuckshot" , 65 };   
    }
#endif
    WeaponCrosshairData GetCrosshairData() override
    {
        return { ACCURACY_AIR | ACCURACY_SPEED, 9, 4, 0 };
    }

    void Spawn()
    {
        m_flRocketReloadTime = invalid_time_point;
        m_iWeaponState = 0;
        m_flTimeWeaponIdle2 = invalid_time_point;
        m_iDefaultAmmo2 = 10;

        Base::Spawn();
    }

    void Precache(void)
    {
        Base::Precache();

        PRECACHE_MODEL("models/w_blockas2.mdl");
        PRECACHE_MODEL("models/v_blockas2.mdl");
        PRECACHE_MODEL("models/p_blockas2.mdl");
        PRECACHE_MODEL("models/v_blockchange.mdl");
        PRECACHE_SOUND("weapons/blockas2_shoot_end.wav");

#ifndef CLIENT_DLL
        UTIL_PrecacheOther("block_missile");
#endif
        m_iShellId = PRECACHE_MODEL("models/blockas1_bullet.mdl");
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

    BOOL Deploy(void) override
    {
        StopIdleSound();
        SetThink(NULL);

        m_iShotsFired = 0;
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;
        m_flAccuracy = 0.1;

        if (m_flRocketReloadTime != invalid_time_point && gpGlobals->time > m_flRocketReloadTime)
            m_flRocketReloadTime = gpGlobals->time + 1.03s;

        m_flTimeWeaponIdle2 = invalid_time_point;

        if (m_iWeaponState == 1)
            m_iWeaponState = 3;
        else if (m_iWeaponState == 2)
            m_iWeaponState = 0;

#ifndef CLIENT_DLL
        m_pPlayer->UpdateShieldCrosshair(m_iWeaponState == 0);
#endif

        if (m_iWeaponState != 0)
        {
            if (m_flRocketReloadTime == invalid_time_point && GetNumRockets() > 0)
                return DefaultDeploy("models/v_blockas2.mdl", "models/p_blockas2.mdl", BLOCKAS2_DRAW, "m249", FALSE, 1.0s, 1.0s);
            else
                return DefaultDeploy("models/v_blockas2.mdl", "models/p_blockas2.mdl", BLOCKAS2_DRAW_EMPTY, "m249", FALSE, 1.0s, 1.0s);
        }
        else
            return DefaultDeploy("models/v_blockas1.mdl", "models/p_blockas1.mdl", BLOCKAS1_DRAW, "shotgun", FALSE, 1.0s, 1.03s);
    }


#ifndef CLIENT_DLL
    void BlockASMissileFire(float flSpread, duration_t flCycleTime)
    {
        m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
        m_pPlayer->pev->effects &= ~EF_MUZZLEFLASH;

        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

        Vector vecSrc = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 16 + gpGlobals->v_right * 8 + gpGlobals->v_up * -8;
        CBlockASMissile* pMissile = CBlockASMissile::Create(vecSrc, gpGlobals->v_forward, m_pPlayer);

        UTIL_MakeVectors(m_pPlayer->pev->v_angle);
        if (pMissile)
        {
            pMissile->pev->velocity += gpGlobals->v_forward * 800;
        }
      
        EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/blockas2_shoot_end.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            KickBack(13.0, 3.0, 0.85, 0.25, 15.0, 5.6, 1);
        else if (m_pPlayer->pev->velocity.Length2D() > 0)
            KickBack(13.0, 2.25, 0.8, 0.1, 15.0, 5.6, 1);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            KickBack(9.0, 2.1, 0.5, 0.2, 15.0, 5.0, 1);
        else
            KickBack(13.0, 3.2, 0.55, 0.2, 15.0, 10.0, 2);
    }
#endif

    const char* GetWorldModel(void)
    {
        switch (m_iWeaponState)
        {
        case 0:
        case 2:
            return "models/w_blockas1.mdl";

        case 1:
        case 3:
            return "models/w_blockas2.mdl";

        default:
            return NULL;
        }
    }

    void Holster(int skiplocal)
    {
        SetThink(NULL);

        m_flTimeWeaponIdle2 = invalid_time_point;

        if (m_iWeaponState == 1)
            m_iWeaponState = 3;
        else if (m_iWeaponState == 2)
            m_iWeaponState = 0;

        StopIdleSound();
        CBasePlayerWeapon::Holster(skiplocal);
    }


    void PrimaryAttack(void)
    {
        if (!m_iWeaponState)
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

            m_iClip--;
            m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

            UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
#ifndef CLIENT_DLL
            m_pPlayer->FireBullets(6, m_pPlayer->GetGunPosition(), gpGlobals->v_forward, Vector(0.0725, 0.0725, 0), 3048, BULLET_PLAYER_BUCKSHOT, 0, GetDamage());
#endif

            int flags;
#ifdef CLIENT_WEAPONS
            flags = FEV_NOTHOST;
#else
            flags = 0;
#endif

            PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFire, 0, g_vecZero, g_vecZero, 0, 0, 0, 0, FALSE, FALSE);

#ifndef CLIENT_DLL
            if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
                m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
            if (m_iClip)
                m_flPumpTime = UTIL_WeaponTimeBase() + 0.125s;

            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.3s;
            m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;

            if (m_iClip)
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5s;
            else
                m_flTimeWeaponIdle = 0.7s;

            m_fInSpecialReload = 0;

            if (m_pPlayer->pev->flags & FL_ONGROUND)
                m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 3, 5);           
            else
                m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 7, 10);
        }
        else if (m_iWeaponState == 3)
        {
            if (GetNumRockets() > 0)
            {
                SendWeaponAnim(BLOCKAS2_SHOOT_START, UseDecrement());

                m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0s;

                pev->nextthink = gpGlobals->time + 0.96s;
                SetThink(&CBlockAS::ShootMissile);
            }
            else
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
            }
        }
    }

    void ItemPostFrame(void)
    {
        CBasePlayerWeapon::ItemPostFrame();

        if (m_flTimeWeaponIdle2 != invalid_time_point && gpGlobals->time > m_flTimeWeaponIdle2)
        {
            switch (m_iWeaponState)
            {
            case 0:
            case 3:
                m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_blockchange.mdl");

                SendWeaponAnim(BLOCKCHANGE_CHANGE);
                m_iWeaponState = 2 - (m_iWeaponState != 0);
                m_flTimeWeaponIdle2 = gpGlobals->time + 2.36s;
                break;

            case 2:
                m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_blockas2.mdl");
                m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_blockas2.mdl");

                strcpy(m_pPlayer->m_szAnimExtention, "m249");

                if (GetNumRockets() > 0)
                    SendWeaponAnim(BLOCKAS2_CHANGE2);
                else
                    SendWeaponAnim(BLOCKAS2_CHANGE2_EMPTY);
#ifndef CLIENT_DLL
                m_pPlayer->UpdateShieldCrosshair(FALSE);
#endif
                m_iWeaponState = 3;
                m_flTimeWeaponIdle2 = invalid_time_point;
                break;

            case 1:
                m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_blockas1.mdl");
                m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_blockas1.mdl");

                strcpy(m_pPlayer->m_szAnimExtention, "rifle");

                SendWeaponAnim(BLOCKAS1_CHANGE2);
#ifndef CLIENT_DLL
                m_pPlayer->UpdateShieldCrosshair(TRUE);
#endif
                m_iWeaponState = 0;
                m_flTimeWeaponIdle2 = invalid_time_point;
                break;
            }
        }

        if (m_iWeaponState == 3)
        {
            if (m_flRocketReloadTime > invalid_time_point && gpGlobals->time > m_flRocketReloadTime && m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] > 0)
            {
                RocketReload();

                m_flRocketReloadTime = invalid_time_point;
            }
        }
    }

    void Reload(void)
    {
        if (m_iWeaponState)
            return;

        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip == MaxClip)
            return;

        if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase())
            return;

        if (!m_fInSpecialReload)
        {
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
            SendWeaponAnim(BLOCKAS1_RELOAD_START, UseDecrement() != FALSE);

            m_fInSpecialReload = 1;
            m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.35s;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.35s;
            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.35s;
            m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.35s;
        }
        else if (m_fInSpecialReload == 1)
        {
            if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
                return;

            m_fInSpecialReload = 2;
            SendWeaponAnim(BLOCKAS1_RELOAD_LOOP, UseDecrement() != FALSE);

            m_flNextReload = UTIL_WeaponTimeBase() + 0.4s;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.4s;
        }
        else
        {
            m_iClip++;
            m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
            m_fInSpecialReload = 1;
        }
    }

    void RocketReload(void)
    {
        SendWeaponAnim(BLOCKAS2_RELOAD);

        SetThink(NULL);

        m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.7s;
    }

    int GetNumRockets(void)
    {
        return m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType];
    }

    int PrimaryAmmoIndex(void)
    {
#ifndef CLIENT_DLL
        if (m_iWeaponState == 3)
            return CBasePlayer::GetAmmoIndex("blockasmissile");
        else if (m_iWeaponState == 0)
            return CBasePlayer::GetAmmoIndex("blockbuckshot");
        else
            return -1;
#else
        return -1;
#endif
    }

    void SecondaryAttack(void)
    {
        if (m_flNextSecondaryAttack > UTIL_WeaponTimeBase())
            return;

        StopIdleSound();

        if (!m_iWeaponState)
        {
            SendWeaponAnim(BLOCKAS1_CHANGE1, UseDecrement());

            PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usFire, 0.53, g_vecZero, g_vecZero, 0.0, 0.0, 0, 0, TRUE, FALSE);
        }
        else if (GetNumRockets() > 0)
        {
            SendWeaponAnim(BLOCKAS2_CHANGE1, UseDecrement());
        }
        else
        {
            SendWeaponAnim(BLOCKAS2_CHANGE1_EMPTY, UseDecrement());
        }
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
        m_flTimeWeaponIdle2 = gpGlobals->time + 1.2s;
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.4s + 2.36s;
    }

    void ShootMissile(void)
    {
#ifndef CLIENT_DLL
        if (FNullEnt(m_pPlayer->pev))
            return;

        if (!m_pPlayer->IsAlive())
            return;

        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            BlockASMissileFire(0.035, 3.0s);
        else if (m_pPlayer->pev->velocity.Length2D() > 0)
            BlockASMissileFire(0.02, 3.0s);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            BlockASMissileFire(0.01, 3.0s);
        else
            BlockASMissileFire(0.01, 3.0s);    
#endif
        m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;

        SendWeaponAnim(BLOCKAS2_SHOOT_END, UseDecrement());

        m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
        m_flRocketReloadTime = gpGlobals->time + 1.0s;

        return;
    }

    void StopIdleSound(void)
    {
#ifndef CLIENT_DLL
        MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
        WRITE_BYTE(79);
        WRITE_SHORT(m_pPlayer->entindex());
        WRITE_SHORT(CHAN_AUTO);
        WRITE_STRING("weapons/blockas2_idle.wav");
        MESSAGE_END();
#endif
    }

    void WeaponIdle(void)
    {
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

        if (m_flPumpTime != zero_duration && m_flPumpTime < UTIL_WeaponTimeBase())
            m_flPumpTime = zero_duration;

        if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
            return;

        if (!m_iWeaponState)
        {
            if (m_iClip == 0 && m_fInSpecialReload == 0 && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
            {
                Reload();
            }
            else if (m_fInSpecialReload != 0)
            {
                if (m_iClip != MaxClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
                {
                    Reload();
                }
                else
                {
                    SendWeaponAnim(BLOCKAS1_RELOAD_END, UseDecrement() != FALSE);

                    m_fInSpecialReload = 0;
                    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
                }
            }
            else
                SendWeaponAnim(BLOCKAS1_IDLE, UseDecrement() != FALSE);
        }
        else if (GetNumRockets() > 0)
        {
            SendWeaponAnim(BLOCKAS2_IDLE, UseDecrement());
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1000s;

        }
        else
        {
            SendWeaponAnim(BLOCKAS2_IDLE_EMPTY, UseDecrement());
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1000s;

        }
    }

};
LINK_ENTITY_TO_CLASS(weapon_blockas, CBlockAS)
}