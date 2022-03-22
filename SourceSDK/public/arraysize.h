#ifndef ARRAYSIZE_H
#define ARRAYSIZE_H
#ifdef _WIN32
#pragma once
#endif

#ifndef __cplusplus
#ifndef ARRAYSIZE
#define ARRAYSIZE(p)		(sizeof(p)/sizeof(p[0]))
#endif
#else // __cplusplus

#ifdef ARRAYSIZE
#undef ARRAYSIZE
#endif

#include <type_traits>
#define ARRAYSIZE(x) (std::extent<decltype(x)>::value)

#endif // __cplusplus

#endif