//========= Copyright ?1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: This header, which must be the final include in a .cpp (or .h) file,
// causes all crt methods to use debugging versions of the memory allocators.
// NOTE: Use memdbgoff.h to disable memory debugging.
//
// $NoKeywords: $
//=============================================================================//

// SPECIAL NOTE! This file must *not* use include guards; we need to be able
// to include this potentially multiple times (since we can deactivate debugging
// by including memdbgoff.h)

#if !defined(STEAM) && !defined(NO_MALLOC_OVERRIDE)

// SPECIAL NOTE #2: This must be the final include in a .cpp or .h file!!!

#if defined(_DEBUG) && !defined(USE_MEM_DEBUG)
#define USE_MEM_DEBUG 1
#endif

// If debug build or ndebug and not already included MS custom alloc files, or already included this file
#if (defined(_DEBUG) || !defined(_INC_CRTDBG)) || defined(MEMDBGON_H)

#include "basetypes.h"
#ifdef _WIN32
#include <tchar.h>
#else
#include <wchar.h>
#endif
#include <string.h>
#include <malloc.h>
#include "commonmacros.h"
#include "memalloc.h"

#endif // _INC_CRTDBG

#endif // !STEAM && !NO_MALLOC_OVERRIDE
