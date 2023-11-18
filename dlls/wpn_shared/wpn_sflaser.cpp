/*
sflaser.cpp
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
    class CSFLaser : public LinkWeaponTemplate< CSFLaser,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_AUG>::template type,
            TPrecacheEvent,
            TSecondaryAttackZoom,
			TPrimaryAttackRifle
    >
    {
    public:
    static constexpr const auto& DefaultReloadTime = 3.3s;
    static constexpr int ZoomFOV = 65;
    static constexpr const char *V_Model = "models/v_sflaser.mdl";
    static constexpr const char *P_Model = "models/p_sflaser.mdl";
    static constexpr const char *W_Model = "models/w_sflaser.mdl";
    static constexpr const char *EventFile = "events/sflaser.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_sflaser";
    static constexpr const char *AnimExtension = "rifle";
    static constexpr int MaxClip = 70;

#define WPNSTATE_SFLASER_MODEA 0
#define WPNSTATE_SFLASER_MODEB 1
#define WPNSTATE_SFLASER_MODEC 2

#define SFLASER_CHARGE_REQUIRED 15

	enum
	{
        ANIM_IDLEA,
        ANIM_IDLEB,
        ANIM_IDLEC,
        ANIM_SHOOTA,
        ANIM_SHOOTB,
        ANIM_SHOOTC,
        ANIM_SHOOT_EMPTYA,
        ANIM_SHOOT_EMPTYB,
        ANIM_SHOOT_EMPTYC,
        ANIM_SHOOT_ENDA,
        ANIM_SHOOT_ENDB,
        ANIM_SHOOT_ENDC,
        ANIM_RELOADA,
        ANIM_RELOADB,
        ANIM_RELOADC,
        ANIM_CHANGEA,
        ANIM_CHANGEB,
        ANIM_DRAWA,
        ANIM_DRAWB,
        ANIM_DRAWC,
	};
    static constexpr float PlayerMaxSpeed = 240;
    static constexpr float ArmorRatioModifier = 1.6f;
	static constexpr float PrimaryAttackWalkingMiniumSpeed = 140;
	static constexpr const auto& SpreadCalcNotOnGround = 0.4 * A + 0.04;
	static constexpr const auto& SpreadCalcWalking = 0.0275 * A;
	static constexpr const auto& SpreadCalcDefault = 0.03 * A;
	static constexpr const auto& SpreadCalcDucking = 0.08 * (1 - A);

	static constexpr const auto& CycleTime = 0.066s;
    static constexpr float AccuracyDefault = 0.25;
    static constexpr float AccuracyMax = 1.0;
    static constexpr float RangeModifier = 0.96;
    static constexpr auto BulletType = BULLET_PLAYER_556MM;
    /*static constexpr*/ int Penetration = 1;
    KnockbackData KnockBack = { 400.0f, 300.0f, 350.0f, 150.0f, 0.7f };

	CBeam* m_pBeam;
	int m_iChargeShotsFired;
	time_point_t m_flCheckCharge;
	time_point_t m_flNextReload;

	void Spawn()
	{
		m_iChargeShotsFired = 0;
		m_flCheckCharge = invalid_time_point;
		Base::Spawn();
	}

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "sflaser";
        p->iMaxAmmo1 = 100;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = MaxClip;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_SFLASER;
        p->iFlags = 0;
        p->iWeight = AUG_WEIGHT;

        return 1;
    }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_sflaser" , 100 }; }
