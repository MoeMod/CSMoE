#ifndef EVENTDISPATCHER_H
#define EVENTDISPATCHER_H
#ifdef _WIN32
#pragma once
#endif

#include <vector>
#include <algorithm>
#include <memory>

/*
class UnsubscribeException : public std::exception {};
template<class F>
class EventDispatcher;
template<class R, class...Args>
class EventDispatcher<R(Args...)>
{
public:
	using F = std::function<R(Args...)>;
	EventDispatcher() = default;
	EventDispatcher(const F &f) : v{ f } {}
	EventDispatcher(std::initializer_list<F> l) : v(l) {}
public:
	void dispatch(Args...args)
	{
		v.erase(std::remove_if(v.begin(), v.end(), [](const F &f) {
			try
			{
				f(std::forward<Args>(args)...);
			}
			catch (const UnsubscribeException &e)
			{
				return true;
			}
			return false;
		}), v.end());
	}
	void subscribe(const F &f)
	{
		if (f) v.push_back(f);
		return *this;
	}
private:
	std::vector<F> v;
};
*/

struct IBaseEventListener
{
	virtual ~IBaseEventListener() = 0;
};
inline IBaseEventListener::~IBaseEventListener() {}
using EventListener = std::shared_ptr<IBaseEventListener>;
template<class F>
class EventDispatcher;
template<class R, class...Args>
class EventDispatcher<R(Args...)>
{
	struct ICallable : IBaseEventListener
	{
		virtual void operator()(Args...args) = 0;
	};
	template<class F>
	struct CCallable : ICallable
	{
		CCallable(const F &f) : m_Function(f) {}
		F m_Function;
		void operator()(Args...args) override
		{
			return m_Function(std::forward<Args>(args)...);
		}
	};
public:
	// dont move args!
	template<class...RealArgs>
	void dispatch(RealArgs&&...args)
	{
		v.erase(std::remove_if(v.begin(), v.end(), [&](const std::weak_ptr<ICallable> &wpf) {
			auto spf = wpf.lock();
			if (!spf)
				return true;
			(*spf)(std::forward<RealArgs>(args)...);
			return false;
		}), v.end());
	}
	// note that the lifespan of EventListener object keeps the callback
	template<class F>
	EventListener subscribe(const F &f)
	{
		auto sp = std::make_shared<CCallable<F>>(f);
		v.push_back(sp);
		return sp;
	}
private:
	std::vector<std::weak_ptr<ICallable>> v;
};


#endif