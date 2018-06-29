#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "bte_mod.h"
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

	RefreshSkillData();
	m_flIntermissionEndTime = 0;
	m_flIntermissionStartTime = 0;
	SERVER_COMMAND("exec server.cfg\n");

	m_iMaxRounds = (int)CVAR_GET_FLOAT("mp_maxkills");
}

BOOL CMod_TeamDeathMatch::IsTeamplay()
{
	return TRUE;
}

void CMod_TeamDeathMatch::Think(void)
{
	if (gamemode.value != 2)
		SERVER_COMMAND("restart\n");

	m_VoiceGameMgr.Update(gpGlobals->frametime);\

	///// Check game rules /////

	if (CheckGameOver())   // someone else quit the game already
		return;
	
	if (CheckTimeLimit())
		return;

	m_iTotalRoundsPlayed = m_iNumCTWins + m_iNumTerroristWins; // hack
	if (CheckMaxRounds())
		return;

	
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

int CMod_TeamDeathMatch::PlayerRelationship(CBasePlayer *pPlayer, CBaseEntity *pTarget)
{
	return IBaseMod::PlayerRelationship(pPlayer, pTarget);
}

BOOL CMod_TeamDeathMatch::ClientCommand(CBasePlayer *pPlayer, const char *pcmd)
{
	if (m_VoiceGameMgr.ClientCommand(pPlayer, pcmd))
		return TRUE;

	return FALSE;
}

void CMod_TeamDeathMatch::InitHUD(CBasePlayer *pl)
{
	IBaseMod::InitHUD(pl); // CHalfLifeMultiplay

	if (g_fGameOver)
	{
		MESSAGE_BEGIN(MSG_ONE, SVC_INTERMISSION, NULL, pl->edict());
		MESSAGE_END();
	}

	//CLIENT_COMMAND(pl->edict(), "spectate\n");

}

void CMod_TeamDeathMatch::DeathNotice(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pevInflictor)
{
	CHalfLifeMultiplay::DeathNotice(pVictim, pKiller, pevInflictor);
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
	// Wait to Respawn...
	if (gpGlobals->time < pPlayer->m_fDeadTime + 3.0)
	{
		return FALSE;
	}

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
	WRITE_BYTE(maxrounds.value); // MaxRound (mp_roundlimit)
	WRITE_BYTE(0); // Reserved. (MaxTime?)

	
	MESSAGE_END();
}