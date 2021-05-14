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

inline void AngleVectors(const float *angles, float *forward, float *right, float *up)
{
	float angle;
	float sr, sp, sy, cr, cp, cy;

	angle = angles[YAW] * ( M_PI * 2 / 360 );
	sy = sin( angle );
	cy = cos( angle );
	angle = angles[PITCH] * ( M_PI*2 / 360 );
	sp = sin( angle );
	cp = cos( angle );
	angle = angles[ROLL] * ( M_PI*2 / 360 );
	sr = sin( angle );
	cr = cos( angle );

	if( forward )
	{
		forward[0] = cp * cy;
		forward[1] = cp * sy;
		forward[2] = -sp;
	}
	if( right )
	{
		right[0] = ( -1 * sr * sp * cy + -1 * cr * -sy );
		right[1] = ( -1 * sr * sp * sy + -1 * cr * cy );
		right[2] = -1 * sr * cp;
	}
	if( up )
	{
		up[0] = ( cr * sp * cy + -sr * -sy );
		up[1] = ( cr * sp * sy + -sr * cy );
		up[2] = cr * cp;
	}
}

inline void AngleVectorsTranspose(const float *angles, float *forward, float *right, float *up)
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

	if( forward )
	{
		forward[0] = cp * cy;
		forward[1] = ( sr * sp * cy + cr * -sy );
		forward[2] = ( cr * sp * cy + -sr * -sy );
	}
	if( right )
	{
		right[0] = cp * sy;
		right[1] = ( sr * sp * sy + cr * cy );
		right[2] = ( cr * sp * sy + -sr * cy );
	}
	if( up )
	{
		up[0] = -sp;
		up[1] = sr * cp;
		up[2] = cr * cp;
	}
}

inline void AngleMatrix(const float *angles, float (*matrix)[4])
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

inline void AngleIMatrix(const float *angles, float (*matrix)[4])
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

inline void NormalizeAngles(float *angles)
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
inline void InterpolateAngles( float *start, float *end, float *output, float frac )
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

inline float DotProduct(const float *v1, const float *v2)
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

inline float Length(const float *v)
{
	return sqrt(DotProduct(v, v));
}

/*
===================
AngleBetweenVectors

===================
*/
inline float AngleBetweenVectors( const float *v1, const float *v2 )
{
	float angle;
	float l1 = Length( v1 );
	float l2 = Length( v2 );

	if( !l1 || !l2 )
		return 0.0f;

	angle = acos( DotProduct( const_cast<float *>(v1), const_cast<float *>(v2) ) / ( l1 * l2 ) );
	angle = ( angle  * 180.0f ) / M_PI;

	return angle;
}

inline void VectorTransform(const float *in1, float (*in2)[4], float *out)
{
	out[0] = DotProduct( const_cast<float *>(in1), in2[0] ) + in2[0][3];
	out[1] = DotProduct( const_cast<float *>(in1), in2[1] ) + in2[1][3];
	out[2] = DotProduct( const_cast<float *>(in1), in2[2] ) + in2[2][3];
}

inline int VectorCompare(const float *v1, const float *v2)
{
	int i;

	for( i = 0; i < 3; i++ )
		if( v1[i] != v2[i] )
			return 0;

	return 1;
}

inline void  VectorMA(const float *veca, float scale, const float *vecb, float *vecc)
{
	vecc[0] = veca[0] + scale * vecb[0];
	vecc[1] = veca[1] + scale * vecb[1];
	vecc[2] = veca[2] + scale * vecb[2];
}

inline void  VectorSubtract(const float *veca, const float *vecb, float *out)
{
	out[0] = veca[0] - vecb[0];
	out[1] = veca[1] - vecb[1];
	out[2] = veca[2] - vecb[2];
}

inline void  VectorAdd(float *veca, float *vecb, float *out)
{
	out[0] = veca[0] + vecb[0];
	out[1] = veca[1] + vecb[1];
	out[2] = veca[2] + vecb[2];
}

inline void  VectorCopy(const float *in, float *out)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
}

inline void  CrossProduct(const float *v1, const float *v2, float *cross)
{
	cross[0] = v1[1] * v2[2] - v1[2] * v2[1];
	cross[1] = v1[2] * v2[0] - v1[0] * v2[2];
	cross[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

inline float Distance(const float *v1, const float *v2)
{
	vec3_t d;
	VectorSubtract( v2, v1, d );
	return Length( d );
}

inline float VectorNormalize(float *v)
{
	float length, ilength;

	length = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	length = sqrt( length );		// FIXME

	if( length )
	{
		ilength = 1 / length;
		v[0] *= ilength;
		v[1] *= ilength;
		v[2] *= ilength;
	}

	return length;
}

inline void  VectorInverse(float *v)
{
	v[0] = -v[0];
	v[1] = -v[1];
	v[2] = -v[2];
}

inline void  VectorScale(const float *in, float scale, float *out)
{
	out[0] = in[0] * scale;
	out[1] = in[1] * scale;
	out[2] = in[2] * scale;
}

inline int   Q_log2(int val)
{
	int answer = 0;
	while( val >>= 1 )
		answer++;
	return answer;
}

inline void  VectorMatrix(float *forward, float *right, float *up)
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

inline void  VectorAngles(const float *forward, float *angles)
{
	float tmp, yaw, pitch;

	if( forward[1] == 0 && forward[0] == 0 )
	{
		yaw = 0;
		if( forward[2] > 0 )
			pitch = 90;
		else
			pitch = 270;
	}
	else
	{
		yaw = ( atan2( forward[1], forward[0] ) * 180 / M_PI );
		if( yaw < 0 )
			yaw += 360;

		tmp = sqrt( forward[0] * forward[0] + forward[1] * forward[1] );
		pitch = ( atan2( forward[2], tmp ) * 180 / M_PI );
		if( pitch < 0 )
			pitch += 360;
	}

	angles[0] = pitch;
	angles[1] = yaw;
	angles[2] = 0;
}

inline void VectorClear(float *v)
{
	v[0] = v[1] =v[2] = 0.0f;
}

}

#endif // PM_MATH_H
