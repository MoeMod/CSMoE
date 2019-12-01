/*
FireRifle.hpp - part of CSMoE template weapon framework, to auto-gen Fire() function for rifles
Copyright (C) 2019 Moemod Hyakuya

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#pragma once

template<class CFinal, class CBase = CBaseTemplateWeapon>
class TFireRifle : public CBase
{
public:
	// Default Settings
//	static constexpr auto &&AccuracyCalc = (x * x * x / 200.0) + 0.35;
//	static constexpr float AccuracyMax = 1.25f;
//	static constexpr float RangeModifier = 0.98;
//	static constexpr auto BulletType = BULLET_PLAYER_762MM;
//	static constexpr int Penetration = 2;
	static constexpr int Distance = 8192;
	static constexpr const auto &N = WeaponTemplate::Varibles::N;

public:
	void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
	{
		CFinal &wpn = static_cast<CFinal &>(*this);
		auto &&data = wpn.WeaponTemplateDataSource();

		CBase::m_bDelayFire = true;
		CBase::m_iShotsFired++;

		CBase::m_flAccuracy = df::AccuracyCalc::Get(data)(N = CBase::m_iShotsFired);

		wpn.CheckAccuracyBoundary();

		if (CBase::m_iClip <= 0)
		{
			if (CBase::m_fFireOnEmpty)
			{
				CBase::PlayEmptySound();
				CBase::m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
			}

			return;
		}

		CBase::m_iClip--;
		CBase::m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
		CBase::m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

		UTIL_MakeVectors(CBase::m_pPlayer->pev->v_angle + CBase::m_pPlayer->pev->punchangle);

		CBase::m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
		CBase::m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

		const float flDistance = df::Distance::Get(data);
		const int iPenetration = df::Penetration::Get(data);
		const Bullet iBulletType = df::BulletType::Get(data);
		const int iDamage = wpn.GetDamage();
		const float flRangeModifier = df::RangeModifier::Get(data);
		const BOOL bPistol = df::ItemSlot::Get(data) == PISTOL_SLOT;
		Vector vecSrc = CBase::m_pPlayer->GetGunPosition();
		Vector vecDir = CBase::m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, flDistance, iPenetration, iBulletType, iDamage, flRangeModifier, CBase::m_pPlayer->pev, bPistol, CBase::m_pPlayer->random_seed);

		wpn.FireEvent(vecDir);

		CBase::m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
		CBase::m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
		CBase::m_flNextPrimaryAttack = CBase::m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
		if (!CBase::m_iClip && CBase::m_pPlayer->m_rgAmmo[CBase::m_iPrimaryAmmoType] <= 0)
			CBase::m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		CBase::m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + flCycleTime + 1.9s;

		wpn.Recoil();
	}

public:
	void FireEvent(const Vector &vecDir)
	{
		int flags = 0;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		CFinal &wpn = static_cast<CFinal &>(*this);
		auto &&data = wpn.WeaponTemplateDataSource();
		PLAYBACK_EVENT_FULL(flags, CBase::m_pPlayer->edict(), wpn.m_usFire, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, static_cast<int>(CBase::m_pPlayer->pev->punchangle.x * 100), static_cast<int>(CBase::m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);
	}


};
