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
#include "wpn_m79.h"


#ifndef CLIENT_DLL
#include "soundent.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_m79g, CM79G)

void CM79G::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_m79g");

	Precache();
	m_iId = WEAPON_M79G;
	SET_MODEL(ENT(pev), "models/w_m79g.mdl");

	m_iDefaultAmmo = 6;
	FallInit();
}

void CM79G::Precache(void)
{
	m_usFireM79 = PRECACHE_EVENT(1, "events/m79.sc");

	PRECACHE_MODEL("sprites/muzzleflash12.spr");

	PRECACHE_SOUND("weapons/m79-1.wav");

	PRECACHE_MODEL("models/p_m79g.mdl");
	PRECACHE_MODEL("models/v_m79g.mdl");
	PRECACHE_MODEL("models/w_m79g.mdl");
}

int CM79G::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "40mmGrenade2";
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 6;
	p->iSlot = 1;
	p->iPosition = 2;
	p->iId = m_iId = WEAPON_M79G;
	p->iFlags = 0;
	p->iWeight = GLOCK18_WEIGHT;

	return 1;
}

BOOL CM79G::Deploy(void)
{
	m_fMaxSpeed = 250;

#ifndef CLIENT_DLL
	m_pPlayer->UpdateShieldCrosshair(false);
#endif

	return DefaultDeploy("models/v_m79g.mdl", "models/p_m79g.mdl", ANIM_DRAW, "onehanded", UseDecrement() != FALSE);
}

void CM79G::M79Fire()
{
	if (m_iClip <= 0)
		return;

	m_iClip--;

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
#ifndef CLIENT_DLL
	m_pPlayer->m_iExtraSoundTypes = bits_SOUND_DANGER;

	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	UTIL_MakeVectors(m_pPlayer->pev->v_angle);

	Vector vecVelocity = gpGlobals->v_forward * 800 + Vector(0, 0, 100);

	CM79Rocket* pRocket = CM79Rocket::Create(1, m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 16 + gpGlobals->v_right * 8 + gpGlobals->v_up * -8, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev), m_pPlayer->m_iTeam);

	if (pRocket)
	{
		pRocket->pev->velocity = vecVelocity;
		pRocket->pev->owner = m_pPlayer->edict();
		pRocket->m_iTeam = m_pPlayer->m_iTeam;
	}

	if (m_iClip)
		EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/m79-1.wav", 0.7, ATTN_NORM, 0, PITCH_NORM);
#endif

	int flags;

#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireM79, 0, g_vecZero, g_vecZero, 0.0, 0.0, int(m_pPlayer->pev->punchangle.x * 100), int(m_pPlayer->pev->punchangle.y * 100), m_iClip != 0, FALSE);


	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 2.8s;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.8s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0s;

#ifndef CLIENT_DLL
	if (m_iClip <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif

	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 2, 5);
	else
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 2, 5);

}

}
