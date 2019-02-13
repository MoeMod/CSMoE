#pragma once

#include <tuple>

// limited RAII support for convenience...
class IBaseHudSub
{
protected:
	virtual		~IBaseHudSub() {}
public:
	virtual int VidInit(void) { return 0; }
	virtual int Draw(float flTime) { return 0; }
	virtual void Think(void) { return; }
	virtual void Reset(void) { return; }
	virtual void InitHUDData(void) {}		// called every time a server is connected to
};

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
}

template<class T, class First, class...Tail>
struct FindElementId : std::integral_constant<size_t, FindElementId<T, Tail...>::value + 1> {};
template<class T, class...Tail>
struct FindElementId<T, T, Tail...> : std::integral_constant<size_t, 0> {};

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
		return std::get<FindElementId<T, Elements...>::value>(t);
	}
	template<class F, class...Args>
	void for_each(F IBaseHudSub::*f, Args &&...args)
	{
		int x[]{ ((get<Elements>().*f)(std::forward<Args>(args)...), 0)... };
	}
};
