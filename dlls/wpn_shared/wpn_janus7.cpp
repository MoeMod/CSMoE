/*
janus7.cpp
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
    class CJANUS7 : public LinkWeaponTemplate< CJANUS7,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_M249>::template type,
            TPrecacheEvent,
            TGetDamageDefault
    >
    {
    public:

    static constexpr const char *V_Model = "models/v_janus7.mdl";
    static constexpr const char *P_Model = "models/p_janus7.mdl";
    static constexpr const char *W_Model = "models/w_janus7.mdl";
    static constexpr const char *EventFile = "events/janus7.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_janus7";
    static constexpr const char *AnimExtension = "m249";
    static constexpr int MaxClip = 200;
	enum
	{
        ANIM_IDLE1 = 0,
        ANIM_RELOAD1,
        ANIM_DRAW1,
        ANIM_SHOOT1_1,
        ANIM_SHOOT1_2,
        ANIM_SHOOT1_SIGNAL,
        ANIM_CHANGE1,
        ANIM_IDLE2,
        ANIM_DRAW2,
        ANIM_SHOOT2_1,
        ANIM_SHOOT2_2,
        ANIM_CHANGE2,
        ANIM_SIGNAL,
        ANIM_RELOAD_SIGNAL,
        ANIM_DRAW_SIGNAL,
	};
    static constexpr float PlayerMaxSpeed = 230;

    static constexpr int DamageDefault = 33;
    static constexpr int DamageZB = 34;
    static constexpr int DamageZBS = 53;

    KnockbackData KnockBack = { 370.0f, 270.0f, 320.0f, 120.0f, 0.6f };

    time_point_t m_flNextReset;
    int m_iShotsRequired;
    int m_iBeamIndex;
    bool m_bHit;

#define WPNSTATE_JANUS7_SIGNAL 1
#define WPNSTATE_JANUS7_USING 2


    void Spawn() override
    {
        m_flNextReset = invalid_time_point;
        m_iShotsRequired = GetShotsRequired();
        Base::Spawn();
    }

    int GetShotsRequired()
    {
        int iShotsRequired = 100;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            iShotsRequired = 110;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            iShotsRequired = 120;
#endif
        return iShotsRequired;
    }

    void Precache()
    {
        m_iBeamIndex = PRECACHE_MODEL("sprites/lgtning.spr");

        PRECACHE_SOUND("weapons/janus7-2.wav");

        PRECACHE_MODEL("sprites/muzzleflash2.spr");
        PRECACHE_MODEL("sprites/muzzleflash5.spr");
        PRECACHE_SOUND("weapons/change1_ready.wav");
        Base::Precache();
    }

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "762NatoBox";
        p->iMaxAmmo1 = 200;
        p->pszAmmo2 = "InfiniteAmmo";
        p->iMaxAmmo2 = 255;
        p->iMaxClip = 200;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_JANUS7;
        p->iFlags = 0;
        p->iWeight = M249_WEIGHT;

        return 1;
    }

    int ExtractAmmo(CBasePlayerWeapon* pWeapon)
    {
        m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 0;
        return CBasePlayerWeapon::ExtractAmmo(pWeapon);
    }


#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_762natobox" , 60 }; }
#endif
    float GetArmorRatioModifier() override { return 1.5; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 6, 3, 140 }; }

    BOOL Deploy() override
    {
        m_flAccuracy = 0.22;
        m_iShotsFired = 0;

        int iAnim;
        if (m_iWeaponState == WPNSTATE_JANUS7_USING)
            iAnim = ANIM_DRAW2;
        else if (m_iWeaponState == WPNSTATE_JANUS7_SIGNAL)
            iAnim = ANIM_DRAW_SIGNAL;
        else
            iAnim = ANIM_DRAW1;

        return DefaultDeploy("models/v_janus7.mdl", "models/p_janus7.mdl", iAnim, "m249", UseDecrement() != FALSE);
    }

    void PrimaryAttack() override
    {
        if (m_iWeaponState == WPNSTATE_JANUS7_USING)
            Fire(0.0715, 0.072s, FALSE);
        else
        {
            if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
                Fire(m_flAccuracy * 0.2 + 0.05, 0.095s, FALSE);
            else if (m_pPlayer->pev->velocity.Length2D() > 140)
                Fire(m_flAccuracy * 0.03 + 0.035, 0.095s, FALSE);
            else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
                Fire(m_flAccuracy * 0.02 + 0.01, 0.095s, FALSE);
            else
                Fire(m_flAccuracy * 0.0 + 0.021, 0.095s, FALSE);
        }
        
    }

    void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_bDelayFire = true;
        m_iShotsFired++;
        m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 190) + 0.3;

        if (m_flAccuracy > 0.9)
            m_flAccuracy = 0.9;

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


        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecDir;

        m_bHit = m_iWeaponState == WPNSTATE_JANUS7_USING ? true : false;

        if (m_iWeaponState == WPNSTATE_JANUS7_USING)
        {
#ifndef CLIENT_DLL
            Janus7Lighting();
#endif
            return;
        }
        else
        {
            vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_762MM, GetDamage(), 0.97, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);
           
        }
        m_iClip--;

#ifndef CLIENT_DLL
        PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, static_cast<int>(m_pPlayer->pev->punchangle.x * 100), static_cast<int>(m_pPlayer->pev->punchangle.y * 100), m_iWeaponState, FALSE);
#endif
        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

        if (!m_iWeaponState)
        {
            if (pev->iuser1 >= m_iShotsRequired)
            {
                m_iWeaponState = WPNSTATE_JANUS7_SIGNAL;
                m_flNextReset = gpGlobals->time + 9.0s;
                EMIT_SOUND_DYN(edict(), CHAN_ITEM, "weapons/change1_ready.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);
                pev->iuser1 = 0;
            }
        }

#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.03s;

        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            KickBack(1.5, 0.55, 0.3, 0.3, 6, 5.0, 5);
        else if (m_pPlayer->pev->velocity.Length2D() > 0)
            KickBack(1.1, 0.3, 0.2, 0.06, 4.0, 2.5, 8);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            KickBack(0.75, 0.1, 0.1, 0.018, 3.5, 1.2, 9);
        else
            KickBack(0.8, 0.2, 0.18, 0.02, 3.2, 2.25, 7);
        
    }


#ifndef CLIENT_DLL
    void FireBullet3CallBack(int iPenetration, Vector vecDir, TraceResult* pTrace)
    {
        if (!m_bHit)
        {
            if (CBaseEntity* pEntity = gMultiDamage.pEntity)
            {
                if (pEntity->IsPlayer() || pEntity->Classify() == CLASS_PLAYER_ALLY)
                {
                    if (g_pGameRules->PlayerRelationship(m_pPlayer, pEntity) != GR_TEAMMATE)
                    {
                        pev->iuser1++;

                        MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
                        WRITE_BYTE(1);
                        WRITE_BYTE(pev->iuser1);
                        MESSAGE_END();

                        m_bHit = true;
                    }
                }
            }
        }

    }
#endif

    void Janus7Lighting(void)
    {
        SendWeaponAnim(RANDOM_FLOAT(0, 1) ? ANIM_SHOOT2_1 : ANIM_SHOOT2_2, UseDecrement() != FALSE);

        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

        EMIT_SOUND_DYN(this->edict(), CHAN_WEAPON, "weapons/janus7-2.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

        Vector vecSrc = m_pPlayer->GetGunPosition();

        CBaseEntity* pEntity = NULL;
        while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, 400.0)) != nullptr)
        {
            if (IsTargetAvailable(m_pPlayer, pEntity, vecSrc))
                break;
        }
        Vector VecEnd;
        if (pEntity != nullptr && pEntity->IsAlive())
        {
            VecEnd = pEntity->pev->origin;
            entvars_t* const pevAttacker = VARS(pev->owner);
            entvars_t* const pevInflictor = this->pev;
            pEntity->m_LastHitGroup = HITGROUP_CHEST;
            pEntity->TakeDamage(pevInflictor, pevAttacker, GetDamageB(), DMG_BULLET);
        }
        else
        {
            Vector VecForward;
            VecForward = gpGlobals->v_forward;
            VecForward *= 400;
            VecEnd = vecSrc + VecForward;
        }

        MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
        WRITE_BYTE(TE_BEAMENTPOINT);
        WRITE_SHORT(ENTINDEX(m_pPlayer->edict()) | 0x1000);
        WRITE_COORD(VecEnd.x);
        WRITE_COORD(VecEnd.y);
        WRITE_COORD(VecEnd.z);
        WRITE_SHORT(m_iBeamIndex);
        WRITE_BYTE(0); // FRAMERATE
        WRITE_BYTE(0); // FRAMERATE
        WRITE_BYTE(1); // LIFE
        WRITE_BYTE(40); // WIDTH
        WRITE_BYTE(35);   // NOISE
        WRITE_BYTE(255);   // R, G, B
        WRITE_BYTE(255);  // R, G, B
        WRITE_BYTE(0);   // R, G, B
        WRITE_BYTE(255);	// BRIGHTNESS
        WRITE_BYTE(25);		// SPEED
        MESSAGE_END();

        MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
        WRITE_BYTE(TE_BEAMENTPOINT);
        WRITE_SHORT(ENTINDEX(m_pPlayer->edict()) | 0x1000);
        WRITE_COORD(VecEnd.x);
        WRITE_COORD(VecEnd.y);
        WRITE_COORD(VecEnd.z);
        WRITE_SHORT(m_iBeamIndex);
        WRITE_BYTE(0); // FRAMERATE
        WRITE_BYTE(0); // FRAMERATE
        WRITE_BYTE(1);// LIFE
        WRITE_BYTE(20);  // WIDTH
        WRITE_BYTE(50);   // NOISE
        WRITE_BYTE(250);   // R, G, B
        WRITE_BYTE(200);   // R, G, B
        WRITE_BYTE(0);  // R, G, B
        WRITE_BYTE(255);	// BRIGHTNESS
        WRITE_BYTE(25);		// SPEED
        MESSAGE_END();
#endif

        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME; // 600
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH; // 512

        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.075s;
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;

        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            KickBack(1.5, 0.55, 0.3, 0.3, 6.0, 5.0, 5);
        else if (m_pPlayer->pev->velocity.Length2D() > 0)
            KickBack(1.1, 0.3, 0.2, 0.06, 4.0, 2.5, 8);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            KickBack(0.75, 0.1, 0.1, 0.018, 3.5, 1.2, 9);
        else
            KickBack(0.8, 0.2, 0.18, 0.02, 3.2, 2.25, 7);
    }

    BOOL IsTargetAvailable(CBasePlayer* m_pPlayer, CBaseEntity* pEntity, Vector vecSrc)
    {
        if (!pEntity->IsAlive())
            return false;

#ifndef CLIENT_DLL
        if (g_pGameRules->PlayerRelationship(m_pPlayer, pEntity) == GR_TEAMMATE)
            return false;
#endif

        if (pEntity->IsBSPModel())
            return false;

        if (pEntity->pev == m_pPlayer->pev)
            return false;

        Vector vecGunPostion = m_pPlayer->GetGunPosition();
        Vector vecDelta = (pEntity->Center() - vecGunPostion).Normalize();

        UTIL_MakeVectors(m_pPlayer->pev->v_angle);

        if (DotProduct(gpGlobals->v_forward, vecDelta) < 0.5)
            return false;

        TraceResult tr;
        UTIL_TraceLine(m_pPlayer->pev->origin, pEntity->pev->origin, missile, m_pPlayer->edict(), &tr);

        if (tr.flFraction < 1.0)
        {
            if (!tr.pHit)
                return false;

            if (tr.pHit != pEntity->edict())
                return false;
        }

        return true;
    }



    void ItemPostFrame(void) override
    {
        if (m_flNextReset != invalid_time_point && m_flNextReset < gpGlobals->time)
        {
            if (m_iWeaponState == WPNSTATE_JANUS7_SIGNAL)
            {
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0s;
                m_flNextReset = invalid_time_point;
            }
            if (m_iWeaponState == WPNSTATE_JANUS7_USING)
            {
                m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 0;
                SendWeaponAnim(ANIM_CHANGE2, UseDecrement() != FALSE);
                m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.7s;
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.7s;
                m_flNextReset = invalid_time_point;
            }
            pev->iuser1 = 0;
            m_iWeaponState = 0;
        }
        return CBasePlayerWeapon::ItemPostFrame();
    }

    void SecondaryAttack(void)
    {
        if (m_iWeaponState == WPNSTATE_JANUS7_SIGNAL)
        {
            m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 255;
            m_iWeaponState = WPNSTATE_JANUS7_USING;
            SendWeaponAnim(ANIM_CHANGE1, UseDecrement() != FALSE);
            m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.0s;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;

            m_flNextReset = gpGlobals->time + 15.0s;
        }
    }

    void Reload() override
    {
        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iWeaponState == WPNSTATE_JANUS7_USING)
            return;

        if (DefaultReload(MaxClip, m_iWeaponState == WPNSTATE_JANUS7_SIGNAL ? ANIM_RELOAD_SIGNAL : ANIM_RELOAD1, 4.73s))
        {
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
            m_flAccuracy = 0.22;
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

        int iAnim;
        if (m_iWeaponState == WPNSTATE_JANUS7_USING)
            iAnim = ANIM_IDLE2;
        else if (m_iWeaponState == WPNSTATE_JANUS7_SIGNAL)
            iAnim = ANIM_SIGNAL;
        else
            iAnim = ANIM_IDLE1;

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
        SendWeaponAnim(iAnim, UseDecrement() != FALSE);
    }

    int GetDamageB()
    {
        int iDamage = 50;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            iDamage = 47;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            iDamage = 190;
#endif
        return iDamage;
    }

};
LINK_ENTITY_TO_CLASS(weapon_janus7, CJANUS7)
}