#endif
    float GetArmorRatioModifier() override { return 1.6; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 3, 3, 140 }; }

	duration_t GetModeTime()
	{
		duration_t flTime;
		switch (m_iWeaponState)
		{
		case WPNSTATE_SFLASER_MODEA: flTime = 5.0s; break;
		case WPNSTATE_SFLASER_MODEB: flTime = 5.0s; break;
		case WPNSTATE_SFLASER_MODEC: flTime = 5.0s; break;
		default:
			flTime = 0.0s;
			break;
		}

		return flTime;
	}

    void Precache()
    {
        PRECACHE_MODEL("sprites/laserbeam.spr");

        Base::Precache();
    }

	void CreateEffect()
	{
#ifndef CLIENT_DLL
		m_pBeam = CBeam::BeamCreate("sprites/laserbeam.spr", 30);
		m_pBeam->SetColor(0, 255, 255);
		m_pBeam->SetBrightness(0);
		m_pBeam->pev->effects |= EF_NODRAW;
#endif
	}

	void ClearEffect()
	{
#ifndef CLIENT_DLL
		if (m_pBeam)
		{
			m_pBeam->SetBrightness(0);
			m_pBeam->pev->effects |= EF_NODRAW; // 0x80
		}
#endif
	}

	 void DestroyEffect()
    {
#ifndef CLIENT_DLL
		 if (m_pBeam)
		 {
			 UTIL_Remove(m_pBeam);
		 }
		 m_pBeam = NULL;
#endif
    }

	BOOL Deploy() override
	{
		m_flAccuracy = AccuracyDefault;
		m_bDelayFire = false;
		CreateEffect();

		int iAnim = ANIM_DRAWA + m_iWeaponState;

		return DefaultDeploy(V_Model, P_Model, iAnim, "carbine", UseDecrement() != FALSE);
	}

	void Reload(void)
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			return;

		int iAnim = ANIM_RELOADA + m_iWeaponState;

		if (DefaultReload(MaxClip, iAnim, DefaultReloadTime))
		{
			m_flAccuracy = AccuracyDefault;
			m_bDelayFire = false;
			StopIdleSound();
			ClearEffect();
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		}
	}

	void WeaponIdle(void)
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		if (m_pPlayer->pev->weaponanim >= ANIM_SHOOTA && m_pPlayer->pev->weaponanim <= ANIM_SHOOT_EMPTYC)
		{
			StopShootSound();

			ClearEffect();

			int iAnim;
			if (m_iWeaponState == WPNSTATE_SFLASER_MODEC)
				iAnim = ANIM_SHOOT_ENDC;
			else if (m_iWeaponState == WPNSTATE_SFLASER_MODEB)
				iAnim = ANIM_SHOOT_ENDB;
			else
				iAnim = ANIM_SHOOT_ENDA;

			SendWeaponAnim(iAnim, UseDecrement() != FALSE);

			m_flNextPrimaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.528s;
		}

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;

		int iAnim;
		if (m_iWeaponState == WPNSTATE_SFLASER_MODEC)
			iAnim = ANIM_IDLEC;
		else if (m_iWeaponState == WPNSTATE_SFLASER_MODEB)
			iAnim = ANIM_IDLEB;
		else
			iAnim = ANIM_IDLEA;

		SendWeaponAnim(iAnim, UseDecrement() != FALSE);
	}

	void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
	{
		m_bDelayFire = true;
		m_iShotsFired++;
		m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 220) + 0.3;

		if (m_flAccuracy > 1.0)
			m_flAccuracy = 1.0;

		if (m_iClip <= 0)
		{
			StopShootSound();

			if (m_fFireOnEmpty)
			{
				PlayEmptySound();

				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
			}

#ifndef CLIENT_DLL
			if (TheBots)
			{
				TheBots->OnEvent(EVENT_WEAPON_FIRED_ON_EMPTY, m_pPlayer);
			}
#endif
			return;
		}

		StopIdleSound();

		

		int iAnim;
		if (m_iWeaponState == WPNSTATE_SFLASER_MODEC)
			iAnim = ANIM_SHOOTC;
		else if (m_iWeaponState == WPNSTATE_SFLASER_MODEB)
			iAnim = ANIM_SHOOTB;
		else
			iAnim = ANIM_SHOOTA;

		if (m_pPlayer->pev->weaponanim != iAnim)
			SendWeaponAnim(iAnim, UseDecrement() != FALSE);

		m_iClip--;

		m_flNextPrimaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.066s;

		m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
		m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;

		UTIL_MakeVectors(m_pPlayer->pev->v_angle);

		Vector vecDir = m_pPlayer->FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, 0.0, 4096, Penetration, BulletType, GetDamage(), RangeModifier, m_pPlayer->pev, true, m_pPlayer->random_seed);

		if (m_iChargeShotsFired >= SFLASER_CHARGE_REQUIRED && m_iWeaponState < WPNSTATE_SFLASER_MODEC)
		{
			m_iWeaponState++;
			m_iChargeShotsFired = 0;
		}

		PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, int(m_pPlayer->pev->punchangle.x * 100), m_iWeaponState, m_iClip == 0, FALSE);

