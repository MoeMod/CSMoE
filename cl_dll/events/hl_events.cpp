/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
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
#include "events.h"

#ifdef XASH_LUASH
#include "luash_cl/ev_lua.h"
#endif

namespace cl {

int g_iRShell, g_iPShell, g_iBlackSmoke, g_iShotgunShell;
cvar_t *cl_gunbubbles, *cl_tracereffect;

#define HOOK_EVENT( x, y ) namespace event::x { \
	void EV_##y( struct event_args_s *args ); \
	} \
	using event::x::EV_##y
#include "eventdef.h"
#undef HOOK_EVENT
	
/*
======================
Game_HookEvents

Associate script file name with callback functions.  Callback's must be extern "C" so
 the engine doesn't get confused about name mangling stuff.  Note that the format is
 always the same.  Of course, a clever mod team could actually embed parameters, behavior
 into the actual .sc files and create a .sc file parser and hook their functionality through
 that.. i.e., a scripting system.

That was what we were going to do, but we ran out of time...oh well.
======================
*/
void Game_HookEvents( void )
{
#define HOOK_EVENT( x, y ) gEngfuncs.pfnHookEvent( "events/" #x ".sc", EV_##y )
#include "eventdef.h"
#undef

#ifdef XASH_LUASH
	LuaCL_HookEvents();
#endif

	cl_gunbubbles = gEngfuncs.pfnRegisterVariable("cl_gunbubbles", "2", FCVAR_ARCHIVE);
	cl_tracereffect = gEngfuncs.pfnRegisterVariable("cl_tracereffect", "0", FCVAR_ARCHIVE);
}

}
