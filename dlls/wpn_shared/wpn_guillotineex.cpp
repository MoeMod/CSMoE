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

#define BTE_VER 1

enum guillotineex_e
{
	guillotineex_IDLE,
	guillotineex_IDLE_EMPTY,
	guillotineex_SHOOT1,
	guillotineex_DRAW,
	guillotineex_DRAW_EMPTY,
	guillotineex_CATCH,
	guillotineex_LOST,
	guillotineex_EIDLE,
	guillotineex_EIDLE_FX,
	guillotineex_SHOOT_EMPTY,
	
	
};

void CGuillotineex::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_guillotineex");

	Precache();
	m_iId = WEAPON_GUILLOTINEEX;
	SET_MODEL(ENT(pev), "models/w_guillotineex.mdl");
	m_iDefaultAmmo = 50;

	FallInit();
	m_iState = 0;
	m_iWeaponState = 0;
	m_flTimeWeaponIdle2 = invalid_time_point;

#ifndef CLIENT_DLL
	m_hAmmo = NULL;
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

	PRECACHE_SOUND("weapons/guillotineex-1.wav");
	PRECACHE_SOUND("weapons/guillotineex_catch2.wav");
}

int CGuillotineex::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "guillotineexAmmo";
	p->iMaxAmmo1 = iMaxClip();
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = -1;
	p->iSlot = 0;
	p->iPosition = 5;
	p->iId = m_iId = WEAPON_GUILLOTINEEX;
	p->iFlags = 0;
	p->iWeight = AK47_WEIGHT;

	return 1;
}

int CGuillotineex::ExtractAmmo(CBasePlayerWeapon* pWeapon)
{
	m_iClip = WEAPON_NOCLIP;
	m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = iMaxClip();
	return CBasePlayerWeapon::ExtractAmmo(pWeapon);
}


BOOL CGuillotineex::Deploy(void)
{
	int iAnim;

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0)
		iAnim = guillotineex_DRAW;
	else
		iAnim = guillotineex_DRAW_EMPTY;

#ifndef CLIENT_DLL
	if (m_hAmmo.Get() != NULL)
	{
		iAnim = guillotineex_DRAW_EMPTY;
	}
	else
	{
		m_iWeaponState = 0;
		m_flTimeWeaponIdle2 = invalid_time_point;
	}
#endif

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
#ifndef CLIENT_DLL
	int iMaxClip = this->iMaxClip();

	if (m_iWeaponState == 1 || m_iWeaponState == 2)
		iMaxClip++;

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] < iMaxClip)
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]++;

#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
	SendWeaponAnim(guillotineex_CATCH);

	m_iWeaponState = 0;

	duration_t flNextAttack = UTIL_WeaponTimeBase() + 1.03s;

	if (flNextAttack > m_flNextPrimaryAttack)
		m_flNextPrimaryAttack = flNextAttack;

	m_flTimeWeaponIdle = flNextAttack;

	EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/guillotineex_catch2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
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

	if (m_hAmmo.Get() != NULL)
	{
		((CGuillotineexAmmo*)((CBaseEntity*)m_hAmmo))->Crash();	
	}

	m_hAmmo = NULL;

}
#endif


void CGuillotineex::ItemPostFrame(void)
{
	CBasePlayerWeapon::ItemPostFrame();

#ifndef CLIENT_DLL

	CGuillotineexAmmo* pAmmo = (CGuillotineexAmmo*)((CBaseEntity*)m_hAmmo);

	if (m_hAmmo.Get() && pAmmo)
	{
		if (m_iWeaponState == 1 && m_pPlayer->pev->weaponanim != guillotineex_SHOOT1 && pAmmo->IsHeadCutting())
		{
			m_iWeaponState = 2;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase();
		}
	}
	else
	{
		if (m_iWeaponState && m_iWeaponState != 3)
		{
			SendWeaponAnim(guillotineex_LOST);

			m_iWeaponState = 3;
			m_flTimeWeaponIdle2 = gpGlobals->time + 1.36s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.36s;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.36s + 1.2s;
		}
	}
#endif

	if ((m_iWeaponState == 3 && m_flTimeWeaponIdle2 != invalid_time_point && gpGlobals->time > m_flTimeWeaponIdle2) ||
		(m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0 && (m_pPlayer->pev->weaponanim == guillotineex_IDLE_EMPTY || m_pPlayer->pev->weaponanim == guillotineex_DRAW_EMPTY)))
	{
		Deploy();
	}
}

void CGuillotineex::SecondaryAttack(void)
{
	m_iState = (m_iState + 1) % 2;

	SendWeaponAnim(guillotineex_DRAW, UseDecrement() != FALSE);
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.36s;
	m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0s;
}

void CGuillotineex::PrimaryAttack(void)
{
	if (m_iWeaponState)
		return;

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
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

	m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	m_pPlayer->pev->effects &= ~EF_MUZZLEFLASH;
	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;

#ifndef CLIENT_DLL
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	CGuillotineexAmmo* pAmmo = static_cast<CGuillotineexAmmo*>(CBaseEntity::Create("guillotineexAmmo", m_pPlayer->GetGunPosition(), UTIL_VecToAngles(gpGlobals->v_forward), m_pPlayer->edict()));

	if (pAmmo)
	{
		pAmmo->m_iTeam = m_pPlayer->m_iTeam;
		m_hAmmo = pAmmo;
		pAmmo->pev->solid = m_iState ? SOLID_BBOX : SOLID_TRIGGER;
		float flSpeed = 1000.0;
		
		pAmmo->pev->velocity = gpGlobals->v_forward * flSpeed;
		pAmmo->m_flDirectDamage = GetDamageAmountA();
		pAmmo->m_flHeadCutDamage = GetDamageAmountB();
		pAmmo->m_pOwner = m_pPlayer;

		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/guillotineex-1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
	}
#endif

	m_iWeaponState = 1;
	SendWeaponAnim(guillotineex_SHOOT1, UseDecrement() != FALSE);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.75s;
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

	if (m_iWeaponState)
	{
		if (m_iWeaponState == 1)
			iAnim = guillotineex_IDLE;
		else
			iAnim = guillotineex_IDLE;
	}
	else if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0)
	{
		iAnim = guillotineex_IDLE;
	}
	else
	{
		iAnim = guillotineex_IDLE_EMPTY;
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
			flDamage = 400;
		else
			flDamage = 400;	//200
	}
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 3025;	//2750
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
			flDamage = 432;
		else
			flDamage = 432;	//360
	}
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 432;	//2750
#endif
	return flDamage;
}


int CGuillotineex::iMaxClip(void)
{
	int iMaxClip = GUILLOTINEEX_MAX_AMMO;

	if (m_iWeaponState == 1 || m_iWeaponState == 2)
		iMaxClip = iMaxClip - 1;

	return iMaxClip;
}

}
