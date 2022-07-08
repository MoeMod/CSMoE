

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "player.h"
#include "gamerules.h"
#include "training_gamerules.h"
#include "hltv.h"
#include "game.h"
#include "trains.h"
#include "vehicle.h"
#include "globals.h"

#include "bot_include.h"

#include "util/u_range.hpp"

namespace sv {

// Begin defusing the bomb

void SupplyBoxRushState::OnEnter(CCSBot *me)
{
	me->DestroyPath();
	m_entity = nullptr;
}

// Defuse the bomb

void SupplyBoxRushState::OnUpdate(CCSBot *me)
{
	if (!m_entity)
	{
		auto startPos = me->pev->origin;
		auto costf = [startPos](CBaseEntity* supplybox) {
			ShortestPathCost cost;
			return NavAreaTravelDistance(TheNavAreaGrid.GetNavArea(&startPos), TheNavAreaGrid.GetNavArea(&supplybox->pev->origin), cost);
		};
		moe::range::EntityList<moe::Enumer_ClassName<CBaseEntity>> candidaterange("supplybox");
		std::vector<CBaseEntity*> candidate(candidaterange.begin(), candidaterange.end());
		CBaseEntity* supplybox = nullptr;
		auto iter = std::min_element(candidate.begin(), candidate.end(), [costf](auto a, auto b) { return costf(a) < costf(b); });
		if (iter != candidate.end())
			supplybox = *iter;
		m_entity = supplybox;
	}
	CBaseEntity* supplybox = m_entity;


	if (supplybox != NULL)
	{
		if (!me->HasPath())
		{
			// build a path to the supplybox
			if (me->ComputePath(TheNavAreaGrid.GetNavArea(&supplybox->pev->origin), &supplybox->pev->origin, FASTEST_ROUTE) == false)
			{
				me->PrintIfWatched("Fetch supplybox pathfind failed\n");

				// go Hunt instead of Idle to prevent continuous re-pathing to inaccessible bomb
				me->Hunt();
				//return;
			}
		}
	}
	else
	{
		// someone picked up the supplybox
		me->PrintIfWatched("Supplybox not found\n");
		me->Idle();
		return;
	}

	// look around
	//me->UpdateLookAround();

	if (me->UpdatePathMovement() != CCSBot::PROGRESSING)
		me->Idle();
}

void SupplyBoxRushState::OnExit(CCSBot *me)
{
	// nothing to do
	m_entity = nullptr;
}

}
