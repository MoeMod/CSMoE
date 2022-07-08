/*
WeaponDataVaribles.hpp - part of CSMoE template weapon framework
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

#include "ExpressionBuilder.hpp"

namespace WeaponTemplate {
	namespace Varibles
	{
		namespace detail
		{
			class id_A;
			class id_T;
			class id_N;
		}
		constexpr auto A = ExpressionBuilder::VaribleType<detail::id_A>{};
		constexpr auto T = ExpressionBuilder::VaribleType<detail::id_T>{};
		constexpr auto N = ExpressionBuilder::VaribleType<detail::id_N>{};
	}
}
