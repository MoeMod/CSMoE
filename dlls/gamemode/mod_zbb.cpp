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
#include "client.h"

#include "usercmd.h"
#include "entity_state.h"

#include "mod_zbb.h"

#include "dlls/gamemode/zbb/zbb_basebuilder.h"
#include "dlls/gamemode/zbb/zbb_ghost.h"
#include "dlls/gamemode/zbb/zbb_zombie.h"

#include "util/u_range.hpp"

namespace sv {

class CZBBCountdownDelegate : public CZB1CountdownDelegate
{
public:
	explicit CZBBCountdownDelegate(CMod_ZombieBaseBuilder *mod) : CZB1CountdownDelegate(mod), m_pModZBB(mod)
	{

	}
	void OnCountdownStart() override
	{
		m_pModZBB->m_GameStatus = ZBB_GS_BUILDING;
		//return CZB1CountdownDelegate::OnCountdownStart();
	}
	void OnCountdownChanged(int iCurrentCount) override
	{
		UTIL_ClientPrintAll(HUD_PRINTCENTER, "Time Remaining for Building Phrase: %s1 Sec", UTIL_dtos1(iCurrentCount)); // #CSO_ZombiSelectCount
		//return CZB1CountdownDelegate::OnCountdownChanged(iCurrentCount);
	}
	void OnCountdownEnd() override
	{
		m_pModZBB->m_GameStatus = ZBB_GS_EVE;
		// respawn everyone
		for (CBasePlayer* pPlayer : moe::range::PlayersList())
		{
			if (pPlayer->IsAlive())
			{
				pPlayer->Spawn();
			}
		}
		// choose ghosts...
		CZB1CountdownDelegate::OnCountdownEnd();
	}

