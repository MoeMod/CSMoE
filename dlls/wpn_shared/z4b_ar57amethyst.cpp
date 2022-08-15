/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"


#include "z4b_ar57amethyst.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

	LINK_ENTITY_TO_CLASS(z4b_ar57amethyst, CAR57AMETHYST)

	void CAR57AMETHYST::Spawn(void)
	{
		pev->classname = MAKE_STRING("z4b_ar57amethyst");

		Precache();
		m_iId = WEAPON_Z4B_AR57AMETHYST;
		SET_MODEL(ENT(pev), "models/w_ar57_amethyst.mdl");

		m_iDefaultAmmo = 9999;
		m_flAccuracy = 0.2;
		m_iShotsFired = 0;

		FallInit();
	}

	void CAR57AMETHYST::Precache(void)
	{
		PRECACHE_MODEL("models/v_ar57_amethyst.mdl");
		PRECACHE_MODEL("models/p_ar57_amethyst.mdl");
		PRECACHE_MODEL("models/w_ar57_amethyst.mdl");

		PRECACHE_SOUND("weapons/balrig3-1.wav");
		PRECACHE_SOUND("weapons/balrig3-1.wav");

		m_iShell = PRECACHE_MODEL("models/rshell.mdl");
		m_usFireAR57AMETHYST = PRECACHE_EVENT(1, "events/ak47.sc");
	}

	int CAR57AMETHYST::GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "ammo_amethyst";
		p->iMaxAmmo1 = 9999;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = 40;
		p->iSlot = 0;
		p->iPosition = 1;
		p->iId = m_iId = WEAPON_Z4B_AR57AMETHYST;
		p->iFlags = 0;
		p->iWeight = AK47_WEIGHT;

		return 1;
	}

	BOOL CAR57AMETHYST::Deploy(void)
	{
		m_flAccuracy = 0.2;
		m_iShotsFired = 0;
		iShellOn = 1;
		return DefaultDeploy("models/v_ar57_amethyst.mdl", "models/p_ar57_amethyst.mdl", 2, "ak47", UseDecrement() != FALSE);

	}

	void CAR57AMETHYST::PrimaryAttack(void)
	{
		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			AR57AMETHYSTFire(0.04 + (0.4) * m_flAccuracy, 0.10505s, FALSE);
		else if (m_pPlayer->pev->velocity.Length2D() > 140)
			AR57AMETHYSTFire(0.04 + (0.07) * m_flAccuracy, 0.10505s, FALSE);
		else
			AR57AMETHYSTFire((0.0275), 0.10505s, FALSE);
	}

	void CAR57AMETHYST::AR57AMETHYSTFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
	{
		m_bDelayFire = true;
		m_iShotsFired++;
		m_flAccuracy = ((float)(m_iShotsFired * m_iShotsFired * m_iShotsFired) / 200.0) + 0.35;

		if (m_flAccuracy > 1.25)
			m_flAccuracy = 1.25;

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
		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_762MM, GetDamage(), 0.98, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireAR57AMETHYST, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);

		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.9s;

		if (m_pPlayer->pev->velocity.Length2D() > 0)
			KickBack(1.5, 0.45, 0.225, 0.05, 6.5, 2.5, 7);
		else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			KickBack(2.0, 1.0, 0.5, 0.35, 9.0, 6.0, 5);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			KickBack(0.9, 0.35, 0.15, 0.025, 5.5, 1.5, 9);
		else
			KickBack(1.0, 0.375, 0.175, 0.0375, 5.75, 1.75, 8);
	}
	
	void CAR57AMETHYST::Reload(void)
	{


		if (DefaultReload(40, 1, 3.63s))
		{
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
			m_flAccuracy = 0.2;
			m_iShotsFired = 0;
			m_bDelayFire = false;
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = 9999;
		}
	}

	void CAR57AMETHYST::WeaponIdle(void)
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
		SendWeaponAnim(0, UseDecrement() != FALSE);
	}

	float CAR57AMETHYST::GetDamage() const
	{
		float flDamage = 36.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 45.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 73.0f;
#endif
		return flDamage;
	}

} // namespace cl/sv
