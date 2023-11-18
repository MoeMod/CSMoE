/*
augexex.cpp
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
#include "soundent.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

#ifndef CLIENT_DLL
	class CAUGEXGrenade : public CGrenade
	{
	public:
		void Spawn(void);
		void Precache(void);

	public:
		void EXPORT FollowThink(void);
		void EXPORT IgniteThink(void);
		void EXPORT RocketTouch(CBaseEntity* pOther);

	public:

	public:
		int GetWeaponsId(void)
		{
			return WEAPON_AUGEX;
		}

		float GetRadius()
		{
			float flRadius = 3.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flRadius = 3.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flRadius = 3.0f;
			return flRadius * 39.37f;
		}

		float GetExplodeDamage()
		{
			float flDamage = 30.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 900.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 1000.0f;
			return flDamage;
		}

		float GetTouchDamage()
		{
			float flDamage = 50.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 50.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 1500.0f;
			return flDamage;
		}

		int m_iType;

	private:
		int m_iTrail;

		time_point_t m_flIgniteTime;
	};

	LINK_ENTITY_TO_CLASS(augex_grenade, CAUGEXGrenade)

	void CAUGEXGrenade::Spawn(void)
	{
		Precache();

		pev->movetype = MOVETYPE_TOSS;
		pev->solid = SOLID_TRIGGER;

		SET_MODEL(ENT(pev), "models/s_grenade.mdl");
		UTIL_SetSize(pev, Vector(-1.0, -1.0, -1.0), Vector(1.0, 1.0, 1.0));

		SetThink(&CAUGEXGrenade::IgniteThink);

		pev->angles.x -= 30.0;

		UTIL_MakeVectors(pev->angles);
		pev->angles.x = -(pev->angles.x + 30.0);

		pev->velocity = gpGlobals->v_forward * 300;
		pev->gravity = 0.4;
		pev->nextthink = gpGlobals->time + 0.1s;
		m_flIgniteTime = gpGlobals->time + 0.4s;
		pev->dmg = GetExplodeDamage();

		m_iType = 0;
	}

	void CAUGEXGrenade::FollowThink(void)
	{
		UTIL_MakeAimVectors(pev->angles);

		pev->angles = UTIL_VecToAngles(gpGlobals->v_forward);
		pev->nextthink = gpGlobals->time + 0.1s;

		if (!m_iType && m_flIgniteTime < gpGlobals->time)
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

			entvars_t* pevOwner;
			if (pev->owner)
				pevOwner = VARS(pev->owner);
			else
				pevOwner = NULL;

			RadiusDamage(pev, pevOwner, GetExplodeDamage(), CLASS_NONE, DMG_EXPLOSION, GetRadius());

			if (TheBots != NULL)
			{
				TheBots->OnEvent(EVENT_HE_GRENADE_EXPLODED, CBaseEntity::Instance(pev->owner));
			}

		
			for (int i = -1; i <= 1; i++)
			{
				Vector vecNewVelocity = UTIL_VecToAngles(pev->velocity);
				vecNewVelocity.y += float(i) * 12.0;

				Vector vecForward, vecRight, vecUp;
				AngleVectors(vecNewVelocity, vecForward, vecRight, vecUp);

				vecNewVelocity = vecForward * 300;
				CAUGEXGrenade* pGrenade = (CAUGEXGrenade*)CBaseEntity::Create("augex_grenade", pev->origin, pev->angles, pev->owner);

				if (pGrenade)
				{
					pGrenade->pev->velocity = vecNewVelocity;
					pGrenade->pev->owner = pev->owner;
					pGrenade->m_iTeam = m_iTeam;
					pGrenade->m_iType = 1;
				}
			}

			SetThink(nullptr);
			SetTouch(nullptr);

			UTIL_Remove(this);
		}
	}

	void CAUGEXGrenade::IgniteThink(void)
	{
		pev->movetype = MOVETYPE_BOUNCE;

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_BEAMFOLLOW);
		WRITE_SHORT(entindex());
		WRITE_SHORT(m_iTrail);
		WRITE_BYTE(5);
		WRITE_BYTE(1);
		WRITE_BYTE(224);
		WRITE_BYTE(224);
		WRITE_BYTE(255);
		WRITE_BYTE(255);
		MESSAGE_END();

		SetThink(&CAUGEXGrenade::FollowThink);
		SetTouch(&CAUGEXGrenade::RocketTouch);

		pev->nextthink = gpGlobals->time + 0.1s;
	}

	void CAUGEXGrenade::Precache(void)
	{
		m_iTrail = PRECACHE_MODEL("sprites/smoke.spr");

		PRECACHE_MODEL("models/s_grenade.mdl");
	}


	void CAUGEXGrenade::RocketTouch(CBaseEntity* pOther)
	{
		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_KILLBEAM);
		WRITE_SHORT(this->entindex());
		MESSAGE_END();

		ExplodeTouch3(pOther, GetRadius(), EXPTYPE_M79ROCKET, false);
	}
#endif

    class CAUGEX : public LinkWeaponTemplate< CAUGEX,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_AUG>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TReloadDefault,
            TGetDamageDefault,
            TPrimaryAttackRifle,
            TFireRifle,
			TRecoilKickBack,
            TWeaponIdleDefault
    >
    {
    public:
    static constexpr const auto &DefaultReloadTime = 3.1s;
    static constexpr const char *V_Model = "models/v_augex.mdl";
    static constexpr const char *P_Model = "models/p_augex.mdl";
    static constexpr const char *W_Model = "models/w_augex.mdl";
    static constexpr const char *EventFile = "events/augex.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_augex";
    static constexpr const char *AnimExtension = "rifle";
    static constexpr int MaxClip = 30;
	enum
	{
		ANIM_IDLE1 = 0,
		ANIM_RELOAD,
		ANIM_DRAW,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_SHOOT3,
		ANIM_SHOOT3_EMPTY,
	};
    static constexpr float PlayerMaxSpeed = 240;
    static constexpr float ArmorRatioModifier = 1.5f;
	static constexpr float PrimaryAttackWalkingMiniumSpeed = 140;
    static constexpr const auto &SpreadCalcNotOnGround = 0.4*A + 0.035;
    static constexpr const auto &SpreadCalcWalking = 0.07*A + 0.035;
    static constexpr const auto &SpreadCalcDefault = 0.02*A;

	static constexpr const auto& CycleTime = 0.0825s;
    static constexpr int DamageDefault = 32;
    static constexpr int DamageZB = 70;
    static constexpr int DamageZBS = 110;
    static constexpr const auto &AccuracyCalc = (N * N * N / 215.0) + 0.3;
    static constexpr float AccuracyDefault = 0.2;
    static constexpr float AccuracyMax = 1.0;
    static constexpr float RangeModifier = 0.96;
    static constexpr auto BulletType = BULLET_PLAYER_556MM;
    /*static constexpr*/ int Penetration = 2;
    KnockbackData KnockBack = { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f };

	KickBackData KickBackWalking = { 1.0f, 0.45f, 0.275f, 0.05f, 4.0f, 2.5f, 7 };
	KickBackData KickBackNotOnGround = { 1.25f, 0.45f, 0.22f, 0.18f, 5.5f, 4.0f, 5 };
	KickBackData KickBackDucking = { 0.575f, 0.325f, 0.2f, 0.011f, 3.25f, 2.0f, 8 };
	KickBackData KickBackDefault = { 0.625f, 0.375f, 0.25f, 0.0125f, 3.5f, 2.25f, 8 };
    

	int m_iDefaultAmmo2;

	void Spawn() override
	{
		m_iDefaultAmmo2 = 10;

		Base::Spawn();
	}

	void Precache(void) override
	{
		PRECACHE_MODEL("sprites/muzzleflash12.spr");

		Base::Precache();
	}

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "556Nato";
        p->iMaxAmmo1 = MAX_AMMO_556NATO;
		p->pszAmmo2 = "AUGEXRocket";
        p->iMaxAmmo2 = 10;
        p->iMaxClip = MaxClip;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_AUGEX;
        p->iFlags = 0;
        p->iWeight = AUG_WEIGHT;

        return 1;
    }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_AUG); }
