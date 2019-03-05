#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "nodes.h"
#include "soundent.h"
#include "decals.h"
#include "skill.h"
#include "effects.h"
#include "player.h"
#include "client.h"
#include "weapons.h"
#include "gamerules.h"
#include "globals.h"
#include "sound.h"
#include "bmodels.h"
#include "world.h"
#include "game.h"
#include "gamemode/mods.h"
#include "player/csdm_randomspawn.h"

edict_t *g_pBodyQueueHead;
CGlobalState gGlobalState;
float g_flWeaponCheat;

extern void BotPrecache(); // bot_utils.cpp

/*
* Globals initialization
*/
DLL_DECALLIST gDecals[] =
{
	{ "{shot1", 0 },		// DECAL_GUNSHOT1
	{ "{shot2", 0 },		// DECAL_GUNSHOT2
	{ "{shot3", 0 },		// DECAL_GUNSHOT3
	{ "{shot4", 0 },		// DECAL_GUNSHOT4
	{ "{shot5", 0 },		// DECAL_GUNSHOT5
	{ "{lambda01", 0 },		// DECAL_LAMBDA1
	{ "{lambda02", 0 },		// DECAL_LAMBDA2
	{ "{lambda03", 0 },		// DECAL_LAMBDA3
	{ "{lambda04", 0 },		// DECAL_LAMBDA4
	{ "{lambda05", 0 },		// DECAL_LAMBDA5
	{ "{lambda06", 0 },		// DECAL_LAMBDA6
	{ "{scorch1", 0 },		// DECAL_SCORCH1
	{ "{scorch2", 0 },		// DECAL_SCORCH2
	{ "{blood1", 0 },		// DECAL_BLOOD1
	{ "{blood2", 0 },		// DECAL_BLOOD2
	{ "{blood3", 0 },		// DECAL_BLOOD3
	{ "{blood4", 0 },		// DECAL_BLOOD4
	{ "{blood5", 0 },		// DECAL_BLOOD5
	{ "{blood6", 0 },		// DECAL_BLOOD6
	{ "{yblood1", 0 },		// DECAL_YBLOOD1
	{ "{yblood2", 0 },		// DECAL_YBLOOD2
	{ "{yblood3", 0 },		// DECAL_YBLOOD3
	{ "{yblood4", 0 },		// DECAL_YBLOOD4
	{ "{yblood5", 0 },		// DECAL_YBLOOD5
	{ "{yblood6", 0 },		// DECAL_YBLOOD6
	{ "{break1", 0 },		// DECAL_GLASSBREAK1
	{ "{break2", 0 },		// DECAL_GLASSBREAK2
	{ "{break3", 0 },		// DECAL_GLASSBREAK3
	{ "{bigshot1", 0 },		// DECAL_BIGSHOT1
	{ "{bigshot2", 0 },		// DECAL_BIGSHOT2
	{ "{bigshot3", 0 },		// DECAL_BIGSHOT3
	{ "{bigshot4", 0 },		// DECAL_BIGSHOT4
	{ "{bigshot5", 0 },		// DECAL_BIGSHOT5
	{ "{spit1", 0 },		// DECAL_SPIT1
	{ "{spit2", 0 },		// DECAL_SPIT2
	{ "{bproof1", 0 },		// DECAL_BPROOF1
	{ "{gargstomp", 0 },		// DECAL_GARGSTOMP1,		// Gargantua stomp crack
	{ "{smscorch1", 0 },		// DECAL_SMALLSCORCH1,		// Small scorch mark
	{ "{smscorch2", 0 },		// DECAL_SMALLSCORCH2,		// Small scorch mark
	{ "{smscorch3", 0 },		// DECAL_SMALLSCORCH3,		// Small scorch mark
	{ "{mommablob", 0 },		// DECAL_MOMMABIRTH		// BM Birth spray
	{ "{mommablob", 0 },		// DECAL_MOMMASPLAT		// BM Mortar spray?? need decal*/
};

