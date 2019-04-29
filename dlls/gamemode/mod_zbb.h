/*
mob_zbb.h - CSMoE Gameplay server : Zombie Mod Base-builder
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

#ifndef PROJECT_MOD_ZBB_H
#define PROJECT_MOD_ZBB_H

#include "mod_zb1.h"

class CMod_ZombieBaseBuilder : public CMod_Zombi
{
public:
	CMod_ZombieBaseBuilder();

	void UpdateGameMode(CBasePlayer *pPlayer) override;

	void InstallPlayerModStrategy(CBasePlayer *player) override;

};

#endif //PROJECT_MOD_ZBB_H
