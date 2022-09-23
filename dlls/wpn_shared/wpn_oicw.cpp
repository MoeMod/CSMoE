/*
oicw.cpp
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
    class COICWRocket : public CGrenade
    {
    public:
        //float GetArmorRatioModifier() { return 1.0; }
        KnockbackData GetKnockBackData() override { return { 350.0f, 450.0f, 600.0f, 450.0f, 0.65f }; }
        int GetWeaponsId(void) { return WEAPON_OICW; }

        static COICWRocket* Create(Vector vecSrc, Vector vecDir, CBasePlayer* pPlayer)
        {
           
            COICWRocket* pGrenade = (COICWRocket*)CBaseEntity::Create("oicw_grenade", vecSrc, vecDir, pPlayer->edict());

            if (!pGrenade)
                return NULL;

            pGrenade->pev->dmg = pGrenade->GetExplodeDamage();
            pGrenade->m_iTeam = pPlayer->m_iTeam;

            return pGrenade;
        }

        void Spawn(void) override
        {
            Precache();

            pev->movetype = MOVETYPE_BOUNCE;
            pev->solid = SOLID_BBOX;

            SET_MODEL(ENT(pev), "models/s_oicw.mdl");

            UTIL_SetSize(pev, g_vecZero, g_vecZero);
            UTIL_SetOrigin(pev, pev->origin);

            pev->frame = 0;
            pev->sequence = 0;

            pev->friction = 0.9;

            ResetSequenceInfo();

            SetThink(&COICWRocket::IgniteThink);
            
            pev->angles.x -= 30.0;
            UTIL_MakeVectors(pev->angles);
            pev->angles.x = -(pev->angles.x + 30.0);

            pev->velocity = gpGlobals->v_forward * 380;
            pev->gravity = 0.5;
            pev->nextthink = gpGlobals->time + 0.1s;
        }

        void Precache(void) override
        {
            PRECACHE_MODEL("models/s_oicw.mdl");
        }

    public:
        void EXPORT IgniteThink()
        {
            if (!IsInWorld())
            {
                SetThink(NULL);

                UTIL_Remove(this);
                return;
            }

            MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
            WRITE_BYTE(TE_BEAMFOLLOW);
            WRITE_SHORT(entindex());
            WRITE_SHORT(g_sModelIndexSmokeBeam);
            WRITE_BYTE(5);
            WRITE_BYTE(1);
            WRITE_BYTE(224);
            WRITE_BYTE(224);
            WRITE_BYTE(255);
            WRITE_BYTE(255);
            MESSAGE_END();

            SetThink(&COICWRocket::FollowThink);

            m_flExplodeTime = gpGlobals->time + 2.5s;
            pev->nextthink = gpGlobals->time + 0.1s;
        }

        void EXPORT FollowThink(void)
        {
            pev->nextthink = gpGlobals->time + 0.1s;

            if (!IsInWorld())
            {
                MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
                WRITE_BYTE(TE_KILLBEAM);
                WRITE_SHORT(this->entindex());
                MESSAGE_END();

                SetThink(NULL);

                UTIL_Remove(this);
                return;
            }

            if (pev->flags & FL_ONGROUND)
            {
                pev->velocity = pev->velocity * 0.2f;
            }

            if (m_flExplodeTime < gpGlobals->time)
            {
                Explode();
            }      
        }

        void Explode()
        {
            MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
            WRITE_BYTE(TE_KILLBEAM);
            WRITE_SHORT(this->entindex());
            MESSAGE_END();

            SetThink(NULL);
          
            ExplodeTouch3(NULL, 5 * 39.37, CGrenade::EXPTYPE_OICWROCKET, true);
        }

    private:
        float GetExplodeDamage()
        {
            float flDamage = 110;
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flDamage = 1350;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = 1800;
#endif
            return flDamage;
        }

        time_point_t m_flExplodeTime;
    };

    LINK_ENTITY_TO_CLASS(oicw_grenade, COICWRocket);
#endif


    class COICW : public LinkWeaponTemplate< COICW,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_AUG>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TReloadDefault,
            TGetDamageDefault
    >
    {
    public:

    static constexpr const auto& DefaultReloadTime = 3.9s;
    static constexpr const char *V_Model = "models/v_oicw.mdl";
    static constexpr const char *P_Model = "models/p_oicw.mdl";
    static constexpr const char *W_Model = "models/w_oicw.mdl";
    static constexpr const char *EventFile = "events/oicw.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_oicw";
    static constexpr const char *AnimExtension = "mp5";

    static constexpr int MaxClip = 30;

	enum
	{
        ANIM_IDLE1 = 0,
        ANIM_SHOOT1,
        ANIM_SHOOT2,
        ANIM_SHOOT3,
        ANIM_RELOAD,
        ANIM_DRAW,
        ANIM_GRENADE_IDLE,
        ANIM_GRENADE_SHOOT1,
        ANIM_GRENADE_SHOOT2,
        ANIM_MOVE_GRENADE,
        ANIM_MOVE_CARBINE,
	};
    static constexpr float PlayerMaxSpeed = 230;
    static constexpr float ArmorRatioModifier = 1.4f;
    static constexpr int DamageDefault = 36;
    static constexpr int DamageZB = 65;
    static constexpr int DamageZBS = 85;

    KnockbackData KnockBack = { 550.0f, 350.0f, 400.0f, 200.0f, 0.5f };

    int m_iDefaultAmmo2;

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "556Nato";
        p->iMaxAmmo1 = 90;
        p->pszAmmo2 = "OICWGrenade";
        p->iMaxAmmo2 = 6;
        p->iMaxClip = 30;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_OICW;
        p->iFlags = 0;
        p->iWeight = MP5NAVY_WEIGHT;

        return 1;
    }
    
	float GetMaxSpeed(void)
	{
		return 230;
	}

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_AUG); }
#endif
    float GetArmorRatioModifier() override { return 1.4; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 5, 3, 130 }; }

    void Spawn()
    {
        m_iDefaultAmmo2 = 6;
        Base::Spawn();
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

    void Precache()
    {
        PRECACHE_SOUND("weapons/oicw_grenade-1.wav");
        PRECACHE_SOUND("weapons/oicw_grenade-2.wav");
        PRECACHE_MODEL("sprites/muzzleflash12.spr");

        Base::Precache();
    }

    BOOL Deploy(void) override
    {
        m_iWeaponState = 0;
#ifndef CLIENT_DLL
        m_pPlayer->UpdateShieldCrosshair((m_iWeaponState & WPNSTATE_M4A1_SILENCED) == 0);
#endif
        return Base::Deploy();
    }

    void SecondaryAttack()
    {
        if (!m_iWeaponState)
        {
            m_iWeaponState |= WPNSTATE_M4A1_SILENCED;
            SendWeaponAnim(ANIM_MOVE_GRENADE, UseDecrement() != FALSE);
        }
        else
        {
            m_iWeaponState = 0;
            SendWeaponAnim(ANIM_MOVE_CARBINE, UseDecrement() != FALSE);
        }

#ifndef CLIENT_DLL
        m_pPlayer->UpdateShieldCrosshair((m_iWeaponState & WPNSTATE_M4A1_SILENCED) == 0);
#endif
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.3s;
        m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.05s;
        m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.05s;
    }

    void PrimaryAttack(void)
    {
        if (m_iWeaponState)
        {
            if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] <= 0)
            {
                PlayEmptySound();

                m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.4s;
                return;
            }

            m_pPlayer->pev->effects &= ~EF_MUZZLEFLASH;

            m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
            m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

            m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;

            if (!m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType])
            {
                EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/oicw_grenade-2.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);

                SendWeaponAnim(ANIM_GRENADE_SHOOT2, UseDecrement() != FALSE);
            }
            else
            {

                EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/oicw_grenade-1.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);

                SendWeaponAnim(ANIM_GRENADE_SHOOT1, UseDecrement() != FALSE);
            }

            m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 3.0s;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0s;

#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_ATTACK1);
     
            UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

            Vector vecSrc = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 28 - gpGlobals->v_up * 7+ gpGlobals->v_right * 6;
            COICWRocket* pRocket = COICWRocket::Create(vecSrc, m_pPlayer->pev->v_angle, m_pPlayer);

            UTIL_MakeVectors(m_pPlayer->pev->v_angle);
            pRocket->pev->velocity += gpGlobals->v_forward * 800;
#endif
        }
        else
        {
            if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
                Fire(m_flAccuracy * 0.4 + 0.035, 0.091s, FALSE);
            else if (m_pPlayer->pev->velocity.Length2D() > 140)
                Fire(m_flAccuracy * 0.07 + 0.035, 0.091s, FALSE);
            else
                Fire(m_flAccuracy * 0.02, 0.091s, FALSE);
        }
    }

    void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_bDelayFire = true;
        m_iShotsFired++;

        m_flAccuracy = ((m_iShotsFired * m_iShotsFired) / 220.0) + 0.3;
        if (m_flAccuracy > 1.0)
            m_flAccuracy = 1.0;
              
 
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

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;

        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_556MM, GetDamage(), 0.97, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);
       
        int flags;
#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), (m_iWeaponState & WPNSTATE_M4A1_SILENCED) ? true : false, FALSE);
       
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;

        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            KickBack(0.535, 0.375, 0.255, 0.15, 4.25, 2.75, 3);
        else if (m_pPlayer->pev->velocity.Length2D() > 0)
            KickBack(0.475, 0.355, 0.165, 0.115, 3.75, 2.15, 10);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            KickBack(0.415, 0.315, 0.105, 0.035, 3.25, 1.85, 15);
        else
            KickBack(0.455, 0.345, 0.145, 0.095, 3.65, 2.05, 12);
    }

    void Reload(void) override
    {
        if (m_iWeaponState)
            return;

        Base::Reload();
    }

    void WeaponIdle(void) override
    {
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

        if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
            return;

        if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
            SendWeaponAnim(ANIM_GRENADE_IDLE, UseDecrement() != FALSE);
        else
            SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
    }

};
LINK_ENTITY_TO_CLASS(weapon_oicw, COICW)
}