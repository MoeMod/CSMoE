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

#include <array>
#include "wpn_knife.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

using WeaponTemplate::Varibles::A;
using WeaponTemplate::Varibles::T;

struct CZ4B_DeagleEL_WeaponData
{
public:
	static constexpr const auto &DefaultReloadTime = 1.5s;
	static constexpr const auto &DefaultDeployTime = 0s;
	static constexpr InventorySlotType ItemSlot = PISTOL_SLOT;
	static constexpr const char *V_Model = "models/z4b/v_deagleel.mdl";
	static constexpr const char *P_Model = "models/z4b/pw_deagleel.mdl";
	static constexpr const char* P_Model_2 = "models/z4b/pw_deagleel_1.mdl";
	static constexpr const char *W_Model = "models/z4b/pw_deagleel.mdl";
	static constexpr const char *EventFile = "events/deagleel.sc";
	static constexpr const char *ClassName = "z4b_deagleel";
	static constexpr const char *AnimExtension = "onehanded";
	static constexpr int MaxClip = 40;
	static constexpr float ArmorRatioModifier = 1.6f;
	enum
	{
		ANIM_IDLE1,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_SHOOT3,
		ANIM_RELOAD,
		ANIM_DRAW,
		ANIM_RUN,
		ANIM_STAB,
		ANIM_CHANGEB,
		ANIM_SHOOTB_R1,
		ANIM_SHOOTB_R2,
		ANIM_SHOOTB_R3,
		ANIM_SHOOTB_R4,
		ANIM_SHOOTB_L1,
		ANIM_SHOOTB_L2,
		ANIM_SHOOTB_L3,
		ANIM_SHOOTB_L4,
	};
	static constexpr const auto & SpreadCalcNotOnGround = 1.5 * (1 - A);
	static constexpr const auto & SpreadCalcWalking = 0.25 * (1 - A);
	static constexpr const auto & SpreadCalcDucking = 0.115 * (1 - A);
	static constexpr const auto & SpreadCalcDefault = 0.13 * (1 - A);
	static constexpr const auto &CycleTime = 0.3s;
	static constexpr int DamageDefault = 57;
	static constexpr int DamageZB = 80;
	static constexpr int DamageZBS = 80;
	static constexpr float AccuracyDefault = 0.95f;
	
	KnockbackData KnockBack = { 400, 300, 300, 200, 0.7f };
	std::array<float, 3> RecoilPunchAngleDelta = { -2, 0, 0 };
};

