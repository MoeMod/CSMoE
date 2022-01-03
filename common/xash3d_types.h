// basic typedefs
#ifndef XASH_TYPES_H
#define XASH_TYPES_H

#ifndef XASH_ALIGN
#ifdef __cplusplus
#define XASH_ALIGN(N) alignas(N)
#elif defined _MSC_VER
#define XASH_ALIGN(N) __declspec(align(16))
#else
#define XASH_ALIGN(N) __attribute__((aligned (n)))
#endif
#endif

#ifdef XASH_SIMD
#include <xmmintrin.h>
#define XASH_VECTOR_ALIGN XASH_ALIGN(16)
#else
#define XASH_VECTOR_ALIGN
#endif

typedef unsigned char byte;
typedef int		sound_t;
typedef float		vec_t;
typedef vec_t		vec2_t[2];
typedef vec_t       *vec2_t_ref;

#ifndef vec3_t
#ifdef XASH_SIMD
typedef __m128		vec3_t;
typedef __m128		*vec3_t_ref;
#else
typedef vec_t		vec3_t[3];
typedef vec_t		*vec3_t_ref;
#endif
#endif

typedef vec_t		vec4_t[4];
typedef vec_t		*vec4_t_ref;
typedef vec_t		quat_t[4];
typedef byte		rgba_t[4];	// unsigned byte colorpack
typedef byte		rgb_t[3];		// unsigned byte colorpack
typedef vec_t		matrix3x4[3][4];
typedef vec_t		matrix4x4[4][4];

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
typedef Uint64 integer64;
#else
typedef unsigned long long integer64;
#endif
typedef integer64 longtime_t;
#endif // XASH_TYPES_H
