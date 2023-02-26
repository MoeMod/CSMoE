/*
pm_math.h - CSMoE Foundation : mathlib.h replacement for modern C++
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

#ifndef PM_MATH_H
#define PM_MATH_H
#ifdef _WIN32
#pragma once
#endif

#include "vector.h"

#if defined(CLIENT_DLL)
namespace cl {
#elif defined(SERVER_DLL)
namespace sv {
#elif defined(__cplusplus)
extern "C++" {
#endif

constexpr vec3_t vec3_origin {0.0f, 0.0f, 0.0f};
constexpr int nanmask = 255 << 23;

inline bool IS_NAN(float x) { return ((*reinterpret_cast<int *>(&(x)) & nanmask) == nanmask); }

#if defined(CLIENT_DLL)
void	XASH_VECTORCALL VectorTransform(const vec3_t in1, cmatrix3x4 in2, vec3_t_ref out);
#else
inline void AngleMatrix(const vec3_t angles, matrix3x4_ref matrix)
{
	float angle;
	float sr, sp, sy, cr, cp, cy;

	angle = angles[YAW] * ( M_PI * 2 / 360 );
	sy = sin( angle );
	cy = cos( angle );
	angle = angles[PITCH] * ( M_PI * 2 / 360 );
	sp = sin( angle );
	cp = cos( angle );
	angle = angles[ROLL] * ( M_PI * 2 / 360 );
	sr = sin( angle );
	cr = cos( angle );

	// matrix = ( YAW * PITCH ) * ROLL
	matrix[0][0] = cp * cy;
	matrix[1][0] = cp * sy;
	matrix[2][0] = -sp;
	matrix[0][1] = sr * sp * cy + cr * -sy;
	matrix[1][1] = sr * sp * sy + cr * cy;
	matrix[2][1] = sr * cp;
	matrix[0][2] = ( cr * sp * cy + -sr * -sy );
	matrix[1][2] = ( cr * sp * sy + -sr * cy );
	matrix[2][2] = cr * cp;
	matrix[0][3] = 0.0;
	matrix[1][3] = 0.0;
	matrix[2][3] = 0.0;
}
#endif

inline void AngleIMatrix(const vec3_t angles, matrix3x4_ref matrix)
{
	float angle;
	float sr, sp, sy, cr, cp, cy;

	angle = angles[YAW] * ( M_PI * 2 / 360 );
	sy = sin( angle );
	cy = cos( angle );
	angle = angles[PITCH] * ( M_PI * 2 / 360 );
	sp = sin( angle );
	cp = cos( angle );
	angle = angles[ROLL] * ( M_PI * 2 / 360 );
	sr = sin( angle );
	cr = cos( angle );

	// matrix = ( YAW * PITCH ) * ROLL
	matrix[0][0] = cp * cy;
	matrix[0][1] = cp * sy;
	matrix[0][2] = -sp;
	matrix[1][0] = sr * sp * cy + cr * -sy;
	matrix[1][1] = sr * sp * sy + cr * cy;
	matrix[1][2] = sr * cp;
	matrix[2][0] = ( cr * sp * cy + -sr * -sy );
	matrix[2][1] = ( cr * sp * sy + -sr * cy );
	matrix[2][2] = cr * cp;
	matrix[0][3] = 0.0;
	matrix[1][3] = 0.0;
	matrix[2][3] = 0.0;
}

inline void NormalizeAngles(vec3_t_ref angles)
{
	int i;
	// Normalize angles
	for( i = 0; i < 3; i++ )
	{
		if( angles[i] > 180.0 )
		{
			angles[i] -= 360.0;
		}
		else if( angles[i] < -180.0 )
		{
			angles[i] += 360.0;
		}
	}
}

/*
===================
InterpolateAngles

Interpolate Euler angles.
FIXME:  Use Quaternions to avoid discontinuities
Frac is 0.0 to 1.0 ( i.e., should probably be clamped, but doesn't have to be )
===================
*/
inline void InterpolateAngles( vec3_t start, vec3_t end, vec3_t_ref output, float frac )
{
	int i;
	float ang1, ang2;
	float d;

	NormalizeAngles( start );
	NormalizeAngles( end );

	for( i = 0; i < 3; i++ )
	{
		ang1 = start[i];
		ang2 = end[i];

		d = ang2 - ang1;
		if( d > 180 )
		{
			d -= 360;
		}
		else if( d < -180 )
		{
			d += 360;
		}

		output[i] = ang1 + d * frac;
	}

	NormalizeAngles( output );
}

/*
===================
AngleBetweenVectors

===================
*/

#if defined(CLIENT_DLL)
void	XASH_VECTORCALL VectorTransform(const vec3_t in1, cmatrix3x4 in2, vec3_t_ref out);
#else
inline void VectorTransform(const vec3_t in1, cmatrix3x4 in2, vec3_t_ref out)
{
	out[0] = DotProduct( in1, vec3_t(in2[0][0], in2[0][1], in2[0][2]) ) + in2[0][3];
	out[1] = DotProduct( in1, vec3_t(in2[1][0], in2[1][1], in2[1][2]) ) + in2[1][3];
	out[2] = DotProduct( in1, vec3_t(in2[2][0], in2[2][1], in2[2][2]) ) + in2[2][3];
}
#endif

inline int   Q_log2(int val)
{
	int answer = 0;
	while( val >>= 1 )
		answer++;
	return answer;
}

inline void  VectorMatrix(const vec3_t forward, vec3_t_ref right, vec3_t_ref up)
{
	vec3_t tmp;

	if( forward[0] == 0 && forward[1] == 0 )
	{
		right[0] = 1;
		right[1] = 0;
		right[2] = 0;
		up[0] = -forward[2];
		up[1] = 0;
		up[2] = 0;
		return;
	}

	tmp[0] = 0; tmp[1] = 0; tmp[2] = 1.0;
	CrossProduct( forward, tmp, right );
	VectorNormalize( right );
	CrossProduct( right, forward, up );
	VectorNormalize( up );
}

}

#endif // PM_MATH_H
