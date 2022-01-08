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
	{"weapon_infinity",    "恒宇双星", 1500, PISTOL_SLOT,         UNASSIGNED},
	{"weapon_infinityex1", "星红双子",   1500, PISTOL_SLOT,         UNASSIGNED},
	{"weapon_infinityex2", "金红双蝎",     1500, PISTOL_SLOT,         UNASSIGNED},
	{"weapon_infinityss",  "恒宇银星",       600,  PISTOL_SLOT,         UNASSIGNED},
	{"weapon_infinitysr",  "恒宇红星",          600,  PISTOL_SLOT,         UNASSIGNED},
	{"weapon_infinitysb",  "恒宇黑星",        600,  PISTOL_SLOT,         UNASSIGNED},
	{"weapon_anaconda",	 "左轮军魂",			  650,  PISTOL_SLOT,         UNASSIGNED},

	{"knife_skullaxe",     "旋风",               0,    KNIFE_SLOT,          UNASSIGNED},
	{"knife_katana",     "狂花乱舞",               0,    KNIFE_SLOT,          UNASSIGNED},
	{"weapon_m1887",       "退魔圣焰",                 2800, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_kriss",       "致命蝎刺",                 1700, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_k1a",         "K1A",                   1850, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_thompson",    "汤姆逊",              2000, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	//{"weapon_mp7a1c",      "MP7A1",                 2150, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_m14ebr",      "咆哮怒焰",               3100, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_m2",      "守护者",               6200, PRIMARY_WEAPON_SLOT, UNASSIGNED },
	{"weapon_m134",      "终结者",               7000, PRIMARY_WEAPON_SLOT, UNASSIGNED },
	{"weapon_m134ex",      "终结者",               7000, PRIMARY_WEAPON_SLOT, UNASSIGNED },
	{"weapon_tbarrel",         "破碎炙焱",                  2600, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_m134xmas",         "圣诞终结者",                  7000, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_mg36",         "开拓者",                  5750, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	//{"weapon_xm8c",        "XM8",                   3250, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	//{"weapon_scarl",       "Scar",                  3250, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_tar21",       "塔沃尔",                3600, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_bendita",        "冰封圣骑",                  5500, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_skull5",        "死亡骑士",                  6000, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_mg3",         "毁灭者",                  5750, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_mg3xmas",         "圣诞毁灭者",                  5750, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_molotov",         "燃烧瓶",                  600, GRENADE_SLOT, UNASSIGNED},
	{"weapon_m95xmas",         "圣诞巴雷特",                  5000, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	
	//{"z4b_m37se",         "恶棍",                  2700, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	//{"csgo_zeus",         "电击枪",                  200, KNIFE_SLOT, UNASSIGNED},
	
	{"z4b_infinityx", "英菲尼迪双枪",     1500, PISTOL_SLOT,         UNASSIGNED},
	
	{"weapon_voidpistol",         "黑曜审判",                  1700, PISTOL_SLOT, UNASSIGNED},
	{"weapon_janus7xmas",         "圣诞雷狱",                  9000, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_sgdrill",         "怒海狂鲨",                 6250, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_bow",      "鹰眼",               6200, PRIMARY_WEAPON_SLOT, UNASSIGNED },
	{"weapon_huntbow",      "魂狩猎弓",               6200, PRIMARY_WEAPON_SLOT, UNASSIGNED },
	{"weapon_gatling",      "加特林",               4000, PRIMARY_WEAPON_SLOT, UNASSIGNED },
	{"weapon_chainsaw",         "生命收割者",                  5750, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_desperado",         "永恒",                  1500, PISTOL_SLOT, UNASSIGNED},
	{"weapon_cannon",      "黑龙炮",    3800, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_gungnir",     "断狱雷魂",               6000, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	
	{"knife_lance",     "天罚龙枪",               0,    KNIFE_SLOT,          UNASSIGNED },
	{"knife_dualsword",    "弑神双剑",0, KNIFE_SLOT,          UNASSIGNED},
	{"weapon_balrog7",      "炎魔",               9000, PRIMARY_WEAPON_SLOT, UNASSIGNED },
	{"weapon_starchaserar",      "灭却星光",               5000, PRIMARY_WEAPON_SLOT, UNASSIGNED },
	{"weapon_gunkata",	 "深渊",			  1500,  PISTOL_SLOT,         UNASSIGNED},
	{"weapon_gatlingex",        "日冕加特林",                  4600, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"knife_dgaxe",     "神鬼开天",               0,    KNIFE_SLOT,          UNASSIGNED },
	{"weapon_buffsg552ex",         "熔火岩狼",           5000, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_balrog11",        "龙炎",                  5750, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_balrog11b",        "凛风",                  5750, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_balrog11wc",        "黄金龙炎",                  5750, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_sfsmg",         "疾风之翼",           1900, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_sfgun",         "星际重炮",           3500, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_qbarrel",         "破碎炙燚",           5000, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_buffm249",      "朱雀",    9000, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_buffm4",      "邪皇",    5000, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_buffak",      "圣帝",    5000, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"knife_stormgiant",      "星陨巨锤",    0, KNIFE_SLOT, UNASSIGNED},
	
	{"knife_thanatos9",      "陨裂",    0, KNIFE_SLOT, UNASSIGNED},
	{"weapon_y21s1jetgunmd",  "冰雪魔咒",	2000, PISTOL_SLOT, UNASSIGNED},
	{"knife_skullt9",     "阵风T9",               0,    KNIFE_SLOT,          UNASSIGNED },
	{"weapon_sfsniper",         "雷神",           5000, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"knife_runeblade",      "【魔魂】重剑",    0, KNIFE_SLOT, UNASSIGNED},
	{"weapon_plasmagun",      "破晓黎明",    0, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"knife_janus9",     "灾厄Janus-9",               0,    KNIFE_SLOT,          UNASSIGNED },

	{"z4b_cannonplasma",      "黎明离子炮",    3800, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"z4b_cannontiger",      "裂地虎炮",    3800, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"z4b_Frequency9",      "迷迭F9",    0, KNIFE_SLOT, UNASSIGNED},
	{"z4b_frequency1",        "劫曦F1",                  1500, PISTOL_SLOT, UNASSIGNED},
	{"z4b_lc_sword",      "军团指挥官圣剑",    0, KNIFE_SLOT, UNASSIGNED},
	{"z4b_facelessvoid",      "虚空假面智杖",    0, KNIFE_SLOT, UNASSIGNED},
	{"z4b_strongknifex",      "新月刃",    0, KNIFE_SLOT, UNASSIGNED},
	{"z4b_nataknifedx",      "双持新月刃",    0, KNIFE_SLOT, UNASSIGNED},
	{"z4b_oppovivo",      "绝地光剑",    0, KNIFE_SLOT, UNASSIGNED},
	{"z4b_stormgiantx",      "骇影巨锤",    0, KNIFE_SLOT, UNASSIGNED},
	{"z4b_m134heroi",      "漩涡i7",    7700, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"z4b_deagleb",      "修罗",    700, PISTOL_SLOT, UNASSIGNED},
	{"z4b_xm2010pc",      "罪恶虚空",    7700, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"z4b_awpnvidia",      "矿难AWP",    4650, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"z4b_m4a1razer",      "雷蛇M4A1",    4250, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"z4b_dmp7a1x",      "双持审判效应N7",    2350, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"z4b_m4a1x",      "绝影M4A1",    4288, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"z4b_ak47x",      "黑武士AK47",    4288, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"z4b_m4a1mw",      "暗羽冥王M4A1",    3600, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"z4b_barrettd",        "毁灭巴菲特",                  5288, PRIMARY_WEAPON_SLOT, UNASSIGNED },
	{"z4b_k1ases",        "银蝎K1A",                  8000, PRIMARY_WEAPON_SLOT, UNASSIGNED },
	{"z4b_m1887se",        "降魔圣焰M1887",                  5500, PRIMARY_WEAPON_SLOT, UNASSIGNED },

	{"weapon_ak47l",      "AK47-旋风",    2500, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_mp7a1d",      "双持MP7",    1700, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_deagled",      "双持沙鹰",    700, PISTOL_SLOT, UNASSIGNED},
#endif
	{"weapon_m95",         "巴雷特",           5000, PRIMARY_WEAPON_SLOT, UNASSIGNED },
	{"weapon_wa2000",      "狂怒骑士",                4600, PRIMARY_WEAPON_SLOT, UNASSIGNED },
	{"weapon_as50",        "末日骑士",                  5000, PRIMARY_WEAPON_SLOT, UNASSIGNED },
	{"weapon_stg44",        "咆哮飓风",                  3100, PRIMARY_WEAPON_SLOT, UNASSIGNED },
	{"weapon_m249ex",        "碎魂者",                  9000, PRIMARY_WEAPON_SLOT, UNASSIGNED },
	{"weapon_sfmg",         "赤色彗星",           5750, PRIMARY_WEAPON_SLOT, UNASSIGNED },
	{"knife_dragonsword",     "青龙偃月刀",               0,    KNIFE_SLOT,          UNASSIGNED },
	{"weapon_bison",         "野牛PP-Bison",                  1400, PRIMARY_WEAPON_SLOT, UNASSIGNED },
	{"weapon_m4a1dragon",         "青龙M4A1",                  3100, PRIMARY_WEAPON_SLOT, UNASSIGNED },
	{"weapon_ak47dragon",         "赤龙AK47",                  2500, PRIMARY_WEAPON_SLOT, UNASSIGNED },
	{"knife_knifedragon",         "龙吻",                  0, KNIFE_SLOT, UNASSIGNED },
	{"knife_nataknife",      "背刺银刃",    0, KNIFE_SLOT, UNASSIGNED },
	{"knife_nataknifed",      "双持银刃",    0, KNIFE_SLOT, UNASSIGNED },
	{"csgo_negev",         "内格夫Negev",                  1700, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"csgo_mag7",         "警喷MAG-7",                  1800, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"csgo_cz75",         "CZ75",                  500, PISTOL_SLOT, UNASSIGNED},
	{"csgo_sawedoff",         "匪喷Sawed-Off",                  1100, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"csgo_tec9",         "Tec-9",                  500, PISTOL_SLOT, UNASSIGNED},
	{"csgo_bizon",         "野牛PP-Bizon",                  1400, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"csgo_r8",        "左轮R8",                  850, PISTOL_SLOT, UNASSIGNED},

	{"weapon_knife", "海豹短刀",            0,    KNIFE_SLOT,          UNASSIGNED},
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
