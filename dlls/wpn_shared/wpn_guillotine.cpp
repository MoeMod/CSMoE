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

LINK_ENTITY_TO_CLASS(weapon_guillotine, CGuillotine)

enum guillotine_e
{
	GUILLOTINE_IDLE,
	GUILLOTINE_IDLE_EMPTY,
	GUILLOTINE_SHOOT1,
	GUILLOTINE_DRAW,
	GUILLOTINE_DRAW_EMPTY,
	GUILLOTINE_EIDLE,
	GUILLOTINE_EIDLE_FX,
	GUILLOTINE_CATCH,
	GUILLOTINE_LOST
};

void CGuillotine::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_guillotine");

	Precache();
	m_iId = WEAPON_GUILLOTINE;
	SET_MODEL(ENT(pev), "models/w_guillotine.mdl");
	m_iDefaultAmmo = 10;

	FallInit();

	m_iWeaponState = 0;
	m_flTimeWeaponIdle2 = invalid_time_point;

#ifndef CLIENT_DLL
	m_hAmmo = NULL;
#endif
}

void CGuillotine::Precache(void)
{
#ifndef CLIENT_DLL
	UTIL_PrecacheOther("guillotineAmmo");
#endif
	PRECACHE_MODEL("models/p_guillotine.mdl");
	PRECACHE_MODEL("models/v_guillotine.mdl");
	PRECACHE_MODEL("models/w_guillotine.mdl");

	PRECACHE_SOUND("weapons/guillotine-1.wav");
	PRECACHE_SOUND("weapons/guillotine_catch2.wav");
}

int CGuillotine::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "guillotineAmmo";
	p->iMaxAmmo1 = iMaxClip();
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = -1;
	p->iSlot = 0;
	p->iPosition = 5;
	p->iId = m_iId = WEAPON_GUILLOTINE;
	p->iFlags = 0;
	p->iWeight = AK47_WEIGHT;

	return 1;
}

int CGuillotine::ExtractAmmo(CBasePlayerWeapon* pWeapon)
{
	m_iClip = WEAPON_NOCLIP;
	m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = iMaxClip();
	return CBasePlayerWeapon::ExtractAmmo(pWeapon);
}


BOOL CGuillotine::Deploy(void)
{
	int iAnim;

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0)
		iAnim = GUILLOTINE_DRAW;
	else
		iAnim = GUILLOTINE_DRAW_EMPTY;

#ifndef CLIENT_DLL
	if (m_hAmmo.Get() != NULL)
	{
		iAnim = GUILLOTINE_DRAW_EMPTY;
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

	if (DefaultDeploy("models/v_guillotine.mdl", "models/p_guillotine.mdl", iAnim, m_pPlayer->m_bIsFemale ? "gt" :"guilotine" , UseDecrement() != FALSE))
	{
		m_pPlayer->m_flNextAttack = m_flNextPrimaryAttack = flNextAttack;

		return TRUE;
	}

	return FALSE;
}

void CGuillotine::CatchAmmo(void)
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
	SendWeaponAnim(GUILLOTINE_CATCH);

	m_iWeaponState = 0;

	duration_t flNextAttack = UTIL_WeaponTimeBase() + 1.03s;

	if (flNextAttack > m_flNextPrimaryAttack)
		m_flNextPrimaryAttack = flNextAttack;

	m_flTimeWeaponIdle = flNextAttack;

	EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/guillotine_catch2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
#endif
}

#ifndef CLIENT_DLL
void CGuillotine::Drop(void)
{
	CrashAmmo();

	CBasePlayerWeapon::Drop();
}

void CGuillotine::CrashAmmo(void)
{

	if (m_hAmmo.Get() != NULL)
	{
		((CGuillotineAmmo*)((CBaseEntity*)m_hAmmo))->Crash();		
	}

	m_hAmmo = NULL;

}
#endif


void CGuillotine::ItemPostFrame(void)
{
	CBasePlayerWeapon::ItemPostFrame();

#ifndef CLIENT_DLL

	CGuillotineAmmo* pAmmo = (CGuillotineAmmo*)((CBaseEntity*)m_hAmmo);

	if (m_hAmmo.Get() && pAmmo)
	{
		if (m_iWeaponState == 1 && m_pPlayer->pev->weaponanim != GUILLOTINE_SHOOT1 && pAmmo->IsHeadCutting())
		{
			m_iWeaponState = 2;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase();
		}
	}
	else
	{
		if (m_iWeaponState && m_iWeaponState != 3)
		{
			SendWeaponAnim(GUILLOTINE_LOST);

			m_iWeaponState = 3;
			m_flTimeWeaponIdle2 = gpGlobals->time + 1.36s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.36s;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.36s + 1.2s;
		}
	}
#endif

	if ((m_iWeaponState == 3 && m_flTimeWeaponIdle2 != invalid_time_point && gpGlobals->time > m_flTimeWeaponIdle2) ||
		(m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0 && (m_pPlayer->pev->weaponanim == GUILLOTINE_IDLE_EMPTY || m_pPlayer->pev->weaponanim == GUILLOTINE_DRAW_EMPTY)))
	{
		Deploy();
	}
}

void CGuillotine::PrimaryAttack(void)
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

	CGuillotineAmmo* pAmmo = static_cast<CGuillotineAmmo*>(CBaseEntity::Create("guillotineAmmo", m_pPlayer->GetGunPosition(), UTIL_VecToAngles(gpGlobals->v_forward), m_pPlayer->edict()));

	if (pAmmo)
	{
		pAmmo->m_iTeam = m_pPlayer->m_iTeam;
		m_hAmmo = pAmmo;
		float flSpeed = 1000.0;

		pAmmo->pev->velocity = gpGlobals->v_forward * flSpeed;
		pAmmo->m_flDirectDamage = GetDamageAmountA();
		pAmmo->m_flHeadCutDamage = GetDamageAmountB();
		pAmmo->m_pOwner = m_pPlayer;

		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/guillotine-1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
	}
#endif

	m_iWeaponState = 1;
	SendWeaponAnim(GUILLOTINE_SHOOT1, UseDecrement() != FALSE);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.75s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.73s;
}

void CGuillotine::WeaponIdle(void)
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
			iAnim = GUILLOTINE_EIDLE;
		else
			iAnim = GUILLOTINE_EIDLE_FX;
	}
	else if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0)
	{
		iAnim = GUILLOTINE_IDLE;
	}
	else
	{
		iAnim = GUILLOTINE_IDLE_EMPTY;
	}

	SendWeaponAnim(iAnim, UseDecrement() != FALSE);
}

float CGuillotine::GetDamageAmountA(void)
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

float CGuillotine::GetDamageAmountB(void)
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


int CGuillotine::iMaxClip(void)
{
	int iMaxClip = GUILLOTINE_MAX_AMMO;

	if (m_iWeaponState == 1 || m_iWeaponState == 2)
		iMaxClip = iMaxClip - 1;

	return iMaxClip;
}

}
