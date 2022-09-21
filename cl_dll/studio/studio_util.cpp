//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include <memory.h>
#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "com_model.h"
#include "studio_util.h"

namespace cl {

/*
================
ConcatTransforms

================
*/
void ConcatTransforms(cmatrix3x4 in1, cmatrix3x4 in2, matrix3x4_ref out)
{
	out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] +
		in1[0][2] * in2[2][0];
	out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] +
		in1[0][2] * in2[2][1];
	out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] +
		in1[0][2] * in2[2][2];
	out[0][3] = in1[0][0] * in2[0][3] + in1[0][1] * in2[1][3] +
		in1[0][2] * in2[2][3] + in1[0][3];
	out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] +
		in1[1][2] * in2[2][0];
	out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] +
		in1[1][2] * in2[2][1];
	out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] +
		in1[1][2] * in2[2][2];
	out[1][3] = in1[1][0] * in2[0][3] + in1[1][1] * in2[1][3] +
		in1[1][2] * in2[2][3] + in1[1][3];
	out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] +
		in1[2][2] * in2[2][0];
	out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] +
		in1[2][2] * in2[2][1];
	out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] +
		in1[2][2] * in2[2][2];
	out[2][3] = in1[2][0] * in2[0][3] + in1[2][1] * in2[1][3] +
		in1[2][2] * in2[2][3] + in1[2][3];
}

// angles index are not the same as ROLL, PITCH, YAW

/*
====================
AngleQuaternion

====================
*/

#ifdef U_VECTOR_SSE
const __m128 AngleQuaternion_sign1 = _mm_castsi128_ps(_mm_setr_epi32(0x80000000, 0x00000000, 0x00000000, 0x00000000));
const __m128 AngleQuaternion_sign2 = _mm_castsi128_ps(_mm_setr_epi32(0x00000000, 0x00000000, 0x80000000, 0x00000000));
const __m128 AngleQuaternion_sign1f = _mm_setr_ps(-1.0f, 1.0f, 1.0f, 1.0f);
const __m128 AngleQuaternion_sign2f = _mm_setr_ps(1.0f, 1.0f, -1.0f, 1.0f);
#endif
void AngleQuaternion(const vec3_t angles, vec4_t_ref quaternion)
{
#ifdef U_VECTOR_SSE
	__m128 sina, cosa;
	sincos_ps(angles * 0.5, &sina, &cosa);
	auto sr_sr_sr_sr = _mm_shuffle_ps(sina, sina, _MM_SHUFFLE(0, 0, 0, 0));
	auto cr_cr_cr_cr = _mm_shuffle_ps(cosa, cosa, _MM_SHUFFLE(0, 0, 0, 0));
	auto sy_sy_cy_cy = _mm_shuffle_ps(sina, cosa, _MM_SHUFFLE(2, 2, 2, 2));
	auto sp_sp_cp_cp = _mm_shuffle_ps(sina, cosa, _MM_SHUFFLE(1, 1, 1, 1));
	auto sy_cy_sy_cy = _mm_shuffle_ps(sy_sy_cy_cy, sy_sy_cy_cy, _MM_SHUFFLE(2, 0, 2, 0));
	auto cp_cp_sp_sp = _mm_shuffle_ps(cosa, sina, _MM_SHUFFLE(1, 1, 1, 1));
	auto cy_sy_cy_sy = _mm_shuffle_ps(sy_sy_cy_cy, sy_sy_cy_cy, _MM_SHUFFLE(0, 2, 0, 2));
	auto left = _mm_xor_ps(AngleQuaternion_sign1, _mm_mul_ps(cr_cr_cr_cr, _mm_mul_ps(sp_sp_cp_cp, sy_cy_sy_cy)));
	auto right = _mm_xor_ps(AngleQuaternion_sign2, _mm_mul_ps(sr_sr_sr_sr, _mm_mul_ps(cp_cp_sp_sp, cy_sy_cy_sy)));
	quaternion = _mm_add_ps(left, right);
#elif defined U_VECTOR_SIMD
	vec3_t sina, cosa;
	float sr, sp, sy, cr, cp, cy;
	sincos_ps(angles * 0.5, &sina.m_xmm, &cosa.m_xmm);
	sy = sina[2];
	cy = cosa[2];
	sp = sina[1];
	cp = cosa[1];
	sr = sina[0];
	cr = cosa[0];
	quaternion[0] = sr * cp * cy - cr * sp * sy; // X
	quaternion[1] = cr * sp * cy + sr * cp * sy; // Y
	quaternion[2] = cr * cp * sy - sr * sp * cy; // Z
	quaternion[3] = cr * cp * cy + sr * sp * sy; // W
#else
	float angle;
	float sr, sp, sy, cr, cp, cy;
	// FIXME: rescale the inputs to 1/2 angle
	angle = angles[2] * 0.5;
	sy = sin(angle);
	cy = cos(angle);
	angle = angles[1] * 0.5;
	sp = sin(angle);
	cp = cos(angle);
	angle = angles[0] * 0.5;
	sr = sin(angle);
	cr = cos(angle);
	quaternion[0] = sr * cp * cy - cr * sp * sy; // X
	quaternion[1] = cr * sp * cy + sr * cp * sy; // Y
	quaternion[2] = cr * cp * sy - sr * sp * cy; // Z
	quaternion[3] = cr * cp * cy + sr * sp * sy; // W
#endif
}

