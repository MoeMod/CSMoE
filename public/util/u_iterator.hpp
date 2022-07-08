/*
u_iterator.hpp - some iterators wrapping UTIL_* with Modern C++
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

#ifndef PROJECT_U_ITERATOR_HPP
#define PROJECT_U_ITERATOR_HPP

#include <iterator>
#include "meta/EBOBase.hpp"

#ifndef CLIENT_DLL
namespace sv {
#else
namespace cl {
#endif
namespace moe {
namespace iterator {
	/*
	 * template<class EntityType, class Enumer>
	 * concept Enumer_c = =requires(Enumer e, EntityType p)
	 * {
	 *      {e(p)} -> EntityType;
	 * }
	 */

	template<class Enumer>
	class Enum_Iterator : private EBOBase<Enumer>
    {
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = decltype(std::declval<Enumer>()(nullptr)) ;
		using difference_type = ptrdiff_t;
		using pointer = const value_type *;
		using reference = const value_type &;

	public:
		constexpr explicit Enum_Iterator(value_type p = nullptr, const Enumer &f = Enumer())
				: EBOBase<Enumer>(f), m_pCurrent(p) {}

	public:
		Enum_Iterator &operator++() noexcept { return (m_pCurrent = EBOBase<Enumer>::get()(m_pCurrent)), *this; }

		const Enum_Iterator operator++(int) noexcept {
			return Enum_Iterator(std::exchange(m_pCurrent, EBOBase<Enumer>::get()(m_pCurrent)));
		}

		constexpr reference operator*() const noexcept { return m_pCurrent; }

	public:
		constexpr bool operator==(const Enum_Iterator &other) const noexcept { return m_pCurrent == other.m_pCurrent; }

		constexpr bool operator!=(const Enum_Iterator &other) const noexcept { return !(*this == other); }

	private:
		value_type m_pCurrent;
	};
}
}

}
#endif //PROJECT_U_ITERATOR_HPP
