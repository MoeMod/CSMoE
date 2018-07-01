#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "game.h"
#include "globals.h"
#include "trains.h"
#include "bmodels.h"

#include "mod_tdm.h"

class CMultiplayGameMgrHelper : public IVoiceGameMgrHelper
{
public:
	virtual bool		CanPlayerHearPlayer(CBasePlayer *pListener, CBasePlayer *pTalker)
	{
		if (g_pGameRules->IsTeamplay())
		{
			if (g_pGameRules->PlayerRelationship(pListener, pTalker) != GR_TEAMMATE)
			{
				return false;
			}
		}

		return true;
	}
};
static CMultiplayGameMgrHelper g_GameMgrHelper;

CMod_TeamDeathMatch::CMod_TeamDeathMatch()
{
	m_VoiceGameMgr.Init(&g_GameMgrHelper, gpGlobals->maxClients);

	m_flIntermissionEndTime = 0;
	m_flIntermissionStartTime = 0;

	m_iMaxRoundsWon = (int)maxkills.value;
}

void CMod_TeamDeathMatch::Think(void)
{
	m_VoiceGameMgr.Update(gpGlobals->frametime);\

	///// Check game rules /////

	if (CheckGameOver())   // someone else quit the game already
		return;
	
	if (CheckTimeLimit())
		return;

	m_iTotalRoundsPlayed = m_iNumCTWins + m_iNumTerroristWins; // hack
	/*if (CheckMaxRounds())
		return;
	*/
	if (CheckWinLimit())
		return;
	
	
	if (IsFreezePeriod())
	{
		CheckFreezePeriodExpired();
	}

	CheckLevelInitialized();

	if (gpGlobals->time > m_tmNextPeriodicThink)
	{
		CheckRestartRound();
		m_tmNextPeriodicThink = gpGlobals->time + 1.0f;

		if (g_psv_accelerate->value != 5.0f)
		{
			CVAR_SET_FLOAT("sv_accelerate", 5.0);
		}

		if (g_psv_friction->value != 4.0f)
		{
			CVAR_SET_FLOAT("sv_friction", 4.0);
		}

		if (g_psv_stopspeed->value != 75.0f)
		{
			CVAR_SET_FLOAT("sv_stopspeed", 75.0);
		}

		
		m_iMaxRoundsWon = (int)maxkills.value;

		if (m_iMaxRoundsWon < 0)
		{
			m_iMaxRoundsWon = 0;
			CVAR_SET_FLOAT("mp_maxkills", 0);
		}
	}

	for (int iIndex = 1; iIndex <= gpGlobals->maxClients; ++iIndex)
	{
		CBaseEntity *entity = UTIL_PlayerByIndex(iIndex);

		if (!entity)
			continue;

		CBasePlayer *player = static_cast<CBasePlayer *>(entity);

		if (player->pev->deadflag != DEAD_DEAD && player->pev->deadflag != DEAD_RESPAWNABLE)
			continue;

		if (player->m_iTeam == TEAM_UNASSIGNED  || player->m_iTeam == TEAM_SPECTATOR)
			continue;

		if(gpGlobals->time < player->m_fDeadTime + 5.0f)
			continue;

		player->RoundRespawn();
	}
}

void CMod_TeamDeathMatch::PlayerKilled(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor)
{
	IBaseMod::PlayerKilled(pVictim, pKiller, pInflictor);

	CBasePlayer *peKiller = NULL;
	CBaseEntity *ktmp = CBaseEntity::Instance(pKiller);

	if (ktmp && ktmp->Classify() == CLASS_PLAYER)
	{
		peKiller = static_cast<CBasePlayer *>(ktmp);
	}
	else if (ktmp && ktmp->Classify() == CLASS_VEHICLE)
	{
		CBasePlayer *pDriver = static_cast<CBasePlayer *>(((CFuncVehicle *)ktmp)->m_pDriver);

		if (pDriver != NULL)
		{
			pKiller = pDriver->pev;
			peKiller = static_cast<CBasePlayer *>(pDriver);
		}
	}
	if (peKiller && peKiller->IsPlayer())
	{
		if (pVictim->m_iTeam != peKiller->m_iTeam)
		{
			switch (peKiller->m_iTeam)
			{
			case TEAM_CT:
				++m_iNumCTWins;
				break;
			case TEAM_TERRORIST:
				++m_iNumTerroristWins;
				break;
			default:
				// ?
				break;
			}
			UpdateTeamScores();
		}
	}

	// TODO: RespawnBar.
}

BOOL CMod_TeamDeathMatch::FPlayerCanTakeDamage(CBasePlayer *pPlayer, CBaseEntity *pAttacker)
{
	if (pAttacker && PlayerRelationship(pPlayer, pAttacker) == GR_TEAMMATE)
	{
		// my teammate hit me.
		if ((friendlyfire.value == 0) && (pAttacker != pPlayer))
		{
			// friendly fire is off, and this hit came from someone other than myself,  then don't get hurt
			return FALSE;
		}
	}

	return CHalfLifeMultiplay::FPlayerCanTakeDamage(pPlayer, pAttacker);
}

BOOL CMod_TeamDeathMatch::FPlayerCanRespawn(CBasePlayer *pPlayer)
{
	/*// Wait to Respawn...
	if (gpGlobals->time < pPlayer->m_fDeadTime + 3.0)
	{
		return FALSE;
	}*/

	// Player cannot respawn while in the Choose Appearance menu
	if (pPlayer->m_iMenu == Menu_ChooseAppearance)
	{
		return FALSE;
	}

	return TRUE;
}

void CMod_TeamDeathMatch::UpdateGameMode(CBasePlayer *pPlayer)
{
	MESSAGE_BEGIN(MSG_ONE, gmsgGameMode, NULL, pPlayer->edict());
	WRITE_BYTE(MOD_TDM);
	WRITE_BYTE(0); // Reserved. (weapon restriction? )
	WRITE_BYTE(maxkills.value); // MaxRound (mp_roundlimit)
	WRITE_BYTE(0); // Reserved. (MaxTime?)

	
	MESSAGE_END();
}

void CMod_TeamDeathMatch::PlayerSpawn(CBasePlayer *pPlayer)
{
	IBaseMod::PlayerSpawn(pPlayer);
	pPlayer->AddAccount(16000);

	// Give Armor
	pPlayer->m_iKevlar = ARMOR_TYPE_HELMET;
	pPlayer->pev->armorvalue = 100;

}