/*
z4_const.h - CSMoE : Zombie 4 Shared Constants
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

#ifndef Z4_CONST_H
#define Z4_CONST_H
#ifdef _WIN32
#pragma once
#endif

enum Z4MessageType : byte
{
	Z4_MESSAGE_ENERGY, // { byte msg_type, byte energy, byte flashtime }
	Z4_MESSAGE_DAMAGE, // { byte msg_type, byte a, byte b }
	Z4_MESSAGE_STATUS, // { byte msg_type, byte slot, byte id, byte status }
	Z4_MESSAGE_NIGHTTIME, // { byte msg_type, byte start, byte end }
	Z4_MESSAGE_ZCLASS, // { byte msg_type, byte zclass }
	Z4_MESSAGE_SKILL, // { byte msg_type, byte skill, byte state }

};

enum Z4Status : byte {
	Z4_POWERUP,
	Z4_CRITICALSHOT,
	Z4_MIDNIGHTUP,
	Z4_MIDNIGHTDOWN,
	Z4_SPEEDUP,
	Z4_SPEEDDOWN,
	Z4_CANNOTCONTROL,
	Z4_DEFENSEUP,
	Z4_DEFENSEDOWN,
	Z4_HPUP,
	Z4_HPDOWN,
	Z4_MAX_STATUS
};

enum Z4StatusIconDraw : byte {
	Z4_ICON_HIDE,
	Z4_ICON_SHOW,
	Z4_ICON_FLASH,
	Z4_ICON_FLASH2
};

enum Z4ZClassType : byte {
	Z4_CLASS_NORMAL,
	Z4_CLASS_LIGHT,
	Z4_CLASS_HEAVY,
	Z4_CLASS_HIDE,
	Z4_CLASS_HUMPBACK
};

enum Z4ZSkillType : byte {
	Z4_SKILL_ACCSHOOT,
	Z4_SKILL_DASH
};

#endif
