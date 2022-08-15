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
#include "wpn_m3dragon.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_m3dragon, CM3Dragon)

void CM3Dragon::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_m3dragon");

	Precache();
	m_iId = WEAPON_M3DRAGON;
	SET_MODEL(ENT(pev), "models/w_m3dragon.mdl");
	m_iDefaultAmmo = 8;
	m_flNextPlaySound = invalid_time_point;
	FallInit();
}

void CM3Dragon::Precache(void)
{
	PRECACHE_MODEL("models/v_m3dragon.mdl");
	PRECACHE_MODEL("models/w_m3dragon.mdl");
	PRECACHE_MODEL("models/p_m3dragon.mdl");

	PRECACHE_SOUND("weapons/m3dragon-2.wav");
	PRECACHE_SOUND("weapons/m3dragon_fire_loop.wav");

	m_iShellId = m_iShell = PRECACHE_MODEL("models/shotgunshell.mdl");

	m_usFireM3Dragon = PRECACHE_EVENT(1, "events/m3dragon.sc");
}

int CM3Dragon::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "buckshot";
	p->iMaxAmmo1 = MAX_AMMO_BUCKSHOT;
	p->pszAmmo2 = "m3dragonshot";
	p->iMaxAmmo2 = 1;
	p->iMaxClip = 8;
	p->iSlot = 0;
	p->iPosition = 5;
	p->iId = m_iId = WEAPON_M3DRAGON;
	p->iFlags = 0;
	p->iWeight = M3_WEIGHT;

	return 1;
}

int CM3Dragon::ExtractAmmo(CBasePlayerWeapon* pWeapon)
{
	m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 0;
	return CBasePlayerWeapon::ExtractAmmo(pWeapon);
}

BOOL CM3Dragon::Deploy(void)
{
	PLAYBACK_EVENT_FULL(FEV_HOSTONLY, ENT(m_pPlayer->pev), m_usFireM3Dragon, 0, g_vecZero, g_vecZero, 0, 0, 1, 0, FALSE, m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]);
	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType])
		m_flNextPlaySound = gpGlobals->time + 0.1s;
	return DefaultDeploy("models/v_m3dragon.mdl", "models/p_m3dragon.mdl", m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] > 0 ? M3DRAGON_SECONDARY_DRAW : M3DRAGON_DRAW, "shotgun", UseDecrement() != FALSE);
}

void CM3Dragon::SecondaryAttack(void)
{
	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] <= 0)
		return;

#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

	SendWeaponAnim(M3DRAGON_SECONDARY_SHOOT, UseDecrement() != FALSE);

	EMIT_SOUND_DYN(ENT(this->pev), CHAN_AUTO, "weapons/m3dragon-2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

	m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;
	pev->iuser1 = 0;

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.875s;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.875s;

	if (m_iClip)
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5s;
	else
		m_flTimeWeaponIdle = 0.875s;

	PLAYBACK_EVENT_FULL(FEV_HOSTONLY, ENT(m_pPlayer->pev), m_usFireM3Dragon, 0, g_vecZero, g_vecZero, 0, 0, 1, 0, FALSE, m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]);
	m_flNextPlaySound = invalid_time_point;
#ifndef CLIENT_DLL
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	CM3DragonCannon* pCannon = CM3DragonCannon::Create(0, vecSrc, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev), m_pPlayer->m_iTeam);

	if (pCannon)
	{
		pCannon->pev->velocity = gpGlobals->v_forward * 1000;
	}
#endif
}

