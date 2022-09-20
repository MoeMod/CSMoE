/*
blockmg.cpp
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
    class CBlockMGMissile : public CGrenade
    {
    public:
        //float GetArmorRatioModifier() { return 1.0; }
        //KnockbackData GetKnockBackData() override { return { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }; }
        int GetWeaponsId(void) { return WEAPON_BLOCKMG; }

        static CBlockMGMissile* Create(Vector vecSrc, Vector vecDir, CBasePlayer* pPlayer)
        {
            CBlockMGMissile* pMissile = (CBlockMGMissile*)CBaseEntity::Create("blockmg_missile", vecSrc, UTIL_VecToAngles(vecDir), pPlayer->edict());

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

            pMissile->pev->velocity = vecDir * 1650;
            pMissile->pev->dmg = pMissile->GetExplodeDamage();
            pMissile->m_iTeam = pPlayer->m_iTeam;

            return pMissile;
        }

        void Spawn(void) override
        {
            Precache();

            pev->movetype = MOVETYPE_FLY;
            pev->solid = SOLID_BBOX;

            SET_MODEL(ENT(pev), "models/blockmg_missile.mdl");

            UTIL_SetSize(pev, g_vecZero, g_vecZero);
            UTIL_SetOrigin(pev, pev->origin);

            SetThink(&CBlockMGMissile::IgniteThink);
            SetTouch(&CBlockMGMissile::RocketTouch);

            pev->nextthink = gpGlobals->time + 0.1s;
        }
        void Precache(void) override
        {
            PRECACHE_MODEL("models/blockmg_missile.mdl");

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
            WRITE_BYTE(15);
            WRITE_BYTE(5);
            WRITE_BYTE(224);
            WRITE_BYTE(224);
            WRITE_BYTE(255);
            WRITE_BYTE(200);
            MESSAGE_END();

            SetThink(&CBlockMGMissile::FollowThink);
 
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
            MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
            WRITE_BYTE(TE_KILLBEAM);
            WRITE_SHORT(this->entindex());
            MESSAGE_END();

            CBasePlayer* pOwner = (CBasePlayer*)CBaseEntity::Instance(pev->owner);

            if (pOther && pOwner && pOwner->IsAlive() && !pOther->IsDormant() && pOther->pev->takedamage != DAMAGE_NO)
            {
                TraceResult tr = UTIL_GetGlobalTrace();
                entvars_t* pevOwner = VARS(pev->owner);

                ClearMultiDamage();

                pOther->TraceAttack(pevOwner, GetExplodeDamage(), pev->velocity.Normalize(), &tr, DMG_BULLET | DMG_NEVERGIB);

                ApplyMultiDamage(pev, pevOwner);
            }

            if (g_pModRunning->DamageTrack() == DT_ZBS)
                ExplodeTouch3(pOther, 4.0 * 39.37, CGrenade::EXPTYPE_AT4ROCKET, false);
            else
            {
                MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
                WRITE_BYTE(TE_EXPLOSION);	// This makes a dynamic light and the explosion sprites/sound
                WRITE_COORD(pev->origin.x);		// Send to PAS because of the sound
                WRITE_COORD(pev->origin.y);
                WRITE_COORD(pev->origin.z + 20.0f);
                WRITE_SHORT(g_sModelIndexFireball3);
                WRITE_BYTE(25);			// scale * 10
                WRITE_BYTE(30);		// framerate
                WRITE_BYTE(TE_EXPLFLAG_NONE);	// flags
                MESSAGE_END();

                MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
                WRITE_BYTE(TE_EXPLOSION);	// This makes a dynamic light and the explosion sprites/sound
                WRITE_COORD(pev->origin.x + RANDOM_FLOAT(-64, 64));	// Send to PAS because of the sound
                WRITE_COORD(pev->origin.y + RANDOM_FLOAT(-64, 64));
                WRITE_COORD(pev->origin.z + RANDOM_FLOAT(30, 35));
                WRITE_SHORT(g_sModelIndexFireball2);
                WRITE_BYTE(30);			// scale * 10
                WRITE_BYTE(30);		// framerate
                WRITE_BYTE(TE_EXPLFLAG_NONE);	// flags
                MESSAGE_END();

                pev->effects |= EF_NODRAW;
                SetTouch(NULL);

                SetThink(&CGrenade::Smoke3_C);
                pev->velocity = g_vecZero;
                pev->nextthink = gpGlobals->time + 0.55s;
            }
        }

    private:
        float GetExplodeDamage()
        {
            float flDamage = 125;
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flDamage = 1250;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = 1250;
#endif
            return flDamage;
        }

        int m_iBeamModelIndex;
        int m_iShellModelIndex;
    };

    LINK_ENTITY_TO_CLASS(blockmg_missile, CBlockMGMissile);
#endif

    class CBlockMG : public LinkWeaponTemplate< CBlockMG,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_M249>::template type,
            TPrecacheEvent,
            TGetDamageDefault,
            TPrimaryAttackRifle,
            TFireRifle,
            TRecoilKickBack
    >
    {
    public:
    static constexpr const char *V_Model = "models/v_blockmg1.mdl";
    static constexpr const char *P_Model = "models/p_blockmg1.mdl";
    static constexpr const char *W_Model = "models/w_blockmg1.mdl";
    static constexpr const char *EventFile = "events/blockmg.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_blockmg";
    static constexpr const char *AnimExtension = "m249";
    static constexpr int MaxClip = 150;

    enum blockmg1_e
    {
        BLOCKMG1_IDLE,
        BLOCKMG1_SHOOT1,
        BLOCKMG1_SHOOT2,
        BLOCKMG1_RELOAD,
        BLOCKMG1_DRAW,
        BLOCKMG1_CHANGE1,
        BLOCKMG1_CHANGE2,    
    };

    enum blockchange_e
    {
        BLOCKCHANGE_CHANGE
    };

    enum blockmg2_e
    {
        BLOCKMG2_IDLE,
        BLOCKMG2_SHOOT1,
        BLOCKMG2_DRAW,
        BLOCKMG2_CHANGE1,
        BLOCKMG2_CHANGE2,
        BLOCKMG2_IDLE2,
        BLOCKMG2_SHOOT_EMPTY,
    };

    static constexpr float PlayerMaxSpeed = 220;
    static constexpr float ArmorRatioModifier = 1.5f;
	static constexpr float PrimaryAttackWalkingMiniumSpeed = 140;
    static constexpr const auto &SpreadCalcNotOnGround = 0.4*A+0.055;
    static constexpr const auto &SpreadCalcWalking = 0.07*A+0.035;
    static constexpr const auto &SpreadCalcDefault = 0.03*A;

	static constexpr const auto& CycleTime = 0.098s;
    static constexpr int DamageDefault = 34;
    static constexpr int DamageZB = 70;
    static constexpr int DamageZBS = 75;

    static constexpr const auto& AccuracyCalc = (N * N * N / 220.0) + 0.3;
    static constexpr float AccuracyDefault = 0.2;
    static constexpr float AccuracyMax = 0.8;
    static constexpr float RangeModifier = 0.98;

    static constexpr auto BulletType = BULLET_PLAYER_762MM;
    /*static constexpr*/ int Penetration = 2;
    KnockbackData KnockBack = { 350.0f, 450.0f, 400.0f, 170.0f, 0.6f };

    KickBackData KickBackWalking = { 1.1, 0.46, 0.28, 0.045, 3.75, 3.0, 8 };
    KickBackData KickBackNotOnGround = { 1.2, 0.5, 0.23, 0.15, 5.5, 3.5, 5 };
    KickBackData KickBackDucking = { 0.62, 0.32, 0.2, 0.0125, 3.25, 2.0, 7 };
    KickBackData KickBackDefault = { 0.66, 0.36, 0.25, 0.015, 3.5, 2.25, 7 };
    
    time_point_t m_flRocketReloadTime;
    time_point_t m_flTimeWeaponIdle2;
    duration_t m_flTimeWeaponIdle3;
    int m_iDefaultAmmo2;

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "blockbullet";
        p->iMaxAmmo1 = 200;
        p->pszAmmo2 = "blockmgmissile";
        p->iMaxAmmo2 = 10;
        p->iMaxClip = 150;
        p->iSlot = 0;
        p->iPosition = 1;
        p->iId = m_iId = WEAPON_BLOCKMG;
        p->iFlags = 0;
        p->iWeight = M249_WEIGHT;

        return 1;
    }

    float GetArmorRatioModifier() override { return 1.5; }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override 
    {
        if (m_iWeaponState == 3)
            return { "ammo_blockmgmissile" , 200 };
        else
            return { "ammo_blockbullet" , 60 };   
    }
