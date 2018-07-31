#include "common.h"

#include "extdll.h"
#include "util.h"
#include "cbase.h"

//! Holds engine functionality callbacks
enginefuncs_t g_engfuncs;
globalvars_t *gpGlobals;
/*
#if defined (_MSC_VER) && (_MSC_VER > 1000)
	#pragma comment (linker, "/EXPORT:GiveFnptrsToDll=_GiveFnptrsToDll@8,@1")
	#pragma comment (linker, "/SECTION:.data,RW")
#endif
*/
#ifndef _WIN32
#define WINAPI // LINUX SUPPORT
#endif

// Receive engine function table from engine.
// This appears to be the _first_ DLL routine called by the engine, so we
// do some setup operations here.
C_DLLEXPORT void WINAPI GiveFnptrsToDll(enginefuncs_t *pEnginefuncsTable, globalvars_t *pGlobals)
{
	Q_memcpy(&g_engfuncs, pEnginefuncsTable, sizeof(enginefuncs_t));
	gpGlobals = pGlobals;
}
