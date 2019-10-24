//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
#pragma once
#if !defined( IN_DEFSH )
#define IN_DEFSH

#include "angledef.h"

#ifdef WINAPI_FAMILY
#if (!WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP))
#define XASH_WINRT
#endif
#endif

#ifdef _WIN32
#include <WinUser.h>
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
