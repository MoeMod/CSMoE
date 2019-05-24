/*
const_server.h - CSMoE Server : Consts
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

#include "const_shared.h"

// These are caps bits to indicate what an object's capabilities (currently used for save/restore and level transitions)
constexpr int FCAP_CUSTOMSAVE = 0x00000001;
constexpr int FCAP_ACROSS_TRANSITION = 0x00000002;	// Should transfer between transitions
constexpr int FCAP_MUST_SPAWN = 0x00000004;			// Spawn after restore
constexpr int FCAP_DONT_SAVE = 0x80000000;			// Don't save this
constexpr int FCAP_IMPULSE_USE = 0x00000008;		// Can be used by the player
constexpr int FCAP_CONTINUOUS_USE = 0x00000010;		// Can be used by the player
constexpr int FCAP_ONOFF_USE = 0x00000020;			// Can be used by the player
constexpr int FCAP_DIRECTIONAL_USE = 0x00000040;	// Player sends +/- 1 when using (currently only tracktrains)
constexpr int FCAP_MASTER = 0x00000080;				// Can be used to "master" other entities (like multisource)
constexpr int FCAP_MUST_RESET = 0x00000100;			// Should reset on the new round
constexpr int FCAP_MUST_RELEASE = 0x00000200;		// Should release on the new round

// UNDONE: This will ignore transition volumes (trigger_transition), but not the PVS!!!
constexpr int FCAP_FORCE_TRANSITION = 0x00000080;	// ALWAYS goes across transitions

constexpr size_t ROUTE_SIZE = 8;
constexpr size_t MAX_OLD_ENEMIES = 4;

constexpr int bits_CAP_DUCK = (1 << 0);
constexpr int bits_CAP_JUMP = (1 << 1);
constexpr int bits_CAP_STRAFE = (1 << 2);
constexpr int bits_CAP_SQUAD = (1 << 3);
constexpr int bits_CAP_SWIM = (1 << 4);
constexpr int bits_CAP_CLIMB = (1 << 5);
constexpr int bits_CAP_USE = (1 << 6);
constexpr int bits_CAP_HEAR = (1 << 7);
constexpr int bits_CAP_AUTO_DOORS = (1 << 8);
constexpr int bits_CAP_OPEN_DOORS = (1 << 9);
constexpr int bits_CAP_TURN_HEA = (1 << 10);
constexpr int bits_CAP_RANGE_ATTACK1 = (1 << 11);
constexpr int bits_CAP_RANGE_ATTACK2 = (1 << 12);
constexpr int bits_CAP_MELEE_ATTACK1 = (1 << 13);
constexpr int bits_CAP_MELEE_ATTACK2 = (1 << 14);
constexpr int bits_CAP_FLY = (1 << 15);
constexpr int bits_CAP_DOORS_GROUP = (bits_CAP_USE | bits_CAP_AUTO_DOORS | bits_CAP_OPEN_DOORS);

// for Classify
enum {
	CLASS_NONE = 0,
	CLASS_MACHINE = 1,
	CLASS_PLAYER = 2,
	CLASS_HUMAN_PASSIVE = 3,
	CLASS_HUMAN_MILITARY = 4,
	CLASS_ALIEN_MILITARY = 5,
	CLASS_ALIEN_PASSIVE = 6,
	CLASS_ALIEN_MONSTER = 7,
	CLASS_ALIEN_PREY = 8,
	CLASS_ALIEN_PREDATOR = 9,
	CLASS_INSECT = 10,
	CLASS_PLAYER_ALLY = 11,
	CLASS_PLAYER_BIOWEAPON = 12, // hornets and snarks.launched by players
	CLASS_ALIEN_BIOWEAPON = 13, // hornets and snarks.launched by the alien menace
	CLASS_VEHICLE = 14,
	CLASS_BARNACLE = 99, // special because no one pays attention to it, and it eats a wide cross-section of creatures.
};

constexpr int SF_NORESPAWN = (1 << 30); // set this bit on guns and stuff that should never respawn.
constexpr int SF_ITEM_USE_ONLY = (1 << 8);

// These are the damage types that are allowed to gib corpses
constexpr auto DMG_GIB_CORPSE = (DMG_CRUSH | DMG_FALL | DMG_BLAST | DMG_SONIC | DMG_CLUB);

// These are the damage types that have client hud art
constexpr auto DMG_SHOWNHUD = (DMG_POISON | DMG_ACID | DMG_FREEZE | DMG_SLOWFREEZE | DMG_DROWN | DMG_BURN | DMG_SLOWBURN | DMG_NERVEGAS | DMG_RADIATION | DMG_SHOCK);

// When calling KILLED(), a value that governs gib behavior is expected to be
// one of these three values
enum
{
	GIB_NORMAL = 0,
	GIB_NEVER = 1,
	GIB_ALWAYS = 2,
};
constexpr auto GIB_HEALTH_VALUE = -30;


constexpr BYTE PARALYZE_DURATION = 2;
constexpr float PARALYZE_DAMAGE = 1.0f;

constexpr BYTE NERVEGAS_DURATION = 2;
constexpr float NERVEGAS_DAMAGE = 5.0f;

constexpr BYTE POISON_DURATION = 5;
constexpr float POISON_DAMAGE = 2.0f;

constexpr BYTE RADIATION_DURATION = 2;
constexpr float RADIATION_DAMAGE = 1.0f;

constexpr BYTE ACID_DURATION = 2;
constexpr float ACID_DAMAGE = 5.0f;

constexpr BYTE SLOWBURN_DURATION = 2;
constexpr float SLOWBURN_DAMAGE = 1.0f;

constexpr BYTE SLOWFREEZE_DURATION = 2;
constexpr float SLOWFREEZE_DAMAGE = 1.0f;

enum {
	itbd_Paralyze = 0,
	itbd_NerveGas = 1,
	itbd_Poison = 2,
	itbd_Radiation = 3,
	itbd_DrownRecover = 4,
	itbd_Acid = 5,
	itbd_SlowBurn = 6,
	itbd_SlowFreeze = 7,
};
constexpr size_t CDMG_TIMEBASED = 8;

