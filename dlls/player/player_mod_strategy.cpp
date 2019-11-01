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
#include "entity_state.h"
#include "player_mod_strategy.h"
#include "gamemode/mods.h"

namespace sv {

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

	if (gpGlobals->time - g_pGameRules->m_fRoundCount > buyTime * 1s)
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
		m_pPlayer->GiveNamedItem("knife_dragonsword");
		//m_pPlayer->GiveNamedItem("weapon_svdex");
		m_pPlayer->GiveNamedItem("weapon_usp");
		m_pPlayer->GiveAmmo(m_pPlayer->m_bIsVIP ? 12 : 24, "45acp", MAX_AMMO_45ACP);

		break;
	case TERRORIST:
		m_pPlayer->GiveNamedItem("knife_dragonsword");
		//m_pPlayer->GiveNamedItem("weapon_svdex");
		m_pPlayer->GiveNamedItem("weapon_glock18");
		m_pPlayer->GiveAmmo(40, "9mm", MAX_AMMO_9MM);

		break;
	default:
		break;
	}
}

void CPlayerModStrategy_Default::CmdStart(usercmd_s * cmd, unsigned int random_seed)
{
	if (m_pPlayer->pev->groupinfo)
	{
		UTIL_SetGroupTrace(m_pPlayer->pev->groupinfo, GROUP_OP_AND);
	}

	m_pPlayer->random_seed = random_seed;
}

void CPlayerModStrategy_Default::UpdateClientData(int sendweapons, clientdata_s * cd, entvars_t *pevOrg)
{
	CBasePlayer *pl = m_pPlayer;
	entvars_t *pev = pl->pev;
	edict_t *ent = ENT(pev);

	cd->flags = pev->flags;
	cd->health = pev->health;
	cd->viewmodel = MODEL_INDEX(STRING(pev->viewmodel));
	cd->waterlevel = pev->waterlevel;
	cd->watertype = pev->watertype;
	cd->weapons = pev->weapons;
	cd->origin = pev->origin;
	cd->velocity = pev->velocity;
	cd->view_ofs = pev->view_ofs;
	cd->punchangle = pev->punchangle;
	cd->bInDuck = pev->bInDuck;
	cd->flTimeStepSound = pev->flTimeStepSound;
	cd->flDuckTime = pev->flDuckTime;
	cd->flSwimTime = pev->flSwimTime;
	cd->waterjumptime = (int)pev->teleport_time;

	Q_strcpy(cd->physinfo, ENGINE_GETPHYSINFO(ent));

	cd->maxspeed = pev->maxspeed;
	cd->fov = pev->fov;
	cd->weaponanim = pev->weaponanim;
	cd->pushmsec = pev->pushmsec;

	if (pevOrg)
	{
		cd->iuser1 = pevOrg->iuser1;
		cd->iuser2 = pevOrg->iuser2;
		cd->iuser3 = pevOrg->iuser3;
	}
	else
	{
		cd->iuser1 = pev->iuser1;
		cd->iuser2 = pev->iuser2;
		cd->iuser3 = pev->iuser3;
	}

	cd->fuser1 = pev->fuser1;
	cd->fuser3 = pev->fuser3;
	cd->fuser2 = pev->fuser2;

	if (sendweapons && pl != NULL)
	{
		cd->ammo_shells = pl->ammo_buckshot;
		cd->ammo_nails = pl->ammo_9mm;
		cd->ammo_cells = pl->ammo_556nato;
		cd->ammo_rockets = pl->ammo_556natobox;
		cd->vuser2.x = pl->ammo_762nato;
		cd->vuser2.y = pl->ammo_45acp;
		cd->vuser2.z = pl->ammo_50ae;
		cd->vuser3.x = pl->ammo_338mag;
		cd->vuser3.y = pl->ammo_57mm;
		cd->vuser3.z = pl->ammo_357sig;
		cd->m_flNextAttack = pl->m_flNextAttack /1s;

		int iUser3 = 0;
		if (pl->m_bCanShoot && !pl->m_bIsDefusing)
			iUser3 |= DATA_IUSER3_CANSHOOT;

		if (g_pGameRules->IsFreezePeriod())
			iUser3 |= DATA_IUSER3_FREEZETIMEOVER;
		else
			iUser3 &= ~DATA_IUSER3_FREEZETIMEOVER;

		if (pl->m_signals.GetState() & SIGNAL_BOMB)
			iUser3 |= DATA_IUSER3_INBOMBZONE;
		else
			iUser3 &= ~DATA_IUSER3_INBOMBZONE;

		if (pl->HasShield())
			iUser3 |= DATA_IUSER3_HOLDINGSHIELD;
		else
			iUser3 &= ~DATA_IUSER3_HOLDINGSHIELD;

		if (!pl->pev->iuser1 && !pevOrg)
			cd->iuser3 = iUser3;

		if (pl->m_pActiveItem != NULL)
		{
			ItemInfo II;
			Q_memset(&II, 0, sizeof(II));

			CBasePlayerWeapon *gun = dynamic_cast<CBasePlayerWeapon *>(pl->m_pActiveItem->GetWeaponPtr());

			if (gun != NULL && gun->UseDecrement() && gun->GetItemInfo(&II))
			{
				cd->m_iId = II.iId;

				if ((unsigned int)gun->m_iPrimaryAmmoType < MAX_AMMO_TYPES)
				{
					cd->vuser4.x = gun->m_iPrimaryAmmoType;
					cd->vuser4.y = pl->m_rgAmmo[gun->m_iPrimaryAmmoType];
				}
				else
				{
					cd->vuser4.x = -1.0;
					cd->vuser4.y = 0;
				}
			}
		}
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

}
