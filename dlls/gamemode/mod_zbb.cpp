/*
mob_zbb.cpp - CSMoE Gameplay server : Zombie Mod Base-builder
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
#include "game.h"

#include "usercmd.h"
#include "entity_state.h"

#include "mod_zbb.h"

class IPlayerBuildingInterface
{
public:
	virtual ~IPlayerBuildingInterface() = default;
	virtual bool IsBuilding() = 0;
	virtual CBaseEntity *CurrentTarget() = 0;
};

CMod_ZombieBaseBuilder::CMod_ZombieBaseBuilder() // precache
{

}

BOOL CMod_ZombieBaseBuilder::IsAllowedToSpawn(CBaseEntity *pEntity)
{
	if (FStrEq(STRING(pEntity->pev->targetname), "barrier"))
		return FALSE;

	return CMod_Zombi::IsAllowedToSpawn(pEntity);
}

void CMod_ZombieBaseBuilder::UpdateGameMode(CBasePlayer *pPlayer)
{
	MESSAGE_BEGIN(MSG_ONE, gmsgGameMode, nullptr, pPlayer->edict());
	WRITE_BYTE(MOD_ZB1);
	WRITE_BYTE(0); // Reserved. (weapon restriction? )
	WRITE_BYTE(static_cast<int>(maxrounds.value)); // MaxRound (mp_roundlimit)
	WRITE_BYTE(0); // Reserved. (MaxTime?)

	MESSAGE_END();
}

int CMod_ZombieBaseBuilder::AddToFullPack_Post(struct entity_state_s *state, int e, edict_t *ent, edict_t *host, int hostflags, int player, unsigned char *pSet)
{
	CBaseEntity *pHost = CBaseEntity::Instance(host);

	if (!pHost || !pHost->IsPlayer())
		return 0;

	int id = pHost->entindex();
	assert(id >= 1 && id <= 32);

	IPlayerBuildingInterface *pbi = m_BuildingInterfaces[id];
	CBaseEntity *pEntity = pbi->CurrentTarget();

	if (pEntity && ent == pEntity->edict())
	{
		if (pbi->IsBuilding())
		{
			state->rendermode = kRenderTransColor;
			state->renderfx = kRenderFxGlowShell;
			state->rendercolor = { 90, 190, 90 };
			state->renderamt = 125;

		}
		else
		{
			state->rendermode = kRenderTransColor;
			state->renderfx = kRenderFxGlowShell;
			state->rendercolor = { 90, 90, 90 };
			state->renderamt = 125;
		}
	}
	return 1;
}

bool CMod_ZombieBaseBuilder::CanEntityBuild(CBaseEntity *pEntity)
{
	return FClassnameIs(pEntity->edict(), "func_wall");
}

class CPlayerModStrategy_ZBB : public CPlayerModStrategy_ZB1, public IPlayerBuildingInterface
{
public:
	CPlayerModStrategy_ZBB(CBasePlayer *player, CMod_ZombieBaseBuilder *mp) : CPlayerModStrategy_ZB1(player, mp), m_pModZBB(mp)
	{
		m_pModZBB->m_BuildingInterfaces[player->entindex()] = this;
	}

private:
	CMod_ZombieBaseBuilder * const m_pModZBB;

public:
	bool IsBuilding() override
	{
		return m_bIsBuilding;
	}
	CBaseEntity *CurrentTarget() override
	{
		return m_pPointingTarget;
	}

private:
	void CmdStart(struct usercmd_s *cmd, unsigned int random_seed) override
	{
		auto bitsCurButton = cmd->buttons;
		auto bitsOldButton = m_pPlayer->pev->oldbuttons;

		if (m_bIsBuilding)
		{
			if (bitsCurButton & IN_ATTACK)
			{
				m_flBuildDistance += BUILD_PUSHPULLRATE;
				if (m_flBuildDistance > BUILD_MAXDIST)
				{
					m_flBuildDistance = BUILD_MAXDIST;
				}
				bitsCurButton &= ~IN_ATTACK;
			}
			else if (bitsCurButton & IN_ATTACK2)
			{
				m_flBuildDistance -= BUILD_PUSHPULLRATE;
				if (m_flBuildDistance < BUILD_SETDIST)
				{
					m_flBuildDistance = BUILD_SETDIST;
				}
				bitsCurButton &= ~IN_ATTACK2;
			}
			cmd->buttons = bitsCurButton;
		}

		// Press USE
		if (bitsCurButton & ~bitsOldButton & IN_USE)
		{
			Build_Start();
		}
		// Release USE
		else if (m_bIsBuilding && !(bitsCurButton & IN_USE))
		{
			Build_End();
		}

		return CPlayerModStrategy_ZB1::CmdStart(cmd, random_seed);
	}

	void UpdateClientData(int sendweapons, struct clientdata_s *cd, entvars_t *pevOrg) override
	{
		CPlayerModStrategy_ZB1::UpdateClientData(sendweapons, cd, pevOrg);

		// MoeMod : hack the client prediction
		if (IsBuilding())
			cd->m_flNextAttack = gpGlobals->time + 0.01f;
	}

	void OnThink() override
	{
		Build_Update();
	}

	bool m_bCanBuild = true;
	bool m_bIsBuilding = false;
	float m_flBuildDistance;
	CBaseEntity* m_pPointingTarget;
	Vector m_vecOffset;
	float m_flBuildDelay;

	static constexpr auto BUILD_DELAY = 0.75;
	static constexpr auto BUILD_MAXDIST = 720;
	static constexpr auto BUILD_MINDIST = 30;
	static constexpr auto BUILD_SETDIST = 64;
	static constexpr auto BUILD_PUSHPULLRATE = 4.0;
	
	void Build_Start()
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

	void Build_Update()
	{
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

	void Build_End()
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
};

void CMod_ZombieBaseBuilder::InstallPlayerModStrategy(CBasePlayer *player)
{
	std::unique_ptr<CPlayerModStrategy_ZBB> up(new CPlayerModStrategy_ZBB(player, this));
	player->m_pModStrategy = std::move(up);
}