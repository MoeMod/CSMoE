#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "game.h"
#include "client.h"
#include "bmodels.h"

#include "mod_zb2.h"
#include "zb2/supplybox.h"
#include "player/csdm_randomspawn.h"

#include <algorithm>
#include <vector>


CMod_ZombieMod2::CMod_ZombieMod2() // precache
{
	UTIL_PrecacheOther("supplybox");

	PRECACHE_SOUND("zombi/zombi_box.wav");
}

void CMod_ZombieMod2::UpdateGameMode(CBasePlayer *pPlayer)
{
	MESSAGE_BEGIN(MSG_ONE, gmsgGameMode, NULL, pPlayer->edict());
	WRITE_BYTE(MOD_ZB2);
	WRITE_BYTE(0); // Reserved. (weapon restriction? )
	WRITE_BYTE(maxrounds.value); // MaxRound (mp_roundlimit)
	WRITE_BYTE(0); // Reserved. (MaxTime?)

	MESSAGE_END();
}

void CMod_ZombieMod2::Think()
{
	MakeSupplyboxThink();

	return CMod_Zombi::Think();
}

void CMod_ZombieMod2::PlayerSpawn(CBasePlayer *pPlayer)
{
	CMod_Zombi::PlayerSpawn(pPlayer);
	UpdatePlayerEvolutionHUD(pPlayer);
}

void CMod_ZombieMod2::PlayerThink(CBasePlayer *pPlayer)
{
	pPlayer->Zombie_HealthRecoveryThink();
	pPlayer->ZombieSkill_Check();
	return CMod_Zombi::PlayerThink(pPlayer);
}

void CMod_ZombieMod2::RestartRound()
{
	RemoveAllSupplybox();
	m_flTimeNextMakeSupplybox = gpGlobals->time + RANDOM_FLOAT(30.0f, 60.0f);
	return CMod_Zombi::RestartRound();
}

BOOL CMod_ZombieMod2::ClientCommand(CBasePlayer *pPlayer, const char *pcmd)
{
	if (!Q_stricmp(pcmd, "BTE_ZombieSkill1") && CanUseZombieSkill(pPlayer))
	{
		pPlayer->ZombieSkill_Start();
		return TRUE;
	}

	return CMod_Zombi::ClientCommand(pPlayer, pcmd);
}

void CMod_ZombieMod2::MakeSupplyboxThink()
{
	if (!FInfectionStarted())
		return;
	if (gpGlobals->time < m_flTimeNextMakeSupplybox)
		return;
	m_flTimeNextMakeSupplybox = gpGlobals->time + RANDOM_FLOAT(20.0f, 30.0f);

	RemoveAllSupplybox();

	int iSupplyboxCount = SupplyboxCount();
	for (int i = 0; i < iSupplyboxCount; ++i)
	{
		CSupplyBox *sb = CreateSupplybox();
		if (!sb) 
			continue;
		sb->m_iSupplyboxIndex = i + 1;

		for (int iIndex = 1; iIndex <= gpGlobals->maxClients; ++iIndex)
		{
			CBaseEntity *entity = UTIL_PlayerByIndex(iIndex);
			if (!entity)
				continue;
			if(static_cast<CBasePlayer *>(entity)->m_bIsZombie)
				continue;

			MESSAGE_BEGIN(MSG_ALL, gmsgHostagePos, NULL, entity->pev);
			WRITE_BYTE(1);
			WRITE_BYTE(sb->m_iSupplyboxIndex);
			WRITE_COORD(sb->pev->origin.x);
			WRITE_COORD(sb->pev->origin.y);
			WRITE_COORD(sb->pev->origin.z);
			MESSAGE_END();
		}
	}

	UTIL_ClientPrintAll(HUD_PRINTCENTER, "A Supply Box has arrived!"); // #CSO_SupportItemSpawned

	for (int iIndex = 1; iIndex <= gpGlobals->maxClients; ++iIndex)
	{
		CBaseEntity *entity = UTIL_PlayerByIndex(iIndex);
		if (!entity)
			continue;
		CLIENT_COMMAND(entity->edict(), "spk zombi/zombi_box.wav\n");

	}

}

