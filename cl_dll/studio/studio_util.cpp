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
====================
AngleMatrix
====================
*/
void XASH_VECTORCALL AngleMatrix(const vec3_t angles, matrix3x4_ref matrix)
{
#if defined(XASH_SIMD) && U_VECTOR_NEON
    float32x4x2_t sp_sy_sr_0_cp_cy_cr_1;
    sincos_ps(angles * (M_PI*2 / 360), &sp_sy_sr_0_cp_cy_cr_1.val[0], &sp_sy_sr_0_cp_cy_cr_1.val[1]);

    auto sp_sr_cp_cr_sy_0_cy_1 = vuzpq_f32(sp_sy_sr_0_cp_cy_cr_1.val[0], sp_sy_sr_0_cp_cy_cr_1.val[1]);
    auto sp_cp_sy_cy_sr_cr_0_1 = vzipq_f32(sp_sy_sr_0_cp_cy_cr_1.val[0], sp_sy_sr_0_cp_cy_cr_1.val[1]);

    auto _0_sr_cr_0 = vextq_f32(sp_sy_sr_0_cp_cy_cr_1.val[0], sp_cp_sy_cy_sr_cr_0_1.val[1], 3);
    auto cp_cr_sr_0 = vcombine_f32(vget_high_f32(sp_sr_cp_cr_sy_0_cy_1.val[0]), vget_high_f32(sp_sy_sr_0_cp_cy_cr_1.val[0]));
    auto cy_sy_sy_0 = vcombine_f32(vrev64_f32(vget_high_f32(sp_cp_sy_cy_sr_cr_0_1.val[0])), vget_low_f32(sp_sr_cp_cr_sy_0_cy_1.val[1]));
    auto sy_cy_cy_1 = vcombine_f32(vget_high_f32(sp_cp_sy_cy_sr_cr_0_1.val[0]), vget_high_f32(sp_sr_cp_cr_sy_0_cy_1.val[1]));

    auto _0_srsp_crsp_0 = vmulq_laneq_f32(_0_sr_cr_0, sp_sy_sr_0_cp_cy_cr_1.val[0], 0); // *sp
    matrix[0] = vmulq_laneq_f32(_0_srsp_crsp_0, sp_sy_sr_0_cp_cy_cr_1.val[1], 1); // *cy
    matrix[1] = vmulq_laneq_f32(_0_srsp_crsp_0, sp_sy_sr_0_cp_cy_cr_1.val[0], 1); // *sy

    const auto sign0 = vreinterpretq_f32_u32(vsetq_lane_u32(0x80000000, vdupq_n_u32(0), 0));
    const auto sign1 = vreinterpretq_f32_u32(vsetq_lane_u32(0x80000000, vdupq_n_u32(0), 1));
    const auto sign2 = vreinterpretq_f32_u32(vsetq_lane_u32(0x80000000, vdupq_n_u32(0), 2));

    cy_sy_sy_0 = vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(cy_sy_sy_0), sign1));
    sy_cy_cy_1 = vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(sy_cy_cy_1), sign2));
    matrix[0] = vfmaq_f32(matrix[0], cp_cr_sr_0, cy_sy_sy_0);
    matrix[1] = vfmaq_f32(matrix[1], cp_cr_sr_0, sy_cy_cy_1);

    auto cp_cr_0_1 = vcombine_f32(vget_high_f32(sp_sr_cp_cr_sy_0_cy_1.val[0]), vget_high_f32(sp_cp_sy_cy_sr_cr_0_1.val[1]));
    auto _1_cp_cr_0 = vextq_f32(cp_cr_0_1, cp_cr_0_1, 3);
    matrix[2] = vmulq_f32(sp_sr_cp_cr_sy_0_cy_1.val[0], _1_cp_cr_0);
    matrix[2] = vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(matrix[2]), sign0));
