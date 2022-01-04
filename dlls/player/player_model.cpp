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
	"models/player/militia/militia.mdl"	// CZ
};

struct AppearanceInfo
{
	const char* tr;
	const char* ct;
};

static AppearanceInfo gPlayerAppearance[] =
{
	// t   ct
	{NULL, NULL},
	{"terror", "urban"},
	{"leet", "gsg9"},
	{"arctic", "sas"},
	{"guerilla", "gign"},
	{"militia", "spetsnaz"},
};

constexpr auto NUM_APPEARANCES = std::extent<decltype(gPlayerAppearance)>::value;

const char* PlayerModel_GetApperance(int iApperance, TeamName iTeam)
{
	if (iApperance >= NUM_APPEARANCES || iApperance < 1) {
		iApperance = 1;
	}

	return (iTeam == TERRORIST) ? gPlayerAppearance[iApperance].tr : gPlayerAppearance[iApperance].ct;
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
		ENGINE_FORCE_UNMODIFIED(force_model_specifybounds, (float *)&vMin, (float *)&vMax, psz);
}

}
