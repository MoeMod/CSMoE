/*
turbulent1.cpp
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
#include "wpn_turbulent1.h"
#ifndef CLIENT_DLL
#include "gamemode/mod_zb1.h"
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif
LINK_ENTITY_TO_CLASS(weapon_turbulent1, CTurbulent1)

int CTurbulent1::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "Turbulent1Ammo";
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 150;
	p->iSlot = 1;
	p->iPosition = 2;
	p->iId = m_iId = WEAPON_TURBULENT1;
	p->iFlags = 0;
	p->iWeight = GLOCK18_WEIGHT;

	return 1;
}

void CTurbulent1::Precache()
{
	PRECACHE_MODEL("sprites/muzzleflash57.spr");
	PRECACHE_SOUND("weapons/turbulent1_idle.wav");
	return Base::Precache();
}

void CTurbulent1::Spawn()
{
	m_flTimePlaySound = invalid_time_point;
	m_flNextReload = invalid_time_point;
	Base::Spawn();
}

void CTurbulent1::ItemPostFrame()
{
	if (m_fInReload && m_flNextReload != invalid_time_point && m_flNextReload < gpGlobals->time)
	{
		// complete the reload.
		int j = MaxClip - m_iClip;

		// Add them to the clip
		m_iClip += j;
		m_fInReload = FALSE;
		m_flNextReload = invalid_time_point;
		m_iWeaponState = 0;
	}

	if (m_fInReload)
		return;

	if (m_iWeaponState)
	{	
		Reload();
	}


	return CBasePlayerWeapon::ItemPostFrame();
}


BOOL CTurbulent1::Deploy()
{
	m_flAccuracy = AccuracyDefault;

	m_fInReload = FALSE;

	return DefaultDeploy(V_Model, P_Model, m_iClip > 0 ? ANIM_DRAW : ANIM_DRAW_EMPTY, "onehanded", UseDecrement() != FALSE, 1.3s);
}
#ifndef CLIENT_DLL
int CTurbulent1::DefaultReload(int iClipSize, int iAnim, duration_t fDelay, int body)
{
	duration_t fNewDelay = m_pPlayer->m_pModStrategy->OnDefaultReload_Pre(this, fDelay);
	if (body)
		this->m_flNextPrimaryAttack = m_flNextSecondaryAttack = fNewDelay;
	else
		m_pPlayer->m_flNextAttack = fNewDelay;
	ReloadSound();
	SendWeaponAnimEx(iAnim, UseDecrement() ? 1 : 0, fDelay / fNewDelay);

	m_fInReload = TRUE;
	m_flTimeWeaponIdle = fDelay + 0.5s;

	MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
	WRITE_BYTE(41);
	WRITE_BYTE(1);
	WRITE_COORD(float(fNewDelay / 1.0s));		// framerate
	MESSAGE_END();

	return TRUE;
}
#else
BOOL CTurbulent1::DefaultReload(int iClipSize, int iAnim, duration_t fDelay, int body)
{
	if (!m_pPlayer->m_pActiveItem)
		return FALSE;

	if (body)
		this->m_flNextPrimaryAttack = m_flNextSecondaryAttack = fDelay;
	else
		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + fDelay;

	//!!UNDONE -- reload sound goes here !!!

	if (gHUD.m_ZBZ.CheckHasSkill(Skill_RapidReloader)) {
		SendWeaponAnimEx(iAnim, UseDecrement(), 1 / max(0.0f, 1.0f - (0.32f + 0.022f * (gHUD.m_Scoreboard.m_iTeamScore_T + gHUD.m_Scoreboard.m_iTeamScore_CT))));
	}
	else {
		SendWeaponAnim(iAnim, UseDecrement());
	}

	m_fInReload = TRUE;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + fDelay + 0.5s;
	return TRUE;
}
#endif

void CTurbulent1::Reload(void)
{
	if (!m_iWeaponState)
		return;

	if (DefaultReload(150, m_iClip > 0 ? ANIM_RELOAD : ANIM_RELOAD_EMPTY, 4.03s, 1))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		m_flNextReload = gpGlobals->time + 4.03s;
		m_flAccuracy = AccuracyDefault;
		m_iShotsFired = 0;
	}
}


void CTurbulent1::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimePlaySound < gpGlobals->time)
	{
		m_flTimePlaySound = gpGlobals->time + 1s;
		if (m_iClip > 0)
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/turbulent1_idle.wav", VOL_NORM, ATTN_NORM, 0, 94);
	}

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
	SendWeaponAnim(m_iClip > 0 ? ANIM_IDLE1 : ANIM_IDLE1_EMPTY, UseDecrement() != FALSE);
}



void CTurbulent1::Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	if (m_flLastFire != invalid_time_point)
	{
		m_flAccuracy -= (0.325 - ((gpGlobals->time - m_flLastFire) / 1s)) * 0.285;

		if (m_flAccuracy > 0.9)
			m_flAccuracy = 0.9;
		else if (m_flAccuracy < 0.525)
			m_flAccuracy = 0.525;
	}

	m_flLastFire = gpGlobals->time;

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

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 4096, 1, BULLET_PLAYER_9MM, GetDamage(), 0.76, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), m_iClip != 0, FALSE);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;

	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		KickBack(1.2f, 0.8f, 0.5f, 0.5f, 7.0f, 5.0f, 4);
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
		KickBack(0.7f, 0.5f, 0.325f, 0.065f, 4.0f, 3.0f, 5);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		KickBack(0.55f, 0.3f, 0.25f, 0.035f, 3.0f, 2.0f, 6);
	else
		KickBack(0.6f, 0.35f, 0.3f, 0.05f, 3.25f, 2.0f, 5);
}

}