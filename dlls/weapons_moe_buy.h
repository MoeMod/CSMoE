/*
weapons_moe_buy.h - CSMoE Gameplay server : Weapon buy command handler
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

#ifndef WEAPONS_MOE_BUY_H
#define WEAPONS_MOE_BUY_H

#include "player/player_const.h"

#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

enum MoEWeaponBuyType
{
    BUY_NONE = -1,
    BUY_PISTOL = 0,
    BUY_SHOTGUN,
    BUY_SMG,
    BUY_RIFLE,
    BUY_MG,
    BUY_EQUIP,
    BUY_KNIFE
};

struct MoEWeaponBuyInfo_s
{
	int iID;
	const char* pszClassName;
	const char* pszDisplayName;
	int iCost;
	InventorySlotType iSlot;
	TeamName iTeam;
    MoEWeaponBuyType iMenu;
};

inline std::vector<MoEWeaponBuyInfo_s> g_MoEWeaponBuyInfo = {
    { WEAPON_USP, "weapon_usp",      "USP45",    500, PISTOL_SLOT, UNASSIGNED, BUY_PISTOL },
    { WEAPON_GLOCK18, "weapon_glock18",      "Glock-18",    400, PISTOL_SLOT, UNASSIGNED, BUY_PISTOL },
    { WEAPON_P228, "weapon_p228",      "P228",    600, PISTOL_SLOT, UNASSIGNED, BUY_PISTOL },
    { WEAPON_DEAGLE, "weapon_deagle",      "Deagle",    650, PISTOL_SLOT, UNASSIGNED, BUY_PISTOL },
    { WEAPON_ELITE, "weapon_elite",      "Elite",    800, PISTOL_SLOT, UNASSIGNED, BUY_PISTOL },
    { WEAPON_FIVESEVEN, "weapon_fiveseven",      "FiveSeven",    750, PISTOL_SLOT, UNASSIGNED, BUY_PISTOL },
    { WEAPON_M3, "weapon_m3",      "M3",    1700, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN },
    { WEAPON_XM1014, "weapon_xm1014",      "XM1014",    3000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN },
    { WEAPON_MAC10, "weapon_mac10",      "MAC10",    1400, PRIMARY_WEAPON_SLOT, UNASSIGNED,  BUY_SMG },
    { WEAPON_TMP, "weapon_tmp",      "TMP",    1250, PRIMARY_WEAPON_SLOT, UNASSIGNED,  BUY_SMG },
    { WEAPON_MP5N, "weapon_mp5navy",      "MP5",    1500, PRIMARY_WEAPON_SLOT, UNASSIGNED,  BUY_SMG },
    { WEAPON_UMP45, "weapon_ump45",      "UMP-45",    1700, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SMG },
    { WEAPON_P90, "weapon_p90",      "P90",    2350, PRIMARY_WEAPON_SLOT, UNASSIGNED,  BUY_SMG },
    { WEAPON_GALIL, "weapon_galil",      "Galil",    2000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },
    { WEAPON_FAMAS, "weapon_famas",      "Famas",    2250, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },
    { WEAPON_AK47, "weapon_ak47",      "CV-47",    2500, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },
    { WEAPON_SCOUT, "weapon_scout",      "Scout",    2750, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },
    { WEAPON_M4A1, "weapon_m4a1",      "M4A1",    3100, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },
    { WEAPON_AUG, "weapon_aug",      "AUG",    3500, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },
    { WEAPON_SG552, "weapon_sg552",      "SG552",    3500, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },
    { WEAPON_SG550, "weapon_sg550",      "SG550",    4200, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },
    { WEAPON_AWP, "weapon_awp",      "AWP",    4750, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },
    { WEAPON_G3SG1, "weapon_g3sg1",      "G3SG1",    5000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },
    { WEAPON_M249, "weapon_m249",      "M249",    5750, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG },

    { WEAPON_KNIFE, "weapon_knife", "海豹短刀",            0,    KNIFE_SLOT,          UNASSIGNED, BUY_KNIFE },
};

}

#endif
