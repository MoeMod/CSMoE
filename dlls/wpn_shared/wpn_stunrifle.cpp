/*
stunrifle.cpp
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

#ifndef CLIENT_DLL
#include "effects.h"
#include "customentity.h"
#include "monsters.h"
#endif


#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif
    class CBeam;
    class CStunRifle : public LinkWeaponTemplate< CStunRifle,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_AK47>::template type,
            TPrecacheEvent,
            TGetDamageDefault,
            TPrimaryAttackRifle,
            TFireRifle,
			TRecoilKickBack
    >
    {
    public:
    static constexpr const char *V_Model = "models/v_stunrifle.mdl";
    static constexpr const char *P_Model = "models/p_stunrifle.mdl";
    static constexpr const char *W_Model = "models/w_stunrifle.mdl";
    static constexpr const char *EventFile = "events/stunrifle.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_stunrifle";
    static constexpr const char *AnimExtension = "ak47";
    static constexpr int MaxClip = 30;
	enum
	{
		ANIM_IDLEA = 0,
        ANIM_IDLEB,
        ANIM_IDLEC,
        ANIM_SHOOTA,
        ANIM_SHOOTB,
        ANIM_SHOOTC,
        ANIM_RELOADA,
        ANIM_RELOADB,
        ANIM_RELOADC,
        ANIM_DRAWA,
        ANIM_DRAWB,
        ANIM_DRAWC,
	};
    static constexpr float PlayerMaxSpeed = 235;
    static constexpr float ArmorRatioModifier = 1.4f;
	static constexpr float PrimaryAttackWalkingMiniumSpeed = 140;
    static constexpr const auto &SpreadCalcNotOnGround = 0.4*A + 0.04;
    static constexpr const auto &SpreadCalcWalking = 0.0275*A;
    static constexpr const auto &SpreadCalcDefault = 0.03*A;

	static constexpr const auto& CycleTime = 0.1s;
    static constexpr int DamageDefault = 34;
    static constexpr int DamageZB = 139;
    static constexpr int DamageZBS = 169;
    static constexpr const auto &AccuracyCalc = (N * N * N / 220.0) + 0.3;
    static constexpr float AccuracyDefault = 0.2;
    static constexpr float AccuracyMax = 1.0;
    static constexpr float RangeModifier = 0.98;
    static constexpr auto BulletType = BULLET_PLAYER_556MM;
    /*static constexpr*/ int Penetration = 1;

   // KnockbackData KnockBack = { 100.0f, 150.0f, 200.0f, 100.0f, 0.3f };
    KnockbackData KnockBack = { 350.0f, 250.0f, 300.0f, 100.0f, 0.5f };
    static constexpr const char* Beam_SPR = "sprites/lgtning.spr"; //

	KickBackData KickBackWalking = { 0.7, 0.35, 0.18, 0.035, 2.75, 2.3, 7 };
	KickBackData KickBackNotOnGround = { 1.2, 0.5, 0.23, 0.15, 5.5, 3.5, 6 };
	KickBackData KickBackDucking = { 0.5, 0.2, 0.1, 0.0125, 2.25, 2.0, 7 };
	KickBackData KickBackDefault = { 0.6, 0.3, 0.15, 0.015, 2.5, 2.15, 7 };

    int m_iEf_Elec;
    time_point_t m_flTimeNextCharge;
    time_point_t m_flTimeNextCharge2;
    std::array<CBeam*, 3> m_pBeamList;
    std::vector<EHANDLE> m_pEnemyList;


    KnockbackData GetKnockBackData() override
    {
        if (m_fInReload)
            return { 100.0f, 150.0f, 200.0f, 100.0f, 0.3f };
        else
            return { 350.0f, 250.0f, 300.0f, 100.0f, 0.5f };
    }

    void Spawn() override
    {
        InitChargeState();
        Base::Spawn();
        m_iId = WEAPON_STUNRIFLE;
    }

    int GetItemInfo(ItemInfo* p) override
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "556Nato";
        p->iMaxAmmo1 = MAX_AMMO_762NATO;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = 30;
        p->iSlot = 0;
        p->iPosition = 1;
        p->iId = m_iId = WEAPON_STUNRIFLE;
        p->iFlags = 0;
        p->iWeight = AK47_WEIGHT;

        return 1;
    }

    float GetArmorRatioModifier() override { return 1.4; }
