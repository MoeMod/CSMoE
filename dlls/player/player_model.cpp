#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "gamerules.h"
#include "player_model.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

struct AppearanceInfo
{
	bool isTRFemale;
	bool isCTFemale;
	const char* tr;
	const char* ct;
};

//static AppearanceInfo gPlayerAppearance[] =
//{
//	// t   ct
//	{NULL, NULL},
//	{false, true, "buffclass19s1tr", "buffclass19s1ct"},
//	{true, false, "buffclass19s2tr", "buffclass19s2ct"},
//	{true, true, "buffclass19s3tr", "buffclass19s3ct"},
//	{true, false, "buffclass19s4tr", "buffclass19s4ct"},
//	{true, false, "buffclass20s2tr", "buffclass20s2ct"},
//	{false, true, "buffclass20s3tr", "buffclass20s3ct"},
//	{true, true, "buffclass20s4tr", "buffclass20s4ct"},
//};

static AppearanceInfo gPlayerAppearance[] =
{
	// t   ct
	{NULL, NULL},
	{true, false, "yuri", "saf"},
	{false, true, "pirateboy", "choijiyoon"},
	{false, false, "marineboy", "fernando"},
	{true, false, "pirategirl", "707"},
	{false, false, "rb", "sozo"},
	{true, false, "jpngirl01", "magui"},
	{true, true, "ritsuka", "natasha"},
};

constexpr auto NUM_APPEARANCES = std::extent<decltype(gPlayerAppearance)>::value;

const char* PlayerModel_GetApperance(int iApperance, TeamName iTeam)
{
	if (iApperance >= NUM_APPEARANCES || iApperance < 1) {
		iApperance = 1;
	}

	return (iTeam == TERRORIST) ? gPlayerAppearance[iApperance].tr : gPlayerAppearance[iApperance].ct;
}

bool PlayerModel_IsFemale(int iApperance, TeamName iTeam)
{
	if (iApperance >= NUM_APPEARANCES || iApperance < 1) {
		iApperance = 1;
	}

	return (iTeam == TERRORIST) ? gPlayerAppearance[iApperance].isTRFemale : gPlayerAppearance[iApperance].isCTFemale;
}

ModelName PlayerModel_GetRandomSkin()
{
	return (ModelName)RANDOM_LONG(1, NUM_APPEARANCES-1);
}

int PlayerModel_GetNumSkins()
{
	return NUM_APPEARANCES;
}

void Client_ApperanceToModel(char* buffer, int iApperance, TeamName iTeam)
{
	auto model = iTeam == TERRORIST ? gPlayerAppearance[iApperance].tr : gPlayerAppearance[iApperance].ct;
	Q_sprintf(buffer, "models/player/%s/%s.mdl", model, model);
}

void PlayerModel_Precache()
{
	for (auto psz : sPlayerModelFiles)
		PRECACHE_MODEL(const_cast<char*>(psz));

	char path[128];
	for (int i = 1; i < NUM_APPEARANCES; i++)
	{
		Client_ApperanceToModel(path, i, TERRORIST);
		PRECACHE_MODEL(path);

		Client_ApperanceToModel(path, i, CT);
		PRECACHE_MODEL(path);
	}
}

void PlayerModel_ForceUnmodified(const Vector &vMin, const Vector &vMax)
{
	for (auto psz : sPlayerModelFiles)
		ENGINE_FORCE_UNMODIFIED(force_model_specifybounds, vMin, vMax, psz);
}

CPlayerClassManager& PlayerClassManager()
{
	static CPlayerClassManager x;
	return x;
}

