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

struct CRagingBull_WeaponData
{
public:
	static constexpr const auto &DefaultReloadTime = 2.3s;
	static constexpr const auto &DefaultDeployTime = 0s;
	static constexpr InventorySlotType ItemSlot = PISTOL_SLOT;
	static constexpr const char *V_Model = "models/z4b/v_ragingbull.mdl";
	static constexpr const char *P_Model = "models/z4b/p_ragingbull.mdl";
	static constexpr const char *W_Model = "models/z4b/p_ragingbull.mdl";
	static constexpr const char *EventFile = "events/ragingbull.sc";
	static constexpr const char *ClassName = "z4b_ragingbull";
	static constexpr const char *AnimExtension = "onehanded";
	static constexpr int MaxClip = 14;
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
		ANIM_CHANGE,
		ANIM_KNIFE_IDLE,
		ANIM_KNIFE_SLASH1,
		ANIM_KNIFE_SLASH2,
		ANIM_KNIFE_DRAW,
		ANIM_KNIFE_RUN,
		ANIM_KNIFE_CHANGE,
		ANIM_KNIFE_SKILL,

	};
	static constexpr const auto & SpreadCalcNotOnGround = 1.5 * (1 - A);
	static constexpr const auto & SpreadCalcWalking = 0.25 * (1 - A);
	static constexpr const auto & SpreadCalcDucking = 0.115 * (1 - A);
	static constexpr const auto & SpreadCalcDefault = 0.13 * (1 - A);
	static constexpr const auto &CycleTime = 0.3s;
	static constexpr int DamageDefault = 57;
	static constexpr int DamageZB = 80;
	static constexpr int DamageZBS = 80;
	static constexpr float AccuracyDefault = 0.9f;
	static constexpr const auto & AccuracyCalc = A - (0.4 - T) * 0.35;
	static constexpr float AccuracyMin = 0.0;
	static constexpr float AccuracyMax = 0.9;
	static constexpr float RangeModifier = 0.87;
	static constexpr auto BulletType = BULLET_PLAYER_50AE;
	static constexpr int Penetration = 2;
	KnockbackData KnockBack = { 400, 300, 300, 200, 0.7f };
	std::array<float, 3> RecoilPunchAngleDelta = { -2, 0, 0 };

	
};

