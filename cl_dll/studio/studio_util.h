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
//void    AngleMatrix (const float *angles, float (*matrix)[4]);
//int		VectorCompare (const float *v1, const float *v2);
//void	CrossProduct (const float *v1, const float *v2, float *cross);
//void	VectorTransform (const float *in1, float in2[3][4], float *out);
void	ConcatTransforms (float in1[3][4], float in2[3][4], float out[3][4]);
void	MatrixCopy( float in[3][4], float out[3][4] );
void	QuaternionMatrix( vec4_t quaternion, float (*matrix)[4] );
void	QuaternionSlerp( vec4_t p, vec4_t q, float t, vec4_t qt );
void	AngleQuaternion( float *angles, vec4_t quaternion );
}

#endif // STUDIO_UTIL_H