/*
    matrix[0][0] =          cp*cy;
    matrix[0][1] = sr*sp*cy-cr*sy;
    matrix[0][2] = cr*sp*cy+sr*sy;
    matrix[0][3] = 0.0;
    matrix[1][0] =          cp*sy;
    matrix[1][1] = sr*sp*sy+cr*cy;
    matrix[1][2] = cr*sp*sy-sr*cy;
    matrix[1][3] = 0.0;
    matrix[2][0] = -sp*1;
    matrix[2][1] = sr*cp;
    matrix[2][2] = cp*cr;
    matrix[2][3] = cr*0;
    */
#else
    float		angle;
    float		sr, sp, sy, cr, cp, cy;

    angle = angles[YAW] * (M_PI*2 / 360);
    sy = sin(angle);
    cy = cos(angle);
    angle = angles[PITCH] * (M_PI*2 / 360);
    sp = sin(angle);
    cp = cos(angle);
    angle = angles[ROLL] * (M_PI*2 / 360);
    sr = sin(angle);
    cr = cos(angle);

    // matrix = (YAW * PITCH) * ROLL
    matrix[0][0] = cp*cy;
    matrix[1][0] = cp*sy;
    matrix[2][0] = -sp;
    matrix[0][1] = sr*sp*cy+cr*-sy;
    matrix[1][1] = sr*sp*sy+cr*cy;
    matrix[2][1] = sr*cp;
    matrix[0][2] = (cr*sp*cy+-sr*-sy);
    matrix[1][2] = (cr*sp*sy+-sr*cy);
    matrix[2][2] = cr*cp;
    matrix[0][3] = 0.0;
    matrix[1][3] = 0.0;
    matrix[2][3] = 0.0;
#endif
}

/*
====================
VectorCompare
====================
*/
int	XASH_VECTORCALL VectorCompare (const vec3_t v1, const vec3_t v2)
{
#if defined(XASH_SIMD) && U_VECTOR_NEON
    return !vaddvq_u32(vceqq_f32(v1, v2));
#elif defined(XASH_SIMD) && U_VECTOR_SSE
    return 0xF == _mm_movemask_ps(_mm_cmpeq_ps(v1, v2));
#else
    int		i;

    for (i=0 ; i<3 ; i++)
        if (v1[i] != v2[i])
            return 0;

    return 1;
#endif
}

/*
====================
CrossProduct
====================
*/
void XASH_VECTORCALL CrossProduct (const vec3_t v1, const vec3_t v2, vec3_t_ref cross)
{
#if defined(XASH_SIMD) && U_VECTOR_NEON
    float32x4_t yzxy_a = vextq_f32(vextq_f32(v1, v1, 3), v1, 2); // [aj, ak, ai, aj]
    float32x4_t yzxy_b = vextq_f32(vextq_f32(v2, v2, 3), v2, 2); // [bj, bk, bi, bj]
    float32x4_t zxyy_a = vextq_f32(yzxy_a, yzxy_a, 1); // [ak, ai, aj, aj]
    float32x4_t zxyy_b = vextq_f32(yzxy_b, yzxy_b, 1); // [bk, ai, bj, bj]
    cross = vfmsq_f32(vmulq_f32(yzxy_a, zxyy_b), zxyy_a, yzxy_b); // [ajbk-akbj, akbi-aibk, aibj-ajbi, 0]
#elif defined(XASH_SIMD) && U_VECTOR_SSE
    __m128 result = _mm_sub_ps(
        _mm_mul_ps(v1, _mm_shuffle_ps(v1, v1, _MM_SHUFFLE(3, 0, 2, 1))),
        _mm_mul_ps(v2, _mm_shuffle_ps(v2, v2, _MM_SHUFFLE(3, 0, 2, 1)))
    );
    cross = _mm_shuffle_ps(result, result, _MM_SHUFFLE(3, 0, 2, 1));
#else
    cross[0] = v1[1]*v2[2] - v1[2]*v2[1];
    cross[1] = v1[2]*v2[0] - v1[0]*v2[2];
    cross[2] = v1[0]*v2[1] - v1[1]*v2[0];
#endif
}

