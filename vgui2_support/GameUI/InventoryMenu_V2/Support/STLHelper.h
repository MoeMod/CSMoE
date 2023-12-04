#pragma once

#include <string>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <tuple>
#include <vector>
#include <iterator>

inline std::string ToLower(std::string sz)
{
	std::transform(sz.cbegin(), sz.cend(), sz.begin(), ::tolower);
	return sz;
}

inline std::string ToUpper(std::string sz)
{
	std::transform(sz.cbegin(), sz.cend(), sz.begin(), ::toupper);
	return sz;
}

inline std::string Trim(std::string sz)
{
	const char* delim = " \t";
	sz.erase(0, sz.find_first_not_of(delim));
	sz.erase(sz.find_last_not_of(delim) + 1);
	return sz;
}

inline std::wstring Trim(std::wstring sz)
{
	const wchar_t* delim = L" \t";
	sz.erase(0, sz.find_first_not_of(delim));
	sz.erase(sz.find_last_not_of(delim) + 1);
	return sz;
}

std::vector<std::string> ParseString(const std::string& sz);
std::vector<std::wstring> ParseString(const std::wstring& sz);

template<class...Args>
inline std::string MakeString(Args&&...args)
{
	std::ostringstream oss;
	// (oss << ... << args);
	int p[] = { ((oss << args), 0)... };
	return oss.str();
}

template<class...Args>
inline std::wstring MakeStringW(const Args&...args)
{
	std::wostringstream oss;
	// (oss << ... << args);
	int p[] = { ((oss << args), 0)... };
	return oss.str();
}

template<size_t n, class First, class...Args>
struct VA_GetArg
{
	using type = typename VA_GetArg<n - 1, Args...>::type;
};
template<class First, class...Args>
struct VA_GetArg<0, First, Args...>
{
	using type = First;
};
template<size_t n, class T>
struct Template_GetArg {};
template<template<class...> class T, size_t n, class...Args>
struct Template_GetArg<n, T<Args...>>
{
	using type = typename VA_GetArg<n, Args...>::type;
};

template<size_t n, class Tuple>
inline bool IsKeyEqualsToTupleElement(const typename Template_GetArg<n, Tuple>::type& sz, const Tuple& pair)
{
	return sz == std::get<n>(pair);
}

struct range_iterator
{
	int n, step, end;
	using difference_type = int;
	using iterator_category = std::input_iterator_tag;
	using pointer = range_iterator;
	using reference = int&;
	using value_type = int;
	range_iterator& operator++() { return  n += step, *this; }
	bool operator!=(range_iterator& other) { return is_end() != other.is_end() && n != other.n; }
	int operator*() { return n; }
	bool is_end() { return n >= end; }
};

struct range_iterator_one
{
	int n;
	using difference_type = int;
	using iterator_category = std::input_iterator_tag;
	using pointer = range_iterator_one;
	using reference = int&;
	using value_type = int;
	range_iterator_one& operator++() { return  ++n, * this; }
	bool operator!=(range_iterator_one& other) { return n != other.n; }
	int operator*() { return n; }
};

struct range_generator
{
	int start, stop, step;
	range_iterator begin() { return { start, step, stop }; }
	range_iterator end() { return { stop, step, stop }; }
};

struct range_generator_one
{
	int start, stop;
	range_iterator_one begin() { return { start }; }
	range_iterator_one end() { return { stop }; }
};

inline range_generator xrange(int start, int stop, int step)
{
	return { start ,stop , step };
}

inline range_generator_one xrange(int start, int stop)
{
	return { start ,stop };
}

template<class...Args>
inline std::vector<int> range(Args&&...args)
{
	auto&& ran = xrange(args...);
	return std::vector<int>(ran.begin(), ran.end());
}