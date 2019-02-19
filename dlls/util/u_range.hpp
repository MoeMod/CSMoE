/*
u_range.hpp - some ranges wrapping UTIL_* with Modern C++
Copyright (C) 2019 Moemod Hyakuya

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef PROJECT_U_RANGE_HPP
#define PROJECT_U_RANGE_HPP

#include "u_iterator.hpp"
#include "u_functor.hpp"

namespace moe
{
	namespace range
	{
		using iterator::Enum_Iterator;
		struct PlayersList
		{
			static Enum_Iterator<Enumer_Player> begin() { return Enum_Iterator<Enumer_Player> (Enumer_Player()(nullptr), Enumer_Player() ); }
			static Enum_Iterator<Enumer_Player> end() { return Enum_Iterator<Enumer_Player> (nullptr, Enumer_Player() ); }
		};
	}

}

#endif //PROJECT_U_RANGE_HPP