class CZ4B_DeagleEL : public LinkWeaponTemplate< CZ4B_DeagleEL,
		TGeneralData,
		BuildTGetItemInfoFromCSW<WEAPON_DEAGLE>::template type,
		TPrecacheEvent,
		TDeployDefault,
		TReloadDefault,
		TPrimaryAttackRifle,
		TFirePistol,
		TGetDamageDefault
	>, public CZ4B_DeagleEL_WeaponData
{
public:

#define WPNSTATE_DUALKRISSHERO_LEFT	(1<<0)

	float GetArmorRatioModifier() override { return 1.5; }

	WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED | ACCURACY_DUCK, 8, 3, 0 }; }
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_DEAGLE); }
#endif

	int m_iModeState;
	duration_t m_flNextRunIdle;
	duration_t m_flNextIdle;
	time_point_t m_flTimeCheckMode;
	int m_usFireLeft;
	int m_usFireRight;
	int m_iAnimLeft;
	int m_iAnimRight;

	int GetItemInfo(ItemInfo* p) override
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "50AE";
		p->iMaxAmmo1 = 35;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = 40;
		p->iSlot = 1;
		p->iPosition = 1;
		p->iId = m_iId = WEAPON_Z4B_DEAGLEEL;
		p->iFlags = 0;
		p->iWeight = DEAGLE_WEIGHT;

		return 1;
	};

	void Precache() override
	{

		PRECACHE_SOUND("weapons/mastercombat_slash.wav");
	
		m_usFireLeft = PRECACHE_EVENT(1, "events/deagleel_left.sc");
		m_usFireRight = PRECACHE_EVENT(1, "events/deagleel_right.sc");

		return Base::Precache();
	}
	
	void PrimaryAttack() override
	{
		if (!m_iModeState)
		{
			if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
				Fire((0.08) * m_flAccuracy, 0.2s, FALSE);
			else if (m_pPlayer->pev->velocity.Length2D() > 0)
				Fire((0.04) * m_flAccuracy, 0.2s, FALSE);
			else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
				Fire((0.02) * m_flAccuracy, 0.2s, FALSE);
			else
				Fire((0.03) * m_flAccuracy, 0.2s, FALSE);

			if (m_iClip)
			{
				m_flTimeCheckMode = gpGlobals->time + 0.2s;
				m_iModeState = 1;
			}	
		}

		if (m_flTimeCheckMode < gpGlobals->time)
		{
			switch (m_iModeState)
			{
			case 1:
			{
				m_pPlayer->pev->weaponmodel = MAKE_STRING(P_Model_2);
				Q_strcpy(m_pPlayer->m_szAnimExtention, m_pPlayer->GetAnimExtDualPistols());

				SendWeaponAnim(ANIM_CHANGEB);

				UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "CK_WPN/WEAPON/DEAGLE_EL/DEAGLE_EL_StanceChange.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM, 1);

				m_iAnimLeft = m_iAnimRight = 0;
				m_flTimeCheckMode = gpGlobals->time + 0.55s;
				m_iModeState = 2;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.55s;
				m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.55s;
				break;
			}

			case 2:
			{
				if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
					DualDeagleFire((0.08) * m_flAccuracy, 0.1s, FALSE);
				else if (m_pPlayer->pev->velocity.Length2D() > 0)
					DualDeagleFire((0.04) * m_flAccuracy, 0.1s, FALSE);
				else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
					DualDeagleFire((0.02) * m_flAccuracy, 0.1s, FALSE);
				else
					DualDeagleFire((0.03) * m_flAccuracy, 0.1s, FALSE);
				break;
			}
			}
		}
	}

	void DualDeagleFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
	{
		m_bDelayFire = true;
		m_iShotsFired++;
		m_flAccuracy = (m_iShotsFired * m_iShotsFired * m_iShotsFired) / 220.0 + 0.25;

		if (m_flAccuracy > 1.0)
			m_flAccuracy = 1.0;

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

		unsigned short eventindex;
		int anim;
		if (m_iWeaponState & WPNSTATE_DUALKRISSHERO_LEFT)
		{
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
			m_iWeaponState &= ~WPNSTATE_DUALKRISSHERO_LEFT;
			eventindex = m_usFireLeft;
			anim = m_iAnimLeft;

			if (++m_iAnimLeft > 3)
				m_iAnimLeft = 0;
		}
		else
		{
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_ATTACK2);
#endif
			m_iWeaponState |= WPNSTATE_DUALKRISSHERO_LEFT;
			eventindex = m_usFireRight;
			anim = m_iAnimRight;

			if (++m_iAnimRight > 3)
				m_iAnimRight = 0;
		}

		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
		Vector vecDir = m_pPlayer->FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192.0, 1, BULLET_PLAYER_45ACP, GetDamage(), 0.975, m_pPlayer->pev, false, m_pPlayer->random_seed);

		int flags;

#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), eventindex, 0.0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, int(m_pPlayer->pev->punchangle.x * 100), int(m_pPlayer->pev->punchangle.y * 100), TRUE, anim);

		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

		m_flTimeCheckMode = gpGlobals->time + flCycleTime;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", 0, 0);
