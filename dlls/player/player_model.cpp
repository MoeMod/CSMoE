#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "gamerules.h"
#include "player_model.h"

#include <iostream>
#include <fstream>

#include "FileSystem.h"
#include "fs_int.h"
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

static std::map<ModelName, ClassData> gPlayerClass =
{
	{ MODEL_YURI, { MODEL_YURI, "yuri", TERRORIST, true, SHOW_SPEED | SHOW_DAMAGE, "woman" } },
	{ MODEL_SAF, { MODEL_SAF, "saf", CT, false, SHOW_SPEED | SHOW_DAMAGE, "man" } },
	{ MODEL_PIRATEBOY, { MODEL_PIRATEBOY, "pirateboy", TERRORIST, false, SHOW_SPEED | SHOW_DAMAGE, "man" } },
	{ MODEL_CHOIJIYOON, { MODEL_CHOIJIYOON, "choijiyoon", CT, true, SHOW_SPEED | SHOW_DAMAGE, "woman" } },
	{ MODEL_MARINEBOY, { MODEL_MARINEBOY, "marineboy", CT, false, SHOW_SPEED | SHOW_DAMAGE, "man" } },
	{ MODEL_FERNANDO, { MODEL_FERNANDO, "fernando", CT, false, SHOW_SPEED | SHOW_DAMAGE, "man" } },
	{ MODEL_PIRATEGIRL, { MODEL_PIRATEGIRL, "pirategirl", TERRORIST, true, SHOW_SPEED | SHOW_DAMAGE, "woman" } },
	{ MODEL_707, { MODEL_707, "707", CT, false, SHOW_SPEED | SHOW_DAMAGE, "man" } },
	{ MODEL_RB, { MODEL_RB, "rb", TERRORIST, false, SHOW_SPEED | SHOW_DAMAGE, "man" } },
	{ MODEL_SOZO, { MODEL_SOZO, "sozo", CT, false, SHOW_SPEED | SHOW_DAMAGE, "man" } },
	{ MODEL_JPNGIRL01, { MODEL_JPNGIRL01, "jpngirl01", TERRORIST, true, SHOW_SPEED | SHOW_DAMAGE, "chn_woman"} },
	{ MODEL_MAGUI, { MODEL_MAGUI, "magui", CT, false, SHOW_SPEED | SHOW_DAMAGE, "man" } },
	{ MODEL_RITSUKA, { MODEL_RITSUKA, "ritsuka", TERRORIST, true ,SHOW_SPEED | SHOW_DAMAGE, "woman" } },
	{ MODEL_NATASHA, { MODEL_NATASHA, "natasha", CT, true, SHOW_SPEED | SHOW_DAMAGE, "woman" } },
	{ MODEL_TERROR, { MODEL_TERROR, "terror", TERRORIST, false, SHOW_SPEED | SHOW_DAMAGE, "man" } },
	{ MODEL_URBAN, { MODEL_URBAN, "urban", CT, false, SHOW_SPEED | SHOW_DAMAGE, "man" } },
	{ MODEL_LEET, { MODEL_LEET, "leet", TERRORIST, false, SHOW_SPEED | SHOW_DAMAGE, "man" } },
	{ MODEL_GSG9, { MODEL_GSG9, "gsg9", CT, false, SHOW_SPEED | SHOW_DAMAGE, "man" } },
	{ MODEL_ARCTIC, { MODEL_ARCTIC, "arctic", TERRORIST, false, SHOW_SPEED | SHOW_DAMAGE, "man" } },
	{ MODEL_SAS, { MODEL_SAS, "sas", CT, false, SHOW_SPEED | SHOW_DAMAGE, "man" } },
	{ MODEL_GUERILLA, { MODEL_GUERILLA, "guerilla", TERRORIST, false, SHOW_SPEED | SHOW_DAMAGE, "man" } },
	{ MODEL_GIGN, { MODEL_GIGN, "gign", CT, false, SHOW_SPEED | SHOW_DAMAGE, "man" } },
	{ MODEL_MILITIA, { MODEL_MILITIA, "militia", TERRORIST, false, SHOW_SPEED | SHOW_DAMAGE, "man" } },
	{ MODEL_SPETSNAZ, { MODEL_SPETSNAZ, "spetsnaz", CT, false, SHOW_SPEED | SHOW_DAMAGE, "man" } },
	{ MODEL_BUFFCLASSB, { MODEL_BUFFCLASSB, "buffclassb", TERRORIST, true, SHOW_SPEED | SHOW_DAMAGE, "woman" } },
	{ MODEL_BUFFCLASSA, { MODEL_BUFFCLASSA, "buffclassa", CT, true, SHOW_SPEED | SHOW_DAMAGE, "woman" } },
	{ MODEL_BUFFCLASSHUNTER, { MODEL_BUFFCLASSHUNTER, "buffclasshunter", TERRORIST, true, SHOW_SPEED | SHOW_DAMAGE, "woman" } },
	{ MODEL_BUFFCLASSLYCAN, { MODEL_BUFFCLASSLYCAN, "buffclasslycan", CT, true, SHOW_SPEED | SHOW_DAMAGE, "woman" } },
	{ MODEL_BUFFCLASSBLAIR, { MODEL_BUFFCLASSBLAIR, "buffclassblair", TERRORIST, true, SHOW_SPEED | SHOW_DAMAGE, "woman" } },
	{ MODEL_BUFFCLASSFERNANDO, { MODEL_BUFFCLASSFERNANDO, "buffclassfernando", CT, false, SHOW_SPEED | SHOW_DAMAGE, "man" } },
	{ MODEL_JPNGIRL01ORG, { MODEL_JPNGIRL01ORG, "jpngirl01org", TERRORIST, true, SHOW_SPEED | SHOW_DAMAGE, "woman" } },
	{ MODEL_SOI, { MODEL_SOI, "soi", CT, true, SHOW_SPEED | SHOW_DAMAGE, "woman" } },
	{ MODEL_IDOLGIRL, { MODEL_IDOLGIRL, "idolgirl", TERRORIST, true, SHOW_SPEED | SHOW_DAMAGE, "woman" } },
	{ MODEL_MARINEGIRL, { MODEL_MARINEGIRL, "marinegirl", CT, true, SHOW_SPEED | SHOW_DAMAGE, "woman" } },
	{ MODEL_BLAIR, { MODEL_BLAIR, "blair", TERRORIST, true, SHOW_SPEED | SHOW_DAMAGE, "woman" } },
	{ MODEL_JENNIFER, { MODEL_JENNIFER, "jennifer", TERRORIST, true, SHOW_SPEED | SHOW_DAMAGE, "woman" } },
	{ MODEL_MA_ALICEBLUE, { MODEL_MA_ALICEBLUE, "ma_aliceblue", CT, true, SHOW_SPEED | SHOW_DAMAGE, "woman" } },
	{ MODEL_MA_ALICERED, { MODEL_MA_ALICERED, "ma_alicered", TERRORIST, true, SHOW_SPEED | SHOW_DAMAGE, "woman" } },
	{ MODEL_JRA, { MODEL_JRA, "jra", TERRORIST, false, SHOW_SPEED | SHOW_DAMAGE, "man" } },
	{ MODEL_SDEFENCE, { MODEL_SDEFENCE, "sdefence", CT, false, SHOW_SPEED | SHOW_DAMAGE, "man" } },
	{ MODEL_SCJRA, { MODEL_SCJRA, "scjra", TERRORIST, false, SHOW_SPEED | SHOW_DAMAGE, "man" } },
	{ MODEL_SCVIP, { MODEL_SCVIP, "scvip", CT, false, SHOW_SPEED | SHOW_DAMAGE, "man" } },
	{ MODEL_SCYURI, { MODEL_SCYURI, "scyuri", TERRORIST, true, SHOW_SPEED | SHOW_DAMAGE, "woman" } },
	{ MODEL_SCCJY, { MODEL_SCCJY, "sccjy", CT, true, SHOW_SPEED | SHOW_DAMAGE, "woman" } },
	{ MODEL_YURI2, { MODEL_YURI2, "yuri2", TERRORIST, true, SHOW_SPEED | SHOW_DAMAGE, "woman" } },
	{ MODEL_ALICE2, { MODEL_ALICE2, "alice2", CT, true, SHOW_SPEED | SHOW_DAMAGE, "woman" } },
	{ MODEL_ETA, { MODEL_ETA, "eta", TERRORIST, false, SHOW_SPEED | SHOW_DAMAGE, "man" } },
	{ MODEL_IRA, { MODEL_IRA, "ira", TERRORIST, false, SHOW_SPEED | SHOW_DAMAGE, "man" } },
	{ MODEL_CHNGIRL01, { MODEL_CHNGIRL01, "chngirl01", CT, true, SHOW_SPEED | SHOW_DAMAGE, "chn_woman" } },
	{ MODEL_HUNTER, { MODEL_HUNTER, "hunter", CT, false, SHOW_SPEED | SHOW_DAMAGE, "man" } },
	{ MODEL_SPADE, { MODEL_SPADE, "spade", TERRORIST, false, SHOW_SPEED | SHOW_DAMAGE, "man" } },
	{ MODEL_CRISS, { MODEL_CRISS, "criss", CT, true, SHOW_SPEED | SHOW_DAMAGE, "woman" } },
	{ MODEL_NATASHA2, { MODEL_NATASHA2, "natasha2", CT, true, SHOW_SPEED | SHOW_DAMAGE, "woman" } },
	{ MODEL_CHOIJIYOON2, { MODEL_CHOIJIYOON2, "choijiyoon2", CT, true, SHOW_SPEED | SHOW_DAMAGE, "woman" } },
	{ MODEL_JENNIFER2, { MODEL_JENNIFER2, "jennifer2", TERRORIST, true, SHOW_SPEED | SHOW_DAMAGE, "woman" } },
	{ MODEL_PIRATEGIRL2, { MODEL_PIRATEGIRL2, "pirategirl2", TERRORIST, true, SHOW_SPEED | SHOW_DAMAGE, "woman" } },
	{ MODEL_MERCENARYCT, { MODEL_MERCENARYCT, "mercenaryct", CT, false, SHOW_SPEED | SHOW_DAMAGE, "man" } },
	{ MODEL_MERCENARYTR, { MODEL_MERCENARYTR, "mercenarytr", TERRORIST, false, SHOW_SPEED | SHOW_DAMAGE, "man" } },
	{ MODEL_Y20S1YEONGSIN, { MODEL_Y20S1YEONGSIN, "y20s1yeongsin", CT, false, SHOW_SPEED | SHOW_DAMAGE, "yeongsin"}},
	{ MODEL_Y20S1LEECHANG, { MODEL_Y20S1LEECHANG, "y20s1leechang", TERRORIST, false, SHOW_SPEED | SHOW_DAMAGE, "leechang" } },
	{ MODEL_BUFFCLASS22S4CT, { MODEL_BUFFCLASS22S4CT, "buffclass22s4ct", CT, true, SHOW_SPEED | SHOW_DAMAGE, "buffdefault" },},
	{ MODEL_BUFFCLASS22S4TR, { MODEL_BUFFCLASS22S4TR, "buffclass22s4tr", TERRORIST, true, SHOW_SPEED | SHOW_DAMAGE, "buffdefault"}},
	{ MODEL_BUFFCLASS19S3CT, { MODEL_BUFFCLASS19S3CT, "buffclass19s3ct", CT, true, SHOW_SPEED | SHOW_DAMAGE, "19s3ct" },},
	{ MODEL_BUFFCLASS19S3TR, { MODEL_BUFFCLASS19S3TR, "buffclass19s3tr", TERRORIST, true, SHOW_SPEED | SHOW_DAMAGE, "19s3tr"}},
	{ MODEL_BUFFCLASS22S1CT, { MODEL_BUFFCLASS22S1CT, "buffclass22s1ct", CT, true, SHOW_SPEED | SHOW_DAMAGE, "22s1ct" },},
	{ MODEL_BUFFCLASS22S1TR, { MODEL_BUFFCLASS22S1TR, "buffclass22s1tr", TERRORIST, true, SHOW_SPEED | SHOW_DAMAGE, "22s1tr"}},
	{ MODEL_BUFFCLASS22S2CT, { MODEL_BUFFCLASS22S2CT, "buffclass22s2ct", CT, true, SHOW_SPEED | SHOW_DAMAGE, "buffdefault" },},
	{ MODEL_BUFFCLASS22S2TR, { MODEL_BUFFCLASS22S2TR, "buffclass22s2tr", TERRORIST, false, SHOW_SPEED | SHOW_DAMAGE, "buffdefault"}},
	{ MODEL_BUFFCLASS23S1CT, { MODEL_BUFFCLASS23S1CT, "buffclass23s1ct", CT, true, SHOW_SPEED | SHOW_DAMAGE, "buffdefault" },},
	{ MODEL_BUFFCLASS23S1TR, { MODEL_BUFFCLASS23S1TR, "buffclass23s1tr", TERRORIST, true, SHOW_SPEED | SHOW_DAMAGE, "buffdefault"}},
	{ MODEL_BUFFCLASS23S2CT, { MODEL_BUFFCLASS23S2CT, "buffclass23s2ct", CT, true, SHOW_SPEED | SHOW_DAMAGE, "buffdefault" },},
	{ MODEL_BUFFCLASS23S2TR, { MODEL_BUFFCLASS23S2TR, "buffclass23s2tr", TERRORIST, true, SHOW_SPEED | SHOW_DAMAGE, "buffdefault"}},

	/*{ MODEL_UNASSIGNED, { MODEL_UNASSIGNED, NULL, UNASSIGNED } },
	{ MODEL_YURI, { MODEL_YURI, "yuri", TERRORIST, true, SHOW_SPEED | SHOW_DAMAGE} },
	{ MODEL_SAF, "saf", CT, false, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_PIRATEBOY, "pirateboy", TERRORIST, false, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_CHOIJIYOON, "choijiyoon", CT, true, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_MARINEBOY, "marineboy", CT, false, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_FERNANDO, "fernando", CT, false, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_PIRATEGIRL, "pirategirl", TERRORIST, true, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_707, "707", CT, false, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_RB, "rb", TERRORIST, false, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_SOZO, "sozo", CT, false, SHOW_SPEED | SHOW_DAMAGE},
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
	{ MODEL_JPNGIRL01ORG, "jpngirl01org", TERRORIST, true, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_SOI, "soi", CT, true, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_IDOLGIRL, "idolgirl", TERRORIST, true, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_MARINEGIRL, "marinegirl", CT, true, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_BLAIR, "blair", TERRORIST, true, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_JENNIFER, "jennifer", TERRORIST, true, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_ALICEBLUE, "ma_alice", CT, true, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_ALICERED, "ma_alice", TERRORIST, true, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_JIA, "jra", TERRORIST, false, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_SDEFENCE, "sdefence", CT, false, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_SCJIA, "scjra", TERRORIST, false, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_SCVIP, "scvip", CT, false, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_SCYURI, "scyuri", TERRORIST, true, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_SCCJY, "sccjy", CT, true, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_YURI2, "yuri2", TERRORIST, true, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_ALICE2, "alice2", CT, true, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_ETA, "eta", TERRORIST, false, SHOW_SPEED | SHOW_DAMAGE},
	{ MODEL_IRA, "ira", TERRORIST, false, SHOW_SPEED | SHOW_DAMAGE},*/
};
int NUM_CLASS = gPlayerClass.size();

