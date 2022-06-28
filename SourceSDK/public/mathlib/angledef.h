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

float anglemod(float a);
//constexpr float anglemod( float a )
//{
//	return ( 360.0f / 65536 ) * ( (int)( a * ( 65536 / 360.0f ) ) & 65535 );
//}

#ifdef M_PI
#undef M_PI
#endif
#ifdef RAD2DEG
#undef RAD2DEG
#endif
#ifdef DEG2RAD
#undef DEG2RAD
#endif
constexpr auto M_PI = 3.14159265358979323846;	// matches value in gcc v2 math.h
constexpr float M_PI_F = (float)M_PI;	// matches value in gcc v2 math.h
constexpr auto M_PI2 = 6.28318530717958647692;	// matches value in gcc v2 math.h
constexpr float M_PI2_F = (float)M_PI2;	// matches value in gcc v2 math.h
constexpr float RAD2DEG(float x) { return (x) * static_cast<float>(180.f / M_PI); }
constexpr float DEG2RAD(float x) { return (x) * static_cast<float>(M_PI / 180.f); }