#endif
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;

		ResetRunIdle();

		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			KickBack(1.0, 0.45, 0.25, 0.075, 4.5, 3.0, 6);
		else if (m_pPlayer->pev->velocity.Length2D() > 0)
			KickBack(0.5, 0.25, 0.0825, 0.0225, 3.5, 2.25, 7);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			KickBack(0.3, 0.15, 0.05, 0.015, 2.5, 0.8, 8);
		else
			KickBack(0.4, 0.2, 0.0625, 0.0175, 3.0, 1.0, 7);

	}

	void SecondaryAttack() override
	{
		Q_strcpy(m_pPlayer->m_szAnimExtention, "m16a1ep");
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK2);
#endif
		SendWeaponAnim(ANIM_STAB, UseDecrement() != FALSE);

		ResetRunIdle();

		SetThink(&CZ4B_DeagleEL::DelaySecondaryAttack);
		pev->nextthink = gpGlobals->time + 0.233s;

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.6s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.6s;
	}

	float GetSecondaryAttackDamage() const
	{
		float flDamage = 105;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 3160.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 1250.0f;
#endif
		return flDamage;
	}

	float GetDeployAttackDamage() const
	{
		float flDamage = 80;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 1080.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 660.0f;
#endif
		return flDamage;
	}


	void WeaponIdle(void)
	{
		ResetEmptySound();

		if (m_iModeState > 1 && !(m_pPlayer->pev->button & IN_ATTACK))
		{
			OnAttackReleased();
		}

		if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
		{
			if (m_pPlayer->pev->button & (IN_FORWARD | IN_MOVELEFT | IN_MOVERIGHT | IN_BACK))
			{
				if (m_flNextRunIdle <= UTIL_WeaponTimeBase())
				{
					SendWeaponAnim(ANIM_RUN, UseDecrement() != FALSE);

					m_flNextRunIdle = UTIL_WeaponTimeBase() + 0.87s;
				}

				m_flNextIdle = UTIL_WeaponTimeBase();
			}
			else
			{
				if (m_flNextIdle <= UTIL_WeaponTimeBase())
				{
					SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);

					m_flNextIdle = UTIL_WeaponTimeBase() + 26s;
				}

				m_flNextRunIdle = UTIL_WeaponTimeBase();
			}

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.1s;
		}
	}

	void ResetRunIdle()
	{
		m_flNextIdle = m_flNextRunIdle = UTIL_WeaponTimeBase();
	}


	void OnAttackReleased()
	{
		m_pPlayer->pev->weaponmodel = MAKE_STRING(P_Model);
		Q_strcpy(m_pPlayer->m_szAnimExtention, AnimExtension);

		SendWeaponAnim(ANIM_IDLE1);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase();
		m_iModeState = 0;
	}

	void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
	{
		m_iShotsFired++;

		if (m_iShotsFired > 1)
			return;

		if (m_flLastFire != invalid_time_point)
		{
			m_flAccuracy -= (0.375 - ((gpGlobals->time - m_flLastFire) / 1s)) * 0.315;

			if (m_flAccuracy > 0.95)
				m_flAccuracy = 0.95;
			else if (m_flAccuracy < 0.57)
				m_flAccuracy = 0.57;
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

#ifdef CLIENT_DLL
		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0.0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, int(m_pPlayer->pev->punchangle.x * 100), int(m_pPlayer->pev->punchangle.y * 100), (m_iModeState == 2) ? TRUE : FALSE, FALSE);
#else
		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0.0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, int(m_pPlayer->pev->punchangle.x * 100), int(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);
#endif
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

		ResetRunIdle();

#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif

		m_pPlayer->pev->punchangle.x -= 2;
	}

	BOOL Deploy() override
	{
		//EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/skullaxe_draw.wav", 0.3, 2.4);

		m_fMaxSpeed = 250;
		m_iSwing = 1;
		m_pPlayer->m_bShieldDrawn = false;

		SetThink(&CZ4B_DeagleEL::DeployAttack);
		pev->nextthink = gpGlobals->time + 0.3s;

		BOOL ret = Base::Deploy();

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.8s;

		Q_strcpy(m_pPlayer->m_szAnimExtention, "m16a1ep");
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK2);
#endif

		return ret;
	}

	void DeployAttack()
	{
#ifndef CLIENT_DLL
		switch (KnifeAttack5(GetDeployAttackDamage(), 90, 90, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer))
		{
		case HIT_NONE:
		{
			break;
		}
		case HIT_PLAYER:
		{
			UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "CK_WPN/WEAPON/DEAGLE_EL/DEAGLE_EL_Hit.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
			break;
		}
		case HIT_WALL:
		{
			UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "CK_WPN/WEAPON/DEAGLE_EL/DEAGLE_EL_Stab.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			break;
		}
		}
#endif

		Q_strcpy(m_pPlayer->m_szAnimExtention, AnimExtension);
	}

	void DelaySecondaryAttack()
	{
#ifndef CLIENT_DLL
		switch (KnifeAttack5(GetSecondaryAttackDamage(), 110, 120, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer))
		{
		case HIT_NONE:
		{
			break;
		}
		case HIT_PLAYER:
		{
			UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "CK_WPN/WEAPON/DEAGLE_EL/DEAGLE_EL_Hit.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
			break;
		}
		case HIT_WALL:
		{
			UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "CK_WPN/WEAPON/DEAGLE_EL/DEAGLE_EL_Stab.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			break;
		}
		}
#endif

		Q_strcpy(m_pPlayer->m_szAnimExtention, AnimExtension);
	}

	void Holster(int skiplocal) override
	{
		SetThink(nullptr);
		return Base::Holster();
	}

	bool HasSecondaryAttack() override
	{
		return true;
	}
};
LINK_ENTITY_TO_CLASS(z4b_deagleel, CZ4B_DeagleEL)

}
