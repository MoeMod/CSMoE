#pragma once

#include <tuple>

// limited RAII support for convenience...
class IBaseHudSub
{
protected:
	virtual		~IBaseHudSub() {}
public:
	virtual int VidInit( void ) {return 0;}
	virtual int Draw(float flTime) {return 0;}
	virtual void Think(void) {return;}
	virtual void Reset(void) {return;}
	virtual void InitHUDData( void ) {}		// called every time a server is connected to
};

template<class...Elements>
class THudSubDispatcher
{
private:
	std::tuple<Elements...> t;
public:
	template<class T>
	constexpr T &get() noexcept
	{
		return std::get<T>(t);
	}
	template<class F, class...Args>
	void for_each(F IBaseHudSub::*f, Args &&...args)
	{
		int x[] { ((get<Elements>().*f)(std::forward<Args>(args)...), 0)... };
	}
};