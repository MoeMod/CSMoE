/***
*
*	Copyright (C) 2019 Itndkl CSAE Build 900
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
#include "wpn_guillotine.h"
#include "weapons.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_guillotineex, CGuillotineex)

enum guillotineex_e
{
	GUILLOTINEEX_IDLE,
	GUILLOTINEEX_IDLE_EMPTY,
	GUILLOTINEEX_SHOOT,
	GUILLOTINEEX_DRAW,
	GUILLOTINEEX_DRAW_EMPTY,
	GUILLOTINEEX_CATCH,
	GUILLOTINEEX_LOST,
	GUILLOTINEEX_EIDLE,
	GUILLOTINEEX_EIDLE_FX,
	GUILLOTINEEX_SHOOT_EMPTY,
	
	
};

void CGuillotineex::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_guillotineex");

	Precache();
	m_iId = WEAPON_GUILLOTINEEX;
	SET_MODEL(ENT(pev), "models/w_guillotineex.mdl");
	m_iDefaultAmmo = 50;

	FallInit();

#ifndef CLIENT_DLL
	for (int i = 0; i < GUILLOTINEEX_MAX_PROJECTILE; i++)
	{
		m_hAmmo[i] = NULL;
	}
#endif
}

void CGuillotineex::Precache(void)
{
#ifndef CLIENT_DLL
	UTIL_PrecacheOther("guillotineexAmmo");
#endif
	PRECACHE_MODEL("models/p_guillotineex.mdl");
	PRECACHE_MODEL("models/v_guillotineex.mdl");
	PRECACHE_MODEL("models/w_guillotineex.mdl");

	PRECACHE_SOUND("weapons/guillotineex_shoot_1_start.wav");
	PRECACHE_SOUND("weapons/guillotineex_shoot-1_exp.wav");
	PRECACHE_SOUND("weapons/guillotineex_shoot-1_end.wav");
	PRECACHE_SOUND("weapons/guillotineex_shoot-2_exp.wav");
	
	PRECACHE_SOUND("weapons/guillotineex_shoot-2.wav");

	PRECACHE_SOUND("weapons/guillotineex_idle1.wav");
	PRECACHE_SOUND("weapons/guillotineex_idle2.wav");
	PRECACHE_SOUND("weapons/guillotineex_catch2.wav");
}

int CGuillotineex::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "guillotineexAmmo";
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = "guillotineexAmmo2";
	p->iMaxAmmo2 = 1;
	p->iMaxClip = GUILLOTINEEX_MAX_AMMO;
	p->iSlot = 0;
	p->iPosition = 5;
	p->iId = m_iId = WEAPON_GUILLOTINEEX;
	p->iFlags = 0;
	p->iWeight = AK47_WEIGHT;

	return 1;
}

BOOL CGuillotineex::Deploy(void)
{
	int iAnim;

	if (m_iClip > 0)
		iAnim = GUILLOTINEEX_DRAW;
	else
		iAnim = GUILLOTINEEX_DRAW_EMPTY;

	duration_t flNextAttack = 1.2s;

	if (m_flNextPrimaryAttack > 1.2s)
		flNextAttack = m_flNextPrimaryAttack;

	if (DefaultDeploy("models/v_guillotineex.mdl", "models/p_guillotineex.mdl", iAnim, m_pPlayer->m_bIsFemale ? "gt" :"guilotine" , UseDecrement() != FALSE))
	{
		m_pPlayer->m_flNextAttack = m_flNextPrimaryAttack = flNextAttack;

		return TRUE;
	}

	return FALSE;
}

void CGuillotineex::CatchAmmo(void)
{
	SendWeaponAnim(GUILLOTINEEX_CATCH);
#ifndef CLIENT_DLL
	int iMaxClip = this->iGetMaxClip();

	if (m_iClip < iMaxClip)
		m_iClip++;

	m_pPlayer->SetAnimation(PLAYER_RELOAD);

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.03s;

	EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/guillotineex_shoot-1_end.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
#endif

	
}

#ifndef CLIENT_DLL
void CGuillotineex::Drop(void)
{
	CrashAmmo();

	CBasePlayerWeapon::Drop();
}

void CGuillotineex::CrashAmmo(void)
{
	for (int i = 0; i < GUILLOTINEEX_MAX_PROJECTILE; i++)
	{
		if (m_hAmmo[i].Get() != NULL)
		{
			((CGuillotineExAmmo*)((CBaseEntity*)m_hAmmo[i]))->Crash();
		}

		m_hAmmo[i] = NULL;
	}
}
#endif


void CGuillotineex::ItemPostFrame(void)
{
#ifndef CLIENT_DLL
	if (!m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] && m_iHitCount >= 15)
	{
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]++;
		pev->skin = 1;
		
	}
#endif
	CBasePlayerWeapon::ItemPostFrame();
}

void CGuillotineex::SecondaryAttack(void)
{
	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] <= 0)
		return;

	m_iHitCount = 0;
#ifndef CLIENT_DLL
	m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;

	if (!m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType])
		pev->skin = 0;

	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	m_pPlayer->pev->effects &= ~EF_MUZZLEFLASH;
	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;

#ifndef CLIENT_DLL
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	CGuillotineExAmmo* pAmmo = CGuillotineExAmmo::CreateAmmo(m_pPlayer->GetGunPosition(), UTIL_VecToAngles(gpGlobals->v_forward), m_pPlayer, 1);

	if (pAmmo)
	{
		pAmmo->m_iTeam = m_pPlayer->m_iTeam;

		float flSpeed = 1000.0;

		pAmmo->pev->velocity = gpGlobals->v_forward * flSpeed;
		pAmmo->m_flDirectDamage = GetDamageAmountA();
		pAmmo->m_flHeadCutDamage = GetDamageAmountB();
		pAmmo->m_pWeapon = this;

		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/guillotineex_shoot-2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
	}
#endif

	SendWeaponAnim(m_iClip ? GUILLOTINEEX_SHOOT : GUILLOTINEEX_SHOOT_EMPTY, UseDecrement() != FALSE);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.75s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.73s;
}

void CGuillotineex::PrimaryAttack(void)
{
	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();

			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
		}

#ifndef CLIENT_DLL
		if (TheBots)
			TheBots->OnEvent(EVENT_WEAPON_FIRED_ON_EMPTY, m_pPlayer);
#endif

		return;
	}

#ifndef CLIENT_DLL
	int iAmmoOutside = 0;
	for (int i = 0; i < GUILLOTINEEX_MAX_PROJECTILE; i++)
	{
		if (m_hAmmo[i])
			iAmmoOutside++;
	}

	if (iAmmoOutside >= GUILLOTINEEX_MAX_PROJECTILE)
		return;
#endif

	m_iClip--;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	m_pPlayer->pev->effects &= ~EF_MUZZLEFLASH;
	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;

#ifndef CLIENT_DLL
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	CGuillotineExAmmo* pAmmo = CGuillotineExAmmo::CreateAmmo(m_pPlayer->GetGunPosition(), UTIL_VecToAngles(gpGlobals->v_forward), m_pPlayer, 0);

	if (pAmmo)
	{
		int iId = 0;
		for (iId = 0; iId < GUILLOTINEEX_MAX_PROJECTILE; iId++)
		{
			if (!m_hAmmo[iId])
				break;
		}
	
		pAmmo->m_iTeam = m_pPlayer->m_iTeam;
		m_hAmmo[iId] = pAmmo;

		float flSpeed = 1000.0;
		
		pAmmo->pev->velocity = gpGlobals->v_forward * flSpeed;
		pAmmo->m_flDirectDamage = GetDamageAmountA();
		pAmmo->m_flHeadCutDamage = GetDamageAmountB();
		pAmmo->m_pWeapon = this;

		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/guillotineex_shoot_1_start.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
	}
#endif

	SendWeaponAnim(m_iClip ? GUILLOTINEEX_SHOOT : GUILLOTINEEX_SHOOT_EMPTY, UseDecrement() != FALSE);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.73s;
}

void CGuillotineex::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1000s;

	int iAnim;

	if (m_iClip > 0)
	{
		iAnim = GUILLOTINEEX_IDLE;
	}
	else
	{
		iAnim = GUILLOTINEEX_IDLE_EMPTY;
	}

	SendWeaponAnim(iAnim, UseDecrement() != FALSE);
}

float CGuillotineex::GetDamageAmountA(void)
{
	float flDamage = 35;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
	{
		if (enhancedzombie.value)
			flDamage = 1000;
		else
			flDamage = 1000;	//200
	}
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 2450;	//2750
#endif
	return flDamage;
}

float CGuillotineex::GetDamageAmountB(void)
{
	float flDamage = 14;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
	{
		if (enhancedzombie.value)
			flDamage = 1050;
		else
			flDamage = 1050;	//360
	}
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 1050;	//2750
#endif
	return flDamage;
}

int CGuillotineex::iGetMaxClip(void)
{
	return GUILLOTINEEX_MAX_AMMO;
}

}
