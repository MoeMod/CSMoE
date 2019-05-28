/*
zbb_basebuilder.cpp - CSMoE Gameplay server
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
#include "zbb_basebuilder.h"

namespace sv {

//static constexpr float BUILD_DELAY = 0.75;
static constexpr float BUILD_MAXDIST = 720;
static constexpr float BUILD_MINDIST = 30;
static constexpr float BUILD_SETDIST = 64;
static constexpr float BUILD_PUSHPULLRATE = 4.0;

void PlayerBuildingDelegate::Build_End()
{
	if (!m_bIsBuilding)
		return;

	CBaseEntity * pEntity = m_pPointingTarget;

	const Vector vecAngles = pEntity->pev->angles;
	DispatchSpawn(pEntity->edict());
	pEntity->pev->angles = vecAngles;

	m_bIsBuilding = false;
	m_pPointingTarget = nullptr;
	m_pModZBB->m_BuildingEntities.erase(pEntity);
}

void PlayerBuildingDelegate::Build_Update()
{
	m_bCanBuild = m_pModZBB->GetGameStatus() == ZBB_GS_BUILDING;
	if (!m_bCanBuild || !m_pPlayer->IsAlive())
	{
		Build_End();
		return;
	}

	const Vector vecStart = m_pPlayer->GetGunPosition();

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	const Vector vecDir = gpGlobals->v_forward;
	const Vector vecEnd = vecStart + vecDir * BUILD_MAXDIST;

	TraceResult tr{};
	UTIL_TraceLine(vecStart, vecEnd, dont_ignore_monsters, m_pPlayer->edict(), &tr);

	if (tr.flFraction >= 1)
	{
		UTIL_TraceHull(vecStart, vecEnd, dont_ignore_monsters, head_hull, m_pPlayer->edict(), &tr);
	}

	if (m_bIsBuilding)
	{
		const Vector vecLook = tr.vecEndPos;

		float flLength = (vecLook - vecStart).Length();
		if (flLength == 0.0) flLength = 1.0;
		const Vector vecNewOrigin = (vecStart + (vecLook - vecStart) * m_flBuildDistance / flLength) + m_vecOffset;

		g_engfuncs.pfnSetOrigin(m_pPointingTarget->edict(), vecNewOrigin);
	}
	else
	{
		CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);
		if (!pEntity || pEntity->IsAlive() || !m_pModZBB->CanEntityBuild(pEntity))
		{
			m_pPointingTarget = nullptr;
			return;
		}

		m_pPointingTarget = pEntity;
		m_vecOffset = pEntity->pev->origin - tr.vecEndPos;
		m_flBuildDistance = (tr.vecEndPos - vecStart).Length();

		if (m_flBuildDistance < BUILD_MINDIST)
			m_flBuildDistance = BUILD_SETDIST;
	}
}

void PlayerBuildingDelegate::Build_Start()
{

	if (!m_bCanBuild || !m_pPlayer->IsAlive())
		return;

	if (m_bIsBuilding)
		return;

	CBaseEntity* pEntity = m_pPointingTarget;

	if (!pEntity || pEntity->IsAlive() || !m_pModZBB->CanEntityBuild(pEntity))
		return;

	if (m_pModZBB->m_BuildingEntities.find(pEntity) != m_pModZBB->m_BuildingEntities.end())
		return;

	m_bIsBuilding = true;
	m_pModZBB->m_BuildingEntities.emplace(pEntity, this);
}

void PlayerBuildingDelegate::AddPushRate(float delta)
{
	m_flBuildDistance += delta;
	if (m_flBuildDistance > BUILD_MAXDIST)
	{
		m_flBuildDistance = BUILD_MAXDIST;
	}
	if (m_flBuildDistance < BUILD_SETDIST)
	{
		m_flBuildDistance = BUILD_SETDIST;
	}
}

void CHuman_ZBB::ButtonEvent(unsigned short &bitsCurButton, int bitsOldButton)
{
	if (m_Build.IsBuilding())
	{
		if (bitsCurButton & IN_ATTACK)
		{
			m_Build.AddPushRate(BUILD_PUSHPULLRATE);
			bitsCurButton &= ~IN_ATTACK;
		}
		else if (bitsCurButton & IN_ATTACK2)
		{
			m_Build.AddPushRate(-BUILD_PUSHPULLRATE);
			bitsCurButton &= ~IN_ATTACK2;
		}
	}

	// Press USE
	if (bitsCurButton & ~bitsOldButton & IN_USE)
	{
		m_Build.Build_Start();
	}
		// Release USE
	else if (m_Build.IsBuilding() && !(bitsCurButton & IN_USE))
	{
		m_Build.Build_End();
	}
}

}
