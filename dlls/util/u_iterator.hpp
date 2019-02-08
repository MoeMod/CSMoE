//
// Created by 小白白 on 2019-02-09.
//

#ifndef PROJECT_U_ITERATOR_HPP
#define PROJECT_U_ITERATOR_HPP

#include <iterator>
#include "u_ebobase.hpp"

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
		explicit Enum_Iterator(value_type p = nullptr, const Enumer &f = Enumer())
				: m_pCurrent(p), EBOBase<Enumer>(f) {}

	public:
		Enum_Iterator &operator++() noexcept { return (m_pCurrent = EBOBase<Enumer>::get()(m_pCurrent)), *this; }

		Enum_Iterator operator++(int) noexcept {
			return Enum_Iterator(std::exchange(m_pCurrent, EBOBase<Enumer>::get()(m_pCurrent)));
		}

		reference operator*() const noexcept { return m_pCurrent; }

	public:
		bool operator==(const Enum_Iterator &other) const noexcept { return m_pCurrent == other.m_pCurrent; }

		bool operator!=(const Enum_Iterator &other) const noexcept { return !(*this == other); }

	private:
		value_type m_pCurrent;
	};
}
}


#endif //PROJECT_U_ITERATOR_HPP
