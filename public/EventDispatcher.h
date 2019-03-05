/*
EventDispatcher.h - simple RAII and functor-based observer pattern implement
Copyright (C) 2018 Moemod Hyakuya

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef EVENTDISPATCHER_H
#define EVENTDISPATCHER_H
#ifdef _WIN32
#pragma once
#endif

#include <vector>
#include <algorithm>
#include <memory>

using EventListener = std::shared_ptr<void>;
template<class F>
class EventDispatcher;
template<class R, class...Args>
class EventDispatcher<R(Args...)>
{
	template<class T>
	struct ParseArg
	{
		using type = const T &;
	};
	template<class T>
	struct ParseArg<T &>
	{
		using type = T & ;
	};
	template<class T>
	struct ParseArg<T &&>
	{
		template<class = void> struct Error;
		using type = typename Error<>::not_allowed_to_move_arg;
	};

	struct ICallable
	{
		// NOT using vtable any more! 
		ICallable(void(ICallable::*p)(typename ParseArg<Args>::type...) const) : pfn(p) {}
		void(ICallable::* const pfn)(typename ParseArg<Args>::type...) const = nullptr;
		void operator()(typename ParseArg<Args>::type...args) const
		{
			(this->*pfn)(args...);
		}
	};
	template<class F, class = void>
	struct CCallable : ICallable
	{
		template<class RealF>
		CCallable(RealF &&f) :
			ICallable(static_cast<void(ICallable::*)(typename ParseArg<Args>::type...) const>(&CCallable::operator())),
			m_Function(std::forward<RealF>(f))
		{}
		const F m_Function;
		void operator()(typename ParseArg<Args>::type...args) const
		{
			m_Function(args...);
		}
	};
	// empty base-class optimization for F...
	template<class F>
	struct CCallable<F, typename std::enable_if<std::is_empty<F>::value>::type> : ICallable, private F
	{
		template<class RealF>
		CCallable(RealF &&f) :
			ICallable{ static_cast<void(ICallable::*)(typename ParseArg<Args>::type...)>(&CCallable::operator()) },
			F(std::forward<RealF>(f))
		{}
		void operator()(typename ParseArg<Args>::type...args) const
		{
			F::operator()(args...);
		}
	};
public:
	// dont move args!
	void dispatch(typename ParseArg<Args>::type...args)
	{
		v.erase(std::remove_if(v.begin(), v.end(), [&](const std::weak_ptr<ICallable> &wpf) {
			auto spf = wpf.lock();
			if (!spf)
				return true;
			(*spf)(args...);
			return false;
		}), v.end());
	}
	// note that the lifespan of EventListener object keeps the callback
	template<class F>
	/*[[nodiscard]]*/ EventListener subscribe(F &&f)
	{
		auto sp = std::make_shared<CCallable<typename std::remove_reference<F>::type>>(std::forward<F>(f));
		v.emplace_back(sp);
		return sp;
	}

	template<class C, class Ret, class...ArgsListener>
	/*[[nodiscard]]*/ EventListener subscribe(Ret (C::*pmem_fn)(ArgsListener...), C *pthis)
	{
		return subscribe([pmem_fn, pthis](typename ParseArg<Args>::type...args) { return (pthis->*pmem_fn)(args...); });
	}

private:
	std::vector<std::weak_ptr<ICallable>> v;
};


#endif
