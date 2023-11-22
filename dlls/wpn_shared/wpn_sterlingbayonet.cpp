/*
sterlingbayonet.cpp
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
#include "weapons/KnifeAttack.h"
#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif
    class CSterlingBayonet : public LinkWeaponTemplate< CSterlingBayonet,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_P90>::template type,
            TPrecacheEvent,
            TReloadDefault,
            TGetDamageDefault,
            TPrimaryAttackRifle,
            TFireRifle,
			TRecoilKickBack
    >
    {
    public:
    static constexpr const auto &DefaultReloadTime = 2.7s;
    static constexpr const char *V_Model = "models/v_sterlingbayonet.mdl";
    static constexpr const char *P_Model = "models/p_sterlingbayonet.mdl";
    static constexpr const char *W_Model = "models/w_sterlingbayonet.mdl";
    static constexpr const char* W_Model_2 = "models/w_sterlingbayonetb.mdl";
    static constexpr const char *EventFile = "events/sterlingbayonet.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_sterlingbayonet";
    static constexpr const char *AnimExtension = "mp5";
    static constexpr int MaxClip = 50;
	enum
	{
		ANIM_IDLE1 = 0,
        ANIM_IDLE1_BROKEN,
        ANIM_SHOOT1,
        ANIM_SHOOT1_BROKEN,
        ANIM_KNIFE_READY,
        ANIM_KNIFE_01,
        ANIM_KNIFE_02,
        ANIM_KNIFE_BROKEN01,
        ANIM_KNIFE_BROKEN02,
        ANIM_RELOAD,
        ANIM_RELOAD_BROKEN,
        ANIM_DRAW,
        ANIM_DRAW_BROKEN,
	};
    static constexpr float PlayerMaxSpeed = 260;
    static constexpr float ArmorRatioModifier = 1.0f;
    static constexpr const auto &SpreadCalcNotOnGround = 0.2*A;
    static constexpr const auto &SpreadCalcDefault = 0.01*A;

	static constexpr const auto& CycleTime = 0.06s;
    static constexpr int DamageDefault = 30;
    static constexpr int DamageZB = 130;
    static constexpr int DamageZBS = 240;
    static constexpr const auto &AccuracyCalc = (N * N / 230.0) + 0.3;
    static constexpr float AccuracyDefault = 0.0;
    static constexpr float AccuracyMax = 0.6;
    static constexpr float RangeModifier = 0.88;
    static constexpr auto BulletType = BULLET_PLAYER_9MM;
    /*static constexpr*/ int Penetration = 1;
    KnockbackData KnockBack = { 450.0f, 350.0f, 400.0f, 200.0f, 0.7f };

	KickBackData KickBackWalking = { 0.3f, 0.15f, 0.15f, 0.02f, 3.0f, 2.0f, 10 };
	KickBackData KickBackNotOnGround = { 0.9f, 0.45f, 0.35f, 0.035f, 5.0f, 3.0f, 6 };
	KickBackData KickBackDucking = { 0.2f, 0.1f, 0.1f, 0.01f, 2.0f, 1.0f, 10 };
	KickBackData KickBackDefault = { 0.22f, 0.12f, 0.12f, 0.012f, 2.2f, 1.2f, 10 };

    int m_iKnifeModel;
    		 	
    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "9mm";
        p->iMaxAmmo1 = 100;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = MaxClip;
        p->iSlot = 0;
        p->iPosition = 1;
        p->iId = m_iId = WEAPON_STERLINGBAYONET;
        p->iFlags = 0;
        p->iWeight = TMP_WEIGHT;

        return 1;
    }

    float GetArmorRatioModifier() override { return pev->iuser1 ? 1.7 : 1.0; }
#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_TMP); }
#endif
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR, 6, 2, 0 }; }

    float GetSecondaryAttackDamage() const
    {
        float flDamage = m_iWeaponState ? 38.0f : 118.0f;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = m_iWeaponState ? 1045.0f : 2000.0f;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = m_iWeaponState ? 1000.0f : 8020.0f;
#endif
        return flDamage;
    }

    float GetRadius() const
    {
        float flRadius = m_iWeaponState ? 1.9f : 2.1f;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flRadius = m_iWeaponState ? 1.9f : 2.1f;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flRadius = m_iWeaponState ? 1.9f : 2.1f;
#endif
        return flRadius * 39.37;
    }

    void Precache()
    {
        m_iKnifeModel = PRECACHE_MODEL("models/sterlingbayonet_broken.mdl");
        PRECACHE_MODEL(W_Model_2);

        Base::Precache();
    }

    const char* GetCSModelName() override { return m_iWeaponState ? W_Model_2 : W_Model; }

    void Holster(int skiplocal) override
    {
        SetThink(nullptr);
        return CBasePlayerWeapon::Holster(skiplocal);
    }

    void FireEvent(const Vector& vecDir)
    {
        int flags = 0;
#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif
        
        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, static_cast<int>(m_pPlayer->pev->punchangle.x * 100), static_cast<int>(m_pPlayer->pev->punchangle.y * 100), m_iWeaponState == 1, FALSE);
    }

