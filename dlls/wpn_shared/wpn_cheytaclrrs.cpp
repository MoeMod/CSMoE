/*
z4b_cheytaclrrs.cpp - Port of Zombie IV Mod
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

    class CCheytaclrrs : public LinkWeaponTemplate< CCheytaclrrs,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_AWP>::template type,
            TPrecacheEvent,
            TDeployDefault,
			TSecondaryAttackSniperZoom2,
			TRecoilPunch,
            TWeaponIdleDefault,
            TGetDamageDefault
    >
    {
    public:
    static constexpr int ZoomFOV1 = 40;
    static constexpr int ZoomFOV2 = 10;
    static constexpr const char *V_Model = "models/v_cheytaclrrs.mdl";
    static constexpr const char *P_Model = "models/p_cheytaclrrs.mdl";
    static constexpr const char *W_Model = "models/w_cheytaclrrs.mdl";
    static constexpr const char *EventFile = "events/cheytaclrrs.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_cheytaclrrs";
    static constexpr const char *AnimExtension = "rifle";
    static constexpr int MaxClip = 30;
	enum
	{
		ANIM_IDLE1 = 0,
		ANIM_SHOOT,
		ANIM_DRAW,
		ANIM_SHOOT_EMPTY,
	};
    static constexpr float PlayerMaxSpeed = 196;
    static constexpr float ArmorRatioModifier = 1.95f;
    static constexpr int DamageDefault = 197;
    static constexpr int DamageZB = 1365;
    static constexpr int DamageZBS = 605;
    static constexpr float RangeModifier = 0.995;
    static constexpr auto BulletType = BULLET_PLAYER_338MAG;
    static constexpr int Penetration = 9;
	std::array<float, 3> RecoilPunchAngleDelta = { -2, 0, 0 };
    KnockbackData KnockBack = { 5000.0f, 500.0f, 1200.0f, 800.0f, 0.65f };

	void Precache() override
	{
		m_iShellId = PRECACHE_MODEL("models/rshell_big.mdl");
		return Base::Precache();
	}

	int GetItemInfo(ItemInfo* p) override
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "408CheyTac";
		p->iMaxAmmo1 = 30;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = -1;
		p->iSlot = 0;
		p->iPosition = 2;
		p->iId = m_iId = WEAPON_CHEYTACLRRS;
		p->iFlags = 0;
		p->iWeight = AWP_WEIGHT;

		return 1;
	}
    	
	void Reload(void)
	{
		return;
	}

	int ExtractAmmo(CBasePlayerWeapon* pWeapon)
	{
		if (m_iDefaultAmmo)
		{
			if (m_pPlayer)
				m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = m_iDefaultAmmo;
			m_iClip = WEAPON_NOCLIP;
			m_iDefaultAmmo = 0;
			return TRUE;
		}
		return CBasePlayerWeapon::ExtractAmmo(pWeapon);
	}


	void PrimaryAttack(void)
	{
		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			Fire(0.5, 2.25s, FALSE);
		else if (m_pPlayer->pev->velocity.Length2D() > 120)
			Fire(0.2, 2.25s, FALSE);
		else if (m_pPlayer->pev->velocity.Length2D() > 10)
			Fire(0.1, 2.25s, FALSE);
		else
			Fire(0, 2.25s, FALSE);
	}

	float GetMaxSpeed(void) override
	{
		if (m_pPlayer->m_iFOV == 90)
			return 196;

		return 136;
	}

	void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
	{
		if (m_pPlayer->pev->fov != 90)
		{
			m_pPlayer->m_bResumeZoom = true;
			m_pPlayer->m_iLastZoom = m_pPlayer->m_iFOV;
			m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 90;
		}
		else // not zoomed, so bullets diverts more
			flSpread += 0.05;

		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		{
			if (m_fFireOnEmpty)
			{
				PlayEmptySound();
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
			}

			return;
		}
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;

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
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
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

		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, static_cast<int>(m_pPlayer->pev->punchangle.x * 100), static_cast<int>(m_pPlayer->pev->punchangle.y * 100), m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] == 0, FALSE);
	}

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_408cheytac" , 95 }; }
#endif

};
LINK_ENTITY_TO_CLASS(weapon_cheytaclrrs, CCheytaclrrs)
}