#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_M4A1); }
#endif
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 4, 4, 140 }; }

    void Precache()
    {
        PRECACHE_MODEL(Beam_SPR);
        m_iEf_Elec = PRECACHE_MODEL("sprites/ef_elec.spr");
        m_iEf_Elec = PRECACHE_MODEL("sprites/muzzleflash67.spr");
        PRECACHE_SOUND("weapons/stunrifle_lowbattery.wav");
        PRECACHE_SOUND("weapons/stunrifle-2.wav");
        return Base::Precache();
    }

    void InitChargeState(void)
    {
        m_iWeaponState = 0;
        m_flTimeNextCharge = m_flTimeNextCharge2 = invalid_time_point;

        pev->skin = 0;
    }

    void SetChargeSkin(void)
    {
        pev->skin = m_iWeaponState;
    }

    void ResetChargeTimer(void)
    {
        if (m_iWeaponState != 8 && m_flTimeNextCharge == invalid_time_point)
            m_flTimeNextCharge = gpGlobals->time + 1.0s;
    }

    void Holster(int skiplocal)
    {
        ResetChargeTimer();
        SetThink(nullptr);
        ClearEffect();
        DestroyEffect();
        CBasePlayerWeapon::Holster();
    }

    void CreateEffect()
    {
#ifndef CLIENT_DLL
        for (size_t i = 0; i < 3; ++i)
        {
            CBeam* pBeam = CBeam::BeamCreate(Beam_SPR, 30);
            pBeam->SetColor(0, 100, 255);
            pBeam->SetScrollRate(30);
            pBeam->SetBrightness(0);
            pBeam->SetNoise(30);
            pev->effects |= EF_NODRAW;

            m_pBeamList[i] = pBeam;
        }
#endif
    }

    void ClearEffect()
    {
#ifndef CLIENT_DLL
        for (CBeam* pBeam : m_pBeamList)
        {
            if (pBeam)
            {
                pBeam->SetBrightness(0);
                pev->effects |= EF_NODRAW; // 0x80
            }
        }
#endif
        m_pEnemyList.clear();
    }


    void DestroyEffect()
    {
#ifndef CLIENT_DLL
        for (CBeam* p : m_pBeamList)
        {
            if (p)
                UTIL_Remove(p);
        }
        m_pBeamList[0] = NULL;
        m_pBeamList[1] = NULL;
        m_pBeamList[2] = NULL;
#endif
    }

    BOOL Deploy(void) override
    {
        ResetChargeTimer();
        CreateEffect();
        m_flAccuracy = 0.2;
        m_iShotsFired = 0;

        if (!m_iWeaponState)
        {
            return DefaultDeploy("models/v_stunrifle.mdl", "models/p_stunrifle.mdl", ANIM_DRAWA, "ak47", UseDecrement() != FALSE);
        }
        else if (m_iWeaponState < 8)
        {
            return DefaultDeploy("models/v_stunrifle.mdl", "models/p_stunrifle.mdl", ANIM_DRAWB, "ak47", UseDecrement() != FALSE);
        }
        else
        {
            return DefaultDeploy("models/v_stunrifle.mdl", "models/p_stunrifle.mdl", ANIM_DRAWC, "ak47", UseDecrement() != FALSE);
        }
    }

    void PrimaryAttack() override
    {
        if (m_iClip > 0)
        {
            if (!m_iWeaponState)
                SendWeaponAnim(ANIM_SHOOTA, UseDecrement() != FALSE);
            else if (m_iWeaponState < 8)
                SendWeaponAnim(ANIM_SHOOTB, UseDecrement() != FALSE);
            else
                SendWeaponAnim(ANIM_SHOOTC, UseDecrement() != FALSE);
        }

        Base::PrimaryAttack();

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.2s;
    }

    void FireEvent(const Vector& vecDir)
    {
        int flags = 0;
#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif
        
        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, static_cast<int>(m_pPlayer->pev->punchangle.x * 100), static_cast<int>(m_pPlayer->pev->punchangle.y * 100), m_iWeaponState, FALSE);
    }

    void ItemPostFrame() override
    {
        if (m_flTimeNextCharge != invalid_time_point && m_flTimeNextCharge <= gpGlobals->time)
        {
            m_iWeaponState += int((gpGlobals->time - m_flTimeNextCharge) / 2.0s + 1.0);

            if (m_iWeaponState >= 8)
                m_iWeaponState = 8;

            m_flTimeNextCharge = gpGlobals->time + 2.0s;

            if (m_iWeaponState == 8)
                m_flTimeNextCharge = invalid_time_point;

            SetChargeSkin();
        }

        return CBasePlayerWeapon::ItemPostFrame();
    }

    void PrimaryAttack_FindTargets()
    {
        const float flRadius = 320;

        m_pEnemyList.clear();
#ifndef CLIENT_DLL
        CBaseEntity* pEntity = NULL;
        while ((pEntity = UTIL_FindEntityInSphere(pEntity, m_pPlayer->pev->origin, flRadius)) != nullptr)
        {
            if (PrimaryAttack_CheckTargetAvailable(pEntity, m_pPlayer->pev->v_angle))
            {
                EHANDLE eh;
                eh.Set(pEntity->edict());
                m_pEnemyList.push_back(eh);
            }
        }
#endif
    }

    bool PrimaryAttack_CheckTargetAvailable(CBaseEntity* a2, Vector vecAngleDirection)
    {
        const float flRadius = 255;

        if (!a2->IsPlayer() && a2->Classify() != CLASS_PLAYER_ALLY)
            return false;

        if (!a2->IsAlive())
            return false;

#ifndef CLIENT_DLL
        if (g_pGameRules->PlayerRelationship(m_pPlayer, a2) == GR_TEAMMATE)
            return false;
#endif

        if (a2->pev == m_pPlayer->pev)
            return false;

        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecDelta = (a2->Center() - vecSrc).Normalize();

        UTIL_MakeVectors(vecAngleDirection);

        if (DotProduct(gpGlobals->v_forward, vecDelta) < 0.5)
            return false;

        TraceResult tr;
        UTIL_TraceLine(m_pPlayer->pev->origin, a2->pev->origin, missile, m_pPlayer->edict(), &tr);

        if (tr.flFraction < 1.0)
        {
            if (!tr.pHit)
                return false;

            if (tr.pHit != a2->edict())
                return false;
        }

        return true;
    }


    void PrimaryAttack_InstantDamage()
    {
        if (m_pEnemyList.empty())	//m_pEnemyList
            return;
 
        size_t v8 = 0;
        for (CBaseEntity* pEntity : m_pEnemyList)
        {
            if (v8 >= 3)
                break;
            if (!pEntity)
                continue;
#ifndef CLIENT_DLL
            pEntity->m_LastHitGroup = HITGROUP_CHEST;
#endif
            pEntity->TakeDamage(pev, m_pPlayer->pev, GetDamageB(), DMG_BULLET);

            if (v8 < 3)
            {
#ifndef CLIENT_DLL
                CBeam* pBeam = m_pBeamList[v8];
                if (pBeam)
                {
                    pBeam->SetType(BEAM_ENTS);
                    pBeam->SetStartEntity(ENTINDEX(m_pPlayer->edict()));
                    pBeam->SetEndEntity(ENTINDEX(pEntity->edict()));
                    pBeam->SetStartAttachment(2);
                    pBeam->SetEndAttachment(0);
                    pBeam->RelinkBeam();
                    pBeam->SetBrightness(230);
                    pBeam->pev->effects &= ~EF_NODRAW;
                }

                UTIL_TempModel(Vector(0, 0, 0), g_vecZero, g_vecZero, m_iEf_Elec, 30, 0, 255, 0, 0, 210, kRenderTransAdd, pEntity, false, 0, 6, 16, 0);
#endif
                ++v8;
            }
        }
    }

    void CreateStunBeamEffect()
    {
        pev->nextthink = gpGlobals->time + 0.1s;

        int iStoredAmmo = m_iWeaponState;

        if (m_iWeaponState)
        {      
            if (!m_fInReload)
            {
                ClearEffect();
                pev->nextthink = gpGlobals->time;
                SetThink(nullptr);
                return;
            }
            else
            {
                if (m_flTimeNextCharge2 < gpGlobals->time)
                {
                    m_flTimeNextCharge2 = gpGlobals->time + 0.5s;
                    m_iWeaponState--;
                    SetChargeSkin();
                }

                PrimaryAttack_FindTargets();
                PrimaryAttack_InstantDamage();        
            }         
        }

        if (!m_iWeaponState)
        {
            //remove beam
            pev->nextthink = gpGlobals->time;
            if (iStoredAmmo != m_iWeaponState)
            {
                int flags = 0;
#ifdef CLIENT_WEAPONS
                flags = FEV_NOTHOST;
#else
                flags = 0;
#endif

                PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, 0, 0, 0, 0, 0, TRUE);
            }
            ClearEffect();
            SetThink(nullptr);
        }
    }

    void Reload(void) override
    {
        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            return;

        m_flTimeNextCharge = invalid_time_point;

        int iAnim;

        if (!m_iWeaponState)
            iAnim = ANIM_RELOADA;
        else if (m_iWeaponState < 8)
            iAnim = ANIM_RELOADB;
        else
            iAnim = ANIM_RELOADC;

        if (DefaultReload(30, iAnim, 3.2s))
        {
            if (m_iWeaponState)
            {
                SetThink(&CStunRifle::CreateStunBeamEffect);
                pev->nextthink = gpGlobals->time + 0.1s;
                m_flTimeNextCharge2 = gpGlobals->time + 0.5s;

                EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/stunrifle-2.wav", VOL_NORM, ATTN_NORM, 0, 94);


            }
           
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

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;

        ResetChargeTimer();

        if (!m_iWeaponState)
        {
            if (m_pPlayer->pev->weaponanim != ANIM_IDLEA)
                SendWeaponAnim(ANIM_IDLEA, UseDecrement() != FALSE);
        }
        else if (m_iWeaponState < 8)
        {
            if (m_pPlayer->pev->weaponanim != ANIM_IDLEB)
                SendWeaponAnim(ANIM_IDLEB, UseDecrement() != FALSE);
        }
        else
        {
            if (m_pPlayer->pev->weaponanim != ANIM_IDLEC)
                SendWeaponAnim(ANIM_IDLEC, UseDecrement() != FALSE);
        }
    }

    float GetDamageB() const
    {
        float flDamage = 5.0;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = 180.0f;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = 180.0f;
#endif
        return flDamage;
    }

};
LINK_ENTITY_TO_CLASS(weapon_stunrifle, CStunRifle)
}