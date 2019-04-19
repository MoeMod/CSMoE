//========= Copyright ?1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Insert this file into all projects using the memory system
// It will cause that project to use the shader memory allocator
//
// $NoKeywords: $
//=============================================================================//


#if !defined(STEAM) && !defined(NO_MALLOC_OVERRIDE)

#undef PROTECTED_THINGS_ENABLE   // allow use of _vsnprintf

#if defined(_WIN32) && !defined(_XBOX)
#define WIN_32_LEAN_AND_MEAN
#include <windows.h>
#endif



#include "tier0/dbg.h"
#include "tier0/memalloc.h"
#include <string.h>
#include <stdio.h>
#include "tier0/memdbgoff.h"

// Tags this DLL as debug
#if _DEBUG
DLL_EXPORT void BuiltDebug() {}
#endif

#if defined(_WIN32) && !defined(_STATIC_LINKED)

// Most files include this file, so when it's used it adds an extra .ValveDbg section,
// to help identify debug binaries.
#ifdef _WIN32
	#ifndef NDEBUG // _DEBUG
		#pragma data_seg("ValveDBG") 
		volatile const char* DBG = "*** DEBUG STUB ***";                     
	#endif
#endif

#endif

#endif // !STEAM && !NO_MALLOC_OVERRIDE
