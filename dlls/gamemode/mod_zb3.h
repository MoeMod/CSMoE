/*
mod_zb3.h - CSMoE Gameplay server : Zombie Hero
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

#ifndef PROJECT_MOD_ZB3_H
#define PROJECT_MOD_ZB3_H

#include "mod_zb2.h"

class CMod_ZombieHero : public CMod_ZombieMod2
{
public:
	void InstallPlayerModStrategy(CBasePlayer *) override;

protected:
	void PickZombieOrigin() override;


protected:
	virtual void PickHero();
	virtual size_t HeroNum();
	virtual void MakeHero(CBasePlayer *);
};

class CPlayerModStrategy_ZB3 : public CPlayerModStrategy_ZB2
{
public:
	CPlayerModStrategy_ZB3(CBasePlayer *player, CMod_ZombieHero *mp);

	bool CanUseZombieSkill() override;

private:
	CMod_ZombieHero * const m_pModZB3;
};


#endif //PROJECT_MOD_ZB3_H
