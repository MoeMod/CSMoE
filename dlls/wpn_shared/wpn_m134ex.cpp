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
#include "wpn_m134ex.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

enum m134ex_e
{
	M134_IDLE1,
	M134_DRAW,
	M134_RELOAD,
	M134_FIRE_READY,
	M134_SHOOT,
	M134_FIRE_AFTER,
	M134_FIRE_CHANGE,
	M134_IDLE_CHANGE,
	M134_FIRE_LOOP,
};

LINK_ENTITY_TO_CLASS(weapon_m134ex, CM134ex)

void CM134ex::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_m134ex");

	Precache();
	m_iId = WEAPON_M249;
	SET_MODEL(ENT(pev), "models/w_m134ex.mdl");

	m_iDefaultAmmo = M134_DEFAULT_GIVE;
	m_flAccuracy = 0.0;
	m_iShotsFired = 0;

	FallInit();
}

void CM134ex::Precache(void)
{
	PRECACHE_MODEL("models/v_m134ex.mdl");
	PRECACHE_MODEL("models/w_m134ex.mdl");
	PRECACHE_MODEL("models/p_m134ex.mdl");

	PRECACHE_SOUND("weapons/m134ex-1.wav");
	PRECACHE_SOUND("weapons/m134_boxout.wav");
	PRECACHE_SOUND("weapons/m134_boxin.wav");
	PRECACHE_SOUND("weapons/m134_chain.wav");
	PRECACHE_SOUND("weapons/m134_coverup.wav");
	PRECACHE_SOUND("weapons/m134_coverdown.wav");
	PRECACHE_SOUND("weapons/m134_clipoff.wav");
	PRECACHE_SOUND("weapons/m134_clipon.wav");
	PRECACHE_SOUND("weapons/m134_spindown.wav");
	PRECACHE_SOUND("weapons/m134_spinup.wav");
	PRECACHE_SOUND("weapons/m134_pinpull.wav");
	PRECACHE_SOUND("weapons/m134ex_spin.wav");

	PRECACHE_MODEL("models/shell762_m134.mdl");
	PRECACHE_MODEL("models/shell762_m134_01.mdl");
	m_usFireM134ex = PRECACHE_EVENT(1, "events/m134ex.sc");
}

int CM134ex::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "556NatoBox";
	p->iMaxAmmo1 = MAX_AMMO_556NATOBOX;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = M134_DEFAULT_GIVE;
	p->iSlot = 0;
	p->iPosition = 4;
	p->iId = m_iId = WEAPON_M249;
	p->iFlags = 0;
	p->iWeight = M249_WEIGHT;

	return 1;
}

BOOL CM134ex::Deploy(void)
{
	m_flAccuracy = 0.0;
	m_iShotsFired = 0;
	iShellOn = 1;
	m_iWeaponState = WPNSTATE_M134_IDLE;

	return DefaultDeploy("models/v_m134ex.mdl", "models/p_m134ex.mdl", M134_DRAW, "m134ex", UseDecrement() != FALSE);
}

void CM134ex::PrimaryAttack(void)
{
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		M134exFire(0.065 + (0.75) * m_flAccuracy, 0.06155s, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 140)
		M134exFire(0.045 + (0.095) * m_flAccuracy, 0.06155s, FALSE);
	else
		M134exFire((0.04) * m_flAccuracy, 0.06155s, FALSE);
}