	CMod_ZombieBaseBuilder *m_pModZBB;
};

CMod_ZombieBaseBuilder::CMod_ZombieBaseBuilder() : m_BuildingInterfaces{} // precache
{
	LIGHT_STYLE(0, "i");

	m_Countdown.SetCounts(20);
	std::unique_ptr<CZBBCountdownDelegate> pd(new CZBBCountdownDelegate(this));
	m_Countdown.SetDelegate(std::move(pd));
}

BOOL CMod_ZombieBaseBuilder::IsAllowedToSpawn(CBaseEntity *pEntity)
{
	if (FStrEq(STRING(pEntity->pev->targetname), "barrier"))
		return FALSE;

	return CMod_Zombi::IsAllowedToSpawn(pEntity);
}

void CMod_ZombieBaseBuilder::RestartRound()
{
	for (CBaseEntity *pEntity : moe::range::EntityList<moe::Enumer_ClassName<CBaseEntity>>("func_wall"))
	{
		const Vector vecMins = pEntity->pev->mins;
		const Vector vecMaxs = pEntity->pev->maxs;
		Vector flSquare;
		flSquare[0] = abs(vecMaxs[0] - vecMins[0]) * abs(vecMaxs[1] - vecMins[1]);
		flSquare[1] = abs(vecMaxs[2] - vecMins[2]) * abs(vecMaxs[1] - vecMins[1]);
		flSquare[2] = abs(vecMaxs[0] - vecMins[0]) * abs(vecMaxs[2] - vecMins[2]);

		if(flSquare[0] > 100000.0 || flSquare[1] > 100000.0 || flSquare[2] > 100000.0)
		{
			pEntity->SUB_Remove();
			continue;
		}

		pEntity->pev->origin = {};
		DispatchSpawn(pEntity->edict());
	}
	m_BuildingEntities.clear();
	return CMod_Zombi::RestartRound();
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

	auto id = pHost->entindex();
	assert(id >= 1 && id <= 32);

	auto pbi = m_BuildingInterfaces[id];
	if(pbi)
	{
		if(player)
		{
			CBaseEntity *pEntity = CBaseEntity::Instance(ent);
			auto id2 = pEntity->entindex();
			if(id2 >= 1 && id2 <= 32 && m_BuildingInterfaces[id2]->IsGhost())
			{
				state->effects |= EF_NODRAW;
			}
		}

		CBaseEntity *pCurrentBuildTarget = pbi->CurrentTarget();
		if(pbi->IsGhost())
		{
			CBaseEntity *pEntity = CBaseEntity::Instance(ent);
			if(player)
			{
				// already checked pHost->IsPlayer(), static_cast is safe.
				if(static_cast<CBasePlayer *>(pHost)->m_bNightVisionOn)
				{
					state->rendermode = kRenderNormal;
					state->renderamt = 0;
				}
				else
				{
					state->rendermode = kRenderTransTexture;
					state->renderamt = 1;
				}

				state->renderfx = kRenderFxGlowShell;

				if(pEntity->IsPlayer() && static_cast<CBasePlayer *>(pEntity)->m_bIsZombie)
					state->rendercolor = { 190, 90, 90 };
				else
					state->rendercolor = { 90, 190, 90 };
			}
			else if(CanEntityBuild(pEntity))
			{
				// ghost can see through all entities
				if(static_cast<CBasePlayer *>(pHost)->m_bNightVisionOn)
				{
					state->rendermode = kRenderTransAdd;
					state->renderfx = kRenderFxGlowShell;
					state->rendercolor = { 200, 200, 200 };
					state->renderamt = 36;
				}
				else
				{
					// dont bother when nvg off
				}

				// hack the client prediction... in case looks like player is stuck with walls.
				state->solid = SOLID_NOT;
			}

		}
		else if (pCurrentBuildTarget && ent == pCurrentBuildTarget->edict())
		{
			if(pbi->IsBuilding())
			{
				state->rendermode = kRenderTransColor;
				state->renderfx = kRenderFxGlowShell;
				state->rendercolor = { 90, 190, 90 };
				state->renderamt = 125;
			}
			else{
				state->rendermode = kRenderTransColor;
				state->renderfx = kRenderFxGlowShell;
				state->rendercolor = { 90, 90, 90 };
				state->renderamt = 125;
			}
		}
	}

	return 1;
}

int CMod_ZombieBaseBuilder::ShouldCollide(CBaseEntity *pTouched, CBaseEntity *pOther)
{
	auto f = [this](CBaseEntity *player, CBaseEntity *entity) {

		const auto id = player->entindex();
		assert(id >= 1 && id <= 32);

		const auto pbi = m_BuildingInterfaces[id];
		if (pbi)
		{
			// ghosts can penetrate all players
			if (pbi->IsGhost() && entity && (entity->IsPlayer() || CanEntityBuild(entity)))
				return 0;
		}
		return 1;
	};

	if (pTouched && pTouched->IsPlayer())
		return f(pTouched, pOther);

	if (pOther && pOther->IsPlayer())
		return f(pOther, pTouched);
	
	return IBaseMod::ShouldCollide(pTouched, pOther);
}

bool CMod_ZombieBaseBuilder::CanEntityBuild(CBaseEntity *pEntity)
{
	return pEntity && static_cast<bool>(FClassnameIs(pEntity->edict(), "func_wall"));
}

class CPlayerModStrategy_ZBB : public CPlayerModStrategy_ZB1
{
public:
	CPlayerModStrategy_ZBB(CBasePlayer *player, CMod_ZombieBaseBuilder *mp)
		:   CPlayerModStrategy_ZB1(player, mp),
			m_pModZBB(mp)
	{
		m_pModZBB->m_BuildingInterfaces[player->entindex()] = nullptr;
	}

	~CPlayerModStrategy_ZBB() override
	{
		m_pModZBB->m_BuildingInterfaces[m_pPlayer->entindex()] = nullptr;
	}

private:
	void CmdStart(struct usercmd_s *cmd, unsigned int random_seed) override
	{
		auto &bitsCurButton = cmd->buttons;
		auto &bitsOldButton = m_pPlayer->pev->oldbuttons;

		if(m_pZBB_Delegate)
		{
			if( m_pModZBB->m_BuildingInterfaces[m_pPlayer->entindex()] &&
				m_pModZBB->m_BuildingInterfaces[m_pPlayer->entindex()]->IsGhost() &&
				(bitsCurButton & ~bitsOldButton & IN_ATTACK)
			)
			{
				Ghost_Check_Spawn();
			}
			else
			{
				m_pZBB_Delegate->ButtonEvent(bitsCurButton, bitsOldButton);
			}
		}

		return CPlayerModStrategy_ZB1::CmdStart(cmd, random_seed);
	}

	void UpdateClientData(int sendweapons, struct clientdata_s *cd, entvars_t *pevOrg) override
	{
		CPlayerModStrategy_ZB1::UpdateClientData(sendweapons, cd, pevOrg);

		// MoeMod : hack the client prediction
		if (m_pZBB_Delegate && m_pZBB_Delegate->IsBuilding())
			cd->m_flNextAttack = gpGlobals->time + 0.01f;
	}

