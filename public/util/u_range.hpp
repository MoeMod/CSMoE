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

#include "meta/EBOBase.hpp"
#include "u_iterator.hpp"
#include "u_functor.hpp"

#ifndef CLIENT_DLL
namespace sv {
#else
namespace cl {
#endif
namespace moe {
	namespace range
	{
		using iterator::Enum_Iterator;
		template<class Enumer>
		struct EntityList : private EBOBase<Enumer>
        {
			template<class...Args> explicit EntityList(Args &&...args) : EBOBase<Enumer>(Enumer(std::forward<Args>(args)...)) {}
			Enum_Iterator<Enumer> begin() const  { return Enum_Iterator<Enumer> (EBOBase<Enumer>::get()(nullptr), EBOBase<Enumer>::get() ); }
			Enum_Iterator<Enumer> end() const { return Enum_Iterator<Enumer> (nullptr, EBOBase<Enumer>::get() ); }
		};

		struct PlayersList : EntityList<Enumer_Player> {};

		/*struct PlayersList
		{
			Enum_Iterator<Enumer_Player> begin() const { return Enum_Iterator<Enumer_Player> (Enumer_Player()(nullptr), Enumer_Player() ); }
			Enum_Iterator<Enumer_Player> end() const { return Enum_Iterator<Enumer_Player> (nullptr, Enumer_Player() ); }
		};*/
	}
}
}

#endif //PROJECT_U_RANGE_HPP
