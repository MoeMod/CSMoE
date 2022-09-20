/*
blockar.cpp
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
    class CBlockMissile : public CGrenade
    {
    public:
        //float GetArmorRatioModifier() { return 1.0; }
        //KnockbackData GetKnockBackData() override { return { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }; }
        int GetWeaponsId(void) { return WEAPON_BLOCKAR; }

        static CBlockMissile* Create(Vector vecSrc, Vector vecDir, CBasePlayer* pPlayer)
        {
            CBlockMissile* pMissile = (CBlockMissile*)CBaseEntity::Create("block_missile", vecSrc, UTIL_VecToAngles(vecDir), pPlayer->edict());

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

            pMissile->pev->velocity = vecDir * 1400;
            pMissile->pev->dmg = pMissile->GetExplodeDamage();
            pMissile->m_iTeam = pPlayer->m_iTeam;

            return pMissile;
        }

        void Spawn(void) override
        {
            Precache();

            pev->movetype = MOVETYPE_FLY;
            pev->solid = SOLID_BBOX;

            SET_MODEL(ENT(pev), "models/block_missile.mdl");

            UTIL_SetSize(pev, g_vecZero, g_vecZero);
            UTIL_SetOrigin(pev, pev->origin);

            SetThink(&CBlockMissile::RocketThink);
            SetTouch(&CBlockMissile::RocketTouch);

            pev->nextthink = gpGlobals->time + 0.1s;
        }
        void Precache(void) override
        {
            PRECACHE_MODEL("models/block_missile.mdl");

            m_iBeamModelIndex = PRECACHE_MODEL("sprites/smoke.spr");
            m_iShellModelIndex = PRECACHE_MODEL("models/block_shell.mdl");
        }

    public:
        void EXPORT RocketThink()
        {
            if (!IsInWorld())
            {
                SetThink(NULL);
                SetTouch(NULL);

                UTIL_Remove(this);
                return;
            }

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
            float flDamage = 115;
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flDamage = 115 * 6;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = 115 * 6;
#endif
            return flDamage;
        }

        float GetExplodeDamage()
        {
            float flDamage = 80;
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flDamage = 80 * 6;	//3500
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = 80 * 6;
#endif
            return flDamage;
        }

        int m_iBeamModelIndex;
        int m_iShellModelIndex;
    };

    LINK_ENTITY_TO_CLASS(block_missile, CBlockMissile);
#endif

    class CBlockAR : public LinkWeaponTemplate< CBlockAR,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_M4A1>::template type,
            TPrecacheEvent,
            TGetDamageDefault,
            TPrimaryAttackRifle,
            TFireRifle,
            TRecoilKickBack
    >
    {
    public:
    static constexpr const char *V_Model = "models/v_blockar1.mdl";
    static constexpr const char *P_Model = "models/p_blockar1.mdl";
    static constexpr const char *W_Model = "models/w_blockar1.mdl";
    static constexpr const char *EventFile = "events/blockar.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_blockar";
    static constexpr const char *AnimExtension = "carbine";
    static constexpr int MaxClip = 40;

    enum blockar1_e
    {
        BLOCKAR1_IDLE,
        BLOCKAR1_SHOOT1,
        BLOCKAR1_SHOOT2,
        BLOCKAR1_SHOOT3,
        BLOCKAR1_CHANGE1,
        BLOCKAR1_CHANGE2,
        BLOCKAR1_RELOAD,
        BLOCKAR1_DRAW
    };

    enum blockchange_e
    {
        BLOCKCHANGE_CHANGE
    };

    enum blockar2_e
    {
        BLOCKAR2_IDLE,
        BLOCKAR2_IDLE_EMPTY,
        BLOCKAR2_SHOOT_START,
        BLOCKAR2_SHOOT_END,
        BLOCKAR2_CHANGE1,
        BLOCKAR2_CHANGE1_EMPTY,
        BLOCKAR2_CHANGE2,
        BLOCKAR2_CHANGE2_EMPTY,
        BLOCKAR2_RELOAD,
        BLOCKAR2_DRAW,
        BLOCKAR2_DRAW_EMPTY
    };

    static constexpr float PlayerMaxSpeed = 230;
    static constexpr float ArmorRatioModifier = 1.4f;
	static constexpr float PrimaryAttackWalkingMiniumSpeed = 130;
    static constexpr const auto &SpreadCalcNotOnGround = 0.4*A+0.035;
    static constexpr const auto &SpreadCalcWalking = 0.07*A+0.035;
    static constexpr const auto &SpreadCalcDefault = 0.02*A;

	static constexpr const auto& CycleTime = 0.099s;
    static constexpr int DamageDefault = 32;
    static constexpr int DamageZB = 50;
    static constexpr int DamageZBS = 90;

    static constexpr const auto& AccuracyCalc = (N * N * N / 220.0) + 0.3;
    static constexpr float AccuracyDefault = 0.1;
    static constexpr float AccuracyMax = 1.3;
    static constexpr float RangeModifier = 0.97;

    static constexpr auto BulletType = BULLET_PLAYER_762MM;
    /*static constexpr*/ int Penetration = 2;
    KnockbackData KnockBack = { 350.0f, 450.0f, 400.0f, 170.0f, 0.6f };

    KickBackData KickBackWalking = { 1.1, 0.46, 0.28, 0.045, 3.75, 3.0, 8 };
    KickBackData KickBackNotOnGround = { 1.2, 0.5, 0.23, 0.15, 5.5, 3.5, 5 };
    KickBackData KickBackDucking = { 0.62, 0.32, 0.2, 0.0125, 3.25, 2.0, 7 };
    KickBackData KickBackDefault = { 0.66, 0.36, 0.25, 0.015, 3.5, 2.25, 7 };
    
    time_point_t m_flRocketReloadTime;
    time_point_t m_flTimeWeaponIdle2;
    int m_iDefaultAmmo2;

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "blockbullet";
        p->iMaxAmmo1 = 200;
        p->pszAmmo2 = "blockmissile";
        p->iMaxAmmo2 = 10;
        p->iMaxClip = 40;
        p->iSlot = 0;
        p->iPosition = 1;
        p->iId = m_iId = WEAPON_BLOCKAR;
        p->iFlags = 0;
        p->iWeight = M4A1_WEIGHT;

        return 1;
    }

    float GetArmorRatioModifier() override { return 1.4; }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override 
    {
        if (m_iWeaponState == 3)
            return { "ammo_blockmissile" , 200 };
        else
            return { "ammo_blockbullet" , 60 };   
    }
