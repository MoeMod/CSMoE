//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
#pragma once
#if !defined( IN_DEFSH )
#define IN_DEFSH

enum : size_t
{
	// up / down
	PITCH = 0,
	// left / right
	YAW = 1,
	// fall over
	ROLL = 2
};

#ifdef _WIN32
#include <windows.h>
#else
#ifndef PORT_H
typedef struct point_s{
	int x;
	int y;
} POINT;
#endif
#define GetCursorPos(x)
#define SetCursorPos(x,y)
#endif

#endif
