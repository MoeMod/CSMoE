/*
mathlib.h - base math functions
Copyright (C) 2007 Uncle Mike

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef MATHLIB_H
#define MATHLIB_H

#ifdef XASH_SIMD
#include <immintrin.h>
#endif
#include <math.h>

#ifdef MSC_VER
#pragma warning(disable : 4201)	// nonstandard extension used
#endif

#ifdef VECTORIZE_SINCOS
// on x86 it isn't so effective
#if defined(__SSE__) || defined(_M_IX86_FP) || defined(__SSE2__)
#define XASH_VECTORIZE_SINCOS
#endif

#if defined(__ARM_NEON__) || defined(__NEON__)
#define XASH_VECTORIZE_SINCOS
#endif
#endif


// euler angle order
#include "angledef.h"

#define SIDE_FRONT		0
#define SIDE_BACK		1
#define SIDE_ON		2
#define SIDE_CROSS		-2

#define PLANE_X		0	// 0 - 2 are axial planes
#define PLANE_Y		1	// 3 needs alternate calc
#define PLANE_Z		2
#define PLANE_NONAXIAL	3

#define EQUAL_EPSILON	0.001f
#define STOP_EPSILON	0.1f
#define ON_EPSILON		0.1f

#define RAD_TO_STUDIO	(32768.0 / M_PI)
#define STUDIO_TO_RAD	(M_PI / 32768.0)
#define nanmask		(255<<23)

template <class Type>
int Q_rint(const Type& x)
{
    return ((x) < 0 ? ((int)((x)-0.5f)) : ((int)((x)+0.5f)));
}

template <class Type>
bool IS_NAN(const Type& x)
{
    return (((*(int*)&x) & nanmask) == nanmask);
}

template <class VectorType>
bool VectorIsNAN(const VectorType& a)
{
    return (IS_NAN(a[0]) || IS_NAN(a[1]) || IS_NAN(a[2]));
}

template <class VectorTypeA, class VectorTypeB, class VectorTypeC>
void Vector2Subtract(const VectorTypeA& a, const VectorTypeB& b, VectorTypeC& c)
{
    (c)[0] = (a)[0] - (b)[0];
    (c)[1] = (a)[1] - (b)[1];
}

template <class VectorTypeA, class VectorTypeB, class VectorTypeC>
void Vector2Add(const VectorTypeA& a, const VectorTypeB& b, VectorTypeC& c)
{
    (c)[0] = (a)[0] + (b)[0];
    (c)[1] = (a)[1] + (b)[1];
}

template <class VectorTypeA, class VectorTypeB>
void Vector2Copy(const VectorTypeA& a, VectorTypeB& b)
{
    (b)[0] = (a)[0];
    (b)[1] = (a)[1];
}

template <class VectorTypeA, class VectorTypeB>
void Vector4Copy(const VectorTypeA& a, VectorTypeB& b)
{
    (b)[0] = (a)[0];
    (b)[1] = (a)[1];
    (b)[2] = (a)[2];
    (b)[3] = (a)[3];
}

template <class VectorTypeA, class VectorTypeB, class VectorTypeC>
void VectorDivide(const VectorTypeA& a, const VectorTypeB& b, VectorTypeC& c)
{
    VectorScale(a, (1.0f / (b)), c);
}

template <class VectorTypeA, class VectorTypeB, class VectorTypeC>
void Vector2Average(const VectorTypeA& a, const VectorTypeB& b, VectorTypeC& c)
{
    (c)[0] = ((a)[0] + (b)[0]) * 0.5;
    (c)[1] = ((a)[1] + (b)[1]) * 0.5;
}

template <class VectorTypeA, class VectorTypeB, class VectorTypeC>
void Vector2Set(VectorTypeA& v, const VectorTypeB& x, const VectorTypeC& y)
{
    (v)[0] = (x);
    (v)[1] = (y);
}

template <class VectorTypeA, class VectorTypeB, class VectorTypeC, class VectorTypeD, class VectorTypeE>
void Vector4Set(VectorTypeA& v, const VectorTypeB& a, const VectorTypeC& b, const VectorTypeD& c, const VectorTypeE& d)
{
    (v)[0] = (a);
    (v)[1] = (b);
    (v)[2] = (c);
    (v)[3] = (d);
}

template <class VectorTypeA, class VectorTypeB, class VectorTypeC, class VectorTypeD>
void Vector2Lerp(const VectorTypeA& v1, const VectorTypeB& lerp, const VectorTypeC& v2, VectorTypeD& c)
{
    (c)[0] = (v1)[0] + (lerp) * ((v2)[0] - (v1)[0]);
    (c)[1] = (v1)[1] + (lerp) * ((v2)[1] - (v1)[1]);
}

template <class Type>
bool Vector2IsNull(const Type& v)
{
    return ((v)[0] == 0.0f && (v)[1] == 0.0f);
}

// inner mathlib

float rsqrt(float number);
float anglemod(const float a);
word FloatToHalf(float v);
float HalfToFloat(word h);
int SignbitsForPlane(const vec3_t normal);
int NearestPOW(int value, qboolean roundDown);
void SinCos(float radians, float* sine, float* cosine);

float VectorNormalizeLength2(const vec3_t v, vec3_t_ref out);
void VectorVectors(const vec3_t forward, vec3_t_ref right, vec3_t_ref up);
void VectorAngles(const vec3_t forward, vec3_t_ref angles);
void AngleVectors(const vec3_t angles, vec3_t_ref forward, vec3_t_ref right, vec3_t_ref up);
void VectorsAngles(const vec3_t forward, const vec3_t right, const vec3_t up, vec3_t_ref angles);
void RotatePointAroundVector(vec3_t_ref dst, const vec3_t dir, const vec3_t point, float degrees);
vec3_t RotatePointAroundVector( const vec3_t dir, const vec3_t point, float degrees );

void ClearBounds(vec3_t_ref mins, vec3_t_ref maxs);
void AddPointToBounds(const vec3_t v, vec3_t_ref mins, vec3_t_ref maxs);
qboolean BoundsIntersect(const vec3_t mins1, const vec3_t maxs1, const vec3_t mins2, const vec3_t maxs2);
qboolean BoundsAndSphereIntersect(const vec3_t mins, const vec3_t maxs, const vec3_t origin, float radius);
float RadiusFromBounds(const vec3_t mins, const vec3_t maxs);

void AngleQuaternion(const vec3_t angles, vec4_t_ref q);
void QuaternionSlerp(const vec4_t p, vec4_t_ref q, float t, vec4_t_ref qt);
float RemapVal(float val, float A, float B, float C, float D);
float ApproachVal(float target, float value, float speed);
void InterpolateAngles(const vec3_t start, const vec3_t end, vec3_t_ref output, float frac);

#define VectorTransform1(a, b, f) ((b)[0]=f(a[0]),(b)[1]= f(a[1]),f(b)[2]=(a)[2])
#define VectorTransform2(a, b, c, f) ((c)[0] = f((a)[0], (b)[0]), (c)[1] = f((a)[1], (b)[1]), (c)[2] = f((a)[2], (b)[2]))
#define VectorCopySign(a, b, c) VectorTransform2(a, b, c, copysign)
#define VectorSgn(a, b) VectorCopySign(a, 1.0f, b)
#define VectorAbs(a, b) VectorTransform1(a, b, abs)
#define VectorMins(a, b) VectorTransform1(a, b, min)
#define VectorMaxs(a, b) VectorTransform1(a, b, max)
#define VectorClamp(a, b) VectorTransform1(a, b, clamp)

template <class VectorTypeA, class VectorTypeB>
auto DotProductAbs(const VectorTypeA& x, const VectorTypeB& y) -> decltype(abs((x)[0] * (y)[0]) + abs((x)[1] * (y)[1]) + abs((x)[2] * (y)[2]))
{
    return (abs((x)[0] * (y)[0]) + abs((x)[1] * (y)[1]) + abs((x)[2] * (y)[2]));
}

template <class VectorTypeA, class VectorTypeB>
auto DotProductFabs(const VectorTypeA& x, const VectorTypeB& y) -> decltype(fabs((x)[0] * (y)[0]) + fabs((x)[1] * (y)[1]) + fabs((x)[2] * (y)[2]))
{
    return (fabs((x)[0] * (y)[0]) + fabs((x)[1] * (y)[1]) + fabs((x)[2] * (y)[2]));
}

template <class VectorTypeA, class VectorTypeB>
auto DotProduct(const VectorTypeA& x, const VectorTypeB& y) -> decltype(((x)[0] * (y)[0] + (x)[1] * (y)[1] + (x)[2] * (y)[2]))
{
    return ((x)[0] * (y)[0] + (x)[1] * (y)[1] + (x)[2] * (y)[2]);
}

template <class VectorTypeA, class VectorTypeB, class VectorTypeC>
void VectorSubtract(const VectorTypeA& a, const VectorTypeB& b, VectorTypeC& c)
{
    (c)[0] = (a)[0] - (b)[0];
    (c)[1] = (a)[1] - (b)[1];
    (c)[2] = (a)[2] - (b)[2];
}

template <class VectorTypeA, class VectorTypeB, class VectorTypeC>
void VectorAdd(const VectorTypeA& a, const VectorTypeB& b, VectorTypeC& c)
{
    (c)[0] = (a)[0] + (b)[0];
    (c)[1] = (a)[1] + (b)[1];
    (c)[2] = (a)[2] + (b)[2];
}

template <class VectorTypeA, class VectorTypeB>
void VectorCopy(const VectorTypeA& a, VectorTypeB& b)
{
    (b)[0] = (a)[0];
    (b)[1] = (a)[1];
    (b)[2] = (a)[2];
}

template <class VectorTypeA>
void VectorCopy(const VectorTypeA& a, float *b)
{
    (b)[0] = (a)[0];
    (b)[1] = (a)[1];
    (b)[2] = (a)[2];
}

template <class VectorTypeA, class VectorTypeB, class VectorTypeC>
void VectorScale(const VectorTypeA& a, VectorTypeB b, VectorTypeC &c)
{
    (c)[0] = (a)[0] * (b);
    (c)[1] = (a)[1] * (b);
    (c)[2] = (a)[2] * (b);
}

template <class VectorTypeA, class VectorTypeB>
void VectorScale(const VectorTypeA& a, VectorTypeB b, float *c)
{
    (c)[0] = (a)[0] * (b);
    (c)[1] = (a)[1] * (b);
    (c)[2] = (a)[2] * (b);
}

template <class VectorTypeA, class VectorTypeB>
bool VectorCompare(const VectorTypeA& v1, const VectorTypeB& v2)
{
    return ((v1)[0] == (v2)[0] && (v1)[1] == (v2)[1] && (v1)[2] == (v2)[2]);
}

template <class VectorTypeA, class VectorTypeB, class VectorTypeC>
void VectorAverage(const VectorTypeA& a, const VectorTypeB& b, VectorTypeC& o)
{
    (o)[0] = ((a)[0] + (b)[0]) * 0.5;
    (o)[1] = ((a)[1] + (b)[1]) * 0.5;
    (o)[2] = ((a)[2] + (b)[2]) * 0.5;
}

template <class VectorTypeA, class VectorTypeB, class VectorTypeC, class VectorTypeD>
void VectorSet(VectorTypeA& v, const VectorTypeB& x, const VectorTypeC& y, const VectorTypeD& z)
{
    (v)[0] = (x);
    (v)[1] = (y);
    (v)[2] = (z);
}

template <class VectorTypeA, class VectorTypeB>
void VectorNegate(const VectorTypeA& x, VectorTypeB& y)
{
    (y)[0] = -(x)[0];
    (y)[1] = -(x)[1];
    (y)[2] = -(x)[2];
}

template <class VectorTypeA, class VectorTypeB, class VectorTypeC, class VectorTypeD>
void VectorMA(const VectorTypeA& a, const VectorTypeB& scale, const VectorTypeC& b, VectorTypeD& c)
{
    (c)[0] = (a)[0] + (scale) * (b)[0];
    (c)[1] = (a)[1] + (scale) * (b)[1];
    (c)[2] = (a)[2] + (scale) * (b)[2];
}

template <class VectorTypeA, class VectorTypeB, class VectorTypeC, class VectorTypeD, class VectorTypeE, class VectorTypeF, class OutType>
void VectorMAMAM(const VectorTypeA& scale1, const VectorTypeB& b1, const VectorTypeC& scale2, const VectorTypeD& b2, const VectorTypeE& scale3, const VectorTypeF& b3, OutType& c)
{
    (c)[0] = (scale1) * (b1)[0] + (scale2) * (b2)[0] + (scale3) * (b3)[0];
    (c)[1] = (scale1) * (b1)[1] + (scale2) * (b2)[1] + (scale3) * (b3)[1];
    (c)[2] = (scale1) * (b1)[2] + (scale2) * (b2)[2] + (scale3) * (b3)[2];
}

template <class VectorTypeA, class VectorTypeB, class VectorTypeC, class VectorTypeD, class VectorTypeE>
void MakeRGBA(VectorTypeA& out, const VectorTypeB& x, const VectorTypeC& y, const VectorTypeD& z, const VectorTypeE& w)
{
    Vector4Set(out, x, y, z, w);
}

template <class VectorTypeA, class VectorTypeB>
auto PlaneDist(const VectorTypeA& point, const VectorTypeB& plane) -> decltype((plane)->type < 3 ? (point)[(plane)->type] : DotProduct((point), (plane)->normal))
{
    return (plane)->type < 3 ? (point)[(plane)->type] : DotProduct((point), (plane)->normal);
}

template <class VectorTypeA, class VectorTypeB>
auto PlaneDiff(const VectorTypeA& point, const VectorTypeB& plane) -> decltype(((plane)->type < 3 ? (point)[(plane)->type] : DotProduct((point), (plane)->normal)) - (plane)->dist)
{
    return (((plane)->type < 3 ? (point)[(plane)->type] : DotProduct((point), (plane)->normal)) - (plane)->dist);
}

template <class T>
auto boundmax(T num, std::type_identity_t<T> high)
{
    using std::min;
    return min(num, high);
}

template <class T>
auto boundmin(T num, std::type_identity_t<T> low)
{
    using std::max;
    return max(num, low);
}

template <class T>
auto bound(std::type_identity_t<T> low, T num, std::type_identity_t<T> high)
{
    using std::clamp;
    return clamp(num, low, high);
}

#define VectorUnpack(v) (v)[0], (v)[1], (v)[2]

#ifdef XASH_VECTORIZE_SINCOS
void SinCosFastVector4(float r1, float r2, float r3, float r4,
    float* s0, float* s1, float* s2, float* s3,
    float* c0, float* c1, float* c2, float* c3)
#if defined(__GNUC__)
    __attribute__((nonnull))
#endif
    ;

void SinCosFastVector3(float r1, float r2, float r3,
    float* s0, float* s1, float* s2,
    float* c0, float* c1, float* c2)
#if defined(__GNUC__)
    __attribute__((nonnull))
#endif
    ;

void SinCosFastVector2(float r1, float r2,
    float* s0, float* s1,
    float* c0, float* c1)
#if defined(__GNUC__)
    __attribute__((nonnull))
#endif
    ;
#endif


//
// matrixlib.c
//

void Matrix3x4_VectorTransform( cmatrix3x4 in, const vec3_t v, vec3_t_ref out );
void Matrix3x4_VectorITransform( cmatrix3x4 in, const vec3_t v, vec3_t_ref out );
void Matrix3x4_VectorRotate( cmatrix3x4 in, const vec3_t v, vec3_t_ref out );
void Matrix3x4_VectorIRotate( cmatrix3x4 in, const vec3_t v, vec3_t_ref out );
void Matrix3x4_ConcatTransforms( matrix3x4_ref out, cmatrix3x4 in1, cmatrix3x4 in2 );
void Matrix3x4_FromOriginQuat( matrix3x4_ref out, const vec4_t quaternion, const vec3_t origin );
void Matrix3x4_CreateFromEntity( matrix3x4_ref out, const vec3_t angles, const vec3_t origin, float scale );
void Matrix3x4_TransformPositivePlane( cmatrix3x4 in, const vec3_t normal, float d, vec3_t_ref out, float *dist );
void Matrix3x4_SetOrigin( matrix3x4_ref out, float x, float y, float z );
void Matrix3x4_Invert_Simple( matrix3x4_ref out, cmatrix3x4 in1 );
void Matrix3x4_OriginFromMatrix( cmatrix3x4 in, vec3_t_ref out );

void Matrix4x4_VectorTransform( cmatrix4x4 in, const vec3_t v, vec3_t_ref out );
void Matrix4x4_VectorITransform( cmatrix4x4 in, const vec3_t v, vec3_t_ref out );
void Matrix4x4_VectorRotate( cmatrix4x4 in, const vec3_t v, vec3_t_ref out );
void Matrix4x4_VectorIRotate( cmatrix4x4 in, const vec3_t v, vec3_t_ref out );
void Matrix4x4_ConcatTransforms( matrix4x4_ref out, cmatrix4x4 in1, cmatrix4x4 in2 );
void Matrix4x4_FromOriginQuat( matrix4x4_ref out, const vec4_t quaternion, const vec3_t origin );
void Matrix4x4_CreateFromEntity( matrix4x4_ref out, const vec3_t angles, const vec3_t origin, float scale );
void Matrix4x4_TransformPositivePlane( cmatrix4x4 in, const vec3_t normal, float d, vec3_t_ref out, float *dist );
vec3_t Matrix4x4_TransformPositivePlane( cmatrix4x4 in, const vec3_t normal, float d, float *dist );
void Matrix4x4_TransformStandardPlane( cmatrix4x4 in, const vec3_t normal, float d, vec3_t_ref out, float *dist );
void Matrix4x4_ConvertToEntity( cmatrix4x4 in, vec3_t_ref angles, vec3_t_ref origin );
void Matrix4x4_SetOrigin( matrix4x4_ref out, float x, float y, float z );
void Matrix4x4_Invert_Simple( matrix4x4_ref out, cmatrix4x4 in1 );
void Matrix4x4_OriginFromMatrix( cmatrix4x4 in, vec3_t_ref out );
void Matrix4x4_Transpose( matrix4x4_ref out, cmatrix4x4 in1 );
qboolean Matrix4x4_Invert_Full( matrix4x4_ref out, cmatrix4x4 in1 );

constexpr vec3_t		vec3_origin = { 0, 0, 0 };

inline void Matrix3x4_Copy( matrix3x4_ref out, cmatrix3x4 in ) { out = in; }
void Matrix3x4_LoadIdentity( matrix3x4_ref mat );
inline void Matrix4x4_Copy( matrix4x4_ref out, cmatrix4x4 in ) { out = in; }
void Matrix4x4_LoadIdentity( matrix4x4_ref mat );

#endif//MATHLIB_H
