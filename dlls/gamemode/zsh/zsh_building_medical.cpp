#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "schedule.h"
#include "player.h"
#include "monsters.h"
#include "trains.h"

#include "gamemode/mods.h"

#include "zsh_building_medical.h"

namespace sv
{

	void CBuilding_Medical::OnCreate()
	{
		PRECACHE_MODEL("models/zsh_deadcity/bg_medicot.mdl");
		PRECACHE_MODEL("sprites/zombihealer.spr");


		SET_MODEL(m_pBuilding->edict(), "models/zsh_deadcity/bg_medicot.mdl");
		m_vecMins = Vector(-50.0, -26.0, 0.0);
		m_vecMaxs = Vector(50.0, 26.0, 32.0);
		m_pBuilding->m_flFieldOfView = VIEW_FIELD_FULL;


	}

	void CBuilding_Medical::OnSettle()
	{

	}

	void CBuilding_Medical::OnCompleted()
	{

	}

	void CBuilding_Medical::OnThink()
	{
		if (!m_pBuilding->isCompleted()) return;

		m_pBuilding->pev->nextthink = gpGlobals->time + ZSH_Building_Medical_ThinkRate;
		m_pBuilding->StudioFrameAdvance();

		OnRecovery();
	}

	void CBuilding_Medical::OnRecovery()
	{
		float flHealth = 1.0f, flMaxHealth = 1.0f;

		CBaseEntity* pEntity = NULL;
		while ((pEntity = UTIL_FindEntityInSphere(pEntity, m_pBuilding->pev->origin, ZSH_Building_Medical_RecoverRadius)) != NULL)
		{
			if (!pEntity->IsAlive()) continue;
			if (!pEntity->IsPlayer()) continue;

			if (FClassnameIs(pEntity->pev, "monster_entity")) continue;

			flHealth = pEntity->pev->health;
			flMaxHealth = pEntity->pev->max_health;

			if (flHealth >= flMaxHealth)
				continue;

			pEntity->pev->health = min(flMaxHealth * 0.1 + flHealth, flMaxHealth);

			MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL, pEntity->pev);
			WRITE_BYTE(4);
			WRITE_SHORT(pEntity->entindex());
			WRITE_BYTE(2);
			MESSAGE_END();
		}
	}
}