/*
y23s2buffaug.cpp
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
namespace sv {

#define SOLIDER_TIME	 5.3s

	class CY23S2BuffAugSoldier : public CBaseAnimating
	{
	public:
		void Spawn(void);
		int GetWeaponsId(void) { return WEAPON_Y23S2BUFFAUG; }
		float GetArmorRatioModifier() { return 1.0; }

		void Precache()
		{
			PRECACHE_MODEL("models/y23s2buffaug_soldier.mdl");

			PRECACHE_SOUND("weapons/y23s2buffaug-2_2.wav");
		};

		KnockbackData GetKnockBackData() override
		{
            if(pev->iuser1)
			    return { -1.0f,  -1.0f,  -1.0f,  -1.0f, 0.5f };
            else
                return { 350.0f, 250.0f, 300.0f, 100.0f, 0.5f };
		}


	public:
		void EXPORT OnThink(void);
        void StartFadeOut();
        void FadeOut();

		bool Attack(bool bAppear);
		void AttackSound();  
		void Remove()
		{
			MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, this->edict());
			WRITE_BYTE(79);
			WRITE_SHORT(this->entindex());
			WRITE_SHORT(CHAN_ITEM);
			WRITE_STRING("weapons/y23s2buffaug-2_2.wav");
			MESSAGE_END();

			SetThink(NULL);
			SetTouch(NULL);

			UTIL_Remove(this);
		}

        time_point_t m_flNextIdle;
		time_point_t m_flDamageTime;
		time_point_t m_fLaunchTime;
		Vector m_vecAngle;

	private:
		float GetRadiusDamage() const
		{
			float flDamage = 4.0f;

			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 629.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 1260.0f;

			return flDamage;
		}

		float GetAppearDamage() const
		{
			float flDamage = 15.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 4600.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 10300.0f;

			return flDamage;
		}

        float GetRadius() const
        {
            float flRadius = 78.74f;

            if (g_pModRunning->DamageTrack() == DT_ZB)
                flRadius = 157.48f;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flRadius = 157.48f;

            return flRadius;
        }
	};

	LINK_ENTITY_TO_CLASS(y23s2buffaug_soldier, CY23S2BuffAugSoldier)


    void CY23S2BuffAugSoldier::StartFadeOut()
    {
        pev->nextthink = gpGlobals->time + 0.01s;

        SetThink(&CY23S2BuffAugSoldier::FadeOut);
    }

    void CY23S2BuffAugSoldier::FadeOut()
    {
        if (pev->renderamt > 10)
        {
            pev->renderamt -= 10.0f;
            pev->nextthink = gpGlobals->time + 0.01s;
        }
        else
        {
            pev->renderamt = 0.0f;
            pev->nextthink = gpGlobals->time + 0.01s;
            SetThink(&CY23S2BuffAugSoldier::SUB_Remove);
        }
    }

	void CY23S2BuffAugSoldier::OnThink(void)
	{
        pev->nextthink = gpGlobals->time + 0.1s;

		if (pev->owner)
		{
			CBasePlayer* pOwner = (CBasePlayer*)CBaseEntity::Instance(pev->owner);

			if (pOwner)
			{
				if (pOwner->IsPlayer() && (pOwner->m_bIsZombie || !pOwner->IsAlive()))
					Remove();
			}
		}
		else
			Remove();

		if (gpGlobals->time > m_fLaunchTime + SOLIDER_TIME)
		{
            SetThink(&CY23S2BuffAugSoldier::StartFadeOut);

			return;
		}

		if (m_flDamageTime < gpGlobals->time)
		{
			m_flDamageTime = gpGlobals->time + 0.2s;
            if (Attack(false))
            {
                if (m_flNextIdle < gpGlobals->time)
                {
                    AttackSound();
                    pev->sequence = 2;
                    ResetSequenceInfo();      

                    m_flNextIdle = gpGlobals->time + 0.3s;
                }
            }
            else
            {
                if (m_flNextIdle < gpGlobals->time)
                {
                    pev->sequence = 0;

                    m_flNextIdle = gpGlobals->time + 0.3s;
                }
            }   
		}
	}

	void CY23S2BuffAugSoldier::AttackSound()
	{
		EMIT_SOUND_DYN(this->edict(), CHAN_WEAPON, "weapons/y23s2buffaug-2_2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
	}

	bool CY23S2BuffAugSoldier::Attack(bool bAppear)
	{
        bool bResult = false;
		TraceResult tr;

        pev->iuser1 = bAppear ? 0 : 1;
		float flDamage = bAppear ? GetAppearDamage() : GetRadiusDamage();
		CBaseEntity* pList[100];
		Vector vecStart;

		vecStart = pev->origin + Vector(0.0, 0.0, 5.0);

		const float flWidth = GetRadius();
		const float flLength = 4 * 39.37 / 2.0;
		const float flHeight = 4 * 39.37 / 2.0;

		int iCount = UTIL_EntitiesInRotatedBox(pList, 100, vecStart, m_vecAngle, flLength, flWidth, flHeight, FL_CLIENT | FL_MONSTER);

		for (int i = 0; i < iCount; i++)
		{
			CBaseEntity* pEntity = pList[i];

			if (pEntity == this)
				continue;

			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			if (pEntity->edict() == pev->owner)
				continue;

			UTIL_TraceLine(vecStart, pEntity->BodyTarget(vecStart), dont_ignore_monsters, edict(), &tr);

			if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
				continue;

            pEntity->m_LastHitGroup = HITGROUP_CHEST;
			pEntity->TakeDamage(pev, VARS(pev->owner), flDamage, DMG_GENERIC);

			if (pEntity->IsPlayer())
			{
				CBasePlayer* player = (CBasePlayer*)pEntity;

				if (player->m_bIsZombie)
				{
                    Vector vecDirection = (pEntity->pev->origin - vecStart).Normalize();
                    UTIL_AddKickRateBoost(pEntity, vecDirection.Make2D(), 1.0, 1200.0, 120.0);
				}
			}
            
            bResult = true;
		}

        return bResult;
	}

	void CY23S2BuffAugSoldier::Spawn(void)
	{
		pev->movetype = MOVETYPE_TOSS;
		pev->solid = SOLID_NOT;
		pev->friction = 0.0;
		pev->takedamage = DAMAGE_NO;
        pev->gravity = 1.0;
        pev->rendermode = kRenderTransTexture;
        pev->renderamt = 180.0f;

		Precache();

		SET_MODEL(ENT(pev), "models/y23s2buffaug_soldier.mdl");
		UTIL_SetSize(pev, Vector(0.0, 0.0, 0.0), Vector(0.0, 0.0, 0.0));
		UTIL_SetOrigin(pev, pev->origin);

		pev->frame = 0;

		ResetSequenceInfo();

        pev->sequence = 1;

		SetThink(&CY23S2BuffAugSoldier::OnThink);
		pev->nextthink = gpGlobals->time + 0.1s;

		SetTouch(NULL);
		m_fLaunchTime = gpGlobals->time;
		m_flDamageTime = gpGlobals->time;
	}
}
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif
    class CY23S2BuffAug : public LinkWeaponTemplate< CY23S2BuffAug,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_AUG>::template type,
            TPrecacheEvent,
            TGetDamageDefault
    >
    {
    public:

    static constexpr const char *V_Model = "models/v_y23s2buffaug.mdl";
    static constexpr const char *P_Model = "models/p_y23s2buffaug.mdl";
    static constexpr const char *W_Model = "models/w_y23s2buffaug.mdl";
    static constexpr const char *EventFile = "events/y23s2buffaug.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_y23s2buffaug";
    static constexpr const char *AnimExtension = "rifle";
    static constexpr int MaxClip = 50;
	enum
	{
        ANIM_IDLE1 = 0,
        ANIM_RELOAD1,
        ANIM_DRAW1,
        ANIM_SHOOT1,
        ANIM_SHOOT2,
        ANIM_SHOOT3,
        ANIM_CHANGE1,
        ANIM_IDLE2,
        ANIM_RELOAD2,
        ANIM_DRAW2,
        ANIM_SHOOT21,
        ANIM_SHOOT22,
        ANIM_SHOOT23,
        ANIM_CHANGE2,
        ANIM_SKILL,
	};
    static constexpr float PlayerMaxSpeed = 240;

    static constexpr int DamageDefault = 20;
    static constexpr int DamageZB = 152;
    static constexpr int DamageZBS = 389;

    KnockbackData KnockBack = { 350.0f, 250.0f, 300.0f, 100.0f, 0.5f };

#define Y23S2BUFFAUG_MAXAMMO 80
#define Y23S2BUFFAUG_AMMOCOST 40

    int iSkillModelIndex;

    void Spawn() override
    {

        Base::Spawn();
    }

    void Precache()
    {
        PRECACHE_MODEL("sprites/ef_y23s2buffaug_a.spr");
        PRECACHE_MODEL("sprites/ef_y23s2buffaug_b.spr");
        iSkillModelIndex = PRECACHE_MODEL("sprites/ef_y23s2buffaug_skill.spr");

        PRECACHE_MODEL("sprites/muzzleflash314.spr");

        PRECACHE_SOUND("weapons/y23s2buffaug-2_1.wav");
        PRECACHE_SOUND("weapons/y23s2buffaug-2_2.wav");
        PRECACHE_SOUND("weapons/y23s2buffaug_idle1.wav");
        PRECACHE_SOUND("weapons/y23s2buffaug_idle2.wav");
        Base::Precache();
    }

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "buffaug_bullet";
        p->iMaxAmmo1 = 90;
        p->pszAmmo2 = "y23s2buffaug";
        p->iMaxAmmo2 = Y23S2BUFFAUG_MAXAMMO;
        p->iMaxClip = 50;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_Y23S2BUFFAUG;
        p->iFlags = 0;
        p->iWeight = AUG_WEIGHT;

        return 1;
    }

    int ExtractAmmo(CBasePlayerWeapon* pWeapon)
    {
        m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 0;
        return CBasePlayerWeapon::ExtractAmmo(pWeapon);
    }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_buffaug_bullet" , 200 }; }
#endif
    float GetArmorRatioModifier() override { return 1.6; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 3, 3, 140 }; }

    BOOL Deploy() override
    {
        m_flAccuracy = 0.2;
        m_iShotsFired = 0;

        return DefaultDeploy("models/v_y23s2buffaug.mdl", "models/p_y23s2buffaug.mdl", m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] < Y23S2BUFFAUG_AMMOCOST ? ANIM_DRAW1 : ANIM_DRAW2, "carbine", UseDecrement() != FALSE);
    }

    void PrimaryAttack() override
    {
        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            Fire(0.02 + (0.2) * m_flAccuracy, 0.097s, FALSE);
        else if (m_pPlayer->pev->velocity.Length2D() > 140)
            Fire(0.02 + (0.01) * m_flAccuracy, 0.097s, FALSE);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            Fire((0.01) * m_flAccuracy, 0.097s, FALSE);
        else
            Fire((0.02) * m_flAccuracy, 0.097s, FALSE);
    }

    void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_bDelayFire = true;
        m_iShotsFired++;
        m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 210.0) + 0.2;

        if (m_flAccuracy > 0.7)
            m_flAccuracy = 0.7;

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

        m_iClip--;
        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

        StopIdleSound();

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_556MM, GetDamage(), 0.97, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

        if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] < Y23S2BUFFAUG_MAXAMMO)
            m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]++;

#ifndef CLIENT_DLL
        PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, static_cast<int>(m_pPlayer->pev->punchangle.x * 100), static_cast<int>(m_pPlayer->pev->punchangle.y * 100), m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType], FALSE);
#endif
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.9s;

        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            KickBack(0.3, 0.2, 0.2, 0.02, 2.9, 1.9, 7);
        else if (m_pPlayer->pev->velocity.Length2D() > 0)
            KickBack(0.2, 0.1, 0.1, 0.01, 1.9, 0.9, 9);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            KickBack(0.16, 0.06, 0.06, 0.006, 1.7, 0.7, 10);
        else
            KickBack(0.18, 0.08, 0.08, 0.008, 1.8, 0.8, 10);
    }

    void SecondaryAttack(void)
    {
        if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] < Y23S2BUFFAUG_AMMOCOST)
            return;

        m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] -= Y23S2BUFFAUG_AMMOCOST;

        EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/y23s2buffaug-2_1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
#ifndef CLIENT_DLL
        SendWeaponAnim(m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] < Y23S2BUFFAUG_AMMOCOST ? ANIM_CHANGE2 : ANIM_SKILL, UseDecrement() != FALSE);

        Vector vecAngle = m_pPlayer->pev->v_angle;
        vecAngle.x = 0;

        UTIL_MakeVectors(vecAngle);

        Vector vecForward = gpGlobals->v_forward;
        Vector vecRight = gpGlobals->v_right;

        Vector VecOrigin = m_pPlayer->pev->origin + vecForward * 39.37 * 3;

        CY23S2BuffAugSoldier* pSoldier = static_cast<CY23S2BuffAugSoldier*>(CBaseEntity::Create("y23s2buffaug_soldier", VecOrigin, vecAngle, m_pPlayer->edict()));
        if (pSoldier)
        {
            pSoldier->m_iTeam = m_pPlayer->m_iTeam;

            pSoldier->m_vecAngle = vecAngle;

            pSoldier->Attack(true);
           
        }

        MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
        WRITE_BYTE(TE_TEMPSPRITE);
        WRITE_COORD(VecOrigin.x);
        WRITE_COORD(VecOrigin.y);
        WRITE_COORD(VecOrigin.z - 20.0);
        WRITE_SHORT(iSkillModelIndex);
        WRITE_BYTE(5);
        WRITE_BYTE(255);
        WRITE_BYTE(30);
        MESSAGE_END();

        VecOrigin += vecRight * 50;

        MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
        WRITE_BYTE(TE_TEMPSPRITE);
        WRITE_COORD(VecOrigin.x);
        WRITE_COORD(VecOrigin.y);
        WRITE_COORD(VecOrigin.z - 20.0);
        WRITE_SHORT(iSkillModelIndex);
        WRITE_BYTE(5);
        WRITE_BYTE(255);
        WRITE_BYTE(30);
        MESSAGE_END();

        VecOrigin -= vecRight * 100;

        MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
        WRITE_BYTE(TE_TEMPSPRITE);
        WRITE_COORD(VecOrigin.x);
        WRITE_COORD(VecOrigin.y);
        WRITE_COORD(VecOrigin.z - 20.0);
        WRITE_SHORT(iSkillModelIndex);
        WRITE_BYTE(5);
        WRITE_BYTE(255);
        WRITE_BYTE(30);
        MESSAGE_END();

#endif

        m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5s;
        m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.6s;
    }

    void Reload() override
    {
        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            return;

        if (DefaultReload(MaxClip, m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] < Y23S2BUFFAUG_AMMOCOST ? ANIM_RELOAD1 : ANIM_RELOAD2, 1.7s))
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

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
        SendWeaponAnim(m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] < Y23S2BUFFAUG_AMMOCOST ? ANIM_IDLE1 : ANIM_IDLE2, UseDecrement() != FALSE);
    }

    void StopIdleSound(void)
    {
#ifndef CLIENT_DLL
        StopSound("weapons/y23s2buffaug_idle1.wav");
        StopSound("weapons/y23s2buffaug_idle2.wav");
#endif
    }

    void StopSound(const char* s)
    {
#ifndef CLIENT_DLL
        MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
        WRITE_BYTE(79);
        WRITE_SHORT(m_pPlayer->entindex());
        WRITE_SHORT(CHAN_AUTO);
        WRITE_STRING(s);
        MESSAGE_END();
#endif
    }
};
LINK_ENTITY_TO_CLASS(weapon_y23s2buffaug, CY23S2BuffAug)
}