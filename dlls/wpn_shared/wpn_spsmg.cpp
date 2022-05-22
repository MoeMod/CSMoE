/*
spsmg.cpp
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
    class CSPSMG : public LinkWeaponTemplate< CSPSMG,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_TMP>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TReloadDefault,
            TPrimaryAttackRifle,
            TFirePistol,
            TWeaponIdleDefault
    >
    {
    public:
    static constexpr const auto &DefaultReloadTime = 3.6s;
    static constexpr const char *V_Model = "models/v_spsmg.mdl";
    static constexpr const char *P_Model = "models/p_spsmg.mdl";
    static constexpr const char *W_Model = "models/w_spsmg.mdl";
    static constexpr const char *EventFile = "events/spsmg.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_spsmg";
    static constexpr const char *AnimExtension = "onehanded";
    static constexpr int MaxClip = 30;
	enum
	{
		ANIM_IDLE1,
		ANIM_RELOAD,
		ANIM_DRAW,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
	};
    static constexpr float PlayerMaxSpeed = 250;
    static constexpr const auto &SpreadCalcNotOnGround = 0.37*A;
    static constexpr const auto &SpreadCalcDefault = 0.03*A;

	static constexpr const auto& CycleTime = 0.07s;
    static constexpr float AccuracyDefault = 0.0;
	KnockbackData KnockBack = { 4000.0f, 600.0f, 1000.0f, 600.0f, 0.4f };

	int m_iClipLeft;

	KnockbackData GetKnockBackData() override
	{
		float flGround = 0.0f, flAir = 0.0f, flAirStrafing = 0.0f, flDucking = 0.0f, flVelocityModifier = 1.0f;
		if (m_iClipLeft <= 0)
			return { flGround, flAir, flAirStrafing, flDucking, flVelocityModifier };

		if (m_iClipLeft > 1)
		{
			flGround = 2200;
			flAir = 1350;
			flAirStrafing = 1400;
			flDucking = 520;

			flVelocityModifier = 0.45;
		}
		else
		{
			flGround = 250;
			flAir = 200;
			flAirStrafing = 250;
			flDucking = 90;

			flVelocityModifier = 0.6;
		}

		return { flGround, flAir, flAirStrafing, flDucking, flVelocityModifier };
	}


#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_46mm" , AMMO_46MM_PRICE }; }
#endif
	float GetArmorRatioModifier() override { return 1.0; }
	WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 9, 3, 170 }; }

	int GetItemInfo(ItemInfo* p) override
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "46MM";
		p->iMaxAmmo1 = MAX_AMMO_46MM;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = 30;
		p->iSlot = 0;
		p->iPosition = 7;
		p->iId = m_iId = WEAPON_SPSMG;
		p->iFlags = 0;
		p->iWeight = MP5NAVY_WEIGHT;

		return 1;
	};

	void StopIdleSound(void)
	{
#ifndef CLIENT_DLL
		MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
		WRITE_BYTE(79);
		WRITE_SHORT(m_pPlayer->entindex());
		WRITE_SHORT(CHAN_AUTO);
		WRITE_STRING("weapons/spsmg_idle.wav");
		MESSAGE_END();
#endif
	}

	void Holster(int skiplocal) override
	{
		CBasePlayerWeapon::Holster(skiplocal);

		StopIdleSound();
	}

    void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
		m_bDelayFire = true;
		m_iShotsFired++;
		m_flAccuracy = ((float)(m_iShotsFired * m_iShotsFired) / 185) + 0.55;

		if (m_flAccuracy > 1.45)
			m_flAccuracy = 1.45;

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
		m_iClipLeft = 1;
		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;

		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 1, BULLET_PLAYER_45ACP, GetDamageA(), 0.83, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, 1);

		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.3s;

		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			KickBack(1.0, 0.55, 0.4, 0.05, 5.0, 3.25, 9);
		else if (m_pPlayer->pev->velocity.Length2D() > 0)
			KickBack(0.6, 0.3, 0.25, 0.035, 3.5, 2.75, 10);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			KickBack(0.25, 0.175, 0.125, 0.02, 2.25, 1.2, 10);
		else
			KickBack(0.275, 0.2, 0.15, 0.02, 3.0, 1.75, 9);
    }

	void SecondaryAttack()
	{
		if (m_iClip > 0)
			StopIdleSound();

		if (m_iClip == 1)
		{
			PrimaryAttack();

			return;
		}

		if (m_pPlayer->pev->waterlevel == 3)
		{
			PlayEmptySound();

			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2s;

			return;
		}

		if (m_iClip <= 0)
		{
			Reload();

			if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			{
				PlayEmptySound();

				m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2s;
			}

			return;
		}

		m_pPlayer->m_iWeaponVolume = NORMAL_EXPLOSION_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

		m_iClipLeft = m_iClip;

		m_iClip = 0;

		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

		if (m_iClipLeft > 1 && m_iClipLeft <=  30)
		{
			float flSpread = 0.067;;
#ifndef CLIENT_DLL
			m_pPlayer->FireBullets(m_iClipLeft, m_pPlayer->GetGunPosition(), gpGlobals->v_forward, Vector(flSpread, flSpread, 0.0), 3200, BULLET_PLAYER_45ACP, 16, GetDamageB());
#endif
			int flags;

#ifdef CLIENT_WEAPONS
			flags = FEV_NOTHOST;
#else
			flags = 0;
#endif

			PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, 0.0, flSpread, 7, int(m_vVecAiming.x * 100.0), TRUE, m_iClipLeft);

		}
#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", 0, 0);
#endif
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0)
		{
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.07s;
		}
		else
		{
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 3.6s;
		}

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.3s;

		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			KickBack(1.0, 0.55, 0.4, 0.05, 5.0, 3.25, 9);
		else if (m_pPlayer->pev->velocity.Length2D() > 0)
			KickBack(0.6, 0.3, 0.25, 0.035, 3.5, 2.75, 10);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			KickBack(0.25, 0.175, 0.125, 0.02, 2.25, 1.2, 10);
		else
			KickBack(0.275, 0.2, 0.15, 0.02, 3.0, 1.75, 9);
	}
    		 
	float GetDamageB() const
	{
		if (m_iClipLeft <= 0)
			return 0.0;

		float a;
		float b;

		a = 28.0;
		b = 40.0;

#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
		{
			a = 38.0;
			b = 40.0;
		}
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
		{
			a = 86.0;
			b = 89.0;
		}
#endif
		if (m_iClipLeft > 1)
			return a + b;

		return a;
	}

	float GetDamageA() const
	{
		float flDamage = 28.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
		{
			flDamage = 38;
		}
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 75.0f;
#endif
		return flDamage;
	}
};
LINK_ENTITY_TO_CLASS(weapon_spsmg, CSPSMG)
}