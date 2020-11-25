/*
mods.cpp - CSMoE Server Gameplay : gamemode object factory
Copyright (C) 2018 Moemod Hyakuya

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

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
#include "mod_zb1.h"
#include "mod_zbs.h"
#include "mod_zb2.h"
#include "mod_zb3.h"
#include "mod_gd.h"
#include "mod_zbb.h"

namespace sv {

[[hlsdk::dll_global]] IBaseMod *g_pModRunning = nullptr;

template<class T>
IBaseMod *DefaultFactory()
{
	return new T;
}

std::pair<const char *, IBaseMod *(*)()> g_FindList[] = {
	{ "", DefaultFactory<CMod_None> }, // default
	{ "", DefaultFactory<CMod_None> }, // BTE_MOD_CS16
	{ "none", DefaultFactory<CMod_None> }, // BTE_MOD_NONE
	{ "dm", DefaultFactory<CMod_DeathMatch> },
	{ "tdm", DefaultFactory<CMod_TeamDeathMatch> },
	{ "zb1", DefaultFactory<CMod_Zombi> },
	{ "zbs", DefaultFactory<CMod_ZombieScenario> },
	{ "zb2", DefaultFactory<CMod_ZombieMod2> },
	{ "zb3", DefaultFactory<CMod_ZombieHero> },
	{ "gd", DefaultFactory<CMod_GunDeath> },
	{ "zbb", DefaultFactory<CMod_ZombieBaseBuilder> }
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

}