#endif
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 4, 3, 140 }; }

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

        PRECACHE_MODEL("models/w_blockar2.mdl");
        PRECACHE_MODEL("models/v_blockar2.mdl");
        PRECACHE_MODEL("models/p_blockar2.mdl");
        PRECACHE_MODEL("models/v_blockchange.mdl");
        PRECACHE_SOUND("weapons/blockar2-1.wav");

#ifndef CLIENT_DLL
        UTIL_PrecacheOther("block_missile");
#endif
        m_iShellId = PRECACHE_MODEL("models/block_shell.mdl");
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

        if (m_iWeaponState != 0)
        {
            if (m_flRocketReloadTime == invalid_time_point && GetNumRockets() > 0)
                return DefaultDeploy("models/v_blockar2.mdl", "models/p_blockar2.mdl", BLOCKAR2_DRAW, "m249", FALSE, 1.0s, 1.0s);
            else
                return DefaultDeploy("models/v_blockar2.mdl", "models/p_blockar2.mdl", BLOCKAR2_DRAW_EMPTY, "m249", FALSE, 1.0s, 1.0s);
        }
        else
            return DefaultDeploy("models/v_blockar1.mdl", "models/p_blockar1.mdl", BLOCKAR1_DRAW, "rifle", FALSE, 1.0s, 1.03s);
    }


