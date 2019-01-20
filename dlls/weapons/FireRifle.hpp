//
// Created by 小白白 on 2019-01-19.
//

#pragma once

#include "ExpressionBuilder.hpp"

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

	static constexpr auto &&N = ExpressionBuilder::x;

public:
	void Fire(float flSpread, float flCycleTime, BOOL fUseAutoAim)
	{
		CFinal &wpn = static_cast<CFinal &>(*this);

		CBase::m_bDelayFire = true;
		CBase::m_iShotsFired++;

		CBase::m_flAccuracy = wpn.AccuracyCalc(N = CBase::m_iShotsFired);

		CheckAccuracyBoundaryMin(&wpn);
		CheckAccuracyBoundaryMax(&wpn);

		if (CBase::m_iClip <= 0)
		{
			if (CBase::m_fFireOnEmpty)
			{
				CBase::PlayEmptySound();
				CBase::m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;
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

		const float flDistance = wpn.Distance;
		const int iPenetration = wpn.Penetration;
		const Bullet iBulletType = wpn.BulletType;
		const int iDamage = wpn.GetDamage();
		const float flRangeModifier = wpn.RangeModifier;
		const BOOL bPistol = wpn.ItemSlot == PISTOL_SLOT;
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
		CBase::m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + flCycleTime + 1.9f;

		wpn.Recoil();
	}

public:
	void FireEvent(const Vector &vecDir)
	{
		int flags = 0;
		/*
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif
		 */

		CFinal &wpn = static_cast<CFinal &>(*this);
		PLAYBACK_EVENT_FULL(flags, CBase::m_pPlayer->edict(), wpn.m_usFire, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, static_cast<int>(CBase::m_pPlayer->pev->punchangle.x * 100),static_cast<int>(CBase::m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);
	}

private:
	template<class ClassToCheck = CFinal>
	auto CheckAccuracyBoundaryMin(ClassToCheck *p) -> decltype(&ClassToCheck::AccuracyMin(), void())
	{
		if (CBase::m_flAccuracy < p->AccuracyMin)
			CBase::m_flAccuracy = p->AccuracyMin;
	}
	auto CheckAccuracyBoundaryMin(...) {}

	template<class ClassToCheck = CFinal>
	auto CheckAccuracyBoundaryMax(ClassToCheck *p) -> decltype(&ClassToCheck::AccuracyMax(), void())
	{
		if (CBase::m_flAccuracy > p->AccuracyMax)
			CBase::m_flAccuracy = p->AccuracyMax;
	}
	auto CheckAccuracyBoundaryMax(...) {}
};