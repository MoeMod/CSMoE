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
void monster_flyer( entvars_t *pev );
void monster_flyer_flock( entvars_t *pev );
void monster_alien_grunt( entvars_t *pev );
void item_airtank( entvars_t *pev );
void monster_apache( entvars_t *pev );
void hvr_rocket( entvars_t *pev );
void monster_barnacle( entvars_t *pev );
void monster_barney( entvars_t *pev );
void monster_barney_dead( entvars_t *pev );
void info_bigmomma( entvars_t *pev );
void bmortar( entvars_t *pev );
void monster_bigmomma( entvars_t *pev );
void monster_bloater( entvars_t *pev );
void func_wall( entvars_t *pev );
void func_wall_toggle( entvars_t *pev );
void func_conveyor( entvars_t *pev );
void func_illusionary( entvars_t *pev );
void func_monsterclip( entvars_t *pev );
void func_rotating( entvars_t *pev );
void func_pendulum( entvars_t *pev );
void squidspit( entvars_t *pev );
void monster_bullchicken( entvars_t *pev );
void env_global( entvars_t *pev );
void multisource( entvars_t *pev );
void func_button( entvars_t *pev );
void func_rot_button( entvars_t *pev );
void momentary_rot_button( entvars_t *pev );
void env_spark( entvars_t *pev );
void env_debris( entvars_t *pev );
void button_target( entvars_t *pev );
void monster_alien_controller( entvars_t *pev );
void controller_head_ball( entvars_t *pev );
void controller_energy_ball( entvars_t *pev );
void crossbow_bolt( entvars_t *pev );
void weapon_crossbow( entvars_t *pev );
void ammo_crossbow( entvars_t *pev );
void weapon_crowbar( entvars_t *pev );
void func_door( entvars_t *pev );
void func_water( entvars_t *pev );
void func_door_rotating( entvars_t *pev );
void momentary_door( entvars_t *pev );
void info_target( entvars_t *pev );
void env_bubbles( entvars_t *pev );
void beam( entvars_t *pev );
void env_lightning( entvars_t *pev );
void env_beam( entvars_t *pev );
// void trip_beam( entvars_t *pev );
void env_laser( entvars_t *pev );
void env_glow( entvars_t *pev );
void env_sprite( entvars_t *pev );
void gibshooter( entvars_t *pev );
void env_shooter( entvars_t *pev );
void test_effect( entvars_t *pev );
void env_blood( entvars_t *pev );
void env_shake( entvars_t *pev );
void env_fade( entvars_t *pev );
void env_message( entvars_t *pev );
void env_funnel( entvars_t *pev );
void env_beverage( entvars_t *pev );
void item_sodacan( entvars_t *pev );
void weapon_egon( entvars_t *pev );
void ammo_egonclip( entvars_t *pev );
void spark_shower( entvars_t *pev );
void env_explosion( entvars_t *pev );
void func_breakable( entvars_t *pev );
void func_pushable( entvars_t *pev );
void func_tank( entvars_t *pev );
void func_tanklaser( entvars_t *pev );
void func_tankrocket( entvars_t *pev );
void func_tankmortar( entvars_t *pev );
void func_tankcontrols( entvars_t *pev );
void streak_spiral( entvars_t *pev );
void garg_stomp( entvars_t *pev );
void monster_gargantua( entvars_t *pev );
void env_smoker( entvars_t *pev );
void weapon_gauss( entvars_t *pev );
void ammo_gaussclip( entvars_t *pev );
void monster_generic( entvars_t *pev );
void grenade( entvars_t *pev );
void weapon_glock( entvars_t *pev );
void weapon_9mmhandgun( entvars_t *pev );
void ammo_glockclip( entvars_t *pev );
void ammo_9mmclip( entvars_t *pev );
void monster_gman( entvars_t *pev );
void weapon_handgrenade( entvars_t *pev );
void monster_human_assassin( entvars_t *pev );
void monster_headcrab( entvars_t *pev );
void monster_babycrab( entvars_t *pev );
void item_healthkit( entvars_t *pev );
void func_healthcharger( entvars_t *pev );
void monster_human_grunt( entvars_t *pev );
void monster_grunt_repel( entvars_t *pev );
void monster_hgrunt_dead( entvars_t *pev );
void hornet( entvars_t *pev );
void weapon_hornetgun( entvars_t *pev );
void monster_houndeye( entvars_t *pev );
void func_recharge( entvars_t *pev );
void monster_cine_scientist( entvars_t *pev );
void monster_cine_panther( entvars_t *pev );
void monster_cine_barney( entvars_t *pev );
void monster_cine2_scientist( entvars_t *pev );
void monster_cine2_hvyweapons( entvars_t *pev );
void monster_cine2_slave( entvars_t *pev );
void monster_cine3_scientist( entvars_t *pev );
void monster_cine3_barney( entvars_t *pev );
void cine_blood( entvars_t *pev );
void cycler( entvars_t *pev );
void cycler_prdroid( entvars_t *pev );
void cycler_sprite( entvars_t *pev );
void cycler_weapon( entvars_t *pev );
void cycler_wreckage( entvars_t *pev );
void monster_ichthyosaur( entvars_t *pev );
void monster_alien_slave( entvars_t *pev );
void monster_vortigaunt( entvars_t *pev );
void world_items( entvars_t *pev );
void item_suit( entvars_t *pev );
void item_battery( entvars_t *pev );
void item_antidote( entvars_t *pev );
void item_security( entvars_t *pev );
void item_longjump( entvars_t *pev );
void monster_leech( entvars_t *pev );
void light( entvars_t *pev );
void light_spot( entvars_t *pev );
void light_environment( entvars_t *pev );
void game_score( entvars_t *pev );
void game_end( entvars_t *pev );
void game_text( entvars_t *pev );
void game_team_master( entvars_t *pev );
void game_team_set( entvars_t *pev );
void game_zone_player( entvars_t *pev );
void game_player_hurt( entvars_t *pev );
void game_counter( entvars_t *pev );
void game_counter_set( entvars_t *pev );
void game_player_equip( entvars_t *pev );
void game_player_team( entvars_t *pev );
void monstermaker( entvars_t *pev );
void func_mortar_field( entvars_t *pev );
void monster_mortar( entvars_t *pev );
void weapon_mp5( entvars_t *pev );
void weapon_9mmAR( entvars_t *pev );
void ammo_mp5clip( entvars_t *pev );
void ammo_9mmAR( entvars_t *pev );
void ammo_9mmbox( entvars_t *pev );
void ammo_mp5grenades( entvars_t *pev );
void ammo_ARgrenades( entvars_t *pev );
void monster_nihilanth( entvars_t *pev );
void nihilanth_energy_ball( entvars_t *pev );
void info_node( entvars_t *pev );
void info_node_air( entvars_t *pev );
void testhull( entvars_t *pev );
void node_viewer( entvars_t *pev );
void node_viewer_human( entvars_t *pev );
void node_viewer_fly( entvars_t *pev );
void node_viewer_large( entvars_t *pev );
void monster_osprey( entvars_t *pev );
void path_corner( entvars_t *pev );
void path_track( entvars_t *pev );
void func_plat( entvars_t *pev );
void func_platrot( entvars_t *pev );
void func_train( entvars_t *pev );
void func_tracktrain( entvars_t *pev );
void func_traincontrols( entvars_t *pev );
void func_trackchange( entvars_t *pev );
void func_trackautochange( entvars_t *pev );
void func_guntarget( entvars_t *pev );
void player( entvars_t *pev );
void monster_hevsuit_dead( entvars_t *pev );
void player_weaponstrip( entvars_t *pev );
void player_loadsaved( entvars_t *pev );
void info_intermission( entvars_t *pev );
void monster_player( entvars_t *pev );
// void prop( entvars_t *pev );
void weapon_python( entvars_t *pev );
void weapon_357( entvars_t *pev );
void ammo_357( entvars_t *pev );
void monster_rat( entvars_t *pev );
void monster_cockroach( entvars_t *pev );
void weapon_rpg( entvars_t *pev );
void laser_spot( entvars_t *pev );
void rpg_rocket( entvars_t *pev );
void ammo_rpgclip( entvars_t *pev );
void monster_satchel( entvars_t *pev );
void weapon_satchel( entvars_t *pev );
void monster_scientist( entvars_t *pev );
void monster_scientist_dead( entvars_t *pev );
void monster_sitting_scientist( entvars_t *pev );
void scripted_sequence( entvars_t *pev );
void aiscripted_sequence( entvars_t *pev );
void scripted_sentence( entvars_t *pev );
void monster_furniture( entvars_t *pev );
void weapon_shotgun( entvars_t *pev );
void ammo_buckshot( entvars_t *pev );
void ambient_generic( entvars_t *pev );
void env_sound( entvars_t *pev );
void speaker( entvars_t *pev );
void soundent( entvars_t *pev );
void monster_snark( entvars_t *pev );
void weapon_snark( entvars_t *pev );
void info_null( entvars_t *pev );
void info_player_deathmatch( entvars_t *pev );
void info_player_start( entvars_t *pev );
void info_landmark( entvars_t *pev );
void DelayedUse( entvars_t *pev );
// void my_monster( entvars_t *pev );
void monster_tentacle( entvars_t *pev );
void monster_tentaclemaw( entvars_t *pev );
void func_friction( entvars_t *pev );
void trigger_auto( entvars_t *pev );
void trigger_relay( entvars_t *pev );
void multi_manager( entvars_t *pev );
void env_render( entvars_t *pev );
void trigger( entvars_t *pev );
void trigger_hurt( entvars_t *pev );
void trigger_monsterjump( entvars_t *pev );
void trigger_cdaudio( entvars_t *pev );
void target_cdaudio( entvars_t *pev );
void trigger_multiple( entvars_t *pev );
void trigger_once( entvars_t *pev );
void trigger_counter( entvars_t *pev );
void trigger_transition( entvars_t *pev );
void fireanddie( entvars_t *pev );
void trigger_changelevel( entvars_t *pev );
void func_ladder( entvars_t *pev );
void trigger_push( entvars_t *pev );
void trigger_teleport( entvars_t *pev );
void info_teleport_destination( entvars_t *pev );
void trigger_autosave( entvars_t *pev );
void trigger_endsection( entvars_t *pev );
void trigger_gravity( entvars_t *pev );
void trigger_changetarget( entvars_t *pev );
void trigger_camera( entvars_t *pev );
void monster_tripmine( entvars_t *pev );
void weapon_tripmine( entvars_t *pev );
void monster_turret( entvars_t *pev );
void monster_miniturret( entvars_t *pev );
void monster_sentry( entvars_t *pev );
void weaponbox( entvars_t *pev );
void infodecal( entvars_t *pev );
void bodyque( entvars_t *pev );
void worldspawn( entvars_t *pev );
void xen_plantlight( entvars_t *pev );
void xen_hair( entvars_t *pev );
void xen_ttrigger( entvars_t *pev );
void xen_tree( entvars_t *pev );
void xen_spore_small( entvars_t *pev );
void xen_spore_medium( entvars_t *pev );
void xen_spore_large( entvars_t *pev );
void xen_hull( entvars_t *pev );
void monster_zombie( entvars_t *pev );
}

static dllexport_t switch_server_exports[] = {
		{ "GiveFnptrsToDll", (void*)GiveFnptrsToDll },
		{ "GetEntityAPI", (void*)GetEntityAPI },
		{ "GetEntityAPI2", (void*)GetEntityAPI2 },
		{ "GetNewDLLFunctions", (void*)GetNewDLLFunctions },
		{ "Server_GetPhysicsInterface", (void*)Server_GetPhysicsInterface },
};

extern "C" int dll_register( const char *name, dllexport_t *exports );

extern "C" int switch_installdll_server( void )
{
	return dll_register( "server", switch_server_exports );
}
#endif