#ifndef CLIENT_DLL
    void BlockMissileFire(float flSpread, duration_t flCycleTime)
    {
        m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
        m_pPlayer->pev->effects &= ~EF_MUZZLEFLASH;

        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

        Vector vecSrc = m_pPlayer->GetGunPosition() - gpGlobals->v_up * 2;
        Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 0, BULLET_NONE, 0, 0.0, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

        CBlockMissile::Create(vecSrc, gpGlobals->v_forward + gpGlobals->v_right * vecDir.x + gpGlobals->v_up * vecDir.y, m_pPlayer);

        EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/blockar2-1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

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
            return "models/w_blockar1.mdl";

        case 1:
        case 3:
            return "models/w_blockar2.mdl";

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
            Base::PrimaryAttack();
        }
        else if (m_iWeaponState == 3)
        {
            if (GetNumRockets() > 0)
            {
                SendWeaponAnim(BLOCKAR2_SHOOT_START, UseDecrement());

                m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0s;

                pev->nextthink = gpGlobals->time + 0.96s;
                SetThink(&CBlockAR::ShootMissile);
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
                m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_blockar2.mdl");
                m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_blockar2.mdl");

                strcpy(m_pPlayer->m_szAnimExtention, "m249");

                if (GetNumRockets() > 0)
                    SendWeaponAnim(BLOCKAR2_CHANGE2);
                else
                    SendWeaponAnim(BLOCKAR2_CHANGE2_EMPTY);

                m_iWeaponState = 3;
                m_flTimeWeaponIdle2 = invalid_time_point;
                break;

            case 1:
                m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_blockar1.mdl");
                m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_blockar1.mdl");

                strcpy(m_pPlayer->m_szAnimExtention, "rifle");

                SendWeaponAnim(BLOCKAR1_CHANGE2);

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

        if (DefaultReload(MaxClip, BLOCKAR1_RELOAD, 3.05s))
        {
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
            m_flAccuracy = 0.1;
            m_iShotsFired = 0;
            m_bDelayFire = false;
        }
    }

    void RocketReload(void)
    {
        SendWeaponAnim(BLOCKAR2_RELOAD);

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
            return CBasePlayer::GetAmmoIndex("blockmissile");
        else if (m_iWeaponState == 0)
            return CBasePlayer::GetAmmoIndex("blockbullet");
        else
            return -1;
#else
        return -1;
#endif
    }

   /* int SecondaryAmmoIndex(void)
    {
#ifndef CLIENT_DLL
        if (m_iWeaponState == 3)
            return CBasePlayer::GetAmmoIndex("blockmissile");
        else
            return -1;
#else
        return -1;
#endif
    }*/

    void SecondaryAttack(void)
    {
        if (m_flNextSecondaryAttack > UTIL_WeaponTimeBase())
            return;

        StopIdleSound();

        if (!m_iWeaponState)
        {
            SendWeaponAnim(BLOCKAR1_CHANGE1, UseDecrement());

            PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usFire, 0.53, g_vecZero, g_vecZero, 0.0, 0.0, 0, 0, TRUE, FALSE);
        }
        else if (GetNumRockets() > 0)
        {
            SendWeaponAnim(BLOCKAR2_CHANGE1, UseDecrement());
        }
        else
        {
            SendWeaponAnim(BLOCKAR2_CHANGE1_EMPTY, UseDecrement());
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
            BlockMissileFire(0.035, 3.0s);
        else if (m_pPlayer->pev->velocity.Length2D() > 0)
            BlockMissileFire(0.02, 3.0s);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            BlockMissileFire(0.01, 3.0s);
        else
            BlockMissileFire(0.01, 3.0s);    
#endif
        m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;

        SendWeaponAnim(BLOCKAR2_SHOOT_END, UseDecrement());

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
        WRITE_STRING("weapons/blockar2_idle.wav");
        MESSAGE_END();
#endif
    }

    void WeaponIdle(void)
    {
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

        if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
            return;

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1000s;

        if (!m_iWeaponState)
        {
            SendWeaponAnim(BLOCKAR1_IDLE, UseDecrement() != FALSE);
        }
        else if (GetNumRockets() > 0)
        {
            SendWeaponAnim(BLOCKAR2_IDLE, UseDecrement());
        }
        else
        {
            SendWeaponAnim(BLOCKAR2_IDLE_EMPTY, UseDecrement());
        }
    }

};
LINK_ENTITY_TO_CLASS(weapon_blockar, CBlockAR)
}