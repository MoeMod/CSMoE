/*
zbb_zombie.h - CSMoE Gameplay server : Zombie Mod Base-builder
Copyright (C) 2019 Moemod Hymei

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "gamemode/zb1/zb1_zclass.h"
#include "gamemode/zb1/zb1_const.h"
#include "gamemode/mod_zbb.h"

namespace sv {

class CZombieClass_ZBB : public CZombie_ZB1, public IZombieModeCharacter_ZBB_ExtraDelegate
{
public:
	explicit CZombieClass_ZBB(CBasePlayer *player);

public:
	void ButtonEvent(unsigned short &bitsCurButton, int bitsOldButton) override {}
	bool IsBuilding() override { return false; }
	void PostThink() override;

public:
	void Think() override {}

};

extern IPlayerBuildingDelegate *g_pDelegateZombieShared;

}
