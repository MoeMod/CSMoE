/*
kingcobra.cpp
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
    class CKingcobra : public LinkWeaponTemplate< CKingcobra,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_DEAGLE>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TPrimaryAttackRifle,
			TWeaponIdleDefault,
            TFirePistol,
			TGetDamageDefault
    >
    {
    public:

    static constexpr const char *V_Model = "models/v_kingcobra.mdl";
    static constexpr const char *P_Model = "models/p_kingcobra.mdl";
    static constexpr const char *W_Model = "models/w_kingcobra.mdl";
    static constexpr const char *EventFile = "events/kingcobra.sc";
    static constexpr InventorySlotType ItemSlot = PISTOL_SLOT;
    static constexpr const char *ClassName = "weapon_kingcobra";
    static constexpr const char *AnimExtension = "onehanded";
    static constexpr int MaxClip = 7;

	enum
	{
		ANIM_IDLE1 = 0,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_SHOOT_EMPTY,
		ANIM_RELOAD,
		ANIM_DRAW,
	};
    static constexpr float PlayerMaxSpeed = 250;
    static constexpr float ArmorRatioModifier = 1.5f;
    static constexpr const auto &SpreadCalcNotOnGround = 1.4*(1-A);
    static constexpr const auto &SpreadCalcWalking = 0.3*(1-A);
    static constexpr const auto& SpreadCalcDucking = 0.05*(1-A);
    static constexpr const auto &SpreadCalcDefault = 0.125*(1-A);

	static constexpr const auto& CycleTime1 = 0.33s;
	static constexpr const auto& CycleTime2 = 0.95s;
	duration_t CycleTime = CycleTime1;
    static constexpr int DamageDefault = 52;
    static constexpr int DamageZB = 65;
    static constexpr int DamageZBS = 90;

    static constexpr float AccuracyDefault = 0.9;
    /*static constexpr*/ int Penetration = 2;
    KnockbackData KnockBack = { 400.0f, 300.0f, 350.0f, 150.0f, 0.6f };

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_DEAGLE); }
#endif
	float GetArmorRatioModifier() override { return 1.5; }
	WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED | ACCURACY_DUCK, 8, 3, 0 }; }

	bool m_bZoomed = false;

	KnockbackData GetKnockBackData() override
	{
		if (m_bZoomed)
			return { 600.0f, 450.0f, 350.0f, 200.0f, 0.5f };
		else
			return { 470.0f, 350.00f, 300.0f, 150.0f, 0.5f };
	}

	int GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "50ae";
		p->iMaxAmmo1 = 35;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = 7;
		p->iSlot = 1;
		p->iPosition = 2;
		p->iId = m_iId = WEAPON_KINGCOBRA;
		p->iFlags = 0;
		p->iWeight = DEAGLE_WEIGHT;

		return 1;
	}

	void SecondaryAttack(void)
	{
		switch (m_pPlayer->m_iFOV)
		{
		case 90: m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 44; break;
		default: m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 90; break;
		}

		m_pPlayer->ResetMaxSpeed();
		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/zoom.wav", 0.2, 2.4);
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;
	}

	float GetMaxSpeed(void)
	{
		if (m_pPlayer->m_iFOV == 90)
			return 250;

		return 220;
	}

	void PrimaryAttack()
	{
		if (m_pPlayer->pev->fov != 90)
		{
			CycleTime = CycleTime2;
		}
		else
		{
			CycleTime = CycleTime1;
			
		}
		Base::PrimaryAttack();
	}

    void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
		bool m_bZoomed = false;
		m_iShotsFired++;

		if (m_pPlayer->pev->fov != 90)
		{
			m_pPlayer->m_bResumeZoom = true;
			m_pPlayer->m_iLastZoom = m_pPlayer->m_iFOV;
			m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 90;
			m_bZoomed = true;
		}
		else // not zoomed, so bullets diverts more
			flSpread += 0.01;

		if (m_iShotsFired > 1)
			return;

		if (m_flLastFire != invalid_time_point)
		{
			if (m_pPlayer->pev->fov != 90)
			{
				m_flAccuracy -= (0.35 - ((gpGlobals->time - m_flLastFire) / 1s)) * 0.15;

				if (m_flAccuracy > 0.95)
					m_flAccuracy = 0.95;
				else if (m_flAccuracy < 0.55)
					m_flAccuracy = 0.55;
			}			
			else
			{
				m_flAccuracy -= (0.4 - ((gpGlobals->time - m_flLastFire) / 1s)) * 0.3;

				if (m_flAccuracy > 0.9)
					m_flAccuracy = 0.9;
				else if (m_flAccuracy < 0.5)
					m_flAccuracy = 0.5;
			}		
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
		Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 4096, 2, BULLET_PLAYER_50AE, m_bZoomed ? GetDamageB() : GetDamage(), m_bZoomed ? 0.81 : 0.8, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);

		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif
		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0.0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, int(m_pPlayer->pev->punchangle.x * 100), int(m_pPlayer->pev->punchangle.y * 100), m_iClip != 0, FALSE);
		
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
		m_pPlayer->pev->punchangle.x -= 2;
    }

	void Reload(void)
	{
		if (m_pPlayer->ammo_50ae <= 0)
			return;

		if (DefaultReload(MaxClip, ANIM_RELOAD, 4.0s))
		{
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
			if (m_pPlayer->pev->fov != 90)
			{
				SecondaryAttack();
			}
			m_flAccuracy = 0.9;
		}
	}

	float GetDamageB() const
	{
		float flDamage = 70.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 92.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 125.0f;
#endif
		return flDamage;
	}

};
LINK_ENTITY_TO_CLASS(weapon_kingcobra, CKingcobra)
}