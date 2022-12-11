/*
wpn_anaconda.cpp @ codename LCSM
Copyright (C) 2019 Moemod Hymei

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

#ifndef CLIENT_DLL
#include "soundent.h"
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

using WeaponTemplate::Varibles::A;
using WeaponTemplate::Varibles::T;

struct CMalorian3516_WeaponData
{
public:
	static constexpr const auto &DefaultDeployTime = 0.8s;
	static constexpr InventorySlotType ItemSlot = PISTOL_SLOT;
	static constexpr const char *V_Model = "models/z4b/v_malorian3516.mdl";
	static constexpr const char *P_Model = "models/z4b/pw_malorian3516.mdl";
	static constexpr const char *W_Model = "models/z4b/pw_malorian3516.mdl";
	static constexpr const char *EventFile = "events/malorian3516.sc";
	static constexpr const char *ClassName = "z4b_malorian3516";
	static constexpr const char *AnimExtension = "onehanded";
	static constexpr int MaxClip = 10;
	static constexpr float ArmorRatioModifier = 1.5f;
	enum
	{
		ANIM_IDLE1,
		ANIM_SHOOT0,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_SHOOT3,
		ANIM_SHOOT4,
		ANIM_RELOAD0,
		ANIM_RELOAD0_EMPTY,
		ANIM_RELOAD1,
		ANIM_RELOAD1_EMPTY,
		ANIM_RELOAD2,
		ANIM_RELOAD2_EMPTY,
		ANIM_DRAW,
		ANIM_MELEE,
		ANIM_WALK,
	};
	static constexpr const auto & SpreadCalcNotOnGround = 1.5 * (1 - A);
	static constexpr const auto & SpreadCalcWalking = 0.25 * (1 - A);
	static constexpr const auto & SpreadCalcDucking = 0.115 * (1 - A);
	static constexpr const auto & SpreadCalcDefault = 0.13 * (1 - A);
	static constexpr const auto &CycleTime = 0.45s;
	static constexpr int DamageDefault = 61;
	static constexpr int DamageZB = 790;
	static constexpr int DamageZBS = 1350;
	static constexpr float AccuracyDefault = 0.9f;
	static constexpr const auto & AccuracyCalc = A - (0.4 - T) * 0.35;
	static constexpr float AccuracyMin = 0.0;
	static constexpr float AccuracyMax = 0.9;
	static constexpr float RangeModifier = 0.87;
	static constexpr auto BulletType = BULLET_PLAYER_50AE;
	static constexpr int Penetration = 4;
	KnockbackData KnockBack = { 450.0f, 350.0f, 400.0f, 300.0f, 0.3f };
	std::array<float, 3> RecoilPunchAngleDelta = { -3, 0, 0 };
};

class CMalorian3516 : public LinkWeaponTemplate< CMalorian3516,
		TGeneralData,
		BuildTGetItemInfoFromCSW<WEAPON_DEAGLE>::template type,
		TPrecacheEvent,
		TDeployDefault,
		TPrimaryAttackRifle,
		TFirePistol,
		TRecoilPunch,
		TGetDamageDefault
	>, public CMalorian3516_WeaponData
{
public:
	float GetArmorRatioModifier() override { return 1.5; }

	WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED | ACCURACY_DUCK, 8, 3, 0 }; }
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_14mm" , 60 }; }
#endif

	duration_t m_flNextRunIdle;
	duration_t m_flNextIdle;
	time_point_t m_flNextReload;

	KnockbackData GetKnockBackData() override
	{
		if (m_iWeaponState)
			return { 1100.f, 500.f, 700.f, 400.f, 0.9f };
		else
			return { 450.0f, 350.0f, 400.0f, 300.0f, 0.3f };
	}

	int GetItemInfo(ItemInfo* p) override
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "14mm";
		p->iMaxAmmo1 = 50;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = 10;
		p->iSlot = 1;
		p->iPosition = 1;
		p->iId = m_iId = WEAPON_Z4B_MALORIAN3516;
		p->iFlags = 0;
		p->iWeight = DEAGLE_WEIGHT;

		return 1;
	};

	void Precache() override
	{
		m_iShellId = PRECACHE_MODEL("models/shell762.mdl");
		PRECACHE_MODEL("sprites/flame_puff01.spr");
		PRECACHE_MODEL("sprites/smoke_ia.spr");
		PRECACHE_MODEL("sprites/muzzleflash60.spr");

		return Base::Precache();
	}

	void ResetRunIdle()
	{
		m_flNextIdle = m_flNextRunIdle = UTIL_WeaponTimeBase();
	}

	BOOL Deploy() override
	{
		m_flNextReload = invalid_time_point;
		ResetRunIdle();
		return Base::Deploy();
	}

	void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
	{
		m_iWeaponState = 0;
		Base::Fire(flSpread, flCycleTime, fUseAutoAim);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.8s;
	}

	void FireEvent(const Vector& vecDir)
	{
		int flags = 0;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif
		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, static_cast<int>(m_pPlayer->pev->punchangle.x * 100), static_cast<int>(m_pPlayer->pev->punchangle.y * 100), FALSE, m_iClip != 0);
		
		ResetRunIdle();
	}

	void SecondaryAttack() override
	{
		if (m_fInReload && m_flNextReload != invalid_time_point && m_flNextReload > gpGlobals->time)
		{
			m_fInReload = FALSE;
			m_flNextReload = invalid_time_point;
		}
		m_iWeaponState = 1;
		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
		Vector vecSrc = m_pPlayer->GetGunPosition();

#ifndef CLIENT_DLL
		Vector vecDir = KnifeAttack2(vecSrc, gpGlobals->v_forward, GetSecondaryAttackDamage(), 340.0f, 45.0f, DMG_BULLET, this->pev, m_pPlayer->pev);
#else
		Vector vecDir = gpGlobals->v_forward;
#endif

		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), TRUE, FALSE);

		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
#ifndef CLIENT_DLL
		m_pPlayer->m_iExtraSoundTypes = bits_SOUND_DANGER;
#endif
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.1s;

#ifndef CLIENT_DLL
		if (!m_iClip)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;

		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			KickBack(13.0, 5.0, 1.85, 0.55, 15.0, 5.7, 2.0);
		else if (m_pPlayer->pev->velocity.Length2D() > 0)
			KickBack(13.0, 2.25, 1.45, 0.7, 12.0, 10.0, 1);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			KickBack(9.0, 2.1, 1.25, 0.5, 15.0, 5.5, 1);
		else
			KickBack(13.0, 3.2, 1.5, 0.5, 15.0, 10.0, 2);

		ResetRunIdle();
	}

	void ItemPostFrame() override
	{
		int usableButtons = m_pPlayer->pev->button;

		if (m_fInReload && m_flNextReload != invalid_time_point && m_flNextReload < gpGlobals->time)
		{
			// complete the reload.
			int j = Q_min(MaxClip - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);
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

		if ((usableButtons & IN_ATTACK2) && m_flNextSecondaryAttack < UTIL_WeaponTimeBase())
		{
			SecondaryAttack();
			m_pPlayer->pev->button &= ~IN_ATTACK2;
			return;
		}

		if (m_fInReload)
			return;

		return CBasePlayerWeapon::ItemPostFrame();
	}

	void Reload() override
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			return;

		int iAnim = RANDOM_LONG(0,2);
		duration_t flDelay = m_iClip > 0 ? 2.6s : 3.6s;

		switch (iAnim)
		{
		case 0: iAnim = ANIM_RELOAD0; break;
		case 1: iAnim = ANIM_RELOAD1; break;
		case 2: iAnim = ANIM_RELOAD2; break;
		}

		if (m_iClip <= 0)
			iAnim += 1;

		if (DefaultReload(MaxClip, iAnim, flDelay, 1))
		{
#ifndef CLIENT_DLL         
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
			m_flNextReload = gpGlobals->time + (m_iClip > 0 ? 1.7s : 3.6s);
			ResetRunIdle();
			m_flAccuracy = AccuracyDefault;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flDelay;
		}
	}

	void WeaponIdle(void) override
	{
		ResetEmptySound();
		if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
		{
			if (m_pPlayer->pev->button & (IN_FORWARD | IN_MOVELEFT | IN_MOVERIGHT | IN_BACK))
			{
				if (m_flNextRunIdle <= UTIL_WeaponTimeBase())
				{
					SendWeaponAnim(ANIM_WALK, UseDecrement() != FALSE);

					m_flNextRunIdle = UTIL_WeaponTimeBase() + 0.8s;
				}

				m_flNextIdle = UTIL_WeaponTimeBase();
			}
			else
			{
				if (m_flNextIdle <= UTIL_WeaponTimeBase())
				{
					SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);

					m_flNextIdle = UTIL_WeaponTimeBase() + 20s;
				}

				m_flNextRunIdle = UTIL_WeaponTimeBase();
			}

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.1s;
		}

	}

	float GetSecondaryAttackDamage() const
	{
		float flDamage = 310;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 2350.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 2950.0f;
#endif
		return flDamage;
	}

	bool HasSecondaryAttack() override
	{
		return true;
	}
};
LINK_ENTITY_TO_CLASS(z4b_malorian3516, CMalorian3516)

}
