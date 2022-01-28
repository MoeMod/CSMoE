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
	Z4_MESSAGE_ICON, // { byte msg_type, byte slot, byte id, byte status }
	Z4_MESSAGE_NIGHTTIME, // { byte msg_type, byte start, byte end }
	Z4_MESSAGE_ZCLASS,
};

#endif
