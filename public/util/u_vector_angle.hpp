#pragma once

#include "angledef.h"
#include "u_vector.hpp"

#if defined(__ARM_NEON) || defined(__ARM_NEON__) || defined(_M_ARM) || defined(_M_ARM64)
#include "neon_mathfun.h"
#endif
#if defined __SSE__ || defined __x86_64__ || defined _M_X64 || defined _M_AMD64 || defined _M_IX86_FP
#include "sse_mathfun.h"
#endif

namespace moe {

    template<class T, std::size_t N, std::size_t Align> T AngleBetweenVectors(VectorBase<T, N, Align> a, VectorBase<T, N, Align> b)
    {
        return moe_math_util::acos(DotProduct(a.Normalize(), b.Normalize())) * (180 / M_PI);
    }

    template<class T, std::size_t Align> void AngleVectors(VectorBase<T, 3, Align> angles, VectorBase<T, 3, Align> &forward, VectorBase<T, 3, Align>& right, VectorBase<T, 3, Align>& up)
    {
#ifdef XASH_SIMD
        VectorBase<T, 3, 16> a = angles;
        a *= static_cast<float>(M_PI / 180.f);
        xmm_t sina, cosa;
        sincos_ps(a, &sina, &cosa);
        auto sp = s4f_x(sina); //sin(DEG2RAD(angles[PITCH]));
        auto sy = s4f_y(sina); //sin(DEG2RAD(angles[YAW]));
        auto sr = s4f_z(sina); //sin(DEG2RAD(angles[ROLL]));
        auto cp = s4f_x(cosa); //cos(DEG2RAD(angles[PITCH]));
        auto cy = s4f_y(cosa); //cos(DEG2RAD(angles[YAW]));
        auto cr = s4f_z(cosa); //cos(DEG2RAD(angles[ROLL]));
#else
        auto sp = sin(DEG2RAD(angles[PITCH]));
        auto sy = sin(DEG2RAD(angles[YAW]));
        auto sr = sin(DEG2RAD(angles[ROLL]));
        auto cp = cos(DEG2RAD(angles[PITCH]));
        auto cy = cos(DEG2RAD(angles[YAW]));
        auto cr = cos(DEG2RAD(angles[ROLL]));
#endif
        forward = { cp * cy, cp * sy, -sp };
        right = { (-sr * sp * cy + -cr * -sy), (-sr * sp * sy + -cr * cy), (-sr * cp) };
        up = { (cr * sp * cy + -sr * -sy), (cr * sp * sy + -sr * cy), (cr * cp) };
    }

    template<class T, std::size_t Align> void AngleVectorsTranspose(VectorBase<T, 3, Align> angles, VectorBase<T, 3, Align> &forward, VectorBase<T, 3, Align>& right, VectorBase<T, 3, Align>& up)
    {
#ifdef XASH_SIMD
        VectorBase<T, 3, 16> a = angles;
        a *= static_cast<float>(M_PI / 180.f);
        xmm_t sina, cosa;
        sincos_ps(a, &sina, &cosa);
        auto sp = s4f_x(sina); //sin(DEG2RAD(angles[PITCH]));
        auto sy = s4f_y(sina); //sin(DEG2RAD(angles[YAW]));
        auto sr = s4f_z(sina); //sin(DEG2RAD(angles[ROLL]));
        auto cp = s4f_x(cosa); //cos(DEG2RAD(angles[PITCH]));
        auto cy = s4f_y(cosa); //cos(DEG2RAD(angles[YAW]));
        auto cr = s4f_z(cosa); //cos(DEG2RAD(angles[ROLL]));
#else
        auto sp = sin(DEG2RAD(angles[PITCH]));
        auto sy = sin(DEG2RAD(angles[YAW]));
        auto sr = sin(DEG2RAD(angles[ROLL]));
        auto cp = cos(DEG2RAD(angles[PITCH]));
        auto cy = cos(DEG2RAD(angles[YAW]));
        auto cr = cos(DEG2RAD(angles[ROLL]));
#endif
        forward = { cp * cy, sr * sp * cy + cr * -sy, cr * sp * cy + -sr * -sy };
        right = { cp * sy, ( sr * sp * sy + cr * cy ), ( cr * sp * sy + -sr * cy ) };
        up = { -sp, sr * cp, cr * cp };
    }

    template<class T, std::size_t Align> void VectorAngles(VectorBase<T, 3, Align> forward, VectorBase<T, 3, Align>& angles)
    {
        if (forward.x == 0 && forward.y == 0)
        {
            // fast case
            angles[YAW] = 0;
            if (forward.z > 0)
                angles[PITCH] = 90;
            else angles[PITCH] = 270;
        }
        else
        {
            angles[YAW] = RAD2DEG(atan2(forward.y, forward.x));
            if (angles[YAW] < 0) angles[YAW] += 360;

            auto tmp = hypot(forward.x, forward.y);
            angles[PITCH] = RAD2DEG(atan2(forward.z, tmp));
            if (angles[PITCH] < 0) angles[PITCH] += 360;
        }

        angles[ROLL] = 0;
    }
}