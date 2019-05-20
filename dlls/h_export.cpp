#include "common.h"

#include "extdll.h"
#include "util.h"
#include "cbase.h"

#include "cbase/cbase_physint.h"

//! Holds engine functionality callbacks

namespace sv {
enginefuncs_t g_engfuncs;
globalvars_t *gpGlobals;
}

#if defined (_MSC_VER) && (_MSC_VER > 1000) && !defined(_WIN64)
	#pragma comment (linker, "/EXPORT:GiveFnptrsToDll=_GiveFnptrsToDll@8,@1")
	#pragma comment (linker, "/SECTION:.data,RW")
#endif

#ifndef _WIN32
#define WINAPI // LINUX SUPPORT
#endif

// Receive engine function table from engine.
// This appears to be the _first_ DLL routine called by the engine, so we
// do some setup operations here.
C_DLLEXPORT void WINAPI GiveFnptrsToDll(enginefuncs_t *pEnginefuncsTable, globalvars_t *pGlobals)
{
	Q_memcpy(&sv::g_engfuncs, pEnginefuncsTable, sizeof(enginefuncs_t));
	gpGlobals = pGlobals;
}

#ifdef XASH_STATIC_GAMELIB
typedef struct dllexport_s
{
	const char *name;
	void *func;
} dllexport_t;

