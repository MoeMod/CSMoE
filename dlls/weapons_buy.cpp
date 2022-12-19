#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "monsters.h"
#include "weapons.h"
#include "client.h"
#include "weapons_moe_buy.h"
#include "wpn_shared/wpn_patroldrone.h"


namespace sv {

/*
* Globals initialization
*/
DLL_GLOBAL AutoBuyInfoStruct g_autoBuyInfo[] =
{
	{ AUTOBUYCLASS_PRIMARY|AUTOBUYCLASS_RIFLE,		"galil",	"weapon_galil" },
	{ AUTOBUYCLASS_PRIMARY|AUTOBUYCLASS_RIFLE,		"ak47",		"weapon_ak47" },
	{ AUTOBUYCLASS_PRIMARY|AUTOBUYCLASS_SNIPERRIFLE,	"scout",	"weapon_scout" },
	{ AUTOBUYCLASS_PRIMARY|AUTOBUYCLASS_RIFLE,		"sg552",	"weapon_sg552" },
	{ AUTOBUYCLASS_PRIMARY|AUTOBUYCLASS_SNIPERRIFLE,	"awp",		"weapon_awp" },
	{ AUTOBUYCLASS_PRIMARY|AUTOBUYCLASS_SNIPERRIFLE,	"g3sg1",	"weapon_g3sg1" },
	{ AUTOBUYCLASS_PRIMARY|AUTOBUYCLASS_RIFLE,		"famas",	"weapon_famas" },
	{ AUTOBUYCLASS_PRIMARY|AUTOBUYCLASS_RIFLE,		"m4a1",		"weapon_m4a1" },
	{ AUTOBUYCLASS_PRIMARY|AUTOBUYCLASS_RIFLE,		"aug",		"weapon_aug" },
	{ AUTOBUYCLASS_PRIMARY|AUTOBUYCLASS_SNIPERRIFLE,	"sg550",	"weapon_sg550" },
	{ AUTOBUYCLASS_SECONDARY|AUTOBUYCLASS_PISTOL,		"glock",	"weapon_glock18" },
	{ AUTOBUYCLASS_SECONDARY|AUTOBUYCLASS_PISTOL,		"usp",		"weapon_usp" },
	{ AUTOBUYCLASS_SECONDARY|AUTOBUYCLASS_PISTOL,		"p228",		"weapon_p228" },
	{ AUTOBUYCLASS_SECONDARY|AUTOBUYCLASS_PISTOL,		"deagle",	"weapon_deagle" },
	{ AUTOBUYCLASS_SECONDARY|AUTOBUYCLASS_PISTOL,		"elites",	"weapon_elite" },
	{ AUTOBUYCLASS_SECONDARY|AUTOBUYCLASS_PISTOL,		"fn57",		"weapon_fiveseven" },
	{ AUTOBUYCLASS_PRIMARY|AUTOBUYCLASS_SHOTGUN,		"m3",		"weapon_m3" },
	{ AUTOBUYCLASS_PRIMARY|AUTOBUYCLASS_SHOTGUN,		"xm1014",	"weapon_xm1014" },
	{ AUTOBUYCLASS_PRIMARY|AUTOBUYCLASS_SMG,		"mac10",	"weapon_mac10" },
	{ AUTOBUYCLASS_PRIMARY|AUTOBUYCLASS_SMG,		"tmp",		"weapon_tmp" },
	{ AUTOBUYCLASS_PRIMARY|AUTOBUYCLASS_SMG,		"mp5",		"weapon_mp5navy" },
	{ AUTOBUYCLASS_PRIMARY|AUTOBUYCLASS_SMG,		"ump45",	"weapon_ump45" },
	{ AUTOBUYCLASS_PRIMARY|AUTOBUYCLASS_SMG,		"p90",		"weapon_p90" },
	{ AUTOBUYCLASS_PRIMARY|AUTOBUYCLASS_MACHINEGUN,		"m249",		"weapon_m249" },
	{ AUTOBUYCLASS_PRIMARY|AUTOBUYCLASS_AMMO,		"primammo",	"primammo" },
	{ AUTOBUYCLASS_SECONDARY|AUTOBUYCLASS_AMMO,		"secammo",	"secammo" },
	{ AUTOBUYCLASS_ARMOR,					"vest",		"item_kevlar" },
	{ AUTOBUYCLASS_ARMOR,					"vesthelm",	"item_assaultsuit" },
	{ AUTOBUYCLASS_GRENADE,					"flash",	"weapon_flashbang" },
	{ AUTOBUYCLASS_GRENADE,					"hegren",	"weapon_hegrenade" },
	{ AUTOBUYCLASS_GRENADE,					"sgren",	"weapon_smokegrenade"},
	{ AUTOBUYCLASS_NIGHTVISION,				"nvgs",		"nvgs" },
	{ AUTOBUYCLASS_DEFUSER,					"defuser",	"defuser" },
	{ AUTOBUYCLASS_PRIMARY|AUTOBUYCLASS_SHIELD,		"shield",	"shield" },
	{ AUTOBUYCLASS_NONE, NULL, NULL }
};

DLL_GLOBAL WeaponAliasInfo weaponAliasInfo[] =
{
	{ "p228",	WEAPON_P228 },
	{ "???",	WEAPON_GLOCK },
	{ "scout",	WEAPON_SCOUT },
	{ "hegren",	WEAPON_HEGRENADE },
	{ "xm1014",	WEAPON_XM1014 },
	{ "c4",		WEAPON_C4 },
	{ "mac10",	WEAPON_MAC10 },
	{ "aug",	WEAPON_AUG },
	{ "sgren",	WEAPON_SMOKEGRENADE },
	{ "elites",	WEAPON_ELITE },
	{ "fn57",	WEAPON_FIVESEVEN },
	{ "ump45",	WEAPON_UMP45 },
	{ "sg550",	WEAPON_SG550 },
	{ "galil",	WEAPON_GALIL },
	{ "famas",	WEAPON_FAMAS },
	{ "usp",	WEAPON_USP },
	{ "glock",	WEAPON_GLOCK18 },
	{ "awp",	WEAPON_AWP },
	{ "mp5",	WEAPON_MP5N },
	{ "m249",	WEAPON_M249 },
	{ "m3",		WEAPON_M3 },
	{ "m4a1",	WEAPON_M4A1 },
	{ "tmp",	WEAPON_TMP },
	{ "g3sg1",	WEAPON_G3SG1 },
	{ "flash",	WEAPON_FLASHBANG },
	{ "deagle",	WEAPON_DEAGLE },
	{ "sg552",	WEAPON_SG552 },
	{ "ak47",	WEAPON_AK47 },
	{ "knife",	WEAPON_KNIFE },
	{ "p90",	WEAPON_P90 },
	{ "shield",	WEAPON_SHIELDGUN },
	{ "none",	WEAPON_NONE },
	{ "grenade",	WEAPON_HEGRENADE },
	{ "hegrenade",	WEAPON_HEGRENADE },
	{ "glock18",	WEAPON_GLOCK18 },
	{ "elite",	WEAPON_ELITE },
	{ "fiveseven",	WEAPON_FIVESEVEN },
	{ "mp5navy",	WEAPON_MP5N },
	
	{ "anaconda",	WEAPON_ANACONDA },
	{ "deagled",	WEAPON_DEAGLED },
	{ "desperado",	WEAPON_DESPERADO },
	{ "csgo_cz75",	WEAPON_CSGO_CZ75 },
	{ "z4b_deagleb",	WEAPON_Z4B_DEAGLESHURA },
	{ "z4b_frequency1",	WEAPON_Z4B_FREQUENCY1 },
	{ "gunkata",	WEAPON_GUNKATA },
	{ "infinity",	WEAPON_INFINITY },
	{ "infinityex1",	WEAPON_INFINITYEX1 },
	{ "infinityex2",	WEAPON_INFINITYEX2 },
	{ "infinitysb",	WEAPON_INFINITYSB },
	{ "infinitysr",	WEAPON_INFINITYSR },
	{ "infinityss",	WEAPON_INFINITYSS },
	{ "z4b_infinityx",	WEAPON_Z4B_INFINITYX },
	{ "csgo_r8",	WEAPON_CSGO_R8 },
	{ "csgo_tec9",	WEAPON_CSGO_TEC9 },
	{ "voidpistol",	WEAPON_VOIDPISTOL },
	{ "y21s1jetgunmd",	WEAPON_Y21S1JETGUNMD },
	{ "m950",	WEAPON_M950 },
	{ "m950se",	WEAPON_M950SE },
	{ "sfpistol",	WEAPON_SFPISTOL },
	{ "y22s2sfpistol",	WEAPON_Y22S2SFPISTOL },
	{ "ragingbull",	WEAPON_Z4B_RAGINGBULL },
	{ "balrog1",	WEAPON_BALROG1},
	{ "sapientia",	WEAPON_SAPIENTIA },
	{ "bloodhunter", WEAPON_BLOODHUNTER },
	{ "thunderpistol", WEAPON_THUNDERPISTOL },
	{ "kronos1", WEAPON_KRONOS1 },
	{ "kingcobra", WEAPON_KINGCOBRA },
	{ "kingcobrag", WEAPON_KINGCOBRAG },
	{ "skull2", WEAPON_SKULL2 },
	{ "musket", WEAPON_MUSKET },
	{ "bufffiveseven", WEAPON_BUFFFIVESEVEN },
	{ "restrictionpistol", WEAPON_RESTRICTIONPISTOL },
	{ "voidpistolex",	WEAPON_VOIDPISTOLEX },
	{ "waterpistol",	WEAPON_WATERPISTOL },
	{ "monkeywpnset2",	WEAPON_MONKEYWPNSET2 },
	{ "tknife",	WEAPON_TKNIFE },
	{ "tknifeex",	WEAPON_TKNIFEEX },
	{ "tknifeex2",	WEAPON_TKNIFEEX2 },
	{ "tknifedx",	WEAPON_Z4B_TKNIFEDX },
	{ "malorian3516" , WEAPON_Z4B_MALORIAN3516},

	{ "balrog11",	WEAPON_BALROG11 },
	{ "balrog11b",	WEAPON_BALROG11B },
	{ "balrog11wc",	WEAPON_BALROG11WC },
	{ "gatling",	WEAPON_GATLING },
	{ "gatlingex",	WEAPON_GATLINGEX },
	{ "m1887",	WEAPON_M1887 },
	{ "csgo_mag7",	WEAPON_CSGO_MAG7 },
	{ "csgo_sawedoff",	WEAPON_CSGO_SAWEDOFF },
	{ "qbarrel",	WEAPON_QBARREL },
	{ "tbarrel",	WEAPON_TBARREL },
	{ "csg12ss",	WEAPON_Z4B_CSG12SS },
	{ "m3dragon",	WEAPON_M3DRAGON },
	{ "m3dragonm",	WEAPON_M3DRAGONM },
	{ "kronos12",	WEAPON_KRONOS12 },
	{ "railcannon",	WEAPON_RAILCANNON },
	{ "skull11",	WEAPON_SKULL11 },
	{ "janus11",	WEAPON_JANUS11 },
	{ "crow3",	WEAPON_CROW3 },
	{ "m1887g",	WEAPON_M1887G },
	{ "thanatos11",	WEAPON_THANATOS11 },
	{ "m1887xmas",	WEAPON_M1887XMAS },
	{ "rainbowgun",	WEAPON_RAINBOWGUN },
	{ "spsg",	WEAPON_SPSG },
	{ "blockas",	WEAPON_BLOCKAS },
	{ "bouncer",	WEAPON_BOUNCER },
	{ "spas12",	WEAPON_SPAS12 },
	{ "spas12ex",	WEAPON_SPAS12EX },
	{ "spas12ex2",	WEAPON_SPAS12EX2 },
	{ "mk3a1",	WEAPON_MK3A1 },
	{ "mk3a1se",	WEAPON_MK3A1SE },
	{ "vulcanus11",	WEAPON_VULCANUS11 },


	{ "csgo_bizon",	WEAPON_CSGO_BIZON },
	{ "k1a",	WEAPON_K1A },
	{ "z4b_k1ases",	WEAPON_Z4B_K1ASES },
	{ "kriss",	WEAPON_KRISS },
	{ "mp7a1d",	WEAPON_MP7A1D },
	{ "mp7a1c",	WEAPON_MP7A1_CARBINE },
	{ "mp7a1p",	WEAPON_MP7A1_PISTOL },
	{ "sfsmg",	WEAPON_SFSMG },
	{ "thompson",	WEAPON_THOMPSON },
	{ "z4b_dmp7a1x",	WEAPON_Z4B_MP7A1DX },
	{ "bison",	WEAPON_BISON },
	{ "tmpdragon",	WEAPON_TMPDRAGON },
	{ "spsmg",	WEAPON_SPSMG },
	{ "janus3",	WEAPON_JANUS3 },
	{ "crow3",	WEAPON_CROW3 },
	{ "balrog3",	WEAPON_BALROG3 },
	{ "kronos3",	WEAPON_KRONOS3 },
	{ "laserfist",	WEAPON_LASERFIST },
	{ "laserfistex",	WEAPON_LASERFISTEX },
	{ "coilgun",	WEAPON_COILGUN },
	{ "dualkrisshero",	WEAPON_DUALKRISSHERO },
	{ "dualuzi",	WEAPON_DUALUZI },
	{ "dualkriss",	WEAPON_DUALKRISS },
	{ "steyrtmpd",	WEAPON_Z4B_STEYRTMPD },
	{ "vulcanus3",	WEAPON_VULCANUS3 },
	{ "skull3",	WEAPON_SKULL3 },
	{ "skull3d",	WEAPON_SKULL3D },

	{ "ak47dragon",	WEAPON_AK47DRAGON },
	{ "ak47l",	WEAPON_AK47_LONG },
	{ "buffsg552ex",	WEAPON_BUFFSG552EX },
	{ "m14ebr",	WEAPON_M14EBR },
	{ "m4a1dragon",	WEAPON_M4A1DRAGON },
	{ "z4b_m4a1razer",	WEAPON_Z4B_M4A1RAZER },
	{ "scarh",	WEAPON_SCARHEAVY },
	{ "scarl",	WEAPON_SCARLIGHT },
	{ "sfgun",	WEAPON_SFGUN },
	{ "starchaserar",	WEAPON_STARCHASERAR },
	{ "stg44",	WEAPON_STG44 },
	{ "svdex",	WEAPON_SVDEX },
	{ "tar21",	WEAPON_TAR21 },
	{ "xm8c",	WEAPON_XM8CARBINE },
	{ "xm8s",	WEAPON_XM8SHARPSHOOTER },
	{ "z4b_ak47x",	WEAPON_Z4B_AK47X },
	{ "z4b_m4a1mw",	WEAPON_Z4B_M4A1MW },
	{ "z4b_m4a1x",	WEAPON_Z4B_M4A1X },
	{ "buffm4",	WEAPON_BUFFM4 },
	{ "buffak",	WEAPON_BUFFAK },
	{ "plasmagun",	WEAPON_PLASMAGUN },
	{ "aquariusplasmagun",	WEAPON_Z4B_AQUARIUSPLASMAGUN },
	{ "y20s2scard",	WEAPON_Y20S2SCARD },
	{ "lightzg",	WEAPON_LIGHTZG },
	{ "heavyzg",	WEAPON_HEAVYZG },
	{ "ak47g",	WEAPON_AK47G },
	{ "skull4",	WEAPON_SKULL4 },
	{ "ethereal",	WEAPON_ETHEREAL },
	{ "crow5",	WEAPON_CROW5 },
	{ "stunrifle",	WEAPON_STUNRIFLE },
	{ "crossbowex21",	WEAPON_CROSSBOWEX21 },
	{ "janusmk5",	WEAPON_JANUSMK5 },
	{ "crossbow",	WEAPON_CROSSBOW },
	{ "charger5",	WEAPON_CHARGER5 },
	{ "lockongun",	WEAPON_LOCKONGUN },
	{ "buffaug",	WEAPON_BUFFAUG },
	{ "buffsg552",	WEAPON_BUFFSG552 },
	{ "balrog5",	WEAPON_BALROG5 },
	{ "kronos5",	WEAPON_KRONOS5 },
	{ "an94",	WEAPON_AN94 },
	{ "m16a4",	WEAPON_M16A4 },
	{ "blockar",	WEAPON_BLOCKAR },
	{ "thanatos5",	WEAPON_THANATOS5 },
	{ "oicw",	WEAPON_OICW },
	{ "m1garand",	WEAPON_M1GARAND },
	{ "gilboaex",	WEAPON_GILBOAEX },
	{ "guitar",	WEAPON_GUITAR },
	{ "violingun",	WEAPON_VIOLINGUN },
	{ "cartbluec",	WEAPON_CARTBLUEC },
	{ "cartblues",	WEAPON_CARTBLUES },
	{ "cartredl",	WEAPON_CARTREDL },
	{ "cartredh",	WEAPON_CARTREDH },

	{ "as50",	WEAPON_AS50 },
	{ "as50g",	WEAPON_AS50G },
	{ "bendita",	WEAPON_BENDITA },
	{ "skull5",	WEAPON_SKULL5 },
	{ "sfsniper",	WEAPON_SFSNIPER },
	{ "z4b_awpnividia",	WEAPON_Z4B_AWPNVIDIA },
	{ "m95",	WEAPON_M95 },
	{ "m95xmas",	WEAPON_M95XMAS },
	{ "wa2000",	WEAPON_WA2000 },
	{ "z4b_xm2010pc",	WEAPON_Z4B_XM2010PC },
	{ "z4b_m1887se",	WEAPON_Z4B_M1887SE },
	{ "m95tiger",	WEAPON_M95TIGER },
	{ "cheytaclrrs",	WEAPON_CHEYTACLRRS },
	{ "m400",	WEAPON_M400 },
	{ "m82",	WEAPON_M82 },
	{ "sl8",	WEAPON_SL8 },
	{ "sl8g",	WEAPON_SL8G },
	{ "sl8ex",	WEAPON_SL8EX },
	{ "trg42",	WEAPON_TRG42 },
	{ "trg42g",	WEAPON_TRG42G },
	{ "m24",	WEAPON_M24 },
	{ "aw50",	WEAPON_AW50 },
	{ "sprifile",	WEAPON_SPRIFLE },
	{ "bpgm",	WEAPON_BPGM },
	{ "zgun",	WEAPON_ZGUN },
	{ "barrettd",	WEAPON_Z4B_BARRETTD },
	{ "freedom",	WEAPON_Z4B_FREEDOM },
	{ "buffawp",	WEAPON_BUFFAWP },
	{ "destroyer",	WEAPON_DESTROYER },
	{ "starchasersr",	WEAPON_STARCHASERSR },
	{ "mosin",	WEAPON_MOSIN },
	{ "deathray",	WEAPON_Z4B_DEATHRAY },

	{ "balrog7",	WEAPON_BALROG7 },
	{ "buffm249",	WEAPON_BUFFM249 },
	{ "janus7xmas",	WEAPON_JANUS7XMAS },
	{ "m134",	WEAPON_M134 },
	{ "m134ex",	WEAPON_M134EX },
	{ "z4b_m134heroi",	WEAPON_Z4B_M134HEROI },
	{ "m134xmas",	WEAPON_M134XMAS },
	{ "m2",	WEAPON_M2 },
	{ "m249ex",	WEAPON_M249EX },
	{ "mg3",	WEAPON_MG3 },
	{ "mg36",	WEAPON_MG36 },
	{ "mg3xmas",	WEAPON_MG3XMAS },
	{ "csgo_negev",	WEAPON_CSGO_NEGEV },
	{ "sfmg",	WEAPON_SFMG },
	{ "hk23",	WEAPON_HK23 },
	{ "m60",	WEAPON_M60 },
	{ "mk48",	WEAPON_MK48 },
	{ "skull8",	WEAPON_SKULL8 },
	{ "skull6",	WEAPON_SKULL6 },
	{ "m249ep",	WEAPON_M249EP },
	{ "thanatos7",	WEAPON_THANATOS7 },
	{ "crow7",	WEAPON_CROW7 },
	{ "k3",	WEAPON_K3 },
	{ "charger7",	WEAPON_CHARGER7 },
	{ "z4b_m60amethyst",	WEAPON_Z4B_M60AMETHYST },
	{ "z4b_dbarrelamethyst",	WEAPON_Z4B_DBARRELAMETHYST },
	{ "z4b_deagleamethyst",	WEAPON_Z4B_DEAGLEAMETHYST },
	{ "z4b_acramethyst",	WEAPON_Z4B_ACRAMETHYST },
	{ "z4b_ar57amethyst",	WEAPON_Z4B_AR57AMETHYST },
	{ "broad",	WEAPON_BROAD },
	{ "coilmg",	WEAPON_COILMG },
	{ "spmg",	WEAPON_SPMG },
	{ "cameragun",	WEAPON_CAMERAGUN },
	{ "mgsm",	WEAPON_MGSM },
	{ "kronos7",	WEAPON_KRONOS7 },
	{ "blockmg",	WEAPON_BLOCKMG },
	{ "buffng7",	WEAPON_BUFFNG7 },
	{ "janus7",	WEAPON_JANUS7 },
	{ "y22s3janus7",	WEAPON_Y22S3JANUS7 },
	{ "laserminigun",	WEAPON_LASERMINIGUN },
	{ "vulcanus7",	WEAPON_VULCANUS7 },

	{ "crossbow",	WEAPON_CROSSBOW },
	{ "chainsaw",	WEAPON_CHAINSAW },
	{ "bow",	WEAPON_BOW },
	{ "cannon",	WEAPON_CANNON },
	{ "z4b_cannonplasma",	WEAPON_Z4B_CANNONPLASMA },
	{ "z4b_cannontiger",	WEAPON_Z4B_CANNONTIGER },
	{ "gungnir",	WEAPON_GUNGNIR },
	{ "huntbow",	WEAPON_HUNTBOW },
	{ "sgdrill",	WEAPON_SGDRILL },
	{ "revivegun",	WEAPON_REVIVEGUN },
	{ "csgo_zeus",	WEAPON_CSGO_ZEUS },
	{ "zombielaw",	WEAPON_ZOMBIELAW },
	{ "m32venom",	WEAPON_M32VENOM },
	{ "cannonex",	WEAPON_CANNONEX },
	{ "speargun",	WEAPON_SPEARGUN },
	{ "sgmissile",	WEAPON_SGMISSILE },
	{ "sgmissilem",	WEAPON_SGMISSILEM },
	{ "sgmissileex",	WEAPON_SGMISSILEEX },
	{ "syringe",	WEAPON_Z4B_SYRINGE },
	{ "chainsawm",	WEAPON_CHAINSAWM },
	{ "herochainsaw",	WEAPON_HEROCHAINSAW },
	{ "winggun",	WEAPON_WINGGUN },
	{ "railgun",	WEAPON_RAILGUN },
	{ "chainsawex",	WEAPON_CHAINSAWEX },
	{ "lasersg",	WEAPON_LASERSG },
	{ "wondercannon",	WEAPON_WONDERCANNON },
	{ "pianogun",	WEAPON_PIANOGUN },
	{ "pianogunex",	WEAPON_PIANOGUNEX },
	{ "sgdrillex",	WEAPON_SGDRILLEX },
	{ "airburster",	WEAPON_AIRBURSTER },
	{ "flamethrower",	WEAPON_FLAMETHROWER },
	{ "poisongun",	WEAPON_POISONGUN },
	{ "watercannon",	WEAPON_WATERCANNON },
	{ "fglauncher",	WEAPON_FGLAUNCHER },
	{ "guillotine",	WEAPON_GUILLOTINE },
	{ "guillotineex",	WEAPON_GUILLOTINEEX },
	{ "speargunm",	WEAPON_SPEARGUNM },
	{ "wondercannonex",	WEAPON_WONDERCANNONEX },
	{ "drillgun",	WEAPON_DRILLGUN },

	{ NULL,		WEAPON_NONE }
};

// must same as classname
DLL_GLOBAL KnifeAliasInfo knifeInfo[] =
{
	{ "weapon_knife", KNIFE_NORMAL },
	{ "knife_dgaxe",	KNIFE_DGAXE },
	{ "knife_knifedragon",	KNIFE_DRAGON },
	{ "knife_dragonsword",	KNIFE_DRAGONSWORD },
	{ "knife_dualsword",	KNIFE_DUALSWORD },
	{ "z4b_facelessvoid",	KNIFE_Z4B_FACELESSVOID },
	{ "z4b_frequency9",	KNIFE_Z4B_FREQUENCY9 },
	{ "knife_katana",	KNIFE_KATANA },
	{ "knife_lance",	KNIFE_LANCE },
	{ "z4b_lc_sword",	KNIFE_Z4B_LCSWORD },
	{ "z4b_nataknifedx",	KNIFE_Z4B_NATAKNIFEDX },
	{ "z4b_oppovivo",	KNIFE_Z4B_OPPOVIVO },
	{ "knife_skullaxe",	KNIFE_SKULLAXE },
	{ "z4b_stormgiantx",	KNIFE_Z4B_STORMGIANTX },
	{ "z4b_strongknifex",	KNIFE_Z4B_STRONGKNIFEX },
	{ "knife_stormgiant",	KNIFE_STORMGIANT },
	{ "knife_nataknife",	KNIFE_STRONGKNIFE },
	{ "knife_nataknifed",	KNIFE_NATAKNIFED },
	{ "knife_thanatos9",	KNIFE_THANATOS9 },
	{ "knife_skullt9",	KNIFE_SKULLT9 },
	{ "knife_runeblade",	KNIFE_RUNEBLADE },
	{ "knife_janus9",	KNIFE_JANUS9 },
	{ "knife_summonknife",	KNIFE_SUMMONKNIFE },
	{ "knife_balrog9",	KNIFE_BALROG9 },
	{ "knife_holysword",	KNIFE_HOLYSWORD },
	{ "knife_y22s1holyswordmb",	KNIFE_Y22S1HOLYSWORDMB },
	{ "knife_magicknife",	KNIFE_MAGICKNIFE },
	{ "knife_whipsword",	KNIFE_WHIPSWORD },
	{ "z4b_dualkukri",	KNIFE_DUALKUKRI },
	{ "z4b_dualslayer",	KNIFE_DUALSLAYER },
	{ "knife_jknife",	KNIFE_JKNIFE },
	{ "knife_combat",	KNIFE_COMBAT },
	{ "knife_mastercombat",	KNIFE_MASTERCOMBAT },
	{ "knife_hdagger",	KNIFE_HDAGGER },
	{ "knife_axe",	KNIFE_AXE },
	{ "knife_hammer",	KNIFE_HAMMER },
	{ "z4b_holyfist",	KNIFE_HOLYFIST },
	{ "z4b_dualstinger",	KNIFE_DUALSTINGER },
	{ "z4b_heartstimulater",	KNIFE_HEARTSTIMULATER },
	{ "knife_y22s2sfsword",	KNIFE_Y22S2SFSWORD },
	{ "knife_armtorch",	KNIFE_ARMTORCH },
	{ "z4b_tiga",	KNIFE_TIGA },
	{ "knife_y22s3dragonsword",	KNIFE_Y22S3DRAGONSWORD },
	{ "knife_monkeywpnset3",	KNIFE_MONKEYWPNSET3 },
	{ "knife_ozwpnset3",	KNIFE_OZWPNSET3 },
	{ "knife_tomahawk",	KNIFE_TOMAHAWK },
	{ "knife_crowbarcraft",	KNIFE_CROWBARCRAFT },
	{ "knife_dragontail",	KNIFE_DRAGONTAIL },
	{ "knife_katanad",	KNIFE_KATANAD },
	{ "knife_vulcanus9",	KNIFE_VULCANUS9 },
	{ "knife_sfsword",	KNIFE_SFSWORD },
	{ "knife_spknife",	KNIFE_SPKNIFE },
	{ "knife_crow9",	KNIFE_CROW9 },
	{ "knife_turbulent9",	KNIFE_TURBULENT9 },
	{ "knife_ironfan",	KNIFE_IRONFAN },
	{ "knife_swordbombard",	KNIFE_SWORDBOMBARD },


	
};

// must same as classname
DLL_GLOBAL GrenadeAliasInfo grenadeInfo[] =
{
	{ "weapon_hegrenade", GRENADE_HEGRENADE },
	{ "weapon_pumpkin", GRENADE_PUMPKIN },
	{ "weapon_holybomb", GRENADE_HOLYBOMB },
	{ "weapon_bunkerbuster", GRENADE_BUNKERBUSTER },
	{ "weapon_patroldrone", GRENADE_PATROLDRONE },
	{ "weapon_divinetitan", GRENADE_DIVINETITAN },
	{ "weapon_m24grenade", GRENADE_M24GRENADE },
	{ "weapon_fgrenade2", GRENADE_FGRENADE2 },
	{ "weapon_fgrenade", GRENADE_FIREBOMB },
	{ "weapon_heartbomb", GRENADE_HEARTBOMB },
	{ "weapon_cake", GRENADE_CAKEBOMB },
	{ "weapon_mooncake", GRENADE_MOONCAKE },
	{ "weapon_cartfrag", GRENADE_CARTFRAG },
	{ "weapon_sfgrenade", GRENADE_SFGRENADE },
	{ "weapon_chaingren", GRENADE_CHAINGRENADE },
};

DLL_GLOBAL WeaponBuyAliasInfo weaponBuyAliasInfo[] =
{
	{ "galil",		WEAPON_GALIL,		"#Galil" },
	{ "defender",		WEAPON_GALIL,		"#Galil" },
	{ "ak47",		WEAPON_AK47,		"#AK47" },
	{ "cv47",		WEAPON_AK47,		"#AK47" },
	{ "scout",		WEAPON_SCOUT,		NULL },
	{ "sg552",		WEAPON_SG552,		"#SG552" },
	{ "krieg552",		WEAPON_SG552,		"#SG552" },
	{ "awp",		WEAPON_AWP,		NULL },
	{ "magnum",		WEAPON_AWP,		NULL },
	{ "g3sg1",		WEAPON_G3SG1,		"#G3SG1" },
	{ "d3au1",		WEAPON_G3SG1,		"#G3SG1" },
	{ "famas",		WEAPON_FAMAS,		"#Famas" },
	{ "clarion",		WEAPON_FAMAS,		"#Famas" },
	{ "m4a1",		WEAPON_M4A1,		"#M4A1" },
	{ "aug",		WEAPON_AUG,		"#Aug" },
	{ "bullpup",		WEAPON_AUG,		"#Aug" },
	{ "sg550",		WEAPON_SG550,		"#SG550" },
	{ "krieg550",		WEAPON_SG550,		"#SG550" },
	{ "glock",		WEAPON_GLOCK18,		NULL },
	{ "9x19mm",		WEAPON_GLOCK18,		NULL },
	{ "usp",		WEAPON_USP,		NULL },
	{ "km45",		WEAPON_USP,		NULL },
	{ "p228",		WEAPON_P228,		NULL },
	{ "228compact",		WEAPON_P228,		NULL },
	{ "deagle",		WEAPON_DEAGLE,		NULL },
	{ "nighthaw",		WEAPON_DEAGLE,		NULL },
	{ "elites",		WEAPON_ELITE,		"#Beretta96G" },
	{ "fn57",		WEAPON_FIVESEVEN,	"#FiveSeven" },
	{ "fiveseven",		WEAPON_FIVESEVEN,	"#FiveSeven" },
	{ "m3",			WEAPON_M3,		NULL },
	{ "12gauge",		WEAPON_M3,		NULL },
	{ "xm1014",		WEAPON_XM1014,		NULL },
	{ "autoshotgun",	WEAPON_XM1014,		NULL },
	{ "mac10",		WEAPON_MAC10,		"#Mac10" },
	{ "tmp",		WEAPON_TMP,		"#tmp" },
	{ "mp",			WEAPON_TMP,		"#tmp" },
	{ "mp5",		WEAPON_MP5N,		NULL },
	{ "smg",		WEAPON_MP5N,		NULL },
	{ "ump45",		WEAPON_UMP45,		NULL },
	{ "p90",		WEAPON_P90,		NULL },
	{ "c90",		WEAPON_P90,		NULL },
	{ "m249",		WEAPON_M249,		NULL },
	{ NULL,			WEAPON_NONE,		NULL }
};

DLL_GLOBAL WeaponClassAliasInfo weaponClassAliasInfo[] =
{
	{ "p228",	WEAPONCLASS_PISTOL },
	{ "???",	WEAPONCLASS_PISTOL },
	{ "scout",	WEAPONCLASS_SNIPERRIFLE },
	{ "hegren",	WEAPONCLASS_GRENADE },
	{ "xm1014",	WEAPONCLASS_SHOTGUN },
	{ "c4",		WEAPONCLASS_GRENADE },
	{ "mac10",	WEAPONCLASS_SUBMACHINEGUN },
	{ "aug",	WEAPONCLASS_RIFLE },
	{ "sgren",	WEAPONCLASS_GRENADE },
	{ "elites",	WEAPONCLASS_PISTOL },
	{ "fn57",	WEAPONCLASS_PISTOL },
	{ "ump45",	WEAPONCLASS_SUBMACHINEGUN },
	{ "sg550",	WEAPONCLASS_SNIPERRIFLE },
	{ "galil",	WEAPONCLASS_RIFLE },
	{ "famas",	WEAPONCLASS_RIFLE },
	{ "usp",	WEAPONCLASS_PISTOL },
	{ "glock",	WEAPONCLASS_PISTOL },
	{ "awp",	WEAPONCLASS_SNIPERRIFLE },
	{ "mp5",	WEAPONCLASS_SUBMACHINEGUN },
	{ "m249",	WEAPONCLASS_MACHINEGUN },
	{ "m3",		WEAPONCLASS_SHOTGUN },
	{ "m4a1",	WEAPONCLASS_RIFLE },
	{ "tmp",	WEAPONCLASS_SUBMACHINEGUN },
	{ "g3sg1",	WEAPONCLASS_SNIPERRIFLE },
	{ "flash",	WEAPONCLASS_GRENADE },
	{ "deagle",	WEAPONCLASS_PISTOL },
	{ "sg552",	WEAPONCLASS_RIFLE },
	{ "ak47",	WEAPONCLASS_RIFLE },
	{ "knife",	WEAPONCLASS_KNIFE },
	{ "p90",	WEAPONCLASS_SUBMACHINEGUN },
	{ "shield",	WEAPONCLASS_PISTOL },
	{ "grenade",	WEAPONCLASS_GRENADE },
	{ "hegrenade",	WEAPONCLASS_GRENADE },
	{ "glock18",	WEAPONCLASS_PISTOL },
	{ "elite",	WEAPONCLASS_PISTOL },
	{ "fiveseven",	WEAPONCLASS_PISTOL },
	{ "mp5navy",	WEAPONCLASS_SUBMACHINEGUN },
	{ "grenade",	WEAPONCLASS_GRENADE },
	{ "pistol",	WEAPONCLASS_PISTOL },
	{ "SMG",	WEAPONCLASS_SUBMACHINEGUN },
	{ "machinegun",	WEAPONCLASS_MACHINEGUN },
	{ "shotgun",	WEAPONCLASS_SHOTGUN },
	{ "rifle",	WEAPONCLASS_RIFLE },
	{ "sniper",	WEAPONCLASS_SNIPERRIFLE },
	{ "none",	WEAPONCLASS_NONE },
	
	{ "anaconda",	WEAPONCLASS_PISTOL },
	{ "deagled",	WEAPONCLASS_PISTOL },
	{ "desperado",	WEAPONCLASS_PISTOL },
	{ "csgo_cz75",	WEAPONCLASS_PISTOL },
	{ "z4b_deagleb",	WEAPONCLASS_PISTOL },
	{ "z4b_frequency1",	WEAPONCLASS_PISTOL },
	{ "infinity",	WEAPONCLASS_PISTOL },
	{ "infinityex1",	WEAPONCLASS_PISTOL },
	{ "infinityex2",	WEAPONCLASS_PISTOL },
	{ "infinitysb",	WEAPONCLASS_PISTOL },
	{ "infinitysr",	WEAPONCLASS_PISTOL },
	{ "infinityss",	WEAPONCLASS_PISTOL },
	{ "z4b_infinityx",	WEAPONCLASS_PISTOL },
	{ "csgo_r8",	WEAPONCLASS_PISTOL },
	{ "csgo_tec9",	WEAPONCLASS_PISTOL },
	{ "voidpistol",	WEAPONCLASS_PISTOL },
	{ "y21s1jetgunmd",	WEAPONCLASS_PISTOL },
	{ "m950",	WEAPONCLASS_PISTOL },
	{ "csgo_r8",	WEAPONCLASS_PISTOL },
	{ "m950se",	WEAPONCLASS_PISTOL },
	{ "sfpistol",	WEAPONCLASS_PISTOL },
	{ "y22s2sfpistol",	WEAPONCLASS_PISTOL },
	{ "ragingbull",	WEAPONCLASS_PISTOL },
	{ "balrog1",	WEAPONCLASS_PISTOL},
	{ "sapientia",	WEAPONCLASS_PISTOL },
	{ "z4b_deagleamethyst",	WEAPONCLASS_PISTOL },
	{ "bloodhunter", WEAPONCLASS_PISTOL },
	{ "kronos1", WEAPONCLASS_PISTOL },
	{ "thunderpistol", WEAPONCLASS_PISTOL },
	{ "kingcobra", WEAPONCLASS_PISTOL },
	{ "kingcobrag", WEAPONCLASS_PISTOL },
	{ "skull2", WEAPONCLASS_PISTOL },
	{ "musket", WEAPONCLASS_PISTOL },
	{ "bufffiveseven", WEAPONCLASS_PISTOL },
	{ "voidpistolex",	WEAPONCLASS_PISTOL },
	{ "monkeywpnset2",	WEAPONCLASS_PISTOL },
	{ "tknife",	WEAPONCLASS_PISTOL },
	{ "tknifeex",	WEAPONCLASS_PISTOL },
	{ "tknifeex2",	WEAPONCLASS_PISTOL },
	{ "tknifedx",	WEAPONCLASS_PISTOL },
	{ "malorian3516" , WEAPONCLASS_PISTOL},

	{ "balrog11",	WEAPONCLASS_SHOTGUN },
	{ "balrog11b",	WEAPONCLASS_SHOTGUN },
	{ "balrog11wc",	WEAPONCLASS_SHOTGUN },
	{ "gatling",	WEAPONCLASS_SHOTGUN },
	{ "gatlingex",	WEAPONCLASS_SHOTGUN },
	{ "m1887",	WEAPONCLASS_SHOTGUN },
	{ "csgo_mag7",	WEAPONCLASS_SHOTGUN },
	{ "csgo_sawedoff",	WEAPONCLASS_SHOTGUN },
	{ "qbarrel",	WEAPONCLASS_SHOTGUN },
	{ "tbarrel",	WEAPONCLASS_SHOTGUN },
	{ "csg12ss",	WEAPONCLASS_SHOTGUN },
	{ "m3dragon",	WEAPONCLASS_SHOTGUN },
	{ "m3dragonm",	WEAPONCLASS_SHOTGUN },
	{ "lasersg",	WEAPONCLASS_SHOTGUN },
	{ "kronos12",	WEAPONCLASS_SHOTGUN },
	{ "railcannon",	WEAPONCLASS_SHOTGUN },
	{ "skull11",	WEAPONCLASS_SHOTGUN },
	{ "janus11",	WEAPONCLASS_SHOTGUN },
	{ "crow3",	WEAPONCLASS_SHOTGUN },
	{ "z4b_dbarrelamethyst",	WEAPONCLASS_SHOTGUN },
	{ "m1887g",	WEAPONCLASS_SHOTGUN },
	{ "thanatos11",	WEAPONCLASS_SHOTGUN },
	{ "m1887xmas",	WEAPONCLASS_SHOTGUN },
	{ "rainbowgun",	WEAPONCLASS_SHOTGUN },
	{ "spsg",	WEAPONCLASS_SHOTGUN },
	{ "blockas",	WEAPONCLASS_SHOTGUN },
	{ "bouncer",	WEAPONCLASS_SHOTGUN },
	{ "spas12",	WEAPONCLASS_SHOTGUN },
	{ "spas12ex",	WEAPONCLASS_SHOTGUN },
	{ "spas12ex2",	WEAPONCLASS_SHOTGUN },
	{ "mk3a1",	WEAPONCLASS_SHOTGUN },
	{ "mk3a1se",	WEAPONCLASS_SHOTGUN },
	{ "vulcanus11",	WEAPONCLASS_SHOTGUN },

	{ "csgo_bizon",	WEAPONCLASS_SUBMACHINEGUN },
	{ "k1a",	WEAPONCLASS_SUBMACHINEGUN },
	{ "z4b_k1ases",	WEAPONCLASS_SUBMACHINEGUN },
	{ "kriss",	WEAPONCLASS_SUBMACHINEGUN },
	{ "mp7a1d",	WEAPONCLASS_SUBMACHINEGUN },
	{ "mp7a1c",	WEAPONCLASS_SUBMACHINEGUN },
	{ "mp7a1p",	WEAPONCLASS_SUBMACHINEGUN },
	{ "sfsmg",	WEAPONCLASS_SUBMACHINEGUN },
	{ "thompson",	WEAPONCLASS_SUBMACHINEGUN },
	{ "z4b_dmp7a1x",	WEAPONCLASS_SUBMACHINEGUN },
	{ "bison",	WEAPONCLASS_SUBMACHINEGUN },
	{ "tmpdragon",	WEAPONCLASS_SUBMACHINEGUN },
	{ "spsmg",	WEAPONCLASS_SUBMACHINEGUN },
	{ "janus3",	WEAPONCLASS_SUBMACHINEGUN },
	{ "crow3",	WEAPONCLASS_SUBMACHINEGUN },
	{ "balrog3", WEAPONCLASS_SUBMACHINEGUN },
	{ "kronos3",	WEAPONCLASS_SUBMACHINEGUN },
	{ "laserfist",	WEAPONCLASS_SUBMACHINEGUN },
	{ "laserfistex",	WEAPONCLASS_SUBMACHINEGUN },
	{ "coilgun",	WEAPONCLASS_SUBMACHINEGUN },
	{ "dualkrisshero",	WEAPONCLASS_SUBMACHINEGUN },
	{ "dualuzi",	WEAPONCLASS_SUBMACHINEGUN },
	{ "dualkriss",	WEAPONCLASS_SUBMACHINEGUN },
	{ "steyrtmpd",	WEAPONCLASS_SUBMACHINEGUN },
	{ "vulcanus3",	WEAPONCLASS_SUBMACHINEGUN },
	{ "thanatos3",	WEAPONCLASS_SUBMACHINEGUN },
	{ "sterlingbayonet",	WEAPONCLASS_SUBMACHINEGUN },
	{ "skull3",	WEAPONCLASS_SUBMACHINEGUN },
	{ "skull3d",	WEAPONCLASS_SUBMACHINEGUN },

	{ "bow",	WEAPONCLASS_RIFLE },
	{ "ak47dragon",	WEAPONCLASS_RIFLE },
	{ "ak47l",	WEAPONCLASS_RIFLE },
	{ "buffsg552ex",	WEAPONCLASS_RIFLE },
	{ "m14ebr",	WEAPONCLASS_RIFLE },
	{ "m4a1dragon",	WEAPONCLASS_RIFLE },
	{ "z4b_m4a1razer",	WEAPONCLASS_RIFLE },
	{ "scarh",	WEAPONCLASS_RIFLE },
	{ "scarl",	WEAPONCLASS_RIFLE },
	{ "sfgun",	WEAPONCLASS_RIFLE },
	{ "starchaserar",	WEAPONCLASS_RIFLE },
	{ "stg44",	WEAPONCLASS_RIFLE },
	{ "svdex",	WEAPONCLASS_RIFLE },
	{ "tar21",	WEAPONCLASS_RIFLE },
	{ "xm8c",	WEAPONCLASS_RIFLE },
	{ "xm8s",	WEAPONCLASS_SNIPERRIFLE },
	{ "z4b_ak47x",	WEAPONCLASS_RIFLE },
	{ "z4b_m4a1mw",	WEAPONCLASS_RIFLE },
	{ "z4b_m4a1x",	WEAPONCLASS_RIFLE },
	{ "buffm4",	WEAPONCLASS_RIFLE },
	{ "buffak",	WEAPONCLASS_RIFLE },
	{ "plasmagun",	WEAPONCLASS_RIFLE },
	{ "aquariusplasmagun",	WEAPONCLASS_RIFLE },
	{ "y20s2scard",	WEAPONCLASS_RIFLE },
	{ "lightzg",	WEAPONCLASS_RIFLE },
	{ "heavyzg",	WEAPONCLASS_RIFLE },
	{ "ak47g",	WEAPONCLASS_RIFLE },
	{ "skull4",	WEAPONCLASS_RIFLE },
	{ "ethereal",	WEAPONCLASS_RIFLE },
	{ "railgun",	WEAPONCLASS_RIFLE },
	{ "crow5",	WEAPONCLASS_RIFLE },
	{ "stunrifle",	WEAPONCLASS_RIFLE },
	{ "crossbowex21",	WEAPONCLASS_RIFLE },
	{ "janusmk5",	WEAPONCLASS_RIFLE },
	{ "crossbow",	WEAPONCLASS_RIFLE },
	{ "charger5",	WEAPONCLASS_RIFLE },
	{ "z4b_acramethyst",	WEAPONCLASS_RIFLE },
	{ "z4b_ar57amethyst",	WEAPONCLASS_RIFLE },
	{ "lockongun",	WEAPONCLASS_RIFLE },
	{ "buffaug",	WEAPONCLASS_RIFLE },
	{ "buffsg552",	WEAPONCLASS_RIFLE },
	{ "balrog5",	WEAPONCLASS_RIFLE },
	{ "kronos5",	WEAPONCLASS_RIFLE },
	{ "an94",	WEAPONCLASS_RIFLE },
	{ "m16a4",	WEAPONCLASS_RIFLE },
	{ "blockar",	WEAPONCLASS_RIFLE },
	{ "thanatos5",	WEAPONCLASS_RIFLE },
	{ "oicw",	WEAPONCLASS_RIFLE },
	{ "m1garand",	WEAPONCLASS_RIFLE },
	{ "gilboaex",	WEAPONCLASS_RIFLE },
	{ "guitar",	WEAPONCLASS_RIFLE },
	{ "violingun",	WEAPONCLASS_RIFLE },
	{ "cartbluec",	WEAPONCLASS_RIFLE },
	{ "cartredl",	WEAPONCLASS_RIFLE },
	{ "cartredh",	WEAPONCLASS_RIFLE },

	{ "as50",	WEAPONCLASS_SNIPERRIFLE },
	{ "as50g",	WEAPONCLASS_SNIPERRIFLE },
	{ "bendita",	WEAPONCLASS_SNIPERRIFLE },
	{ "skull5",	WEAPONCLASS_SNIPERRIFLE },
	{ "sfsniper",	WEAPONCLASS_SNIPERRIFLE },
	{ "z4b_awpnividia",	WEAPONCLASS_SNIPERRIFLE },
	{ "m95",	WEAPONCLASS_SNIPERRIFLE },
	{ "m95xmas",	WEAPONCLASS_SNIPERRIFLE },
	{ "wa2000",	WEAPONCLASS_SNIPERRIFLE },
	{ "z4b_xm2010pc",	WEAPONCLASS_SNIPERRIFLE },
	{ "z4b_m1887se",	WEAPONCLASS_SNIPERRIFLE },
	{ "m95tiger",	WEAPONCLASS_SNIPERRIFLE },
	{ "cheytaclrrs",	WEAPONCLASS_SNIPERRIFLE },
	{ "m400",	WEAPONCLASS_SNIPERRIFLE },
	{ "m82",	WEAPONCLASS_SNIPERRIFLE },
	{ "sl8",	WEAPONCLASS_SNIPERRIFLE },
	{ "sl8g",	WEAPONCLASS_SNIPERRIFLE },
	{ "sl8ex",	WEAPONCLASS_SNIPERRIFLE },
	{ "trg42",	WEAPONCLASS_SNIPERRIFLE },
	{ "trg42g",	WEAPONCLASS_SNIPERRIFLE },
	{ "m24",	WEAPONCLASS_SNIPERRIFLE },
	{ "aw50",	WEAPONCLASS_SNIPERRIFLE },
	{ "sprifile",	WEAPONCLASS_SNIPERRIFLE },
	{ "bpgm",	WEAPONCLASS_SNIPERRIFLE },
	{ "zgun",	WEAPONCLASS_SNIPERRIFLE },
	{ "barrettd",	WEAPONCLASS_SNIPERRIFLE },
	{ "freedom",	WEAPONCLASS_SNIPERRIFLE },
	{ "buffawp",	WEAPONCLASS_SNIPERRIFLE },
	{ "destroyer",	WEAPONCLASS_SNIPERRIFLE },
	{ "starchasersr",	WEAPONCLASS_SNIPERRIFLE },
	{ "mosin",	WEAPONCLASS_SNIPERRIFLE },
	{ "deathray",	WEAPONCLASS_SNIPERRIFLE },
	{ "cartblues",	WEAPONCLASS_SNIPERRIFLE },


	{ "balrog7",	WEAPONCLASS_MACHINEGUN },
	{ "buffm249",	WEAPONCLASS_MACHINEGUN },
	{ "janus7xmas",	WEAPONCLASS_MACHINEGUN },
	{ "m134",	WEAPONCLASS_MACHINEGUN },
	{ "m134ex",	WEAPONCLASS_MACHINEGUN },
	{ "z4b_m134heroi",	WEAPONCLASS_MACHINEGUN },
	{ "m134xmas",	WEAPONCLASS_MACHINEGUN },
	{ "m2",	WEAPONCLASS_MACHINEGUN },
	{ "m249ex",	WEAPONCLASS_MACHINEGUN },
	{ "mg3",	WEAPONCLASS_MACHINEGUN },
	{ "mg36",	WEAPONCLASS_MACHINEGUN },
	{ "mg3xmas",	WEAPONCLASS_MACHINEGUN },
	{ "csgo_negev",	WEAPONCLASS_MACHINEGUN },
	{ "sfmg",	WEAPONCLASS_MACHINEGUN },
	{ "hk23",	WEAPONCLASS_MACHINEGUN },
	{ "m60",	WEAPONCLASS_MACHINEGUN },
	{ "mk48",	WEAPONCLASS_MACHINEGUN },
	{ "skull8",	WEAPONCLASS_MACHINEGUN },
	{ "skull6",	WEAPONCLASS_MACHINEGUN },
	{ "m249ep",	WEAPONCLASS_MACHINEGUN },
	{ "charger7",	WEAPONCLASS_MACHINEGUN },
	{ "z4b_m60amethyst",	WEAPONCLASS_MACHINEGUN },
	{ "broad",	WEAPONCLASS_MACHINEGUN },
	{ "thanatos7",	WEAPONCLASS_MACHINEGUN },
	{ "crow7",	WEAPONCLASS_MACHINEGUN },
	{ "coilmg",	WEAPONCLASS_MACHINEGUN },
	{ "spmg",	WEAPONCLASS_MACHINEGUN },
	{ "cameragun",	WEAPONCLASS_MACHINEGUN },
	{ "mgsm",	WEAPONCLASS_MACHINEGUN },
	{ "kronos7",	WEAPONCLASS_MACHINEGUN },
	{ "blockmg",	WEAPONCLASS_MACHINEGUN },
	{ "buffng7",	WEAPONCLASS_MACHINEGUN },
	{ "janus7",	WEAPONCLASS_MACHINEGUN },
	{ "y22s3janus7",	WEAPONCLASS_MACHINEGUN },
	{ "laserminigun",	WEAPONCLASS_MACHINEGUN },
	{ "vulcanus7",	WEAPONCLASS_MACHINEGUN },

	{ "crossbow",	WEAPONCLASS_EQUIPMENT },
	{ "chainsaw",	WEAPONCLASS_EQUIPMENT },
	{ "chainsawm",	WEAPONCLASS_EQUIPMENT },
	{ "chainsawex",	WEAPONCLASS_EQUIPMENT },
	{ "herochainsaw",	WEAPONCLASS_EQUIPMENT },
	{ "cannon",	WEAPONCLASS_EQUIPMENT },
	{ "z4b_cannonplasma",	WEAPONCLASS_EQUIPMENT },
	{ "z4b_cannontiger",	WEAPONCLASS_EQUIPMENT },
	{ "gungnir",	WEAPONCLASS_EQUIPMENT },
	{ "huntbow",	WEAPONCLASS_EQUIPMENT },
	{ "sgdrill",	WEAPONCLASS_EQUIPMENT },
	{ "revivegun",	WEAPONCLASS_EQUIPMENT },
	{ "csgo_zeus",	WEAPONCLASS_EQUIPMENT },
	{ "zombielaw",	WEAPONCLASS_EQUIPMENT },
	{ "m32venom",	WEAPONCLASS_EQUIPMENT },
	{ "cannonex",	WEAPONCLASS_EQUIPMENT },
	{ "speargun",	WEAPONCLASS_EQUIPMENT },
	{ "sgmissile",	WEAPONCLASS_EQUIPMENT },
	{ "sgmissilem",	WEAPONCLASS_EQUIPMENT },
	{ "sgmissileex",	WEAPONCLASS_EQUIPMENT },
	{ "winggun",	WEAPONCLASS_EQUIPMENT },
	{ "wondercannon",	WEAPONCLASS_EQUIPMENT },
	{ "pianogun",	WEAPONCLASS_EQUIPMENT },
	{ "pianogunex",	WEAPONCLASS_EQUIPMENT },
	{ "sgdrillex",	WEAPONCLASS_EQUIPMENT },
	{ "airburster",	WEAPONCLASS_EQUIPMENT },
	{ "flamethrower",	WEAPONCLASS_EQUIPMENT },
	{ "poisongun",	WEAPONCLASS_EQUIPMENT },
	{ "watercannon",	WEAPONCLASS_EQUIPMENT },
	{ "fglauncher",	WEAPONCLASS_EQUIPMENT },
	{ "guillotine",	WEAPONCLASS_EQUIPMENT },
	{ "guillotineex",	WEAPONCLASS_EQUIPMENT },
	{ "speargunm",	WEAPONCLASS_EQUIPMENT },
	{ "wondercannonex",	WEAPONCLASS_EQUIPMENT },
	{ "drillgun",	WEAPONCLASS_EQUIPMENT },

	{ "syringe",	WEAPONCLASS_EQUIPMENT },
	{ "zombielaw",	WEAPONCLASS_EQUIPMENT },

	{ "knife",	WEAPONCLASS_KNIFE },
	{ NULL,		WEAPONCLASS_NONE }
};

DLL_GLOBAL WeaponInfoStruct weaponInfo[] =
{
	{ WEAPON_P228,		P228_PRICE,		AMMO_357SIG_PRICE,	AMMO_357SIG_BUY,	P228_MAX_CLIP,		MAX_AMMO_357SIG,	AMMO_357SIG,		"weapon_p228" },
	{ WEAPON_GLOCK,		GLOCK18_PRICE,		AMMO_9MM_PRICE,		AMMO_9MM_BUY,		GLOCK18_MAX_CLIP,	MAX_AMMO_9MM,		AMMO_9MM,		"weapon_glock18" },
	{ WEAPON_GLOCK18,	GLOCK18_PRICE,		AMMO_9MM_PRICE,		AMMO_9MM_BUY,		GLOCK18_MAX_CLIP,	MAX_AMMO_9MM,		AMMO_9MM,		"weapon_glock18" },
	{ WEAPON_SCOUT,		SCOUT_PRICE,		AMMO_762MM_PRICE,	AMMO_762NATO_BUY,	SCOUT_MAX_CLIP,		MAX_AMMO_762NATO,	AMMO_762NATO,		"weapon_scout" },
	{ WEAPON_XM1014,	XM1014_PRICE,		AMMO_BUCKSHOT_PRICE,	AMMO_BUCKSHOT_BUY,	XM1014_MAX_CLIP,	MAX_AMMO_BUCKSHOT,	AMMO_BUCKSHOT,		"weapon_xm1014" },
	{ WEAPON_MAC10,		MAC10_PRICE,		AMMO_45ACP_PRICE,	AMMO_45ACP_BUY,		MAC10_MAX_CLIP,		MAX_AMMO_45ACP,		AMMO_45ACP,		"weapon_mac10" },
	{ WEAPON_AUG,		AUG_PRICE,		AMMO_556MM_PRICE,	AMMO_556NATO_BUY,	AUG_MAX_CLIP,		MAX_AMMO_556NATO,	AMMO_556NATO,		"weapon_aug" },
	{ WEAPON_ELITE,		ELITE_PRICE,		AMMO_9MM_PRICE,		AMMO_9MM_BUY,		ELITE_MAX_CLIP,		MAX_AMMO_9MM,		AMMO_9MM,		"weapon_elite" },
	{ WEAPON_FIVESEVEN,	FIVESEVEN_PRICE,	AMMO_57MM_PRICE,	AMMO_57MM_BUY,		FIVESEVEN_MAX_CLIP,	MAX_AMMO_57MM,		AMMO_57MM,		"weapon_fiveseven" },
	{ WEAPON_UMP45,		UMP45_PRICE,		AMMO_45ACP_PRICE,	AMMO_45ACP_BUY, 	UMP45_MAX_CLIP,		MAX_AMMO_45ACP,		AMMO_45ACP,		"weapon_ump45" },
	{ WEAPON_SG550,		SG550_PRICE,		AMMO_556MM_PRICE,	AMMO_556NATO_BUY,	SG550_MAX_CLIP,		MAX_AMMO_556NATO,	AMMO_556NATO,		"weapon_sg550" },
	{ WEAPON_GALIL,		GALIL_PRICE,		AMMO_556MM_PRICE,	AMMO_556NATO_BUY,	GALIL_MAX_CLIP,		MAX_AMMO_556NATO,	AMMO_556NATO,		"weapon_galil" },
	{ WEAPON_FAMAS,		FAMAS_PRICE,		AMMO_556MM_PRICE,	AMMO_556NATO_BUY,	FAMAS_MAX_CLIP,		MAX_AMMO_556NATO,	AMMO_556NATO,		"weapon_famas" },
	{ WEAPON_USP,		USP_PRICE,		AMMO_45ACP_PRICE,	AMMO_45ACP_BUY,		USP_MAX_CLIP,		MAX_AMMO_45ACP,		AMMO_45ACP,		"weapon_usp" },
	{ WEAPON_AWP,		AWP_PRICE,		AMMO_338MAG_PRICE,	AMMO_338MAG_BUY,	AWP_MAX_CLIP,		MAX_AMMO_338MAGNUM,	AMMO_338MAGNUM,		"weapon_awp" },
	{ WEAPON_MP5N,		MP5NAVY_PRICE,		AMMO_9MM_PRICE,		AMMO_9MM_BUY,		MP5N_MAX_CLIP,		MAX_AMMO_9MM,		AMMO_9MM,		"weapon_mp5navy" },
	{ WEAPON_M249,		M249_PRICE,		AMMO_556MM_PRICE,	AMMO_556NATOBOX_BUY,	M249_MAX_CLIP,		MAX_AMMO_556NATOBOX,	AMMO_556NATOBOX,	"weapon_m249" },
	{ WEAPON_M3,		M3_PRICE,		AMMO_BUCKSHOT_PRICE,	AMMO_BUCKSHOT_BUY,	M3_MAX_CLIP,		MAX_AMMO_BUCKSHOT,	AMMO_BUCKSHOT,		"weapon_m3" },
	{ WEAPON_M4A1,		M4A1_PRICE,		AMMO_556MM_PRICE,	AMMO_556NATO_BUY,	M4A1_MAX_CLIP,		MAX_AMMO_556NATO,	AMMO_556NATO,		"weapon_m4a1" },
	{ WEAPON_TMP,		TMP_PRICE,		AMMO_9MM_PRICE,		AMMO_9MM_BUY,		TMP_MAX_CLIP,		MAX_AMMO_9MM,		AMMO_9MM,		"weapon_tmp" },
	{ WEAPON_G3SG1,		G3SG1_PRICE,		AMMO_762MM_PRICE,	AMMO_762NATO_BUY,	G3SG1_MAX_CLIP,		MAX_AMMO_762NATO,	AMMO_762NATO,		"weapon_g3sg1" },
	{ WEAPON_DEAGLE,	DEAGLE_PRICE,		AMMO_50AE_PRICE,	AMMO_50AE_BUY,		DEAGLE_MAX_CLIP,	MAX_AMMO_50AE,		AMMO_50AE,		"weapon_deagle" },
	{ WEAPON_SG552,		SG552_PRICE,		AMMO_556MM_PRICE,	AMMO_556NATO_BUY,	SG552_MAX_CLIP,		MAX_AMMO_556NATO,	AMMO_556NATO,		"weapon_sg552" },
	{ WEAPON_AK47,		AK47_PRICE,		AMMO_762MM_PRICE,	AMMO_762NATO_BUY,	AK47_MAX_CLIP,		MAX_AMMO_762NATO,	AMMO_762NATO,		"weapon_ak47" },
	{ WEAPON_P90,		P90_PRICE,		AMMO_57MM_PRICE,	AMMO_57MM_BUY,		P90_MAX_CLIP,		MAX_AMMO_57MM,		AMMO_57MM,		"weapon_p90" },
	{ WEAPON_SHIELDGUN,	SHIELDGUN_PRICE,	0,			0,			0,			0,			-1,			NULL },
	{ 0,			0,			0,			0,			0,			0,			-1,			NULL }
};

// Given an alias, return the associated weapon ID
DLL_GLOBAL WeaponIdType AliasToWeaponID(const char *alias)
{
	if (alias != NULL)
	{
		for (int i = 0; weaponAliasInfo[i].alias != NULL; ++i)
		{
			if (!Q_stricmp(weaponAliasInfo[i].alias, alias))
				return weaponAliasInfo[i].id;
		}
	}

	return WEAPON_NONE;
}

const char *BuyAliasToWeaponID(const char *alias, WeaponIdType &id)
{
	if (alias)
	{
		for (int i = 0; weaponBuyAliasInfo[i].alias != NULL; ++i)
		{
			if (!Q_stricmp(weaponBuyAliasInfo[i].alias, alias))
			{
				id = weaponBuyAliasInfo[i].id;
				return weaponBuyAliasInfo[i].failName;
			}
		}
	}

	id = WEAPON_NONE;
	return NULL;
}

// Given a weapon ID, return its alias

const char *WeaponIDToAlias(int id)
{
	for (int i = 0; weaponAliasInfo[i].alias != NULL; ++i)
	{
		if (weaponAliasInfo[i].id == id)
			return weaponAliasInfo[i].alias;
	}

	return NULL;
}

WeaponClassType AliasToWeaponClass(const char *alias)
{
	if (alias != NULL)
	{
		for (int i = 0; weaponClassAliasInfo[i].alias != NULL; ++i)
		{
			if (!Q_stricmp(weaponClassAliasInfo[i].alias, alias))
				return weaponClassAliasInfo[i].id;
		}
	}

	return WEAPONCLASS_NONE;
}

WeaponClassType WeaponIDToWeaponClass(int id)
{
	return AliasToWeaponClass(WeaponIDToAlias(id));
}

// Return true if given weapon ID is a primary weapon

bool IsPrimaryWeapon(int id)
{
	switch (id)
	{
	case WEAPON_SCOUT:
	case WEAPON_XM1014:
	case WEAPON_MAC10:
	case WEAPON_AUG:
	case WEAPON_UMP45:
	case WEAPON_SG550:
	case WEAPON_GALIL:
	case WEAPON_FAMAS:
	case WEAPON_AWP:
	case WEAPON_MP5N:
	case WEAPON_M249:
	case WEAPON_M3:
	case WEAPON_M4A1:
	case WEAPON_TMP:
	case WEAPON_G3SG1:
	case WEAPON_SG552:
	case WEAPON_AK47:
	case WEAPON_P90:
	case WEAPON_SHIELDGUN:
	case WEAPON_BALROG11:
	case WEAPON_BALROG11B:
	case WEAPON_BALROG11WC:
	case WEAPON_GATLING:
	case WEAPON_GATLINGEX:
	case WEAPON_M1887:
	case WEAPON_Z4B_M1887SE:
	case WEAPON_CSGO_MAG7:
	case WEAPON_QBARREL:
	case WEAPON_CSGO_SAWEDOFF:
	case WEAPON_TBARREL:
	case WEAPON_CSGO_BIZON:
	case WEAPON_K1A:
	case WEAPON_Z4B_K1ASES:
	case WEAPON_KRISS:
	case WEAPON_MP7A1D:
	case WEAPON_MP7A1_CARBINE:
	case WEAPON_MP7A1_PISTOL:
	case WEAPON_SFSMG:
	case WEAPON_THOMPSON:
	case WEAPON_Z4B_MP7A1DX:
	case WEAPON_BISON:
	case WEAPON_TMPDRAGON:
	case WEAPON_SPSMG:
	case WEAPON_AK47DRAGON:
	case WEAPON_AK47_LONG:
	case WEAPON_BUFFSG552EX:
	case WEAPON_M14EBR:
	case WEAPON_Z4B_M37SE:
	case WEAPON_M4A1DRAGON:
	case WEAPON_Z4B_M4A1RAZER:
	case WEAPON_SCARHEAVY:
	case WEAPON_SCARLIGHT:
	case WEAPON_SFGUN:
	case WEAPON_SKULL5:
	case WEAPON_STARCHASERAR:
	case WEAPON_STG44:
	case WEAPON_SVDEX:
	case WEAPON_TAR21:
	case WEAPON_XM8CARBINE:
	case WEAPON_XM8SHARPSHOOTER:
	case WEAPON_Z4B_AK47X:
	case WEAPON_Z4B_M4A1MW:
	case WEAPON_Z4B_M4A1X:
	case WEAPON_BUFFM4:
	case WEAPON_BUFFAK:
	case WEAPON_PLASMAGUN:
	case WEAPON_Z4B_AQUARIUSPLASMAGUN:
	case WEAPON_Y20S2SCARD:
	case WEAPON_LIGHTZG:
	case WEAPON_HEAVYZG:
	case WEAPON_AK47G:
	case WEAPON_AS50:
	case WEAPON_Z4B_AWPNVIDIA:
	case WEAPON_BENDITA:
	case WEAPON_M95:
	case WEAPON_M95XMAS:
	case WEAPON_WA2000:
	case WEAPON_Z4B_XM2010PC:
	case WEAPON_SFSNIPER:
	case WEAPON_M95TIGER:
	case WEAPON_CHEYTACLRRS:
	case WEAPON_M400:
	case WEAPON_M82:
	case WEAPON_SL8:
	case WEAPON_SL8EX:
	case WEAPON_TRG42:
	case WEAPON_TRG42G:
	case WEAPON_M24:
	case WEAPON_AW50:
	case WEAPON_SPRIFLE:
	case WEAPON_BPGM:
	case WEAPON_AS50G:
	case WEAPON_ZGUN:
	case WEAPON_Z4B_BARRETTD:
	case WEAPON_BALROG7:
	case WEAPON_BUFFM249:
	case WEAPON_JANUS7XMAS:
	case WEAPON_M134:
	case WEAPON_M134EX:
	case WEAPON_Z4B_M134HEROI:
	case WEAPON_M134XMAS:
	case WEAPON_M2:
	case WEAPON_M249EX:
	case WEAPON_MG3:
	case WEAPON_MG36:
	case WEAPON_MG3XMAS:
	case WEAPON_CSGO_NEGEV:
	case WEAPON_SFMG:
	case WEAPON_HK23:
	case WEAPON_M60:
	case WEAPON_MK48:
	case WEAPON_K3:
	case WEAPON_CROSSBOW:
	case WEAPON_CHAINSAW:
	case WEAPON_BOW:
	case WEAPON_CANNON:
	case WEAPON_Z4B_CANNONPLASMA:
	case WEAPON_Z4B_CANNONTIGER:
	case WEAPON_GUNGNIR:
	case WEAPON_HUNTBOW:
	case WEAPON_SGDRILL:
	case WEAPON_REVIVEGUN:
	case WEAPON_CSGO_ZEUS:
	case WEAPON_ZOMBIELAW:
	case WEAPON_M32VENOM:
	case WEAPON_CANNONEX:
	case WEAPON_SPEARGUN:
	case WEAPON_SGMISSILE:
	case WEAPON_SGMISSILEM:
	case WEAPON_SGMISSILEEX:
	case WEAPON_Z4B_FREEDOM:
	case WEAPON_Z4B_CSG12SS:
	case WEAPON_SKULL8:
	case WEAPON_SKULL4:
	case WEAPON_SKULL6:
	case WEAPON_M249EP:
	case WEAPON_M3DRAGON:
	case WEAPON_M3DRAGONM:
	case WEAPON_ETHEREAL:
	case WEAPON_RAILGUN:
	case WEAPON_WINGGUN:
	case WEAPON_CROW7:
	case WEAPON_CROW5:
	case WEAPON_LASERSG:
	case WEAPON_LASERFIST:
	case WEAPON_WONDERCANNON:
	case WEAPON_PIANOGUN:
	case WEAPON_STUNRIFLE:
	case WEAPON_KRONOS12:
	case WEAPON_CROSSBOWEX21:
	case WEAPON_BUFFAWP:
	case WEAPON_RAILCANNON:
	case WEAPON_SKULL11:
	case WEAPON_JANUS11:
	case WEAPON_JANUSMK5:
	case WEAPON_JANUS3:
	case WEAPON_CROW3:
	case WEAPON_CROW11:
	case WEAPON_CHARGER5:
	case WEAPON_CHARGER7:
	case WEAPON_Z4B_M60AMETHYST:
	case WEAPON_Z4B_DBARRELAMETHYST:
	case WEAPON_Z4B_ACRAMETHYST:
	case WEAPON_Z4B_AR57AMETHYST:
	case WEAPON_BROAD:
	case WEAPON_PIANOGUNEX:
	case WEAPON_SGDRILLEX:
	case WEAPON_BALROG3:
	case WEAPON_BALROG5:
	case WEAPON_COILMG:
	case WEAPON_AIRBURSTER:
	case WEAPON_POISONGUN:
	case WEAPON_FLAMETHROWER:
	case WEAPON_WATERCANNON:
	case WEAPON_LOCKONGUN:
	case WEAPON_FGLAUNCHER:
	case WEAPON_BUFFAUG:
	case WEAPON_BUFFSG552:
	case WEAPON_CAMERAGUN:
	case WEAPON_M1887G:
	case WEAPON_MGSM:
	case WEAPON_THANATOS11:
	case WEAPON_M1887XMAS:
	case WEAPON_KRONOS5:
	case WEAPON_AN94:
	case WEAPON_M16A4:
	case WEAPON_KRONOS3:
	case WEAPON_KRONOS7:
	case WEAPON_LASERFISTEX:
	case WEAPON_RAINBOWGUN:
	case WEAPON_GUILLOTINE:
	case WEAPON_GUILLOTINEEX:
	case WEAPON_SPEARGUNM:
	case WEAPON_SPSG:
	case WEAPON_DESTROYER:
	case WEAPON_BLOCKAR:
	case WEAPON_BLOCKMG:
	case WEAPON_BLOCKAS:
	case WEAPON_COILGUN:
	case WEAPON_BOUNCER:
	case WEAPON_THANATOS5:
	case WEAPON_OICW:
	case WEAPON_STARCHASERSR:
	case WEAPON_M1GARAND:
	case WEAPON_DUALKRISSHERO:
	case WEAPON_DUALUZI:
	case WEAPON_GILBOAEX:
	case WEAPON_BUFFNG7:
	case WEAPON_GUITAR:
	case WEAPON_VIOLINGUN:
	case WEAPON_WONDERCANNONEX:
	case WEAPON_JANUS7:
	case WEAPON_Y22S3JANUS7:
	case WEAPON_DUALKRISS:
	case WEAPON_MOSIN:
	case WEAPON_Z4B_STEYRTMPD:
	case WEAPON_MK3A1:
	case WEAPON_MK3A1SE:
	case WEAPON_DRILLGUN:
	case WEAPON_VULCANUS3:
	case WEAPON_SKULL3:
	case WEAPON_SKULL3D:
	case WEAPON_Z4B_DEATHRAY:
	case WEAPON_CARTBLUEC:
	case WEAPON_CARTBLUES:
	case WEAPON_CARTREDL:
	case WEAPON_CARTREDH:
		return true;
	default:
		break;
	}

	return false;
}

// Return true if given weapon ID is a secondary weapon

bool IsSecondaryWeapon(int id)
{
	switch (id)
	{
	case WEAPON_P228:
	case WEAPON_ELITE:
	case WEAPON_FIVESEVEN:
	case WEAPON_USP:
	case WEAPON_GLOCK18:
	case WEAPON_DEAGLE:
	case WEAPON_ANACONDA:
	case WEAPON_CSGO_CZ75:
	case WEAPON_DEAGLED:
	case WEAPON_Z4B_DEAGLESHURA:
	case WEAPON_DESPERADO:
	case WEAPON_Z4B_FREQUENCY1:
	case WEAPON_GUNKATA:
	case WEAPON_INFINITY:
	case WEAPON_INFINITYEX1:
	case WEAPON_INFINITYEX2:
	case WEAPON_INFINITYSB:
	case WEAPON_INFINITYSR:
	case WEAPON_INFINITYSS:
	case WEAPON_Z4B_INFINITYX:
	case WEAPON_CSGO_R8:
	case WEAPON_CSGO_TEC9:
	case WEAPON_VOIDPISTOL:
	case WEAPON_Y21S1JETGUNMD:
	case WEAPON_M950:
	case WEAPON_SKULL1:
	case WEAPON_M950SE:
	case WEAPON_SFPISTOL:
	case WEAPON_Y22S2SFPISTOL:
	case WEAPON_Z4B_RAGINGBULL:
	case WEAPON_BALROG1:
	case WEAPON_SAPIENTIA:
	case WEAPON_Z4B_DEAGLEAMETHYST:
	case WEAPON_BLOODHUNTER:
	case WEAPON_THUNDERPISTOL:
	case WEAPON_KRONOS1:
	case WEAPON_KINGCOBRA:
	case WEAPON_KINGCOBRAG:
	case WEAPON_SKULL2:
	case WEAPON_MUSKET:
	case WEAPON_BUFFFIVESEVEN:
	case WEAPON_RESTRICTIONPISTOL:
	case WEAPON_VOIDPISTOLEX:
	case WEAPON_WATERPISTOL:
	case WEAPON_MONKEYWPNSET2:
	case WEAPON_TKNIFE:
	case WEAPON_TKNIFEEX:
	case WEAPON_TKNIFEEX2:
	case WEAPON_Z4B_TKNIFEDX:
	case WEAPON_Z4B_MALORIAN3516:
		return true;
	default:
		break;
	}

	return false;
}

WeaponInfoStruct *GetWeaponInfo(int weaponID)
{
	for (int i = 0; weaponInfo[i].id != 0; ++i)
	{
		if (weaponInfo[i].id == weaponID)
			return &weaponInfo[i];
	}

	return NULL;
}

bool CanBuyWeaponByMaptype(int playerTeam, WeaponIdType weaponID, bool useAssasinationRestrictions)
{
	if (useAssasinationRestrictions)
	{
		if (playerTeam == CT)
		{
			switch (weaponID)
			{
			case WEAPON_P228:
			case WEAPON_XM1014:
			case WEAPON_AUG:
			case WEAPON_FIVESEVEN:
			case WEAPON_UMP45:
			case WEAPON_SG550:
			case WEAPON_FAMAS:
			case WEAPON_USP:
			case WEAPON_GLOCK18:
			case WEAPON_MP5N:
			case WEAPON_M249:
			case WEAPON_M3:
			case WEAPON_M4A1:
			case WEAPON_TMP:
			case WEAPON_DEAGLE:
			case WEAPON_P90:
			case WEAPON_SHIELDGUN:
				return true;
			default:
				return false;
			}
		}
		else if (playerTeam == TERRORIST)
		{
			switch (weaponID)
			{
			case WEAPON_P228:
			case WEAPON_MAC10:
			case WEAPON_ELITE:
			case WEAPON_UMP45:
			case WEAPON_GALIL:
			case WEAPON_USP:
			case WEAPON_GLOCK18:
			case WEAPON_AWP:
			case WEAPON_DEAGLE:
			case WEAPON_AK47:
				return true;
			default:
				return false;
			}
		}

		return false;
	}
	if (playerTeam == CT)
	{
		switch (weaponID)
		{
		case WEAPON_P228:
		case WEAPON_SCOUT:
		case WEAPON_XM1014:
		case WEAPON_AUG:
		case WEAPON_FIVESEVEN:
		case WEAPON_UMP45:
		case WEAPON_SG550:
		case WEAPON_FAMAS:
		case WEAPON_USP:
		case WEAPON_GLOCK18:
		case WEAPON_AWP:
		case WEAPON_MP5N:
		case WEAPON_M249:
		case WEAPON_M3:
		case WEAPON_M4A1:
		case WEAPON_TMP:
		case WEAPON_DEAGLE:
		case WEAPON_P90:
		case WEAPON_SHIELDGUN:
			return true;
		default:
			return false;
		}
	}
	else if (playerTeam == TERRORIST)
	{
		switch (weaponID)
		{
		case WEAPON_P228:
		case WEAPON_SCOUT:
		case WEAPON_XM1014:
		case WEAPON_MAC10:
		case WEAPON_ELITE:
		case WEAPON_UMP45:
		case WEAPON_GALIL:
		case WEAPON_USP:
		case WEAPON_GLOCK18:
		case WEAPON_AWP:
		case WEAPON_MP5N:
		case WEAPON_M249:
		case WEAPON_M3:
		case WEAPON_G3SG1:
		case WEAPON_DEAGLE:
		case WEAPON_SG552:
		case WEAPON_AK47:
		case WEAPON_P90:
			return true;
		default:
			return false;
		}
	}

	return false;
}

bool HasPlayerItem(CBasePlayer *pPlayer, const char *pszCheckItem)
{
	for (CBasePlayerItem *pItem : pPlayer->m_rgpPlayerItems) {
		while (pItem != nullptr) {
			if (FClassnameIs(pItem->pev, "weapon_patroldrone"))
			{

			}
			else if (FClassnameIs(pItem->pev, pszCheckItem))
				return true;

			pItem = pItem->m_pNext;
		}
	}
	return false;
}

bool MoE_HandleBuyCommands(CBasePlayer* pPlayer, const char* pszCommand)
{
	if (!pPlayer->CanPlayerBuy(true))
		return false;

	if (HasPlayerItem(pPlayer, pszCommand)) {
		ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "#Cstrike_Already_Own_Weapon");
		return false;
	}
#ifndef XASH_DEDICATED
	auto iter = std::find_if(std::begin(g_MoEWeaponBuyInfoLocal), std::end(g_MoEWeaponBuyInfoLocal),
		[pszCommand](const MoEWeaponBuyInfo_s& info) {
			return !strcmp(info.pszClassName, pszCommand);
		});

