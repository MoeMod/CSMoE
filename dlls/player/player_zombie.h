#ifndef PLAYER_ZOMBIE_H
#define PLAYER_ZOMBIE_H
#ifdef _WIN32
#pragma once
#endif

#include "player_zombie_skill.h"

enum ZombieLevel : int
{
	ZOMBIE_LEVEL_HOST,
	ZOMBIE_LEVEL_ORIGIN,
	ZOMBIE_LEVEL_ORIGIN_LV2, // TBD
};

void PlayerZombie_Precache();

#endif