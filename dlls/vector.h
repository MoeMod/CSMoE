/*
vector.h - CSMoE Foundation : Vector
Copyright (C) 2019 Moemod Yanase

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

#ifdef DotProduct
#undef DotProduct
#endif

#include "xash3d_types.h"
#include "util/u_vector.hpp"
#include "util/u_vector_angle.hpp"

#ifndef CLIENT_DLL
namespace sv::moe {
#else
namespace cl::moe {
#endif

using Vector2D = vec2_t;
using Vector = vec3_t;

}
