
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "zbs_map_manager.h"
#include "zs_subs.h"
#include "gamerules.h"
#include "../dlls/gamemode/mod_zbs.h"

namespace sv {

LINK_ENTITY_TO_CLASS(zombiespawn, CZombieSpawn);

void CZombieSpawn::Spawn()
{
	return CPointEntity::Spawn();
}

void CZombieSpawn::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "master"))
	{
		pev->netname = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "id"))
	{
		m_iId = Q_atoi(pkvd->szValue);
	}
	else
		CPointEntity::KeyValue(pkvd);
}

BOOL CZombieSpawn::IsTriggered(CBaseEntity *pEntity)
{
	BOOL master = UTIL_IsMasterTriggered(pev->netname, pEntity);

	return master;
}

LINK_ENTITY_TO_CLASS(zbsbreak, CZBSBreak);

extern ZBSData g_ZBSData;

void CZBSBreak::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "hmdmgratio"))
	{
		m_flHumanDamageRatio = Q_atof(pkvd->szValue);
	}
	else if (FStrEq(pkvd->szKeyName, "zbdmgratio"))
	{
		m_flZombiDamageRatio = Q_atof(pkvd->szValue);
	}

	return CBreakable::KeyValue(pkvd);
}

void CZBSBreak::Spawn()
{
	Precache();

	if (pev->spawnflags & SF_BREAK_TRIGGER_ONLY)
		pev->takedamage = DAMAGE_NO;
	else
		pev->takedamage = DAMAGE_YES;

	m_flHealth = pev->health;
	pev->solid = SOLID_BSP;
	pev->movetype = MOVETYPE_PUSH;
	m_angle = pev->angles.y;
	pev->angles.y = 0;

	// HACK:  matGlass can receive decals, we need the client to know about this
	//  so use class to store the material flag
	if (m_Material == matGlass)
	{
		pev->playerclass = 1;
	}

	//set size and link into world.
	SET_MODEL(ENT(pev), STRING(pev->model));

	SetTouch(&CZBSBreak::BreakTouch);

	// Only break on trigger
	if (pev->spawnflags & SF_BREAK_TRIGGER_ONLY)
	{
		SetTouch(NULL);
	}

	// Flag unbreakable glass as "worldbrush" so it will block ALL tracelines
	if (!IsBreakable() && pev->rendermode != kRenderNormal)
	{
		pev->flags |= FL_WORLDBRUSH;
	}
}

void CZBSBreak::BreakTouch(CBaseEntity* pOther)
{
	if (pev->health < 2.0)
	{
		pev->takedamage = DAMAGE_NO;
		pev->deadflag = DEAD_DEAD;
		pev->effects = EF_NODRAW;

		Die();

		if (m_flDelay == 0.0s)
		{
			m_flDelay = 0.1s;
		}

		pev->nextthink = pev->ltime + m_flDelay;

	}

	CBreakable::BreakTouch(pOther);
}



