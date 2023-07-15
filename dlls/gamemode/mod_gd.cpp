/*
mod_gd.cpp - CSMoE Gameplay server : GunDeath
Copyright (C) 2019 Moemod Hyakuya

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
#include "globals.h"

#include "mod_gd.h"
#include "player/player_mod_strategy.h"

#include <random>
#include <string>

namespace sv {

constexpr int MAX_LEVEL = 18;
constexpr int KILL_NUMS[MAX_LEVEL] = { 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 2, 3, 3, 3, 0 };
constexpr const char *DEFAULT_WEAPONS[MAX_LEVEL][2] = {
		{"weapon_m4a1","weapon_ak47"},
		{"weapon_sg550","weapon_g3sg1"},
		{"weapon_tar21","weapon_tar21"},
		{"weapon_aug","weapon_sg552"},
		{"weapon_an94","weapon_m16a4"},
		{"weapon_awp","weapon_svd"},
		{"weapon_scar","weapon_xm8"},
		{"weapon_famas","weapon_galil"},
		{"weapon_k1a","weapon_usas"},
		{"weapon_mp5","weapon_mp7a1"},
		{"weapon_m3","weapon_xm1014"},
		{"weapon_m249","weapon_qbb95"},
		{"weapon_scout","weapon_scout"},
		{"weapon_p90","weapon_p90"},
		{"weapon_deagle","weapon_elite"},
		{"weapon_tmp","weapon_mac10"},
		{"weapon_p228","weapon_fiveseven"},
		{"weapon_glock18","weapon_glock18"}
};
constexpr const int DEFAULT_WEAPONS_RATE[MAX_LEVEL][2] = {
		{45, 45},
		{45, 45},
		{50, 50},
		{45, 45},
		{50, 40},
		{65, 25},
		{50, 40},
		{50, 40},
		{60, 30},
		{60, 30},
		{30, 60},
		{30, 60},
		{50, 50},
		{50, 50},
		{50, 40},
		{40, 50},
		{40, 50},
		{50, 50}
};

constexpr const char* WEAPON_LIMIT_RARE[MAX_LEVEL] =
{
	"weapon_m14ebr",
	"weapon_mg36",
	"",
	"weapon_as50",
	"weapon_stg44",
	"weapon_trg42",
	"weapon_m24",
	"weapon_kriss",
	"weapon_ksg15",
	"weapon_tomason",
	"weapon_dbarrel",
	"weapon_m60",
	"",
	"",
	"weapon_infinity",
	"weapon_infinitya",
	"weapon_infinityb"
	""
};

class PlayerExtraHumanLevel_GD : public BasePlayerExtra
{
public:
	explicit PlayerExtraHumanLevel_GD(CBasePlayer *player) : BasePlayerExtra(player), m_iLevel(0), m_iKillsRemaining(0)
	{
		m_iKillsRemaining = KillRemainingForCurrentLevel();
	}

	void OnKillEnemy()
	{
		if (m_iLevel >= MAX_LEVEL - 1)
		{
			GiveGreande();
			return;
		}

		--m_iKillsRemaining;
		if (!m_iKillsRemaining)
		{
			++m_iLevel;
			m_iKillsRemaining = KillRemainingForCurrentLevel();
			GiveWeaponsToPlayer();
		}

		UpdateHUD();
	}

	void GiveWeaponsToPlayer() const
	{
		// remove current
		m_pPlayer->RemoveAllItems(FALSE);
		m_pPlayer->m_bHasPrimary = false;
		// TODO : give new weapons
		m_pPlayer->GiveNamedItem("weapon_knife");

		int iRd = RANDOM_LONG(0, 100);

		if (iRd < DEFAULT_WEAPONS_RATE[m_iLevel][0])
			m_pPlayer->GiveNamedItem(DEFAULT_WEAPONS[m_iLevel][0]);
		else if (iRd <= (DEFAULT_WEAPONS_RATE[m_iLevel][0] + DEFAULT_WEAPONS_RATE[m_iLevel][1]))
			m_pPlayer->GiveNamedItem(DEFAULT_WEAPONS[m_iLevel][1]);
		else
			m_pPlayer->GiveNamedItem(WEAPON_LIMIT_RARE[m_iLevel]);

		for (auto iSlot : { PRIMARY_WEAPON_SLOT, PISTOL_SLOT })
		{
			m_pPlayer->m_pModStrategy->GiveSlotAmmo(m_pPlayer, iSlot);
		}
	}

	void GiveGreande() const
	{
		m_pPlayer->GiveNamedItem("weapon_hegrenade");
	}

	void UpdateHUD() const
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgGunDeath, nullptr, m_pPlayer->pev);
		WRITE_BYTE(1); // type, reserved.
		WRITE_BYTE(m_iLevel); // Level
		MESSAGE_END();

		if (m_iKillsRemaining)
		{
			char buffer[64];
			sprintf(buffer, "hostage%d", m_iKillsRemaining);
			MESSAGE_BEGIN(MSG_ONE, gmsgScenarioIcon, nullptr, m_pPlayer->pev);
			WRITE_BYTE(1);
			WRITE_STRING(buffer);
			WRITE_BYTE(0);
			MESSAGE_END();
		}
		else
		{
			MESSAGE_BEGIN(MSG_ONE, gmsgScenarioIcon, nullptr, m_pPlayer->pev);
			WRITE_BYTE(0);
			MESSAGE_END();
		}
	}

protected:
	int KillRemainingForCurrentLevel() const { return KILL_NUMS[m_iLevel]; }

private:
	int m_iLevel;
	int m_iKillsRemaining;
};

CMod_GunDeath::CMod_GunDeath()
{
	m_iWinTeam = UNASSIGNED;
	m_bGrenadeKilled = false;
}

class CMod_GunDeath::PlayerModStrategy : public CPlayerModStrategy_Default
{
public:
	explicit PlayerModStrategy(CBasePlayer *player, CMod_GunDeath *pGD)
		: CPlayerModStrategy_Default(player),
		mp(pGD),
		lv(player),
		m_eventPlayerKilledListener(pGD->m_eventPlayerKilled.subscribe(&PlayerModStrategy::Event_OnPlayerKilled, this))
	{}
	bool CanPlayerBuy(bool display) override { return false; }
	bool CanDropWeapon(const char *pszItemName) override { return false; }
	void CheckBuyZone() override { /* */ }
	void OnSpawn() override
	{
		lv.UpdateHUD();
	}
	void Event_OnPlayerKilled(CBasePlayer *pVictim, CBasePlayer *pKiller, entvars_t *pInflictor)
	{
		if (pKiller != m_pPlayer)
			return;

		lv.OnKillEnemy();
	}
	void GiveDefaultItems() override
	{
		lv.GiveWeaponsToPlayer();
	}



