/*
const_shared.h - CSMoE Server / Client : Consts
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

#pragma once

using DamageType_t = int;

constexpr DamageType_t DMG_GENERIC = 0;					// generic damage was done
constexpr DamageType_t DMG_CRUSH = (1 << 0);			// crushed by falling or moving object
constexpr DamageType_t DMG_BULLET = (1 << 1);			// shot
constexpr DamageType_t DMG_SLASH = (1 << 2);			// cut, clawed, stabbed
constexpr DamageType_t DMG_BURN = (1 << 3);				// heat burned
constexpr DamageType_t DMG_FREEZE = (1 << 4);			// frozen
constexpr DamageType_t DMG_FALL = (1 << 5);				// fell too far
constexpr DamageType_t DMG_BLAST = (1 << 6);			// explosive blast damage
constexpr DamageType_t DMG_CLUB = (1 << 7);				// crowbar, punch, headbutt
constexpr DamageType_t DMG_SHOCK = (1 << 8);			// electric shock
constexpr DamageType_t DMG_SONIC = (1 << 9);			// sound pulse shockwave
constexpr DamageType_t DMG_ENERGYBEAM = (1 << 10);		// laser or other high energy beam
constexpr DamageType_t DMG_NEVERGIB = (1 << 12);		// with this bit OR'd in, no damage type will be able to gib victims upon death
constexpr DamageType_t DMG_ALWAYSGIB = (1 << 13);		// with this bit OR'd in, any damage type can be made to gib victims upon death
constexpr DamageType_t DMG_DROWN = (1 << 14);			// Drowning

// time-based damage
//mask off TF-specific stuff too
constexpr int DMG_TIMEBASED = (~(0xff003fff));			// mask for time-based damage

constexpr DamageType_t DMG_PARALYZE = (1 << 15);		// slows affected creature down
constexpr DamageType_t DMG_NERVEGAS = (1 << 16);		// nerve toxins, very bad
constexpr DamageType_t DMG_POISON = (1 << 17);			// blood poisioning
constexpr DamageType_t DMG_RADIATION = (1 << 18);		// radiation exposure
constexpr DamageType_t DMG_DROWNRECOVER = (1 << 19);	// drowning recovery
constexpr DamageType_t DMG_ACID = (1 << 20);			// toxic chemicals or acid burns
constexpr DamageType_t DMG_SLOWBURN = (1 << 21);		// in an oven
constexpr DamageType_t DMG_SLOWFREEZE = (1 << 22);		// in a subzero freezer
constexpr DamageType_t DMG_MORTAR = (1 << 23);			// Hit by air raid (done to distinguish grenade from mortar)
constexpr DamageType_t DMG_EXPLOSION = (1 << 24);

constexpr int WEAPON_IS_ONTARGET = 0x40;
