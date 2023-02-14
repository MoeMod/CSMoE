/*
studio_util.h
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
#if !defined( STUDIO_UTIL_H )
#define STUDIO_UTIL_H

#include "angledef.h"

template<class VectorTypeA, class VectorTypeB>
auto FDotProduct(VectorTypeA a, VectorTypeB b) -> decltype(fabs((a[0])*(b[0])) + fabs((a[1])*(b[1])) + fabs((a[2])*(b[2])))
{
	return fabs((a[0])*(b[0])) + fabs((a[1])*(b[1])) + fabs((a[2])*(b[2]));
}

namespace cl {
void    XASH_VECTORCALL AngleMatrix (const vec3_t angles, matrix3x4_ref matrix);
int		XASH_VECTORCALL VectorCompare (const vec3_t v1, const vec3_t v2);
void	XASH_VECTORCALL CrossProduct (const vec3_t v1, const vec3_t v2, vec3_t_ref cross);
void	XASH_VECTORCALL VectorTransform (const vec3_t in1, cmatrix3x4 in2, vec3_t_ref out);

void	XASH_VECTORCALL ConcatTransforms (cmatrix3x4 in1, cmatrix3x4 in2, matrix3x4_ref out);
void	XASH_VECTORCALL MatrixCopy( cmatrix3x4 in, matrix3x4_ref out );
void	XASH_VECTORCALL QuaternionMatrix( const vec4_t quaternion, matrix3x4_ref matrix );
void	XASH_VECTORCALL QuaternionSlerp( const vec4_t p, vec4_t q, float t, vec4_t_ref qt );
void    XASH_VECTORCALL QuaternionSlerpX4(const vec4_t p[4], vec4_t q[4], float t, vec4_t qt[4]);
void	XASH_VECTORCALL AngleQuaternion( const vec3_t angles, vec4_t_ref quaternion );
}

#endif // STUDIO_UTIL_H