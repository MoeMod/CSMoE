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
#include "wpn_at4.h"


#ifndef CLIENT_DLL
#include "soundent.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_at4ex, CAT4EX)

void CAT4EX::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_at4ex");

	Precache();
	m_iId = WEAPON_AT4EX;
	SET_MODEL(ENT(pev), "models/w_at4ex.mdl");

	m_iDefaultAmmo = 1;
	FallInit();
}

void CAT4EX::Precache(void)
{
	CAT4::Precache();

	PRECACHE_MODEL("models/p_at4ex.mdl");
	PRECACHE_MODEL("models/v_at4ex.mdl");
	PRECACHE_MODEL("models/w_at4ex.mdl");
}

int CAT4EX::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "84mmRocket";
	p->iMaxAmmo1 = 5;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 1;
	p->iSlot = 0;
	p->iPosition = 2;
	p->iId = m_iId = WEAPON_AT4EX;
	p->iFlags = 0;
	p->iWeight = SCOUT_WEIGHT;

	return 1;
}

BOOL CAT4EX::Deploy(void)
{
	return DefaultDeploy("models/v_at4ex.mdl", "models/p_at4ex.mdl", ANIM_DRAW, "at4", UseDecrement() != FALSE);
}

void CAT4EX::PrimaryAttack(void)
{
	if (m_pPlayer->pev->fov != 90)
		AT4Fire(true);
	else
		AT4Fire(false);
}

}