#ifndef CLIENT_DLL
		if (!m_iClip && !m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", 0, 0);
#endif
		if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			KickBack(0.625, 0.26, 0.25, 0.01, 3, 2, 6);
		else
			KickBack(0.7, 0.35, 0.3, 0.011, 3.5, 2.5, 7);
	}

#ifndef CLIENT_DLL
	void FireBullet3CallBack(int iPenetration, Vector vecDir, TraceResult* pTrace)
	{
		if (CBaseEntity* pEntity = gMultiDamage.pEntity)
		{
			if (pEntity->IsPlayer() || pEntity->Classify() == CLASS_PLAYER_ALLY)
			{
				if (g_pGameRules->PlayerRelationship(m_pPlayer, pEntity) != GR_TEAMMATE)
				{
					if (m_iWeaponState < WPNSTATE_SFLASER_MODEC)
						m_iChargeShotsFired++;

					if (m_iWeaponState)
						m_flCheckCharge = gpGlobals->time + GetModeTime();

				
				}
			}
		}

		int width;
		if (m_iWeaponState == WPNSTATE_SFLASER_MODEC)
			width = 15;
		else if (m_iWeaponState == WPNSTATE_SFLASER_MODEB)
			width = 10;
		else
			width = 5;

		if (m_pBeam)
		{		
			m_pBeam->PointEntInit(pTrace->vecEndPos, m_pPlayer->entindex());
			m_pBeam->SetEndAttachment(1);
			m_pBeam->SetWidth(width);
			m_pBeam->SetBrightness(230);
			m_pBeam->SetFlags(BEAM_FSINE);
			m_pBeam->pev->spawnflags |= SF_BEAM_TEMPORARY;	// Flag these to be destroyed on save/restore or level transition
			m_pBeam->pev->owner = m_pPlayer->edict();
			m_pBeam->pev->effects &= ~EF_NODRAW;
		}
	}
#endif

	void ItemPostFrame() override
	{
		if (m_flCheckCharge != invalid_time_point && m_flCheckCharge < gpGlobals->time)
		{
			if (m_iChargeShotsFired < SFLASER_CHARGE_REQUIRED)
			{
				m_flCheckCharge = invalid_time_point;
				m_iWeaponState--;
				m_iChargeShotsFired = 0;

				m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
				SendWeaponAnim(m_iWeaponState == WPNSTATE_SFLASER_MODEA ? ANIM_CHANGEA : ANIM_CHANGEB, UseDecrement() != FALSE);
			}
		}

		return CBasePlayerWeapon::ItemPostFrame();
	}

	void StopIdleSound(void)
	{
#ifndef CLIENT_DLL
		MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
		WRITE_BYTE(79);
		WRITE_SHORT(m_pPlayer->entindex());
		WRITE_SHORT(CHAN_AUTO);
		WRITE_STRING("weapons/sfpistol_idle.wav");
		MESSAGE_END();
#endif
	}

	void StopShootSound(void)
	{
#ifndef CLIENT_DLL
		MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL);
		WRITE_BYTE(79);
		WRITE_SHORT(m_pPlayer->entindex());
		WRITE_SHORT(CHAN_WEAPON);
		if (m_iWeaponState == WPNSTATE_SFLASER_MODEC)
			WRITE_STRING("weapons/sflaser-3.wav");
		else if (m_iWeaponState == WPNSTATE_SFLASER_MODEB)
			WRITE_STRING("weapons/sflaser-2.wav");
		else
			WRITE_STRING("weapons/sflaser-1.wav");
		MESSAGE_END();
#endif
	}

	void Holster(int skiplocal)
	{
		StopIdleSound();
		StopShootSound();
		ClearEffect();
		DestroyEffect();
		CBasePlayerWeapon::Holster(skiplocal);
	}


	float GetDamage() const
	{
		float flDamage = 25.0;
		if (m_iWeaponState == WPNSTATE_SFLASER_MODEC)
		{
			flDamage = 35.0;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 75.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 120.0f;
#endif
			
		}
		else if (m_iWeaponState == WPNSTATE_SFLASER_MODEB)
		{
			flDamage = 30.0;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 65.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 95.0f;
#endif
		}
		else
		{
			flDamage = 25.0;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 55.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 70.0f;
#endif
		}
		return flDamage;
	}

};
LINK_ENTITY_TO_CLASS(weapon_sflaser, CSFLaser)
}