/*
====================
VectorTransform
====================
*/
void XASH_VECTORCALL VectorTransform (const vec3_t in1, cmatrix3x4 in2, vec3_t_ref out)
{
#if defined(XASH_SIMD) && U_VECTOR_NEON
    float32x4x4_t in_t;
    memcpy(&in_t, &in2, sizeof(in2));
    //memset(&in_t.val[3], 0, sizeof(in_t.val[3]));
    in_t = vld4q_f32((const float *)&in_t);

    out = in_t.val[3];
    out = vfmaq_laneq_f32(out, in_t.val[0], in1, 0);
    out = vfmaq_laneq_f32(out, in_t.val[1], in1, 1);
    out = vfmaq_laneq_f32(out, in_t.val[2], in1, 2);
#else
    out[0] = DotProduct(in1, in2[0]) + in2[0][3];
    out[1] = DotProduct(in1, in2[1]) + in2[1][3];
    out[2] = DotProduct(in1, in2[2]) + in2[2][3];
#endif
}

/*
================
ConcatTransforms

================
*/
void XASH_VECTORCALL ConcatTransforms(cmatrix3x4 in1, cmatrix3x4 in2, matrix3x4_ref out)
{
#if defined(XASH_SIMD) && U_VECTOR_NEON
    float32x4x3_t out_reg = {};

    out_reg.val[0] = vcopyq_laneq_f32(out_reg.val[0], 3, in1[0], 3); // out[0][3] = in[0][3]
    out_reg.val[0] = vfmaq_laneq_f32(out_reg.val[0], in2[0], in1[0], 0); // out[0][n] += in2[0][n] * in1[0][0]
    out_reg.val[0] = vfmaq_laneq_f32(out_reg.val[0], in2[1], in1[0], 1); // out[0][n] += in2[1][n] * in1[0][1]
    out_reg.val[0] = vfmaq_laneq_f32(out_reg.val[0], in2[2], in1[0], 2); // out[0][n] += in2[2][n] * in1[0][2]

    out_reg.val[1] = vcopyq_laneq_f32(out_reg.val[1], 3, in1[1], 3);
    out_reg.val[1] = vfmaq_laneq_f32(out_reg.val[1], in2[0], in1[1], 0);
    out_reg.val[1] = vfmaq_laneq_f32(out_reg.val[1], in2[1], in1[1], 1);
    out_reg.val[1] = vfmaq_laneq_f32(out_reg.val[1], in2[2], in1[1], 2);

    out_reg.val[2] = vcopyq_laneq_f32(out_reg.val[2], 3, in1[2], 3);
    out_reg.val[2] = vfmaq_laneq_f32(out_reg.val[2], in2[0], in1[2], 0);
    out_reg.val[2] = vfmaq_laneq_f32(out_reg.val[2], in2[1], in1[2], 1);
    out_reg.val[2] = vfmaq_laneq_f32(out_reg.val[2], in2[2], in1[2], 2);

    memcpy(&out, &out_reg, sizeof(out));
#else
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
#endif
}

// angles index are not the same as ROLL, PITCH, YAW

/*
====================
AngleQuaternion

====================
*/