private:
	CMod_GunDeath * const mp;
	PlayerExtraHumanLevel_GD lv;
	const EventListener m_eventPlayerKilledListener;
};

void SV_Start_VoteMaps();


void CMod_GunDeath::Think(void)
{
	if ((m_bGrenadeKilled))
	{
		m_bGrenadeKilled = false;

		if(m_iWinTeam == TERRORIST)
			Broadcast("terwin");
		else if(m_iWinTeam == CT)
			Broadcast("ctwin");
		else
			Broadcast("rounddraw");

		m_iWinTeam = UNASSIGNED;

		ALERT(at_console, "Changing maps...one team has won the specified number of rounds\n");
		if (CheckCanStartVoteMaps())
		{
			SV_Start_VoteMaps();
		}
		else
		{
			GoToIntermission();
		}
		return CMod_TeamDeathMatch::Think();
	}


	if (gpGlobals->time > m_tmNextPeriodicThink)
	{
		CheckRestartRound();
		m_tmNextPeriodicThink = gpGlobals->time + 1.0s;

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
			CVAR_SET_FLOAT("mp_maxkills", 500);
		}
	}

	CMod_TeamDeathMatch::Think();
}

void CMod_GunDeath::UpdateGameMode(CBasePlayer *pPlayer)
{
	MESSAGE_BEGIN(MSG_ONE, gmsgGameMode, nullptr, pPlayer->edict());
	WRITE_BYTE(MOD_GD);
	WRITE_BYTE(0); // Reserved. (weapon restriction? )
	WRITE_BYTE(static_cast<int>(maxkills.value)); // MaxRound (mp_roundlimit)
	WRITE_BYTE(0); // Reserved. (MaxTime?)
	MESSAGE_END();
}

void CMod_GunDeath::InstallPlayerModStrategy(CBasePlayer *player)
{
	std::unique_ptr<PlayerModStrategy> up(new PlayerModStrategy(player, this));
	player->m_pModStrategy = std::move(up);
}
void CMod_GunDeath::PlayerKilled(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor)
{
	CMod_TeamDeathMatch::PlayerKilled(pVictim, pKiller, pInflictor);
	CBasePlayer *pKillerPlayer = dynamic_ent_cast<CBasePlayer *>(pKiller);
	if (pKillerPlayer)
		m_eventPlayerKilled.dispatch(pVictim, pKillerPlayer, pInflictor);

	CBaseEntity* pEnt = CBaseEntity::Instance(pInflictor);
	CGrenade* pGrenade = dynamic_cast<CGrenade*>(pEnt);
	if (pGrenade && pVictim->pev != pKiller)
	{
		CBasePlayer* killer = GetClassPtr<CBasePlayer>(pKiller);
		if (killer->m_iTeam != pVictim->m_iTeam)
		{
			m_iWinTeam = killer->m_iTeam;
			m_bGrenadeKilled = true;
		}
	}
}

ModConfigData_t CMod_GunDeath::QueryModConfig(const char* key, ModConfigData_t defaultValue, CBaseEntity* invoker)
{
	if (!strcmp(key, "WeaponBoxLifeTime"))
		return -1.0s;

	if (!strcmp(key, "CanPickWeapon"))
		return false;

	return IBaseMod::QueryModConfig(key, defaultValue, invoker);
}


}
