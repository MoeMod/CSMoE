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
#include <numeric>
#include <functional>

#include "nodiscard.h"

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
	struct ParseArg<T &&>;

	// 使用函数指针进行类型擦除（不使用虚表）
	struct ICallable
	{
	public:
		explicit ICallable(R(ICallable::*p)(typename ParseArg<Args>::type...)) : pfn(p) {}
		R operator()(typename ParseArg<Args>::type...args)
		{
			//assert(pfn != nullptr && pfn != &ICallable::operator());
			return (this->*pfn)(args...);
		}
	private:
		R(ICallable::* const pfn)(typename ParseArg<Args>::type...);
	};

	template<class F>
	struct FunctionHolder
	{
		const F m_Function;
		R operator()(typename ParseArg<Args>::type...args)
		{
			return m_Function(args...);
		}
	};

	// 如果F是空类直接继承F作为空基类，否则用FunctionHolder<F>装下
	// ICallable必须是第一基类才能进行成员函数指针的转型
	template<class F, class MyFunctionHolder = typename std::conditional<std::is_empty<F>::value && !std::is_final<F>::value, F, FunctionHolder<F>>::type>
	class CCallable : public ICallable, private MyFunctionHolder
	{
	public:
		template<class RealF>
		explicit CCallable(RealF &&f) :
				ICallable( static_cast<R(ICallable::*)(typename ParseArg<Args>::type...)>(&CCallable::operator()) ),
				MyFunctionHolder{ std::forward<RealF>(f) }
		{}
		R operator()(typename ParseArg<Args>::type...args)
		{
			return MyFunctionHolder::operator()(args...);
		}
	};

private:
	// 找出还存在的订阅集合并且返回
	std::vector<std::shared_ptr<ICallable>> makeCallableSet() const
	{
		std::vector<std::shared_ptr<ICallable>> sv(v.size());
		// 将弱引用变成强引用
		std::transform(v.begin(), v.end(), sv.begin(), std::mem_fn(&std::weak_ptr<ICallable>::lock));
		// 去掉已经失效的订阅
		sv.erase(std::remove(sv.begin(), sv.end(), nullptr), sv.end());
		return sv;
	}
	// 用新的订阅集合替换之前的（按值传递是故意的）
	void applyCallableSet(std::vector<std::weak_ptr<ICallable>> new_v) noexcept(std::is_nothrow_move_assignable<decltype(new_v)>::value)
	{
		v = std::move(new_v);
	}

public:
	// TODO 非线程安全
	// 发布订阅，无视返回值（可能以任意回调函数抛出异常中断）
	void dispatch(typename ParseArg<Args>::type...args)
	{
		const std::vector<std::shared_ptr<ICallable>> sv = makeCallableSet();
		std::for_each(sv.begin(), sv.end(), [&args...](const std::shared_ptr<ICallable> &sp) { return (*sp)(args...); });
		applyCallableSet({ sv.begin(), sv.end() });
	}
	// 发布订阅，并将调用的结果拷贝至out
	// 注意：用conditional将R转换为依赖名才能进行SFINAE
	template<class OutputIter, class = typename std::enable_if<!std::is_void<typename std::conditional<false, OutputIter, R>::type>::value>::type>
	void dispatch_copy(typename ParseArg<Args>::type...args, OutputIter out)
	{
		const std::vector<std::shared_ptr<ICallable>> sv = makeCallableSet();
		std::transform(sv.begin(), sv.end(), out, [&args...](const std::shared_ptr<ICallable> &sp) { return (*sp)(args...); });
		applyCallableSet({ sv.begin(), sv.end() });
	}
	// 发布订阅，并使用map-reduce产生结果
	// 注意：用conditional将R转换为依赖名才能进行SFINAE
	template<class Mapper, class Reducer, class ResultType, class = typename std::enable_if<!std::is_void<typename std::conditional<false, ResultType, R>::type>::value>::type>
	ResultType dispatch_map_reduce(typename ParseArg<Args>::type...args, Mapper mapper, Reducer reducer, ResultType init)
	{
		const std::vector<std::shared_ptr<ICallable>> sv = makeCallableSet();
		const ResultType Result = std::inner_product(sv.begin(), sv.end(), sv.begin(), std::move(init), reducer,
		                                             [mapper, &args...](const std::shared_ptr<ICallable> &sp, const std::shared_ptr<ICallable> &) { return mapper( (*sp)(args...) ); }
		);
		applyCallableSet({ sv.begin(), sv.end() });
		return Result;
	}
	// 注册订阅普通回调函数，EventListener析构时订阅自动注销
	template<class RealF>
	NODISCARD EventListener subscribe(RealF &&f)
	{
		auto sp = std::make_shared<CCallable<typename std::decay<RealF>::type>>(std::forward<RealF>(f));
		v.emplace_back(sp);
		return std::move(sp);
	}
	// 注册订阅类成员函数
	template<class C, class M>
	NODISCARD EventListener subscribe(M C::*pmem_fn, C *pthis)
	{
		return subscribe([pmem_fn, pthis](typename ParseArg<Args>::type...args) { return (pthis->*pmem_fn)(args...); });
	}

private:
	std::vector<std::weak_ptr<ICallable>> v;
};


#endif
