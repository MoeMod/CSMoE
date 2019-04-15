/*
player_mod_strategy.cpp - CSMoE Gameplay server : player strategy for gamemodes
Copyright (C) 2018 Moemod Hyakuya

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
#include "client.h"
#include "monsters.h"
#include "player_mod_strategy.h"
#include "gamemode/mods.h"

void CPlayerModStrategy_Default::CheckBuyZone()
{
	if (g_pGameRules->m_bMapHasBuyZone)
		return;
	CBasePlayer *player = m_pPlayer;

	const char *pszSpawnClass = nullptr;

	if (player->m_iTeam == TERRORIST)
		pszSpawnClass = "info_player_deathmatch";

	else if (player->m_iTeam == CT)
		pszSpawnClass = "info_player_start";

	if (pszSpawnClass != nullptr)
	{
		CBaseEntity *pSpot = nullptr;
		while ((pSpot = UTIL_FindEntityByClassname(pSpot, pszSpawnClass)) != nullptr)
		{
			if ((pSpot->pev->origin - player->pev->origin).Length() < 200.0f)
				player->m_signals.Signal(SIGNAL_BUY);
		}
	}
}

bool CPlayerModStrategy_Default::CanPlayerBuy(bool display)
{
	CBasePlayer *player = m_pPlayer;

	// is the player alive?
	if (player->pev->deadflag != DEAD_NO)
	{
		return false;
	}

	// is the player in a buy zone?
	if (!(player->m_signals.GetState() & SIGNAL_BUY))
	{
		return false;
	}

	int buyTime = (int)(CVAR_GET_FLOAT("mp_buytime") * 60.0f);

	if (buyTime < MIN_BUY_TIME)
	{
		buyTime = MIN_BUY_TIME;
		CVAR_SET_FLOAT("mp_buytime", (MIN_BUY_TIME / 60.0f));
	}

	if (gpGlobals->time - g_pGameRules->m_fRoundCount > buyTime)
	{
		if (display)
		{
			ClientPrint(player->pev, HUD_PRINTCENTER, "#Cant_buy", UTIL_dtos1(buyTime));
		}

		return false;
	}

	if (player->m_bIsVIP)
	{
		if (display)
		{
			ClientPrint(player->pev, HUD_PRINTCENTER, "#VIP_cant_buy");
		}

		return false;
	}

	if (g_pGameRules->m_bCTCantBuy && player->m_iTeam == CT)
	{
		if (display)
		{
			ClientPrint(player->pev, HUD_PRINTCENTER, "#CT_cant_buy");
		}

		return false;
	}

	if (g_pGameRules->m_bTCantBuy && player->m_iTeam == TERRORIST)
	{
		if (display)
		{
			ClientPrint(player->pev, HUD_PRINTCENTER, "#Terrorist_cant_buy");
		}

		return false;
	}

	return true;
}

void CPlayerModStrategy_Default::Pain(int m_LastHitGroup, bool HasArmour)
{
	auto temp = RANDOM_LONG(0, 2);

	if (m_LastHitGroup == HITGROUP_HEAD)
	{
		if (m_pPlayer->m_iKevlar == ARMOR_TYPE_HELMET)
		{
			EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "player/bhit_helmet-1.wav", VOL_NORM, ATTN_NORM);
			return;
		}

		switch (temp)
		{
		case 0: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "player/headshot1.wav", VOL_NORM, ATTN_NORM); break;
		case 1: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "player/headshot2.wav", VOL_NORM, ATTN_NORM); break;
		default: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "player/headshot3.wav", VOL_NORM, ATTN_NORM); break;
		}
	}
	else
	{
		if (m_LastHitGroup != HITGROUP_LEFTLEG && m_LastHitGroup != HITGROUP_RIGHTLEG)
		{
			if (HasArmour)
			{
				EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "player/bhit_kevlar-1.wav", VOL_NORM, ATTN_NORM);
				return;
			}
		}

		switch (temp)
		{
		case 0: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "player/bhit_flesh-1.wav", VOL_NORM, ATTN_NORM); break;
		case 1: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "player/bhit_flesh-2.wav", VOL_NORM, ATTN_NORM); break;
		default: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "player/bhit_flesh-3.wav", VOL_NORM, ATTN_NORM); break;
		}
	}
}

void CPlayerModStrategy_Default::DeathSound()
{
	// temporarily using pain sounds for death sounds
	switch (RANDOM_LONG(1, 4))
	{
	case 1: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "player/die1.wav", VOL_NORM, ATTN_NORM); break;
	case 2: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "player/die2.wav", VOL_NORM, ATTN_NORM); break;
	case 3: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "player/die3.wav", VOL_NORM, ATTN_NORM); break;
	case 4: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "player/death6.wav", VOL_NORM, ATTN_NORM); break;
	default:break;
	}
}

void CPlayerModStrategy_Default::GiveDefaultItems()
{
	m_pPlayer->RemoveAllItems(FALSE);
	m_pPlayer->m_bHasPrimary = false;

	switch (m_pPlayer->m_iTeam)
	{
	case CT:
		//m_pPlayer->GiveNamedItem("weapon_knife");
		m_pPlayer->GiveNamedItem("knife_skullaxe");
		m_pPlayer->GiveNamedItem("weapon_usp");
		m_pPlayer->GiveAmmo(m_pPlayer->m_bIsVIP ? 12 : 24, "45acp", MAX_AMMO_45ACP);

		break;
	case TERRORIST:
		//m_pPlayer->GiveNamedItem("weapon_knife");
		m_pPlayer->GiveNamedItem("knife_skullaxe");
		m_pPlayer->GiveNamedItem("weapon_glock18");
		m_pPlayer->GiveAmmo(40, "9mm", MAX_AMMO_9MM);

		break;
	default:
		break;
	}
}

void CPlayerModStrategy_Zombie::Pain(int m_LastHitGroup, bool HasArmour)
{
	CPlayerModStrategy_Default::Pain(m_LastHitGroup, HasArmour);
}

void CPlayerModStrategy_Zombie::DeathSound()
{
	CPlayerModStrategy_Default::DeathSound();
}

float CPlayerModStrategy_Zombie::AdjustDamageTaken(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType)
{
	// grenade damage 5x in zb mode
	if (g_pModRunning->DamageTrack() == DT_ZB && !Q_strcmp(STRING(pevInflictor->classname), "grenade"))
	{
		if (bitsDamageType & DMG_EXPLOSION)
		{
			if (m_pPlayer->m_bIsZombie)
				flDamage *= 5.0f;
			else
				flDamage *= 2.5f;
		}
	}
	return CPlayerModStrategy_Default::AdjustDamageTaken(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

void CPlayerModStrategy_Zombie::GiveDefaultItems()
{
	if (!m_pPlayer->m_bIsZombie)
		return CPlayerModStrategy_Default::GiveDefaultItems();
	
	m_pPlayer->RemoveAllItems(FALSE);
	m_pPlayer->m_bHasPrimary = false;

	m_pPlayer->GiveNamedItem("knife_zombi");

	if (!(m_pPlayer->m_flDisplayHistory & DHF_NIGHTVISION))
	{
		m_pPlayer->HintMessage("#Hint_use_nightvision");
		m_pPlayer->m_flDisplayHistory |= DHF_NIGHTVISION;
	}
	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "items/equip_nvg.wav", VOL_NORM, ATTN_NORM);
	m_pPlayer->m_bHasNightVision = true;
	SendItemStatus(m_pPlayer);
	
}
