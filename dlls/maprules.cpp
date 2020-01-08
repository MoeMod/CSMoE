
#include "extdll.h"
#include "eiface.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "gamerules.h"
#include "maprules.h"

namespace sv {

/*
* Globals initialization
*/
TYPEDESCRIPTION CRuleEntity::m_SaveData[] =
{
	DEFINE_FIELD(CRuleEntity, m_iszMaster, FIELD_STRING),
};

// Save parms as a block.  Will break save/restore if the structure changes, but this entity didn't ship with Half-Life, so
// it can't impact saved Half-Life games.
TYPEDESCRIPTION CGameText::m_SaveData[] =
{
	DEFINE_ARRAY(CGameText, m_textParms, FIELD_CHARACTER, sizeof(hudtextparms_t)),
};

TYPEDESCRIPTION CGamePlayerZone::m_SaveData[] =
{
	DEFINE_FIELD(CGamePlayerZone, m_iszInTarget, FIELD_STRING),
	DEFINE_FIELD(CGamePlayerZone, m_iszOutTarget, FIELD_STRING),
	DEFINE_FIELD(CGamePlayerZone, m_iszInCount, FIELD_STRING),
	DEFINE_FIELD(CGamePlayerZone, m_iszOutCount, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CRuleEntity, CBaseEntity);

void CRuleEntity::Spawn()
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->effects = EF_NODRAW;
}

void CRuleEntity::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "master"))
	{
		SetMaster(ALLOC_STRING(pkvd->szValue));
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

BOOL CRuleEntity::CanFireForActivator(CBaseEntity *pActivator)
{
	if (!FStringNull(m_iszMaster))
	{
		if (UTIL_IsMasterTriggered(m_iszMaster, pActivator))
			return TRUE;
		else
			return FALSE;
	}

	return TRUE;
}

void CRulePointEntity::Spawn()
{
	CRuleEntity::Spawn();
	pev->frame = 0;
	pev->model = 0;
}

void CRuleBrushEntity::Spawn()
{
	SET_MODEL(edict(), STRING(pev->model));
	CRuleEntity::Spawn();
}

LINK_ENTITY_TO_CLASS(game_score, CGameScore);

void CGameScore::Spawn()
{
	CRulePointEntity::Spawn();
}

void CGameScore::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "points"))
	{
		SetPoints(Q_atoi(pkvd->szValue));
		pkvd->fHandled = TRUE;
	}
	else
		CRulePointEntity::KeyValue(pkvd);
}

void CGameScore::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (!CanFireForActivator(pActivator))
		return;

	// Only players can use this
	if (pActivator->IsPlayer())
	{
		if (AwardToTeam())
		{
			pActivator->AddPointsToTeam(Points(), AllowNegativeScore());
		}
		else
		{
			pActivator->AddPoints(Points(), AllowNegativeScore());
		}
	}
}

LINK_ENTITY_TO_CLASS(game_end, CGameEnd);

void CGameEnd::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (!CanFireForActivator(pActivator))
		return;

	g_pGameRules->EndMultiplayerGame();
}

LINK_ENTITY_TO_CLASS(game_text, CGameText);

IMPLEMENT_SAVERESTORE(CGameText, CRulePointEntity);

void CGameText::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "channel"))
	{
		m_textParms.channel = Q_atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "x"))
	{
		m_textParms.x = Q_atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "y"))
	{
		m_textParms.y = Q_atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "effect"))
	{
		m_textParms.effect = Q_atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "color"))
	{
		int color[4];
		UTIL_StringToIntArray(color, ARRAYSIZE(color), pkvd->szValue);

		m_textParms.r1 = color[0];
		m_textParms.g1 = color[1];
		m_textParms.b1 = color[2];
		m_textParms.a1 = color[3];

		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "color2"))
	{
		int color[4];
		UTIL_StringToIntArray(color, ARRAYSIZE(color), pkvd->szValue);

		m_textParms.r2 = color[0];
		m_textParms.g2 = color[1];
		m_textParms.b2 = color[2];
		m_textParms.a2 = color[3];

		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "fadein"))
	{
		m_textParms.fadeinTime = Q_atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "fadeout"))
	{
		m_textParms.fadeoutTime = Q_atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "holdtime"))
	{
		m_textParms.holdTime = Q_atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "fxtime"))
	{
		m_textParms.fxTime = Q_atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CRulePointEntity::KeyValue(pkvd);
}

