/*
mod_gd.h - CSMoE Gameplay server : GunDeath
Copyright (C) 2019 Moemod Hyakuya

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef PROJECT_MOD_GD_H
#define PROJECT_MOD_GD_H

#include "mod_tdm.h"

#include "EventDispatcher.h"

namespace sv {

class CMod_GunDeath : public CMod_TeamDeathMatch
{
public:
	void UpdateGameMode(CBasePlayer *pPlayer) override;
	void InstallPlayerModStrategy(CBasePlayer *player) override;
	void PlayerKilled(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor) override;

	class PlayerModStrategy;

private:
	EventDispatcher<void(CBasePlayer *pVictim, CBasePlayer *pKiller, entvars_t *pInflictor)> m_eventPlayerKilled;
};

}

#endif //PROJECT_MOD_GD_H
