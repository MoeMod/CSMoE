
#include "extdll.h"
#include "util.h"
#include "game.h"
#include "globals.h"

namespace sv {

/*
* Globals initialization
*/
cvar_t *g_psv_gravity = NULL;
cvar_t *g_psv_aim = NULL;
cvar_t *g_footsteps = NULL;
cvar_t *g_psv_accelerate = NULL;
cvar_t *g_psv_friction = NULL;
cvar_t *g_psv_stopspeed = NULL;
cvar_t displaysoundlist = {"displaysoundlist", "0", 0, 0.0f, NULL};
cvar_t timelimit = {"mp_timelimit", "0", FCVAR_SERVER, 0.0f, NULL};
cvar_t flashlight = {"mp_flashlight", "0", FCVAR_SERVER, 0.0f, NULL};
cvar_t decalfrequency = {"decalfrequency", "30", FCVAR_SERVER, 0.0f, NULL};
cvar_t fadetoblack = {"mp_fadetoblack", "0", FCVAR_SERVER, 0.0f, NULL};
cvar_t fragsleft = {"mp_fragsleft", "0", FCVAR_SERVER | FCVAR_UNLOGGED, 0.0f, NULL};
cvar_t timeleft = {"mp_timeleft", "0", FCVAR_SERVER | FCVAR_UNLOGGED, 0.0f, NULL};
cvar_t friendlyfire = {"mp_friendlyfire", "0", FCVAR_SERVER, 0.0f, NULL};
cvar_t allowmonsters = {"mp_allowmonsters", "0", FCVAR_SERVER, 0.0f, NULL};
cvar_t roundtime = {"mp_roundtime", "3", FCVAR_SERVER, 0.0f, NULL};
cvar_t buytime = {"mp_buytime", "1.5", FCVAR_SERVER, 0.0f, NULL};
cvar_t freezetime = {"mp_freezetime", "3", FCVAR_SERVER, 0.0f, NULL};
cvar_t csgoinspect = { "mp_csgoinspect", "0", FCVAR_SERVER, 0.0f, NULL };
cvar_t csgosilencedwpn = { "mp_csgosilencedwpn", "0", FCVAR_SERVER, 0.0f, NULL };
cvar_t csgospecialeffect = { "mp_csgospecialeffect", "0", FCVAR_SERVER, 0.0f, NULL };
cvar_t csgobackweapon = { "mp_csgobackweapon", "0", FCVAR_SERVER, 0.0f, NULL };
cvar_t c4timer = {"mp_c4timer", "45", FCVAR_SERVER, 0.0f, NULL};
cvar_t ghostfrequency = {"mp_ghostfrequency", "0.1", FCVAR_SERVER, 0.0f, NULL};
cvar_t autokick = {"mp_autokick", "1", FCVAR_SERVER, 0.0f, NULL};
cvar_t autokick_timeout = {"mp_autokick_timeout", "-1", FCVAR_SERVER, 0.0f, NULL};
cvar_t restartround = {"sv_restartround", "0", FCVAR_SERVER, 0.0f, NULL};
cvar_t sv_restart = {"sv_restart", "0", FCVAR_SERVER, 0.0f, NULL};
cvar_t limitteams = {"mp_limitteams", "2", FCVAR_SERVER, 0.0f, NULL};
cvar_t autoteambalance = {"mp_autoteambalance", "1", FCVAR_SERVER, 0.0f, NULL};
cvar_t tkpunish = {"mp_tkpunish", "0", FCVAR_SERVER, 0.0f, NULL};
cvar_t hostagepenalty = {"mp_hostagepenalty", "13", FCVAR_SERVER, 0.0f, NULL};
cvar_t mirrordamage = {"mp_mirrordamage", "0", FCVAR_SERVER, 0.0f, NULL};
cvar_t logmessages = {"mp_logmessages", "1", FCVAR_SERVER, 0.0f, NULL};
cvar_t forcecamera = {"mp_forcecamera", "0", FCVAR_SERVER, 0.0f, NULL};
cvar_t forcechasecam = {"mp_forcechasecam", "0", FCVAR_SERVER, 0.0f, NULL};
cvar_t mapvoteratio = {"mp_mapvoteratio", "0.66", FCVAR_SERVER, 0.0f, NULL};
cvar_t logdetail = {"mp_logdetail", "0", FCVAR_SERVER, 0.0f, NULL};
cvar_t startmoney = {"mp_startmoney", "800", FCVAR_SERVER, 0.0f, NULL};
cvar_t maxrounds = {"mp_maxrounds", "13", FCVAR_SERVER, 0.0f, NULL};
cvar_t winlimit = {"mp_winlimit", "0", FCVAR_SERVER, 0.0f, NULL};
cvar_t windifference = {"mp_windifference", "1", FCVAR_SERVER, 0.0f, NULL};
cvar_t playerid = {"mp_playerid", "0", FCVAR_SERVER, 0.0f, NULL};
cvar_t allow_spectators = {"allow_spectators", "1.0", FCVAR_SERVER, 0.0f, NULL};
cvar_t mp_chattime = {"mp_chattime", "10", FCVAR_SERVER, 0.0f, NULL};
cvar_t kick_percent = {"mp_kickpercent", "0.66", FCVAR_SERVER, 0.0f, NULL};
cvar_t humans_join_team = {"humans_join_team", "any", FCVAR_SERVER, 0.0f, NULL};
cvar_t sk_plr_9mm_bullet1 = {"sk_plr_9mm_bullet1", "0", 0, 0.0f, NULL};
cvar_t sk_plr_9mm_bullet2 = {"sk_plr_9mm_bullet2", "0", 0, 0.0f, NULL};
cvar_t sk_plr_9mm_bullet3 = {"sk_plr_9mm_bullet3", "0", 0, 0.0f, NULL};
cvar_t sk_plr_357_bullet1 = {"sk_plr_357_bullet1", "0", 0, 0.0f, NULL};
cvar_t sk_plr_357_bullet2 = {"sk_plr_357_bullet2", "0", 0, 0.0f, NULL};
cvar_t sk_plr_357_bullet3 = {"sk_plr_357_bullet3", "0", 0, 0.0f, NULL};
cvar_t sk_plr_9mmAR_bullet1 = {"sk_plr_9mmAR_bullet1", "0", 0, 0.0f, NULL};
cvar_t sk_plr_9mmAR_bullet2 = {"sk_plr_9mmAR_bullet2", "0", 0, 0.0f, NULL};
cvar_t sk_plr_9mmAR_bullet3 = {"sk_plr_9mmAR_bullet3", "0", 0, 0.0f, NULL};
cvar_t sk_plr_9mmAR_grenade1 = {"sk_plr_9mmAR_grenade1", "0", 0, 0.0f, NULL};
cvar_t sk_plr_9mmAR_grenade2 = {"sk_plr_9mmAR_grenade2", "0", 0, 0.0f, NULL};
cvar_t sk_plr_9mmAR_grenade3 = {"sk_plr_9mmAR_grenade3", "0", 0, 0.0f, NULL};
cvar_t sk_plr_buckshot1 = {"sk_plr_buckshot1", "0", 0, 0.0f, NULL};
cvar_t sk_plr_buckshot2 = {"sk_plr_buckshot2", "0", 0, 0.0f, NULL};
cvar_t sk_plr_buckshot3 = {"sk_plr_buckshot3", "0", 0, 0.0f, NULL};
cvar_t sk_plr_rpg1 = {"sk_plr_rpg1", "0", 0, 0.0f, NULL};
cvar_t sk_plr_rpg2 = {"sk_plr_rpg2", "0", 0, 0.0f, NULL};
cvar_t sk_plr_rpg3 = {"sk_plr_rpg3", "0", 0, 0.0f, NULL};
cvar_t sk_12mm_bullet1 = {"sk_12mm_bullet1", "0", 0, 0.0f, NULL};
cvar_t sk_12mm_bullet2 = {"sk_12mm_bullet2", "0", 0, 0.0f, NULL};
cvar_t sk_12mm_bullet3 = {"sk_12mm_bullet3", "0", 0, 0.0f, NULL};
cvar_t sk_9mmAR_bullet1 = {"sk_9mmAR_bullet1", "0", 0, 0.0f, NULL};
cvar_t sk_9mmAR_bullet2 = {"sk_9mmAR_bullet2", "0", 0, 0.0f, NULL};
cvar_t sk_9mmAR_bullet3 = {"sk_9mmAR_bullet3", "0", 0, 0.0f, NULL};
cvar_t sk_9mm_bullet1 = {"sk_9mm_bullet1", "0", 0, 0.0f, NULL};
cvar_t sk_9mm_bullet2 = {"sk_9mm_bullet2", "0", 0, 0.0f, NULL};
cvar_t sk_9mm_bullet3 = {"sk_9mm_bullet3", "0", 0, 0.0f, NULL};
cvar_t sk_suitcharger1 = {"sk_suitcharger1", "0", 0, 0.0f, NULL};
cvar_t sk_suitcharger2 = {"sk_suitcharger2", "0", 0, 0.0f, NULL};
cvar_t sk_suitcharger3 = {"sk_suitcharger3", "0", 0, 0.0f, NULL};
cvar_t sk_battery1 = {"sk_battery1", "0", 0, 0.0f, NULL};
cvar_t sk_battery2 = {"sk_battery2", "0", 0, 0.0f, NULL};
cvar_t sk_battery3 = {"sk_battery3", "0", 0, 0.0f, NULL};
cvar_t sk_healthcharger1 = {"sk_healthcharger1", "0", 0, 0.0f, NULL};
cvar_t sk_healthcharger2 = {"sk_healthcharger2", "0", 0, 0.0f, NULL};
cvar_t sk_healthcharger3 = {"sk_healthcharger3", "0", 0, 0.0f, NULL};
cvar_t sk_healthkit1 = {"sk_healthkit1", "0", 0, 0.0f, NULL};
cvar_t sk_healthkit2 = {"sk_healthkit2", "0", 0, 0.0f, NULL};
cvar_t sk_healthkit3 = {"sk_healthkit3", "0", 0, 0.0f, NULL};
cvar_t sk_scientist_heal1 = {"sk_scientist_heal1", "0", 0, 0.0f, NULL};
cvar_t sk_scientist_heal2 = {"sk_scientist_heal2", "0", 0, 0.0f, NULL};
cvar_t sk_scientist_heal3 = {"sk_scientist_heal3", "0", 0, 0.0f, NULL};

cvar_t gamemode = {"mp_gamemode", "none", FCVAR_SERVER, 0.0f, NULL};
cvar_t maxkills = {"mp_maxkills", "150", FCVAR_SERVER, 0.0f, NULL};

extern void Bot_RegisterCvars();
extern void Tutor_RegisterCVars();
extern void Hostage_RegisterCVars();

void EXT_FUNC GameDLLInit()
{
	//g_bIsCzeroGame = UTIL_IsGame("czero");
	g_bIsCzeroGame = true;
	g_bEnableCSBot = g_bIsCzeroGame || ENG_CHECK_PARM(const_cast<char *>("-bots"), nullptr) != 0;

	g_psv_gravity = CVAR_GET_POINTER("sv_gravity");
	g_psv_aim = CVAR_GET_POINTER("sv_aim");
	g_footsteps = CVAR_GET_POINTER("mp_footsteps");
	g_psv_accelerate = CVAR_GET_POINTER("sv_accelerate");
	g_psv_friction = CVAR_GET_POINTER("sv_friction");
	g_psv_stopspeed = CVAR_GET_POINTER("sv_stopspeed");

	CVAR_REGISTER(&displaysoundlist);
	CVAR_REGISTER(&timelimit);
	CVAR_REGISTER(&friendlyfire);
	CVAR_REGISTER(&flashlight);
	CVAR_REGISTER(&decalfrequency);
	CVAR_REGISTER(&allowmonsters);
	CVAR_REGISTER(&roundtime);
	CVAR_REGISTER(&buytime);
	CVAR_REGISTER(&freezetime);
	CVAR_REGISTER(&csgoinspect);
	CVAR_REGISTER(&csgospecialeffect);
	CVAR_REGISTER(&csgosilencedwpn);
	CVAR_REGISTER(&csgobackweapon);
	CVAR_REGISTER(&c4timer);
	CVAR_REGISTER(&ghostfrequency);
	CVAR_REGISTER(&autokick);
	CVAR_REGISTER(&autokick_timeout);
	CVAR_REGISTER(&restartround);
	CVAR_REGISTER(&sv_restart);
	CVAR_REGISTER(&limitteams);
	CVAR_REGISTER(&autoteambalance);
	CVAR_REGISTER(&tkpunish);
	CVAR_REGISTER(&hostagepenalty);
	CVAR_REGISTER(&mirrordamage);
	CVAR_REGISTER(&logmessages);
	CVAR_REGISTER(&forcecamera);
	CVAR_REGISTER(&forcechasecam);
	CVAR_REGISTER(&mapvoteratio);
	CVAR_REGISTER(&maxrounds);
	CVAR_REGISTER(&winlimit);
	CVAR_REGISTER(&windifference);
	CVAR_REGISTER(&fadetoblack);
	CVAR_REGISTER(&logdetail);
	CVAR_REGISTER(&startmoney);
	CVAR_REGISTER(&playerid);
	CVAR_REGISTER(&allow_spectators);
	CVAR_REGISTER(&mp_chattime);
	CVAR_REGISTER(&kick_percent);
	CVAR_REGISTER(&fragsleft);
	CVAR_REGISTER(&timeleft);
	CVAR_REGISTER(&humans_join_team);

	CVAR_REGISTER(&gamemode);
	CVAR_REGISTER(&maxkills);

	Bot_RegisterCvars();
	Tutor_RegisterCVars();
	Hostage_RegisterCVars();
}

}