#endif
    float GetArmorRatioModifier() override { return 1.5; }
	WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 3, 3, 140 }; }

	int ExtractAmmo(CBasePlayerWeapon* pWeapon) override
	{
		if (m_iDefaultAmmo2)
		{
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iDefaultAmmo2;
			m_iDefaultAmmo2 = 0;
		}
		return CBasePlayerWeapon::ExtractAmmo(pWeapon);
	}

    void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_bDelayFire = true;
        m_iShotsFired++;
        m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 215) + 0.3;

        if (m_flAccuracy > 1.0)
            m_flAccuracy = 1.0;

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

        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, Penetration, BULLET_PLAYER_556MM, GetDamage(), RangeModifier, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);

#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
        m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.6s;

		Recoil();
    }

    void SecondaryAttack(void)
    {
        if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] <= 0)
        {
            m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.4s;
            return;
        }
            
        m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;

        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
#ifndef CLIENT_DLL
        m_pPlayer->m_iExtraSoundTypes = bits_SOUND_DANGER;

        m_pPlayer->SetAnimation(PLAYER_ATTACK1);

        UTIL_MakeVectors(m_pPlayer->pev->v_angle);

		Vector vecVelocity = gpGlobals->v_forward * 800;

		CAUGEXGrenade* pGrenade = (CAUGEXGrenade*)CBaseEntity::Create("augex_grenade", m_pPlayer->GetGunPosition() - gpGlobals->v_up * 2.0, m_pPlayer->pev->v_angle, m_pPlayer->edict());

        if (pGrenade)
        {
			pGrenade->pev->velocity = vecVelocity;
			pGrenade->pev->owner = m_pPlayer->edict();
			pGrenade->m_iTeam = m_pPlayer->m_iTeam;
			pGrenade->m_iType = 0;

        }
#endif

		if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 0)
		{
			SendWeaponAnim(ANIM_SHOOT3_EMPTY);

			UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/augex_shoot_empty.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		}
		else
		{
			SendWeaponAnim(ANIM_SHOOT3);

			UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/augex_shoot3.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

		}

        m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 3.5s;
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.5s;

#ifndef CLIENT_DLL
        if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif

        m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 2, 5);
            
    }


};
LINK_ENTITY_TO_CLASS(weapon_augex, CAUGEX)
}