#endif
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 6, 3, 140 }; }

    void Spawn()
    {
        m_flRocketReloadTime = invalid_time_point;
        m_iWeaponState = 0;
        m_flTimeWeaponIdle2 = invalid_time_point;
        m_iDefaultAmmo2 = 10;
        m_flTimeWeaponIdle3 = 0s;

        Base::Spawn();
    }

    void Precache(void)
    {
        Base::Precache();

        PRECACHE_MODEL("models/w_blockmg2.mdl");
        PRECACHE_MODEL("models/v_blockmg2.mdl");
        PRECACHE_MODEL("models/p_blockmg2.mdl");
        PRECACHE_MODEL("models/v_blockchange.mdl");
        PRECACHE_SOUND("weapons/blockmg2-1.wav");
        PRECACHE_SOUND("weapons/blockmg2_shoot_empty.wav");

        PRECACHE_MODEL("sprites/muzzleflash36.spr");

#ifndef CLIENT_DLL
        UTIL_PrecacheOther("blockmg_missile");
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

        m_iShotsFired = 0;
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;
        m_flAccuracy = 0.1;

        m_flTimeWeaponIdle2 = invalid_time_point;
        m_flTimeWeaponIdle3 = 0s;

        if (m_iWeaponState == 1)
            m_iWeaponState = 3;
        else if (m_iWeaponState == 2)
            m_iWeaponState = 0;

        if (m_iWeaponState != 0)
        {
            return DefaultDeploy("models/v_blockmg2.mdl", "models/p_blockmg2.mdl", BLOCKMG2_DRAW, "m249", FALSE, 1.0s, 1.0s);
        }
        else
            return DefaultDeploy("models/v_blockmg1.mdl", "models/p_blockmg1.mdl", BLOCKMG1_DRAW, "m249", FALSE, 1.0s, 1.03s);
    }


