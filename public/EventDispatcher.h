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
		using type = T &;
	};
	template<class T>
	struct ParseArg<T &&>
	{
		template<class = void> struct Error;
		using type = typename Error<>::not_allowed_to_move_arg;
	};

	struct ICallable
	{
		virtual void operator()(typename ParseArg<Args>::type...args) = 0;
	};
	template<class F>
	struct CCallable : ICallable
	{
		template<class RealF>
		CCallable(RealF &&f) : m_Function(std::forward<RealF>(f)) {}
		F m_Function;
		void operator()(typename ParseArg<Args>::type...args) override
		{
			m_Function(args...);
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
		auto sp = std::make_shared<CCallable<F>>(std::forward<F>(f));
		v.push_back(sp);
		return sp;
	}
private:
	std::vector<std::weak_ptr<ICallable>> v;
};


#endif