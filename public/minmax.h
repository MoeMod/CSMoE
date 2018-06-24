#ifndef MINMAX_H
#define MINMAX_H
#ifdef _WIN32
#pragma once
#endif

#ifndef __cplusplus
#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif
#else // __cplusplus

#include<algorithm>
using std::min;
using std::max;

#endif // __cplusplus

#endif