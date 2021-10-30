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
#ifdef _WIN32
#pragma once
#endif

struct MoEWeaponBuyInfo_s
{
	const char* pszClassName;
	const char* pszDisplayName;
	int iCost;
	InventorySlotType iSlot;
	TeamName team;
};

constexpr MoEWeaponBuyInfo_s g_MoEWeaponBuyInfo[] = {
#ifdef XASH_DEDICATED
	{"weapon_infinity",    "Infinity Black/Sliver", 1500, PISTOL_SLOT,         UNASSIGNED},
	{"weapon_infinityex1", "Infinity Red/Sliver",   1500, PISTOL_SLOT,         UNASSIGNED},
	{"weapon_infinityex2", "Infinity Red/Gold",     1500, PISTOL_SLOT,         UNASSIGNED},
	{"weapon_infinityss",  "Infinity Sliver",       600,  PISTOL_SLOT,         UNASSIGNED},
	{"weapon_infinitysr",  "Infinity Red",          600,  PISTOL_SLOT,         UNASSIGNED},
	{"weapon_infinitysb",  "Infinity Black",        600,  PISTOL_SLOT,         UNASSIGNED},
	{"weapon_anaconda",	 "Anaconda",			  650,  PISTOL_SLOT,         UNASSIGNED},

	{"knife_skullaxe",     "Skull-9",               0,    KNIFE_SLOT,          UNASSIGNED},
	{"knife_katana",     "Katana",               0,    KNIFE_SLOT,          UNASSIGNED},
	{"weapon_m1887",       "M1887",                 2800, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_kriss",       "Kriss",                 1700, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_k1a",         "K1A",                   1850, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_thompson",    "Thompson",              2000, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	//{"weapon_mp7a1c",      "MP7A1",                 2150, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_m14ebr",      "M14 EBR",               3100, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_m2",      "M2",               6200, PRIMARY_WEAPON_SLOT, UNASSIGNED },
	{"weapon_m134",      "M134 Minigun",               7000, PRIMARY_WEAPON_SLOT, UNASSIGNED },
	{"weapon_m134ex",      "M134 Predator",               7000, PRIMARY_WEAPON_SLOT, UNASSIGNED },
	{"weapon_tbarrel",         "Triple Barrel Shotgun",                  2600, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_m134xmas",         "M134 Xmas Edition",                  7000, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_mg36",         "MG-36",                  5750, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	//{"weapon_xm8c",        "XM8",                   3250, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	//{"weapon_scarl",       "Scar",                  3250, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_tar21",       "TAR-21",                3600, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_m95",         "Barrett M95",           5000, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_wa2000",      "WA2000",                4600, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_as50",        "AS50",                  5000, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_stg44",        "STG-44",                  3100, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_bendita",        "Bendita",                  5500, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"z4b_k1ases",        "K1ase NST Edition",                  8000, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"z4b_m1887se",        "M1887 Sniper Edition",                  5500, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_skull5",        "Skull-5",                  6000, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_mg3",         "MG-3",                  5750, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_mg3xmas",         "MG3 Xmas Edition",                  5750, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_molotov",         "Molotov",                  600, GRENADE_SLOT, UNASSIGNED},
	{"weapon_m95xmas",         "M95 Xmas Edition",                  5000, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_m4a1dragon",         "M4A1 Dragon",                  3100, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	//{"z4b_m37se",         "M37SE",                  2700, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	//{"csgo_zeus",         "Zeus x27",                  200, KNIFE_SLOT, UNASSIGNED},
	{"weapon_ak47dragon",         "AK47 Dragon",                  2500, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"knife_knifedragon",         "Dragon Knife",                  0, KNIFE_SLOT, UNASSIGNED},
	{"z4b_infinityx", "Yet Another Infinity",     1500, PISTOL_SLOT,         UNASSIGNED},
	
	{"weapon_voidpistol",         "Voidpistol",                  1700, PISTOL_SLOT, UNASSIGNED},
	{"weapon_janus7xmas",         "Janus-7 Xmas Edition",                  9000, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_sgdrill",         "Magnum Drill",                 6250, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_bow",      "BOW",               6200, PRIMARY_WEAPON_SLOT, UNASSIGNED },
	{"weapon_huntbow",      "Hunt Bow",               6200, PRIMARY_WEAPON_SLOT, UNASSIGNED },
	{"weapon_gatling",      "Volcano",               4000, PRIMARY_WEAPON_SLOT, UNASSIGNED },
	{"weapon_chainsaw",         "Ripper",                  5750, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_desperado",         "Python Desperado",                  1500, PISTOL_SLOT, UNASSIGNED},
	{"weapon_cannon",      "Black Dragon Cannon",    3800, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_gungnir",     "Gungnir",               6000, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"knife_dragonsword",     "Green Dragon Crescent Blade",               0,    KNIFE_SLOT,          UNASSIGNED },
	{"knife_lance",     "Demonic Scarlet Rose",               0,    KNIFE_SLOT,          UNASSIGNED },
	{"knife_dualsword",    "Dual Sword Phantom Slayer",0, KNIFE_SLOT,          UNASSIGNED},
	{"weapon_balrog7",      "BALROG-VII",               9000, PRIMARY_WEAPON_SLOT, UNASSIGNED },
	{"weapon_starchaserar",      "Star Chaser AR",               5000, PRIMARY_WEAPON_SLOT, UNASSIGNED },
	{"weapon_gunkata",	 "Dual Beretta Gunslinger",			  1500,  PISTOL_SLOT,         UNASSIGNED},
	{"weapon_gatlingex",        "Infernal Cannon",                  4600, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"knife_dgaxe",     "Twin Shadow Axes",               0,    KNIFE_SLOT,          UNASSIGNED },
	{"weapon_buffsg552ex",         "Buff SG552EX",           5000, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_balrog11",        "Balrog-XI",                  5750, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_balrog11b",        "Balrog-XI Blue",                  5750, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_balrog11wc",        "Balrog-XI WC",                  5750, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_m249ex",        "Skull-7",                  9000, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_sfmg",         "Avalanche",           5750, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_sfsmg",         "Tempest",           1900, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_sfgun",         "Blaster",           3500, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"z4b_cannonplasma",      "Plasma Cannon",    3800, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"z4b_cannontiger",      "Tiger Cannon",    3800, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"z4b_Frequency9",      "Frequency 9",    0, KNIFE_SLOT, UNASSIGNED},
	{"z4b_frequency1",        "Frequency 1",                  1500, PISTOL_SLOT, UNASSIGNED},
	{"z4b_lc_sword",      "Legion Commander's Sword",    0, KNIFE_SLOT, UNASSIGNED},
	{"z4b_facelessvoid",      "Faceless Void's Stick",    0, KNIFE_SLOT, UNASSIGNED},
	{"z4b_strongknifex",      "Lunula Knife",    0, KNIFE_SLOT, UNASSIGNED},
	{"z4b_nataknifedx",      "Dual Lunula Knife",    0, KNIFE_SLOT, UNASSIGNED},
	{"z4b_oppovivo",      "Dual Light Stick",    0, KNIFE_SLOT, UNASSIGNED},
	{"z4b_stormgiantx",      "Storm Giant's Hammer",    0, KNIFE_SLOT, UNASSIGNED},
	{"z4b_m134heroi",      "M134 CORE-i7",    7700, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"z4b_deagleb",      "D.Eagle Born Beast",    700, PISTOL_SLOT, UNASSIGNED},
	{"z4b_xm2010pc",      "XM2010 Crystal-P",    7700, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"z4b_awpnvidia",      "AWP-NVIDIA",    4650, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"z4b_m4a1razer",      "M4A1-Razer",    4250, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"z4b_dmp7a1x",      "Dual SMG-N7",    2350, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"z4b_m4a1x",      "M4A1 Shadow Runner",    4288, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"z4b_ak47x",      "AK47 KNIFE-BORN BEAST",    4288, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"z4b_m4a1mw",      "M4A1 S-Transformer",    3600, PRIMARY_WEAPON_SLOT, UNASSIGNED},

	{"weapon_ak47l",      "AK-47 60R",    2500, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_mp7a1d",      "Dual MP7A1",    1700, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_deagled",      "Dual Desert Eagle",    700, PISTOL_SLOT, UNASSIGNED},
#endif
	{"csgo_negev",         "Negev",                  1700, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"csgo_mag7",         "MAG-7",                  1800, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"csgo_cz75",         "CZ75",                  500, PISTOL_SLOT, UNASSIGNED},
	{"csgo_sawedoff",         "Sawed-Off",                  1100, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"csgo_tec9",         "Tec-9",                  500, PISTOL_SLOT, UNASSIGNED},
	{"csgo_bizon",         "PP-Bizon",                  1400, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	//{"csgo_r8",        "R8",                  850, PISTOL_SLOT, UNASSIGNED},

	{"weapon_knife", "Seal Knife",            0,    KNIFE_SLOT,          UNASSIGNED},
	{"weapon_p228",      "P228",    600, PISTOL_SLOT, UNASSIGNED},
	{"weapon_glock18",      "Glock-18",    400, PISTOL_SLOT, UNASSIGNED},
	{"weapon_scout",      "Scout",    2750, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_xm1014",      "XM1014",    3000, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_mac10",      "MAC10",    1400, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_aug",      "AUG",    3500, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_elite",      "Elite",    800, PISTOL_SLOT, UNASSIGNED},
	{"weapon_fiveseven",      "FiveSeven",    750, PISTOL_SLOT, UNASSIGNED},
	{"weapon_ump45",      "UMP-45",    1700, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_sg550",      "SG550",    4200, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_galil",      "Galil",    2000, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_famas",      "Famas",    2250, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_usp",      "USP45",    500, PISTOL_SLOT, UNASSIGNED},
	{"weapon_awp",      "AWP",    4750, PRIMARY_WEAPON_SLOT, UNASSIGNED },
	{"weapon_mp5navy",      "MP5",    1500, PRIMARY_WEAPON_SLOT, UNASSIGNED },
	{"weapon_m249",      "M249",    5750, PRIMARY_WEAPON_SLOT, UNASSIGNED },
	{"weapon_m3",      "M3",    1700, PRIMARY_WEAPON_SLOT, UNASSIGNED },
	{"weapon_m4a1",      "M4A1",    3100, PRIMARY_WEAPON_SLOT, UNASSIGNED },
	{"weapon_tmp",      "TMP",    1250, PRIMARY_WEAPON_SLOT, UNASSIGNED },
	{"weapon_g3sg1",      "G3SG1",    5000, PRIMARY_WEAPON_SLOT, UNASSIGNED },
	{"weapon_deagle",      "Deagle",    650, PISTOL_SLOT, UNASSIGNED },
	{"weapon_sg552",      "SG552",    3500, PRIMARY_WEAPON_SLOT, UNASSIGNED },
	{"weapon_ak47",      "CV-47",    2500, PRIMARY_WEAPON_SLOT, UNASSIGNED },
	{"weapon_p90",      "P90",    2350, PRIMARY_WEAPON_SLOT, UNASSIGNED },

};

#endif