#if defined(XASH_SIMD) && U_VECTOR_SSE
const __m128 AngleQuaternion_sign1 = _mm_castsi128_ps(_mm_setr_epi32(0x80000000, 0x00000000, 0x00000000, 0x00000000));
const __m128 AngleQuaternion_sign2 = _mm_castsi128_ps(_mm_setr_epi32(0x00000000, 0x00000000, 0x80000000, 0x00000000));
#elif defined(XASH_SIMD) && U_VECTOR_NEON
const float32x4_t AngleQuaternion_sign2 = vzipq_f32(vdupq_n_u32(0x80000000), vdupq_n_u32(0x00000000)).val[0]; // { 0x80000000, 0x00000000, 0x80000000, 0x00000000 };
#endif
void XASH_VECTORCALL AngleQuaternion(const vec3_t angles, vec4_t_ref quaternion)
{
#if defined(XASH_SIMD) && U_VECTOR_SSE
	__m128 sina, cosa;
	sincos_ps(angles * 0.5, &sina, &cosa);
	auto sr_sr_sr_sr = _mm_shuffle_ps(sina, sina, _MM_SHUFFLE(0, 0, 0, 0));
	auto cr_cr_cr_cr = _mm_shuffle_ps(cosa, cosa, _MM_SHUFFLE(0, 0, 0, 0));
	auto sy_sy_cy_cy = _mm_shuffle_ps(sina, cosa, _MM_SHUFFLE(2, 2, 2, 2));
	auto sp_sp_cp_cp = _mm_shuffle_ps(sina, cosa, _MM_SHUFFLE(1, 1, 1, 1));
	auto cp_cp_sp_sp = _mm_shuffle_ps(cosa, sina, _MM_SHUFFLE(1, 1, 1, 1));

	auto sy_cy_sy_cy = _mm_shuffle_ps(sy_sy_cy_cy, sy_sy_cy_cy, _MM_SHUFFLE(2, 0, 2, 0));
	auto cy_sy_cy_sy = _mm_shuffle_ps(sy_sy_cy_cy, sy_sy_cy_cy, _MM_SHUFFLE(0, 2, 0, 2));

	auto left = _mm_xor_ps(AngleQuaternion_sign1, _mm_mul_ps(cr_cr_cr_cr, _mm_mul_ps(sp_sp_cp_cp, sy_cy_sy_cy)));
	auto right = _mm_xor_ps(AngleQuaternion_sign2, _mm_mul_ps(sr_sr_sr_sr, _mm_mul_ps(cp_cp_sp_sp, cy_sy_cy_sy)));

	quaternion = _mm_add_ps(left, right);
	//quaternion[0] = - cr * sp * sy + sr * cp * cy; // X
	//quaternion[1] =   cr * sp * cy + sr * cp * sy; // Y
	//quaternion[2] =   cr * cp * sy - sr * sp * cy; // Z
	//quaternion[3] =   cr * cp * cy + sr * sp * sy; // W
#elif defined(XASH_SIMD) && U_VECTOR_NEON
    float32x4x2_t sr_sp_sy_0_cr_cp_cy_1;
    sincos_ps(angles * 0.5, &sr_sp_sy_0_cr_cp_cy_1.val[0], &sr_sp_sy_0_cr_cp_cy_1.val[1]);

    auto sr_sy_cr_cy_sp_0_cp_1 = vuzpq_f32(sr_sp_sy_0_cr_cp_cy_1.val[0], sr_sp_sy_0_cr_cp_cy_1.val[1]);
    float32x4_t cp_cp_cp_cp = vdupq_laneq_f32(sr_sp_sy_0_cr_cp_cy_1.val[1], 1);
    float32x4_t sp_sp_sp_sp = vdupq_laneq_f32(sr_sp_sy_0_cr_cp_cy_1.val[0], 1);

    float32x4_t sr_sy_cr_cy = sr_sy_cr_cy_sp_0_cp_1.val[0];
    float32x4_t sy_cr_cy_sr = vextq_f32(sr_sy_cr_cy_sp_0_cp_1.val[0], sr_sy_cr_cy_sp_0_cp_1.val[0], 1);
    float32x4_t cr_cy_sr_sy = vextq_f32(sr_sy_cr_cy_sp_0_cp_1.val[0], sr_sy_cr_cy_sp_0_cp_1.val[0], 2);
    float32x4_t cy_sr_sy_cr = vextq_f32(sr_sy_cr_cy_sp_0_cp_1.val[0], sr_sy_cr_cy_sp_0_cp_1.val[0], 3);
    float32x4_t sp_sp_sp_sp_signed = veorq_u32(sp_sp_sp_sp, AngleQuaternion_sign2);

    auto left = vmulq_f32(vmulq_f32(sr_sy_cr_cy, cp_cp_cp_cp), cy_sr_sy_cr);

    quaternion = vfmaq_f32(left, vmulq_f32(cr_cy_sr_sy, sp_sp_sp_sp_signed), sy_cr_cy_sr);
    //quaternion[0] =   sr * cp * cy - cr * sp * sy; // X
    //quaternion[1] =   sy * cp * sr + cy * sp * cr; // Y
    //quaternion[2] =   cr * cp * sy - sr * sp * cy; // Z
    //quaternion[3] =   cy * cp * cr + sy * sp * sr; // W
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
void XASH_VECTORCALL QuaternionSlerp(const vec4_t p, vec4_t q, float t, vec4_t_ref qt)
{
#if defined(XASH_SIMD) && U_VECTOR_NEON
    // q = (cos(a/2), xsin(a/2), ysin(a/2), zsin(a/2))
    // cos(a-b) = cosacosb+sinasinb
    const uint32x4_t signmask = vdupq_n_u32(0x80000000);
    const float32x4_t one_minus_epsilon = vdupq_n_f32(1.0f - 0.00001f);

    float32x4_t vcosom = vdupq_n_f32(DotProduct(p, q));
    uint32x4_t sign =  vandq_u32(vreinterpretq_u32_f32(vcosom), signmask);
    q = vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(q), sign));
    vcosom = vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(vcosom), sign));

    vec4_t x = { (1.0f - t), t, 1, 0 }; // cosom -> 1, sinom -> 0, sinx ~ x

    // if ((1.0 - cosom) > 0.000001) x = sin(x * omega)
    uint32x4_t cosom_less_then_one = vcltq_f32(vcosom, one_minus_epsilon);
    float32x4_t vomega = acos_ps(vcosom);
    x = vbslq_f32(cosom_less_then_one, x, sin_ps(vmulq_f32(x, vomega)));

    // qt = (x[0] * p + x[1] * q) / x[2];
    qt = vmulq_laneq_f32(p, x, 0);
    qt = vfmaq_laneq_f32(qt, q, x, 1);
    qt = vdivq_f32(qt, vdupq_laneq_f32(x, 2)); // vdivq_laneq_f32 ?
