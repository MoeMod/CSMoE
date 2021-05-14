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
#include <numeric>
#include <array>
#include <type_traits>

#ifdef __SSE__
#include <xmmintrin.h>
#endif

#include "angledef.h"

#ifndef CLIENT_DLL
namespace sv {
#else
namespace cl {
#endif
namespace moe {

template<class T, std::size_t N, class Seq, std::size_t Align>
struct VectorBase_Gen;

template<class T, std::size_t N, std::size_t...I, std::size_t Align>
struct VectorBase_Gen<T, N, std::index_sequence<I...>, Align>
{
	constexpr VectorBase_Gen() : m_data{}
	{
	}
	constexpr explicit VectorBase_Gen(decltype(I, T())...args) : m_data{args...}
	{
	}
	constexpr VectorBase_Gen(const VectorBase_Gen &) = default;
	template<class InputIter>
	explicit VectorBase_Gen(InputIter arr, typename std::enable_if<std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<InputIter>::iterator_category>::value>::type * = nullptr)
	{
		std::copy_n(arr, N, m_data.begin());
	}

	alignas(Align) std::array<T, N> m_data;

	T &operator[](std::size_t i)
	{
		return m_data[i];
	}
	constexpr const T &operator[](std::size_t i) const
	{
		return m_data[i];
	}
	T *data()
	{
		return m_data.data();
	}
	const T *data() const
	{
		return m_data.data();
	}

	template<class OutputIter>
	void CopyToIter(OutputIter arr) const
	{
		std::copy(m_data.begin(), m_data.end(), arr);
	}
	void CopyToArray(T *arr) const
	{
		return CopyToIter(arr);
	}
};

template<class T, std::size_t Align> struct alignas(Align) VectorBase2_Data
{
	T x;
	T y;
};

template<class T, std::size_t Align>
struct VectorBase_Gen<T, 2, std::index_sequence<0, 1>, Align> : VectorBase2_Data<T, Align>
{
	constexpr VectorBase_Gen() : VectorBase2_Data<T, Align>{}
	{
	}
	constexpr VectorBase_Gen(T x1, T y1) : VectorBase2_Data<T, Align>{x1, y1}
	{
	}
	constexpr VectorBase_Gen(const VectorBase_Gen &) = default;
	template<class InputIter>
	explicit VectorBase_Gen(InputIter arr, typename std::enable_if<std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<InputIter>::iterator_category>::value>::type * = nullptr)
	{
		this->x = *arr++;
		this->y = *arr++;
	}

	T &operator[](std::size_t i)
	{
		return (i == 0) ? x : y;
	}
	constexpr const T &operator[](std::size_t i) const
	{
		return (i == 0) ? x : y;
	}
	T *data()
	{
		return &x;
	}
	const T *data() const
	{
		return &x;
	}
	template<class OutputIter>
	void CopyToIter(OutputIter arr) const
	{
		*arr++ = x;
		*arr++ = y;
	}
	void CopyToArray(T *arr) const
	{
		return CopyToIter(arr);
	}
};

template<class T, std::size_t Align> struct alignas(Align) VectorBase3_Data
{
	T x;
	T y;
	T z;
};

template<class T, std::size_t Align>
struct alignas(Align) VectorBase_Gen<T, 3, std::index_sequence<0, 1, 2>, Align> : VectorBase3_Data<T, Align>
{
	constexpr VectorBase_Gen() : VectorBase3_Data<T, Align>{}
	{
	}
	constexpr VectorBase_Gen(T x1, T y1, T z1) : VectorBase3_Data<T, Align>{ x1, y1, z1 }
	{
	}
	constexpr VectorBase_Gen(const VectorBase_Gen &) = default;
	template<class InputIter>
	explicit VectorBase_Gen(InputIter arr, typename std::enable_if<std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<InputIter>::iterator_category>::value>::type* = nullptr)
	{
		this->x = *arr++;
		this->y = *arr++;
		this->z = *arr++;
	}
	
	T &operator[](std::size_t i)
	{
		return (i == 0) ? this->x : ((i == 1) ? this->y : this->z);
	}
	constexpr const T &operator[](std::size_t i) const
	{
		return (i == 0) ? this->x : ((i == 1) ? this->y : this->z);
	}
	T *data()
	{
		return &this->x;
	}
	const T *data() const
	{
		return &this->x;
	}
	// make it template cast operator function for lower priority
	template<class R, class = typename std::enable_if<std::is_same<T *, R *>::value || std::is_same<void *, R *>::value>::type>
	operator R *()
	{
		return data();
	}
	template<class R, class = typename std::enable_if<std::is_same<T *, R *>::value || std::is_same<void *, R *>::value>::type>
	operator const R *() const
	{
		return data();
	}
	template<class OutputIter>
	void CopyToIter(OutputIter arr) const
	{
		*arr++ = this->x;
		*arr++ = this->y;
		*arr++ = this->z;
	}
	void CopyToArray(T *arr) const
	{
		return CopyToIter(arr);
	}
};

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
	return sqrt(... + (args * args));
}

template<class T> inline auto rsqrt(T x) -> decltype(1 / sqrt(x)) {
	return 1 / sqrt(x);
}

}

template<class VecType, std::size_t...I>
constexpr bool equal_impl(VecType v1, VecType v2, std::index_sequence<I...>)
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

template<class T, std::size_t N, std::size_t Align = alignof(float)>
struct VectorBase : VectorBase_Gen<T, N, std::make_index_sequence<N>, Align>
{
	using Base = VectorBase_Gen<T, N, std::make_index_sequence<N>, Align>;
	using Base::Base;

