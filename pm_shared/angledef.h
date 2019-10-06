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
