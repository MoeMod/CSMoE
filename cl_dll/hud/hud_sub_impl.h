/*
hud_sub.h - CSMoE Client HUD : Pimpl Auto-implement, for hud_xxx.cpp
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

#pragma once

#include "hud_sub.h"
#include <tuple>

namespace detail
{
	template<class T, class...Args>
	struct TypeExists;
	template<class T, class First, class...Args>
	struct TypeExists<T, First, Args...> : TypeExists<T, Args...> {};
	template<class T, class...Args>
	struct TypeExists<T, T, Args...> : std::true_type {};
	template<class T>
	struct TypeExists<T> : std::false_type {};

	template<class...Args>
	struct IsUnique;
	template<class First>
	struct IsUnique<First> : std::true_type {};
	template<class First, class Second, class...Args>
	struct IsUnique<First, Second, Args...> : std::integral_constant<bool,
		!TypeExists<First, Second, Args...>::value && IsUnique<First, Args...>::value && IsUnique<Second, Args...>::value
	> {};
	
	template<class T, class First, class...Tail>
	struct FindElementId : std::integral_constant<size_t, FindElementId<T, Tail...>::value + 1> {};
	template<class T, class...Tail>
	struct FindElementId<T, T, Tail...> : std::integral_constant<size_t, 0> {};
}



// usage:
// class CHudZB2::impl_t : public THudSubDispatcher<...> {...};
// in hud_zb2.cpp
template<class...Elements>
class THudSubDispatcher
{
	// ensure that all elements appear only once
	static_assert(detail::IsUnique<Elements...>::value, "Elements should be unique!");
private:
	std::tuple<Elements...> t;
public:
	template<class T>
	T &get() noexcept
	{
		return std::get<detail::FindElementId<T, Elements...>::value>(t);
	}
	template<class F, class...Args>
	void for_each(F IBaseHudSub::*f, Args &&...args)
	{
		int x[]{ ((get<Elements>().*f)(std::forward<Args>(args)...), 0)... };
		return void(x);
	}
};
