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
#include "wpn_m2.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

	enum M2_e
	{
		M2_IDLEA,
		M2_IDLEB,
		M2_DRAWA,
		M2_DRAW_EMPTY,
		M2_CHANGEA,
		M2_CHANGEA_EMPTY,
		M2_CHANGEB,
		M2_CHANGEB_EMPTY,
		M2_RELOADA,
		M2_RELOADB,
		M2_SHOOTA,
		M2_SHOOTB,
		M2_IDLEA_EMPTY,
		M2_IDLEB_EMPTY,
	};

	LINK_ENTITY_TO_CLASS(weapon_m2, CM2)

		void CM2::Spawn(void)
	{
		pev->classname = MAKE_STRING("weapon_m2");

		Precache();
		m_iId = WEAPON_M4A1;
		SET_MODEL(ENT(pev), "models/w_m2.mdl");

		m_iDefaultAmmo = M2_DEFAULT_GIVE;
		m_flAccuracy = 0.35;
		m_iShotsFired = 0;
		m_bDelayFire = true;

		FallInit();
	}

	void CM2::Precache(void)
	{
		PRECACHE_MODEL("models/v_m2.mdl");
		PRECACHE_MODEL("models/w_m2.mdl");
		PRECACHE_MODEL("models/p_m2_1.mdl");
		PRECACHE_MODEL("models/p_m2_2.mdl");

		PRECACHE_SOUND("weapons/m2-1.wav");
		PRECACHE_SOUND("weapons/m2-2.wav");
		PRECACHE_SOUND("weapons/m2_clipin.wav");
		PRECACHE_SOUND("weapons/m2_clipin2.wav");
		PRECACHE_SOUND("weapons/m2_clipout.wav");
		PRECACHE_SOUND("weapons/m2_cliplock.wav");
		PRECACHE_SOUND("weapons/m2_close.wav");
		PRECACHE_SOUND("weapons/m2_draw.wav");
		PRECACHE_SOUND("weapons/m2_foley1.wav");
		PRECACHE_SOUND("weapons/m2_foley2.wav");
		PRECACHE_SOUND("weapons/m2_open.wav");

		m_iShell = PRECACHE_MODEL("models/rshell.mdl");
		m_usFireM2 = PRECACHE_EVENT(1, "events/m2.sc");
	}

	int CM2::GetItemInfo(ItemInfo *p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "556NatoBox";
		p->iMaxAmmo1 = M2_DEFAULT_GIVE;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = M2_DEFAULT_GIVE;
		p->iSlot = 0;
		p->iPosition = 6;
		p->iId = m_iId = WEAPON_M4A1;
		p->iFlags = 0;
		p->iWeight = M249_WEIGHT;

		return 1;
	}

	BOOL CM2::Deploy(void)
	{
		m_bDelayFire = true;
		iShellOn = 1;
		m_flAccuracy = 0.35;
		m_iShotsFired = 0;

		if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
			return DefaultDeploy("models/v_m2.mdl", "models/p_m2.mdl", M2_DRAWA, "m249", UseDecrement() != FALSE);
		else
			return DefaultDeploy("models/v_m2.mdl", "models/p_m2.mdl", M2_DRAWA, "m249", UseDecrement() != FALSE);
	}
	void CM2::SwitchThink(void)
	{
		m_iWeaponState &= ~WPNSTATE_M4A1_SILENCED;
		m_pPlayer->ResetMaxSpeed();
		SetThink(nullptr);

	}
	void CM2::SecondaryAttack(void)
	{
		if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
		{
			
			if (m_iClip)
				SendWeaponAnim(M2_CHANGEB, UseDecrement() != FALSE);
			else
			{
				SendWeaponAnim(M2_CHANGEB_EMPTY, UseDecrement() != FALSE);
				strcpy(m_pPlayer->m_szAnimExtention, "m249");
			}
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 3.8s;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 3.8s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.8s;
			fTime = gpGlobals->time;
			SetThink(&CM2::SwitchThink);
			pev->nextthink = gpGlobals->time + 3.8s;
		}
		else
		{
			if ((m_pPlayer->pev->v_angle.x > 30.0 || m_pPlayer->pev->v_angle.x < -30.0) || (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND)))
				return;
			m_iWeaponState |= WPNSTATE_M4A1_SILENCED;
#ifndef CLIENT_DLL
			g_engfuncs.pfnSetClientMaxspeed(ENT(m_pPlayer->pev), 1);
#endif
			if (m_iClip)
				SendWeaponAnim(M2_CHANGEA, UseDecrement() != FALSE);
			else
			{
				SendWeaponAnim(M2_CHANGEA_EMPTY, UseDecrement() != FALSE);
				strcpy(m_pPlayer->m_szAnimExtention, "m249");
			
			}	
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 4.7s;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 4.7s;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 4.7s;
			m_pPlayer->ResetMaxSpeed();
		}
	}

	void CM2::PrimaryAttack(void)
	{
		if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
		{
			M2Fire((0.02) * m_flAccuracy, 0.1s, FALSE);
		}
		else
		{
			if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
				M2Fire(0.1 + (0.3) * m_flAccuracy, 0.3s, FALSE);
			else if (m_pPlayer->pev->velocity.Length2D() > 140)
				M2Fire(0.06 + (0.095) * m_flAccuracy, 0.3s, FALSE);
			else if (m_pPlayer->pev->velocity.Length2D() <= 140)
				M2Fire(0.1 + (0.06) * m_flAccuracy, 0.3s, FALSE);
			else
				M2Fire((0.06) * m_flAccuracy, 0.3s, FALSE);
		}
	}

	void CM2::M2Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
	{
		m_bDelayFire = true;
		m_iShotsFired++;
		if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
		{
			m_flAccuracy = ((float)(m_iShotsFired * m_iShotsFired * m_iShotsFired) / 180) + 0.3;
			if (m_flAccuracy > 0.9)
				m_flAccuracy = 0.2;
		}
		else
		{
			m_flAccuracy = ((float)(m_iShotsFired * m_iShotsFired * m_iShotsFired) / 160) + 0.5;
			if (m_flAccuracy > 0.9)
				m_flAccuracy = 0.35;
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
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecDir;

		if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
		{
			vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_556MM, GetDamageB(), 0.99, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);
		}
		else
		{
			vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_556MM, GetDamageA(), 0.99, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

		}
		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireM2, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), (m_iWeaponState & WPNSTATE_M4A1_SILENCED) ? true : false, FALSE);
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;

		if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
			KickBack(0.6, 0.3, 0.012, 0.018, 1.5, 1.5, 1.0);
		else
		{
			if (m_pPlayer->pev->velocity.Length2D() > 0)
				KickBack(2.5, 2.3, 2.1, 1.1, 5.4, 4.6, 7.0);
			else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
				KickBack(2.775, 2.84, 2.4, 1.3, 6.0, 5.0, 5.0);
			else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
				KickBack(2.1, 1.9, 1.35, 0.625, 3.8, 3.2, 7.0);
			else
				KickBack(2.3, 2.2, 2.0, 1.0, 4.0, 3.5, 6.0);
		}
	}

	void CM2::Reload(void)
	{
		if (m_pPlayer->ammo_556natobox <= 0)
			return;

		int iAnim;

		if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
		{
			iAnim = M2_RELOADB;
			if (DefaultReload(M2_DEFAULT_GIVE, iAnim, 5.73s))
			{
#ifndef CLIENT_DLL
				m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
				m_flAccuracy = 0.2;
				m_iShotsFired = 0;
				m_bDelayFire = false;
			}
		}
		else
		{
			iAnim = M2_RELOADA;
			if (DefaultReload(M2_DEFAULT_GIVE, iAnim, 6.1s))
			{
#ifndef CLIENT_DLL
				m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
				m_flAccuracy = 0.35;
				m_iShotsFired = 0;
				m_bDelayFire = false;
			}
		}

	}

	void CM2::WeaponIdle(void)
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;

		if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
		{
			m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_m2_2.mdl");
			if (m_iClip)
				SendWeaponAnim(M2_IDLEB, UseDecrement() != FALSE);
			else
				SendWeaponAnim(M2_IDLEB_EMPTY, UseDecrement() != FALSE);

		}
		else
		{
			m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_m2_1.mdl");
			if (m_iClip)
				SendWeaponAnim(M2_IDLEA, UseDecrement() != FALSE);
			else
				SendWeaponAnim(M2_IDLEA_EMPTY, UseDecrement() != FALSE);
		}
	}

	float CM2::GetDamageA()
	{
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZBS)
			return 100.0f;
		if (g_pModRunning->DamageTrack() == DT_ZB)
			return 65.0f;
#endif
		return 40.0f;
	}

	float CM2::GetDamageB()
	{
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZBS)
			return 90.0f;
		if (g_pModRunning->DamageTrack() == DT_ZB)
			return 40.0f;
#endif
		return 30.0f;
	}

	float CM2::GetMaxSpeed()
	{
		if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
			return 1.0;
		else
			return 160.0;
	}

	BOOL CM2::CanDrop()
	{
		if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
			return FALSE;
		else
			return TRUE;
	}

	BOOL CM2::CanHolster()
	{
		if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
			return FALSE;
		else
			return TRUE;
	}

}