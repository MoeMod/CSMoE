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
#include <random>
#include <cmath>
#include "maintypes.h"

#ifndef CLIENT_DLL
namespace sv {
#else
namespace cl {
#endif
	namespace moe {
		struct EngineClock
		{
			using rep = float;
			using period = std::ratio<1>;
			using duration = std::chrono::duration<rep, period>;
			using time_point = std::chrono::time_point<EngineClock>;
			static constexpr bool is_steady = false;
			static time_point now() noexcept;

			/*
			struct TimePointConverter
			{
				time_point tp;
				DEPRECATED operator float() noexcept
				{
					return tp.time_since_epoch().count();
				}
				DEPRECATED operator time_point() noexcept
				{
					return tp;
				}
			};

			// ADL should find those friends
			DEPRECATED friend TimePointConverter operator+(time_point a, float b) noexcept
			{
				return { a + duration(b) };
			}
			DEPRECATED friend TimePointConverter operator-(time_point a, float b) noexcept
			{
				return { a - duration(b) };
			}
			DEPRECATED friend TimePointConverter operator+(float a, time_point b) noexcept
			{
				return { duration(a) + b };
			}

			DEPRECATED friend constexpr bool operator>(time_point a, float b) noexcept { return a.time_since_epoch().count() > b; }
			DEPRECATED friend constexpr bool operator>=(time_point a, float b) noexcept { return a.time_since_epoch().count() >= b; }
			DEPRECATED friend constexpr bool operator<(time_point a, float b) noexcept { return a.time_since_epoch().count() < b; }
			DEPRECATED friend constexpr bool operator<=(time_point a, float b) noexcept { return a.time_since_epoch().count() <= b; }
			DEPRECATED friend constexpr bool operator==(time_point a, float b) noexcept { return a.time_since_epoch().count() == b; }
			DEPRECATED friend constexpr bool operator!=(time_point a, float b) noexcept { return a.time_since_epoch().count() != b; }
			DEPRECATED friend constexpr bool operator>(float a, time_point b) noexcept { return a > b.time_since_epoch().count(); }
			DEPRECATED friend constexpr bool operator>=(float a, time_point b) noexcept { return a >= b.time_since_epoch().count(); }
			DEPRECATED friend constexpr bool operator<(float a, time_point b) noexcept { return a < b.time_since_epoch().count(); }
			DEPRECATED friend constexpr bool operator<=(float a, time_point b) noexcept { return a <= b.time_since_epoch().count(); }
			DEPRECATED friend constexpr bool operator==(float a, time_point b) noexcept { return a == b.time_since_epoch().count(); }
			DEPRECATED friend constexpr bool operator!=(float a, time_point b) noexcept { return a != b.time_since_epoch().count(); }
			*/
		};
		using namespace std::chrono_literals;

		inline EngineClock::time_point get_gametime() noexcept
		{
			return EngineClock::now();
		}

		constexpr EngineClock::duration zero_duration = EngineClock::duration::zero();
		constexpr EngineClock::time_point invalid_time_point = EngineClock::time_point(zero_duration);

		static_assert(sizeof(EngineClock::duration) == sizeof(float) && sizeof(EngineClock::time_point) == sizeof(float), "EngineClock has not the same layout with engine.");

		template<class T, class RandomEngine>
		auto RandomNumber(T a, T b, RandomEngine &rd) -> typename std::enable_if<std::is_integral<T>::value, T>::type
		{
			return std::uniform_int_distribution<T>(a, b)(rd);
		}

		template<class T, class RandomEngine>
		auto RandomNumber(T a, T b, RandomEngine &rd) -> typename std::enable_if<std::is_floating_point<T>::value, T>::type
		{
			return std::uniform_real_distribution<T>(a, b)(rd);
		}

		template<class Rep1, class Rep2, class RandomEngine = std::random_device, class RetRep = typename std::common_type<Rep1, Rep2>::type>
		auto RandomDuration(std::chrono::duration<Rep1> a, std::chrono::duration<Rep2> b, RandomEngine &rd) -> std::chrono::duration<RetRep>
		{
			
			return std::chrono::duration<RetRep>(RandomNumber<RetRep>(a.count(), b.count(), rd));
		}

		template<class Rep1, class Rep2, class RetRep = typename std::common_type<Rep1, Rep2>::type>
		auto RandomDuration(std::chrono::duration<Rep1> a, std::chrono::duration<Rep2> b) -> std::chrono::duration<RetRep>
		{
			std::random_device rd;
			return RandomDuration(a, b, rd);
		}

		template<class Rep1, class Rep2, class RetRep = typename std::common_type<Rep1, Rep2>::type>
		auto LerpDuration(std::chrono::duration<Rep1> a, std::chrono::duration<Rep2> b, RetRep t) -> typename std::enable_if<std::is_floating_point_v<RetRep>, std::chrono::duration<RetRep>>::type
		{
			return std::chrono::duration<RetRep>(std::lerp((RetRep)a.count(), (RetRep)b.count(), t));
		}
	}
}