extern fs_api_t gFileSystemAPI;

#ifndef CLIENT_DLL
nlohmann::json g_Radio;
#endif

static const char* m_szRadioType[] =
{
	"MRAD_AFFIRM",
	"MRAD_BACKUP",
	"MRAD_BLOW" ,
	"MRAD_BOMBDEF",
	"MRAD_BOMBPL",
	"MRAD_CIRCLEBACK",
	"MRAD_CLEAR",
	"MRAD_COVERME" ,
	"MRAD_CTWIN" ,
	"MRAD_ELIM",
	"MRAD_ENEMYDOWN" ,
	"MRAD_ENEMYSPOT" ,
	"MRAD_ESCAPED" ,
	"MRAD_FALLBACK" ,
	"MRAD_FIREINHOLE" ,
	"MRAD_FOLLOWCOM" ,
	"MRAD_FOLLOWME" ,
	"MRAD_GETINPOS" ,
	"MRAD_GETOUT" ,
	"MRAD_GO" ,
	"MRAD_GOGOGO" ,
	"MRAD_HITASSIST" ,
	"MRAD_HOSDOWN" ,
	"MRAD_IMHIT" ,
	"MRAD_INPOS" ,
	"MRAD_JOKE1" ,
	"MRAD_JOKE2" ,
	"MRAD_LETSGO" ,
	"MRAD_LOCKNLOAD" ,
	"MRAD_MATEDOWN" ,
	"MRAD_MEETME" ,
	"MRAD_MOVEOUT" ,
	"MRAD_NEGATIVE" ,
	"MRAD_POINT" ,
	"MRAD_POSITION" ,
	"MRAD_REGROUP" ,
	"MRAD_REPORTIN" ,
	"MRAD_REPRTINGIN" ,
	"MRAD_RESCUED" ,
	"MRAD_ROGER" ,
	"MRAD_ROUNDDRAW" ,
	"MRAD_STICKTOG" ,
	"MRAD_STORMFRONT" ,
	"MRAD_TAKEPOINT" ,
	"MRAD_TERWIN" ,
	"MRAD_VIP" ,
	"MRAD_ASITE" ,
	"MRAD_BSITE" ,
	"MRAD_NICESHOT" ,
	"MRAD_THANK" ,
	"MRAD_WANTHELP" ,
	"MRAD_WIN",
	"MRAD_LOSE",
	"VOICE_HURT_HEAD",
	"VOICE_HURT_FLESH" ,
	"VOICE_DEATH"
};