void CGameText::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (!CanFireForActivator(pActivator))
		return;

	if (MessageToAll())
	{
		UTIL_HudMessageAll(m_textParms, MessageGet());
	}
	else
	{
		if (pActivator->IsNetClient())
		{
			UTIL_HudMessage(pActivator, m_textParms, MessageGet());
		}
	}
}

LINK_ENTITY_TO_CLASS(game_team_master, CGameTeamMaster);

void CGameTeamMaster::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "teamindex"))
	{
		m_teamIndex = Q_atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "triggerstate"))
	{
		int type = Q_atoi(pkvd->szValue);

		switch (type)
		{
		case 0:
			triggerType = USE_OFF;
			break;
		case 2:
			triggerType = USE_TOGGLE;
			break;
		default:
			triggerType = USE_ON;
			break;
		}

		pkvd->fHandled = TRUE;
	}
	else
		CRulePointEntity::KeyValue(pkvd);
}

void CGameTeamMaster::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (!CanFireForActivator(pActivator))
		return;

	if (useType == USE_SET)
	{
		if (value < 0)
		{
			m_teamIndex = -1;
		}
		else
		{
			m_teamIndex = g_pGameRules->GetTeamIndex(pActivator->TeamID());
		}

		return;
	}

	if (TeamMatch(pActivator))
	{
		SUB_UseTargets(pActivator, triggerType, value);

		if (RemoveOnFire())
		{
			UTIL_Remove(this);
		}
	}
}

BOOL CGameTeamMaster::IsTriggered(CBaseEntity *pActivator)
{
	return TeamMatch(pActivator);
}

const char *CGameTeamMaster::TeamID()
{
	// Currently set to "no team"
	if (m_teamIndex < 0)
	{
		return "";
	}

	// UNDONE: Fill this in with the team from the "teamlist"
	return g_pGameRules->GetIndexedTeamName(m_teamIndex);
}

BOOL CGameTeamMaster::TeamMatch(CBaseEntity *pActivator)
{
	if (m_teamIndex < 0 && AnyTeam())
		return TRUE;

	if (!pActivator)
		return FALSE;

	return UTIL_TeamsMatch(pActivator->TeamID(), TeamID());
}

LINK_ENTITY_TO_CLASS(game_team_set, CGameTeamSet);

void CGameTeamSet::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (!CanFireForActivator(pActivator))
		return;

	if (ShouldClearTeam())
	{
		SUB_UseTargets(pActivator, USE_SET, -1);
	}
	else
	{
		SUB_UseTargets(pActivator, USE_SET, 0);
	}

	if (RemoveOnFire())
	{
		UTIL_Remove(this);
	}
}

LINK_ENTITY_TO_CLASS(game_zone_player, CGamePlayerZone);

IMPLEMENT_SAVERESTORE(CGamePlayerZone, CRuleBrushEntity);

void CGamePlayerZone::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "intarget"))
	{
		m_iszInTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "outtarget"))
	{
		m_iszOutTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "incount"))
	{
		m_iszInCount = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "outcount"))
	{
		m_iszOutCount = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CRuleBrushEntity::KeyValue(pkvd);
}

void CGamePlayerZone::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	int playersInCount = 0;
	int playersOutCount = 0;

	if (!CanFireForActivator(pActivator))
		return;

	CBaseEntity *pPlayer = NULL;

	for (int i = 1; i <= gpGlobals->maxClients; ++i)
	{
		pPlayer = static_cast<CBasePlayer *>(UTIL_PlayerByIndex(i));

		if (pPlayer)
		{
			TraceResult trace;
			int hullNumber;

			hullNumber = human_hull;
			if (pPlayer->pev->flags & FL_DUCKING)
			{
				hullNumber = head_hull;
			}

			UTIL_TraceModel(pPlayer->pev->origin, pPlayer->pev->origin, hullNumber, edict(), &trace);

			if (trace.fStartSolid)
			{
				++playersInCount;
				if (!FStringNull(m_iszInTarget))
				{
					FireTargets(STRING(m_iszInTarget), pPlayer, pActivator, useType, value);
				}
			}
			else
			{
				++playersOutCount;
				if (!FStringNull(m_iszOutTarget))
				{
					FireTargets(STRING(m_iszOutTarget), pPlayer, pActivator, useType, value);
				}
			}
		}
	}

	if (!FStringNull(m_iszInCount))
	{
		FireTargets(STRING(m_iszInCount), pActivator, this, USE_SET, playersInCount);
	}

	if (!FStringNull(m_iszOutCount))
	{
		FireTargets(STRING(m_iszOutCount), pActivator, this, USE_SET, playersOutCount);
	}

}

