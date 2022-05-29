/*
wpn_csg12ss.cpp - Port of Zombie IV Mod
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

    class CCSG12SS : public LinkWeaponTemplate< CCSG12SS,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_M3>::template type,
            TPrecacheEvent,
            TDeployDefault
    >
    {
    public:
    static constexpr const char *V_Model = "models/z4b/v_csg12ss.mdl";
    static constexpr const char *P_Model = "models/p_m3.mdl";
    static constexpr const char *W_Model = "models/w_m3.mdl";
    static constexpr const char *EventFile = "events/csg12ss.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "z4b_csg12ss";
    static constexpr const char *AnimExtension = "shotgun";
    static constexpr int MaxClip = 6;
	enum
	{
		ANIM_IDLE1,
		ANIM_FIRE1,
		ANIM_START_RELOAD,
		ANIM_RELOAD,
		ANIM_PUMP,
		ANIM_PUMP_NOSHOT,
		ANIM_DRAW,
		ANIM_HOLSTER,
		ANIM_SPRINT,
		ANIM_SPRINT_IDLE,
		ANIM_SPRINT_FROM,
		ANIM_WSSB1,
		ANIM_WSSB2,
		ANIM_WSSB3,
		ANIM_IRON_TO,
		ANIM_IRON_IDLE,
		ANIM_IRON_SHOT,
		ANIM_IRON_FROM,
		ANIM_MELEE,
		ANIM_SUICIDE,
	};
    static constexpr float PlayerMaxSpeed = 193;
    static constexpr int DamageDefault = 147;
    static constexpr int DamageZB = 450;
    static constexpr int DamageZBS = 700;
    static constexpr float RangeModifier = 0.99;
    static constexpr auto BulletType = BULLET_PLAYER_BUCKSHOT;
    static constexpr int Penetration = 1;
    KnockbackData KnockBack = { 5000.0f, 500.0f, 1200.0f, 800.0f, 0.3f };

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_M3); }
#endif
	float GetArmorRatioModifier() override { return 1.0; }
	WeaponCrosshairData GetCrosshairData() override { return { 0, 11, 6, 0 }; }

	time_point_t m_tNextThink;
	duration_t m_flPumpTime;

	BOOL Deploy()
	{
		Base::Deploy();
		m_tNextThink = invalid_time_point;
		m_iWeaponState = 0;
		return TRUE;
	}

	void Precache() override
	{
		PRECACHE_SOUND("weapons/shotgun/csg12ss-1.wav");
		return Base::Precache();
	}
    	
	int GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "buckshot";
		p->iMaxAmmo1 = MAX_AMMO_BUCKSHOT;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = 6;
		p->iSlot = 0;
		p->iPosition = 5;
		p->iId = m_iId = WEAPON_Z4B_CSG12SS;
		p->iFlags = 0;
		p->iWeight = M3_WEIGHT;

		return 1;
	}

	void Reload(void)
	{
		if (m_pPlayer->m_iFOV != 90)
			SecondaryAttack();

		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip == 6)
			return;

		if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase())
			return;

		if (!m_fInSpecialReload)
		{
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
			SendWeaponAnim(ANIM_START_RELOAD, UseDecrement() != FALSE);

			m_fInSpecialReload = 1;
			m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.55s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.55s;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.55s;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.55s;
		}
		else if (m_fInSpecialReload == 1)
		{
			if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
				return;

			m_fInSpecialReload = 2;
			SendWeaponAnim(ANIM_RELOAD, UseDecrement() != FALSE);

			m_flNextReload = UTIL_WeaponTimeBase() + 0.45s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.45s;
		}
		else
		{
			m_iClip++;
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
			m_fInSpecialReload = 1;
			m_pPlayer->ammo_buckshot--;
		}
	}

	void WeaponIdle()
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

		if (m_flPumpTime != zero_duration && m_flPumpTime < UTIL_WeaponTimeBase())
			m_flPumpTime = zero_duration;

		if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
		{
			if (m_iClip == 0 && m_fInSpecialReload == 0 && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
			{
				Reload();
			}
			else if (m_fInSpecialReload != 0)
			{
				if (m_iClip != 6 && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
				{
					Reload();
				}
				else
				{
					SendWeaponAnim(ANIM_PUMP, UseDecrement() != FALSE);

					m_fInSpecialReload = 0;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
				}
			}
			else
				SendWeaponAnim(m_iWeaponState ? ANIM_IRON_IDLE : ANIM_IDLE1, UseDecrement() != FALSE);
		}
	}

	void SecondaryAttack(void)
	{
		if (m_fInSpecialReload == 1)
			return;

		if (m_pPlayer->m_iFOV != 90)
		{
#ifndef CLIENT_DLL
			m_pPlayer->UpdateShieldCrosshair(true);
#endif
			SendWeaponAnim(ANIM_IRON_FROM, 0);
			m_tNextThink = gpGlobals->time + 0.2s;
		}
		else
		{
#ifndef CLIENT_DLL
			m_pPlayer->UpdateShieldCrosshair(false);
#endif
			SendWeaponAnim(ANIM_IRON_TO, 0);
			m_tNextThink = gpGlobals->time + 0.2s;
		}

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5s;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.3s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;

	}

#ifndef CLIENT_DLL
	void MaterialSound(float flRadius)
	{
		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
		Vector vecStart = m_pPlayer->GetGunPosition();
		Vector vecEnd = vecStart + gpGlobals->v_forward * flRadius;
		TraceResult ptr;
		UTIL_TraceLine(vecStart, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &ptr);

		const char* rgsz[2];

		switch (UTIL_TextureHit(&ptr, vecStart, vecEnd))
		{
		case CHAR_TEX_WOOD:
			rgsz[0] = "weapons/janus9_wood1.wav";
			rgsz[1] = "weapons/janus9_wood2.wav";

			break;

		case CHAR_TEX_GRATE:
		case CHAR_TEX_METAL:
		case CHAR_TEX_COMPUTER:
			rgsz[0] = "weapons/janus9_metal1.wav";
			rgsz[1] = "weapons/janus9_metal2.wav";

			break;

		default:
			rgsz[0] = "weapons/janus9_stone1.wav";
			rgsz[1] = "weapons/janus9_stone2.wav";

			break;
		}

		UTIL_EmitAmbientSound(ENT(0), ptr.vecEndPos, rgsz[RANDOM_LONG(0, 1)], 0.9, ATTN_NORM, 0, 96 + RANDOM_LONG(0, 15));

	}
#endif

	void ItemPostFrame(void)
	{
		if (m_tNextThink != invalid_time_point && m_tNextThink < gpGlobals->time)
		{
			if (pev->iuser1)
			{
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/shotgun/csg12ss-1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
				m_pPlayer->Killed(m_pPlayer->pev, 1);
			}
				
			if (m_pPlayer->m_iFOV != 90)
			{
				m_iWeaponState = 0;
				m_tNextThink = invalid_time_point;
				m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;
			}
			else
			{
				m_iWeaponState = 1;
				m_tNextThink = invalid_time_point;
				m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 88;
			}
			
		}

		if (m_pPlayer->pev->button & IN_USE && m_pPlayer->m_flNextAttack < 0s && !m_iWeaponState)
		{
			if (m_iClip == 1 && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			{
				SendWeaponAnim(ANIM_SUICIDE, UseDecrement() != FALSE);
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
				pev->iuser1 = 1;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0s;
				m_pPlayer->m_flNextAttack = 3.0s;
				m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 3.0s;
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 3.0s;
				m_tNextThink = gpGlobals->time + 3.0s;
				return;
			}
			SendWeaponAnim(ANIM_MELEE, UseDecrement() != FALSE);
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
			m_pPlayer->m_flNextAttack = 1.2s;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.2s;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.2s;
#ifndef CLIENT_DLL
			switch (KnifeAttack5(GetDamageMelee(), 114, 120, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer))
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
				MaterialSound(90.0);
				break;
			}
			}
#endif
		}
		return CBasePlayerWeapon::ItemPostFrame();
	}

	void PrimaryAttack(void)
	{
		ShotGunFire();
	}

	float GetMaxSpeed(void) override
	{
		if (m_pPlayer->m_iFOV == 90)
			return 210;

		return 145;
	}

	void ShotGunFire()
	{
		if (m_pPlayer->pev->waterlevel == 3)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15s;
			return;
		}

		if (m_iClip <= 0)
		{
			PlayEmptySound();

			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0s;
			return;
		}

		m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

		m_iClip--;
		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
#ifndef CLIENT_DLL
		Vector vecSpread = m_iWeaponState ? Vector(0.03, 0.03, 0) : Vector(0.1, 0.1, 0);
		m_pPlayer->FireBullets(9, m_pPlayer->GetGunPosition(), gpGlobals->v_forward, vecSpread, 3000, BULLET_PLAYER_BUCKSHOT, GetDamageShotGun());
#endif

		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFire, 0, g_vecZero, g_vecZero, 0, 0, 0, 0, FALSE, m_iWeaponState);

#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.875s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.875s;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5s;

		if (m_pPlayer->pev->flags & FL_ONGROUND)
			m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 4, 6);
		else
			m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 8, 11);

		m_pPlayer->m_flEjectBrass = gpGlobals->time + 0.45s;
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

	float GetDamageMelee()
	{
		float flDamage = 110;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
		{
			flDamage = 1300;	//600
		}
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
		{
			flDamage = 800;
		}
#endif
		return flDamage;
	}

	BOOL CanHolster() override
	{
		return !pev->iuser1;
	}

};
LINK_ENTITY_TO_CLASS(z4b_csg12ss, CCSG12SS)
}