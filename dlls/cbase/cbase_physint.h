/*
cbase_physint.h - CSMoE Gameplay server
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

#ifndef PROJECT_CBASE_PHYSINT_H
#define PROJECT_CBASE_PHYSINT_H

#include "physint.h"
#include "exportdef.h"

extern "C" int EXPORT Server_GetPhysicsInterface( int, server_physics_api_t*, physics_interface_t* );


#endif //PROJECT_CBASE_PHYSINT_H
