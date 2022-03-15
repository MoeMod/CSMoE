#ifndef MINMAX_H
#define MINMAX_H
#ifdef _WIN32
#pragma once
#endif

#ifdef min
#undef min
#endif 
#ifdef max
#undef max
#endif 


#ifndef __cplusplus
#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif
#else // __cplusplus

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include <algorithm>
#include <type_traits>
using std::min;
using std::max;

template<class T1, class T2, class Ret = typename std::common_type<T1, T2>::type>
auto min(const T1 &a, const T2 &b) -> typename std::enable_if<!std::is_same<T1, T2>::value, Ret>::type
{
    return std::min(static_cast<Ret>(a), static_cast<Ret>(b));
}
template<class T1, class T2, class Ret = typename std::common_type<T1, T2>::type>
auto max(const T1 &a, const T2 &b) -> typename std::enable_if<!std::is_same<T1, T2>::value, Ret>::type
{
    return std::max(static_cast<Ret>(a), static_cast<Ret>(b));
}

#endif // __cplusplus

#endif