static CPlayerClassManager::ClassData gPlayerClass[] =
{
	{ MODEL_UNASSIGNED, NULL, UNASSIGNED },
	{ MODEL_YURI, "yuri", TERRORIST, true, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_SAF, "saf", CT, false, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_PIRATEBOY, "pirateboy", TERRORIST, false, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_CHOIJIYOON, "choijiyoon", CT, true, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_MARINEBOY, "marineboy", TERRORIST, false, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_FERNANDO, "fernando", CT, false, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_PIRATEGIRL, "pirategirl", TERRORIST, true, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_707, "707", CT, false, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_RB, "rb", TERRORIST, false, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_SOZO, "sozo", CT, true, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_JPNGIRL01, "jpngirl01", TERRORIST, true, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_MAGUI, "magui", CT, false, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_RITSUKA, "ritsuka", TERRORIST, true ,SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_NATASHA, "natasha", CT, true, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_TERROR, "terror", TERRORIST, false, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_URBAN, "urban", CT, false, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_LEET, "leet", TERRORIST, false, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_GSG9, "gsg9", CT, false, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_ARCTIC, "arctic", TERRORIST, false, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_SAS, "sas", CT, false, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_GUERILLA, "guerilla", TERRORIST, false, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_GIGN, "gign", CT, false, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_MILITIA, "militia", TERRORIST, false, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_SPETSNAZ, "spetsnaz", CT, false, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_BUFFCLASSB, "buffclassb", TERRORIST, true, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_BUFFCLASSA, "buffclassa", CT, true, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_BUFFCLASSHUNTER, "buffclasshunter", TERRORIST, true, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_BUFFCLASSLYCAN, "buffclasslycan", CT, true, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_BUFFCLASSBLAIR, "buffclassblair", TERRORIST, true, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_BUFFCLASSFERNADO, "buffclassfernando", CT, false, SHOW_SPEED | SHOW_DAMAGE},
};

constexpr auto NUM_CLASS = std::extent<decltype(gPlayerClass)>::value;

void CPlayerClassManager::PlayerModel_Precache()
{
	char path[128];
	for (int i = 1; i < NUM_CLASS; i++)
	{
		Client_ApperanceToModel(path, i);
		PRECACHE_MODEL(path);
	}
}

const char* CPlayerClassManager::PlayerClass_GetModelName(int iApperance)
{
	if (iApperance >= NUM_CLASS || iApperance < 1) {
		iApperance = 1;
	}

	return gPlayerClass[iApperance].model_name;
}

bool CPlayerClassManager::PlayerClass_IsFemale(int iApperance)
{
	if (iApperance >= NUM_CLASS || iApperance < 1) {
		iApperance = 1;
	}

	return gPlayerClass[iApperance].isFemale;
}

void CPlayerClassManager::Client_ApperanceToModel(char* buffer, int iApperance)
{
	auto model = gPlayerClass[iApperance].model_name;
	Q_sprintf(buffer, "models/player/%s/%s.mdl", model, model);
}

int CPlayerClassManager::Client_ModelToApperance(const char* modelname)
{
	if (!strstr(modelname, "models/player/"))
		return 0;	//cannot find
	//return back modelname
	char name[64];
	strcpy(name, modelname + 14);
	name[strlen(name) - 4] = 0;
	name[strlen(name) / 2] = 0;

	for (int i = 1; i < NUM_CLASS; i++)
	{
		if (!stricmp(name, gPlayerClass[i].model_name))
			return i;
	}

	return 0;	//cannot find
}

ModelName CPlayerClassManager::PlayerClass_GetRandomClass()
{
	return (ModelName)RANDOM_LONG(1, NUM_CLASS - 1);
}

int CPlayerClassManager::PlayerClass_GetNumClass()
{
	return NUM_CLASS;
}

int CPlayerClassManager::PlayerClass_GetNumCT()
{
	int iCount = 0;
	for (int i = 1; i < NUM_CLASS; i++)
	{
		if (gPlayerClass[i].team == CT)
			iCount++;
	}
	return iCount;
}

int CPlayerClassManager::PlayerClass_GetNumTR()
{
	int iCount = 0;
	for (int i = 1; i < NUM_CLASS; i++)
	{
		if (gPlayerClass[i].team == TERRORIST)
			iCount++;
	}
	return iCount;
}

void CPlayerClassManager::SetPlayerClass(int index, char* name)
{
	if (index > 33 || index < 0)
		return;

	int ClassID = Client_ModelToApperance(name);
	
	if (!ClassID)
		m_PlayerClass[index] = &m_NullClass;

	m_PlayerClass[index] = &gPlayerClass[ClassID];
}


}
