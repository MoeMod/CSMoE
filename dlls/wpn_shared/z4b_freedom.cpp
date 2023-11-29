/*
wpn_freedom.cpp - Port of Zombie IV Mod
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
#include "weapons/RadiusDamage.hpp"
#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

    class CFreedom : public LinkWeaponTemplate< CFreedom,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_AWP>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TReloadDefault,
			TSecondaryAttackSniperZoom2,
			TRecoilPunch,
            TWeaponIdleDefault,
            TGetDamageDefault
    >
    {
    public:
    static constexpr const auto &DefaultReloadTime = 2.8s;
    static constexpr int ZoomFOV1 = 40;
    static constexpr int ZoomFOV2 = 10;
    static constexpr const char *V_Model = "models/z4b/v_freedom.mdl";
    static constexpr const char *P_Model = "models/z4b/p_freedom.mdl";
    static constexpr const char *W_Model = "models/z4b/w_freedom.mdl";
    static constexpr const char *EventFile = "events/freedom.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "z4b_freedom";
    static constexpr const char *AnimExtension = "rifle";
    static constexpr int MaxClip = 10;
	enum
	{
		ANIM_IDLE1 = 0,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_SHOOT3,
		ANIM_RELOAD,
		ANIM_DRAW,
		ANIM_TITANSB,
		ANIM_AIMON,
		ANIM_AIMOFF,
	};
    static constexpr float PlayerMaxSpeed = 193;
    static constexpr int DamageDefault = 147;
    static constexpr int DamageZB = 700;
    static constexpr int DamageZBS = 900;
    static constexpr float RangeModifier = 0.99;
    static constexpr auto BulletType = BULLET_PLAYER_338MAG;
    static constexpr int Penetration = 3;
	std::array<float, 3> RecoilPunchAngleDelta = { -2.2, 0, 0 };
    KnockbackData KnockBack = { 5000.0f, 500.0f, 1200.0f, 800.0f, 0.3f };

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_AWP); }
#endif
	float GetArmorRatioModifier() override { return m_iWeaponState ? 1.0 : 1.95; }
	WeaponCrosshairData GetCrosshairData() override { return { 0, 8, 6, 0 }; }
	KnockbackData GetKnockBackData() override
	{ 
		if (m_iWeaponState)
			return { 1800.0f, 480.0f, 900.0f, 600.0f, 0.3f };
		else
			return { 5000.0f, 500.0f, 1200.0f, 800.0f, 0.3f };
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/z4b/v_sight_free_A.mdl");
		PRECACHE_MODEL("models/z4b/v_sight_free_B.mdl");
		PRECACHE_SOUND("weapons/freedom-1.wav");
		return Base::Precache();
	}
    	
	int GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "338Magnum";
		p->iMaxAmmo1 = MAX_AMMO_338MAGNUM;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = 6;
		p->iSlot = 0;
		p->iPosition = 2;
		p->iId = m_iId = WEAPON_Z4B_FREEDOM;
		p->iFlags = 0;
		p->iWeight = AWP_WEIGHT;

		return 1;
	}

	BOOL Deploy()
	{
		Base::Deploy();
		if(!m_iWeaponState)
		{
#ifndef CLIENT_DLL
			m_pPlayer->UpdateShieldCrosshair(false);
#endif
		}
		else
#ifndef CLIENT_DLL
			m_pPlayer->UpdateShieldCrosshair(true);
#endif
		return TRUE;
	}

	void SpecialScope()
	{
		if (m_pPlayer->m_iFOV != 90)
		{
#ifndef CLIENT_DLL
			m_pPlayer->UpdateShieldCrosshair(FALSE);
#endif
			m_pPlayer->pev->viewmodel = MAKE_STRING(V_Model);
			m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;
		}
		else
		{
#ifndef CLIENT_DLL
			m_pPlayer->UpdateShieldCrosshair(FALSE);
#endif
			m_pPlayer->pev->viewmodel = MAKE_STRING("models/z4b/v_sight_free_A.mdl");
			if(m_iWeaponState)
				m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 80;
			else
				m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 45;
			SetThink(NULL);
		}
	}

	void SecondaryAttack() override
	{
		if (!m_iWeaponState)
		{
			if (m_pPlayer->m_iFOV != 90 && m_pPlayer->m_flNextAttack < 0s)
			{
				SpecialScope();
				SendWeaponAnim(ANIM_AIMOFF, 0);
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.16s;
				m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;
			}
			else
			{
				SendWeaponAnim(ANIM_AIMON, 0);
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.16s;
				m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;
				SetThink(&CFreedom::SpecialScope);
				pev->nextthink = gpGlobals->time + 0.5s;
			}
			m_pPlayer->m_flNextAttack = 0.5s;
		}
	}

	void Reload(void)
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			return;

		if (!m_iWeaponState)
			return;

		if (m_pPlayer->pev->fov != 90)
		{
			m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;
			m_pPlayer->pev->viewmodel = MAKE_STRING(V_Model);
		}

		if (DefaultReload(6, ANIM_RELOAD, 2.8s))
		{
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_RELOAD);	
#endif		
			m_pPlayer->m_flNextAttack = 1.46s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.8s;
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.8s;		
		}
	}

	void ItemPostFrame(void)
	{
		if (!m_iWeaponState)
		{
			if (m_pPlayer->m_bResumeZoom)
			{
				m_pPlayer->m_iFOV = m_pPlayer->m_iLastZoom;
				m_pPlayer->pev->fov = m_pPlayer->m_iFOV;
				m_pPlayer->pev->viewmodel = MAKE_STRING("models/z4b/v_sight_free_A.mdl");

				if (m_pPlayer->m_iFOV == m_pPlayer->m_iLastZoom)
				{
					// return the fade level in zoom.
					m_pPlayer->m_bResumeZoom = false;
				}
			}
		}

		if (m_pPlayer->pev->button & IN_USE && m_pPlayer->m_flNextAttack < 0s)
		{
			m_flLastFire = invalid_time_point;
			if (m_iWeaponState)
			{
				m_iWeaponState = 0;
#ifndef CLIENT_DLL
				m_pPlayer->UpdateShieldCrosshair(false);
#endif
				if (m_pPlayer->pev->fov != 90)
				{
					m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;
					m_pPlayer->pev->viewmodel = MAKE_STRING(V_Model);
				}
				SendWeaponAnim(ANIM_RELOAD, UseDecrement() != FALSE);//change to sniper
				strcpy(m_pPlayer->m_szAnimExtention, "rifle");
			}
			else
			{
#ifndef CLIENT_DLL
				m_pPlayer->UpdateShieldCrosshair(true);
#endif
				if (m_pPlayer->pev->fov != 90)
				{
					m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;
					m_pPlayer->pev->viewmodel = MAKE_STRING(V_Model);
				}
				m_iWeaponState = 1;
				SendWeaponAnim(ANIM_RELOAD, UseDecrement() != FALSE);//change to shotgun
				strcpy(m_pPlayer->m_szAnimExtention, "shotgun");
			}
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0s;
			m_pPlayer->m_flNextAttack = 2.8s;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.8s;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 2.8s;
		}
		return CBasePlayerWeapon::ItemPostFrame();
	}

	void PrimaryAttack(void)
	{
		if (m_iWeaponState)
		{
			ShotGunFire();
		}
		else
		{
			if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
				Fire(0.5, 1.1s, FALSE);
			else if (m_pPlayer->pev->velocity.Length2D() > 120)
				Fire(0.2, 1.1s, FALSE);
			else if (m_pPlayer->pev->velocity.Length2D() > 10)
				Fire(0.1, 1.1s, FALSE);
			else
				Fire(0, 1.1s, FALSE);
		}
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
		m_pPlayer->FireBullets(9, m_pPlayer->GetGunPosition(), gpGlobals->v_forward, Vector(0.0675, 0.0675, 0), 3000, BULLET_PLAYER_BUCKSHOT, GetDamageShotGun());
#endif

		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFire, 0, g_vecZero, g_vecZero, 0, 0, 0, 0, FALSE, FALSE);

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

	void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
	{
		if (m_pPlayer->pev->fov != 90)
		{
			m_pPlayer->m_bResumeZoom = true;
			m_pPlayer->m_iLastZoom = m_pPlayer->m_iFOV;
			m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 90;
			m_pPlayer->pev->viewmodel = MAKE_STRING(V_Model);
		}
		else
		{ // not zoomed, so bullets diverts more
			flSpread += 0.05;
		}

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

		m_pPlayer->m_flEjectBrass = gpGlobals->time + 1.0s;
		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

		const float flDistance = 8192;
		const int iPenetration = Penetration;
		const Bullet iBulletType = BulletType;
		const int iDamage = GetDamage();
		const float flRangeModifier = RangeModifier;
		const BOOL bPistol = ItemSlot == PISTOL_SLOT;

		const Vector vecSrc = m_pPlayer->GetGunPosition();
		const Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, flDistance, iPenetration, iBulletType, iDamage, flRangeModifier, m_pPlayer->pev, bPistol, m_pPlayer->random_seed);

		FireEvent(vecDir);

		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + flCycleTime + 1.9s;

		Recoil();
	}

	void FireEvent(const Vector& vecDir)
	{
		int flags = 0;

#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, static_cast<int>(m_pPlayer->pev->punchangle.x * 100), static_cast<int>(m_pPlayer->pev->punchangle.y * 100), FALSE, TRUE);
	}

	float GetDamageShotGun()
	{
		float flDamage = 30;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
		{
			flDamage = 70;	//600
		}
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
		{
			flDamage = 120;
		}
#endif
		return flDamage;
	}

};
LINK_ENTITY_TO_CLASS(z4b_freedom, CFreedom)
}