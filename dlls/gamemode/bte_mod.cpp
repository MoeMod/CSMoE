#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "bte_mod.h"

#include <tuple>
#include <type_traits>

#include "mod_none.h"



template<class T>
IBaseMod *DefaultFactory()
{
	return new T;
}

std::pair<const char *, IBaseMod *(*)()> g_FindList[] = {
	{ "", DefaultFactory<CMod_None> }, // default
	{ "", DefaultFactory<CMod_None> }, // BTE_MOD_CS16
	{ "none", DefaultFactory<CMod_None> } // BTE_MOD_NONE
};

IBaseMod *InstallBteMod(const char *name)
{
	for (auto p : g_FindList)
	{
		if (!strcasecmp(name, p.first))
		{
			return p.second();
		}
	}
	return g_FindList[0].second(); // default
}

IBaseMod *InstallBteMod(GameMode_e n)
{
	return g_FindList[n].second();
}