extern "C" {
void 	ambient_generic	( entvars_t *pev );
void 	ammo_338magnum	( entvars_t *pev );
void 	ammo_357sig	( entvars_t *pev );
void 	ammo_45acp	( entvars_t *pev );
void 	ammo_46mm	( entvars_t *pev );
void 	ammo_50ae	( entvars_t *pev );
void 	ammo_50bmg	( entvars_t *pev );
void 	ammo_556nato	( entvars_t *pev );
void 	ammo_556natobox	( entvars_t *pev );
void 	ammo_57mm	( entvars_t *pev );
void 	ammo_762nato	( entvars_t *pev );
void 	ammo_9mm	( entvars_t *pev );
void 	ammo_buckshot	( entvars_t *pev );
void 	ammo_cannon	( entvars_t *pev );
void 	ammo_gungnir	( entvars_t *pev );
void 	armoury_entity	( entvars_t *pev );
void 	beam	( entvars_t *pev );
void 	bodyque	( entvars_t *pev );
void 	button_target	( entvars_t *pev );
void 	cycler	( entvars_t *pev );
void 	cycler_prdroid	( entvars_t *pev );
void 	cycler_sprite	( entvars_t *pev );
void 	cycler_weapon	( entvars_t *pev );
void 	cycler_wreckage	( entvars_t *pev );
void 	env_beam	( entvars_t *pev );
void 	env_beverage	( entvars_t *pev );
void 	env_blood	( entvars_t *pev );
void 	env_bombglow	( entvars_t *pev );
void 	env_bubbles	( entvars_t *pev );
void 	env_debris	( entvars_t *pev );
void 	env_explosion	( entvars_t *pev );
void 	env_fade	( entvars_t *pev );
void 	env_fog	( entvars_t *pev );
void 	env_funnel	( entvars_t *pev );
void 	env_global	( entvars_t *pev );
void 	env_glow	( entvars_t *pev );
void 	env_laser	( entvars_t *pev );
void 	env_lightning	( entvars_t *pev );
void 	env_message	( entvars_t *pev );
void 	env_rain	( entvars_t *pev );
void 	env_render	( entvars_t *pev );
void 	env_shake	( entvars_t *pev );
void 	env_shooter	( entvars_t *pev );
void 	env_snow	( entvars_t *pev );
void 	env_sound	( entvars_t *pev );
void 	env_spark	( entvars_t *pev );
void 	env_sprite	( entvars_t *pev );
void 	fireanddie	( entvars_t *pev );
void 	func_bomb_target	( entvars_t *pev );
void 	func_breakable	( entvars_t *pev );
void 	func_button	( entvars_t *pev );
void 	func_buyzone	( entvars_t *pev );
void 	func_conveyor	( entvars_t *pev );
void 	func_door	( entvars_t *pev );
void 	func_door_rotating	( entvars_t *pev );
void 	func_escapezone	( entvars_t *pev );
void 	func_friction	( entvars_t *pev );
void 	func_grencatch	( entvars_t *pev );
void 	func_guntarget	( entvars_t *pev );
void 	func_healthcharger	( entvars_t *pev );
void 	func_hostage_rescue	( entvars_t *pev );
void 	func_illusionary	( entvars_t *pev );
void 	func_ladder	( entvars_t *pev );
void 	func_monsterclip	( entvars_t *pev );
void 	func_mortar_field	( entvars_t *pev );
void 	func_pendulum	( entvars_t *pev );
void 	func_plat	( entvars_t *pev );
void 	func_platrot	( entvars_t *pev );
void 	func_pushable	( entvars_t *pev );
void 	func_rain	( entvars_t *pev );
void 	func_recharge	( entvars_t *pev );
void 	func_rot_button	( entvars_t *pev );
void 	func_rotating	( entvars_t *pev );
void 	func_snow	( entvars_t *pev );
void 	func_tank	( entvars_t *pev );
void 	func_tankcontrols	( entvars_t *pev );
void 	func_tanklaser	( entvars_t *pev );
void 	func_tankmortar	( entvars_t *pev );
void 	func_tankrocket	( entvars_t *pev );
void 	func_trackautochange	( entvars_t *pev );
void 	func_trackchange	( entvars_t *pev );
void 	func_tracktrain	( entvars_t *pev );
void 	func_train	( entvars_t *pev );
void 	func_traincontrols	( entvars_t *pev );
void 	func_vehicle	( entvars_t *pev );
void 	func_vehiclecontrols	( entvars_t *pev );
void 	func_vip_safetyzone	( entvars_t *pev );
void 	func_wall	( entvars_t *pev );
void 	func_wall_toggle	( entvars_t *pev );
void 	func_water	( entvars_t *pev );
void 	func_weaponcheck	( entvars_t *pev );
void 	game_counter	( entvars_t *pev );
void 	game_counter_set	( entvars_t *pev );
void 	game_end	( entvars_t *pev );
void 	game_player_equip	( entvars_t *pev );
void 	game_player_hurt	( entvars_t *pev );
void 	game_player_team	( entvars_t *pev );
void 	game_score	( entvars_t *pev );
void 	game_team_master	( entvars_t *pev );
void 	game_team_set	( entvars_t *pev );
void 	game_text	( entvars_t *pev );
void 	game_zone_player	( entvars_t *pev );
void 	gibshooter	( entvars_t *pev );
void 	grenade	( entvars_t *pev );
void 	gungnir_projectile	( entvars_t *pev );
void 	gungnir_spear	( entvars_t *pev );
void 	hostage_entity	( entvars_t *pev );
void 	info_bomb_target	( entvars_t *pev );
void 	info_hostage_rescue	( entvars_t *pev );
void 	info_intermission	( entvars_t *pev );
void 	info_landmark	( entvars_t *pev );
void 	info_map_parameters	( entvars_t *pev );
void 	info_null	( entvars_t *pev );
void 	info_player_deathmatch	( entvars_t *pev );
void 	info_player_start	( entvars_t *pev );
void 	info_target	( entvars_t *pev );
void 	info_teleport_destination	( entvars_t *pev );
void 	info_vip_start	( entvars_t *pev );
void 	infodecal	( entvars_t *pev );
void 	item_airtank	( entvars_t *pev );
void 	item_antidote	( entvars_t *pev );
void 	item_assaultsuit	( entvars_t *pev );
void 	item_battery	( entvars_t *pev );
void 	item_healthkit	( entvars_t *pev );
void 	item_kevlar	( entvars_t *pev );
void 	item_longjump	( entvars_t *pev );
void 	item_security	( entvars_t *pev );
void 	item_sodacan	( entvars_t *pev );
void 	item_suit	( entvars_t *pev );
void 	item_thighpack	( entvars_t *pev );
void 	knife_skullaxe	( entvars_t *pev );
void 	knife_zombi	( entvars_t *pev );
void 	light	( entvars_t *pev );
void 	light_environment	( entvars_t *pev );
void 	light_spot	( entvars_t *pev );
void 	momentary_door	( entvars_t *pev );
void 	momentary_rot_button	( entvars_t *pev );
void 	monster_entity	( entvars_t *pev );
void 	monster_hevsuit_dead	( entvars_t *pev );
void 	monster_mortar	( entvars_t *pev );
void 	monster_scientist	( entvars_t *pev );
void 	multi_manager	( entvars_t *pev );
void 	multisource	( entvars_t *pev );
void 	path_corner	( entvars_t *pev );
void 	path_track	( entvars_t *pev );
void 	player	( entvars_t *pev );
void 	player_loadsaved	( entvars_t *pev );
void 	player_weaponstrip	( entvars_t *pev );
void 	soundent	( entvars_t *pev );
void 	spark_shower	( entvars_t *pev );
void 	speaker	( entvars_t *pev );
void 	supplybox	( entvars_t *pev );
void 	target_cdaudio	( entvars_t *pev );
void 	test_effect	( entvars_t *pev );
void 	trigger	( entvars_t *pev );
void 	trigger_auto	( entvars_t *pev );
void 	trigger_autosave	( entvars_t *pev );
void 	trigger_camera	( entvars_t *pev );
void 	trigger_cdaudio	( entvars_t *pev );
void 	trigger_changelevel	( entvars_t *pev );
void 	trigger_changetarget	( entvars_t *pev );
void 	trigger_counter	( entvars_t *pev );
void 	trigger_endsection	( entvars_t *pev );
void 	trigger_gravity	( entvars_t *pev );
void 	trigger_hurt	( entvars_t *pev );
void 	trigger_monsterjump	( entvars_t *pev );
void 	trigger_multiple	( entvars_t *pev );
void 	trigger_once	( entvars_t *pev );
void 	trigger_push	( entvars_t *pev );
void 	trigger_relay	( entvars_t *pev );
void 	trigger_teleport	( entvars_t *pev );
void 	trigger_transition	( entvars_t *pev );
void 	weapon_ak47	( entvars_t *pev );
void 	weapon_ak47l	( entvars_t *pev );
void 	weapon_as50	( entvars_t *pev );
void 	weapon_aug	( entvars_t *pev );
void 	weapon_awp	( entvars_t *pev );
void 	weapon_c4	( entvars_t *pev );
void 	weapon_cannon	( entvars_t *pev );
void 	weapon_deagle	( entvars_t *pev );
void 	weapon_deagled	( entvars_t *pev );
void 	weapon_elite	( entvars_t *pev );
void 	weapon_famas	( entvars_t *pev );
void 	weapon_fiveseven	( entvars_t *pev );
void 	weapon_flashbang	( entvars_t *pev );
void 	weapon_g3sg1	( entvars_t *pev );
void 	weapon_galil	( entvars_t *pev );
void 	weapon_glock18	( entvars_t *pev );
void 	weapon_gungnir	( entvars_t *pev );
void 	weapon_hegrenade	( entvars_t *pev );
void 	weapon_infinity	( entvars_t *pev );
void 	weapon_infinityex1	( entvars_t *pev );
void 	weapon_infinityex2	( entvars_t *pev );
void 	weapon_infinitysb	( entvars_t *pev );
void 	weapon_infinitysr	( entvars_t *pev );
void 	weapon_infinityss	( entvars_t *pev );
void 	weapon_k1a	( entvars_t *pev );
void 	weapon_knife	( entvars_t *pev );
void 	weapon_kriss	( entvars_t *pev );
void 	weapon_m14ebr	( entvars_t *pev );
void 	weapon_m1887	( entvars_t *pev );
void 	weapon_m249	( entvars_t *pev );
void 	weapon_m3	( entvars_t *pev );
void 	weapon_m4a1	( entvars_t *pev );
void 	weapon_m95	( entvars_t *pev );
void 	weapon_mac10	( entvars_t *pev );
void 	weapon_mg3	( entvars_t *pev );
void 	weapon_mp5navy	( entvars_t *pev );
void 	weapon_mp7a1c	( entvars_t *pev );
void 	weapon_mp7a1d	( entvars_t *pev );
void 	weapon_mp7a1p	( entvars_t *pev );
void 	weapon_p228	( entvars_t *pev );
void 	weapon_p90	( entvars_t *pev );
void 	weapon_scarh	( entvars_t *pev );
void 	weapon_scarl	( entvars_t *pev );
void 	weapon_scout	( entvars_t *pev );
void 	weapon_sg550	( entvars_t *pev );
void 	weapon_sg552	( entvars_t *pev );
void 	weapon_smokegrenade	( entvars_t *pev );
void 	weapon_tar21	( entvars_t *pev );
void 	weapon_thompson	( entvars_t *pev );
void 	weapon_tmp	( entvars_t *pev );
void 	weapon_ump45	( entvars_t *pev );
void 	weapon_usp	( entvars_t *pev );
void 	weapon_wa2000	( entvars_t *pev );
void 	weapon_xm1014	( entvars_t *pev );
void 	weapon_xm8c	( entvars_t *pev );
void 	weapon_xm8s	( entvars_t *pev );
void 	weaponbox	( entvars_t *pev );
void 	world_items	( entvars_t *pev );
void 	worldspawn	( entvars_t *pev );
void 	zbsbreak	( entvars_t *pev );
void 	zombiespawn	( entvars_t *pev );
}

