#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "mods.h"

#include <tuple>
#include <type_traits>

#include "mod_none.h"
#include "mod_dm.h"
#include "mod_tdm.h"

IBaseMod *g_pModRunning = nullptr;

template<class T>
IBaseMod *DefaultFactory()
{
	return new T;
}

std::pair<const char *, IBaseMod *(*)()> g_FindList[] = {
	{ "", DefaultFactory<CMod_None> }, // default
	{ "", DefaultFactory<CMod_None> }, // BTE_MOD_CS16
	{ "none", DefaultFactory<CMod_None> }, // BTE_MOD_NONE
	{ "dm", DefaultFactory<CMod_DeathMatch> }, // BTE_MOD_NONE
	{ "tdm", DefaultFactory<CMod_TeamDeathMatch> } // BTE_MOD_NONE
};

void InstallBteMod(const char *name)
{
	for (auto p : g_FindList)
	{
		if (!strcasecmp(name, p.first))
		{
			g_pModRunning = p.second();
			return;
		}
	}
	g_pModRunning = g_FindList[0].second(); // default
	return;
}