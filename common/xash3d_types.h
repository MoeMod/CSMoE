// basic typedefs
#ifndef XASH_TYPES_H
#define XASH_TYPES_H

#include "util/u_vector.hpp"

typedef unsigned char byte;
typedef int		sound_t;
typedef float		vec_t;
using vec2_t = moe::VectorBase<float, 2>;
using vec2_t_ref = vec2_t &;

using vec3_t = moe::VectorBase<float, 3, 16>;
using vec3_c = moe::VectorBase<float, 3>;
using vec3_t_ref = vec3_t &;

using vec4_t = moe::VectorBase<float, 4, 16>;
using vec4_t_ref = vec4_t &;

typedef byte		rgba_t[4];	// unsigned byte colorpack
typedef byte		rgb_t[3];		// unsigned byte colorpack

using matrix3x4 = moe::VectorBase<vec4_t, 3>;
using matrix4x4 = moe::VectorBase<vec4_t, 4>;

using matrix3x4_ref = matrix3x4 &;
using matrix4x4_ref = matrix4x4 &;

using cmatrix3x4 = const matrix3x4 &;
using cmatrix4x4 = const matrix4x4 &;

#ifndef time_point_t
typedef float		time_point_t;
#endif
#ifndef duration_t
typedef float		duration_t;
#endif

#undef true
#undef false

#ifndef __cplusplus
typedef enum { false, true }	qboolean;
#else
typedef int qboolean;
#endif

#if _MSC_VER == 1200
typedef __int64 integer64; //msvc6
#elif defined (XASH_SDL)
typedef uint64_t integer64;
#else
typedef unsigned long long integer64;
#endif
typedef integer64 longtime_t;

typedef struct mempool_s mempool_t;
#endif // XASH_TYPES_H