void CM3Dragon::ItemPostFrame(void)
{
	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] && m_flNextPlaySound != invalid_time_point && m_flNextPlaySound < gpGlobals->time)
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/m3dragon_fire_loop.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);
		m_flNextPlaySound = gpGlobals->time + 1.0s;
	}
	
	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] <= 0 && pev->iuser1 >= 8)
	{
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 1;
#ifndef CLIENT_DLL
		MESSAGE_BEGIN(MSG_ONE, gmsgAmmoPickup, NULL, m_pPlayer->pev);
		WRITE_BYTE(CBasePlayer::GetAmmoIndex("m3dragonshot"));
		WRITE_BYTE(1);
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
		WRITE_BYTE(1);
		WRITE_BYTE(pev->iuser1);
		MESSAGE_END();
#endif
		m_flNextPlaySound = gpGlobals->time + 0.1s;
		PLAYBACK_EVENT_FULL(FEV_HOSTONLY, ENT(m_pPlayer->pev), m_usFireM3Dragon, 0, g_vecZero, g_vecZero, 0, 0, 1, 0, FALSE, m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]);
	}
	return CBasePlayerWeapon::ItemPostFrame();
}

void CM3Dragon::PrimaryAttack(void)
{
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15s;
		return;
	}

	if (m_iClip <= 0)
	{
		Reload();

		if (m_iClip == 0)
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
	m_pPlayer->FireBullets(10, m_pPlayer->GetGunPosition(), gpGlobals->v_forward, Vector(0.0675, 0.0675, 0), 3000, BULLET_PLAYER_BUCKSHOT, 0, GetDamage());
#endif

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFireM3Dragon, 0, g_vecZero, g_vecZero, 0, 0, 0, 0, FALSE, FALSE);

	if (m_iClip)
		m_flPumpTime = UTIL_WeaponTimeBase() + 0.5s;

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	if (m_iClip)
		m_flPumpTime = UTIL_WeaponTimeBase() + 0.5s;

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.875s;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.875s;

	if (m_iClip)
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5s;
	else
		m_flTimeWeaponIdle = 0.875s;

	m_fInSpecialReload = 0;

	if (m_pPlayer->pev->flags & FL_ONGROUND)
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 4, 6);
	else
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 8, 11);

	m_pPlayer->m_flEjectBrass = gpGlobals->time + 0.45s;
}

void CM3Dragon::Holster(int skiplocal)
{
	CBasePlayerWeapon::Holster(skiplocal);

	StopIdleSound();
}

void CM3Dragon::StopIdleSound()
{
#ifndef CLIENT_DLL
	int player = m_pPlayer->entindex();

	MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->edict());
	WRITE_BYTE(79);
	WRITE_SHORT(player);
	WRITE_SHORT(CHAN_WEAPON);
	WRITE_STRING("weapons/m3dragon_fire_loop.wav");
	MESSAGE_END();
#endif
}

void CM3Dragon::Reload(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip == M3DRAGON_MAX_CLIP)
		return;

	if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase())
		return;

	if (!m_fInSpecialReload)
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		SendWeaponAnim(m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] > 0 ? M3DRAGON_SECONDARY_BEFORE_RELOAD : M3DRAGON_BEFORE_RELOAD, UseDecrement() != FALSE);

		m_fInSpecialReload = 1;
		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.53s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.53s;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.53s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.53s;
	}
	else if (m_fInSpecialReload == 1)
	{
		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_fInSpecialReload = 2;
		SendWeaponAnim(m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] > 0 ? M3DRAGON_SECONDARY_INSERT : M3DRAGON_INSERT, UseDecrement() != FALSE);

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

void CM3Dragon::WeaponIdle(void)
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
			if (m_iClip != M3DRAGON_MAX_CLIP && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
			{
				Reload();
			}
			else
			{
				SendWeaponAnim(m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] > 0 ? M3DRAGON_SECONDARY_AFTER_RELOAD : M3DRAGON_AFTER_RELOAD, UseDecrement() != FALSE);

				m_fInSpecialReload = 0;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
			}
		}
		else
			SendWeaponAnim(m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] > 0 ? M3DRAGON_SECONDARY_READY : M3DRAGON_IDLE, UseDecrement() != FALSE);
	}
}

int CM3Dragon::GetDamage()
{
	int flDamage = 19;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
	{
		if (enhancedzombie.value)
			flDamage = 179;
		else
			flDamage = 120;
	}		
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 309;
#endif
	return flDamage;
}

}
