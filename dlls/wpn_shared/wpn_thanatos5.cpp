/*
thanatos5.cpp
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
    class CThanatos5Rocket : public CGrenade
    {
    public:
        //float GetArmorRatioModifier() { return 1.0; }
        KnockbackData GetKnockBackData() override { return { 350.0f, 450.0f, 600.0f, 450.0f, 0.5f }; }
        int GetWeaponsId(void) { return WEAPON_THANATOS5; }

        static CThanatos5Rocket* Create(Vector vecSrc, Vector vecDir, Vector vecVeclocity, CBasePlayer* pPlayer, int iType)
        {
           
            CThanatos5Rocket* pGrenade = (CThanatos5Rocket*)CBaseEntity::Create("thanatos5_grenade", vecSrc, vecDir, pPlayer->edict());

            if (!pGrenade)
                return NULL;

            pGrenade->m_pOwner = pPlayer;
            pGrenade->m_iType = iType;
            if(!vecVeclocity.IsZero())
                pGrenade->pev->velocity = vecVeclocity;

            pGrenade->pev->dmg = pGrenade->GetExplodeDamage();
            pGrenade->m_iTeam = pPlayer->m_iTeam;

            return pGrenade;
        }

        void Spawn(void) override
        {
            Precache();

            pev->movetype = MOVETYPE_BOUNCE;
            pev->solid = SOLID_BBOX;

            SET_MODEL(ENT(pev), "models/thanatos5_bulleta.mdl");

            UTIL_SetSize(pev, g_vecZero, g_vecZero);
            UTIL_SetOrigin(pev, pev->origin);

            pev->frame = 0;
            pev->sequence = 0;

            pev->friction = 0.9;

            ResetSequenceInfo();

            SetThink(&CThanatos5Rocket::IgniteThink);
            
            pev->angles.x -= 30.0;
            UTIL_MakeVectors(pev->angles);
            pev->angles.x = -(pev->angles.x + 30.0);

            pev->velocity = gpGlobals->v_forward * 300;
            pev->gravity = 0.6;
            pev->nextthink = gpGlobals->time + 0.1s;
        }

        void Precache(void) override
        {
            PRECACHE_MODEL("models/thanatos5_bulleta.mdl");

            m_sModelIndexThanatos5Exp = PRECACHE_MODEL("sprites/thanatos5_explode.spr");
            m_sModelIndexThanatos5Exp2 = PRECACHE_MODEL("sprites/thanatos5_explode2.spr");


            m_iBeamModelIndex = PRECACHE_MODEL("sprites/smoke.spr");
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
            WRITE_BYTE(5);
            WRITE_BYTE(1);
            WRITE_BYTE(224);
            WRITE_BYTE(224);
            WRITE_BYTE(255);
            WRITE_BYTE(255);
            MESSAGE_END();

            SetThink(&CThanatos5Rocket::FollowThink);
            SetTouch(&CThanatos5Rocket::RocketTouch);

            m_flExplodeTime = gpGlobals->time + (m_iType ? 1.8s : 2.0s);
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

                SetTouch(NULL);
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

        void EXPORT RocketTouch(CBaseEntity* pOther)
        {
            if ((pOther->IsPlayer() || pOther->Classify() == CLASS_PLAYER_ALLY))
            {
                pev->velocity = pev->velocity * 0.5f;

                Explode();
                return;
            }
        }

        void Explode()
        {
            MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
            WRITE_BYTE(TE_KILLBEAM);
            WRITE_SHORT(this->entindex());
            MESSAGE_END();

            SetTouch(NULL);
            SetThink(NULL);

            Vector vecVelocity = pev->velocity;
            Vector vecOrigin = pev->origin;

            CBaseEntity* pEntity = NULL;
            while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecOrigin, m_iType ? 63.0f : 105.0f)) != NULL)
            {
                if (pEntity->IsDormant())
                    continue;

                if (pEntity->pev->takedamage == DAMAGE_NO)
                    continue;

                if (pEntity->IsPlayer() && UTIL_IsWallBetweenEntity(pEntity, this))
                    continue;

                pEntity->m_LastHitGroup = HITGROUP_CHEST;
                pEntity->TakeDamage(pev, m_pOwner->pev, GetExplodeDamage(), DMG_GENERIC | DMG_EXPLOSION);
            } 

            MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
            WRITE_BYTE(TE_EXPLOSION);
            WRITE_COORD(pev->origin.x);
            WRITE_COORD(pev->origin.y);
            WRITE_COORD(pev->origin.z);
            WRITE_SHORT(m_sModelIndexThanatos5Exp2);
            WRITE_BYTE(m_iType ? 5 : 10);
            WRITE_BYTE(30);
            WRITE_BYTE(TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOSOUND | TE_EXPLFLAG_NOPARTICLES);
            MESSAGE_END();

            MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
            WRITE_BYTE(TE_EXPLOSION);
            WRITE_COORD(pev->origin.x);
            WRITE_COORD(pev->origin.y);
            WRITE_COORD(pev->origin.z);
            WRITE_SHORT(m_sModelIndexThanatos5Exp);
            WRITE_BYTE(m_iType ? 5 : 10);
            WRITE_BYTE(30);
            WRITE_BYTE(TE_EXPLFLAG_NOSOUND | TE_EXPLFLAG_NOPARTICLES);
            MESSAGE_END();

            char buffer[64];
            int random_sound;
            random_sound = RANDOM_FLOAT(1, 3);
            sprintf(buffer, "weapons/thanatos5_explode%d.wav", random_sound);

            EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, buffer, VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

            if (m_iType < 2 && m_pOwner)
            {
                float flDispersion = m_iType ? 120.0 : 120.0;
                vecVelocity[2] += m_iType ? 100.0 : 100.0;
                //Upper right 
                vecVelocity[0] += flDispersion;
                vecVelocity[1] += flDispersion;
                CThanatos5Rocket::Create(vecOrigin, g_vecZero, vecVelocity, m_pOwner, m_iType + 1);
                //Upper left
                vecVelocity[0] -= flDispersion * 2;
                CThanatos5Rocket::Create(vecOrigin, g_vecZero, vecVelocity, m_pOwner, m_iType + 1);
                //Lower left
                vecVelocity[1] -= flDispersion * 2;
                CThanatos5Rocket::Create(vecOrigin, g_vecZero, vecVelocity, m_pOwner, m_iType + 1);
                //Lower right
                vecVelocity[0] += flDispersion * 2;
                CThanatos5Rocket::Create(vecOrigin, g_vecZero, vecVelocity, m_pOwner, m_iType + 1);
            }

            UTIL_Remove(this);
        }

    private:
        float GetExplodeDamage()
        {
            float flDamage = m_iType ? 8 : 13;
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flDamage = m_iType ? 260 : 500;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = m_iType ? 3000 : 1500;
#endif
            return flDamage;
        }

        CBasePlayer* m_pOwner;
        int m_iType;
        int m_iBeamModelIndex;
        int m_sModelIndexThanatos5Exp;
        int m_sModelIndexThanatos5Exp2;
        time_point_t m_flExplodeTime;
    };

    LINK_ENTITY_TO_CLASS(thanatos5_grenade, CThanatos5Rocket);
#endif


    class CThanatos5 : public LinkWeaponTemplate< CThanatos5,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_AUG>::template type,
            TPrecacheEvent,
            TGetDamageDefault
    >
    {
    public:

    static constexpr const char *V_Model = "models/v_thanatos5.mdl";
    static constexpr const char *P_Model = "models/p_thanatos5.mdl";
    static constexpr const char *W_Model = "models/w_thanatos5.mdl";
    static constexpr const char *EventFile = "events/thanatos5.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_thanatos5";
    static constexpr const char *AnimExtension = "mp5";

    static constexpr int MaxClip = 30;

	enum
	{
        ANIM_IDLEA = 0,
        ANIM_IDLEB,
        ANIM_SHOOTA1,
        ANIM_SHOOTA2,
        ANIM_SHOOTA3,
        ANIM_SHOOTB1_1,
        ANIM_SHOOTB1_2,
        ANIM_SHOOTB1_3,
        ANIM_SHOOTB2_1,
        ANIM_RELOADA,
        ANIM_RELOADB,
        ANIM_CHANGEA,
        ANIM_DRAWA,
        ANIM_DRAWB
	};
    static constexpr float PlayerMaxSpeed = 230;
    static constexpr float ArmorRatioModifier = 1.4f;
    static constexpr int DamageDefault = 32;
    static constexpr int DamageZB = 42;
    static constexpr int DamageZBS = 62;

    KnockbackData KnockBack = { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f };

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "556Nato";
        p->iMaxAmmo1 = 90;
        p->pszAmmo2 = "Thanatos5Grenade";
        p->iMaxAmmo2 = 1;
        p->iMaxClip = 30;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_THANATOS5;
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

    void Precache()
    {
        PRECACHE_MODEL("sprites/muzzleflash20.spr");
        PRECACHE_MODEL("sprites/muzzleflash21.spr");

        PRECACHE_SOUND("weapons/thanatos5_shootb2_1.wav");
        Base::Precache();
    }

    BOOL Deploy() override
    {
        m_flAccuracy = 0.2;
        m_iShotsFired = 0;
        pev->iuser1 = 0;

        return DefaultDeploy(V_Model, P_Model, m_iWeaponState & WPNSTATE_M4A1_SILENCED ? ANIM_DRAWB : ANIM_DRAWA, "mp5", UseDecrement() != FALSE, 0.75s, 2.0s);
    }

    void SecondaryAttack()
    {
        if (!m_iWeaponState)
        {
            pev->iuser1 = 1;
            SendWeaponAnim(ANIM_CHANGEA, UseDecrement() != FALSE);

            m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 5.14s;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.2s;
        }
        else
        {
            m_iWeaponState = 0;
            m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;
            EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/thanatos5_shootb2_1.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);
            SendWeaponAnim(ANIM_SHOOTB2_1, UseDecrement() != FALSE);

#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_ATTACK1);
            m_pPlayer->pev->effects &= ~EF_MUZZLEFLASH;

            m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
            m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

            UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

            Vector vecSrc = m_pPlayer->GetGunPosition() - gpGlobals->v_up * 3 + gpGlobals->v_right * 7;
            CThanatos5Rocket* pRocket = CThanatos5Rocket::Create(vecSrc, m_pPlayer->pev->v_angle, g_vecZero, m_pPlayer, 0);

            UTIL_MakeVectors(m_pPlayer->pev->v_angle);
            pRocket->pev->velocity += gpGlobals->v_forward * 600;
#endif
            m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.0s;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.1s;
        }

    }

    void PrimaryAttack(void)
    {
        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            Fire(m_flAccuracy * 0.4 + 0.035, 0.11s, FALSE);
        else if (m_pPlayer->pev->velocity.Length2D() > 140)
            Fire(m_flAccuracy * 0.07 + 0.035, 0.11s, FALSE);
        else
            Fire(m_flAccuracy * 0.02, 0.11s, FALSE);

    }

    void ItemPostFrame() override
    {
        if (m_flNextSecondaryAttack <= 0.0s)
        {
            if (pev->iuser1)
            {
                pev->iuser1 = 0;
                m_iWeaponState |= WPNSTATE_M4A1_SILENCED;

                m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 1;
            }
        }
        return CBasePlayerWeapon::ItemPostFrame();
    }

    void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_bDelayFire = true;
        m_iShotsFired++;

        m_flAccuracy = ((m_iShotsFired * m_iShotsFired) / 220.0) + 0.3;
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
            KickBack(0.475, 1.2, 0.29, 0.15, 5.75, 3.55, 5);
        else if (m_pPlayer->pev->velocity.Length2D() > 0)
            KickBack(1.0, 0.4, 0.285, 0.045, 4.0, 3.25, 6);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            KickBack(0.65, 0.285, 0.225, 0.0125, 3.25, 2.0, 7);
        else
            KickBack(0.7, 0.325, 0.275, 0.015, 3.75, 2.5, 6);
    }

    void Reload(void) override
    {
        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            return;

        if (DefaultReload(30, m_iWeaponState & WPNSTATE_M4A1_SILENCED ? ANIM_RELOADB : ANIM_RELOADA, 3.25s))
        {
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

        if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
            SendWeaponAnim(ANIM_IDLEB, UseDecrement() != FALSE);
        else
            SendWeaponAnim(ANIM_IDLEA, UseDecrement() != FALSE);

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
    }


};
LINK_ENTITY_TO_CLASS(weapon_thanatos5, CThanatos5)
}