void CM134ex::ItemPostFrame()
{
	if (m_pPlayer->pev->button & IN_ATTACK)
	{
		if (!m_iClip && m_iM134State == WPNSTATE_M134_SPINNING)
		{
			SendWeaponAnim(M134_FIRE_AFTER, UseDecrement() != FALSE);
			m_pPlayer->pev->maxspeed = M134_MAX_SPEED;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
			m_iM134State = WPNSTATE_M134_IDLE;
			m_pPlayer->pev->button &= ~IN_ATTACK;
		}
		if (m_iM134State == WPNSTATE_M134_IDLE)
		{
			if (m_flNextPrimaryAttack <= 0.0s && m_iClip)
			{
				SendWeaponAnim(M134_FIRE_READY, UseDecrement() != FALSE);
				EMIT_SOUND_DYN(this->edict(), CHAN_WEAPON, "weapons/m134_spinup.wav", VOL_NORM, ATTN_NORM, 0, 94);
				m_pPlayer->pev->maxspeed = M134_FIRE_MAX_SPEED;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.06s;
				m_iM134State = WPNSTATE_M134_SPIN_UP;
				m_pPlayer->pev->button &= ~IN_ATTACK;
			}
		}

		if (m_iM134State == WPNSTATE_M134_SPIN_UP && m_flNextPrimaryAttack <= 0.0s)
		{
			m_pPlayer->pev->maxspeed = M134_FIRE_MAX_SPEED;
			m_iM134State = WPNSTATE_M134_SPINNING;
		}

		if (m_iM134State == WPNSTATE_M134_SPIN_DOWN && m_iClip)
		{
			SendWeaponAnim(M134_FIRE_CHANGE, UseDecrement() != FALSE);
			EMIT_SOUND_DYN(this->edict(), CHAN_WEAPON, "weapons/m134_spinup.wav", VOL_NORM, ATTN_NORM, 0, 94);
			m_pPlayer->pev->maxspeed = M134_FIRE_MAX_SPEED;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
			m_iM134State = WPNSTATE_M134_SPIN_UP;
			m_pPlayer->pev->button &= ~IN_ATTACK;
		}
	}
	else if(!(m_pPlayer->pev->button & IN_ATTACK2))
	{
		if (m_iM134State == WPNSTATE_M134_SPIN_UP)
		{
			SendWeaponAnim(M134_IDLE_CHANGE, UseDecrement() != FALSE);
			EMIT_SOUND_DYN(this->edict(), CHAN_WEAPON, "weapons/m134_spindown.wav", VOL_NORM, ATTN_NORM, 0, 94);
			m_pPlayer->pev->maxspeed = M134_MAX_SPEED;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.5s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
			m_iM134State = WPNSTATE_M134_IDLE;
			m_pPlayer->pev->button &= ~IN_ATTACK;
		}

		if (m_iM134State == WPNSTATE_M134_SPINNING)
		{
			SendWeaponAnim(M134_FIRE_AFTER, UseDecrement() != FALSE);
			m_pPlayer->pev->maxspeed = M134_MAX_SPEED;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
			m_iM134State = WPNSTATE_M134_SPIN_DOWN;
			m_pPlayer->pev->button &= ~IN_ATTACK;
		}

		if (m_iM134State == WPNSTATE_M134_SPIN_DOWN && m_flNextPrimaryAttack <= 0.0s)
		{
			m_pPlayer->pev->maxspeed = M134_MAX_SPEED;
			m_iM134State = WPNSTATE_M134_IDLE;
			m_pPlayer->pev->button &= ~IN_ATTACK;
		}
	}
	if (m_pPlayer->pev->button & IN_ATTACK2 && m_iClip)
	{
		if (m_iM134State == WPNSTATE_M134_IDLE)
		{
			if (m_flNextPrimaryAttack <= 0.0s)
			{
				SendWeaponAnim(M134_FIRE_READY, UseDecrement() != FALSE);
				m_pPlayer->pev->maxspeed = M134_FIRE_MAX_SPEED;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60.0s;
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.8s;
				m_iM134State = WPNSTATE_M134_SPIN_UP;
				m_pPlayer->pev->button &= ~IN_ATTACK2;
			}
		}

		if ((m_iM134State == WPNSTATE_M134_SPINNING || m_iM134State == WPNSTATE_M134_SPIN_UP) && m_flNextPrimaryAttack <= 0.0s)
		{
			if (m_pPlayer->pev->weaponanim != M134_FIRE_LOOP)
			{
				SendWeaponAnim(M134_FIRE_LOOP, UseDecrement() != FALSE);
			}
			m_pPlayer->pev->maxspeed = M134_FIRE_MAX_SPEED;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60.0s;
			m_iM134State = WPNSTATE_M134_SPIN_UP;
			m_pPlayer->pev->button &= ~IN_ATTACK2;
		}
	}
	return CBasePlayerWeapon::ItemPostFrame();
}

void CM134ex::M134exFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	m_bDelayFire = true;
	m_iShotsFired++;
	m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 160.0) + 0.4;

	if (m_flAccuracy > 2.5)
		m_flAccuracy = 0.0;

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

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_556MM, GetDamage(), 0.97, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireM134ex, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.6s;

	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		KickBack(0.025, 0.025, 0.045, 0.015, 4.55, 5.15, 1);
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
		KickBack(0.025, 0.025, 0.045, 0.015, 3.55, 2.15, 1);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		KickBack(0.025, 0.025, 0.045, 0.015, 2.25, 0.95, 1);
	else
		KickBack(0.025, 0.025, 0.045, 0.015, 2.55, 1.15, 1);
}

void CM134ex::Reload(void)
{
	if (m_pPlayer->ammo_556natobox <= 0)
		return;

	if (DefaultReload(M249_MAX_CLIP, M134_RELOAD, 5.0s))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		m_flAccuracy = 0.0;
		m_bDelayFire = false;
		m_iShotsFired = 0;
	}
}

void CM134ex::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);
	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
	SendWeaponAnim(M134_IDLE1, UseDecrement() != FALSE);
}

float CM134ex::GetDamage() const
{
	float flDamage = 37.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 40.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 40.0f;
#endif
	return flDamage;
}
}