/*
====================
QuaternionSlerp

====================
*/
void QuaternionSlerp(const vec4_t p, vec4_t_ref q, float t, vec4_t_ref qt)
{
	int i;
	float cosom, sclp, sclq;

    // decide if one of the quaternions is backwards
    float a = (p-q).LengthSquared();
    float b = (p+q).LengthSquared();

	if (a > b)
	{
        q = -q;
	}

	cosom = DotProduct(p, q);

	if ((1.0 + cosom) > 0.000001)
	{
		if ((1.0 - cosom) > 0.000001)
		{
			float omega = acos(cosom);
#ifdef U_VECTOR_SIMD
            vec3_t x = { omega, (1.0f - t) * omega, t * omega };
            vec3_t sinx = sin_ps(x);
            float sinom = sinx[0];
            sclp = sinx[1] / sinom;
            sclq = sinx[2] / sinom;
#else
            float sinom = sin(omega);

			sclp = sin((1.0 - t) * omega) / sinom;
			sclq = sin(t * omega) / sinom;
#endif
		}
		else
		{
			sclp = 1.0 - t;
			sclq = t;
		}

        qt = sclp * p + sclq * q;
	}
	else
	{
		qt[0] = -q[1];
		qt[1] = q[0];
		qt[2] = -q[3];
		qt[3] = q[2];

		sclp = sin((1.0 - t) * (0.5 * M_PI));
		sclq = sin(t * (0.5 * M_PI));

        qt = sclp * p + sclq * qt;
	}
}

/*
====================
QuaternionMatrix

====================
*/
void QuaternionMatrix(const vec4_t quaternion, matrix3x4_ref matrix)
{
	matrix[0][0] = 1.0 - 2.0 * quaternion[1] * quaternion[1] - 2.0 * quaternion[2] * quaternion[2];
	matrix[1][0] = 2.0 * quaternion[0] * quaternion[1] + 2.0 * quaternion[3] * quaternion[2];
	matrix[2][0] = 2.0 * quaternion[0] * quaternion[2] - 2.0 * quaternion[3] * quaternion[1];

	matrix[0][1] = 2.0 * quaternion[0] * quaternion[1] - 2.0 * quaternion[3] * quaternion[2];
	matrix[1][1] = 1.0 - 2.0 * quaternion[0] * quaternion[0] - 2.0 * quaternion[2] * quaternion[2];
	matrix[2][1] = 2.0 * quaternion[1] * quaternion[2] + 2.0 * quaternion[3] * quaternion[0];

	matrix[0][2] = 2.0 * quaternion[0] * quaternion[2] + 2.0 * quaternion[3] * quaternion[1];
	matrix[1][2] = 2.0 * quaternion[1] * quaternion[2] - 2.0 * quaternion[3] * quaternion[0];
	matrix[2][2] = 1.0 - 2.0 * quaternion[0] * quaternion[0] - 2.0 * quaternion[1] * quaternion[1];
}

/*
====================
MatrixCopy

====================
*/
void MatrixCopy(cmatrix3x4 in, matrix3x4_ref out)
{
    out = in;
}

}
