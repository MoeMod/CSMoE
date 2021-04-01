/*
stdafx.h - Pre-compile header
Copyright (C) 2019 Moemod Hymei

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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

#ifndef CLIENT_DLL
#include "monsters.h"
#include "game.h"
#include "client.h"
#include "enginecallback.h"
#include "globals.h"
#include "trains.h"
#include "bmodels.h"
#include "bot_include.h"

#include "gamemode/mods.h"

#include "util/u_functor.hpp"
#include "util/u_iterator.hpp"
#include "util/u_range.hpp"
#include "util/u_time.hpp"
#include "util/u_vector.hpp"
#endif

#include "std_include.h"