#ifndef CLIENT_DLL
    void BlockMGMissileFire(float flSpread, duration_t flCycleTime)
    {
        m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
        m_pPlayer->pev->effects &= ~EF_MUZZLEFLASH;

        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

        Vector vecSrc = m_pPlayer->GetGunPosition() - gpGlobals->v_up * 2;
        Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 0, BULLET_NONE, 0, 0.0, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

        CBlockMGMissile::Create(vecSrc, gpGlobals->v_forward + gpGlobals->v_right * vecDir.x + gpGlobals->v_up * vecDir.y, m_pPlayer);

        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            KickBack(5.0, 3.2, 1.5, 0.5, 7.0, 5.7, 2);
        else if (m_pPlayer->pev->velocity.Length2D() > 0)
            KickBack(5.0, 2.25, 1.45, 0.5, 7.0, 10.0, 1);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            KickBack(9.0, 2.1, 0.5, 0.2, 15.0, 5.0, 1);
        else
            KickBack(5.0, 3.2, 1.35, 0.5, 7.0, 10.0, 2);
    }
#endif

    const char* GetWorldModel(void)
    {
        switch (m_iWeaponState)
        {
        case 0:
        case 2:
            return "models/w_blockmg1.mdl";

        case 1:
        case 3:
            return "models/w_blockmg2.mdl";

        default:
            return NULL;
        }
    }

    void Holster(int skiplocal)
    {
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
            if (GetNumRockets() > 0 && m_flRocketReloadTime < gpGlobals->time)
            {
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 4.0s;
                m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 3.0s;
                m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;

                SendWeaponAnim(m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] ? BLOCKMG2_SHOOT1 : BLOCKMG2_SHOOT_EMPTY, UseDecrement());
                m_flTimeWeaponIdle3 = 0s;

                if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType])
                    EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/blockmg2-1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
                else
                    EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/blockmg2_shoot_empty.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

                ShootMissile();
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
                m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_blockmg2.mdl");
                m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_blockmg2.mdl");

                strcpy(m_pPlayer->m_szAnimExtention, "m249");

                SendWeaponAnim(BLOCKMG2_CHANGE2);
     
                m_iWeaponState = 3;
                m_flTimeWeaponIdle2 = invalid_time_point;
                break;

            case 1:
                m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_blockmg1.mdl");
                m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_blockmg1.mdl");

                strcpy(m_pPlayer->m_szAnimExtention, "m249");

                SendWeaponAnim(BLOCKMG1_CHANGE2);

                m_iWeaponState = 0;
                m_flTimeWeaponIdle2 = invalid_time_point;
                break;
            }
        }
    }

    void Reload(void)
    {
        if (m_iWeaponState)
            return;

        if (DefaultReload(MaxClip, BLOCKMG1_RELOAD, 4.7s))
        {
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
            m_flAccuracy = 0.1;
            m_iShotsFired = 0;
            m_bDelayFire = false;
        }
    }

    int GetNumRockets(void)
    {
        return m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType];
    }

    int PrimaryAmmoIndex(void)
    {
#ifndef CLIENT_DLL
        if (m_iWeaponState == 3)
            return CBasePlayer::GetAmmoIndex("blockmgmissile");
        else if (m_iWeaponState == 0)
            return CBasePlayer::GetAmmoIndex("blockbullet");
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
            SendWeaponAnim(BLOCKMG1_CHANGE1, UseDecrement());

            PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usFire, 0.53, g_vecZero, g_vecZero, 0.0, 0.0, 0, 0, TRUE, FALSE);
        }
        else
        {
            m_flTimeWeaponIdle3 = 0s;
            SendWeaponAnim(BLOCKMG2_CHANGE1, UseDecrement());
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
            BlockMGMissileFire(0.035, 4.0s);
        else if (m_pPlayer->pev->velocity.Length2D() > 0)
            BlockMGMissileFire(0.02, 4.0s);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            BlockMGMissileFire(0.01, 4.0s);
        else
            BlockMGMissileFire(0.01, 4.0s);
#endif
        m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 4.0s;
        m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;
        m_flRocketReloadTime = gpGlobals->time + 4.0s;

        return;
    }

    void StopIdleSound(void)
    {
#ifndef CLIENT_DLL
        MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
        WRITE_BYTE(79);
        WRITE_SHORT(m_pPlayer->entindex());
        WRITE_SHORT(CHAN_AUTO);
        WRITE_STRING("weapons/blockmg2_idle.wav");
        MESSAGE_END();
#endif
    }

    void WeaponIdle(void)
    {
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

        if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
            return;

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;

        if (!m_iWeaponState)
        {
            SendWeaponAnim(BLOCKMG1_IDLE, UseDecrement() != FALSE);
        }
        else
        {
            if (m_flTimeWeaponIdle3 < 12s)
            {
                m_flTimeWeaponIdle3 = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 6s;
                SendWeaponAnim(BLOCKMG2_IDLE, UseDecrement() != FALSE);
            }
            else
            {
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 6s;
                SendWeaponAnim(BLOCKMG2_IDLE2, UseDecrement() != FALSE);
                m_flTimeWeaponIdle3 = 0s;
            }
        }
    }

};
LINK_ENTITY_TO_CLASS(weapon_blockmg, CBlockMG)
}