void CPlayerClassManager::InitRadio()
{
#ifndef CLIENT_DLL
	fs_offset_t length;

	auto data = gFileSystemAPI.FS_MapFile("sound/sentences.json", &length, true);
	if (data)
	{
		g_Radio = nlohmann::json::parse(std::string_view((const char*)data, length));
		gFileSystemAPI.FS_MapFree(data, length);
	}
#endif
}


void CPlayerClassManager::PlayerModel_Precache()
{
#ifndef CLIENT_DLL
	InitRadio();

	char path[128];

	for (auto it : gPlayerClass) {
		Client_ApperanceToModel(path, it.first);
		PRECACHE_MODEL(path);
	}
#endif
}

const char* CPlayerClassManager::PlayerClass_GetModelName(ModelName iApperance)
{
	if (!gPlayerClass.count(iApperance))
		iApperance = MODEL_YURI;

	return gPlayerClass[iApperance].model_name;
}

bool CPlayerClassManager::PlayerClass_IsFemale(ModelName iApperance)
{
	if (!gPlayerClass.count(iApperance))
		iApperance = MODEL_YURI;

	return gPlayerClass[iApperance].isFemale;
}

void CPlayerClassManager::Client_ApperanceToModel(char* buffer, ModelName iApperance)
{
	auto model = gPlayerClass[iApperance].model_name;
	Q_sprintf(buffer, "models/player/%s/%s.mdl", model, model);
}