	if (iter == std::end(g_MoEWeaponBuyInfoLocal)) {
		return false;
	}
#else
	auto iter = std::find_if(std::begin(g_MoEWeaponBuyInfo), std::end(g_MoEWeaponBuyInfo),
		[pszCommand](const MoEWeaponBuyInfo_s& info) {
			return !strcmp(info.pszClassName, pszCommand);
		});

	if (iter == std::end(g_MoEWeaponBuyInfo)) {
		return false;
	}
#endif

	if (pPlayer->m_iAccount < iter->iCost && iter->iID != WEAPON_PATROLDRONE) {
		ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "#Not_Enough_Money");
		BlinkAccount(pPlayer, 2);

		return false;
	}

	if (iter->iSlot == GRENADE_SLOT) {
		if (g_pGameRules->m_setBanGrenade.find(iter->iID) != g_pGameRules->m_setBanGrenade.end())
			return false;
	}
	else if (iter->iSlot == KNIFE_SLOT) {
		if (g_pGameRules->m_setBanKnife.find(iter->iID) != g_pGameRules->m_setBanKnife.end())
			return false;
	}
	else {
		if (g_pGameRules->m_setBanWeapon.find(iter->iID) != g_pGameRules->m_setBanWeapon.end())
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
				if (!strcmp(iter->pszClassName, "csgo_zeus"))
				{
				}
				else
				{
					pPlayer->RemovePlayerItem(pPlayer->m_rgpPlayerItems[KNIFE_SLOT]);
					pPlayer->m_iKnifeID = AliasToKnifeType(iter->pszClassName);
				}
			}
			break;
		case GRENADE_SLOT:
			{		
					bool bGrenadeChanged = pPlayer->m_iGrenadeID != iter->iID;
					bool bWasActiveItem = pPlayer->m_pActiveItem && 
						(pPlayer->m_pActiveItem->m_iId == WEAPON_HEGRENADE || 
						pPlayer->m_pActiveItem->m_iId == WEAPON_PATROLDRONE || 
						pPlayer->m_pActiveItem->m_iId == WEAPON_DIVINETITAN || 
						pPlayer->m_pActiveItem->m_iId == WEAPON_BUNKERBUSTER);

					if (iter->iID == WEAPON_PATROLDRONE)
					{
						CBasePlayerWeapon* pGrenade = (CBasePlayerWeapon*)pPlayer->m_rgpPlayerItems[GRENADE_SLOT];
						//buy patroldrone ammo
						while (pGrenade)
						{
							if (pGrenade->m_iId == WEAPON_PATROLDRONE)
							{
								//add clip
								if (pPlayer->m_iAccount < 500) {
									ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "#Not_Enough_Money");
									BlinkAccount(pPlayer, 2);
									return false;
								}

								if (pPlayer->m_rgAmmo[pGrenade->m_iPrimaryAmmoType] >= PATROLDRONE_MAX_CLIP)
								{
									//full ammo
									ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "#Cstrike_Already_Own_Weapon");
									return false;
								}

								CPatrolDrone* pLinkWeapon = dynamic_cast<CPatrolDrone*>(pGrenade);
								pPlayer->m_rgAmmo[pLinkWeapon->m_iPrimaryAmmoType]++;
								for (int i = 0; i < PATROLDRONE_MAX_CLIP; i++)
								{
									if (pLinkWeapon->m_iClipAmmo[i] == -1)
									{
										pLinkWeapon->m_iClipAmmo[i] = 0;
										MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, pPlayer->pev);
										WRITE_BYTE(24);
										WRITE_BYTE(i);
										WRITE_SHORT(pLinkWeapon->m_iClipAmmo[i]);
										WRITE_BYTE(PATROLDRONE_OFF);
										MESSAGE_END();
										break;
									}
								}
								pPlayer->AddAccount(-500);
								pPlayer->BuildRebuyStruct();

								return true;
							}

							pGrenade = (CBasePlayerWeapon*)pGrenade->m_pNext;
						}
					}

					if (bGrenadeChanged)
					{
						if (pPlayer->m_iAccount < iter->iCost) {
							ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "#Not_Enough_Money");
							BlinkAccount(pPlayer, 2);

							return false;
						}

						CBasePlayerWeapon* pGrenade = (CBasePlayerWeapon*)pPlayer->m_rgpPlayerItems[GRENADE_SLOT];
						std::vector<CBasePlayerWeapon*> GrenadesToRemove;

						while (pGrenade)
						{
							if (pGrenade->m_iId == WEAPON_HEGRENADE ||
								pGrenade->m_iId == WEAPON_PATROLDRONE ||
								pGrenade->m_iId == WEAPON_DIVINETITAN ||
								pGrenade->m_iId == WEAPON_BUNKERBUSTER)
							{
								pGrenade->Kill();

								GrenadesToRemove.push_back(pGrenade);
							}

							pGrenade = (CBasePlayerWeapon*)pGrenade->m_pNext;
						}

						for (auto iter : GrenadesToRemove)
						{
							pPlayer->RemovePlayerItem(iter);
						}
					}

					/*switch (iter->iID)
					{
					case WEAPON_PATROLDRONE: pPlayer->m_iGrenadeID = AliasToGrenadeType(iter->pszClassName); break;
					case WEAPON_DIVINETITAN: pPlayer->m_iGrenadeID = AliasToGrenadeType(iter->pszClassName); break;
					case WEAPON_BUNKERBUSTER: pPlayer->m_iGrenadeID = AliasToGrenadeType(iter->pszClassName); break;
					default:
						pPlayer->m_iGrenadeID = AliasToGrenadeType(iter->pszClassName);
						break;
					}*/

					pPlayer->m_iGrenadeID = AliasToGrenadeType(iter->pszClassName);

					pPlayer->GiveNamedItem(iter->pszClassName);
					pPlayer->AddAccount(-iter->iCost);
					pPlayer->BuildRebuyStruct();

					if (bGrenadeChanged && bWasActiveItem)
					{
						// client->SelectItem("weapon_hegrenade");
						CBasePlayerItem* pGrenade = pPlayer->m_rgpPlayerItems[GRENADE_SLOT];

						while (pGrenade)
						{
							if (pGrenade->m_iId == WEAPON_HEGRENADE ||
								pGrenade->m_iId == WEAPON_PATROLDRONE ||
								pGrenade->m_iId == WEAPON_DIVINETITAN ||
								pGrenade->m_iId == WEAPON_BUNKERBUSTER)
							{
								pPlayer->m_pActiveItem = pGrenade;

								break;
							}

							pGrenade = pGrenade->m_pNext;
						}
					}

					return true;				
