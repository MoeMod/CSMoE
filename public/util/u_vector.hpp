/*
u_vector.hpp - CSMoE Foundation : Vector
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

#ifndef PROJECT_U_VECTOR_HPP
#define PROJECT_U_VECTOR_HPP

#include <cmath>
#include <utility>
#include <algorithm>
#include <numeric>
#include <algorithm>
#include <type_traits>

typedef struct lua_State lua_State;

#if defined __SSE__ || defined __x86_64__ || defined _M_X64 || defined _M_AMD64 || defined _M_IX86_FP
#include <xmmintrin.h> // sse
#include <emmintrin.h> // sse2
#include <smmintrin.h> // sse4.1
#define U_VECTOR_SSE 1
#define U_VECTOR_SIMD U_VECTOR_SSE
#endif

#if defined(__ARM_NEON) || defined(__ARM_NEON__) || defined(_M_ARM) || defined(_M_ARM64)
#include <arm_neon.h>
#define U_VECTOR_NEON 2
#define U_VECTOR_SIMD U_VECTOR_NEON
#endif

#ifdef _MSC_VER
#define U_VECTOR_FORCEINLINE __forceinline
#else
#define U_VECTOR_FORCEINLINE inline
#endif

namespace moe {

template<class T, std::size_t N, std::size_t Align = alignof(T)>
struct VectorBase;

template<class T, std::size_t N, class Seq, std::size_t Align, class Base>
struct VectorBase_Gen;

template<class T, std::size_t N, std::size_t...I, std::size_t Align, class Base>
struct VectorBase_Gen<T, N, std::index_sequence<I...>, Align, Base> : Base
{
	constexpr VectorBase_Gen() : Base()
	{
	}
	template<class U = std::nullptr_t>
	constexpr VectorBase_Gen(typename std::type_identity<U>::type ) : Base()
	{
        (..., (this->m_data[I] = NAN));
	}
	constexpr VectorBase_Gen(std::initializer_list<T> il) : VectorBase_Gen(il.begin())
	{
	}
	constexpr VectorBase_Gen(const VectorBase_Gen &) = default;
	template<class InputIter>
	constexpr explicit VectorBase_Gen(InputIter arr, typename std::enable_if<std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<InputIter>::iterator_category>::value>::type * = nullptr) : Base()
	{
		std::copy_n(arr, N, this->m_data);
	}

    // TODO : no more unaligned please!
    template<class T2, std::size_t Align2, class Base2>
    constexpr VectorBase_Gen(const VectorBase_Gen<T2, N, std::index_sequence<I...>, Align2, Base2> &other) : VectorBase_Gen{ other[I]... } {}

	T &operator[](std::size_t i)
	{
		return this->m_data[i];
	}
	constexpr const T &operator[](std::size_t i) const
	{
		return this->m_data[i];
	}
	T *data()
	{
		return this->m_data;
	}
	const T *data() const
	{
		return this->m_data;
	}
};

template<class T, size_t N, std::size_t Align> struct alignas(Align) VectorBase_Data
{
     T m_data[N];
};

template<class T, std::size_t N, std::size_t Align>
struct VectorBase_Storage : VectorBase_Gen<T, N, std::make_index_sequence<N>, Align, VectorBase_Data<T, N, Align>>
{
    using Base = VectorBase_Gen<T, N, std::make_index_sequence<N>, Align, VectorBase_Data<T, N, Align>>;
    using Base::Base;
};

template<class T, std::size_t Align> struct alignas(Align) VectorBase2_Data
{
    constexpr VectorBase2_Data() : m_data{} {}
    union {
        T m_data[2];
        struct {
            T x, y;
        };
    };
};

template<class T, std::size_t Align>
struct VectorBase_Storage<T, 2, Align> : VectorBase_Gen<T, 2, std::index_sequence<0, 1>, Align, VectorBase2_Data<T, Align>>
{
	using Base = VectorBase_Gen<T, 2, std::index_sequence<0, 1>, Align, VectorBase2_Data<T, Align>>;
    using Base::Base;

	constexpr VectorBase_Storage(T x1, T y1) : Base{x1, y1}
	{
	}
};

template<class T, std::size_t Align> struct alignas(Align) VectorBase3_Data
{
    constexpr VectorBase3_Data() : m_data{} {}
    union {
        T m_data[3];
        struct {
            T x, y, z;
        };
    };
};

template<class T, std::size_t Align>
struct VectorBase_Storage<T, 3, Align> : VectorBase_Gen<T, 3, std::index_sequence<0, 1, 2>, Align, VectorBase3_Data<T, Align>>
{
    using Base = VectorBase_Gen<T, 3, std::index_sequence<0, 1, 2>, Align, VectorBase3_Data<T, Align>>;
    using Base::Base;

	constexpr VectorBase_Storage(T x1, T y1, T z1) : Base{ x1, y1, z1 }
	{
	}

	// make it template cast operator function for lower priority
#ifdef U_VECTOR_LEGACY_COMPAT
	template<class R, class = typename std::enable_if<std::is_same<T *, R *>::value || std::is_same<void *, R *>::value>::type>
	operator R *()
	{
		return this->data();
	}
	template<class R, class = typename std::enable_if<std::is_same<T *, R *>::value || std::is_same<void *, R *>::value>::type>
	operator const R *() const
	{
		return this->data();
	}
#endif
};

#ifdef U_VECTOR_SIMD

#if U_VECTOR_NEON
    using xmm_t = float32x4_t;
#elif U_VECTOR_SSE
    using xmm_t = __m128;
#endif

struct VectorBase4f_SIMD_Data
{
    constexpr VectorBase4f_SIMD_Data() : m_data{} {}
    constexpr VectorBase4f_SIMD_Data(float x1, float y1, float z1, float w1) : x(x1), y(y1), z(z1), w(w1) {}
    union
    {
        float m_data[4];
        struct
        {
            float x, y, z, w;
        };
        xmm_t m_xmm;
    };
};

struct VectorBase3f_SIMD_Data
{
    constexpr VectorBase3f_SIMD_Data() : m_data{}, m_data_unused{} {}
    constexpr VectorBase3f_SIMD_Data(float x1, float y1, float z1) : x(x1), y(y1), z(z1), w_ununsed() {}
    union
    {
        struct
        {
            float m_data[3];
            float m_data_unused;
        };
        struct
        {
            float x, y, z;
            float w_ununsed;
        };
        xmm_t m_xmm;
    };
};
static_assert(sizeof(VectorBase3f_SIMD_Data) == 16 && alignof(VectorBase3f_SIMD_Data) == 16);

template<> struct VectorBase_Storage<float, 3, 16> : VectorBase_Gen<float, 3, std::index_sequence<0, 1, 2>, 16, VectorBase3f_SIMD_Data>
{
    using Base = VectorBase_Gen<float, 3, std::index_sequence<0, 1, 2>, 16, VectorBase3f_SIMD_Data>;
    using Base::Base;

    constexpr VectorBase_Storage() = default;

    constexpr VectorBase_Storage(float x1, float y1, float z1)
    {
        if (std::is_constant_evaluated())
        {
            this->x = x1;
            this->y = y1;
            this->z = z1;
            this->w_ununsed = 0;
        }
        else
        {
#if U_VECTOR_NEON
            this->x = x1;
            this->y = y1;
            this->z = z1;
            this->w_ununsed = 0;
#elif U_VECTOR_SSE
            this->m_xmm = _mm_set_ps(0, z1, y1, x1);
#endif
        }
    }

    VectorBase_Storage(std::nullptr_t)
    {
#if U_VECTOR_NEON
        this->m_xmm = vdupq_n_f32(NAN);
#elif U_VECTOR_SSE
        this->m_xmm = _mm_set1_ps(NAN);
#endif
    }

    explicit VectorBase_Storage(const float *p) : VectorBase_Storage(p[0], p[1], p[2]) {}

    VectorBase_Storage(xmm_t xmm)
    {
        this->m_xmm = xmm;
    }

    operator const xmm_t &() const
    {
        return this->m_xmm;
    }

    // TODO : no more unaligned please!
    constexpr VectorBase_Storage(const VectorBase_Gen<float, 3, std::index_sequence<0, 1, 2>, 4, VectorBase3_Data<float, 4>>& other)
    {
        x = other.x;
        y = other.y;
        z = other.z;
    }
};

template<> struct VectorBase_Storage<float, 4, 16> : VectorBase_Gen<float, 4, std::index_sequence<0, 1, 2, 3>, 16, VectorBase4f_SIMD_Data>
{
    using Base = VectorBase_Gen<float, 4, std::index_sequence<0, 1, 2, 3>, 16, VectorBase4f_SIMD_Data>;
    using Base::Base;

    constexpr VectorBase_Storage() = default;

    constexpr VectorBase_Storage(float x1, float y1, float z1, float w1)
    {
        if (std::is_constant_evaluated())
        {
            this->x = x1;
            this->y = y1;
            this->z = z1;
            this->w = w1;
        }
        else
        {
#if U_VECTOR_NEON
            this->x = x1;
            this->y = y1;
            this->z = z1;
            this->w = w1;
#elif U_VECTOR_SSE
            this->m_xmm = _mm_set_ps(w1, z1, y1, x1);
#endif
        }
    }

    VectorBase_Storage(std::nullptr_t)
    {
#if U_VECTOR_NEON
        this->m_xmm = vdupq_n_f32(NAN);
#elif U_VECTOR_SSE
        this->m_xmm = _mm_set1_ps(NAN);
#endif
    }

    explicit VectorBase_Storage(const float *p)
    {
#if U_VECTOR_NEON
        this->m_xmm = vld1q_f32(p);
#elif U_VECTOR_SSE
        this->m_xmm = _mm_loadu_ps(p);
#endif
    }

    VectorBase_Storage(xmm_t xmm)
    {
        this->m_xmm = xmm;
    }

    operator const xmm_t &() const
    {
        return this->m_xmm;
    }
};
#endif
template<class VecType, std::size_t...I>
constexpr VecType add_impl(VecType v1, VecType v2, std::index_sequence<I...>)
{
	return {(v1.template get<I>() + v2.template get<I>())...};
}
template<class VecType, std::size_t...I>
constexpr VecType sub_impl(VecType v1, VecType v2, std::index_sequence<I...>)
{
	return {(v1.template get<I>() - v2.template get<I>())...};
}
template<class VecType, std::size_t...I>
constexpr VecType neg_impl(VecType vec, std::index_sequence<I...>)
{
	return {(-vec.template get<I>())...};
}
template<class VecType, std::size_t...I>
constexpr VecType valmul_impl(VecType vec, typename VecType::value_type val, std::index_sequence<I...>)
{
	return {(vec.template get<I>() * val)...};
}
template<class VecType, std::size_t...I>
constexpr VecType valdiv_impl(VecType vec, typename VecType::value_type val, std::index_sequence<I...>)
{
	return {(vec.template get<I>() / val)...};
}

namespace moe_math_util {

using std::hypot;
using std::sqrt;
using std::abs;
using std::acos;
using std::fma;

template<class T> inline T hypot(T x) { return abs(x); }
template<class...Args> inline auto hypot(Args...args) -> typename std::common_type<Args...>::type {
	return sqrt((... + (args * args)));
}

template<class T> inline auto rsqrt(T x) -> decltype(1 / sqrt(x)) {
	return 1 / sqrt(x);
}

}

template<class VecTypeA, class VecTypeB, std::size_t...I>
constexpr bool equal_impl(VecTypeA v1, VecTypeB v2, std::index_sequence<I...>)
{
	return (... && (v1.template get<I>() == v2.template get<I>()));
}
template<class VecType, std::size_t...I>
constexpr typename VecType::value_type DotProduct_impl(VecType v1, VecType v2, std::index_sequence<I...>)
{
	return (... + (v1.template get<I>() * v2.template get<I>()));
}
template<class VecType, std::size_t...I>
constexpr typename VecType::value_type LengthSquared_impl(VecType vec, std::index_sequence<I...>)
{
	return (... + (vec.template get<I>() * vec.template get<I>()));
}
template<class VecType, std::size_t...I>
inline typename VecType::value_type Length_impl(VecType vec, std::index_sequence<I...>)
{
	return moe_math_util::hypot(vec.template get<I>()...);
}
template<class VecType, std::size_t...I>
inline typename VecType::value_type LengthReverse_impl(VecType vec, std::index_sequence<I...>)
{
	return moe_math_util::rsqrt((... + (vec.template get<I>() * vec.template get<I>())));
}
template<class VecType, std::size_t...I>
inline VecType fma_impl(VecType x, typename VecType::value_type y, VecType z, std::index_sequence<I...>)
{
	return { moe_math_util::fma(x.template get<I>(), y, z.template get<I>())... };
}
template<class VecType, std::size_t...I>
inline VecType fma_impl(typename VecType::value_type x, VecType y, VecType z, std::index_sequence<I...>)
{
	return { moe_math_util::fma(x, y.template get<I>(), z.template get<I>())... };
}
template<class VecType, std::size_t...I>
inline bool IsNaN_impl(VecType x, std::index_sequence<I...>)
{
	return (... || std::isnan(x.template get<I>()));
}

template<class T, std::size_t N, std::size_t Align>
struct VectorBase : VectorBase_Storage<T, N, Align>
{
	using Base = VectorBase_Storage<T, N, Align>;
	using Base::Base;

	using value_type = T;

	template<std::size_t I> T &get() & { return (*this)[I]; }
	template<std::size_t I> T &&get() && { return std::move((*this)[I]); }
	template<std::size_t I> constexpr const T &get() const & { return (*this)[I]; }
	template<std::size_t I> constexpr const T &&get() const && { return std::move((*this)[I]); }
	template<std::size_t I> friend T &get(VectorBase &v) { return v.get<I>(); }
	template<std::size_t I> friend T &&get(VectorBase &&v) { return std::move(v).template get<I>(); }
	template<std::size_t I> friend const T &get(const VectorBase &v) { return v.get<I>(); }
	template<std::size_t I> friend const T &&get(const VectorBase &&v) { return std::move(v).template get<I>(); }

	constexpr bool operator==(VectorBase v) const
	{
		return equal_impl(*this, v, std::make_index_sequence<N>());
	}
	constexpr VectorBase operator+(VectorBase v) const
	{
		return add_impl(*this, v, std::make_index_sequence<N>());
	}
	constexpr VectorBase operator-(VectorBase v) const
	{
		return sub_impl(*this, v, std::make_index_sequence<N>());
	}
	constexpr VectorBase operator+() const
	{
		return *this;
	}
	constexpr VectorBase operator-() const
	{
		return neg_impl(*this, std::make_index_sequence<N>());
	}
	constexpr VectorBase operator*(T fl) const
	{
		return valmul_impl(*this, fl, std::make_index_sequence<N>());
	}
	constexpr VectorBase operator/(T fl) const
	{
		return valdiv_impl(*this, fl, std::make_index_sequence<N>());
	}
	friend constexpr VectorBase operator*(T fl, VectorBase vec)
	{
		return vec * fl;
	}
	VectorBase &operator+=(VectorBase v)
	{
		return *this = *this + v;
	}
	VectorBase &operator-=(VectorBase v)
	{
		return *this = *this - v;
	}
	VectorBase &operator*=(T fl)
	{
		return *this = *this * fl;
	}
	VectorBase &operator/=(T fl)
	{
		return *this = *this / fl;
	}

	friend constexpr T DotProduct(VectorBase v1, VectorBase v2)
	{
		return DotProduct_impl(v1, v2, std::make_index_sequence<N>());
	}

	// (a*b)+c
	friend VectorBase fma(VectorBase x, T y, VectorBase z)
	{
		return fma_impl(x, y, z, std::make_index_sequence<N>());
	}
	friend VectorBase fma(T x, VectorBase y, VectorBase z)
	{
		return fma_impl(x, y, z, std::make_index_sequence<N>());
	}

    // t*(b-a) + a;
	friend VectorBase lerp(VectorBase a, VectorBase b, T t)
	{
		return fma(t, b - a, a);
	}

    // 0.5*(b-a) + a;
	friend VectorBase midpoint(VectorBase a, VectorBase b)
	{
		return (b - a) / 2 + a;
	}

    friend VectorBase maxs(VectorBase a, VectorBase b)
    {
        using std::max;
        std::transform(a.data(), a.data() + N, b.data(), a.data(), [](auto a, auto b){ return max(a, b); });
        return a;
    }

    friend VectorBase mins(VectorBase a, VectorBase b)
    {
        using std::min;
        std::transform(a.data(), a.data() + N, b.data(), a.data(), [](auto a, auto b){ return min(a, b); });
        return a;
    }

    friend T reduce(VectorBase a)
    {
        return std::reduce(a.data(), a.data() + N);
    }

    friend T max_element(VectorBase in)
    {
        return *std::max_element(in.data(), in.data() + N);
    }

    friend T min_element(VectorBase in)
    {
        return *std::min_element(in.data(), in.data() + N);
    }

    friend VectorBase abs(VectorBase a)
    {
        using std::abs;
        std::transform(a.data(), a.data() + N, a.data(), [](auto x){ return abs(x); });
        return a;
    }

	constexpr T LengthSquared() const
	{
		return LengthSquared_impl(*this, std::make_index_sequence<N>());
	}
	template<class ValType>
	constexpr bool IsLengthLessThan(ValType length) const
	{
		return (LengthSquared() < length * length);
	}
	template<class ValType>
	constexpr bool IsLengthGreaterThan(ValType length) const
	{
		return (LengthSquared() > length * length);
	}

	T Length() const
	{
		return Length_impl(*this, std::make_index_sequence<N>());
	}

	T LengthReverse() const
	{
		return LengthReverse_impl(*this, std::make_index_sequence<N>());
	}

	VectorBase Normalize() const
	{
		return *this * LengthReverse();
	}

	T NormalizeInPlace()
	{
		auto flLen = Length();
		if (!flLen)
			return {};
		*this /= flLen;
		return flLen;
	}

	constexpr bool IsZero(T tolerance = std::numeric_limits<T>::epsilon()) const
	{
		return LengthSquared() > tolerance;
	}

	constexpr VectorBase<T, 2> Make2D() const
	{
		return {(*this)[0], (*this)[1]};
	}

	T Length2D()
	{
		return Make2D().Length();
	}

	friend constexpr T DotProduct2D(VectorBase v1, VectorBase v2)
	{
		return DotProduct(v1.Make2D(), v2.Make2D());
	}

	bool IsNaN() const
	{
		return IsNaN_impl(*this, std::make_index_sequence<N>());
	}
	explicit operator bool() const
	{
		return !IsNaN();
	}
	void LuaPush(lua_State* L) const;
	void LuaGet(lua_State* L, int idx);
};

#if U_VECTOR_SIMD
template<class ThisClass, int XmmMask, class = void>
struct VectorBaseSIMD;
template<size_t N, int XmmMask>
struct VectorBaseSIMD<VectorBase<float, N, 16>, XmmMask, typename std::enable_if<(N <= 4)>::type> : VectorBase_Storage<float, N, 16>
{
    using ThisClass = VectorBase<float, N, 16>;
    using BaseClass = VectorBase_Storage<float, N, 16>;
    using BaseClass::BaseClass;

    using T = float;
    using value_type = float;
    static constexpr auto Align = 16;

    template<std::size_t I> T &get() & { return (*this)[I]; }
    template<std::size_t I> T &&get() && { return std::move((*this)[I]); }
    template<std::size_t I> constexpr const T &get() const & { return (*this)[I]; }
    template<std::size_t I> constexpr const T &&get() const && { return std::move((*this)[I]); }
    template<std::size_t I> friend T &get(ThisClass &v) { return v.template get<I>(); }
    template<std::size_t I> friend T &&get(ThisClass &&v) { return std::move(v).template get<I>(); }
    template<std::size_t I> friend const T &get(const ThisClass &v) { return v.template get<I>(); }
    template<std::size_t I> friend const T &&get(const ThisClass &&v) { return std::move(v).template get<I>(); }

    U_VECTOR_FORCEINLINE bool operator==(ThisClass v) const
    {
#if U_VECTOR_NEON
        return !vaddvq_u32(vceqq_f32(*this, v));
#elif U_VECTOR_SSE
        // from bullet3
        return 0xF == _mm_movemask_ps(_mm_cmpeq_ps(*this, v.m_xmm));
#endif
    }
#if !__cpp_impl_three_way_comparison
    U_VECTOR_FORCEINLINE bool operator!=(ThisClass v) const
    {
        return !operator==(v);
    }
#endif
    U_VECTOR_FORCEINLINE ThisClass operator+(ThisClass v) const
    {
#if U_VECTOR_NEON
        return vaddq_f32(*this, v);
#elif U_VECTOR_SSE
        return _mm_add_ps(*this, v.m_xmm);
#endif
    }
    U_VECTOR_FORCEINLINE ThisClass operator-(ThisClass v) const
    {
#if U_VECTOR_NEON
        return vsubq_f32(*this, v);
#elif U_VECTOR_SSE
        return _mm_sub_ps(*this, v.m_xmm);
#endif
    }
    U_VECTOR_FORCEINLINE ThisClass operator+() const
    {
        return *this;
    }
    U_VECTOR_FORCEINLINE ThisClass operator-() const
    {
#if U_VECTOR_NEON
        return vnegq_f32(*this);
#elif U_VECTOR_SSE
        return ThisClass(_mm_setzero_ps()) - (const ThisClass &)*this;
#endif
    }
    U_VECTOR_FORCEINLINE ThisClass operator*(float fl) const
    {
#ifdef U_VECTOR_NEON
        return vmulq_n_f32(*this, fl);
#elif U_VECTOR_SSE
        return _mm_mul_ps(*this, _mm_load_ps1(&fl));
#endif
    }
    U_VECTOR_FORCEINLINE ThisClass operator/(T fl) const
    {
#ifdef U_VECTOR_NEON
        return *this * (1 / fl);
#elif U_VECTOR_SSE
        return _mm_div_ps(*this, _mm_load_ps1(&fl));
#endif
    }
    U_VECTOR_FORCEINLINE friend ThisClass operator*(T fl, ThisClass vec)
    {
        return vec * fl;
    }
    U_VECTOR_FORCEINLINE ThisClass &operator+=(ThisClass v)
    {
        return (ThisClass &)*this = *this + v;
    }
    U_VECTOR_FORCEINLINE ThisClass &operator-=(ThisClass v)
    {
        return (ThisClass &)*this = *this - v;
    }
    U_VECTOR_FORCEINLINE ThisClass &operator*=(T fl)
    {
        return (ThisClass &)*this = *this * fl;
    }
    U_VECTOR_FORCEINLINE ThisClass &operator/=(T fl)
    {
        return (ThisClass &)*this = *this / fl;
    }

    U_VECTOR_FORCEINLINE friend T DotProduct(ThisClass v1, ThisClass v2)
    {
#if U_VECTOR_NEON
        return vaddvq_f32(vmulq_f32(v1, v2));
#elif U_VECTOR_SSE
        return _mm_cvtss_f32(_mm_dp_ps(v1.m_xmm, v2.m_xmm, XmmMask));
#endif
    }

    U_VECTOR_FORCEINLINE friend ThisClass fma(T x, ThisClass y, ThisClass z)
    {
#if U_VECTOR_NEON
        return vfmaq_n_f32(z, y, x);
#elif U_VECTOR_FMA
        return _mm_fmadd_ps(_mm_load_ps1(&x), y, z);
#elif U_VECTOR_SSE
        return _mm_add_ps(_mm_mul_ps(_mm_load_ps1(&x), y), z);
#endif
    }

    // (a*b)+c
    U_VECTOR_FORCEINLINE friend ThisClass fma(ThisClass x, T y, ThisClass z)
    {
#if U_VECTOR_NEON
        return vfmaq_n_f32(z, x, y);
#elif U_VECTOR_FMA
        return _mm_fmadd_ps(x, _mm_load_ps1(&y), z);
#elif U_VECTOR_SSE
        return _mm_add_ps(_mm_mul_ps(x, _mm_load_ps1(&y)), z);
#endif
    }

    // t*(b-a) + a;
    U_VECTOR_FORCEINLINE friend ThisClass lerp(ThisClass a, ThisClass b, T t)
    {
        return fma(t, b - a, a);
    }

    // 0.5*(b-a) + a;
    U_VECTOR_FORCEINLINE friend ThisClass midpoint(ThisClass a, ThisClass b)
    {
        return lerp(a, b, 0.5f);
    }

    U_VECTOR_FORCEINLINE friend ThisClass maxs(ThisClass a, ThisClass b)
    {
#if U_VECTOR_NEON
        return vmaxq_f32(a, b);
#elif U_VECTOR_SSE
        return _mm_max_ps(a, b);
#endif
    }

    U_VECTOR_FORCEINLINE friend ThisClass mins(ThisClass a, ThisClass b)
    {
#if U_VECTOR_NEON
        return vminq_f32(a, b);
#elif U_VECTOR_SSE
        return _mm_min_ps(a, b);
#endif
    }

    U_VECTOR_FORCEINLINE friend T reduce(ThisClass a)
    {
#if U_VECTOR_NEON
        return vaddvq_f32(a);
#elif U_VECTOR_SSE
        // Disable SSE3 _mm_hadd_ps that is extremely slow on all existing Intel's architectures
        // (from Nehalem to Haswell)
        a = _mm_add_ps(a, _mm_movehl_ps(a, a));
        a = _mm_add_ss(a, _mm_shuffle_ps(a, a, 1));
        return _mm_cvtss_f32(a);
#endif
    }

    U_VECTOR_FORCEINLINE friend T max_element(ThisClass a)
    {
        // warning: zero a.w can affect the result
#if U_VECTOR_NEON
        return vmaxvq_f32(a);
#elif U_VECTOR_SSE
        __m128 shuf = _mm_movehdup_ps(a);        // broadcast elements 3,1 to 2,0
        __m128 sums = _mm_max_ps(a, shuf);
        shuf = _mm_movehl_ps(shuf, sums); // high half -> low half
        sums = _mm_max_ss(sums, shuf);
        return _mm_cvtss_f32(sums);
#endif
    }

    U_VECTOR_FORCEINLINE friend T min_element(ThisClass a)
    {
        // warning: zero a.w can affect the result
#if U_VECTOR_NEON
        return vminvq_f32(a);
#elif U_VECTOR_SSE
        __m128 shuf = _mm_movehdup_ps(a);        // broadcast elements 3,1 to 2,0
        __m128 sums = _mm_min_ps(a, shuf);
        shuf = _mm_movehl_ps(shuf, sums); // high half -> low half
        sums = _mm_min_ss(sums, shuf);
        return _mm_cvtss_f32(sums);
#endif
    }

    U_VECTOR_FORCEINLINE friend ThisClass abs(ThisClass a)
    {
#if U_VECTOR_NEON
        static const int32x4_t mask = vsetq_lane_s32(0, vdupq_n_s32(0x7fffffff), 3); // {0x7fffffff, 0x7fffffff, 0x7fffffff, 0};
        return vreinterpretq_f32_s32(vandq_s32(vreinterpretq_s32_f32(a), mask));
#elif U_VECTOR_SSE
        // 0x7fffffff = ~(-0.0f)
        __m128 mask = _mm_castsi128_ps(_mm_set_epi32(0x7fffffff, 0x7fffffff, 0x7fffffff, 0));
        return _mm_and_ps(a, mask);
#endif
    }

    U_VECTOR_FORCEINLINE T LengthSquared() const
    {
#ifdef U_VECTOR_NEON
        return vaddvq_f32(vmulq_f32(*this, *this));
#elif U_VECTOR_SSE
        return _mm_cvtss_f32(_mm_dp_ps(*this, *this, XmmMask));
#endif
    }
    template<class ValType>
    constexpr bool IsLengthLessThan(ValType length) const
    {
        return (LengthSquared() < length * length);
    }
    template<class ValType>
    constexpr bool IsLengthGreaterThan(ValType length) const
    {
        return (LengthSquared() > length * length);
    }

    U_VECTOR_FORCEINLINE T Length() const
    {
#if U_VECTOR_NEON
        return sqrt(vaddvq_f32(vmulq_f32(*this, *this))) ;
#elif U_VECTOR_SSE
        return _mm_cvtss_f32(_mm_sqrt_ss(_mm_dp_ps(*this, *this, XmmMask)));
#endif
    }

    U_VECTOR_FORCEINLINE T LengthReverse() const
    {
#if U_VECTOR_NEON
        return vrsqrtes_f32(vaddvq_f32(vmulq_f32(*this, *this)));
#elif U_VECTOR_SSE
        return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_dp_ps(*this, *this, XmmMask)));
#endif
    }

    U_VECTOR_FORCEINLINE ThisClass Normalize() const
    {
#if U_VECTOR_NEON
        return vmulq_n_f32(*this, vrsqrtes_f32(vaddvq_f32(vmulq_f32(*this, *this))));
#elif U_VECTOR_SSE
        return _mm_mul_ps(*this, _mm_rsqrt_ps(_mm_dp_ps(*this, *this, XmmMask)));
#endif
    }

    U_VECTOR_FORCEINLINE T NormalizeInPlace()
    {
        auto flLen = Length();
        if (!flLen)
            return {};
        *this /= flLen;
        return flLen;
    }

    U_VECTOR_FORCEINLINE bool IsZero(T tolerance = std::numeric_limits<T>::epsilon()) const
    {
        return LengthSquared() <= tolerance;
    }

    constexpr VectorBase<T, 2> Make2D() const
    {
        return {(*this)[0], (*this)[1]};
    }

    U_VECTOR_FORCEINLINE T Length2D()
    {
        return Make2D().Length();
    }

    U_VECTOR_FORCEINLINE friend T DotProduct2D(ThisClass v1, ThisClass v2)
    {
        return DotProduct(v1.Make2D(), v2.Make2D());
    }

    U_VECTOR_FORCEINLINE bool IsNaN() const
    {
        return IsNaN_impl(*this, std::make_index_sequence<N>());
    }
    U_VECTOR_FORCEINLINE explicit operator bool() const
    {
        return !IsNaN();
    }
};

template<>
struct VectorBase<float, 3, 16> : VectorBaseSIMD<VectorBase<float, 3, 16>, 0x7F>
{
    using VectorBaseSIMD<VectorBase<float, 3, 16>, 0x7F>::VectorBaseSIMD;
    void LuaPush(lua_State* L) const;
    void LuaGet(lua_State* L, int idx);
};

template<>
struct VectorBase<float, 4, 16> : VectorBaseSIMD<VectorBase<float, 4, 16>, 0xFF>
{
    using VectorBaseSIMD<VectorBase<float, 4, 16>, 0xFF>::VectorBaseSIMD;
};
#endif

template<class T, std::size_t N, std::size_t Align1, std::size_t Align2> constexpr bool operator==(VectorBase<T, N, Align1> a, VectorBase<T, N, Align2> b)
{
    return equal_impl(a, b, std::make_index_sequence<N>());
}

template<class T, std::size_t N, std::size_t Align> constexpr decltype(auto) VectorAdd(VectorBase<T, N, Align> a, VectorBase<T, N, Align> b, VectorBase<T, N, Align> &out)
{
	return out = a + b;
}

template<class T, std::size_t N, std::size_t Align> constexpr decltype(auto) VectorSubtract(VectorBase<T, N, Align> a, VectorBase<T, N, Align> b, VectorBase<T, N, Align> &out)
{
	return out = a - b;
}

template<class T, std::size_t N, std::size_t Align> constexpr decltype(auto) VectorScale(VectorBase<T, N, Align> a, typename std::type_identity<T>::type b, VectorBase<T, N, Align> &out)
{
	return out = a * b;
}

template<class T, std::size_t N, std::size_t Align> constexpr decltype(auto) VectorClear(VectorBase<T, N, Align> &in)
{
	return in = {};
}

template<class T, std::size_t N, std::size_t Align> constexpr decltype(auto) VectorInverse(VectorBase<T, N, Align> &in)
{
	return in = -in;
}

template<class T, std::size_t N, std::size_t Align> constexpr void VectorNegate(VectorBase<T, N, Align> in, VectorBase<T, N, Align> &out)
{
	out = -in;
}

template<class T, std::size_t N, std::size_t Align> constexpr T VectorLength(VectorBase<T, N, Align> in)
{
	return in.Length();
}

template<class T, std::size_t N, std::size_t Align> constexpr T Length(VectorBase<T, N, Align> in)
{
	return in.Length();
}

template<class T, std::size_t N, std::size_t Align> constexpr T Distance(VectorBase<T, N, Align> a,  VectorBase<T, N, Align> b)
{
	return (a - b).Length();
}

template<class T, std::size_t Align> constexpr VectorBase<T, 3, Align> CrossProduct(VectorBase<T, 3, Align> a, VectorBase<T, 3, Align> b)
{
	return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

#ifdef U_VECTOR_SIMD
U_VECTOR_FORCEINLINE VectorBase<float, 3, 16> CrossProduct(VectorBase<float, 3, 16> a, VectorBase<float, 3, 16> b)
{
#if U_VECTOR_NEON
    float32x4_t yzxy_a = vextq_f32(vextq_f32(a, a, 3), a, 2); // [aj, ak, ai, aj]
    float32x4_t yzxy_b = vextq_f32(vextq_f32(b, b, 3), b, 2); // [bj, bk, bi, bj]
    float32x4_t zxyy_a = vextq_f32(yzxy_a, yzxy_a, 1); // [ak, ai, aj, aj]
    float32x4_t zxyy_b = vextq_f32(yzxy_b, yzxy_b, 1); // [bk, ai, bj, bj]
    return vfmsq_f32(vmulq_f32(yzxy_a, zxyy_b), zxyy_a, yzxy_b); // [ajbk-akbj, akbi-aibk, aibj-ajbi, 0]
#elif U_VECTOR_SSE
    __m128 result = _mm_sub_ps(
            _mm_mul_ps(b.m_xmm, _mm_shuffle_ps(a.m_xmm, a.m_xmm, _MM_SHUFFLE(3, 0, 2, 1))),
            _mm_mul_ps(a.m_xmm, _mm_shuffle_ps(b.m_xmm, b.m_xmm, _MM_SHUFFLE(3, 0, 2, 1)))
    );
    return _mm_shuffle_ps(result, result, _MM_SHUFFLE(3, 0, 2, 1));
#endif
}
#endif

template<class T, std::size_t Align> constexpr void CrossProduct(VectorBase<T, 3, Align> a, VectorBase<T, 3, Align> b, VectorBase<T, 3, Align> &out)
{
	out = CrossProduct(a, b);
}

template<class T, std::size_t N, std::size_t Align> constexpr T VectorNormalize(VectorBase<T, N, Align> &in)
{
	return in.NormalizeInPlace();
}

template<class T, std::size_t N, std::size_t Align> constexpr bool VectorCompare(VectorBase<T, N, Align> a,  VectorBase<T, N, Align> b)
{
	return a == b;
}

template<class T, std::size_t N, std::size_t Align, std::size_t Align2> constexpr VectorBase<T, N, Align> &VectorCopy(VectorBase<T, N, Align2> in,  VectorBase<T, N, Align> &out)
{
	return out = in;
}

template<class T, std::size_t N, std::size_t Align> inline void VectorMA(VectorBase<T, N, Align> a, typename std::type_identity<T>::type scale, VectorBase<T, N, Align> b, VectorBase<T, N, Align> &out)
{
	out = fma(b, scale, a);
}

template<class T, std::size_t N, std::size_t Align> inline void VectorMA(T scale, VectorBase<T, N, Align> a, VectorBase<T, N, Align> b, VectorBase<T, N, Align> &out)
{
	out = fma(scale, a, b);
}

template<class T, std::size_t N, std::size_t Align> inline void VectorLerp(VectorBase<T, N, Align> a, T t, VectorBase<T, N, Align> b, VectorBase<T, N, Align> &out)
{
	out = lerp(a, b, t);
}

template<class T, std::size_t N, std::size_t Align> constexpr bool VectorIsNull(VectorBase<T, N, Align> in)
{
    //return in.IsZero();
    return in == VectorBase<T, N, Align>{};
}

}

namespace std {
template<class T, std::size_t N, std::size_t Align> class tuple_size<moe::VectorBase<T, N, Align>> : public std::integral_constant<std::size_t, N> {};
template<class T, std::size_t N, std::size_t I, std::size_t Align> class tuple_element<I, moe::VectorBase<T, N, Align>> { public: using type = T; };
}

#endif //PROJECT_U_VECTOR_HPP