class CRagingBull : public LinkWeaponTemplate< CRagingBull,
		TGeneralData,
		BuildTGetItemInfoFromCSW<WEAPON_DEAGLE>::template type,
		TPrecacheEvent,
		TReloadDefault,
		TPrimaryAttackRifle,
		TFirePistol,
		TRecoilPunch,
		TWeaponIdleDefault,
		TGetDamageDefault
	>, public CRagingBull_WeaponData
{
private:
	int m_iThinkType;
	int m_iSkillEffect;
	int m_iHolsterAmmo;
	time_point_t m_tNextThink;
	time_point_t m_flTimeNextCharge;
	time_point_t m_flTimeHostler;
	duration_t m_flNextRunIdle;
	duration_t m_flNextIdle;
public:

	float GetArmorRatioModifier() override { return 1.5; }

	WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED | ACCURACY_DUCK, 8, 3, 0 }; }
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_DEAGLE); }
#endif

	void Spawn()
	{
		m_flTimeNextCharge = invalid_time_point;
		Base::Spawn();
	}

	int GetItemInfo(ItemInfo* p) override
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "50AE";
		p->iMaxAmmo1 = MAX_AMMO_50AE;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = 14;
		p->iSlot = 1;
		p->iPosition = 1;
		p->iId = m_iId = Z4B_RAGINGBULL;
		p->iFlags = 0;
		p->iWeight = DEAGLE_WEIGHT;

		return 1;
	};

	void Precache() override
	{
		PRECACHE_SOUND("CK_WPN/WEAPON/RagingBull/RagingBullKnife_KnifeSelect.WAV");
		PRECACHE_SOUND("weapons/mastercombat_slash.wav");
		PRECACHE_SOUND("weapons/mastercombat_slash.wav");
		PRECACHE_SOUND("weapons/mastercombat_slash.wav");
		PRECACHE_SOUND("weapons/mastercombat_slash.wav");
		PRECACHE_SOUND("weapons/mastercombat_slash.wav");

		return Base::Precache();
	}

	void ResetRunIdle()
	{
		m_flNextIdle = m_flNextRunIdle = UTIL_WeaponTimeBase();
	}

	void Reload(void)
	{
		if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
			return;

		if (m_pPlayer->ammo_50ae <= 0)
			return;
		if (DefaultReload(14, ANIM_RELOAD, 1.9s))
		{
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
			SetThink(&CRagingBull::ReloadKnifeAttack);
			pev->nextthink = gpGlobals->time + 0.68s;

			m_flAccuracy = 0.9;
		}
	}

	void PrimaryAttack() override
	{
		ResetRunIdle();

		if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
		{
			if (m_iSwing)
			{
				SendWeaponAnim(ANIM_KNIFE_SLASH2, UseDecrement() != FALSE);

				pev->iuser2 = 0;

				SetThink(&CRagingBull::DelayKnifeStabAttack);

				pev->nextthink = gpGlobals->time + 0.25s;

				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.8s;
				m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.76s;
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.76s;

				m_iSwing = 0;
			}
			else
			{
				SendWeaponAnim(ANIM_KNIFE_SLASH1, UseDecrement() != FALSE);
			
				pev->iuser2 = 0;

				SetThink(&CRagingBull::DelayKnifeAttack);

				pev->nextthink = gpGlobals->time + 0.13s;

				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.6s;
				m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.55s;
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.55s;

				m_iSwing = 1;
			}
		}
		else
			Base::PrimaryAttack();
	}

	void SecondaryAttack() override
	{
		ResetRunIdle();

		if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
		{
			m_iWeaponState &= ~WPNSTATE_M4A1_SILENCED;
			SendWeaponAnim(ANIM_KNIFE_CHANGE, UseDecrement() != FALSE);
			strcpy(m_pPlayer->m_szAnimExtention, "onehanded");
		}
		else
		{
			m_iWeaponState |= WPNSTATE_M4A1_SILENCED;
			SendWeaponAnim(ANIM_CHANGE, UseDecrement() != FALSE);
			strcpy(m_pPlayer->m_szAnimExtention, "tomahawk");
		}
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.8s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.76s;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.76s;
	}

	float GetReloadAttackDamage(int iCombo) const
	{
		float flDamage = 40;
		if(iCombo > 2)
			flDamage = 100;

#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage *= 9.5f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 5.5f;
#endif
		return flDamage;
	}

	float GetKnifeAttackDamage(int iCombo) const
	{
		float flDamage = 40;
		if (iCombo > 2)
			flDamage = 100;

#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage *= 9.5f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 5.5f;
#endif
		return flDamage;
	}


	BOOL Deploy() override
	{
		ResetRunIdle();
		m_flAccuracy = 0.9;
		m_pPlayer->m_bShieldDrawn = false;
		m_fMaxSpeed = 250;
		pev->iuser1 = 0;	//reload atk combo
		pev->iuser2 = 0;	//slash1 atk combo

		if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "CK_WPN/WEAPON/RagingBull/RagingBullKnife_KnifeSelect.WAV", VOL_NORM, ATTN_NORM, 0, 94);

			return DefaultDeploy(V_Model, P_Model, ANIM_KNIFE_DRAW, "tomahawk", UseDecrement() != FALSE);
			
		}	
		else
			return DefaultDeploy(V_Model, P_Model, ANIM_DRAW, "onehanded", UseDecrement() != FALSE);	
	}

	void ReloadKnifeAttack()
	{
		pev->iuser1++;

		if(pev->iuser1 == 1)
			pev->nextthink = gpGlobals->time + 0.23s;
		else if(pev->iuser1 == 2)
			pev->nextthink = gpGlobals->time + 0.27s;
		else
			pev->nextthink = gpGlobals->time + 0.23s;

#ifndef CLIENT_DLL
		switch (KnifeAttack5(GetReloadAttackDamage(pev->iuser1), 65, 90, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer))
		{
		case HIT_NONE:
		{
			break;
		}
		case HIT_PLAYER:
		{
			if (RANDOM_LONG(0, 1))
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/janus9_hit1.wav", VOL_NORM, ATTN_NORM, 0, 94);
			else
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/janus9_hit2.wav", VOL_NORM, ATTN_NORM, 0, 94);
			m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
			break;
		}
		case HIT_WALL:
		{
			m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
			break;
		}
		}
#endif
		if (pev->iuser1 > 2)
		{
			pev->iuser1 = 0;
			SetThink(nullptr);
		}

	}

	void DelayKnifeStabAttack()
	{
		SetThink(nullptr);
#ifndef CLIENT_DLL
		switch (KnifeAttack5(GetKnifeAttackDamage(3), 65, 90, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer))
		{
		case HIT_NONE:
		{
			break;
		}
		case HIT_PLAYER:
		{
			if (RANDOM_LONG(0, 1))
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/janus9_hit1.wav", VOL_NORM, ATTN_NORM, 0, 94);
			else
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/janus9_hit2.wav", VOL_NORM, ATTN_NORM, 0, 94);
			m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
			break;
		}
		case HIT_WALL:
		{
			m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
			break;
		}
		}
#endif
	}

	void DelayKnifeAttack()
	{
#ifndef CLIENT_DLL
		switch (KnifeAttack5(GetKnifeAttackDamage(1), 65, 90, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer))
		{
		case HIT_NONE:
		{
			break;
		}
		case HIT_PLAYER:
		{
			if (RANDOM_LONG(0, 1))
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/janus9_hit1.wav", VOL_NORM, ATTN_NORM, 0, 94);
			else
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/janus9_hit2.wav", VOL_NORM, ATTN_NORM, 0, 94);
			m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
			break;
		}
		case HIT_WALL:
		{
			m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
			break;
		}
		}
#endif

		if (++pev->iuser2 & 1)
		{
			pev->nextthink = gpGlobals->time + 0.146s;
			SetThink(&CRagingBull::DelayKnifeAttack);
		}
		else
		{
			SetThink(nullptr);
		}
	}

	void WeaponIdle(void)
	{
		ResetEmptySound();
		if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
		{
			m_iSwing = 0;
			if (m_pPlayer->pev->button & (IN_FORWARD | IN_MOVELEFT | IN_MOVERIGHT | IN_BACK))
			{
				if (m_flNextRunIdle <= UTIL_WeaponTimeBase())
				{
					if(m_iWeaponState & WPNSTATE_M4A1_SILENCED)
						SendWeaponAnim(ANIM_KNIFE_RUN, UseDecrement() != FALSE);
					else
						SendWeaponAnim(ANIM_RUN, UseDecrement() != FALSE);

					m_flNextRunIdle = UTIL_WeaponTimeBase() + 0.8s;
				}

				m_flNextIdle = UTIL_WeaponTimeBase();
			}
			else
			{
				if (m_flNextIdle <= UTIL_WeaponTimeBase())
				{
					if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
						SendWeaponAnim(ANIM_KNIFE_IDLE, UseDecrement() != FALSE);
					else
						SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);

					m_flNextIdle = UTIL_WeaponTimeBase() + 20s;
				}

				m_flNextRunIdle = UTIL_WeaponTimeBase();
			}

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.1s;
		}

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
LINK_ENTITY_TO_CLASS(z4b_ragingbull, CRagingBull)

}