#if 0
			CBasePlayerItem* pItem = pPlayer->m_rgpPlayerItems[GRENADE_SLOT];

			if (pItem == NULL)
				pPlayer->m_iGrenadeID = AliasToGrenadeType(iter->pszClassName);
	
			// have at least one weapon in this slot
			while (pItem != NULL)
			{
				if (pPlayer->m_rgpPlayerItems[GRENADE_SLOT]->IsWeapon() && pPlayer->m_rgpPlayerItems[GRENADE_SLOT])
				{
					CBasePlayerWeapon* pGrenade = static_cast<CBasePlayerWeapon*>(pItem);

					if (iter->iID == WEAPON_PATROLDRONE)
					{
						//buy patroldrone
						if (pGrenade->m_iId == WEAPON_PATROLDRONE)
						{
							//add clip
							if (pPlayer->m_rgAmmo[pGrenade->m_iPrimaryAmmoType] >= PATROLDRONE_MAX_CLIP)
							{
								//full ammo
								ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "#Cstrike_Already_Own_Weapon");
								return false;
							}

							CPatrolDrone* pLinkWeapon = dynamic_cast<CPatrolDrone*>(pGrenade);
							pPlayer->m_rgAmmo[pLinkWeapon->m_iPrimaryAmmoType]++;
							for (int i = 0; i < PATROLDRONE_MAX_CLIP; i++)
							{
								if (pLinkWeapon->m_iClipAmmo[i] == -1)
								{
									pLinkWeapon->m_iClipAmmo[i] = 0;
									MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, pPlayer->pev);
									WRITE_BYTE(24);
									WRITE_BYTE(i);
									WRITE_SHORT(pLinkWeapon->m_iClipAmmo[i]);
									WRITE_BYTE(PATROLDRONE_OFF);
									MESSAGE_END();
									break;
								}							
							}
							pPlayer->AddAccount(-500);
							pPlayer->BuildRebuyStruct();

							return true;
						}
						else if (pGrenade->m_iId == WEAPON_DIVINETITAN ||
							pGrenade->m_iId == WEAPON_HEGRENADE ||
							pGrenade->m_iId == WEAPON_BUNKERBUSTER)
						{
							pPlayer->RemovePlayerItem(pGrenade);
							break;
						}
					}
					else if(iter->iID == WEAPON_DIVINETITAN)
					{
						//buy divientitan
						if (pGrenade->m_iId == WEAPON_DIVINETITAN)
						{
							ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "#Cstrike_Already_Own_Weapon");
							return false;
							//already have
						}
						else if (pGrenade->m_iId == WEAPON_PATROLDRONE || 
							pGrenade->m_iId == WEAPON_HEGRENADE ||
							pGrenade->m_iId == WEAPON_BUNKERBUSTER)
						{
							pPlayer->RemovePlayerItem(pGrenade);
							break;
						}
					}
					else if (iter->iID == WEAPON_BUNKERBUSTER)
					{
						//buy bunkerbuster
						if (pGrenade->m_iId == WEAPON_BUNKERBUSTER)
						{
							ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "#Cstrike_Already_Own_Weapon");
							return false;
							//already have
						}
						else if (pGrenade->m_iId == WEAPON_PATROLDRONE || 
							pGrenade->m_iId == WEAPON_HEGRENADE || 
							pGrenade->m_iId == WEAPON_DIVINETITAN)
						{
							pPlayer->RemovePlayerItem(pGrenade);
							break;
						}
					}
					else
					{
						//buy grenande
						if (pGrenade->m_iId == WEAPON_DIVINETITAN || 
							pGrenade->m_iId == WEAPON_PATROLDRONE ||
							pGrenade->m_iId == WEAPON_BUNKERBUSTER)
						{
							//remove and add
							pPlayer->RemovePlayerItem(pGrenade);
						}

						bool bGrenadeChanged = pPlayer->m_iGrenadeID != iter->iID;
						bool bWasActiveItem = pPlayer->m_pActiveItem && pPlayer->m_pActiveItem->m_iId == WEAPON_HEGRENADE;

						if (bGrenadeChanged)
						{
							CBasePlayerWeapon* pGrenade = (CBasePlayerWeapon*)pPlayer->m_rgpPlayerItems[GRENADE_SLOT];
							std::vector<CBasePlayerWeapon*> GrenadesToRemove;

							while (pGrenade)
							{
								if (pGrenade->m_iId == WEAPON_HEGRENADE)
								{
									pGrenade->Kill();

									GrenadesToRemove.push_back(pGrenade);
								}

								pGrenade = (CBasePlayerWeapon*)pGrenade->m_pNext;
							}

							for (auto iter : GrenadesToRemove)
							{
								pPlayer->RemovePlayerItem(iter);
							}
						}
						pPlayer->m_iGrenadeID = AliasToGrenadeType(iter->pszClassName);					
						break;
					}
				}
				pItem = pItem->m_pNext;
			}		
