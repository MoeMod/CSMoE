#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "gamerules.h"

namespace sv {

static const char *sPlayerModelFiles[] =
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
};

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

}
