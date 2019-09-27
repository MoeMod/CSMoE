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

#include <utility>
#include <numeric>
#include <array>
#include <type_traits>

#ifndef CLIENT_DLL
namespace sv {
#else
namespace cl {
#endif
namespace moe {

template<class T, std::size_t N, class Seq>
struct VectorBase_Gen;

template<class T, std::size_t N, std::size_t...I>
struct VectorBase_Gen<T, N, std::index_sequence<I...>>
{
	constexpr VectorBase_Gen() : m_data{}
	{
	}
	constexpr explicit VectorBase_Gen(decltype(I, T())...args) : m_data{args...}
	{
	}
	constexpr VectorBase_Gen(const VectorBase_Gen &) = default;
	template<class InputIter>
	explicit VectorBase_Gen(InputIter arr)
	{
		std::copy_n(arr, N, m_data.begin());
	}

	std::array<T, N> m_data;

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

template<class T>
struct VectorBase_Gen<T, 2, std::index_sequence<0, 1>>
{
	constexpr VectorBase_Gen() : x{}, y{}
	{
	}
	constexpr VectorBase_Gen(T x1, T y1) : x{x1}, y{y1}
	{
	}
	constexpr VectorBase_Gen(const VectorBase_Gen &) = default;
	template<class InputIter>
	explicit VectorBase_Gen(InputIter arr)
	{
		x = *arr++;
		y = *arr++;
	}
	T x;
	T y;

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
	// make it template cast operator function for lower priority
	template<class R, class = typename std::enable_if<std::is_convertible<T *, R *>::value>::type>
	operator R *() &
	{
		return data();
	}
	template<class R, class = typename std::enable_if<std::is_convertible<T *, R *>::value>::type>
	operator const R *() const &
	{
		return data();
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

template<class T>
struct VectorBase_Gen<T, 3, std::index_sequence<0, 1, 2>>
{
	constexpr VectorBase_Gen() : x{}, y{}
	{
	}
	constexpr VectorBase_Gen(T x1, T y1, T z1) : x{x1}, y{y1}, z{z1}
	{
	}
	constexpr VectorBase_Gen(const VectorBase_Gen &) = default;
	template<class InputIter>
	explicit VectorBase_Gen(InputIter arr)
	{
		x = *arr++;
		y = *arr++;
		z = *arr++;
	}
	T x;
	T y;
	T z;

	T &operator[](std::size_t i)
	{
		return (i == 0) ? x : ((i == 1) ? y : z);
	}
	constexpr const T &operator[](std::size_t i) const
	{
		return (i == 0) ? x : ((i == 1) ? y : z);
	}
	T *data()
	{
		return &x;
	}
	const T *data() const
	{
		return &x;
	}
	// make it template cast operator function for lower priority
	template<class R, class = typename std::enable_if<std::is_convertible<T *, R *>::value>::type>
	operator R *()
	{
		return data();
	}
	template<class R, class = typename std::enable_if<std::is_convertible<T *, R *>::value>::type>
	operator const R *() const
	{
		return data();
	}
	template<class OutputIter>
	void CopyToIter(OutputIter arr) const
	{
		*arr++ = x;
		*arr++ = y;
		*arr++ = z;
	}
	void CopyToArray(T *arr) const
	{
		return CopyToIter(arr);
	}

};


template<class VecType, std::size_t...I>
constexpr VecType add_impl(VecType v1, VecType v2, std::index_sequence<I...>)
{
	return {(v1[I] + v2[I])...};
}
template<class VecType, std::size_t...I>
constexpr VecType sub_impl(VecType v1, VecType v2, std::index_sequence<I...>)
{
	return {(v1[I] - v2[I])...};
}
template<class VecType, std::size_t...I>
constexpr VecType neg_impl(VecType vec, std::index_sequence<I...>)
{
	return {(-vec[I])...};
}
template<class VecType, std::size_t...I>
constexpr VecType valmul_impl(VecType vec, typename VecType::value_type val, std::index_sequence<I...>)
{
	return {(vec[I] * val)...};
}
template<class VecType, std::size_t...I>
constexpr VecType valdiv_impl(VecType vec, typename VecType::value_type val, std::index_sequence<I...>)
{
	return {(vec[I] / val)...};
}

#if __cplusplus >= 201703L
template<class Ret, class...Args> constexpr Ret Sum(Args...args)
{
	return (... + args);
}
template<class...Args> constexpr bool And(Args...args)
{
	return (... && args);
}
#else
template<class Ret, class First>
constexpr Ret Sum(First a)
{
	return a;
}
template<class Ret, class First, class...Args>
constexpr Ret Sum(First a, Args...args)
{
	return a + Sum<Ret>(args...);
}
constexpr bool And(bool a)
{
	return a;
}
template<class...Args>
constexpr bool And(bool a, Args...args)
{
	return a && And(args...);
}
#endif
template<class VecType, std::size_t...I>
constexpr bool equal_impl(VecType v1, VecType v2, std::index_sequence<I...>)
{
	return And((v1[I] == v2[I])...);
}
template<class VecType, std::size_t...I>
constexpr typename VecType::value_type DotProduct_impl(VecType v1, VecType v2, std::index_sequence<I...>)
{
	return Sum<typename VecType::value_type>((v1[I] * v2[I])...);
}
template<class VecType, std::size_t...I>
constexpr typename VecType::value_type LengthSquared_impl(VecType vec, std::index_sequence<I...>)
{
	return Sum<typename VecType::value_type>((vec[I] * vec[I])...);
}

template<class T, std::size_t N>
struct VectorBase : VectorBase_Gen<T, N, std::make_index_sequence<N>>
{
	using Base = VectorBase_Gen<T, N, std::make_index_sequence<N>>;
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
		return sqrt(LengthSquared());
	}

	VectorBase Normalize() const
	{
		auto flLen = Length();
		if (!flLen)
			return {};
		return *this / flLen;
	}

	T NormalizeInPlace()
	{
		auto flLen = Length();
		if (!flLen)
			return {};
		*this /= flLen;
		return flLen;
	}

	constexpr bool IsZero(T tolerance) const
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

template<class T> constexpr VectorBase<T, 3> CrossProduct(VectorBase<T, 3> a, VectorBase<T, 3> b)
{
	return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

}
}

namespace std {
#ifndef CLIENT_DLL
template<class T, std::size_t N> class tuple_size<::sv::moe::VectorBase<T, N>> : public std::integral_constant<std::size_t, N> {};
template<class T, std::size_t N, std::size_t I> class tuple_element<I, ::sv::moe::VectorBase<T, N>> { public: using type = T; };
#else
template<class T, std::size_t N> class tuple_size<::cl::moe::VectorBase<T, N>> : public std::integral_constant<std::size_t, N> {};
template<class T, std::size_t N, std::size_t I> class tuple_element<I, ::cl::moe::VectorBase<T, N>> { public: using type = T; };
#endif
}

#endif //PROJECT_U_VECTOR_HPP