int CMod_ZombieMod2::SupplyboxCount()
{
	int NumDeadCT, NumDeadTerrorist, NumAliveTerrorist, NumAliveCT;
	InitializePlayerCounts(NumAliveTerrorist, NumAliveCT, NumDeadTerrorist, NumDeadCT);
	int iSupplyboxCount = (NumAliveTerrorist + NumAliveCT + NumDeadTerrorist) / 10 + 1;
	return iSupplyboxCount;
}

void CMod_ZombieMod2::RemoveAllSupplybox()
{
	CBaseEntity *ent = nullptr;
	while ((ent = UTIL_FindEntityByClassname(ent, "supplybox")) != nullptr)
	{
		CSupplyBox *sb = static_cast<CSupplyBox *>(ent);
		sb->pev->effects |= EF_NODRAW;
		sb->pev->flags |= FL_KILLME;
		sb->SendPositionMsg();
		sb->SetThink(&CBaseEntity::SUB_Remove);
	}
}

CSupplyBox *CMod_ZombieMod2::CreateSupplybox()
{
	edict_t *pent = CREATE_NAMED_ENTITY(MAKE_STRING("supplybox"));

	if (FNullEnt(pent))
	{
		ALERT(at_console, "NULL Ent in CreateSupplybox()!\n");
		return nullptr;
	}

	CBaseEntity *monster = CBaseEntity::Instance(pent);

	Vector backup_v_angle = monster->pev->v_angle;
	CSDM_DoRandomSpawn(monster);
	monster->pev->v_angle = backup_v_angle;

	pent->v.spawnflags |= SF_NORESPAWN;

	DispatchSpawn(pent);
	return static_cast<CSupplyBox *>(monster);
}

void CMod_ZombieMod2::HumanInfectionByZombie(CBasePlayer *player, CBasePlayer *attacker)
{
	CMod_Zombi::HumanInfectionByZombie(player, attacker);
	attacker->m_iZombieInfections++;

	if (attacker->m_iZombieLevel == ZOMBIE_LEVEL_HOST && attacker->m_iZombieInfections >= 3)
	{
		MakeZombie(attacker, ZOMBIE_LEVEL_ORIGIN);

		attacker->pev->health = attacker->pev->max_health = 7000.0f;
		attacker->pev->armorvalue = 500.0f;
	}

	if (attacker->m_iZombieLevel == ZOMBIE_LEVEL_ORIGIN && attacker->m_iZombieInfections >= 5)
	{
		MakeZombie(attacker, ZOMBIE_LEVEL_ORIGIN_LV2);

		attacker->pev->health = attacker->pev->max_health = 14000.0f;
		attacker->pev->armorvalue = 1000.0f;
	}

	UpdatePlayerEvolutionHUD(attacker);
}

void CMod_ZombieMod2::UpdatePlayerEvolutionHUD(CBasePlayer *player)
{
	int iCount = 0;
	if (player->m_bIsZombie && player->m_iZombieLevel != ZOMBIE_LEVEL_ORIGIN_LV2)
	{
		int iInfectionRequired = player->m_iZombieLevel == ZOMBIE_LEVEL_HOST ? 3 : 5;
		iCount = iInfectionRequired - player->m_iZombieInfections;
	}

	char buf[16];
	Q_snprintf(buf, ARRAYSIZE(buf), "hostage%d", iCount);

	if (iCount)
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgScenarioIcon, NULL, player->pev);
		WRITE_BYTE(1);
		WRITE_STRING(buf);
		WRITE_BYTE(0);
		MESSAGE_END();
	}
	else
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgScenarioIcon, NULL, player->pev);
		WRITE_BYTE(0);
		MESSAGE_END();
	}
}

bool CMod_ZombieMod2::CanUseZombieSkill(CBasePlayer *player)
{
	return player->m_bIsZombie && player->m_iZombieLevel != ZOMBIE_LEVEL_HOST;
}

void CMod_ZombieMod2::MakeZombie(CBasePlayer *player, ZombieLevel iEvolutionLevel)
{
	CMod_Zombi::MakeZombie(player, iEvolutionLevel);
	UpdatePlayerEvolutionHUD(player);
	player->m_iZombieInfections = 0;
	player->ZombieSkill_Init();
}