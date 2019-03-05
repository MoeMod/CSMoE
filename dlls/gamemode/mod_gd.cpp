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

#include <random>
#include <string>

constexpr int MAX_LEVEL = 18;
constexpr int KILL_NUMS[MAX_LEVEL] = {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 2, 3, 3, 3, 3};
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

class PlayerExtraHumanLevel_GD : public BasePlayerExtra
{
public:
	explicit PlayerExtraHumanLevel_GD(CBasePlayer *player) : BasePlayerExtra(player), m_iLevel(0), m_iKillsRemaining(0)
	{
		m_iKillsRemaining = KillRemainingForCurrentLevel();
	}

	void OnKillEnemy()
	{
		if(m_iLevel >= MAX_LEVEL)
		{
			GiveGreande();
			return;
		}

		--m_iKillsRemaining;
		if(!m_iKillsRemaining)
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
		std::random_device rd;
		m_pPlayer->GiveNamedItem(DEFAULT_WEAPONS[m_iLevel][rd() & 1]);
		// TODO : give ammo

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
			MESSAGE_BEGIN(MSG_ONE, gmsgScenarioIcon, nullptr, m_pPlayer->pev);
			WRITE_BYTE(1);
			WRITE_STRING((std::string("hostage") + std::to_string(m_iKillsRemaining)).c_str());
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
	constexpr int KillRemainingForCurrentLevel() const { return KILL_NUMS[m_iLevel]; }

private:
	int m_iLevel;
	int m_iKillsRemaining;
};

class CMod_GunDeath::PlayerModStrategy : public CPlayerModStrategy_Default
{
public:
	explicit PlayerModStrategy(CBasePlayer *player, CMod_GunDeath *pGD)
		:   CPlayerModStrategy_Default(player),
			mp(pGD),
			lv(player),
			m_eventPlayerKilledListener(pGD->m_eventPlayerKilled.subscribe(&PlayerModStrategy::Event_OnPlayerKilled, this))
		{}
	bool CanPlayerBuy(bool display) override { return false; }
	bool CanDropWeapon(const char *pszItemName) override { return false; }
	void CheckBuyZone() override { /* */ }
	void Event_OnPlayerKilled(CBasePlayer *pVictim, CBasePlayer *pKiller, entvars_t *pInflictor)
	{
		if(pKiller != m_pPlayer)
			return;
		lv.OnKillEnemy();
	}

private:
	CMod_GunDeath * const mp;
	PlayerExtraHumanLevel_GD lv;
	const EventListener m_eventPlayerKilledListener;
};

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
	CBasePlayer *pKillerPlayer = dynamic_ent_cast<CBasePlayer *>(pKiller);
	if(pKillerPlayer)
		m_eventPlayerKilled.dispatch(pVictim, pKillerPlayer, pInflictor);
	CMod_TeamDeathMatch::PlayerKilled(pVictim, pKiller, pInflictor);
}
