#ifndef PLAYER_MODEL_H
#define PLAYER_MODEL_H
#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif


#define	SHOW_DAMAGE (1 << 0)
#define	SHOW_SPEED (1 << 1)
#define	SHOW_ACCSHOOT_FIRST (1 << 2)
#define	SHOW_TRACE_LOW_HEALTH (1 << 3)
#define	SHOW_LAST (1 << 4)

static const char* sPlayerModelFiles[] =
{
	"models/player.mdl",
	"models/player/leet/leet.mdl",
	"models/player/gign/gign.mdl",
	"models/player/vip/vip.mdl",
	"models/player/gsg9/gsg9.mdl",
	"models/player/guerilla/guerilla.mdl",
	"models/player/arctic/arctic.mdl",
	"models/player/sas/sas.mdl",
	"models/player/terror/terror.mdl",
	"models/player/urban/urban.mdl",
	"models/player/spetsnaz/spetsnaz.mdl",	// CZ
	"models/player/militia/militia.mdl",	// CZ
	"models/player/hero/hero.mdl",
	"models/player/heroine/heroine.mdl",
	"models/player/ghosthunter/ghosthunter.mdl",
	"models/player/masterhunter/masterhunter.mdl",
	"models/player/ascetichero/ascetichero.mdl",
	"models/player/timehunter/timehunter.mdl",
	"models/player/healhunter/healhunter.mdl",
	"models/player/lasthero/lasthero.mdl",
	"models/player/mechanichero/mechanichero.mdl",
	"models/player/omoh_bl/omoh_bl.mdl",
};


void Client_ApperanceToModel(char* buffer, int iApperance, TeamName iTeam);

void PlayerModel_Precache();
void PlayerModel_ForceUnmodified(const Vector &vMin, const Vector &vMax);

int PlayerModel_GetNumSkins();
ModelName PlayerModel_GetRandomSkin();
const char* PlayerModel_GetApperance(int iApperance, TeamName iTeam);
bool PlayerModel_IsFemale(int iApperance, TeamName iTeam);

class CPlayerClassManager
{
public:
	struct ClassData
	{
		int ClassID;
		const char* model_name;
		TeamName team;
		bool isFemale;
		int m_iBitsShowState;
		int HandTexid[6];
	};
	void PlayerModel_Precache();
	const char* PlayerClass_GetModelName(int iApperance);
	bool PlayerClass_IsFemale(int iApperance);
	void Client_ApperanceToModel(char* buffer, int iApperance);
	int Client_ModelToApperance(const char* modelname);
	int PlayerClass_GetNumClass();
	int PlayerClass_GetNumCT();
	int PlayerClass_GetNumTR();
	ModelName PlayerClass_GetRandomClass();

	void SetPlayerClass(int index, char* name);

	inline ClassData& GetPlayerClass(int idx)
	{
		if (idx > 33 || idx < 0)
			return m_NullClass;

		if (!m_PlayerClass[idx])
			return m_NullClass;
		return *m_PlayerClass[idx];
	}

	ClassData* m_PlayerClass[33];
	ClassData m_NullClass;
};

CPlayerClassManager &PlayerClassManager();

}

#endif