LINK_ENTITY_TO_CLASS(game_player_hurt, CGamePlayerHurt);

void CGamePlayerHurt::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (!CanFireForActivator(pActivator))
		return;

	if (pActivator->IsPlayer())
	{
		if (pev->dmg < 0)
			pActivator->TakeHealth(-pev->dmg, DMG_GENERIC);
		else
			pActivator->TakeDamage(pev, pev, pev->dmg, DMG_GENERIC);
	}

	SUB_UseTargets(pActivator, useType, value);

	if (RemoveOnFire())
	{
		UTIL_Remove(this);
	}
}

LINK_ENTITY_TO_CLASS(game_counter, CGameCounter);

void CGameCounter::Spawn()
{
	// Save off the initial count
	SetInitialValue(CountValue());
	CRulePointEntity::Spawn();
}

void CGameCounter::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (!CanFireForActivator(pActivator))
		return;

	switch (useType)
	{
	case USE_ON:
	case USE_TOGGLE:
		CountUp();
		break;

	case USE_OFF:
		CountDown();
		break;

	case USE_SET:
		SetCountValue((int)value);
		break;
	}

	if (HitLimit())
	{
		SUB_UseTargets(pActivator, USE_TOGGLE, 0);

		if (RemoveOnFire())
		{
			UTIL_Remove(this);
		}

		if (ResetOnFire())
		{
			ResetCount();
		}
	}
}

LINK_ENTITY_TO_CLASS(game_counter_set, CGameCounterSet);

void CGameCounterSet::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (!CanFireForActivator(pActivator))
		return;

	SUB_UseTargets(pActivator, USE_SET, pev->frags);

	if (RemoveOnFire())
	{
		UTIL_Remove(this);
	}
}

LINK_ENTITY_TO_CLASS(game_player_equip, CGamePlayerEquip);

void CGamePlayerEquip::KeyValue(KeyValueData *pkvd)
{
	CRulePointEntity::KeyValue(pkvd);

	if (!pkvd->fHandled)
	{
		for (std::size_t i = 0; i < MAX_EQUIP; ++i)
		{
			if (FStringNull(m_weaponNames[i]))
			{
				char tmp[128];
				UTIL_StripToken(pkvd->szKeyName, tmp);

				m_weaponNames[i] = ALLOC_STRING(tmp);
				m_weaponCount[i] = Q_atoi(pkvd->szValue);
				m_weaponCount[i] = Q_max(1, m_weaponCount[i]);

				pkvd->fHandled = TRUE;
				break;
			}
		}
	}
}

void CGamePlayerEquip::Touch(CBaseEntity *pOther)
{
	if (!CanFireForActivator(pOther))
		return;

	if (UseOnly())
		return;

	EquipPlayer(pOther);
}

void CGamePlayerEquip::EquipPlayer(CBaseEntity *pEntity)
{
	CBasePlayer *pPlayer = NULL;

	if (pEntity->IsPlayer())
	{
		pPlayer = static_cast<CBasePlayer *>(pEntity);
	}

	if (!pPlayer)
		return;

	for (std::size_t i = 0; i < MAX_EQUIP; ++i)
	{
		if (FStringNull(m_weaponNames[i]))
			break;

		for (int j = 0; j < m_weaponCount[i]; ++j)
		{
 			pPlayer->GiveNamedItem(STRING(m_weaponNames[i]));
		}
	}
}

void CGamePlayerEquip::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	EquipPlayer(pActivator);
}

LINK_ENTITY_TO_CLASS(game_player_team, CGamePlayerTeam);

const char *CGamePlayerTeam::TargetTeamName(const char *pszTargetName)
{
	CBaseEntity *pTeamEntity = NULL;

	while ((pTeamEntity = UTIL_FindEntityByTargetname(pTeamEntity, pszTargetName)) != NULL)
	{
		if (FClassnameIs(pTeamEntity->pev, "game_team_master"))
			return pTeamEntity->TeamID();
	}

	return NULL;
}

void CGamePlayerTeam::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (!CanFireForActivator(pActivator))
		return;

	if (pActivator->IsPlayer())
	{
		const char *pszTargetTeam = TargetTeamName(STRING(pev->target));

		if (pszTargetTeam != NULL)
		{
			CBasePlayer *pPlayer = static_cast<CBasePlayer *>(pActivator);
			g_pGameRules->ChangePlayerTeam(pPlayer, pszTargetTeam, ShouldKillPlayer(), ShouldGibPlayer());
		}
	}

	if (RemoveOnFire())
	{
		UTIL_Remove(this);
	}
}

}
