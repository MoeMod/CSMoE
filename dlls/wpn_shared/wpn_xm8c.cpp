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
#include "wpn_xm8c.h"

LINK_ENTITY_TO_CLASS(weapon_xm8c, CXM8Carbine)

void CXM8Carbine::Spawn(void)
{
	Base::Spawn();

	current_ammo = m_iDefaultAmmo;

	m_pLink = nullptr;
	m_iSwing = 0;
}

#ifndef CLIENT_DLL
void CXM8Carbine::AttachToPlayer(CBasePlayer *pPlayer)
{
	Base::AttachToPlayer(pPlayer);

	// find the linked weapon
	while (!m_pLink)
	{
		for (int i = 0; i < MAX_ITEM_TYPES; ++i)
		{
			CBasePlayerItem *pItem = m_pPlayer->m_rgpPlayerItems[i];
			while (pItem != NULL)
			{
				if (!Q_stricmp(pItem->pszName(), "weapon_xm8s"))
				{
					m_pLink = pItem;

					m_pLink->m_iSwing = 1;
					m_iSwing = 0;
				}

				pItem = pItem->m_pNext;
			}
		}
		if(!m_pLink)
			pPlayer->GiveNamedItem("weapon_xm8s");

	} 
}

void CXM8Carbine::ItemPostFrame()
{
	m_iSwing = 0;
	current_ammo = m_iClip;
	if (m_pLink)
		m_pLink->m_iSwing = 1;
	return Base::ItemPostFrame();
}

CXM8Carbine::~CXM8Carbine()
{
	CBaseEntity *pOther = m_pLink;
	if (pOther && pOther->m_pLink == this)
	{
		pOther->m_pLink = m_pLink = nullptr;
		pOther->SUB_Remove();
	}
}
#endif

void CXM8Carbine::Precache(void)
{
	Base::Precache();
	PRECACHE_SOUND("weapons/xm8_carbine.wav");
}

BOOL CXM8Carbine::Deploy(void)
{
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;

	if (m_iSwing)
	{
		DefaultDeploy("models/v_xm8.mdl", "models/p_xm8.mdl", XM8_CHANGE_CARBINE, "rifle", UseDecrement() != FALSE);
		m_pPlayer->m_flNextAttack = 6.0f;
		// sync ammo
		if (m_pLink)
			m_iClip = m_pLink->current_ammo;
	}
	else
	{
		DefaultDeploy("models/v_xm8.mdl", "models/p_xm8.mdl", CARBINE_XM8_DRAW, "rifle", UseDecrement() != FALSE);
		m_pPlayer->m_flNextAttack = 1.5f;
	}
	return TRUE;
}

float CXM8Carbine::GetMaxSpeed(void)
{
	if (m_pPlayer->m_iFOV == 90)
		return 235;

	return 200;
}
