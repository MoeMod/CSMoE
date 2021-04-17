#pragma once

enum : size_t
{
	// up / down
			PITCH = 0,
	// left / right
			YAW = 1,
	// fall over
			ROLL = 2
};

constexpr float anglemod( float a )
{
	return ( 360.0f / 65536 ) * ( (int)( a * ( 65536 / 360.0f ) ) & 65535 );
}

#ifdef M_PI
#undef M_PI
#endif
constexpr auto M_PI = 3.14159265358979323846;	// matches value in gcc v2 math.h
constexpr float RAD2DEG(float x) { return (x) * static_cast<float>(180.f / M_PI); }
constexpr float DEG2RAD(float x) { return (x) * static_cast<float>(M_PI / 180.f); }