TYPEDESCRIPTION CGlobalState::m_SaveData[] =
{
	DEFINE_FIELD(CGlobalState, m_listCount, FIELD_INTEGER)
};

TYPEDESCRIPTION gGlobalEntitySaveData[] =
{
	DEFINE_ARRAY(globalentity_t, name, FIELD_CHARACTER, 64),
	DEFINE_ARRAY(globalentity_t, levelName, FIELD_CHARACTER, 32),
	DEFINE_FIELD(globalentity_t, state, FIELD_INTEGER)
};

class CDecal: public CBaseEntity
{
public:
	virtual void Spawn();
	virtual void KeyValue(KeyValueData *pkvd);

public:
	void EXPORT StaticDecal();
	void EXPORT TriggerDecal(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
};

LINK_ENTITY_TO_CLASS(infodecal, CDecal);

void CDecal::Spawn()
{
	if (pev->skin < 0 || (gpGlobals->deathmatch != 0.0f && (pev->spawnflags & SF_DECAL_NOTINDEATHMATCH)))
	{
		REMOVE_ENTITY(ENT(pev));
		return;
	}

	if (FStringNull(pev->targetname))
	{
		SetThink(&CDecal::StaticDecal);

		// if there's no targetname, the decal will spray itself on as soon as the world is done spawning.
		pev->nextthink = gpGlobals->time;
	}
	else
	{
		// if there IS a targetname, the decal sprays itself on when it is triggered.
		SetThink(&CBaseEntity::SUB_DoNothing);
		SetUse(&CDecal::TriggerDecal);
	}
}

void CDecal::TriggerDecal(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	// this is set up as a USE function for infodecals that have targetnames, so that the
	// decal doesn't get applied until it is fired. (usually by a scripted sequence)
	TraceResult trace;
	int entityIndex;

	UTIL_TraceLine(pev->origin - Vector(5, 5, 5), pev->origin + Vector(5, 5, 5), ignore_monsters, ENT(pev), &trace);

	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_BSPDECAL);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		WRITE_SHORT((int)pev->skin);
		entityIndex = (short)ENTINDEX(trace.pHit);
		WRITE_SHORT(entityIndex);
		if (entityIndex)
		{
			WRITE_SHORT((int)VARS(trace.pHit)->modelindex);
		}
	MESSAGE_END();

	SetThink(&CDecal::SUB_Remove);
	pev->nextthink = gpGlobals->time + 0.1f;
}

void CDecal::StaticDecal()
{
	TraceResult trace;
	int entityIndex, modelIndex;

	UTIL_TraceLine(pev->origin - Vector(5, 5, 5), pev->origin + Vector(5, 5, 5), ignore_monsters, ENT(pev), &trace);

	entityIndex = (short)ENTINDEX(trace.pHit);

	if (entityIndex)
		modelIndex = VARS(trace.pHit)->modelindex;
	else
		modelIndex = 0;

	STATIC_DECAL(pev->origin, pev->skin, entityIndex, modelIndex);
	SUB_Remove();
}

