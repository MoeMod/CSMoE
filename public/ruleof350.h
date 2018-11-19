#ifndef RULEOF350_H
#define RULEOF350_H
#ifdef _WIN32
#pragma once
#endif

//  Private copy constructor and copy assignment ensure classes derived from
//  class noncopyable cannot be copied.

//  Contributed by Dave Abrahams

namespace detail  // protection from unintended ADL
{

	class noncopyable
	{
	protected:
		noncopyable() = default;
		~noncopyable() = default;
	public:
		noncopyable(const noncopyable&) = delete;
		const noncopyable& operator=(const noncopyable&) = delete;
	};

	class nonmoveable
	{
	protected:
		nonmoveable() = default;
		~nonmoveable() = default;
	public:
		nonmoveable(nonmoveable&&) = delete;
		nonmoveable& operator=(nonmoveable&&) = delete;
	};

	class unique : public noncopyable, public nonmoveable
	{
	protected:
		unique() = default;
		~unique() = default;
	};
}

namespace ruleof350
{
	typedef detail::noncopyable noncopyable;
	typedef detail::nonmoveable nonmoveable;
	typedef detail::unique unique;
}

#endif