#else
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
#ifdef XASH_SIMD
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
#endif
}

void XASH_VECTORCALL QuaternionSlerpX4(const vec4_t p[4], vec4_t q[4], float t, vec4_t qt[4])
{
#if defined(XASH_SIMD) && U_VECTOR_NEON
    //float32x4_t cosom = { DotProduct(p[0], q[0]), DotProduct(p[1], q[1]), DotProduct(p[2], q[2]), DotProduct(p[3], q[3]) };
    float32x4x4_t p_t = vld4q_f32((const float *)p);
    float32x4x4_t q_t = vld4q_f32((const float *)q);
    float32x4_t cosom = vmulq_f32(p_t.val[0], q_t.val[0]);
    cosom = vfmaq_f32(cosom, p_t.val[1], q_t.val[1]);
    cosom = vfmaq_f32(cosom, p_t.val[2], q_t.val[2]);
    cosom = vfmaq_f32(cosom, p_t.val[3], q_t.val[3]);

    // if(cosom < 0) q=-q, cosom=-cosom
    uint32x4_t sign = vandq_u32(vreinterpretq_u32_f32(cosom), vdupq_n_u32(0x80000000));
    q[0] = vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(q[0]), vdupq_laneq_f32(sign, 0)));
    q[1] = vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(q[1]), vdupq_laneq_f32(sign, 1)));
    q[2] = vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(q[2]), vdupq_laneq_f32(sign, 2)));
    q[3] = vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(q[3]), vdupq_laneq_f32(sign, 3)));
    cosom = vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(cosom), sign));

    float32x4_t sclp = vdupq_n_f32(1.0f - t);
    float32x4_t sclq = vdupq_n_f32(t);
    // if ((1.0 - cosom) > 0.000001) scl = sin(scl * omega)
    uint32x4_t cosom_less_then_one = vcltq_f32(cosom, vdupq_n_f32(1.0f - 0.00001f));
    float32x4_t omega = acos_ps(cosom);
    sclp = vbslq_f32(cosom_less_then_one, sclp, sin_ps(vmulq_f32(sclp, omega)));
    sclq = vbslq_f32(cosom_less_then_one, sclq, sin_ps(vmulq_f32(sclq, omega)));
    float32x4_t sinom = vbslq_f32(cosom_less_then_one, vdupq_n_f32(1), sin_ps(omega));

    // qt = (sclp * p + sclq * q) / sinom;
    sclp = vdivq_f32(sclp, sinom);
    sclq = vdivq_f32(sclq, sinom);
    qt[0] = vfmaq_laneq_f32(vmulq_laneq_f32(p[0], sclp, 0), q[0], sclq, 0);
    qt[1] = vfmaq_laneq_f32(vmulq_laneq_f32(p[1], sclp, 1), q[1], sclq, 1);
    qt[2] = vfmaq_laneq_f32(vmulq_laneq_f32(p[2], sclp, 2), q[2], sclq, 2);
    qt[3] = vfmaq_laneq_f32(vmulq_laneq_f32(p[3], sclp, 3), q[3], sclq, 3);
