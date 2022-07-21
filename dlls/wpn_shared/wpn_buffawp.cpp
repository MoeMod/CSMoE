/*
wpn_buffawp.cpp - Port of Zombie IV Mod
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

    class CBUFFAWP : public LinkWeaponTemplate< CBUFFAWP,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_AWP>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TReloadDefault,
			TSecondaryAttackSniperZoom2,
			TRecoilPunch,
            TWeaponIdleDefault
    >
    {
    public:
    static constexpr const auto &DefaultReloadTime = 2.0s;
    static constexpr int ZoomFOV1 = 40;
    static constexpr int ZoomFOV2 = 15;
    static constexpr const char *V_Model = "models/v_buffawp.mdl";
    static constexpr const char *P_Model = "models/p_buffawp.mdl";
    static constexpr const char *W_Model = "models/w_buffawp.mdl";
    static constexpr const char *EventFile = "events/buffawp.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_buffawp";
    static constexpr const char *AnimExtension = "rifle";
    static constexpr int MaxClip = 20;
	enum
	{
		ANIM_IDLE1 = 0,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_SHOOT3,
		ANIM_RELOAD,
		ANIM_DRAW,
	};
    static constexpr float PlayerMaxSpeed = 220;
    static constexpr float RangeModifier = 0.99;
    static constexpr auto BulletType = BULLET_PLAYER_338MAG;
    static constexpr int Penetration = 5;
	std::array<float, 3> RecoilPunchAngleDelta = { -2, 0, 0 };
    KnockbackData KnockBack = { 5000.0f, 700.0f, 1200.0f, 800.0f, 0.2f };

	time_point_t m_flChargeTime;

	void Precache() override
	{
		m_iShellId = PRECACHE_MODEL("models/rshell_big.mdl");
		PRECACHE_MODEL("sprites/muzzleflash43.spr");
		return Base::Precache();
	}

	int GetItemInfo(ItemInfo* p) override
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "338Magnum";
		p->iMaxAmmo1 = 30;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = 20;
		p->iSlot = 0;
		p->iPosition = 2;
		p->iId = m_iId = WEAPON_BUFFAWP;
		p->iFlags = 0;
		p->iWeight = AWP_WEIGHT;

		return 1;
	}

	float GetArmorRatioModifier() override { return 1.98; }
	WeaponCrosshairData GetCrosshairData() override { return { 0, 8, 3, 140 }; }
    	
	void PrimaryAttack(void)
	{
		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			Fire(0.5, 1.2s, FALSE);
		else if (m_pPlayer->pev->velocity.Length2D() > 140)
			Fire(0.1, 1.2s, FALSE);
		else if (m_pPlayer->pev->velocity.Length2D() > 10)
			Fire(0.025, 1.2s, FALSE);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			Fire(0, 1.2s, FALSE);
		else
			Fire(0.001, 1.2s, FALSE);
	}

	float GetMaxSpeed(void) override
	{
		if (m_pPlayer->m_iFOV == 90)
			return 220;

		return 120;
	}

	BOOL Deploy(void)
	{
		ResetCharge();
		return Base::Deploy();
	}

	void SecondaryAttack(void)
	{
		switch (m_pPlayer->m_iFOV)
		{
		case 90: 
		{
			m_pPlayer->m_iFOV = m_pPlayer->pev->fov = ZoomFOV1; 
			StartCharge();
			break;
		}
		case 40: m_pPlayer->m_iFOV = m_pPlayer->pev->fov = ZoomFOV2; break;
		default:
		{
			m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 90;
			ResetCharge();
			break;
		}
		}

		m_pPlayer->ResetMaxSpeed();
		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/zoom.wav", 0.2, 2.4);
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;
	}

	void ItemPostFrame()
	{
		if (m_pPlayer->pev->fov != 90 && m_flChargeTime == invalid_time_point)
			StartCharge();
		if(m_pPlayer->pev->fov == 90)
			ResetCharge();
		return CBasePlayerWeapon::ItemPostFrame();
	}

	void StartCharge()
	{
		m_flChargeTime = gpGlobals->time;
	}

	void ResetCharge()
	{
		m_flChargeTime = invalid_time_point;
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

		int iCharged = GetWeaponCharged();
		const float flDistance = 8192;
		const int iPenetration = Penetration;
		const Bullet iBulletType = BulletType;
		const int iDamage = GetDamage(iCharged);
		const float flRangeModifier = RangeModifier;
		const BOOL bPistol = ItemSlot == PISTOL_SLOT;

		const Vector vecSrc = m_pPlayer->GetGunPosition();
		const Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, flDistance, iPenetration, iBulletType, iDamage, flRangeModifier, m_pPlayer->pev, bPistol, m_pPlayer->random_seed);

		FireEvent(vecDir);

		m_pPlayer->m_flEjectBrass = gpGlobals->time + 0.73s;
		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + flCycleTime + 1.9s;

		Recoil();
		ResetCharge();
	}

	void FireEvent(const Vector& vecDir)
	{
		int flags = 0;

#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, static_cast<int>(m_pPlayer->pev->punchangle.x * 100), static_cast<int>(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);
	}

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_338magnum" , 125 }; }
#endif

	int GetDamage(int iCharged)
	{
		float flDamage[4] = { 115, 172, 242, 322 };
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			float flDamage[4] = { 320, 480, 672, 902 };
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			float flDamage[4] = { 320, 480, 672, 902 };
#endif
		return flDamage[iCharged];
	}

	int GetWeaponCharged()
	{
		int iCharged = 0;

		if (m_flChargeTime != invalid_time_point)
		{
			iCharged = round((gpGlobals->time - m_flChargeTime) / 0.8s);
			if (iCharged >= 3)
			{
				iCharged = 3;
			}
		}
		return iCharged;
	}

};
LINK_ENTITY_TO_CLASS(weapon_buffawp, CBUFFAWP)
}