#endif
			}

			break;
		default:
			break;
	}

	pPlayer->GiveNamedItem(iter->pszClassName);
	pPlayer->AddAccount(-iter->iCost);
	pPlayer->BuildRebuyStruct();

	return true;
}

int AliasToKnifeType(const char* alias)
{
	if (!alias)
		return 0;

	int i = 0;

	while (knifeInfo[i].alias)
	{
		if (!stricmp(alias, knifeInfo[i].alias))
			return knifeInfo[i].id;

		i++;
	}

	return 0;
}

const char* KnifeTypeToAlias(int id)
{
	int i = 0;

	while (1)
	{
		if (id == knifeInfo[i].id)
			return knifeInfo[i].alias;

		i++;
	}

	return NULL;
}

int AliasToGrenadeType(const char* alias)
{
	if (!alias)
		return 0;

	int i = 0;

	while (grenadeInfo[i].alias)
	{
		if (!stricmp(alias, grenadeInfo[i].alias))
			return grenadeInfo[i].id;

		i++;

		if (i > ARRAYSIZE(grenadeInfo) - 1)
			return 0;
	}

	return 0;
}

const char* GrenadeTypeToAlias(int id)
{
	int i = 0;

	while (1)
	{
		if (id == grenadeInfo[i].id)
			return grenadeInfo[i].alias;

		i++;

		if (i > ARRAYSIZE(grenadeInfo) - 1)
			return NULL;
	}

	return NULL;
}

}
