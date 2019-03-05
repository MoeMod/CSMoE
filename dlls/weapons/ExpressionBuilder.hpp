/*
ExpressionBuilder.hpp - compile-time simple math expression lambda generator
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
#include <type_traits>

namespace ExpressionBuilder
{
	namespace detail
	{
		struct Expression {};
		template<class T>
		struct IsExpression : std::is_base_of<Expression, T> {};
		template<class UnderlyingType>
		struct ConstantType : Expression
		{
			const UnderlyingType c;
			constexpr explicit ConstantType(UnderlyingType a) : c(a) {}
			template<class T>
			constexpr UnderlyingType operator()(T x) const
			{
				return c;
			}
			constexpr operator UnderlyingType() const { return c; }
		};

		template<class Varible, class Constant>
		struct BindedType
		{
			const Varible var;
			const Constant con;
			constexpr explicit BindedType(Varible x, Constant c) : var(x), con(c) {}

			template<class T>
			constexpr auto operator()(T x) const -> BindedType<Varible, Constant>
			{
				return BindedType<Varible, Constant>(var, con);
			}
		};

		template<char Name>
		struct VaribleType : Expression
		{
			template<class UnderlyingType>
			constexpr auto operator=(UnderlyingType c) const -> BindedType<VaribleType<Name>, ConstantType<UnderlyingType>>
			{
				return BindedType<VaribleType<Name>, ConstantType<UnderlyingType>>(VaribleType<Name>(), ConstantType<UnderlyingType>(c));
			}

			template<class UnderlyingType>
			constexpr auto operator()(BindedType<VaribleType<Name>, ConstantType<UnderlyingType>> x) const -> UnderlyingType
			{
				return x.con.c;
			}

			template<class T>
			constexpr auto operator()(T x) const -> VaribleType<Name>
			{
				return VaribleType<Name>();
			}
		};

		template<class Exp1, class Exp2>
		struct BinaryOperator : Expression
		{
			const Exp1 _1;
			const Exp2 _2;
			constexpr BinaryOperator(Exp1 a, Exp2 b) : _1(a), _2(b) {}
		};

		template<class T>
		constexpr auto varcon(T x) -> typename std::enable_if<!IsExpression<T>::value, ConstantType<T>>::type
		{
			return ConstantType<T>(x);
		}
		template<class T>
		constexpr auto varcon(T x) -> typename std::enable_if<IsExpression<T>::value, T>::type
		{
			return x;
		}

		template<class Exp1, class Exp2>
		struct OperatorPlus_t : BinaryOperator<Exp1, Exp2>
		{
			using BinaryOperator<Exp1, Exp2>::BinaryOperator;
			using BinaryOperator<Exp1, Exp2>::_1;
			using BinaryOperator<Exp1, Exp2>::_2;
			template<class T>
			constexpr auto operator()(T x) const -> decltype(_1(x) + _2(x)) { return _1(x) + _2(x); }
		};

		template<class Exp1, class Exp2>
		struct OperatorMinus_t : BinaryOperator<Exp1, Exp2>
		{
			using BinaryOperator<Exp1, Exp2>::BinaryOperator;
			using BinaryOperator<Exp1, Exp2>::_1;
			using BinaryOperator<Exp1, Exp2>::_2;
			template<class T>
			constexpr auto operator()(T x) const -> decltype(_1(x) - _2(x)) { return _1(x) - _2(x); }
		};
		template<class Exp1, class Exp2>
		struct OperatorMul_t : BinaryOperator<Exp1, Exp2>
		{
			using BinaryOperator<Exp1, Exp2>::BinaryOperator;
			using BinaryOperator<Exp1, Exp2>::_1;
			using BinaryOperator<Exp1, Exp2>::_2;
			template<class T>
			constexpr auto operator()(T x) const -> decltype(_1(x) * _2(x)) { return _1(x) * _2(x); }
		};
		template<class Exp1, class Exp2>
		struct OperatorDiv_t : BinaryOperator<Exp1, Exp2>
		{
			using BinaryOperator<Exp1, Exp2>::BinaryOperator;
			using BinaryOperator<Exp1, Exp2>::_1;
			using BinaryOperator<Exp1, Exp2>::_2;
			template<class T>
			constexpr auto operator()(T x) const -> decltype(_1(x) / _2(x)) { return _1(x) / _2(x); }
		};

		template<class Exp1, class Exp2, class = typename std::enable_if<IsExpression<Exp1>::value || IsExpression<Exp2>::value>::type>
		constexpr auto operator+(Exp1 _1, Exp2 _2) -> OperatorPlus_t<decltype(varcon(_1)), decltype(varcon(_2))>
		{
			return OperatorPlus_t<decltype(varcon(_1)), decltype(varcon(_2))>(varcon(_1), varcon(_2));
		}
		template<class Exp1, class Exp2, class = typename std::enable_if<IsExpression<Exp1>::value || IsExpression<Exp2>::value>::type>
		constexpr auto operator-(Exp1 _1, Exp2 _2) -> OperatorMinus_t<decltype(varcon(_1)), decltype(varcon(_2))>
		{
			return OperatorMinus_t<decltype(varcon(_1)), decltype(varcon(_2))>(varcon(_1), varcon(_2));
		}
		template<class Exp1, class Exp2, class = typename std::enable_if<IsExpression<Exp1>::value || IsExpression<Exp2>::value>::type>
		constexpr auto operator*(Exp1 _1, Exp2 _2) -> OperatorMul_t<decltype(varcon(_1)), decltype(varcon(_2))>
		{
			return OperatorMul_t<decltype(varcon(_1)), decltype(varcon(_2))>(varcon(_1), varcon(_2));
		}
		template<class Exp1, class Exp2, class = typename std::enable_if<IsExpression<Exp1>::value || IsExpression<Exp2>::value>::type>
		constexpr auto operator/(Exp1 _1, Exp2 _2) -> OperatorDiv_t<decltype(varcon(_1)), decltype(varcon(_2))>
		{
			return OperatorDiv_t<decltype(varcon(_1)), decltype(varcon(_2))>(varcon(_1), varcon(_2));
		}
	}

	constexpr const detail::VaribleType<'x'> x{};
	constexpr const detail::VaribleType<'y'> y{};
	constexpr const detail::VaribleType<'z'> z{};
}