	void OnThink() override
	{
		if(m_pModZBB->GetGameStatus() == ZBB_GS_EVE)
		{
			if (m_pPlayer->m_iTeam != TEAM_UNASSIGNED && m_pPlayer->m_iTeam != TEAM_SPECTATOR)
			{
				// respawn after 5 secs
				if (m_pPlayer->pev->deadflag == DEAD_DEAD || m_pPlayer->pev->deadflag == DEAD_RESPAWNABLE)
				{
					m_pPlayer->RoundRespawn();
					BecomeZombie(m_pPlayer->m_iZombieLevel);

					for (CBasePlayer *dest : moe::range::PlayersList())
						SetScoreAttrib(dest, m_pPlayer);
				}
			}
		}

		return CPlayerModStrategy_ZB1::OnThink();
	}

	void OnKilled(entvars_t * pKiller, entvars_t * pInflictor) override
	{
		return CPlayerModStrategy_ZB1::OnKilled(pKiller, pInflictor);
	}

	void OnPostThink() override
	{
		if(m_pZBB_Delegate)
			m_pZBB_Delegate->PostThink();
	}

	bool Ghost_Check_CanRespawn(bool bTip)
	{
		if(!m_pPlayer->IsAlive())
			return false;

		if(Check_HumanNear())
			return false;

		if(IsUserStucked())
			return false;

		return true;
	}

	bool IsUserStucked()
	{
		auto origin = m_pPlayer->pev->origin;
		auto hull = (m_pPlayer->pev->flags & FL_DUCKING) ? head_hull : human_hull;
		TraceResult tr;
		UTIL_TraceHull(origin, origin + Vector(0, 0, 1), dont_ignore_monsters, hull, m_pPlayer->edict(), &tr);

		return tr.fStartSolid || tr.fAllSolid || !tr.fInOpen;
	}

	bool Check_HumanNear() const
	{
		for(CBaseEntity *pEntity : moe::range::EntityList<moe::Enumer_InSphere<CBaseEntity>>(m_pPlayer->pev->origin, 300))
		{
			if(pEntity->IsAlive())
			{
				CBasePlayer *pPlayer = dynamic_ent_cast<CBasePlayer *>(pEntity);
				if(pPlayer)
				{
					if(!pPlayer->m_bIsZombie)
						return true;
				}
			}
		}
		return false;
	}

	void Ghost_Check_Spawn()
	{
		if(!Ghost_Check_CanRespawn(false))
			return;

		auto sp = std::make_shared<CZombieClass_ZBB>(m_pPlayer);
		m_pCharacter = sp;
		m_pZBB_Delegate = sp;

		m_pModZBB->m_BuildingInterfaces[m_pPlayer->entindex()] = g_pDelegateZombieShared;
	}

	void BecomeZombie(ZombieLevel iEvolutionLevel) override
	{
		auto sp = std::make_shared<CGhost_ZBB>(m_pPlayer);
		m_pCharacter = sp;
		m_pZBB_Delegate = sp;

		m_pModZBB->m_BuildingInterfaces[m_pPlayer->entindex()] = g_pDelegateGhostShared;
		m_pPlayer->m_iTeam = TERRORIST;
		TeamChangeUpdate(m_pPlayer, m_pPlayer->m_iTeam);
	}

	void BecomeHuman() override
	{
		auto sp = std::make_shared<CHuman_ZBB>(m_pPlayer, m_pModZBB);
		m_pCharacter = sp;
		m_pZBB_Delegate = sp;

		m_pModZBB->m_BuildingInterfaces[m_pPlayer->entindex()] = &sp->m_Build;
		m_pPlayer->m_iTeam = CT;
		TeamChangeUpdate(m_pPlayer, m_pPlayer->m_iTeam);
	}

private:
	CMod_ZombieBaseBuilder * const m_pModZBB;
	std::shared_ptr<IZombieModeCharacter_ZBB_ExtraDelegate> m_pZBB_Delegate;
};

void CMod_ZombieBaseBuilder::InstallPlayerModStrategy(CBasePlayer *player)
{
	std::unique_ptr<CPlayerModStrategy_ZBB> up(new CPlayerModStrategy_ZBB(player, this));
	player->m_pModStrategy = std::move(up);
}

BOOL CMod_ZombieBaseBuilder::FPlayerCanTakeDamage(CBasePlayer *pPlayer, CBaseEntity *pAttacker)
{
	// skip zb1 infection...
	return IBaseMod::FPlayerCanTakeDamage(pPlayer, pAttacker);
}

}