#else
    QuaternionSlerp(p[0],q[0], t, qt[0]);
    QuaternionSlerp(p[1],q[1], t, qt[1]);
    QuaternionSlerp(p[2],q[2], t, qt[2]);
    QuaternionSlerp(p[3],q[3], t, qt[3]);
#endif
}

/*
====================
QuaternionMatrix

====================
*/

#if defined(XASH_SIMD) && U_VECTOR_NEON
const matrix3x4	matrix3x4_identity =
{
        { 1, 0, 0, 0 },	// PITCH	[forward], org[0]
        { 0, 1, 0, 0 },	// YAW	[right]  , org[1]
        { 0, 0, 1, 0 },	// ROLL	[up]     , org[2]
};
#endif
void XASH_VECTORCALL QuaternionMatrix(const vec4_t quaternion, matrix3x4_ref matrix)
{
#if defined(XASH_SIMD) && U_VECTOR_NEON
    const uint32x4_t sign1 = vsetq_lane_u32(0x80000000, vdupq_n_u32(0x00000000), 0); // { 0x80000000, 0x00000000, 0x00000000, 0x00000000 };
    const uint32x4_t sign2 = vsetq_lane_u32(0x80000000, vdupq_n_u32(0x00000000), 1); // { 0x00000000, 0x80000000, 0x00000000, 0x00000000 };
	const uint32x4_t sign3 = vsetq_lane_u32(0x00000000, vdupq_n_u32(0x80000000), 2); // { 0x80000000, 0x80000000, 0x00000000, 0x80000000 };

	float32x4_t q1032 = vrev64q_f32(quaternion);
	float32x4_t q1032_signed = vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(q1032), sign1));
	float32x4_t q2301 = vextq_f32(quaternion, quaternion, 2);
	float32x4_t q2301_signed = vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(q2301), sign3));
    float32x4_t q3210 = vrev64q_f32(q2301);
    float32x4_t q3210_signed = vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(q3210), sign2));

    auto v0 = vmulq_laneq_f32(q2301_signed, quaternion, 2);
    v0 = vfmaq_laneq_f32(v0, q1032_signed, quaternion, 1);
    matrix[0] = vfmaq_n_f32(matrix3x4_identity[0], v0, 2.0f);

    auto v1 = vmulq_laneq_f32(q3210_signed, quaternion, 2);
    v1 = vfmsq_laneq_f32(v1, q1032_signed, quaternion, 0);
    matrix[1] = vfmaq_n_f32(matrix3x4_identity[1], v1, 2.0f);

    auto v2 = vmulq_laneq_f32(q3210_signed, quaternion, 1);
    v2 = vfmaq_laneq_f32(v2, q2301_signed, quaternion, 0);
    matrix[2] = vfmsq_n_f32(matrix3x4_identity[2], v2, 2.0f);
