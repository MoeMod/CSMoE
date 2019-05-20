/*
weapons_moe_buy.cpp - CSMoE Gameplay server : Weapon buy command handler
Copyright (C) 2019 Moemod Hymei

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
#include "weapons.h"
#include "gamerules.h"
#include "client.h"
#include "weapons_moe_buy.h"

#include <algorithm>

namespace sv {

struct MoEWeaponBuyInfo_s
{
	const char *pszClassName;
	const char *pszDisplayName;
	int iCost;
	InventorySlotType iSlot;
	TeamName team;
};

static MoEWeaponBuyInfo_s g_MoEWeaponBuyInfo[] = {{"weapon_infinity",    "Infinity Black/Sliver", 1500, PISTOL_SLOT,         UNASSIGNED},
                                                  {"weapon_infinityex1", "Infinity Red/Sliver",   1500, PISTOL_SLOT,         UNASSIGNED},
                                                  {"weapon_infinityex2", "Infinity Red/Gold",     1500, PISTOL_SLOT,         UNASSIGNED},
                                                  {"weapon_infinityss",  "Infinity Sliver",       600,  PISTOL_SLOT,         UNASSIGNED},
                                                  {"weapon_infinitysr",  "Infinity Red",          600,  PISTOL_SLOT,         UNASSIGNED},
                                                  {"weapon_infinitysb",  "Infinity Black",        600,  PISTOL_SLOT,         UNASSIGNED},

                                                  {"weapon_m1887",       "M1887",                 2800, PRIMARY_WEAPON_SLOT, UNASSIGNED},

                                                  {"weapon_kriss",       "Kriss",                 1700, PRIMARY_WEAPON_SLOT, UNASSIGNED},
                                                  {"weapon_k1a",         "K1A",                   1850, PRIMARY_WEAPON_SLOT, UNASSIGNED},
                                                  {"weapon_thompson",    "Thompson",              2000, PRIMARY_WEAPON_SLOT, UNASSIGNED},
                                                  {"weapon_mp7a1c",      "MP7A1",                 2150, PRIMARY_WEAPON_SLOT, UNASSIGNED},

                                                  {"weapon_m14ebr",      "M14 EBR",               3100, PRIMARY_WEAPON_SLOT, UNASSIGNED},
                                                  {"weapon_xm8c",        "XM8",                   3250, PRIMARY_WEAPON_SLOT, UNASSIGNED},
                                                  {"weapon_scarl",       "Scar",                  3250, PRIMARY_WEAPON_SLOT, UNASSIGNED},
                                                  {"weapon_tar21",       "TAR-21",                3600, PRIMARY_WEAPON_SLOT, UNASSIGNED},

                                                  {"weapon_m95",         "Barrett M95",           5000, PRIMARY_WEAPON_SLOT, UNASSIGNED},

                                                  {"weapon_wa2000",      "WA2000",                4600, PRIMARY_WEAPON_SLOT, UNASSIGNED},
                                                  {"weapon_as50",        "AS50",                  5000, PRIMARY_WEAPON_SLOT, UNASSIGNED},

                                                  {"weapon_mg3",         "MG-3",                  5750, PRIMARY_WEAPON_SLOT, UNASSIGNED},

                                                  {"weapon_cannon",      "Blackdragon Cannon",    3800, PRIMARY_WEAPON_SLOT, UNASSIGNED},
                                                  {"weapon_gungnir",     "Gungnir",               6000, PRIMARY_WEAPON_SLOT, UNASSIGNED},

                                                  {"weapon_knife",       "Seal Knife",            0,    KNIFE_SLOT,          UNASSIGNED},
                                                  {"knife_skullaxe",     "Skull-9",               0,    KNIFE_SLOT,          UNASSIGNED}

};

bool HasPlayerItem(CBasePlayer *pPlayer, const char *pszCheckItem)
{
	for (CBasePlayerItem *pItem : pPlayer->m_rgpPlayerItems) {
		while (pItem != nullptr) {
			if (FClassnameIs(pItem->pev, pszCheckItem))
				return true;

			pItem = pItem->m_pNext;
		}
	}
	return false;
}

bool MoE_HandleBuyCommands(CBasePlayer *pPlayer, const char *pszCommand)
{
	if (!pPlayer->CanPlayerBuy(true))
		return false;

	if (HasPlayerItem(pPlayer, pszCommand)) {
		ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "#Cstrike_Already_Own_Weapon");
		return false;
	}

	auto iter = std::find_if(std::begin(g_MoEWeaponBuyInfo), std::end(g_MoEWeaponBuyInfo),
	                         [pszCommand](const MoEWeaponBuyInfo_s &info) {
		                         return !strcmp(info.pszClassName, pszCommand);
	                         });

	if (iter != std::end(g_MoEWeaponBuyInfo)) {
		if (pPlayer->m_iAccount < iter->iCost) {
			ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "#Not_Enough_Money");
			BlinkAccount(pPlayer, 2);

			return false;
		}

		switch (iter->iSlot) {
			case PRIMARY_WEAPON_SLOT:
				DropPrimary(pPlayer);
				break;
			case PISTOL_SLOT :
				DropSecondary(pPlayer);
				break;
			case KNIFE_SLOT:
				if (pPlayer->m_rgpPlayerItems[KNIFE_SLOT]) {
					pPlayer->RemovePlayerItem(pPlayer->m_rgpPlayerItems[KNIFE_SLOT]);
				}
				break;
			default:
				break;
		}

		pPlayer->GiveNamedItem(iter->pszClassName);
		pPlayer->AddAccount(-iter->iCost);

		return true;
	}

	return false;
}

}
