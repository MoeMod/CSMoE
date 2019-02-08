//
// Created by 小白白 on 2019-02-09.
//

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
