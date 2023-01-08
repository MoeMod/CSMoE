/*
hk121ex.cpp
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
	class CHK121Rocket : public CGrenade
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
			return WEAPON_HK121EX;
		}

		float GetRadius()
		{
			float flRadius = 3.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flRadius = 4.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flRadius = 4.0f;
			return flRadius * 39.37f;
		}

		float GetExplodeDamage()
		{
			float flDamage = 80.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 300.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 300.0f;
			return flDamage;
		}

	private:
		int m_iTrail;

		time_point_t m_flIgniteTime;
	};

	LINK_ENTITY_TO_CLASS(hk121_rocket, CHK121Rocket)

	void CHK121Rocket::Spawn(void)
	{
		Precache();

		pev->movetype = MOVETYPE_BOUNCE;
		pev->solid = SOLID_BBOX;

		SET_MODEL(ENT(pev), "models/shell_svdex.mdl");
		UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));
		UTIL_SetOrigin(pev, pev->origin);

		SetTouch(&CHK121Rocket::RocketTouch);
		SetThink(&CHK121Rocket::IgniteThink);

		pev->angles.x -= 30.0;

		UTIL_MakeVectors(pev->angles);
		pev->angles.x = -(pev->angles.x + 30.0);

		pev->velocity = gpGlobals->v_forward * 300;
		pev->gravity = 0.5;
		pev->nextthink = gpGlobals->time + 0.1s;
		pev->dmg = GetExplodeDamage();
	}

	void CHK121Rocket::FollowThink(void)
	{
		UTIL_MakeAimVectors(pev->angles);

		pev->angles = UTIL_VecToAngles(gpGlobals->v_forward);
		pev->nextthink = gpGlobals->time + 0.1s;
	}

	void CHK121Rocket::IgniteThink(void)
	{
		pev->movetype = MOVETYPE_BOUNCE;

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_BEAMFOLLOW);
		WRITE_SHORT(entindex());
		WRITE_SHORT(m_iTrail);
		WRITE_BYTE(20);
		WRITE_BYTE(5);
		WRITE_BYTE(224);
		WRITE_BYTE(224);
		WRITE_BYTE(255);
		WRITE_BYTE(255);
		MESSAGE_END();

		m_flIgniteTime = gpGlobals->time;

		SetThink(&CM79Rocket::FollowThink);
		pev->nextthink = gpGlobals->time + 0.1s;
	}

	void CHK121Rocket::Precache(void)
	{
		m_iTrail = PRECACHE_MODEL("sprites/smoke.spr");

		PRECACHE_MODEL("models/shell_svdex.mdl");
	}


	void CHK121Rocket::RocketTouch(CBaseEntity* pOther)
	{
		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_KILLBEAM);
		WRITE_SHORT(this->entindex());
		MESSAGE_END();

		ExplodeTouch3(pOther, GetRadius(), EXPTYPE_HK121ROCKET, false);
	}
#endif

    class CHK121EX : public LinkWeaponTemplate< CHK121EX,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_M249>::template type,
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
    static constexpr const auto &DefaultReloadTime = 3.9s;
    static constexpr const char *V_Model = "models/v_hk121ex.mdl";
    static constexpr const char *P_Model = "models/p_hk121ex.mdl";
    static constexpr const char *W_Model = "models/w_hk121ex.mdl";
    static constexpr const char *EventFile = "events/hk121ex.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_hk121ex";
    static constexpr const char *AnimExtension = "m249";
    static constexpr int MaxClip = 150;
	enum
	{
		ANIM_IDLE1 = 0,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_RELOAD,
		ANIM_DRAW,
	};
    static constexpr float PlayerMaxSpeed = 220;
    static constexpr float ArmorRatioModifier = 1.5f;
	static constexpr float PrimaryAttackWalkingMiniumSpeed = 140;
    static constexpr const auto &SpreadCalcNotOnGround = 0.5*A + 0.05;
    static constexpr const auto &SpreadCalcWalking = 0.07*A + 0.05;
    static constexpr const auto &SpreadCalcDefault = 0.03*A;

	static constexpr const auto& CycleTime = 0.09s;
    static constexpr int DamageDefault = 33;
    static constexpr int DamageZB = 45;
    static constexpr int DamageZBS = 65;
    static constexpr const auto &AccuracyCalc = (N * N * N / 220.0) + 0.2;
    static constexpr float AccuracyDefault = 0.2;
    static constexpr float AccuracyMax = 0.7;
    static constexpr float RangeModifier = 0.98;
    static constexpr auto BulletType = BULLET_PLAYER_762MM;
    /*static constexpr*/ int Penetration = 1;
    KnockbackData KnockBack = { 450.0f, 350.0f, 300.0f, 200.0f, 0.6f };

	KickBackData KickBackWalking = { 1.3f, 0.4f, 0.3f, 0.06f, 4.0f, 2.0f, 8 };
	KickBackData KickBackNotOnGround = { 1.6f, 0.9f, 0.4f, 0.25f,6.0f, 4.0f, 7 };
	KickBackData KickBackDucking = { 0.65f, 0.2f, 0.2f, 0.02f, 2.0f, 1.2f, 10 };
	KickBackData KickBackDefault = { 0.7f, 0.3f, 0.23f, 0.02f,2.5f, 1.75f, 9 };
    

	int m_iDefaultAmmo2;

	void Spawn() override
	{
		m_iDefaultAmmo2 = 10;

		Base::Spawn();
	}

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "762NatoBox";
        p->iMaxAmmo1 = MAX_AMMO_556NATOBOX;
		p->pszAmmo2 = "HK121Rocket";
        p->iMaxAmmo2 = -1;
        p->iMaxClip = 150;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_HK121EX;
        p->iFlags = 0;
        p->iWeight = M249_WEIGHT;

        return 1;
    }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_762natobox" , 60 }; }
#endif
    float GetArmorRatioModifier() override { return 1.5; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 6, 3, 140 }; }

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
        m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 220) + 0.2;

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

        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_762MM, GetDamage(), 0.98, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

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

        Vector vecVelocity = gpGlobals->v_forward * 800 + Vector(0, 0, 100);

		CHK121Rocket* pRocket = (CHK121Rocket*)CBaseEntity::Create("hk121_rocket", m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 31 + gpGlobals->v_right * 5 + gpGlobals->v_up * -6, m_pPlayer->pev->v_angle, m_pPlayer->edict());

        if (pRocket)
        {
            pRocket->pev->velocity = vecVelocity;
            pRocket->pev->owner = m_pPlayer->edict();
            pRocket->m_iTeam = m_pPlayer->m_iTeam;
        }
#endif
		PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, 0.0, 0.0, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), TRUE, FALSE);

        m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.0s;
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;

#ifndef CLIENT_DLL
        if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif

        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 2, 5);
        else
            m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 2, 5);
            
    }


};
LINK_ENTITY_TO_CLASS(weapon_hk121ex, CHK121EX)
}