#ifndef CLIENT_DLL
    void CreateBrokenKnife(Vector vecStart, Vector vecForward, float flRadius)
    {
        Vector vecEnd = vecStart + vecForward * flRadius;
        TraceResult ptr;
        UTIL_TraceLine(vecStart, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &ptr);
        Vector vecAngle = UTIL_VecToAngles(vecForward);

        vecEnd = ptr.vecEndPos;

        if (ptr.flFraction >= 1.0)
            return;

        CBaseEntity* pEntity = CBaseEntity::Instance(ptr.pHit);

        if (pEntity->IsBSPModel())
        {
            MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecEnd);
            WRITE_BYTE(TE_ARROWMODEL);
            WRITE_COORD(vecEnd.x - vecForward.x * 1);
            WRITE_COORD(vecEnd.y - vecForward.y * 1);
            WRITE_COORD(vecEnd.z - vecForward.z * 1);
            WRITE_COORD(vecAngle[0]);
            WRITE_COORD(vecAngle[1]);
            WRITE_COORD(vecAngle[2]);
            WRITE_SHORT(m_iKnifeModel);
            WRITE_BYTE(50);
            WRITE_BYTE(10);
            WRITE_BYTE(1);
            WRITE_BYTE(200);
            WRITE_BYTE(4);
            WRITE_SHORT(-1);
            WRITE_BYTE(17);
            MESSAGE_END();
        }
    }
#endif

    void DelaySecondaryAttack()
    {
#ifndef CLIENT_DLL
        if (!m_iWeaponState)
        {
            pev->iuser1 = 1;
            UTIL_MakeVectors(m_pPlayer->pev->v_angle);
            Vector vecSrc = m_pPlayer->GetGunPosition();

            float flRadius = GetRadius();

            switch (KnifeAttack(vecSrc, gpGlobals->v_forward, GetSecondaryAttackDamage(), flRadius, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer->pev))
            {
            case HIT_NONE:
            {
                SendWeaponAnim(ANIM_KNIFE_01, UseDecrement() != FALSE);

                UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/sterlingbayonet_knife01.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

                break;
            }
            case HIT_PLAYER:
            {
                m_iWeaponState = 1;

                UTIL_SetPlayerPModel(m_pPlayer->entindex(), WEAPON_STERLINGBAYONET, m_iWeaponState);

                SendWeaponAnim(ANIM_KNIFE_02, UseDecrement() != FALSE);

                UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/sterlingbayonet_knife02.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
                
                UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "weapons/sterlingbayonet_hit.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
                
                m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;

                break;
            }
            case HIT_WALL:
            {
                m_iWeaponState = 1;

                UTIL_SetPlayerPModel(m_pPlayer->entindex(), WEAPON_STERLINGBAYONET, m_iWeaponState);

                SendWeaponAnim(ANIM_KNIFE_02, UseDecrement() != FALSE);

                CreateBrokenKnife(vecSrc, gpGlobals->v_forward, flRadius);

                if (RANDOM_LONG(0, 1))            
                    UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/sterlingbayonet_stone1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
                else
                    UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/sterlingbayonet_stone2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
                
                m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.1;

                break;
            }
            }
        }
        else
        {
            switch (KnifeAttack5(GetSecondaryAttackDamage(), GetRadius(), 90, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer))
            {
            case HIT_NONE:
            {
                break;
            }
            case HIT_PLAYER:
            {
                if (RANDOM_LONG(0, 1))
                    UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/knife_hit2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
                else
                    UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/knife_hit4.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
                m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
                break;
            }
            case HIT_WALL:
            {

                UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "weapons/knife_hitwall1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
                m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
                break;
            }
            }
        }
        pev->iuser1 = 0;
#endif
        SetThink(nullptr);

        Q_strcpy(m_pPlayer->m_szAnimExtention, AnimExtension);

    }

    void SecondaryAttack() override
    {
        Q_strcpy(m_pPlayer->m_szAnimExtention, "m16a1ep");

#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK2);
#endif

        if (m_iWeaponState)
        {
            m_iSwing++;
            if (m_iSwing & 1)
            {
                SendWeaponAnim(ANIM_KNIFE_BROKEN01, UseDecrement() != FALSE);

                UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/sterlingbayonet_knife_broken01.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
            }
              
            else
            {
                SendWeaponAnim(ANIM_KNIFE_BROKEN02, UseDecrement() != FALSE);

                UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/sterlingbayonet_knife_broken02.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
            }
                

            SetThink(&CSterlingBayonet::DelaySecondaryAttack);
            pev->nextthink = gpGlobals->time + 0.46s;

            m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.3s;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
        }
        else
        {
            SendWeaponAnim(ANIM_KNIFE_READY, UseDecrement() != FALSE);

            SetThink(&CSterlingBayonet::DelaySecondaryAttack);
            pev->nextthink = gpGlobals->time + 0.46s;

            m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.3s;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
        }
        

    }

    void Reload() override
    {
        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            return;

        if (DefaultReload(MaxClip, m_iWeaponState ? ANIM_RELOAD_BROKEN : ANIM_RELOAD, DefaultReloadTime))
        {
#ifndef CLIENT_DLL         
            m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif  
            m_flAccuracy = AccuracyDefault;
        }
    }

    void WeaponIdle(void) override
    {
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

        if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
            return;

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
        SendWeaponAnim(m_iWeaponState ? ANIM_IDLE1_BROKEN : ANIM_IDLE1, UseDecrement() != FALSE);
    }

    BOOL Deploy(void) override
    {
        m_iSwing = 0;
        m_iShotsFired = 0;
        m_bDelayFire = false;
        m_flAccuracy = AccuracyDefault;

        if (DefaultDeploy(V_Model, P_Model, m_iWeaponState ? ANIM_DRAW_BROKEN : ANIM_DRAW, AnimExtension, UseDecrement() != FALSE))
        {
#ifndef CLIENT_DLL  
            UTIL_SetPlayerPModel(m_pPlayer->entindex(), WEAPON_STERLINGBAYONET, m_iWeaponState ? 1 : 0);
#endif
            return TRUE;
        }
        return FALSE;
    }



};
LINK_ENTITY_TO_CLASS(weapon_sterlingbayonet, CSterlingBayonet)
}