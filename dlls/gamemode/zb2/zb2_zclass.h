/*
zclass.h - CSMoE Gameplay server : zombie class for zb2
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

#ifndef PROJECT_ZCLASS_H
#define PROJECT_ZCLASS_H

#include "gamemode/zb1/zb1_zclass.h"

class IZombieModeCharacter_ZB2_Extra
{
public:
	virtual void ActivateSkill(ZombieSkillSlot which) = 0;
};

class IZombieClass_ZB2_Extra : public IZombieModeCharacter_ZB2_Extra
{
public:
	virtual float OnHealthRecovery() = 0;
};

class CHuman_ZB2 : public CHuman_ZB1, public IZombieModeCharacter_ZB2_Extra
{
public:
	explicit CHuman_ZB2(CBasePlayer *player); // TODO
	void ActivateSkill(ZombieSkillSlot which) override; // TODO
};

class CBaseZombieClass_ZB2 : public CZombie_ZB1, public IZombieClass_ZB2_Extra
{
public:
	explicit CBaseZombieClass_ZB2(CBasePlayer *player); // TODO
	float OnHealthRecovery() override; // TODO
	void ActivateSkill(ZombieSkillSlot which) override {}
};

class CZombieClass_Default : public CBaseZombieClass_ZB2
{
public:
	void ActivateSkill(ZombieSkillSlot which) override; // TODO
};

#endif //PROJECT_ZCLASS_H
