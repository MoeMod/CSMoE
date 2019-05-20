/*
zbb_ghost.cpp - CSMoE Gameplay server
Copyright (C) 2019 Moemod Hymei

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
#include "zbb_ghost.h"

#include "util/u_range.hpp"

namespace sv {

class : public IPlayerBuildingDelegate
{
public:
	bool IsBuilding() override
	{
		return false;
	}

	CBaseEntity *CurrentTarget() override
	{
		return nullptr;
	}

	bool IsGhost() override
	{
		return true;
	}
} s_DelegateGhost;
IPlayerBuildingDelegate *g_pDelegateGhostShared = &s_DelegateGhost;

CGhost_ZBB::CGhost_ZBB(CBasePlayer *player) : BasePlayerExtra(player)
{
	m_pPlayer->m_bIsZombie = true;
	m_pPlayer->m_bNotKilled = false;

	m_pPlayer->pev->flags |= EF_NODRAW;

	GiveDefaultItems();
	m_pPlayer->m_bNightVisionOn = false;
	m_pPlayer->ClientCommand("nightvision");

	m_pPlayer->SetNewPlayerModel("");

	// Give Armor
	m_pPlayer->pev->health = m_pPlayer->pev->max_health= 1000;
	m_pPlayer->pev->gravity = 0.6f;
	m_pPlayer->m_iKevlar = ARMOR_TYPE_HELMET;
	m_pPlayer->pev->armorvalue = 100;
}

void CGhost_ZBB::ResetMaxSpeed() const
{
	m_pPlayer->pev->maxspeed = 390;
}

void CGhost_ZBB::Pain_Zombie(int m_LastHitGroup, bool HasArmour)
{
	// n0th1ng here
}

void CGhost_ZBB::DeathSound_Zombie()
{
	// n0th1ng here
}

void CGhost_ZBB::Think()
{
	if (!m_pPlayer->m_bNightVisionOn)
		return;

	for(CBaseEntity *pEntity : moe::range::EntityList<moe::Enumer_ClassName<CBaseEntity>>("func_wall"))
	{
		pEntity->pev->groupinfo |=  (1 << m_pPlayer->entindex());
	}
}

void CGhost_ZBB::PostThink()
{
	if (!m_pPlayer->m_bNightVisionOn)
		return;

	for(CBaseEntity *pEntity : moe::range::EntityList<moe::Enumer_ClassName<CBaseEntity>>("func_wall"))
	{
		pEntity->pev->groupinfo &=  ~(1 << m_pPlayer->entindex());
		//g_engfuncs.pfnSetGroupMask;
	}
}

void CGhost_ZBB::GiveDefaultItems()
{
	m_pPlayer->RemoveAllItems(FALSE);
	m_pPlayer->m_bHasPrimary = false;

	if (!(m_pPlayer->m_flDisplayHistory & DHF_NIGHTVISION))
	{
		m_pPlayer->HintMessage("#Hint_use_nightvision");
		m_pPlayer->m_flDisplayHistory |= DHF_NIGHTVISION;
	}
	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "items/equip_nvg.wav", VOL_NORM, ATTN_NORM);
	m_pPlayer->m_bHasNightVision = true;
	SendItemStatus(m_pPlayer);
}

}
