/*
mods_const.h - CSMoE Gameplay shared : Constants
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

#ifndef BTE_MOD_CONST_H
#define BTE_MOD_CONST_H
#ifdef _WIN32
#pragma once
#endif

enum GameMode_e : byte
{
	MOD_SINGLEPLAY = 0, // backward supports
	MOD_MULTIPLAY = 1, // backward supports
	MOD_NONE = 2,
	MOD_DM = 3,
	MOD_TDM = 4,
	MOD_ZB1 = 5,
	MOD_ZB2 = 6,
	MOD_ZBU = 7,
	MOD_ZB3 = 8,
	MOD_ZBS = 9,
	MOD_ZE = 10,
	MOD_ZB4 = 11,
	MOD_GD = 12,
	MOD_ZBB = 13,
};

enum DamageTrack_e : byte
{
	DT_NONE,
	DT_ZB,
	DT_ZBS,
	DT_ZSE
};

#endif