static dllexport_t switch_server_exports[] = {
		{ "GiveFnptrsToDll", (void*)GiveFnptrsToDll },
		{ "GetEntityAPI", (void*)GetEntityAPI },
		{ "GetEntityAPI2", (void*)GetEntityAPI2 },
		{ "GetNewDLLFunctions", (void*)GetNewDLLFunctions },
		{ "Server_GetPhysicsInterface", (void*)Server_GetPhysicsInterface },
		{ "ambient_generic", (void *)ambient_generic },
		{ "ammo_338magnum", (void *)ammo_338magnum },
		{ "ammo_357sig", (void *)ammo_357sig },
		{ "ammo_45acp", (void *)ammo_45acp },
		{ "ammo_46mm", (void *)ammo_46mm },
		{ "ammo_50ae", (void *)ammo_50ae },
		{ "ammo_50bmg", (void *)ammo_50bmg },
		{ "ammo_556nato", (void *)ammo_556nato },
		{ "ammo_556natobox", (void *)ammo_556natobox },
		{ "ammo_57mm", (void *)ammo_57mm },
		{ "ammo_762nato", (void *)ammo_762nato },
		{ "ammo_9mm", (void *)ammo_9mm },
		{ "ammo_buckshot", (void *)ammo_buckshot },
		{ "ammo_cannon", (void *)ammo_cannon },
		{ "ammo_gungnir", (void *)ammo_gungnir },
		{ "armoury_entity", (void *)armoury_entity },
		{ "beam", (void *)beam },
		{ "bodyque", (void *)bodyque },
		{ "button_target", (void *)button_target },
		{ "cycler", (void *)cycler },
		{ "cycler_prdroid", (void *)cycler_prdroid },
		{ "cycler_sprite", (void *)cycler_sprite },
		{ "cycler_weapon", (void *)cycler_weapon },
		{ "cycler_wreckage", (void *)cycler_wreckage },
		{ "env_beam", (void *)env_beam },
		{ "env_beverage", (void *)env_beverage },
		{ "env_blood", (void *)env_blood },
		{ "env_bombglow", (void *)env_bombglow },
		{ "env_bubbles", (void *)env_bubbles },
		{ "env_debris", (void *)env_debris },
		{ "env_explosion", (void *)env_explosion },
		{ "env_fade", (void *)env_fade },
		{ "env_fog", (void *)env_fog },
		{ "env_funnel", (void *)env_funnel },
		{ "env_global", (void *)env_global },
		{ "env_glow", (void *)env_glow },
		{ "env_laser", (void *)env_laser },
		{ "env_lightning", (void *)env_lightning },
		{ "env_message", (void *)env_message },
		{ "env_rain", (void *)env_rain },
		{ "env_render", (void *)env_render },
		{ "env_shake", (void *)env_shake },
		{ "env_shooter", (void *)env_shooter },
		{ "env_snow", (void *)env_snow },
		{ "env_sound", (void *)env_sound },
		{ "env_spark", (void *)env_spark },
		{ "env_sprite", (void *)env_sprite },
		{ "fireanddie", (void *)fireanddie },
		{ "func_bomb_target", (void *)func_bomb_target },
		{ "func_breakable", (void *)func_breakable },
		{ "func_button", (void *)func_button },
		{ "func_buyzone", (void *)func_buyzone },
		{ "func_conveyor", (void *)func_conveyor },
		{ "func_door", (void *)func_door },
		{ "func_door_rotating", (void *)func_door_rotating },
		{ "func_escapezone", (void *)func_escapezone },
		{ "func_friction", (void *)func_friction },
		{ "func_grencatch", (void *)func_grencatch },
		{ "func_guntarget", (void *)func_guntarget },
		{ "func_healthcharger", (void *)func_healthcharger },
		{ "func_hostage_rescue", (void *)func_hostage_rescue },
		{ "func_illusionary", (void *)func_illusionary },
		{ "func_ladder", (void *)func_ladder },
		{ "func_monsterclip", (void *)func_monsterclip },
		{ "func_mortar_field", (void *)func_mortar_field },
		{ "func_pendulum", (void *)func_pendulum },
		{ "func_plat", (void *)func_plat },
		{ "func_platrot", (void *)func_platrot },
		{ "func_pushable", (void *)func_pushable },
		{ "func_rain", (void *)func_rain },
		{ "func_recharge", (void *)func_recharge },
		{ "func_rot_button", (void *)func_rot_button },
		{ "func_rotating", (void *)func_rotating },
		{ "func_snow", (void *)func_snow },
		{ "func_tank", (void *)func_tank },
		{ "func_tankcontrols", (void *)func_tankcontrols },
		{ "func_tanklaser", (void *)func_tanklaser },
		{ "func_tankmortar", (void *)func_tankmortar },
		{ "func_tankrocket", (void *)func_tankrocket },
		{ "func_trackautochange", (void *)func_trackautochange },
		{ "func_trackchange", (void *)func_trackchange },
		{ "func_tracktrain", (void *)func_tracktrain },
		{ "func_train", (void *)func_train },
		{ "func_traincontrols", (void *)func_traincontrols },
		{ "func_vehicle", (void *)func_vehicle },
		{ "func_vehiclecontrols", (void *)func_vehiclecontrols },
		{ "func_vip_safetyzone", (void *)func_vip_safetyzone },
		{ "func_wall", (void *)func_wall },
		{ "func_wall_toggle", (void *)func_wall_toggle },
		{ "func_water", (void *)func_water },
		{ "func_weaponcheck", (void *)func_weaponcheck },
		{ "game_counter", (void *)game_counter },
		{ "game_counter_set", (void *)game_counter_set },
		{ "game_end", (void *)game_end },
		{ "game_player_equip", (void *)game_player_equip },
		{ "game_player_hurt", (void *)game_player_hurt },
		{ "game_player_team", (void *)game_player_team },
		{ "game_score", (void *)game_score },
		{ "game_team_master", (void *)game_team_master },
		{ "game_team_set", (void *)game_team_set },
		{ "game_text", (void *)game_text },
		{ "game_zone_player", (void *)game_zone_player },
		{ "gibshooter", (void *)gibshooter },
		{ "grenade", (void *)grenade },
		{ "gungnir_projectile", (void *)gungnir_projectile },
		{ "gungnir_spear", (void *)gungnir_spear },
		{ "hostage_entity", (void *)hostage_entity },
		{ "info_bomb_target", (void *)info_bomb_target },
		{ "info_hostage_rescue", (void *)info_hostage_rescue },
		{ "info_intermission", (void *)info_intermission },
		{ "info_landmark", (void *)info_landmark },
		{ "info_map_parameters", (void *)info_map_parameters },
		{ "info_null", (void *)info_null },
		{ "info_player_deathmatch", (void *)info_player_deathmatch },
		{ "info_player_start", (void *)info_player_start },
		{ "info_target", (void *)info_target },
		{ "info_teleport_destination", (void *)info_teleport_destination },
		{ "info_vip_start", (void *)info_vip_start },
		{ "infodecal", (void *)infodecal },
		{ "item_airtank", (void *)item_airtank },
		{ "item_antidote", (void *)item_antidote },
		{ "item_assaultsuit", (void *)item_assaultsuit },
		{ "item_battery", (void *)item_battery },
		{ "item_healthkit", (void *)item_healthkit },
		{ "item_kevlar", (void *)item_kevlar },
		{ "item_longjump", (void *)item_longjump },
		{ "item_security", (void *)item_security },
		{ "item_sodacan", (void *)item_sodacan },
		{ "item_suit", (void *)item_suit },
		{ "item_thighpack", (void *)item_thighpack },
		{ "knife_skullaxe", (void *)knife_skullaxe },
		{ "knife_zombi", (void *)knife_zombi },
		{ "light", (void *)light },
		{ "light_environment", (void *)light_environment },
		{ "light_spot", (void *)light_spot },
		{ "momentary_door", (void *)momentary_door },
		{ "momentary_rot_button", (void *)momentary_rot_button },
		{ "monster_entity", (void *)monster_entity },
		{ "monster_hevsuit_dead", (void *)monster_hevsuit_dead },
		{ "monster_mortar", (void *)monster_mortar },
		{ "monster_scientist", (void *)monster_scientist },
		{ "multi_manager", (void *)multi_manager },
		{ "multisource", (void *)multisource },
		{ "path_corner", (void *)path_corner },
		{ "path_track", (void *)path_track },
		{ "player", (void *)player },
		{ "player_loadsaved", (void *)player_loadsaved },
		{ "player_weaponstrip", (void *)player_weaponstrip },
		{ "soundent", (void *)soundent },
		{ "spark_shower", (void *)spark_shower },
		{ "speaker", (void *)speaker },
		{ "supplybox", (void *)supplybox },
		{ "target_cdaudio", (void *)target_cdaudio },
		{ "test_effect", (void *)test_effect },
		{ "trigger", (void *)trigger },
		{ "trigger_auto", (void *)trigger_auto },
		{ "trigger_autosave", (void *)trigger_autosave },
		{ "trigger_camera", (void *)trigger_camera },
		{ "trigger_cdaudio", (void *)trigger_cdaudio },
		{ "trigger_changelevel", (void *)trigger_changelevel },
		{ "trigger_changetarget", (void *)trigger_changetarget },
		{ "trigger_counter", (void *)trigger_counter },
		{ "trigger_endsection", (void *)trigger_endsection },
		{ "trigger_gravity", (void *)trigger_gravity },
		{ "trigger_hurt", (void *)trigger_hurt },
		{ "trigger_monsterjump", (void *)trigger_monsterjump },
		{ "trigger_multiple", (void *)trigger_multiple },
		{ "trigger_once", (void *)trigger_once },
		{ "trigger_push", (void *)trigger_push },
		{ "trigger_relay", (void *)trigger_relay },
		{ "trigger_teleport", (void *)trigger_teleport },
		{ "trigger_transition", (void *)trigger_transition },
		{ "weapon_ak47", (void *)weapon_ak47 },
		{ "weapon_ak47l", (void *)weapon_ak47l },
		{ "weapon_as50", (void *)weapon_as50 },
		{ "weapon_aug", (void *)weapon_aug },
		{ "weapon_awp", (void *)weapon_awp },
		{ "weapon_c4", (void *)weapon_c4 },
		{ "weapon_cannon", (void *)weapon_cannon },
		{ "weapon_deagle", (void *)weapon_deagle },
		{ "weapon_deagled", (void *)weapon_deagled },
		{ "weapon_elite", (void *)weapon_elite },
		{ "weapon_famas", (void *)weapon_famas },
		{ "weapon_fiveseven", (void *)weapon_fiveseven },
		{ "weapon_flashbang", (void *)weapon_flashbang },
		{ "weapon_g3sg1", (void *)weapon_g3sg1 },
		{ "weapon_galil", (void *)weapon_galil },
		{ "weapon_glock18", (void *)weapon_glock18 },
		{ "weapon_gungnir", (void *)weapon_gungnir },
		{ "weapon_hegrenade", (void *)weapon_hegrenade },
		{ "weapon_infinity", (void *)weapon_infinity },
		{ "weapon_infinityex1", (void *)weapon_infinityex1 },
		{ "weapon_infinityex2", (void *)weapon_infinityex2 },
		{ "weapon_infinitysb", (void *)weapon_infinitysb },
		{ "weapon_infinitysr", (void *)weapon_infinitysr },
		{ "weapon_infinityss", (void *)weapon_infinityss },
		{ "weapon_k1a", (void *)weapon_k1a },
		{ "weapon_knife", (void *)weapon_knife },
		{ "weapon_kriss", (void *)weapon_kriss },
		{ "weapon_m14ebr", (void *)weapon_m14ebr },
		{ "weapon_m1887", (void *)weapon_m1887 },
		{ "weapon_m249", (void *)weapon_m249 },
		{ "weapon_m3", (void *)weapon_m3 },
		{ "weapon_m4a1", (void *)weapon_m4a1 },
		{ "weapon_m95", (void *)weapon_m95 },
		{ "weapon_mac10", (void *)weapon_mac10 },
		{ "weapon_mg3", (void *)weapon_mg3 },
		{ "weapon_mp5navy", (void *)weapon_mp5navy },
		{ "weapon_mp7a1c", (void *)weapon_mp7a1c },
		{ "weapon_mp7a1d", (void *)weapon_mp7a1d },
		{ "weapon_mp7a1p", (void *)weapon_mp7a1p },
		{ "weapon_p228", (void *)weapon_p228 },
		{ "weapon_p90", (void *)weapon_p90 },
		{ "weapon_scarh", (void *)weapon_scarh },
		{ "weapon_scarl", (void *)weapon_scarl },
		{ "weapon_scout", (void *)weapon_scout },
		{ "weapon_sg550", (void *)weapon_sg550 },
		{ "weapon_sg552", (void *)weapon_sg552 },
		{ "weapon_smokegrenade", (void *)weapon_smokegrenade },
		{ "weapon_tar21", (void *)weapon_tar21 },
		{ "weapon_thompson", (void *)weapon_thompson },
		{ "weapon_tmp", (void *)weapon_tmp },
		{ "weapon_ump45", (void *)weapon_ump45 },
		{ "weapon_usp", (void *)weapon_usp },
		{ "weapon_wa2000", (void *)weapon_wa2000 },
		{ "weapon_xm1014", (void *)weapon_xm1014 },
		{ "weapon_xm8c", (void *)weapon_xm8c },
		{ "weapon_xm8s", (void *)weapon_xm8s },
		{ "weaponbox", (void *)weaponbox },
		{ "world_items", (void *)world_items },
		{ "worldspawn", (void *)worldspawn },
		{ "zbsbreak", (void *)zbsbreak },
		{ "zombiespawn", (void *)zombiespawn },
		{ nullptr, nullptr }
};

extern "C" int dll_register( const char *name, dllexport_t *exports );

extern "C" int switch_installdll_server( void )
{
	return dll_register( "server", switch_server_exports );
}
#endif