void CDecal::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "texture"))
	{
		pev->skin = DECAL_INDEX(pkvd->szValue);

		// Found
		if (pev->skin >= 0)
			return;

		ALERT(at_console, "Can't find decal %s\n", pkvd->szValue);
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

// Body queue class here.... It's really just CBaseEntity

class CCorpse: public CBaseEntity
{
public:
	virtual int ObjectCaps() { return FCAP_DONT_SAVE; }
};

LINK_ENTITY_TO_CLASS(bodyque, CCorpse);

static void InitBodyQue()
{
	g_pBodyQueueHead = NULL;
}

// make a body que entry for the given ent so the ent can be respawned elsewhere
// GLOBALS ASSUMED SET:  g_eoBodyQueueHeadstion

void CopyToBodyQue(entvars_t *pev)
{
#if 0
	if (pev->effects & EF_NODRAW)
		return;

	entvars_t *pevHead = VARS(g_pBodyQueueHead);

	pevHead->angles = pev->angles;
	pevHead->model = pev->model;
	pevHead->modelindex = pev->modelindex;
	pevHead->frame = pev->frame;
	pevHead->colormap = pev->colormap;
	pevHead->movetype = MOVETYPE_TOSS;
	pevHead->velocity = pev->velocity;
	pevHead->flags = 0;
	pevHead->deadflag = pev->deadflag;
	pevHead->renderfx = kRenderFxDeadPlayer;
	pevHead->renderamt = ENTINDEX(ENT(pev));

	pevHead->effects = pev->effects | EF_NOINTERP;

	pevHead->sequence = pev->sequence;
	pevHead->animtime = pev->animtime;

	UTIL_SetOrigin(pevHead, pev->origin);
	UTIL_SetSize(pevHead, pev->mins, pev->maxs);

	g_pBodyQueueHead = pevHead->owner;
#endif
}

void ClearBodyQue()
{
	;
}

CGlobalState::CGlobalState()
{
	Reset();
}

void CGlobalState::Reset()
{
	m_pList = NULL;
	m_listCount = 0;
}

globalentity_t *CGlobalState::Find(string_t globalname)
{
	if (!globalname)
		return NULL;

	globalentity_t *pTest = m_pList;
	const char *pEntityName = STRING(globalname);

	while (pTest != NULL)
	{
		if (!Q_strcmp(pEntityName, pTest->name))
			break;

		pTest = pTest->pNext;
	}

	return pTest;
}

// This is available all the time now on impulse 104, remove later

void CGlobalState::DumpGlobals()
{
	static const char *estates[] = { "Off", "On", "Dead" };
	globalentity_t *pTest;

	ALERT(at_console, "-- Globals --\n");
	pTest = m_pList;

	while (pTest != NULL)
	{
		ALERT(at_console, "%s: %s (%s)\n", pTest->name, pTest->levelName, estates[ pTest->state ]);
		pTest = pTest->pNext;
	}
}

void CGlobalState::EntityAdd(string_t globalname, string_t mapName, GLOBALESTATE state)
{
	assert(!Find(globalname));

	globalentity_t *pNewEntity = (globalentity_t *)calloc(sizeof(globalentity_t), 1);
	assert(pNewEntity != NULL);

	pNewEntity->pNext = m_pList;
	m_pList = pNewEntity;
	Q_strcpy(pNewEntity->name, STRING(globalname));
	Q_strcpy(pNewEntity->levelName, STRING(mapName));
	pNewEntity->state = state;

	m_listCount++;
}

void CGlobalState::EntitySetState(string_t globalname, GLOBALESTATE state)
{
	globalentity_t *pEnt = Find(globalname);

	if (pEnt)
	{
		pEnt->state = state;
	}
}

const globalentity_t *CGlobalState::EntityFromTable(string_t globalname)
{
	globalentity_t *pEnt = Find(globalname);

	return pEnt;
}

GLOBALESTATE CGlobalState::EntityGetState(string_t globalname)
{
	globalentity_t *pEnt = Find(globalname);

	if (pEnt != NULL)
	{
		return pEnt->state;
	}

	return GLOBAL_OFF;
}

int CGlobalState::Save(CSave &save)
{
	int i;
	globalentity_t *pEntity;

	if (!save.WriteFields("GLOBAL", this, m_SaveData, ARRAYSIZE(m_SaveData)))
	{
		return 0;
	}

	pEntity = m_pList;
	for (i = 0; i < m_listCount && pEntity != NULL; ++i)
	{
		if (!save.WriteFields("GENT", pEntity, gGlobalEntitySaveData, ARRAYSIZE(gGlobalEntitySaveData)))
		{
			return 0;
		}

		pEntity = pEntity->pNext;
	}

	return 1;
}

int CGlobalState::Restore(CRestore &restore)
{
	int i, listCount;
	globalentity_t tmpEntity;

	ClearStates();

	if (!restore.ReadFields("GLOBAL", this, m_SaveData, ARRAYSIZE(m_SaveData)))
	{
		return 0;
	}

	// Get new list count
	listCount = m_listCount;

	// Clear loaded data
	m_listCount = 0;

	for (i = 0; i < listCount; ++i)
	{
		if (!restore.ReadFields("GENT", &tmpEntity, gGlobalEntitySaveData, ARRAYSIZE(gGlobalEntitySaveData)))
		{
			return 0;
		}

		EntityAdd(MAKE_STRING(tmpEntity.name), MAKE_STRING(tmpEntity.levelName), tmpEntity.state);
	}

	return 1;
}

void CGlobalState::EntityUpdate(string_t globalname, string_t mapname)
{
	globalentity_t *pEnt = Find(globalname);

	if (pEnt != NULL)
	{
		Q_strcpy(pEnt->levelName, STRING(mapname));
	}
}

void CGlobalState::ClearStates()
{
	globalentity_t *pFree = m_pList;

	while (pFree != NULL)
	{
		globalentity_t *pNext = pFree->pNext;

		free(pFree);
		pFree = pNext;
	}

	Reset();
}

void EXT_FUNC SaveGlobalState(SAVERESTOREDATA *pSaveData)
{
	CSave saveHelper(pSaveData);
	gGlobalState.Save(saveHelper);
}

void EXT_FUNC RestoreGlobalState(SAVERESTOREDATA *pSaveData)
{
	CRestore restoreHelper(pSaveData);
	gGlobalState.Restore(restoreHelper);
}

void EXT_FUNC ResetGlobalState()
{
	gGlobalState.ClearStates();

	// Init the HUD on a new game / load game
	gInitHUD = TRUE;
}

LINK_ENTITY_TO_CLASS(worldspawn, CWorld);

void CWorld::Spawn()
{
	EmptyEntityHashTable();
	g_fGameOver = FALSE;

	Precache();

	g_flWeaponCheat = CVAR_GET_FLOAT("sv_cheats");
	g_szMapBriefingText[0] = '\0';

	int flength = 0;
	char *pFile = (char *)LOAD_FILE_FOR_ME(UTIL_VarArgs("maps/%s.txt", STRING(gpGlobals->mapname)), &flength);

	if (pFile && flength != 0.0f)
	{
		Q_strncpy(g_szMapBriefingText, pFile, ARRAYSIZE(g_szMapBriefingText) - 2);
		g_szMapBriefingText[ ARRAYSIZE(g_szMapBriefingText) - 2 ] = 0;
      
		PRECACHE_GENERIC(UTIL_VarArgs("maps/%s.txt", STRING(gpGlobals->mapname)));
		FREE_FILE(pFile);
	}
	else
	{
		pFile = (char *)LOAD_FILE_FOR_ME(UTIL_VarArgs("maps/default.txt"), &flength);

		if (pFile != NULL && flength)
		{
			Q_strncpy(g_szMapBriefingText, pFile, ARRAYSIZE(g_szMapBriefingText) - 2);
			g_szMapBriefingText[ ARRAYSIZE(g_szMapBriefingText) - 2 ] = 0;

			PRECACHE_GENERIC(UTIL_VarArgs("maps/default.txt"));
		}

		FREE_FILE(pFile);
	}
}

void CWorld::Precache()
{
	g_pLastSpawn = NULL;
	g_pLastCTSpawn = NULL;
	g_pLastTerroristSpawn = NULL;

	CVAR_SET_STRING("sv_gravity", "800");
	CVAR_SET_STRING("sv_maxspeed", "900");
	CVAR_SET_STRING("sv_stepsize", "18");

	// clear DSP
	CVAR_SET_STRING("room_type", "0");

	// Set up game rules
	if (g_pGameRules)
	{
		delete g_pGameRules; // CS16ND/ReGameDll : NOT virtual destructor ??? Fuck it.
	}
	g_pGameRules = (CHalfLifeMultiplay *)InstallGameRules();
	CSDM_LoadSpawnPoints();
	

	// UNDONE why is there so much Spawn code in the Precache function? I'll just keep it here

	// LATER - do we want a sound ent in deathmatch? (sjb)
	//pSoundEnt = CBaseEntity::Create("soundent", g_vecZero, g_vecZero, edict());
	pSoundEnt = CreateClassPtr<CSoundEnt>();
	pSoundEnt->Spawn();

	if (!pSoundEnt)
	{
		ALERT(at_console, "**COULD NOT CREATE SOUNDENT**\n");
	}

	InitBodyQue();

	// init sentence group playback stuff from sentences.txt.
	// ok to call this multiple times, calls after first are ignored.
	SENTENCEG_Init();

	// init texture type array from materials.txt
	TEXTURETYPE_Init();

	// the area based ambient sounds MUST be the first precache_sounds
	// player precaches
	// get weapon precaches

	W_Precache();
	ClientPrecache();
	BotPrecache();

	// sounds used from C physics code
	// clears sound channels
	PRECACHE_SOUND("common/null.wav");

	// temporary sound for respawning weapons.
	PRECACHE_SOUND("items/suitchargeok1.wav");

	// player picks up a gun.
	PRECACHE_SOUND("items/gunpickup2.wav");

	// dead bodies hitting the ground (animation events)
	PRECACHE_SOUND("common/bodydrop3.wav");
	PRECACHE_SOUND("common/bodydrop4.wav");

	g_Language = (int)CVAR_GET_FLOAT("sv_language");
	if (g_Language == LANGUAGE_GERMAN)
	{
		PRECACHE_MODEL("models/germangibs.mdl");
	}
	else
	{
		PRECACHE_MODEL("models/hgibs.mdl");
		PRECACHE_MODEL("models/agibs.mdl");
	}

	PRECACHE_SOUND("weapons/ric1.wav");
	PRECACHE_SOUND("weapons/ric2.wav");
	PRECACHE_SOUND("weapons/ric3.wav");
	PRECACHE_SOUND("weapons/ric4.wav");
	PRECACHE_SOUND("weapons/ric5.wav");

	PRECACHE_SOUND("weapons/ric_metal-1.wav");
	PRECACHE_SOUND("weapons/ric_metal-2.wav");
	PRECACHE_SOUND("weapons/ric_conc-1.wav");
	PRECACHE_SOUND("weapons/ric_conc-2.wav");

	// Setup light animation tables. 'a' is total darkness, 'z' is maxbright.

	// 0 normal
	LIGHT_STYLE(0, "m");

	// 1 FLICKER (first variety)
	LIGHT_STYLE(1, "mmnmmommommnonmmonqnmmo");

	// 2 SLOW STRONG PULSE
	LIGHT_STYLE(2, "abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba");

	// 3 CANDLE (first variety)
	LIGHT_STYLE(3, "mmmmmaaaaammmmmaaaaaabcdefgabcdefg");

	// 4 FAST STROBE
	LIGHT_STYLE(4, "mamamamamama");

	// 5 GENTLE PULSE 1
	LIGHT_STYLE(5, "jklmnopqrstuvwxyzyxwvutsrqponmlkj");

	// 6 FLICKER (second variety)
	LIGHT_STYLE(6, "nmonqnmomnmomomno");

	// 7 CANDLE (second variety)
	LIGHT_STYLE(7, "mmmaaaabcdefgmmmmaaaammmaamm");

	// 8 CANDLE (third variety)
	LIGHT_STYLE(8, "mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa");

	// 9 SLOW STROBE (fourth variety)
	LIGHT_STYLE(9, "aaaaaaaazzzzzzzz");

	// 10 FLUORESCENT FLICKER
	LIGHT_STYLE(10, "mmamammmmammamamaaamammma");

	// 11 SLOW PULSE NOT FADE TO BLACK
	LIGHT_STYLE(11, "abcdefghijklmnopqrrqponmlkjihgfedcba");

	// 12 UNDERWATER LIGHT MUTATION
	// this light only distorts the lightmap - no contribution
	// is made to the brightness of affected surfaces
	LIGHT_STYLE(12, "mmnnmmnnnmmnn");

	// styles 32-62 are assigned by the light program for switchable lights
	// 63 testing
	LIGHT_STYLE(63, "a");

	for (size_t i = 0; i < ARRAYSIZE(gDecals); ++i)
		gDecals[i].index = DECAL_INDEX(gDecals[i].name);

	// init the WorldGraph.
	WorldGraph.InitGraph();

	// make sure the .NOD file is newer than the .BSP file.
	if (!WorldGraph.CheckNODFile((char *)STRING(gpGlobals->mapname)))
	{
		// NOD file is not present, or is older than the BSP file.
		WorldGraph.AllocNodes();
	}
	else
	{
		// Load the node graph for this level
		if (!WorldGraph.FLoadGraph((char *)STRING(gpGlobals->mapname)))
		{
			// couldn't load, so alloc and prepare to build a graph.
			ALERT(at_console, "*Error opening .NOD file\n");
			WorldGraph.AllocNodes();
		}
		else
		{
			ALERT(at_console, "\n*Graph Loaded!\n");
		}
	}

	if (pev->speed > 0)
		CVAR_SET_FLOAT("sv_zmax", pev->speed);
	else
		CVAR_SET_FLOAT("sv_zmax", 4096);

	if (pev->netname)
	{
		ALERT(at_aiconsole, "Chapter title: %s\n", STRING(pev->netname));

		CBaseEntity *pEntity = CBaseEntity::Create("env_message", g_vecZero, g_vecZero, NULL);

		if (pEntity != NULL)
		{
			pEntity->SetThink(&CBaseEntity::SUB_CallUseToggle);
			pEntity->pev->message = pev->netname;
			pev->netname = 0;
			pEntity->pev->nextthink = gpGlobals->time + 0.3f;
			pEntity->pev->spawnflags = SF_MESSAGE_ONCE;
		}
	}

	if (pev->spawnflags & SF_WORLD_DARK)
		CVAR_SET_FLOAT("v_dark", 1);
	else
		CVAR_SET_FLOAT("v_dark", 0);

	if (pev->spawnflags & SF_WORLD_TITLE)
	{
		// display the game title if this key is set
		gDisplayTitle = TRUE;
	}
	else
		gDisplayTitle = FALSE;
}

void CWorld::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "skyname"))
	{
		CVAR_SET_STRING("sv_skyname", pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "sounds"))
	{
		gpGlobals->cdAudioTrack = Q_atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "WaveHeight"))
	{
		pev->scale = Q_atof(pkvd->szValue) * 0.125;
		CVAR_SET_FLOAT("sv_wateramp", pev->scale);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "MaxRange"))
	{
		pev->speed = Q_atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "chaptertitle"))
	{
		pev->netname = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "startdark"))
	{
		int flag = Q_atoi(pkvd->szValue);
		if (flag)
		{
			pev->spawnflags |= SF_WORLD_DARK;
		}
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "newunit"))
	{
		if (Q_atoi(pkvd->szValue))
			CVAR_SET_FLOAT("sv_newunit", 1.0);

		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "gametitle"))
	{
		if (Q_atoi(pkvd->szValue))
			pev->spawnflags |= SF_WORLD_TITLE;

		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "mapteams"))
	{
		pev->team = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "defaultteam"))
	{
		if (Q_atoi(pkvd->szValue))
			pev->spawnflags |= SF_WORLD_FORCETEAM;

		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}
