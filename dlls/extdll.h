/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
#ifndef EXTDLL_H
#define EXTDLL_H

//
// Global header file for extension DLLs
//

// Allow "DEBUG" in addition to default "_DEBUG"
#ifdef _DEBUG
#define DEBUG 1
#endif

#ifdef _MSC_VER
// Silence certain warnings
#pragma warning(disable : 4244)		// int or float down-conversion
#pragma warning(disable : 4305)		// int or float data truncation
#pragma warning(disable : 4201)		// nameless struct/union
#pragma warning(disable : 4514)		// unreferenced inline function removed
#pragma warning(disable : 4100)		// unreferenced formal parameter
#endif

#include "maintypes.h"
#include "basetypes.h"
#include "common.h"

// Prevent tons of unused windows definitions
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOWINRES
#define NOSERVICE
#define NOMCX
#define NOIME
#include "windows.h"
#else // _WIN32
typedef unsigned char BYTE;
typedef int BOOL;

#ifndef MAX_PATH
#define MAX_PATH PATH_MAX
#endif

#include <limits.h>
#include <stdarg.h>
#include <string.h> // memset 

#define _vsnprintf(a,b,c,d) vsnprintf(a,b,c,d)
#endif //_WIN32

// Misc C-runtime library headers
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "minmax.h"

// Header file containing definition of globalvars_t and entvars_t
									// Vector class
#include "vector.h"
									// Defining it as a (bogus) struct helps enforce type-checking

#include "util/u_time.hpp"

#ifndef CLIENT_DLL
using namespace sv::moe;
#else
using namespace cl::moe;
#endif

#define vec3_t Vector
#define vec3_t_ref Vector &
#define time_point_t EngineClock::time_point
#define duration_t EngineClock::duration

									// Shared engine/DLL constants
#include "const.h"
#include "edict.h"

									// Shared header describing protocol between engine and DLLs
#ifdef DLLEXPORT
#undef DLLEXPORT
#endif
#include "eiface.h"
#undef DLLEXPORT
#include "exportdef.h"

									// Shared header between the client DLL and the game DLLs
#include "cdll_dll.h"
#endif //EXTDLL_H