ModelName CPlayerClassManager::Client_ModelToApperance(const char* modelname)
{
	if (!strstr(modelname, "models/player/"))
		return MODEL_UNASSIGNED;	//cannot find
	//return back modelname
	char name[64];
	strcpy(name, modelname + 14);
	name[strlen(name) - 4] = 0;
	name[strlen(name) / 2] = 0;

	for (auto it : gPlayerClass) {
		if (!stricmp(name, it.second.model_name))
			return it.first;
	}

	return MODEL_UNASSIGNED;	//cannot find
}

ModelName CPlayerClassManager::PlayerClass_GetRandomClass()
{
	auto it = gPlayerClass.cbegin();
	int random = rand() % gPlayerClass.size();
	std::advance(it, random);

	return it->first;
}

int CPlayerClassManager::PlayerClass_GetNumClass()
{
	return NUM_CLASS;
}

int CPlayerClassManager::PlayerClass_GetNumCT()
{
	int iCount = 0;

	for (auto it : gPlayerClass) {
		if (it.second.team == CT)
			iCount++;
	}

	return iCount;
}

int CPlayerClassManager::PlayerClass_GetNumTR()
{
	int iCount = 0;

	for (auto it : gPlayerClass) {
		if (it.second.team == TERRORIST)
			iCount++;
	}

	return iCount;
}

void CPlayerClassManager::SetPlayerClass(int index, char* name)
{
	if (index > 33 || index < 0)
		return;

	ModelName ClassID = Client_ModelToApperance(name);
	
	if (!ClassID)
	{
		m_PlayerClass[index] = &m_NullClass;
		return;
	}
		
	m_PlayerClass[index] = &gPlayerClass[ClassID];
}

void CPlayerClassManager::SetPlayerClass(int index, ModelName name)
{
	if (index > 33 || index < 0)
		return;

	if (!name)
		m_PlayerClass[index] = &m_NullClass;

	m_PlayerClass[index] = &gPlayerClass[name];
}


}
