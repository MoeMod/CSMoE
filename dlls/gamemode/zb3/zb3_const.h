/*
zb3_const.h - CSMoE : Zombie Hero Shared Constants
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

#ifndef ZB3_CONST_H
#define ZB3_CONST_H
#ifdef _WIN32
#pragma once
#endif

enum ZB3HumanMoraleType_e : byte
{
	ZB3_MORALE_DEFAULT = 0,
	ZB3_MORALE_STRENGTHEN = 1
};

enum ZB3MessageType : byte
{
	ZB3_MESSAGE_MORALE, // { byte msg_type, byte morale_type, byte level }
	ZB3_MESSAGE_RAGE, // { byte msg_type, byte zombielevel[0-2], byte percent[0-100] }

};

#endif
