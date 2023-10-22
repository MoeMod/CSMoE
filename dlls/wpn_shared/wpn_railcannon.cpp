/*
wpn_railcannon.cpp - Port of Zombie IV Mod
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

#include "weapons/KnifeAttack.h"
#include "weapons/WeaponTemplate.hpp"
#include "weapons/RadiusDamage.hpp"
#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

    class CRailCannon : public LinkWeaponTemplate< CRailCannon,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_XM1014>::template type,
            TPrecacheEvent,
            TDeployDefault
    >
    {
    public:
    static constexpr const char *V_Model = "models/v_railcannon.mdl";
    static constexpr const char *P_Model = "models/p_railcannon.mdl";
    static constexpr const char *W_Model = "models/w_railcannon.mdl";
    static constexpr const char *EventFile = "events/railcannon.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_railcannon";
    static constexpr const char *AnimExtension = "m249";
    static constexpr int MaxClip = 24;
	enum
	{
		ANIM_IDLE1,
		ANIM_IDLE_CHARGE1,
		ANIM_IDLE_CHARGE2,
		ANIM_IDLE_CHARGE3,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_RELOAD,
		ANIM_DRAW
	};
    static constexpr float PlayerMaxSpeed = 205;
    static constexpr auto BulletType = BULLET_PLAYER_BUCKSHOT;
    static constexpr int Penetration = 1;
    KnockbackData KnockBack = { 1000.0f, 650.0f, 500.0f, 500.0f, 0.3f };

	time_point_t m_flTimeNextCharge;
	int m_iDefaultAmmo2;

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_depletedalloy" , 100 }; }
#endif

	WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 8, 6, 0 }; }

	float GetArmorRatioModifier() override
	{
		switch (m_iWeaponState)
		{
		case 2:
			return 1.1;

		case 3:
			return 1.5;

		case 4:
			return 1.8;

		default:
			return 1.0;
		}
	}

	KnockbackData GetKnockBackData() override 
	{ 
		float flGround, flAir, flAirStrafing, flDucking, flVelocityModifier;
		switch (m_iWeaponState)
		{
		case 2:
			flVelocityModifier = 0.8;
			flGround = 1200;
			flAir = 650;
			flAirStrafing = 500;
			flDucking = 450;
			break;

		case 3:
			flVelocityModifier = 0.9;
			flGround = 1500;
			flAir = 850;
			flAirStrafing = 550;
			flDucking = 450;
			break;

		case 4:
			flVelocityModifier = 0.9;
			flGround = 3000;
			flAir = 850;
			flAirStrafing = 500;
			flDucking = 700;
			break;

		default:
			flVelocityModifier = 0.3;
			flGround = 1000;
			flAir = 600;
			flAirStrafing = 500;
			flDucking = 450;
			break;
		}
		return { flGround, flAir, flAirStrafing, flDucking, flVelocityModifier };
	}

	void Spawn() override
	{
		m_iDefaultAmmo2 = 0;
		m_flTimeNextCharge = invalid_time_point;
		Base::Spawn();
	}

	void Precache() override
	{
		PRECACHE_MODEL("sprites/muzzleflash19.spr");
		PRECACHE_SOUND("weapons/railcanon-1.wav");
		PRECACHE_SOUND("weapons/railcanon_chage1_start.wav");
		PRECACHE_SOUND("weapons/railcanon_chage3_loop.wav");
		PRECACHE_SOUND("weapons/railcanon_chage1.wav");
		PRECACHE_SOUND("weapons/railcanon_chage2.wav");
		PRECACHE_SOUND("weapons/railcanon_chage3.wav");
		return Base::Precache();
	}
    	
	int GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "depletedalloy";
		p->iMaxAmmo1 = 40;
		p->pszAmmo2 = "depletedalloy2";
		p->iMaxAmmo2 = -1;
		p->iMaxClip = 24;
		p->iSlot = 0;
		p->iPosition = 5;
		p->iId = m_iId = WEAPON_RAILCANNON;
		p->iFlags = 0;
		p->iWeight = XM1014_WEIGHT;

		return 1;
	}

	int ExtractAmmo(CBasePlayerWeapon* pWeapon)
	{
		if (m_iDefaultAmmo2)
		{
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iDefaultAmmo2;
			m_iDefaultAmmo2 = 0;
		}
		return CBasePlayerWeapon::ExtractAmmo(pWeapon);
	}

	void StopIdleSound(void)
	{
#ifndef CLIENT_DLL
		MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
		WRITE_BYTE(79);
		WRITE_SHORT(m_pPlayer->entindex());
		WRITE_SHORT(CHAN_WEAPON);
		WRITE_STRING("weapons/railcanon_chage3_loop.wav");
		MESSAGE_END();
#endif
	}

	void SecondaryAttack(void)
	{
		if (m_pPlayer->pev->waterlevel == 3)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.15s;
			return;
		}

		if (m_iWeaponState != 0)
		{
			if (m_iClip <= 0)
			{
				if (gpGlobals->time <= m_flTimeNextCharge)
					return;
				m_flTimeNextCharge = gpGlobals->time + 999999s;
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/railcanon_chage3_loop.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);
				return;
			}
		}
		else if (m_iClip <= 0)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;
			WeaponIdle();
			return;
		}

		if (!m_iWeaponState)
		{
			m_flTimeNextCharge = gpGlobals->time + 0.455s;
			m_iWeaponState = 1;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.455s;

			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/railcanon_chage1_start.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);
		}

		if (m_iWeaponState)
		{
			if (gpGlobals->time > m_flTimeNextCharge)
			{
				m_flTimeNextCharge = gpGlobals->time + 0.455s;

				switch (m_iWeaponState)
				{
				case 1:
					m_iWeaponState = 2;
					SendWeaponAnim(ANIM_IDLE_CHARGE1);

					m_iClip--;
					m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 1;

					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/railcanon_chage1_start.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);
					EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/railcanon_chage1.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);

					break;

				case 2:
					m_iWeaponState = 3;
					SendWeaponAnim(ANIM_IDLE_CHARGE2);

					m_iClip--;
					m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 2;

					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/railcanon_chage1_start.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);
					EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/railcanon_chage2.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);

					break;

				case 3:
					m_iWeaponState = 4;
					SendWeaponAnim(ANIM_IDLE_CHARGE3);

					m_iClip--;
					m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 3;

					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/railcanon_chage3_loop.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);
					EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/railcanon_chage3.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);

					break;

				default:
					break;
				}
			}
		}
	}

	void PrimaryAttack(void)
	{
		StopIdleSound();

		if (m_pPlayer->pev->waterlevel == 3)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15s;
			return;
		}

		if (m_iClip <= 0 && m_iWeaponState < 2)
		{
			Reload();

			if (m_iClip == 0)
				PlayEmptySound();

			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0s;
			return;
		}

		m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

		if (m_iWeaponState < 2)
			m_iClip--;

		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 0;
		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

		SendWeaponAnim(ANIM_SHOOT1 + m_iClip % 2);

#ifndef CLIENT_DLL
		m_pPlayer->FireBullets(GetShots(), m_pPlayer->GetGunPosition(), gpGlobals->v_forward, Vector(GetSpread(), GetSpread(), 0.0), GetRange(),
			m_iWeaponState > 1 ? BULLET_PLAYER_DEPLETEDALLOY_BEAMSHOT : BULLET_PLAYER_DEPLETEDALLOY_BUCKSHOT,
			0,
			GetDamageAmount()
		);
#endif

		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFire, 0, g_vecZero, g_vecZero, 0.0, GetSpread(), 7, int(m_vVecAiming.x * 100), m_iClip == 0, GetShots());

#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		if (m_iWeaponState < 1)
		{
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;
		}
		else
		{
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;
		}

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;

		if (m_pPlayer->pev->flags & FL_ONGROUND)
			m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 2.4, 2.4);
		else
			m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 7, 10);

		m_iWeaponState = 0;
		m_flTimeNextCharge = invalid_time_point;
	}


	void WeaponIdle(void)
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		if (!(m_pPlayer->pev->button & IN_ATTACK2) && m_iWeaponState != 0 && m_flNextPrimaryAttack <= UTIL_WeaponTimeBase())
		{
			PrimaryAttack();
		}

		if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
		{
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;

			SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);
		}
	}

	float GetDamageShotGun()
	{
		float flDamage = 20;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
		{
			flDamage = 80;	//600
		}
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
		{
			flDamage = 120;
		}
#endif
		return flDamage;
	}

	void Reload(void)
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iWeaponState != 0)
			return;

		if (DefaultReload(24, ANIM_RELOAD, 2.93s))
		{
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		}
	}

	int GetShots(void)
	{
		switch (m_iWeaponState)
		{
		default:
		case 2:
			return 7;

		case 3:
		case 4:
			return 8;
		}
	}

	float GetDamageAmount(void)
	{
		float flDamage = 20;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
		{
			switch (m_iWeaponState)
			{
			case 2:	flDamage = 25; break;
			case 3: flDamage = 45; break;
			case 4: flDamage = 72; break;
			default: flDamage = 22; break;
			}
		}
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
		{
			switch (m_iWeaponState)
			{
			case 2:	flDamage = 25; break;
			case 3: flDamage = 45; break;
			case 4: flDamage = 72; break;
			default: flDamage = 22; break;
			}
		}
		else
		{
			switch (m_iWeaponState)
			{
			case 2:	flDamage = 20; break;
			case 3: flDamage = 20; break;
			case 4: flDamage = 16; break;
			default: flDamage = 20; break;
			}
		}
#endif
		return flDamage;
	}

	float GetRange(void)
	{
		switch (m_iWeaponState)
		{
		case 2:
			return 4128;

		case 3:
			return 6096;

		case 4:
			return 8048;

		default:
			return 3048;
		}
	}

	float GetSpread(void)
	{
		switch (m_iWeaponState)
		{
		case 2:
			return 0.0625;

		case 3:
			return 0.045;

		case 4:
			return 0.01;

		default:
			return 0.0715;
		}
	}

	void Holster(int skiplocal = 0)
	{
		StopIdleSound();

		if (m_iWeaponState != 0)
		{
			m_iClip += m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType];
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 0;
			m_iWeaponState = 0;
		}

		CBasePlayerWeapon::Holster(skiplocal);
	}


};
LINK_ENTITY_TO_CLASS(weapon_railcannon, CRailCannon)
}