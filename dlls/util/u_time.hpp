/*
u_time.hpp - wrap time with std::chrono in Modern C++
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

#pragma once

#include <chrono>

namespace sv {
	namespace moe {
		struct EngineClock
		{
			using rep = float;
			using period = std::ratio<1>;
			using duration = std::chrono::duration<rep, period>;
			using time_point = std::chrono::time_point<EngineClock>;
			static constexpr bool is_steady = false;
			static time_point now()
			{
				return time_point(duration(gpGlobals->time));
			}
		};
		using namespace std::chrono_literals;

		inline EngineClock::time_point get_gametime()
		{
			return EngineClock::now();
		}

		static_assert(sizeof(EngineClock::duration) == sizeof(float) && sizeof(EngineClock::time_point) == sizeof(float), "EngineClock has not the same layout with engine.");
	}
}