/*
    matrix[0][0] = 1.0 + 2.0 * (  quaternion[1] * -quaternion[1] + -quaternion[2] * quaternion[2] );
    matrix[0][1] = 0.0 + 2.0 * (  quaternion[1] *  quaternion[0] + -quaternion[3] * quaternion[2] );
    matrix[0][2] = 0.0 + 2.0 * (  quaternion[1] *  quaternion[3] +  quaternion[0] * quaternion[2] );
    matrix[0][3] = 0.0 + 2.0 * (  quaternion[1] *  quaternion[2] + -quaternion[1] * quaternion[2] );

    matrix[1][0] = 0.0 + 2.0 * ( -quaternion[0] * -quaternion[1] +  quaternion[3] * quaternion[2] );
    matrix[1][1] = 1.0 + 2.0 * ( -quaternion[0] *  quaternion[0] + -quaternion[2] * quaternion[2] );
    matrix[1][2] = 0.0 + 2.0 * ( -quaternion[0] *  quaternion[3] +  quaternion[1] * quaternion[2] );
    matrix[1][3] = 0.0 + 2.0 * ( -quaternion[0] *  quaternion[2] +  quaternion[0] * quaternion[2] );
	
    matrix[2][0] = 0.0 + 2.0 * ( -quaternion[0] * -quaternion[2] + -quaternion[3] * quaternion[1] );
    matrix[2][1] = 0.0 + 2.0 * ( -quaternion[0] * -quaternion[3] +  quaternion[2] * quaternion[1] );
    matrix[2][2] = 1.0 + 2.0 * ( -quaternion[0] *  quaternion[0] + -quaternion[1] * quaternion[1] );
    matrix[2][3] = 0.0 + 2.0 * ( -quaternion[0] * -quaternion[1] + -quaternion[0] * quaternion[1] );
*/
#else
	matrix[0][0] = 1.0 - 2.0 * quaternion[1] * quaternion[1] - 2.0 * quaternion[2] * quaternion[2];
	matrix[1][0] = 2.0 * quaternion[0] * quaternion[1] + 2.0 * quaternion[3] * quaternion[2];
	matrix[2][0] = 2.0 * quaternion[0] * quaternion[2] - 2.0 * quaternion[3] * quaternion[1];

	matrix[0][1] = 2.0 * quaternion[0] * quaternion[1] - 2.0 * quaternion[3] * quaternion[2];
	matrix[1][1] = 1.0 - 2.0 * quaternion[0] * quaternion[0] - 2.0 * quaternion[2] * quaternion[2];
	matrix[2][1] = 2.0 * quaternion[1] * quaternion[2] + 2.0 * quaternion[3] * quaternion[0];

	matrix[0][2] = 2.0 * quaternion[0] * quaternion[2] + 2.0 * quaternion[3] * quaternion[1];
	matrix[1][2] = 2.0 * quaternion[1] * quaternion[2] - 2.0 * quaternion[3] * quaternion[0];
	matrix[2][2] = 1.0 - 2.0 * quaternion[0] * quaternion[0] - 2.0 * quaternion[1] * quaternion[1];
#endif
}

/*
====================
MatrixCopy

====================
*/
void XASH_VECTORCALL MatrixCopy(cmatrix3x4 in, matrix3x4_ref out)
{
    out = in;
}

}