	using value_type = T;

	template<std::size_t I> T &get() & { return (*this)[I]; }
	template<std::size_t I> T &&get() && { return (*this)[I]; }
	template<std::size_t I> constexpr const T &get() const & { return (*this)[I]; }
	template<std::size_t I> constexpr const T &&get() const && { return (*this)[I]; }
	template<std::size_t I> friend T &get(VectorBase &v) { return v.get<I>(); }
	template<std::size_t I> friend T &&get(VectorBase &&v) { return std::move(v).template get<I>(); }
	template<std::size_t I> friend const T &get(const VectorBase &v) { return v.get<I>(); }
	template<std::size_t I> friend const T &&get(const VectorBase &&v) { return std::move(v).template get<I>(); }

	constexpr bool operator==(VectorBase v) const
	{
		return equal_impl(*this, v, std::make_index_sequence<N>());
	}
	constexpr bool operator!=(VectorBase v) const
	{
		return !equal_impl(*this, v, std::make_index_sequence<N>());
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

	constexpr bool IsNull() const
	{
		return LengthSquared() == T{};
	}
	explicit constexpr operator bool()
	{
		return !IsNull();
	}
};

template<class T, std::size_t Align> constexpr VectorBase<T, 3, Align> CrossProduct(VectorBase<T, 3, Align> a, VectorBase<T, 3, Align> b)
{
	return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

template<class T, std::size_t N, std::size_t Align> T AngleBetweenVectors(VectorBase<T, N, Align> a, VectorBase<T, N, Align> b)
{
	return moe_math_util::acos(DotProduct(a.Normalize(), b.Normalize())) * (180 / M_PI);
}

template<class T, std::size_t N, std::size_t Align> constexpr T VectorNormalize(VectorBase<T, N, Align> &in)
{
	return in.NormalizeInPlace();
}

template<class T, std::size_t N, std::size_t Align> constexpr bool VectorCompare(VectorBase<T, N, Align> a,  VectorBase<T, N, Align> b)
{
	return a == b;
}

template<class T, std::size_t N, std::size_t Align> constexpr VectorBase<T, N, Align> &VectorCopy(VectorBase<T, N, Align> in,  VectorBase<T, N, Align> &out)
{
	return out = in;
}

template<class T, std::size_t N, std::size_t Align> inline VectorBase<T, N, Align> &VectorMA(VectorBase<T, N, Align> a, T scale, VectorBase<T, N, Align> b, VectorBase<T, N, Align> &out)
{
	return out = fma(a, scale, b);
}

template<class T, std::size_t N, std::size_t Align> inline VectorBase<T, N, Align> &VectorMA(T scale, VectorBase<T, N, Align> a, VectorBase<T, N, Align> b, VectorBase<T, N, Align> &out)
{
	return out = fma(scale, a, b);
}
	
template<class T, std::size_t Align> void AngleVectors(VectorBase<T, 3, Align> angles, VectorBase<T, 3, Align> &forward, VectorBase<T, 3, Align>& right, VectorBase<T, 3, Align>& up)
{
	auto sp = sin(DEG2RAD(angles[PITCH]));
	auto sy = sin(DEG2RAD(angles[YAW]));
	auto sr = sin(DEG2RAD(angles[ROLL]));
	auto cp = cos(DEG2RAD(angles[PITCH]));
	auto cy = cos(DEG2RAD(angles[YAW]));
	auto cr = cos(DEG2RAD(angles[ROLL]));
	forward = { cp * cy, cp * sy, -sp };
	right = { (-sr * sp * cy + -cr * -sy), (-sr * sp * sy + -cr * cy), (-sr * cp) };
	up = { (cr * sp * cy + -sr * -sy), (cr * sp * sy + -sr * cy), (cr * cp) };
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

		float tmp = hypot(forward.x, forward.y);
		angles[PITCH] = RAD2DEG(atan2(forward.z, tmp));
		if (angles[PITCH] < 0) angles[PITCH] += 360;
	}

	angles[ROLL] = 0;
}

}
}

namespace std {
#ifndef CLIENT_DLL
template<class T, std::size_t N, std::size_t Align> class tuple_size<::sv::moe::VectorBase<T, N, Align>> : public std::integral_constant<std::size_t, N> {};
template<class T, std::size_t N, std::size_t I, std::size_t Align> class tuple_element<I, ::sv::moe::VectorBase<T, N, Align>> { public: using type = T; };
#else
template<class T, std::size_t N, std::size_t Align> class tuple_size<::cl::moe::VectorBase<T, N, Align>> : public std::integral_constant<std::size_t, N> {};
template<class T, std::size_t N, std::size_t I, std::size_t Align> class tuple_element<I, ::cl::moe::VectorBase<T, N, Align>> { public: using type = T; };
#endif
}

#endif //PROJECT_U_VECTOR_HPP
