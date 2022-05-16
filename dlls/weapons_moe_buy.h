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
	const char* pszClassName;
	const char* pszDisplayName;
	int iCost;
	InventorySlotType iSlot;
	TeamName iTeam;
    MoEWeaponBuyType iMenu;
	int iScore;
};

constexpr MoEWeaponBuyInfo_s g_MoEWeaponBuyInfo[] = {
    {"weapon_usp",      "USP45",    500, PISTOL_SLOT, UNASSIGNED, BUY_PISTOL,1},
    {"weapon_glock18",      "Glock-18",    400, PISTOL_SLOT, UNASSIGNED, BUY_PISTOL,1},
    {"weapon_p228",      "P228",    600, PISTOL_SLOT, UNASSIGNED, BUY_PISTOL,1},
    {"weapon_deagle",      "Deagle",    650, PISTOL_SLOT, UNASSIGNED, BUY_PISTOL,1 },
    {"weapon_elite",      "Elite",    800, PISTOL_SLOT, UNASSIGNED, BUY_PISTOL,1},
    {"weapon_fiveseven",      "FiveSeven",    750, PISTOL_SLOT, UNASSIGNED, BUY_PISTOL,1},
    {"weapon_m3",      "M3",    1700, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN, 1 },
    {"weapon_xm1014",      "XM1014",    3000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN, 1},
    {"weapon_mac10",      "MAC10",    1400, PRIMARY_WEAPON_SLOT, UNASSIGNED,  BUY_SMG,1},
    {"weapon_tmp",      "TMP",    1250, PRIMARY_WEAPON_SLOT, UNASSIGNED,  BUY_SMG,1 },
    {"weapon_mp5navy",      "MP5",    1500, PRIMARY_WEAPON_SLOT, UNASSIGNED,  BUY_SMG,1 },
    {"weapon_ump45",      "UMP-45",    1700, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SMG, 1},
    {"weapon_p90",      "P90",    2350, PRIMARY_WEAPON_SLOT, UNASSIGNED,  BUY_SMG,1 },
    {"weapon_galil",      "Galil",    2000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1},
    {"weapon_famas",      "Famas",    2250, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1},
    {"weapon_ak47",      "CV-47",    2500, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1 },
    {"weapon_scout",      "Scout",    2750, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1},
    {"weapon_m4a1",      "M4A1",    3100, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1 },
    {"weapon_aug",      "AUG",    3500, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1},
    {"weapon_sg552",      "SG552",    3500, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1 },
    {"weapon_sg550",      "SG550",    4200, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1},
    {"weapon_awp",      "AWP",    4750, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1 },
    {"weapon_g3sg1",      "G3SG1",    5000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1 },
    {"weapon_m249",      "M249",    5750, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG, 1 },

    {"weapon_knife", "海豹短刀",            0,    KNIFE_SLOT,          UNASSIGNED, BUY_KNIFE, 1},

#ifdef XASH_DEDICATED
	{"z4b_infinityx", "英菲尼迪双枪",     1500, PISTOL_SLOT,         UNASSIGNED, BUY_PISTOL, 1},
	{"weapon_voidpistol",         "黑曜审判",                  1700, PISTOL_SLOT, UNASSIGNED, BUY_PISTOL, 50},
	{"weapon_gunkata",	 "深渊",			  1500,  PISTOL_SLOT,         UNASSIGNED, BUY_PISTOL, 50},
	{"weapon_desperado",         "永恒",                  1500, PISTOL_SLOT, UNASSIGNED, BUY_PISTOL, 50},
	{"weapon_y21s1jetgunmd",  "冰雪魔咒",	2000, PISTOL_SLOT, UNASSIGNED, BUY_PISTOL, 50},
	{"z4b_frequency1",        "劫曦F1",                  1500, PISTOL_SLOT, UNASSIGNED, BUY_PISTOL, 1},
	{"z4b_deagleb",      "修罗",    700, PISTOL_SLOT, UNASSIGNED, BUY_PISTOL, 1},
	{"weapon_deagled",      "双持沙鹰",    700, PISTOL_SLOT, UNASSIGNED, BUY_PISTOL, 1},
	//{"z4b_m37se",         "恶棍",                  2700, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN, 1},
	{"weapon_huntbow",      "魂狩猎弓",               6200, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP, 100 },
	{"weapon_gungnir",     "断狱雷魂",               6000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP, 150},
    {"weapon_sgdrill",         "怒海狂鲨",                 6250, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN, 1},
	{"weapon_gatlingex",        "日冕加特林",                  4600, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN, 1},
	{"weapon_buffsg552ex",         "熔火岩狼",           5000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1},
	{"weapon_qbarrel",         "破碎炙燚",           5000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN, 1},
	{"weapon_buffm249",      "朱雀",    9000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG, 1},
	{"weapon_buffm4",      "邪皇",    5000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1},
	{"weapon_buffak",      "圣帝",    5000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1},
	{"weapon_revivegun",	 "逆界星轮",			  1500,  PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP,  },
	{"z4b_cannonplasma",      "黎明离子炮",    3800, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP, 1},
	{"z4b_cannontiger",      "裂地虎炮",    3800, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP, 1},
	{"z4b_m134heroi",      "漩涡i7",    7700, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG, 1},
	{"z4b_xm2010pc",      "罪恶虚空",    7700, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1},
	{"z4b_awpnvidia",      "矿难AWP",    4650, PRIMARY_WEAPON_SLOT, UNASSIGNED,BUY_RIFLE,  1},
	{"z4b_m4a1razer",      "雷蛇M4A1",    4250, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1},
	{"z4b_dmp7a1x",      "双持审判效应N7",    2350, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SMG, 1},
	{"z4b_m4a1x",      "绝影M4A1",    4288, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1},
	{"z4b_ak47x",      "黑武士AK47",    4288, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1},
	{"z4b_m4a1mw",      "暗羽冥王M4A1",    3600, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1},
	
	{"z4b_barrettd",        "毁灭巴菲特",                  5288, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1 },
	{"weapon_cheytaclrrs",         "CheytacLRRS-M200",           5000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1 },
	{"z4b_k1ases",        "银蝎K1A",                  8000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SMG, 1 },
	{"z4b_m1887se",        "降魔圣焰M1887",                  5500, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN, 1 },
	{"weapon_ak47l",      "AK47-旋风",    2500, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1},
	{"weapon_mp7a1d",      "双持MP7",    1700, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SMG, 1},
	{ "weapon_m32venom", "腐化重炮", 6500, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP, 1 },
	{ "weapon_m95tiger",         "【虎王】M95",           5000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1 },
	{ "weapon_y20s2scard",         "绿野仙踪SCARD",           1500, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1 },
    {"weapon_janus7xmas",         "圣诞雷狱",                  9000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG, 1},

	{"knife_lance",     "天罚龙枪",               0,    KNIFE_SLOT,          UNASSIGNED, BUY_KNIFE, 150 },
	{"knife_dualsword",    "弑神双剑",0, KNIFE_SLOT,          UNASSIGNED, BUY_KNIFE, 150},
	{"knife_dgaxe",     "神鬼开天",               0,    KNIFE_SLOT,          UNASSIGNED, BUY_KNIFE, 120 },
	{"knife_stormgiant",      "星陨巨锤",    0, KNIFE_SLOT, UNASSIGNED, BUY_KNIFE, 100},
	{"knife_skullt9",     "阵风T9",               0,    KNIFE_SLOT,          UNASSIGNED, BUY_KNIFE, 100 },
	{"knife_runeblade",      "【魔魂】重剑",    0, KNIFE_SLOT, UNASSIGNED, BUY_KNIFE, 120},
	{"z4b_Frequency9",      "迷迭F9",    0, KNIFE_SLOT, UNASSIGNED, BUY_KNIFE, 1},
	{"z4b_lc_sword",      "军团指挥官圣剑",    0, KNIFE_SLOT, UNASSIGNED, BUY_KNIFE, 1},
	{"z4b_facelessvoid",      "虚空假面智杖",    0, KNIFE_SLOT, UNASSIGNED, BUY_KNIFE, 1},
	{"z4b_strongknifex",      "新月刃",    0, KNIFE_SLOT, UNASSIGNED, BUY_KNIFE, 1},
	{"z4b_nataknifedx",      "双持新月刃",    0, KNIFE_SLOT, UNASSIGNED, BUY_KNIFE, 1},
	{"z4b_oppovivo",      "绝地光剑",    0, KNIFE_SLOT, UNASSIGNED, BUY_KNIFE, 1},
	{"z4b_stormgiantx",      "骇影巨锤",    0, KNIFE_SLOT, UNASSIGNED, BUY_KNIFE, 80},
	{"knife_summonknife",     "星际仲裁",               0,    KNIFE_SLOT,          UNASSIGNED, BUY_KNIFE, 150 },
	{"knife_holysword",     "【辉煌】圣剑",               0,    KNIFE_SLOT,          UNASSIGNED, BUY_KNIFE, 150 },
	{"knife_magicknife",     "幻镜！光棱剑",               0,    KNIFE_SLOT,          UNASSIGNED, BUY_KNIFE, 150 },
	{"weapon_molotov",         "燃烧瓶",                  600, GRENADE_SLOT, UNASSIGNED, BUY_EQUIP, 1},
	{ "knife_nataknifed",      "双持银刃",    0, KNIFE_SLOT, UNASSIGNED, BUY_KNIFE, 1 },
	{ "knife_y22s1holyswordmb",         "【奇幻乐园】双手剑",           0, KNIFE_SLOT, UNASSIGNED, BUY_KNIFE, 150 },
	{"knife_whipsword",     "【极影】蛇心剑",               0,    KNIFE_SLOT,          UNASSIGNED, BUY_KNIFE, 150 },
	{ "weapon_cannonex",         "赤焰魔龙",           6500, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP, 1 },
	{ "weapon_speargun",         "海皇之怒",           5800, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP, 1 },
	{"weapon_sgmissile",        "撼宇碎星",                  6400, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP, 1},
	{"weapon_sgmissilem",        "爆烈神机弩",                  6250, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP, 1},
	//{"weapon_sgmissileex",        "撼宇碎星Zero",                  6250, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP, 1},
	//{"csgo_zeus",         "电击枪",                  200, KNIFE_SLOT, UNASSIGNED, BUY_KNIFE, 1},
	{ "csgo_cz75",         "CZ75",                  500, PISTOL_SLOT, UNASSIGNED, BUY_PISTOL, 1 },
	{ "csgo_tec9",         "Tec-9",                  500, PISTOL_SLOT, UNASSIGNED, BUY_PISTOL, 1 },
	{ "csgo_r8",        "左轮R8",                  850, PISTOL_SLOT, UNASSIGNED, BUY_PISTOL, 1 },
	{ "csgo_negev",         "内格夫Negev",                  1700, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG, 1 },
	{ "csgo_mag7",         "警喷MAG-7",                  1800, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN, 1 },
	{ "csgo_sawedoff",         "匪喷Sawed-Off",                  1100, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN, 1 },
	{ "csgo_bizon",         "野牛PP-Bizon",                  1400, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SMG, 1 },
#endif
	{ "weapon_sprifle",         "狂暴蒸汽",           5500, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1 },
	{"weapon_infinity",    "恒宇双星", 1500, PISTOL_SLOT,         UNASSIGNED, BUY_PISTOL, 1},
    {"weapon_infinityex1", "星红双子",   1500, PISTOL_SLOT,         UNASSIGNED, BUY_PISTOL, 1},
    {"weapon_infinityex2", "金红双蝎",     1500, PISTOL_SLOT,         UNASSIGNED, BUY_PISTOL, 1},
    {"weapon_infinityss",  "恒宇银星",       600,  PISTOL_SLOT,         UNASSIGNED, BUY_PISTOL, 1},
    {"weapon_infinitysr",  "恒宇红星",          600,  PISTOL_SLOT,         UNASSIGNED, BUY_PISTOL, 1},
    {"weapon_infinitysb",  "恒宇黑星",        600,  PISTOL_SLOT,         UNASSIGNED, BUY_PISTOL, 1},
    {"weapon_anaconda",	 "左轮军魂",			  650,  PISTOL_SLOT,         UNASSIGNED, BUY_PISTOL, 1},
	{"weapon_m950",	 "卡利柯M950",			  1500,  PISTOL_SLOT,         UNASSIGNED, BUY_PISTOL, 1 },
    {"weapon_mp7a1c",      "MP7A1",                 2150, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SMG, 1},
    {"weapon_xm8c",        "XM8",                   3250, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1},
    {"weapon_scarl",       "Scar",                  3250, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1},
    {"weapon_m1887",       "退魔圣焰",                 2800, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN, 1},
    {"weapon_kriss",       "致命蝎刺",                 1700, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SMG, 1},
    {"weapon_k1a",         "K1A",                   1850, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SMG, 1},
    {"weapon_thompson",    "汤姆逊",              2000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SMG, 1},
    {"weapon_m14ebr",      "咆哮怒焰",               3100, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1},
	{"weapon_m2",      "守护者",               6200, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG, 1 },
    {"weapon_m134",      "终结者",               7000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG, 1 },
    {"weapon_m134ex",      "终结者EX",               7000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG, 1 },
    {"weapon_tbarrel",         "破碎炙焱",                  2600, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN, 1},
    {"weapon_m134xmas",         "圣诞终结者",                  7000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG, 1},
    {"weapon_mg36",         "开拓者",                  5750, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG, 1},
    {"weapon_tar21",       "塔沃尔",                3600, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1},
    {"weapon_skull5",        "死亡骑士",                  6000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1},
    {"weapon_mg3",         "毁灭者",                  5750, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG, 1},
    {"weapon_mg3xmas",         "圣诞毁灭者",                  5750, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG, 1},
    {"weapon_bow",      "鹰眼",               6200, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP, 1 },
    {"weapon_balrog7",      "炎魔",               9000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG, 1 },
    {"weapon_starchaserar",      "灭却星光",               5000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 90 },
    {"weapon_balrog11",        "龙炎",                  5750, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN, 1},
    {"weapon_balrog11b",        "凛风",                  5750, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN, 1},
    {"weapon_balrog11wc",        "黄金龙炎",                  5750, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN, 1},
    {"weapon_sfsmg",         "疾风之翼",           1900, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SMG, 1},
	{ "weapon_tmpdragon",	 "Tmp金龙",			  1250,  PRIMARY_WEAPON_SLOT,         UNASSIGNED, BUY_SMG, 1 },
	{"weapon_sfgun",         "星际重炮",           3500, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1},
    {"weapon_sfsniper",         "雷神",           5000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1},
	{"weapon_m400",         "死神M200",           5000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1 },
	{ "weapon_m82",         "帕克-黑尔M82",           4700, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1 },
	{ "weapon_m24",         "狂蟒之吻M24",           4500, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1 },
	{ "weapon_trg42",         "芬兰之星TRG-42",           4750, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1 },
	{ "weapon_trg42g",         "芬兰之星(白金典藏)",           4750, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1 },
	{ "weapon_aw50",         "AW50",           5000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1 },
    {"weapon_plasmagun",      "破晓黎明",    0, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 50},
	{ "knife_skullaxe",     "旋风",               0,    KNIFE_SLOT,          UNASSIGNED, BUY_KNIFE, 60 },
	{ "weapon_m95",         "巴雷特",           5000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1 },
	{ "weapon_m95xmas",         "圣诞巴雷特",                  5000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1 },
	{ "weapon_wa2000",      "狂怒骑士",                4600, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1 },
	{ "weapon_as50",        "末日骑士",                  5000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1 },
	{ "weapon_as50g",        "末日金骑",                  5000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1 },
	{ "weapon_bendita",        "冰封圣骑",                  5500, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1 },
	{ "weapon_bpgm",        "PGM Hecate Ⅱ战损版",                  4900, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1 },
	{ "weapon_sl8",        "猎魂骑士SL8",                  4600, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1 },
	{ "weapon_sl8g",        "猎魂金骑(装弹强化)",                  4600, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1 },
	{ "weapon_sl8ex",        "猎魂骑士EX",                  4600, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1 },
	{ "weapon_zgun",        "异变之源AWP-Z",                  4800, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1 },
	{ "weapon_stg44",        "咆哮飓风",                  3100, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1 },
	{ "weapon_lightzg",        "暗影芭比啵啵枪",                  3500, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1 },
	{ "weapon_heavyzg",        "憎恶屠夫噗噗枪",                  3500, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1 },
	{ "weapon_m249ex",        "碎魂者",                  9000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG, 1 },
	{ "weapon_hk23",        "追击者HK23",                  4600, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG, 1 },
	{ "weapon_m60",        "掠食者M60E4",                  4700, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG, 1 },
	{ "weapon_mk48",        "劫掠者MK48",                  5500, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG, 1 },
	{ "weapon_k3",        "k3",                  4750, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG, 1 },
	{ "weapon_sfmg",         "赤色彗星",           5750, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG, 1 },
	{ "weapon_bison",         "野牛PP-Bison",                  1400, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SMG, 1 },
	{ "weapon_m4a1dragon",         "青龙M4A1",                  3100, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1 },
	{ "weapon_ak47dragon",         "赤龙AK47",                  2500, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1 },
	{ "weapon_ak47g",         "雅金AK",                  2500, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE, 1 },
	{ "knife_dragonsword",     "青龙偃月刀",               0,    KNIFE_SLOT,          UNASSIGNED, BUY_KNIFE, 1 },
	{ "knife_knifedragon",         "龙吻",                  0, KNIFE_SLOT, UNASSIGNED, BUY_KNIFE, 1 },
	{ "knife_nataknife",      "背刺银刃",    0, KNIFE_SLOT, UNASSIGNED, BUY_KNIFE, 1 },
	{ "knife_jknife",      "星芒蝶翼",    0, KNIFE_SLOT, UNASSIGNED, BUY_KNIFE, 1 },
	{ "knife_combat",      "灭灵军刺",    0, KNIFE_SLOT, UNASSIGNED, BUY_KNIFE, 1 },
	{ "knife_mastercombat",      "灭灵军刺•锯刃",    0, KNIFE_SLOT, UNASSIGNED, BUY_KNIFE, 1 },
	{ "knife_hdagger",      "兽王铜匕",    0, KNIFE_SLOT, UNASSIGNED, BUY_KNIFE, 1 },
	{ "knife_hammer",      "风暴之锤",    0, KNIFE_SLOT, UNASSIGNED, BUY_KNIFE, 1 },
	{ "knife_axe",      "摄魂魔镰",    0, KNIFE_SLOT, UNASSIGNED, BUY_KNIFE, 1 },
	{"knife_katana",     "狂花乱舞",               0,    KNIFE_SLOT,          UNASSIGNED, BUY_KNIFE, 1},
    {"knife_balrog9",      "爆裂拳套",    0, KNIFE_SLOT, UNASSIGNED, BUY_KNIFE, 35},
    {"knife_janus9",     "灾厄",               0,    KNIFE_SLOT,          UNASSIGNED, BUY_KNIFE, 1 },
    {"knife_thanatos9",      "陨裂",    0, KNIFE_SLOT, UNASSIGNED, BUY_KNIFE, 100},
    {"weapon_gatling",      "加特林",               4000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN, 1 },
    {"weapon_chainsaw",         "生命收割者",                  5750, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP, 1},
    {"weapon_cannon",      "黑龙炮",    3800, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP, 60},
	{ "weapon_zombielaw",      "test",    100, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP, 60 },
		//{ "weapon_pumpkin",     "南瓜手雷",               600, GRENADE_SLOT, UNASSIGNED, BUY_EQUIP, 1 },
	//{ "weapon_holybomb",     "圣水手雷",              600, GRENADE_SLOT, UNASSIGNED, BUY_EQUIP, 1 },
};

}

#endif
