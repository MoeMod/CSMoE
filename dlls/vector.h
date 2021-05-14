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

#ifndef VECTOR_H
#define VECTOR_H
#ifdef _WIN32
#pragma once
#endif

#ifdef DotProduct
#undef DotProduct
#endif

#include "util/u_vector.hpp"

#ifndef CLIENT_DLL
using Vector2D = sv::moe::VectorBase<float, 2>;
#ifdef XASH_SIMD
using Vector = sv::moe::VectorBase<float, 3, 16>;
#else
using Vector = sv::moe::VectorBase<float, 3>;
#endif
#else
using Vector2D = cl::moe::VectorBase<float, 2>;
#ifdef XASH_SIMD
using Vector = cl::moe::VectorBase<float, 3, 16>;
#else
using Vector = cl::moe::VectorBase<float, 3>;
#endif
#endif

static_assert(sizeof(Vector2D) == sizeof(float[2]), "Vector2D should be compatible with engine");
#ifdef XASH_SIMD
static_assert(sizeof(Vector) == sizeof(float[4]) && alignof(Vector) == 16, "Vector should be compatible with engine");
#else
static_assert(sizeof(Vector) == sizeof(float[3]), "Vector should be compatible with engine");
#endif

#endif // VECTOR_H
