/*
thanatos1.cpp
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
    class CThanatos1 : public LinkWeaponTemplate< CThanatos1,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_DEAGLE>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TPrimaryAttackRifle,
            TFirePistol,
			TGetDamageDefault
    >
    {
    public:
    static constexpr const char *V_Model = "models/v_thanatos1.mdl";
    static constexpr const char *P_Model = "models/p_thanatos1.mdl";
    static constexpr const char *W_Model = "models/w_thanatos1.mdl";
    static constexpr const char *EventFile = "events/thanatos1.sc";
    static constexpr InventorySlotType ItemSlot = PISTOL_SLOT;
    static constexpr const char *ClassName = "weapon_thanatos1";
    static constexpr const char *AnimExtension = "onehanded";
    static constexpr int MaxClip = 7;

#define WPNSTATE_THANATOS1_MODEB	1

	enum
	{
		ANIM_IDLEA = 0,
        ANIM_IDLEB,
        ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_SHOOTB,
		ANIM_SHOOT_EMPTY,
		ANIM_SHOOT2_EMPTY,
		ANIM_RELOAD,
		ANIM_RELOAD_B,
		ANIM_CHANGEB,
		ANIM_DRAW,
		ANIM_DRAWB,
	};
    static constexpr float PlayerMaxSpeed = 250;
    static constexpr float ArmorRatioModifier = 1.5f;
    static constexpr const auto &SpreadCalcNotOnGround = 0.1*(1-A);
    static constexpr const auto &SpreadCalcWalking = 0.045*(1-A);
    static constexpr const auto& SpreadCalcDucking = 0.035*(1-A);
    static constexpr const auto &SpreadCalcDefault = 0.04*(1-A);

	static constexpr const auto& CycleTime = 0.225s;
    static constexpr int DamageDefault = 53;
    static constexpr int DamageZB = 83;
    static constexpr int DamageZBS = 196;

    static constexpr float AccuracyDefault = 0.95;
    /*static constexpr*/ int Penetration = 4;
    KnockbackData KnockBack = { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f };

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_DEAGLE); }
#endif
	float GetArmorRatioModifier() override { return 1.5; }
	WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED | ACCURACY_DUCK, 8, 3, 0 }; }

	int m_iExpIndex;

	int GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "50AE";
		p->iMaxAmmo1 = 70;
		p->pszAmmo2 = "thanatos1";
		p->iMaxAmmo2 = 1;
		p->iMaxClip = 7;
		p->iSlot = 1;
		p->iPosition = 2;
		p->iId = m_iId = WEAPON_THANATOS1;
		p->iFlags = 0;
		p->iWeight = DEAGLE_WEIGHT;

		return 1;
	}

	void Precache(void)
	{

		m_iExpIndex = PRECACHE_MODEL("sprites/ef_thanatos1.spr");

		PRECACHE_SOUND("weapons/thanatos1_shoot_b.wav");
		Base::Precache();
	}

	void SecondaryAttack(void)
	{
		if (!m_iWeaponState)
		{
			pev->iuser1 = 1;
			SendWeaponAnim(ANIM_CHANGEB, UseDecrement() != FALSE);

			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.8s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.8s;
		}
		else
		{
			m_iWeaponState = 0;
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/thanatos1_shoot_b.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);

			SendWeaponAnim(ANIM_SHOOTB, UseDecrement() != FALSE);

			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.0s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;
		}
	}

	BOOL Deploy()
	{
		m_flAccuracy = 0.9;
		pev->iuser1 = 0;

		return DefaultDeploy(V_Model, P_Model, m_iWeaponState & WPNSTATE_THANATOS1_MODEB ? ANIM_DRAWB : ANIM_DRAW, "onehanded", UseDecrement() != FALSE);
	}

	void ItemPostFrame() override
	{
		if (m_flNextSecondaryAttack <= 0.0s)
		{
			if (pev->iuser1)
			{
				pev->iuser1 = 0;
				m_iWeaponState |= WPNSTATE_THANATOS1_MODEB;

				m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 1;
			}
		}
		return CBasePlayerWeapon::ItemPostFrame();
	}

	void Reload(void)
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			return;

		if (DefaultReload(10, m_iWeaponState & WPNSTATE_THANATOS1_MODEB ? ANIM_RELOAD_B : ANIM_RELOAD, 2.3s))
		{
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
			m_flAccuracy = 0.9;
		}
	}

    void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
		m_iShotsFired++;

		if (m_iShotsFired > 1)
			return;

		if (m_flLastFire != invalid_time_point)
		{
			m_flAccuracy -= (0.4 - ((gpGlobals->time - m_flLastFire) / 1s)) * 0.35;

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

		m_iClip--;
		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;

		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 4096, 2, BULLET_PLAYER_50AE, GetDamage(), 0.81, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);

		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0.0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, int(m_pPlayer->pev->punchangle.x * 100), int(m_pPlayer->pev->punchangle.y * 100), (m_iWeaponState & WPNSTATE_THANATOS1_MODEB) ? TRUE : FALSE, m_iClip != 0);
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		m_pPlayer->pev->punchangle.x -= 2;
    }

	void WeaponIdle(void)
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;

		if (m_iWeaponState & WPNSTATE_THANATOS1_MODEB)
			SendWeaponAnim(ANIM_IDLEB, UseDecrement() != FALSE);
		else
			SendWeaponAnim(ANIM_IDLEA, UseDecrement() != FALSE);
	}

};
LINK_ENTITY_TO_CLASS(weapon_thanatos1, CThanatos1)
}