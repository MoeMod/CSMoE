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

	int iLevel;
};

inline std::vector<MoEWeaponBuyInfo_s> g_MoEWeaponBuyInfo = {
    { WEAPON_USP, "weapon_usp",      "USP45",    500, PISTOL_SLOT, UNASSIGNED, BUY_PISTOL ,5},
    { WEAPON_GLOCK18, "weapon_glock18",      "Glock-18",    400, PISTOL_SLOT, UNASSIGNED, BUY_PISTOL ,55},
    { WEAPON_P228, "weapon_p228",      "P228",    600, PISTOL_SLOT, UNASSIGNED, BUY_PISTOL ,8},
    { WEAPON_DEAGLE, "weapon_deagle",      "Deagle",    650, PISTOL_SLOT, UNASSIGNED, BUY_PISTOL ,7},
    { WEAPON_ELITE, "weapon_elite",      "Elite",    800, PISTOL_SLOT, UNASSIGNED, BUY_PISTOL ,12},
    { WEAPON_FIVESEVEN, "weapon_fiveseven",      "FiveSeven",    750, PISTOL_SLOT, UNASSIGNED, BUY_PISTOL ,25},
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


constexpr MoEWeaponBuyInfo_s g_MoEWeaponBuyInfoLocal[] = {
#if 0
	{ WEAPON_VULCANUS11, "weapon_vulcanus11",       "逆天行Vulcanus-11",                 4000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SMG },


	{ WEAPON_VULCANUS3, "weapon_vulcanus3",       "逆天行Vulcanus-3",                 4000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SMG },

	{ KNIFE_Y22S3DRAGONSWORD, "knife_y22s3dragonsword",     "遗迹丛林•狂暴者",               0,    KNIFE_SLOT,          UNASSIGNED, BUY_KNIFE },

	{ WEAPON_JANUS7, "weapon_janus7",         "雷狱Janus-7",                  9000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG },
	{ WEAPON_Y22S3JANUS7, "weapon_y22s3janus7",         "遗迹丛林•毁灭者",                  9000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG },

	{ WEAPON_RESTRICTIONPISTOL, "weapon_restrictionpistol",	 "wdnmd",			  2000,  PISTOL_SLOT,         UNASSIGNED, BUY_PISTOL },
	{ GRENADE_M24GRENADE, "weapon_m24grenade",     "M24柄式手雷",              300, GRENADE_SLOT, UNASSIGNED, BUY_EQUIP },
	{ GRENADE_FGRENADE2, "weapon_fgrenade2",     "破片高爆雷",              300, GRENADE_SLOT, UNASSIGNED, BUY_EQUIP },
	{ GRENADE_FIREBOMB, "weapon_fgrenade",     "火焰高爆雷",              300, GRENADE_SLOT, UNASSIGNED, BUY_EQUIP },
	{ GRENADE_HEARTBOMB, "weapon_heartbomb",     "心形手雷",              300, GRENADE_SLOT, UNASSIGNED, BUY_EQUIP },
	{ GRENADE_CAKEBOMB, "weapon_cake",     "周年纪念蛋糕手雷",              300, GRENADE_SLOT, UNASSIGNED, BUY_EQUIP },
	{ GRENADE_MOONCAKE, "weapon_mooncake",     "雷馅月饼",              300, GRENADE_SLOT, UNASSIGNED, BUY_EQUIP },
	{ GRENADE_CARTFRAG, "weapon_cartfrag",     "皮蛋爆雷",              300, GRENADE_SLOT, UNASSIGNED, BUY_EQUIP },
	{ GRENADE_SFGRENADE, "weapon_sfgrenade",     "电浆磁爆雷",              300, GRENADE_SLOT, UNASSIGNED, BUY_EQUIP },
	{ GRENADE_CHAINGRENADE, "weapon_chaingren",     "狂鲨爆雷",              300, GRENADE_SLOT, UNASSIGNED, BUY_EQUIP },

	{ WEAPON_WONDERCANNONEX, "weapon_wondercannonex",        "冰麟寒焱",                  6250, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP },

		{ GRENADE_HOLYBOMB, "weapon_holybomb",     "圣水手雷",              600, GRENADE_SLOT, UNASSIGNED, BUY_EQUIP },

	{ WEAPON_M3DRAGON, "weapon_m3dragon",         "【天龙】m3",           5000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN },
	{ WEAPON_M3DRAGONM, "weapon_m3dragonm",         "巨齿鲲鲨",           5000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN },

	{ WEAPON_REVIVEGUN, "weapon_revivegun",	 "逆界星轮",			  6000,  PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP },

	{ KNIFE_DUALSWORD, "knife_dualsword",    "弑神双剑",	0, KNIFE_SLOT,          UNASSIGNED, BUY_KNIFE },

	{ WEAPON_BUFFFIVESEVEN, "weapon_bufffiveseven",	 "【超兽】FiveSeven",			 1800,  PISTOL_SLOT,         UNASSIGNED, BUY_PISTOL },

	{ WEAPON_BUFFNG7, "weapon_buffng7",        "【壁垒】NG7",                  9000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG },

	{ WEAPON_TURBULENT1, "weapon_turbulent1",	 "涌泉Turbulent-1",			  5000,  PISTOL_SLOT,         UNASSIGNED, BUY_PISTOL },
	{ WEAPON_MUSKET, "weapon_musket",	 "神怒之炎",			  950,  PISTOL_SLOT,         UNASSIGNED, BUY_PISTOL },
		{ WEAPON_GILBOAEX, "weapon_gilboaex",         "蝰蛇勇士EX",                  4100, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },
		{ WEAPON_SPAS12, "weapon_spas12",       "爆裂炙炎SPAS-12",                 2100, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN },
	{ WEAPON_SPAS12EX, "weapon_spas12ex",       "爆裂炙炎EX",                 3500, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN },
	{ WEAPON_SPAS12EX2, "weapon_spas12ex2",       "爆裂炙炎DX",                 3500, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN },


		{ KNIFE_ARMTORCH, "knife_armtorch",    "爆焰火拳",	0, KNIFE_SLOT,          UNASSIGNED, BUY_KNIFE },
		{ WEAPON_DUALUZI, "weapon_dualuzi",        "尤利乌斯",                  5000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SMG },
	{ WEAPON_SKULL2, "weapon_skull2",	 "噬血狂袭SKULL-2",			  2200,  PISTOL_SLOT,         UNASSIGNED, BUY_PISTOL },
		{ WEAPON_DUALKRISSHERO, "weapon_dualkrisshero",        "英雄双刺",                  5000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SMG },
		{ WEAPON_M1GARAND, "weapon_m1garand",         "加兰德M1",                  3400, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },

		{ WEAPON_STARCHASERSR, "weapon_starchasersr",      "灭却星光",               5700, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },

	{ WEAPON_OICW, "weapon_oicw",      "尖端勇士OICW",    4850, PRIMARY_WEAPON_SLOT, UNASSIGNED,  BUY_RIFLE },

	{ WEAPON_THANATOS5, "weapon_thanatos5",      "殇魁Thanatos-5",    6600, PRIMARY_WEAPON_SLOT, UNASSIGNED,  BUY_RIFLE },

		{ WEAPON_BOUNCER, "weapon_bouncer",       "破灭光雷",                 5500, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN },
	{ WEAPON_COILGUN, "weapon_coilgun",	 "千针",			  4750,  PRIMARY_WEAPON_SLOT,         UNASSIGNED, BUY_SMG },

		{ WEAPON_BLOCKAS, "weapon_blockas",        "玩具魔兵M777",                  5800, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN },
	{ WEAPON_BLOCKMG, "weapon_blockmg",        "玩具路霸Leopard-2",                  7500, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG },

		{ WEAPON_BLOCKAR, "weapon_blockar",        "玩具奇兵V2",                  6000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },
		{ WEAPON_WINGGUN, "weapon_winggun",        "曙光辉翼",                  6100, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP },

		{ WEAPON_DESTROYER, "weapon_destroyer",         "虚空破碎",           5500, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },
	{ WEAPON_SPSG, "weapon_spsg",         "暴乱蒸汽",           5650, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN },

	{ WEAPON_KINGCOBRA, "weapon_kingcobra",	 "眼镜王蛇Kingcobra",			  650,  PISTOL_SLOT,         UNASSIGNED, BUY_PISTOL },
	{ WEAPON_SPEARGUN, "weapon_speargun",         "海皇之怒",           5800, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP },

	{ WEAPON_SPEARGUNM, "weapon_speargunm",         "血猎重弩",           6000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP },

	{ WEAPON_FGLAUNCHER, "weapon_fglauncher",        "礼花发射器",                  6500, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP },

	{ GRENADE_BUNKERBUSTER, "weapon_bunkerbuster",      "天降正义LTD",    9000, GRENADE_SLOT, UNASSIGNED, BUY_EQUIP },
	{ WEAPON_Z4B_M4A1RAZER, "z4b_m4a1razer",      "雷蛇M4A1",    4250, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },

		{ WEAPON_Z4B_AK47X, "z4b_ak47x",      "黑武士AK47",    4288, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },
	{ WEAPON_Z4B_M4A1MW, "z4b_m4a1mw",      "暗羽冥王M4A1",    3600, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },

	{ KNIFE_Z4B_FREQUENCY9, "z4b_Frequency9",      "迷迭F9",    0, KNIFE_SLOT, UNASSIGNED, BUY_KNIFE },

	{ WEAPON_GUILLOTINE, "weapon_guillotine",         "盘龙血煞",           7500, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP },
	{ WEAPON_GUILLOTINEEX, "weapon_guillotineex",         "【弧光】回旋刃",           7500, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP },

	{ WEAPON_LASERFISTEX, "weapon_laserfistex",         "【无限】机械臂铠MK2",           10000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SMG },

		{ WEAPON_LASERFIST, "weapon_laserfist",         "【无限】机械臂铠",           10000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SMG },
	{ WEAPON_KRONOS3, "weapon_kronos3",        "追踪者X-90",                  3100, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SMG },
	{ WEAPON_KRONOS1, "weapon_kronos1",        "制御者X-45",                  550, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_PISTOL },
	{ WEAPON_KRONOS7, "weapon_kronos7",        "捍卫者X-7",                  5200, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG },
	{ WEAPON_AN94, "weapon_an94",         "AN94",                  2800, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },
	{ WEAPON_M16A4, "weapon_m16a4",         "M16A4",                  2800, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },

	{ WEAPON_KRONOS5, "weapon_kronos5",         "猎杀者X-15",                  4000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },

		{ WEAPON_THANATOS11, "weapon_thanatos11",         "魄灭Thanatos-11",           6000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN },

		{ WEAPON_BALROG5, "weapon_balrog5",      "地狱犬Balrog-Ⅴ",    6450, PRIMARY_WEAPON_SLOT, UNASSIGNED,  BUY_RIFLE },


	{ WEAPON_THUNDERPISTOL, "weapon_thunderpistol",	 "【幽影】Thunder.50",			  1500,  PISTOL_SLOT,         UNASSIGNED, BUY_PISTOL },

	{ WEAPON_M95TIGER, "weapon_m95tiger",         "【虎王】M95",           5000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },
	{ WEAPON_CHEYTACLRRS, "weapon_cheytaclrrs",         "CheytacLRRS-M200",           5000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },

	{ WEAPON_MGSM, "weapon_mgsm",        "【合金】M2-HB",                  9000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG },

	{ WEAPON_CAMERAGUN, "weapon_cameragun",        "偶像大师",                  5000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG },
	{ WEAPON_M1887G, "weapon_m1887g",       "退魔金焰(攻击强化)",                 2800, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN },
	{ WEAPON_M1887XMAS, "weapon_m1887xmas",       "退魔圣焰(圣诞特别版)",                 2800, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN },
		{ WEAPON_RAINBOWGUN, "weapon_rainbowgun",       "画梅",                 2800, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN },
	{ WEAPON_BUFFSG552, "weapon_buffsg552",         "【狼魂】SG552",                  7000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },
	{ WEAPON_BUFFAUG, "weapon_buffaug",         "【无双】AUG",                  6500, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },
	{ WEAPON_BLOODHUNTER, "weapon_bloodhunter",	 "【血契】D.Eagle",			  1800,  PISTOL_SLOT,         UNASSIGNED, BUY_PISTOL },
	{ WEAPON_LOCKONGUN, "weapon_lockongun",      "【暮光】F2000",    5500, PRIMARY_WEAPON_SLOT, UNASSIGNED,  BUY_RIFLE },
		{ GRENADE_PATROLDRONE, "weapon_patroldrone",      "【幽浮】控制核心",    3000, GRENADE_SLOT, UNASSIGNED, BUY_EQUIP },
	{ WEAPON_SPMG, "weapon_spmg",        "炙热蒸汽",                  7000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG },

{ WEAPON_FLAMETHROWER, "weapon_airburster",        "风魔啸天破",                  5800, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP },
	{ WEAPON_WATERCANNON, "weapon_watercannon",        "极冻水加农",                  5450, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP },
	{ WEAPON_AIRBURSTER, "weapon_flamethrower",        "焚烬者",                  5750, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP },
	{ WEAPON_POISONGUN, "weapon_poisongun",        "腐蚀烈焰",                  5750, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP },
{ WEAPON_CHARGER5, "weapon_charger5",      "追电Charger-5",    5500, PRIMARY_WEAPON_SLOT, UNASSIGNED,  BUY_RIFLE },
	{ WEAPON_CHARGER7, "weapon_charger7",        "雷暴Charger-7",                  5000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG },
	{ WEAPON_JANUS7XMAS, "weapon_janus7xmas",         "圣诞雷狱",                  9000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG },
	{ WEAPON_GUNGNIR, "weapon_gungnir",     "断狱雷魂",               6000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP },
		{ WEAPON_STUNRIFLE, "weapon_stunrifle",      "雷电风暴",    3600, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },

	{ WEAPON_PIANOGUNEX, "weapon_pianogunex",        "【命运】寂静之音EX",                  7000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP },
	{ WEAPON_COILMG, "weapon_coilmg",        "雷霆破灭者",                  9000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG },
	{ WEAPON_BALROG3, "weapon_balrog3",         "灼魂Balrog-Ⅲ",           4350, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SMG },
	{ WEAPON_BALROG1, "weapon_balrog1",	 "红莲Balrog-Ⅰ",			  1500,  PISTOL_SLOT,         UNASSIGNED, BUY_PISTOL },
	{ WEAPON_SAPIENTIA, "weapon_sapientia",	 "冰魄",			  1300,  PISTOL_SLOT,         UNASSIGNED, BUY_PISTOL },
	{ WEAPON_BALROG11, "weapon_balrog11",        "龙炎",                  5750, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN },
	{ WEAPON_BALROG11B, "weapon_balrog11b",        "凛风",                  5750, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN },
	{ WEAPON_BALROG11WC, "weapon_balrog11wc",        "黄金龙炎",                  5750, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN },
	{ WEAPON_SGMISSILE, "weapon_sgmissile",        "撼宇碎星",                  6400, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP },
	{ WEAPON_SGMISSILEM, "weapon_sgmissilem",        "爆烈神机弩",                  6250, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP },
	{ WEAPON_SGMISSILEEX, "weapon_sgmissileex",        "撼宇碎星Zero",                  6250, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP },

	{ WEAPON_BENDITA, "weapon_bendita",        "冰封圣骑",                  5500, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },
	{ WEAPON_BROAD, "weapon_broad",        "冰瀑毁灭者",                  7200, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG },
	{ WEAPON_CROSSBOW, "weapon_crossbow",      "追月连弩",    3500, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },
	{ WEAPON_Z4B_M60AMETHYST, "z4b_m60amethyst",      "致命紫晶-M60",    100, PRIMARY_WEAPON_SLOT, UNASSIGNED,  BUY_MG },
	{ WEAPON_Z4B_DBARRELAMETHYST, "z4b_dbarrelamethyst",      "致命紫晶-暴焱",    100, PRIMARY_WEAPON_SLOT, UNASSIGNED,  BUY_SHOTGUN },
	{ WEAPON_Z4B_DEAGLEAMETHYST, "z4b_deagleamethyst",      "致命紫晶-夜鹰",    100, PISTOL_SLOT, UNASSIGNED,  BUY_PISTOL },
	{ WEAPON_Z4B_ACRAMETHYST, "z4b_acramethyst",      "致命紫晶-scar",    100, PRIMARY_WEAPON_SLOT, UNASSIGNED,  BUY_RIFLE },
	{ WEAPON_Z4B_AR57AMETHYST, "z4b_ar57amethyst",      "致命紫晶-ar57",    100, PRIMARY_WEAPON_SLOT, UNASSIGNED,  BUY_RIFLE },
	{ WEAPON_CROW11, "weapon_crow11",      "鬼雕CROW-11",    5000, PRIMARY_WEAPON_SLOT, UNASSIGNED,  BUY_SHOTGUN },
	{ WEAPON_JANUS3, "weapon_janus3",      "裂空Janus-3",    2000, PRIMARY_WEAPON_SLOT, UNASSIGNED,  BUY_SMG },
	{ WEAPON_CROW3, "weapon_crow3",      "骸隼CROW-3",    4000, PRIMARY_WEAPON_SLOT, UNASSIGNED,  BUY_SMG },
	{ WEAPON_JANUSMK5, "weapon_janusmk5",      "幻影Janus-5",    6000, PRIMARY_WEAPON_SLOT, UNASSIGNED,  BUY_RIFLE },
	{ WEAPON_CROW5, "weapon_crow5",      "黑鹰CROW-5",    5500, PRIMARY_WEAPON_SLOT, UNASSIGNED,  BUY_RIFLE },
	{ WEAPON_JANUS11, "weapon_janus11",        "极光Janus-11",                 5200, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN },
	{ WEAPON_LIGHTZG, "weapon_lightzg",        "暗影芭比啵啵枪",                  3500, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },
	{ WEAPON_HEAVYZG, "weapon_heavyzg",        "憎恶屠夫噗噗枪",                  3500, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },
	{ WEAPON_BOW, "weapon_bow",      "鹰眼",               6200, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },
	{ WEAPON_SKULL11, "weapon_skull11",        "雷霆SKULL-11",                 6500, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN },
	{ WEAPON_RAILCANNON, "weapon_railcannon",        "电浆轨道炮",                 5000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN },
	{ WEAPON_BUFFAWP, "weapon_buffawp",        "【傲君】AWP",                  8000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },
	{ WEAPON_WONDERCANNON, "weapon_wondercannon",        "炽麟煌焱",                  6250, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP },
	{ WEAPON_SFPISTOL, "weapon_sfpistol",	 "死亡射线",			  1500,  PISTOL_SLOT,         UNASSIGNED, BUY_PISTOL },
	{ WEAPON_Y22S2SFPISTOL, "weapon_y22s2sfpistol",	 "死亡射线EX",			  1500,  PISTOL_SLOT,         UNASSIGNED, BUY_PISTOL },
	{ KNIFE_Y22S2SFSWORD, "knife_y22s2sfsword",      "粒子激光剑Ex",    0, KNIFE_SLOT, UNASSIGNED, BUY_KNIFE },
	{ WEAPON_PIANOGUN, "weapon_pianogun",        "【命运】寂静之音",                  7000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP },
	{ WEAPON_PIANOGUNEX, "weapon_pianogunex",        "【命运】寂静之音EX",                  7000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP },
	{ WEAPON_CHAINSAWEX, "weapon_chainsawex",         "光子收割者",                  4000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP },
	{ WEAPON_LASERSG, "weapon_lasersg",         "【异噬】暗雷兽",                  4000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN },
	{ WEAPON_Z4B_RAGINGBULL, "z4b_ragingbull",         "破空枪刃",                  1200, PISTOL_SLOT, UNASSIGNED, BUY_PISTOL },
	{ GRENADE_HEGRENADE, "weapon_hegrenade",     "高爆手雷",               300, GRENADE_SLOT, UNASSIGNED, BUY_EQUIP },
	{ GRENADE_PUMPKIN, "weapon_pumpkin",     "南瓜手雷",               600, GRENADE_SLOT, UNASSIGNED, BUY_EQUIP },
	{ WEAPON_KRONOS12, "weapon_kronos12",        "粉碎者X-12",                 3500, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN },
	{ WEAPON_CROSSBOWEX21, "weapon_crossbowex21",      "【歼星】十字弩",    5000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },
	{ GRENADE_DIVINETITAN, "weapon_divinetitan",      "圣翼皓印",    0, GRENADE_SLOT, UNASSIGNED, BUY_EQUIP },
	{ WEAPON_SGDRILLEX, "weapon_sgdrillex",         "【电光】狂龙钻", 2000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP },
	{ WEAPON_Z4B_AQUARIUSPLASMAGUN, "z4b_aquariusplasmagun",      "破晓黎明 - 水瓶座",    0, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },
	//{ WEAPON_FGLAUNCHER, "weapon_fglauncher",         "礼花发射器", 2000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP },
#else
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

	{ WEAPON_INFINITY, "weapon_infinity",    "恒宇双星", 1500, PISTOL_SLOT,         UNASSIGNED, BUY_PISTOL },
	{ WEAPON_INFINITYEX1, "weapon_infinityex1", "星红双子",   1500, PISTOL_SLOT,         UNASSIGNED, BUY_PISTOL },
	{ WEAPON_INFINITYEX2, "weapon_infinityex2", "金红双蝎",     1500, PISTOL_SLOT,         UNASSIGNED, BUY_PISTOL },
	{ WEAPON_INFINITYSS, "weapon_infinityss",  "恒宇银星",       600,  PISTOL_SLOT,         UNASSIGNED, BUY_PISTOL },
	{ WEAPON_INFINITYSR, "weapon_infinitysr",  "恒宇红星",          600,  PISTOL_SLOT,         UNASSIGNED, BUY_PISTOL },
	{ WEAPON_INFINITYSB, "weapon_infinitysb",  "恒宇黑星",        600,  PISTOL_SLOT,         UNASSIGNED, BUY_PISTOL },
	{ WEAPON_ANACONDA, "weapon_anaconda",	 "左轮军魂",			  650,  PISTOL_SLOT,         UNASSIGNED, BUY_PISTOL },

	{ WEAPON_MP7A1_PISTOL, "weapon_mp7a1p",      "MP7A1",                 2150, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SMG },
	{ WEAPON_XM8CARBINE, "weapon_xm8c",        "XM8",                   3250, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },
	{ WEAPON_SCARLIGHT, "weapon_scarl",       "Scar",                  3250, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },
	{ WEAPON_M1887, "weapon_m1887",       "退魔圣焰",                 2800, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN },
	{ WEAPON_KRISS, "weapon_kriss",       "致命蝎刺",                 1700, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SMG },
	{ WEAPON_K1A, "weapon_k1a",         "K1A",                   1850, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SMG },
	{ WEAPON_THOMPSON, "weapon_thompson",    "汤姆逊",              2000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SMG },
	{ WEAPON_M14EBR, "weapon_m14ebr",      "咆哮怒焰",               3100, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },
	{ WEAPON_M2, "weapon_m2",      "守护者",               6200, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG },
	{ WEAPON_M134, "weapon_m134",      "终结者",               7000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG },
	{ WEAPON_M134EX, "weapon_m134ex",      "终结者EX",               7000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG },
	{ WEAPON_TBARREL, "weapon_tbarrel",         "破碎炙焱",                  2600, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN },
	{ WEAPON_M134XMAS, "weapon_m134xmas",         "圣诞终结者",                  7000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG },
	{ WEAPON_MG36, "weapon_mg36",         "开拓者",                  5750, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG },
	{ WEAPON_TAR21, "weapon_tar21",       "塔沃尔",                3600, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },
	{ WEAPON_MG3, "weapon_mg3",         "毁灭者",                  5750, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG },
	{ WEAPON_MG3XMAS, "weapon_mg3xmas",         "圣诞毁灭者",                  5750, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG },
	{ WEAPON_BOW, "weapon_bow",      "鹰眼",               6200, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },
	{ WEAPON_BALROG7, "weapon_balrog7",      "炎魔",               9000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG },
	{ WEAPON_STARCHASERAR, "weapon_starchaserar",      "璀璨星辰",               5000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },
	{ KNIFE_SKULLAXE, "knife_skullaxe",     "旋风",               0,    KNIFE_SLOT,          UNASSIGNED, BUY_KNIFE },
	{ WEAPON_M95, "weapon_m95",         "巴雷特",           5000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },
	{ WEAPON_M95XMAS, "weapon_m95xmas",         "圣诞巴雷特",                  5000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },
	{ WEAPON_WA2000, "weapon_wa2000",      "狂怒骑士",                4600, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },
	{ WEAPON_AS50, "weapon_as50",        "末日骑士",                  5000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },
	{ WEAPON_JANUS7XMAS, "weapon_janus7xmas",         "圣诞雷狱",                  9000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG },
	{ WEAPON_VOIDPISTOL, "weapon_voidpistol",         "黑曜审判",                  2200, PISTOL_SLOT, UNASSIGNED, BUY_PISTOL ,25 },
	{ WEAPON_GUNKATA, "weapon_gunkata",	 "【深渊】Elites",			  1500,  PISTOL_SLOT,         UNASSIGNED, BUY_PISTOL ,18 },
	{ WEAPON_DESPERADO, "weapon_desperado",         "【永恒】Python",                  1500, PISTOL_SLOT, UNASSIGNED, BUY_PISTOL ,15 },
	{ WEAPON_K3, "weapon_k3",        "k3",                  4750, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG },
	{ WEAPON_M4A1DRAGON, "weapon_m4a1dragon",         "青龙M4A1",                  3100, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },
	{ WEAPON_AK47DRAGON, "weapon_ak47dragon",         "赤龙AK47",                  2500, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE },

	{ WEAPON_KNIFE, "weapon_knife", "海豹短刀",            0,    KNIFE_SLOT,          UNASSIGNED, BUY_KNIFE },
	{ KNIFE_DRAGONSWORD, "knife_dragonsword",     "青龙偃月刀",               0,    KNIFE_SLOT,          UNASSIGNED, BUY_KNIFE },
	{ KNIFE_DRAGON, "knife_knifedragon",         "龙吻",                  0, KNIFE_SLOT, UNASSIGNED, BUY_KNIFE },
	{ KNIFE_STRONGKNIFE, "knife_nataknife",      "背刺银刃",    0, KNIFE_SLOT, UNASSIGNED, BUY_KNIFE },
	{ KNIFE_KATANA, "knife_katana",     "狂花乱舞",               0,    KNIFE_SLOT,          UNASSIGNED, BUY_KNIFE },
	{ KNIFE_DUALSWORD, "knife_dualsword",    "【弑神】双剑",	0, KNIFE_SLOT,          UNASSIGNED, BUY_KNIFE ,30 },
	{ KNIFE_Z4B_FREQUENCY9, "z4b_Frequency9",      "迷迭F9",    0, KNIFE_SLOT, UNASSIGNED, BUY_KNIFE ,8 },
	{ KNIFE_Z4B_LCSWORD, "z4b_lc_sword",      "军团指挥官圣剑",    0, KNIFE_SLOT, UNASSIGNED, BUY_KNIFE ,5 },
	{ KNIFE_Z4B_FACELESSVOID, "z4b_facelessvoid",      "虚空假面智杖",    0, KNIFE_SLOT, UNASSIGNED, BUY_KNIFE ,5 },
	{ KNIFE_Z4B_STRONGKNIFEX, "z4b_strongknifex",      "新月刃",    0, KNIFE_SLOT, UNASSIGNED, BUY_KNIFE ,5 },
	{ KNIFE_Z4B_NATAKNIFEDX, "z4b_nataknifedx",      "双持新月刃",    0, KNIFE_SLOT, UNASSIGNED, BUY_KNIFE ,5 },
	{ KNIFE_Z4B_OPPOVIVO, "z4b_oppovivo",      "绝地光剑",    0, KNIFE_SLOT, UNASSIGNED, BUY_KNIFE ,5 },
	{ KNIFE_Z4B_STORMGIANTX, "z4b_stormgiantx",      "骇影巨锤",    0, KNIFE_SLOT, UNASSIGNED, BUY_KNIFE ,12 },

	{ WEAPON_GATLING, "weapon_gatling",      "加特林",               4000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN },
	{ WEAPON_GATLINGEX, "weapon_gatlingex",        "【日冕】加特林",                  4600, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN ,15 },
	{ WEAPON_CHAINSAW, "weapon_chainsaw",         "生命收割者",                  5750, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP },
	{ WEAPON_CANNON, "weapon_cannon",      "黑龙炮",    3800, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP },
	{ WEAPON_GUNGNIR, "weapon_gungnir",     "断狱雷魂",               6000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP ,18 },
	{ WEAPON_SGDRILL, "weapon_sgdrill",         "怒海狂鲨",                 6250, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP ,15 },
	{ GRENADE_HEGRENADE, "weapon_hegrenade",     "高爆手雷",               300, GRENADE_SLOT, UNASSIGNED, BUY_EQUIP },

	{ WEAPON_Z4B_INFINITYX, "z4b_infinityx", "英菲尼迪双枪",     1500, PISTOL_SLOT,         UNASSIGNED, BUY_PISTOL ,5 },
	{ WEAPON_Z4B_DEAGLESHURA, "z4b_deagleb",      "修罗",    700, PISTOL_SLOT, UNASSIGNED, BUY_PISTOL ,8 },
	{ WEAPON_Z4B_CANNONPLASMA, "z4b_cannonplasma",      "黎明离子炮",    3800, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP ,7 },
	{ WEAPON_Z4B_CANNONTIGER, "z4b_cannontiger",      "裂地虎炮",    3800, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP ,7 },
	{ WEAPON_Z4B_M134HEROI, "z4b_m134heroi",      "漩涡i7",    7700, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG ,13 },
	{ WEAPON_Z4B_XM2010PC, "z4b_xm2010pc",      "罪恶虚空",    7700, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE ,12 },
	{ WEAPON_Z4B_AWPNVIDIA, "z4b_awpnvidia",      "矿难AWP",    4650, PRIMARY_WEAPON_SLOT, UNASSIGNED,BUY_RIFLE ,11 },
	{ WEAPON_Z4B_M4A1RAZER, "z4b_m4a1razer",      "雷蛇M4A1",    4250, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE ,10 },
	{ WEAPON_Z4B_MP7A1DX, "z4b_dmp7a1x",      "双持审判效应N7",    2350, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SMG ,8 },
	{ WEAPON_Z4B_M4A1X, "z4b_m4a1x",      "绝影M4A1",    4288, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE ,14 },
	{ WEAPON_Z4B_AK47X, "z4b_ak47x",      "黑武士AK47",    4288, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE ,14 },
	{ WEAPON_Z4B_M4A1MW, "z4b_m4a1mw",      "暗羽冥王M4A1",    3600, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE ,14 },
	{ WEAPON_Z4B_BARRETTD, "z4b_barrettd",        "毁灭巴菲特",                  5288, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE ,14 },
	
	{ WEAPON_CSGO_CZ75, "csgo_cz75",         "CZ75",                  500, PISTOL_SLOT, UNASSIGNED, BUY_PISTOL ,1 },
	{ WEAPON_CSGO_TEC9, "csgo_tec9",         "Tec-9",                  500, PISTOL_SLOT, UNASSIGNED, BUY_PISTOL ,1 },
	{ WEAPON_CSGO_R8, "csgo_r8",        "左轮R8",                  850, PISTOL_SLOT, UNASSIGNED, BUY_PISTOL ,1 },
	{ WEAPON_CSGO_NEGEV, "csgo_negev",         "内格夫Negev",                  1700, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG ,1 },
	{ WEAPON_CSGO_MAG7, "csgo_mag7",         "警喷MAG-7",                  1800, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN },
	{ WEAPON_CSGO_SAWEDOFF, "csgo_sawedoff",         "匪喷Sawed-Off",                  1100, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN ,1 },
	{ WEAPON_CSGO_BIZON, "csgo_bizon",         "野牛PP-Bizon",                  1400, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SMG ,1 },

	{ WEAPON_LINKATTACKSG, "weapon_linkattacksg",         "【星域】霰弹枪",                  6000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN ,23 },
	{ WEAPON_FGLAUNCHER, "weapon_fglauncher",        "礼花发射器",                  6500, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP },
	{ KNIFE_ARMTORCH, "knife_armtorch",    "爆焰火拳",	0, KNIFE_SLOT,          UNASSIGNED, BUY_KNIFE },
	{ WEAPON_PETROLBOOMER, "weapon_petrolboomer",         "暴风烈焰",                  7900, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP ,3 },
	{ WEAPON_USAS12, "weapon_usas12",        "USAS12",                 3350, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN , 1 },
	{ WEAPON_TURBULENT11, "weapon_turbulent11",        "飞瀑Turbulent-11",                 5000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN ,3 },
	{ WEAPON_KSG12, "weapon_ksg12",         "灼热剃刀KSG12",           2800, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN ,3 },
	{ WEAPON_DBARREL, "weapon_dbarrel",         "破碎炙炎DoubleBarrel",           2500, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN ,3 },
	{ WEAPON_Z4B_VK47, "z4b_vk47",      "平行步枪",    2500, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE ,2 },
	{ WEAPON_STERLINGBAYONET, "weapon_sterlingbayonet",        "斯特林-断剑",     5000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SMG , 1 },
	{ WEAPON_AUGEX, "weapon_augex",      "强袭AUG",               5000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE , 3 },
	{ WEAPON_SFLASER, "weapon_sflaser",      "粒子风暴",               6500, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE ,3 },
	{ WEAPON_DARTPISTOL, "weapon_dartpistol",	 "爆裂镖镖枪",			  1500,  PISTOL_SLOT,         UNASSIGNED, BUY_PISTOL ,1 },
	{ WEAPON_PKM, "weapon_pkm",        "裁决者PKM",                  5600, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG ,2 },
	{ WEAPON_Z4B_MA5B, "z4b_ma5b",        "MA5B",                  2750, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE ,2 },
	{ WEAPON_TURBULENT1, "weapon_turbulent1",	 "涌泉Turbulent-1",			  1500,  PISTOL_SLOT,         UNASSIGNED, BUY_PISTOL ,5 },
	//{ GRENADE_BUNKERBUSTER, "weapon_bunkerbuster",      "天降正义LTD",    6000, GRENADE_SLOT, UNASSIGNED, BUY_EQUIP ,19 },
	//{ GRENADE_PATROLDRONE, "weapon_patroldrone",      "【幽浮】控制核心",    9000, GRENADE_SLOT, UNASSIGNED, BUY_EQUIP ,24 },
	//{ GRENADE_DIVINETITAN, "weapon_divinetitan",      "圣翼皓印",    9000, GRENADE_SLOT, UNASSIGNED, BUY_EQUIP ,32 },
	//{ KNIFE_TYRANTMACE, "knife_tyrantmace",     "【雪崩】冰霜巨锤",               0,    KNIFE_SLOT,          UNASSIGNED, BUY_KNIFE ,10 },
	//{ WEAPON_HEROCHAINSAW, "weapon_herochainsaw",      "2",    100, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP ,32 },
	{ WEAPON_Z4B_M82A1ANGELICBEAST, "z4b_m82a1angelicbeast",      "M82A1",    100, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE ,6 },
	{ KNIFE_GHOSTBLADE, "z4b_ghostblade",      "GhostBlade",    0, KNIFE_SLOT, UNASSIGNED, BUY_KNIFE ,6 },
	{ KNIFE_DUALSLAYER, "z4b_dualslayer",      "DualSlayer",    0, KNIFE_SLOT, UNASSIGNED, BUY_KNIFE ,6 },
	{ KNIFE_DUALSTINGER, "z4b_dualstinger",      "DualStinger",    0, KNIFE_SLOT, UNASSIGNED, BUY_KNIFE ,6 },
	{ KNIFE_HEARTSTIMULATER, "z4b_heartstimulater",      "HeartStimulater",    0, KNIFE_SLOT, UNASSIGNED, BUY_KNIFE ,6 },
	{ KNIFE_MECHANICHAND, "z4b_mechanichand",      "MechanicHand",    0, KNIFE_SLOT, UNASSIGNED, BUY_KNIFE ,6 },
	{ KNIFE_DUALAXE, "z4b_dualaxe",      "DualAxe",    0, KNIFE_SLOT, UNASSIGNED, BUY_KNIFE ,6 },
	{ WEAPON_Z4B_DEAGLEEL, "z4b_deagleel",      "DeagleEL",    100, PISTOL_SLOT, UNASSIGNED, BUY_PISTOL ,6 },
	{ WEAPON_AK47L3, "weapon_ak47l3",      "旋风AK-60R",    2500, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE ,2 },
	{ KNIFE_SHOVELEX, "z4b_shovelex",     "Shovel",               0,    KNIFE_SLOT,          UNASSIGNED, BUY_KNIFE ,10 },
	{ WEAPON_Z4B_RAGINGBULL, "z4b_ragingbull",         "破空枪刃",                  1200, PISTOL_SLOT, UNASSIGNED, BUY_PISTOL ,7 },
	{ WEAPON_Z4B_AK47_ANGELICBEAST, "z4b_ak47_angelicbeast",      "传说辉光",    4288, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE ,4 },
	{ WEAPON_LIGHTNINGSMG, "weapon_lightningsmg",         "闪电Bizon",                  3000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SMG ,10 },
	{ WEAPON_LIGHTNINGAR, "weapon_lightningar",         "闪电AN-94",                  3000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE ,7 },
	{ WEAPON_Z4B_STEYRTMPD, "z4b_steyrtmpd",        "死亡之眼",                  3100, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SMG ,8 },
	{ WEAPON_Z4B_M4A1_S_TRANSFORMERS , "z4b_m4a1_s_transformers",  "M4A1-雷神",    3100, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE ,1 },
	{ WEAPON_AT4EX, "weapon_at4ex",      "AT4CS火箭发射器",    7900, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP ,4 },
	{ WEAPON_CHAINMG , "weapon_chainmg",  "【黯火】M134",    10000, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG , 14 },
	{ WEAPON_SOULREAPER , "weapon_soulreaper",  "碧魄苍龙",    6100, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_EQUIP , 30 },
	{ WEAPON_LEAPSTRIKEGUN, "weapon_leapstrikegun",	 "次元裁决",			  2300,  PISTOL_SLOT,         UNASSIGNED, BUY_PISTOL ,20 },
#endif
};

}

#endif