int CZBSBreak::TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType)
{
	CBaseEntity *entAttacker = CBaseEntity::Instance(pevAttacker);
	if (entAttacker->IsPlayer())
	{
		CBasePlayer *attacker = static_cast<CBasePlayer *>(entAttacker);
		if (attacker->m_bIsZombie)
			flDamage *= m_flZombiDamageRatio;
		else
		{
			flDamage *= m_flHumanDamageRatio;

			auto strategy = dynamic_cast<PlayerModStrategy_ZBS*>(attacker->m_pModStrategy.get());
			if (strategy)
			{
				strategy->m_flWallTakeDamage += flDamage;

				if (g_ZBSData.wallBonusDmg && g_ZBSData.wallBonusPoint)
				{
					if (strategy->m_flWallTakeDamage >= g_ZBSData.wallBonusDmg)
					{
						attacker->AddAccount(g_ZBSData.wallBonusPoint * int(strategy->m_flWallTakeDamage / g_ZBSData.wallBonusDmg));
						strategy->m_flWallTakeDamage = 0;
					}
				}
				else
				{
					if (strategy->m_flWallTakeDamage >= 300)
					{
						attacker->AddAccount(200);
						strategy->m_flWallTakeDamage = 0;
					}
				}
				
			}
		}
		

	}



	return CBreakable::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

LINK_ENTITY_TO_CLASS(zbstrigger, CZBSTrigger);

extern std::vector<TriggerData> g_ZBSTriggerData;
extern std::vector<WaveData> g_ZBSWaveData;

void CZBSTrigger::Spawn()
{
	m_iRound = -1;
	m_iCurentRound = g_pGameRules->m_iTotalRoundsPlayed;
	InitTrigger();
	SetTouch(&CZBSTrigger::TriggerTouch);

	if (pev->team > CT || pev->team < UNASSIGNED)
	{
		ALERT(at_console, "Bad team number (%i) in func_buyzone\n", pev->team);
		pev->team = UNASSIGNED;
	}
}

void CZBSTrigger::TriggerThink()
{

}

void CZBSTrigger::TriggerTouch(CBaseEntity* pOther)
{

	if (!pOther->IsPlayer())
		return;

	CMod_ZombieScenario* pModZBS = dynamic_cast<CMod_ZombieScenario*>(g_pModRunning);

	if (pModZBS)
	{
		if (pModZBS->FRoundStarted() && !pModZBS->m_bRoundTerminating)
		{
			if (!g_ZBSTriggerData.empty())
			{
				for (auto trigger = g_ZBSTriggerData.begin(); trigger != g_ZBSTriggerData.end(); trigger++)
				{
					if (trigger->map_triggerid == m_iTriggerId)
					{
						if (!strcmp(trigger->type.c_str(), "g"))
						{
							CBasePlayer* p = static_cast<CBasePlayer*>(pOther);
							if (p)
							{
								if (std::find(m_iPlayerInTrigger.begin(), m_iPlayerInTrigger.end(), p->entindex()) == m_iPlayerInTrigger.end())
									m_iPlayerInTrigger.push_back(p->entindex());
							}
							//todo clear round;

							float ratio = trigger->destratio / 100.0;

							int NumDeadCT, NumDeadTerrorist, NumAliveTerrorist, NumAliveCT;
							g_pGameRules->InitializePlayerCounts(NumAliveTerrorist, NumAliveCT, NumDeadTerrorist, NumDeadCT);

							if (float(m_iPlayerInTrigger.size()) / float(NumAliveCT) >= ratio)
							{
								if (pModZBS)
									pModZBS->HumanWin();

								SetTouch(nullptr);
								break;
							}
						}
						else
						{
							//zombi spawn or msg once round
							if (m_iRound == g_pGameRules->m_iTotalRoundsPlayed)
								break;

							m_iRound = g_pGameRules->m_iTotalRoundsPlayed;

							for (auto wave = g_ZBSWaveData.begin(); wave != g_ZBSWaveData.end(); wave++)
							{
								if (trigger->waveid == wave->waveID)
								{
									if (wave->wavestate != WAVE_OPEN && !strcmp(trigger->type.c_str(), "w"))
									{
										(*wave).wavestate = WAVE_OPEN;
										(*wave).m_flNextWave = gpGlobals->time + wave->startTime;
										(*wave).m_flEndWave = gpGlobals->time + wave->endTime;
									}
									else if (wave->wavestate != WAVE_KILL && !strcmp(trigger->type.c_str(), "k"))
									{
										//todo kill all wave zombi
									}
								}

								if (wave->waveID == trigger->endwave && wave->wavestate != WAVE_END)
								{
									(*wave).wavestate = WAVE_END;
									(*wave).m_flNextWave = invalid_time_point;
									(*wave).m_flEndWave = invalid_time_point;
								}
							}
						}
						break;
					}
				}
			}

		}
	}

	
}

void CZBSTrigger::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "id"))
	{
		m_iTriggerId = Q_atoi(pkvd->szValue);
	}
	else
		CBaseTrigger::KeyValue(pkvd);
}


}
