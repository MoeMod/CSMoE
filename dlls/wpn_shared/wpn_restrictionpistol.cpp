/*
restrictionpistol.cpp
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
#include "weapons/RadiusDamage.hpp"
#include "weapons/KnifeAttack.h"
#include "weapons/WeaponTemplate.hpp"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

    class CRestrictionPistol : public LinkWeaponTemplate< CRestrictionPistol,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_GLOCK18>::template type,
            TPrecacheEvent     
    >
    {
    public:
    static constexpr const char *V_Model = "models/v_restrictionpistol.mdl";
    static constexpr const char *P_Model = "models/p_restrictionpistol.mdl";
    static constexpr const char *W_Model = "models/w_restrictionpistol.mdl";
    static constexpr const char *EventFile = "events/restrictionpistol.sc";
    static constexpr InventorySlotType ItemSlot = PISTOL_SLOT;
    static constexpr const char *ClassName = "weapon_restrictionpistol";
    static constexpr const char *AnimExtension = "restrictionpistol";
    static constexpr int MaxClip = 50;
    KnockbackData KnockBack = { 420.0f, 380.0f, 380.0f, 550.0f, 0.8f };
	enum
	{
        ANIM_IDLE1,
        ANIM_RELOAD,
        ANIM_DRAW,
        ANIM_SHOOT1,
        ANIM_SHOOT2_START,
        ANIM_SHOOT2_CHARGING1,
        ANIM_SHOOT2_CHARGING2,
        ANIM_SHOOT2_CHARGING3,
        ANIM_SHOOT2_CHARGING4,
        ANIM_SHOOT2_CHARGING5,
        ANIM_SHOOT2_END,
	};

    int m_iClip2;
    int m_iCModeModelIndex;
    int m_iDefaultAmmo2;
    time_point_t m_flTimeNextCharge;
    time_point_t m_flNextReload;
    time_point_t m_flAttackLoop;

    KnockbackData GetKnockBackData() override
    {
       return  { 420.0f, 380.0f, 380.0f, 550.0f, 0.8f };
    }

    void Precache(void)
    {
        m_iCModeModelIndex = PRECACHE_MODEL("models/ef_restrictionpistol_cmode.mdl");

        PRECACHE_MODEL("sprites/ef_restrictionpistol_hit.spr");
        PRECACHE_MODEL("sprites/muzzleflash262.spr");
        PRECACHE_MODEL("sprites/muzzleflash263.spr");
        PRECACHE_MODEL("sprites/muzzleflash264.spr");
        PRECACHE_MODEL("sprites/muzzleflash265.spr");
        PRECACHE_MODEL("sprites/muzzleflash266.spr");
        PRECACHE_MODEL("sprites/muzzleflash267.spr");

        PRECACHE_SOUND("weapons/restrictionpistol-2_end.wav");
        PRECACHE_SOUND("weapons/restrictionpistol-2.wav");
        PRECACHE_SOUND("weapons/restrictionpistol-3.wav");
        PRECACHE_SOUND("weapons/restrictionpistol_kill.wav");
        Base::Precache();
    }

    void Spawn() override
    {
        m_iDefaultAmmo2 = 0;

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

    int GetItemInfo(ItemInfo* p) override
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "restrictionpistol";
        p->iMaxAmmo1 = 50;
        p->pszAmmo2 = "restrictionpistol2";
        p->iMaxAmmo2 = 100;
        p->iMaxClip = 50;
        p->iSlot = 1;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_RESTRICTIONPISTOL;
        p->iFlags = 0;
        p->iWeight = GLOCK18_WEIGHT;
        return 1;
    }

    bool HasSecondaryAttack() override { return true; }
#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_restrictionpistol" , 300 }; }
#endif
    float GetArmorRatioModifier() override { return 1.5; }
    WeaponCrosshairData GetCrosshairData() override { return { 0, 8, 3, 0 }; }


    float GetPrimaryAttackDamage() const
    {
        int flDamage = 15;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = 355;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = 750;
#endif
        return flDamage;
    }

    float GetSecondaryAttackDamage() const
    {
        float flDamage = 5.0f;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = 108.0f;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = 280.0f;
#endif
        return flDamage;
    }

    float GetRadiusDamage(int iState) const
    {
        float flDamage;
        if (iState == 2)
            flDamage = 20.0;
        else if (iState == 1)
            flDamage = 15.0;
        else
            flDamage = 10.0;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
        {
            if (iState == 2)
                flDamage = 225.0;
            else if (iState == 1)
                flDamage = 225.0;
            else
                flDamage = 108.0;
        }
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
        {
            if (iState == 2)
                flDamage = 1127.0;
            else if (iState == 1)
                flDamage = 735.0;
            else
                flDamage = 280.0;
        }
#endif
        return flDamage;
    }


    float GetRadius(int iState) const
    {
        float flRadius;
        if (iState == 2)
            flRadius = 618;
        else if (iState == 1)
            flRadius = 421;
        else
            flRadius = 224;
        return flRadius;
    }


    BOOL Deploy(void) override
    {
        m_iShotsFired = 0;
        m_flAttackLoop = invalid_time_point;
        m_flNextReload = invalid_time_point;
        m_flTimeNextCharge = gpGlobals->time + 1.0s;

        SetChargeSkin();

        return DefaultDeploy("models/v_restrictionpistol.mdl", "models/p_restrictionpistol.mdl", ANIM_DRAW, "onehanded", UseDecrement() != FALSE, 0.2s);
    }


    void Reload(void)
    {
        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            return;

        if (DefaultReload(50, ANIM_RELOAD, 1.9s, 1))
        {
#ifndef CLIENT_DLL         
            m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
            m_flNextReload = gpGlobals->time + 1.9s;
        }
    }

    void RadiusAttack2()
    {
#ifndef CLIENT_DLL
        UTIL_MakeVectors(m_pPlayer->pev->v_angle);
        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecAngle = m_pPlayer->pev->angles;
        vecAngle.x = 0;
        int iState;
        if (m_iWeaponState > 6)
            iState = 2;
        else if (m_iWeaponState > 3)
            iState = 1;
        else
            iState = 0;

        struct
        {
            float RadiusDamageRadius;
            int RadiusDamageBitsDamageType;
            bool RadiusDamageCanHeadshot;
            float RadiusDamageAmount;
        } Data;

        UTIL_TempModel({ 0, 0, -16 }, vecAngle, g_vecZero, m_iCModeModelIndex, 2, iState, 10, 0, 0, 255, kRenderTransAdd, m_pPlayer, false, 0, 0, 11, 0);

        EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/restrictionpistol-3.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

        Data.RadiusDamageAmount = GetRadius(iState);
        Data.RadiusDamageBitsDamageType = DMG_BULLET;
        Data.RadiusDamageCanHeadshot = false;
        Data.RadiusDamageAmount = GetRadiusDamage(iState);

        std::vector<CBaseEntity*> out;
        RadiusDamage(Data, vecSrc, m_pPlayer, m_pPlayer, std::back_inserter(out));
        for (CBaseEntity* ent : out)
        {
            CBasePlayer* player = dynamic_ent_cast<CBasePlayer*>(ent);
            if (player && player->m_bIsZombie)
            {
                Vector vecDir = (player->pev->origin - m_pPlayer->pev->origin);
                vecDir.z = 0;

                ApplyKnockbackData(player, player->Center() - vecSrc, { 3000, 3000, 3000, 3000, 1.0f });
                player->pev->velocity.z = +200.0f;
            }
        }
#endif
    }

    void Holster(int skiplocal)
    {
        if (m_iWeaponState)
            RadiusAttack2();
        m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 0;
        return CBasePlayerWeapon::Holster(skiplocal);
    }

    void SecondaryAttack(void)
    {
        if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] <= 0)
        {
#ifndef CLIENT_DLL
            if (TheBots)
                TheBots->OnEvent(EVENT_WEAPON_FIRED_ON_EMPTY, m_pPlayer, NULL);
#endif

            if (m_flAttackLoop != invalid_time_point)
            {          
                SendWeaponAnim(ANIM_SHOOT2_END, UseDecrement() != FALSE);

                EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/restrictionpistol-2_end.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

                m_flNextSecondaryAttack = m_flNextPrimaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5s;
                m_flTimeNextCharge = gpGlobals->time + 1.0s;
                m_flAttackLoop = invalid_time_point;
            }

            return;
        }

        int flags;
#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        if (m_flAttackLoop == invalid_time_point)
        {
            m_flTimeNextCharge = gpGlobals->time + 1.0s;
            SendWeaponAnim(ANIM_SHOOT2_START, UseDecrement() != FALSE);
            
            m_iClip2 = 0;

            EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/restrictionpistol-2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.23s;
            m_flAttackLoop = gpGlobals->time + 0.2s;

            return;
        }

        if (m_flAttackLoop > gpGlobals->time)
            return;

        m_iShotsFired++;
        m_bDelayFire = true;
        
        m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] -= 5;
        
        if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] < 0)
            m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 0;

#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
        SetChargeSkin();
        int iAnim;
        float flDamage = GetSecondaryAttackDamage();
        if (m_iWeaponState > 8)
        {
            iAnim = ANIM_SHOOT2_CHARGING5;
            flDamage *= 3.0;
        }      
        else if(m_iWeaponState > 6)
        {
            iAnim = ANIM_SHOOT2_CHARGING4;
            flDamage *= 2.0;
        }
        else if (m_iWeaponState > 4)
        {
            iAnim = ANIM_SHOOT2_CHARGING3;
            flDamage *= 1.5;
        }
        else if (m_iWeaponState > 2)
        {
            iAnim = ANIM_SHOOT2_CHARGING2;
            flDamage *= 1.2;
        }
        else
            iAnim = ANIM_SHOOT2_CHARGING1;

        SendWeaponAnim(iAnim, UseDecrement() != FALSE);

        m_flTimeWeaponIdle = m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.125s;
        m_flTimeNextCharge = gpGlobals->time + 1.0s;
        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;

        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, 0.03, 822, 4, BULLET_PLAYER_9MM, flDamage, 0.8, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);


        //PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFire, 0.0, g_vecZero, g_vecZero, 0.0, 0.0, int(m_pPlayer->pev->punchangle.x * 100.0), int(m_pPlayer->pev->punchangle.y * 100.0), FALSE, TRUE);
#ifndef CLIENT_DLL
        if (!m_iClip && !m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", 0, 0);
#endif

        m_pPlayer->pev->punchangle.x -= 2;
    }


    void SetChargeSkin(void)
    {
        if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] >= 100)
            m_iWeaponState = 10;
        else if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] >= 92)
            m_iWeaponState = 9;
        else if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] >= 80)
            m_iWeaponState = 8;
        else if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] >= 72)
            m_iWeaponState = 7;
        else if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] >= 60)
            m_iWeaponState = 6;
        else if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] >= 52)
            m_iWeaponState = 5;
        else if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] >= 40)
            m_iWeaponState = 4;
        else if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] >= 32)
            m_iWeaponState = 3;
        else if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] >= 20)
            m_iWeaponState = 2;
        else if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] >= 12)
            m_iWeaponState = 1;
        else
            m_iWeaponState = 0;

        pev->skin = m_iWeaponState;
    }


    void PrimaryAttack(void)
    {
        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            RestrictionPistoFire((1.2) * (1 - m_flAccuracy), 0.25s, TRUE);
        else if (m_pPlayer->pev->velocity.Length2D() > 0)
            RestrictionPistoFire((0.185) * (1 - m_flAccuracy), 0.25s, TRUE);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            RestrictionPistoFire((0.095) * (1 - m_flAccuracy), 0.25s, TRUE);
        else
            RestrictionPistoFire((0.3) * (1 - m_flAccuracy), 0.25s, TRUE);
    }


    void RestrictionPistoFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        if (m_flLastFire != invalid_time_point)
        {
            m_flAccuracy -= (0.3 - ((gpGlobals->time - m_flLastFire) / 1s)) * 0.325;

            if (m_flAccuracy > 0.9)
                m_flAccuracy = 0.9;
            else if (m_flAccuracy < 0.55)
                m_flAccuracy = 0.55;
        }

        m_flLastFire = gpGlobals->time;

        if (m_iClip <= 0)
        {
            if (m_fFireOnEmpty)
            {
                PlayEmptySound();
                m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
            }
            return;
        }
       
        if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] > 100)
            m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 100;
        else
            m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] += 4;

        SetChargeSkin();
        m_iClip--;
        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;

        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 4, BULLET_PLAYER_9MM, GetPrimaryAttackDamage(), 0.8, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

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
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
        m_pPlayer->pev->punchangle.x -= 2;
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

        }

        if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] < 100)
        {
            if (m_flTimeNextCharge <= gpGlobals->time)
            {
                m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] += 4;
                if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] > 100)
                    m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 100;

                SetChargeSkin();
                m_flTimeNextCharge = gpGlobals->time + 1.0s;
            }
        }
        else
            m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 100;
        
        if (m_fInReload)
            return;

        return CBasePlayerWeapon::ItemPostFrame();
    }

    void WeaponIdle(void)
    {
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

        if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
            return;

        if (m_flAttackLoop != invalid_time_point)
        {
            SendWeaponAnim(ANIM_SHOOT2_END, UseDecrement() != FALSE);

            EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/restrictionpistol-2_end.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

            m_flNextSecondaryAttack = m_flNextPrimaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5s;
            m_flTimeNextCharge = gpGlobals->time + 1.0s;
            m_flAttackLoop = invalid_time_point;
        }
        else
        {
            SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);

            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
        }
    }
    		 	
};
LINK_ENTITY_TO_CLASS(weapon_restrictionpistol, CRestrictionPistol)
}