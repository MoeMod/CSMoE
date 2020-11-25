module;

#include "util/u_vector.hpp"
#include "util/u_time.hpp"

export module xash3d.types;

#ifndef CLIENT_DLL
using sv::moe::EngineClock;
using sv::moe::VectorBase;
#else
using cl::moe::EngineClock;
using cl::moe::VectorBase;
#endif

export namespace xash3d
{
	typedef unsigned char byte;
	typedef int qboolean;
	typedef unsigned char BYTE;
	typedef int BOOL;
	typedef unsigned short word;
	typedef unsigned int uint;
	typedef unsigned int uint32; //!!!

	typedef int EOFFSET;
	typedef int string_t;

	using Vector2D = VectorBase<float, 2>;
	using Vector = VectorBase<float, 3>;

	using vec3_t = Vector;

	using time_point_t = EngineClock::time_point;
	using duration_t = EngineClock::duration;

	constexpr auto FALSE = 0;
	constexpr auto TRUE = !FALSE;
}
