/*
const_client.h - CSMoE Client : Consts
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

//TF ADDITIONS
constexpr DamageType_t DMG_IGNITE = (1 << 24);			// Players hit by this begin to burn
constexpr DamageType_t DMG_RADIUS_MAX = (1 << 25);		// Radius damage with this flag doesn't decrease over distance
constexpr DamageType_t DMG_RADIUS_QUAKE = (1 << 26);	// Radius damage is done like Quake. 1/2 damage at 1/2 radius.
constexpr DamageType_t DMG_IGNOREARMOR = (1 << 27);		// Damage ignores target's armor
constexpr DamageType_t DMG_AIMED = (1 << 28);			// Does Hit location damage
constexpr DamageType_t DMG_WALLPIERCING = (1 << 29);	// Blast Damages ents through walls
constexpr DamageType_t DMG_CALTROP = (1 << 30);
constexpr DamageType_t DMG_HALLUC = (1 << 31);

constexpr auto DMG_FIRSTTIMEBASED = DMG_DROWN;

// TF Healing Additions for TakeHealth
constexpr auto DMG_IGNORE_MAXHEALTH = DMG_IGNITE;
// TF Redefines since we never use the originals
constexpr auto DMG_NAIL = DMG_SLASH;
constexpr auto DMG_NOT_SELF = DMG_FREEZE;
constexpr auto DMG_TRANQ = DMG_MORTAR;
constexpr auto DMG_CONCUSS = DMG_SONIC;