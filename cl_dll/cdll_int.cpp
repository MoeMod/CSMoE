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
//
//  cdll_int.c
//
// this implementation handles the linking of the engine to the DLL
//

#include "hud.h"
#include "cl_util.h"
#include "netadr.h"

#include <string.h>
//#include "interface.h" // not used here
#include "render_api.h"
#include "mobility_int.h"

#include "entity_state.h"
#include "usercmd.h"
#include "ref_params.h"
#include "cl_entity.h"
#include "cdll_exp.h"
#include "events.h"
#include "StudioModelRenderer.h"
#include "GameStudioModelRenderer.h"
#ifdef XASH_LUASH
#include "luash_cl/lua_cl.h"
#endif

extern "C"
{
#include "pmtrace.h"
#include "pm_shared.h"
}

#ifdef XASH_STATIC_GAMELIB
int FS_GetAPI(fs_api_t * g_api);
#endif

namespace cl {
cl_enginefunc_t gEngfuncs = { };
render_api_t gRenderAPI = { };
mobile_engfuncs_t gMobileAPI = { };
fs_api_t gFileSystemAPI = { };
CHud gHUD;
int g_iXash = 0; // indicates a buildnum
vec3_t g_velocity;
long g_iDamage[MAX_CLIENTS + 1];
long g_iDamageTotal[MAX_CLIENTS + 1];
int g_iMobileAPIVersion = 0;
void InitInput (void);
void IN_Commands( void );
void Input_Shutdown (void);
extern engine_studio_api_t IEngineStudio;
/*
==========================
	Initialize

Called when the DLL is first loaded.
==========================
*/
int DLLEXPORT Initialize( cl_enginefunc_t *pEnginefuncs, int iVersion )
{
	if (iVersion != CLDLL_INTERFACE_VERSION)
		return 0;

	gEngfuncs = *pEnginefuncs;

	g_iXash = (int)CVAR_GET_FLOAT("build");

	Game_HookEvents();

	FS_GetAPI(&gFileSystemAPI);

	return 1;
}


/*
=============
HUD_Shutdown

=============
*/
void DLLEXPORT HUD_Shutdown( void )
{
	gHUD.Shutdown();
	Input_Shutdown();
#ifdef XASH_LUASH
	LuaCL_Shutdown();
#endif
}


/*
================================
HUD_GetHullBounds

  Engine calls this to enumerate player collision hulls, for prediction.  Return 0 if the hullnumber doesn't exist.
================================
*/
int DLLEXPORT HUD_GetHullBounds( int hullnumber, vec3_t_ref mins, vec3_t_ref maxs )
{
	int iret = 0;

	switch ( hullnumber )
	{
	case 0:				// Normal player
		mins = Vector(-16, -16, -36);
		maxs = Vector(16, 16, 36);
		iret = 1;
		break;
	case 1:				// Crouched player
		mins = Vector(-16, -16, -18);
		maxs = Vector(16, 16, 18);
		iret = 1;
		break;
	case 2:				// Point based hull
		mins = Vector(0, 0, 0);
		maxs = Vector(0, 0, 0);
		iret = 1;
		break;
	}

	return iret;
}

/*
================================
HUD_ConnectionlessPacket

 Return 1 if the packet is valid.  Set response_buffer_size if you want to send a response packet.  Incoming, it holds the max
  size of the response_buffer, so you must zero it out if you choose not to respond.
================================
*/
int	DLLEXPORT HUD_ConnectionlessPacket( const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size )
{
	// Parse stuff from args
	// int max_buffer_size = *response_buffer_size;

	// Zero it out since we aren't going to respond.
	// If we wanted to response, we'd write data into response_buffer
	*response_buffer_size = 0;

	// Since we don't listen for anything here, just respond that it's a bogus message
	// If we didn't reject the message, we'd return 1 for success instead.
	return 0;
}

void DLLEXPORT HUD_PlayerMoveInit( struct playermove_s *ppmove )
{
	PM_Init( ppmove );
}

char DLLEXPORT HUD_PlayerMoveTexture( char *name )
{
	return PM_FindTextureType( name );
}

void DLLEXPORT HUD_PlayerMove( struct playermove_s *ppmove, int server )
{
	VectorCopy(ppmove->velocity, g_velocity);

	PM_Move( ppmove, server );
}

/*
==========================
	HUD_VidInit

Called when the game initializes
and whenever the vid_mode is changed
so the HUD can reinitialize itself.
==========================
*/

bool isLoaded = false;

int DLLEXPORT HUD_VidInit( void )
{
	gHUD.VidInit();

	isLoaded = true;

	CStudioModelRenderer::s_pBuffAugViewModel = IEngineStudio.Mod_ForName("models/v_buffaug.mdl", FALSE);
	CStudioModelRenderer::s_pBuffAugSmokeModel = IEngineStudio.Mod_ForName("sprites/ef_buffaugsmoke.spr", FALSE);
	CStudioModelRenderer::s_pBloodHunterViewModel = IEngineStudio.Mod_ForName("models/v_bloodhunter.mdl", FALSE);
	CStudioModelRenderer::s_pMGSMViewModel = IEngineStudio.Mod_ForName("models/v_mgsm.mdl", FALSE);
	CStudioModelRenderer::s_pMGSMLauncherModel = IEngineStudio.Mod_ForName("models/ef_mgsm_launcher.mdl", FALSE);
	CStudioModelRenderer::s_pM1887xmasViewModel = IEngineStudio.Mod_ForName("models/v_m1887xmas.mdl", FALSE);
	CStudioModelRenderer::s_pXmasEmptyModel = IEngineStudio.Mod_ForName("models/xmas_empty.mdl", FALSE);

	//VGui_Startup();

	return 1;
}

/*
==========================
	HUD_Init

Called whenever the client connects
to a server.  Reinitializes all
the hud variables.
==========================
*/

void DLLEXPORT HUD_Init( void )
{
#ifdef XASH_LUASH
	LuaCL_Init();
#endif
	InitInput();
	gHUD.Init();
	//Scheme_Init();
}


/*
==========================
	HUD_Redraw

called every screen frame to
redraw the HUD.
===========================
*/

int DLLEXPORT HUD_Redraw( float time, int intermission )
{
	gHUD.Redraw( time, intermission );

	return 1;
}


/*
==========================
	HUD_UpdateClientData

called every time shared client
dll/engine data gets changed,
and gives the cdll a chance
to modify the data.

returns 1 if anything has been changed, 0 otherwise.
==========================
*/

int DLLEXPORT HUD_UpdateClientData(client_data_t *pcldata, float flTime )
{
	IN_Commands();

	return gHUD.UpdateClientData(pcldata, flTime );
}

/*
==========================
	HUD_Reset

Called at start and end of demos to restore to "non"HUD state.
==========================
*/

void DLLEXPORT HUD_Reset( void )
{
	gHUD.VidInit();
}

/*
=================
HUD_GetRect

VGui stub
=================
*/
void getAbsExtents(int &x0, int &y0, int &x1, int &y1)
{
	x0 = gEngfuncs.GetWindowCenterX() - ScreenWidth / 2;
	y0 = gEngfuncs.GetWindowCenterY() - ScreenHeight / 2;
	x1 = gEngfuncs.GetWindowCenterX() + ScreenWidth / 2;
	y1 = gEngfuncs.GetWindowCenterY() + ScreenHeight / 2;
}

/*
==========================
HUD_Frame

Called by engine every frame that client .dll is loaded
==========================
*/

void DLLEXPORT HUD_Frame( double time )
{
	int extents[4];
	getAbsExtents(extents[0], extents[1], extents[2], extents[3]);
	gEngfuncs.VGui_ViewportPaintBackground(extents);
}


/*
==========================
HUD_VoiceStatus

Called when a player starts or stops talking.
==========================
*/

void DLLEXPORT HUD_VoiceStatus(int entindex, qboolean bTalking)
{
	gHUD.m_Radio.Voice( entindex, bTalking );
}

/*
==========================
HUD_DirectorEvent

Called when a director event message was received
==========================
*/

void DLLEXPORT HUD_DirectorMessage( int iSize, void *pbuf )
{
	 gHUD.m_Spectator.DirectorMessage( iSize, pbuf );
}

/*
==========================
HUD_GetRenderInterface

Called when Xash3D sends render api to us
==========================
*/

int DLLEXPORT HUD_GetRenderInterface( int version, render_api_t *renderfuncs, render_interface_t *callback )
{
	if( version != CL_RENDER_INTERFACE_VERSION )
	{
		return false;
	}

	gRenderAPI = *renderfuncs;

	// we didn't send callbacks to engine, because we don't use it
	// *callback = renderInterface;

	// we have here a Host_Error, so check Xash for version
#ifdef __ANDROID__
	if( g_iXash < 3224 )
	{
		gRenderAPI.Host_Error("Xash3D Android version check failed!\nPlease update your Xash3D Android!\n");
	}
#endif

	return true;
}

/*
========================
HUD_MobilityInterface
========================
*/
int DLLEXPORT HUD_MobilityInterface( mobile_engfuncs_t *mobileapi )
{
	if( mobileapi->version != MOBILITY_API_VERSION )
	{
		gEngfuncs.Con_Printf("Client Error: Mobile API version mismatch. Got: %i, want: %i\n",
			mobileapi->version, MOBILITY_API_VERSION);

#ifdef __ANDROID__
		if( gRenderAPI.Host_Error )
		{
			gRenderAPI.Host_Error("Xash3D Android version check failed!\nPlease update your Xash3D Android!\n");
		}
#endif
		return 1;
	}

	g_iMobileAPIVersion = MOBILITY_API_VERSION;
	gMobileAPI = *mobileapi;

	return 0;
}

/*
========================
HUD_MobilityInterface
========================
*/
void DLLEXPORT CL_OnPrecache(int type, const char* name, int index)
{
#ifdef XASH_LUASH
	LuaCL_OnPrecache((resourcetype_t)type, name, index);
#endif
}

/*
========================
ClientFactory

This function is never called, but it has to exist in order for the engine to load stuff from the client. - Solokiller
Identification to detect whether the Client supports VGUI2.
No need for Xash3D.
========================
*/
extern "C" DLLEXPORT void* ClientFactory()
{
	return nullptr;
}

/*
========================
F

For GoldSrc CS1.6 with VGUI2 to export client funcs.
No need for Xash3D.
========================
*/
#ifndef XASH_STATIC_GAMELIB
extern "C" void DLLEXPORT F(void *pv) {
	cldll_func_t *pcldll_func = reinterpret_cast<cldll_func_t *>(pv);

	static cldll_func_t cldll_func = {
		Initialize,
		HUD_Init,
		HUD_VidInit,
		HUD_Redraw,
		HUD_UpdateClientData,
		HUD_Reset,
		HUD_PlayerMove,
		HUD_PlayerMoveInit,
		HUD_PlayerMoveTexture,
		IN_ActivateMouse,
		IN_DeactivateMouse,
		IN_MouseEvent,
		IN_ClearStates,
		IN_Accumulate,
		CL_CreateMove,
		CL_IsThirdPerson,
		CL_CameraOffset,
		KB_Find,
		CAM_Think,
		V_CalcRefdef,
		HUD_AddEntity,
		HUD_CreateEntities,
		HUD_DrawNormalTriangles,
		HUD_DrawTransparentTriangles,
		HUD_StudioEvent,
		HUD_PostRunCmd,
		HUD_Shutdown,
		HUD_TxferLocalOverrides,
		HUD_ProcessPlayerState,
		HUD_TxferPredictionData,
		Demo_ReadBuffer,
		HUD_ConnectionlessPacket,
		HUD_GetHullBounds,
		HUD_Frame,
		HUD_Key_Event,
		HUD_TempEntUpdate,
		HUD_GetUserEntity,
		HUD_VoiceStatus,
		HUD_DirectorMessage,
		HUD_GetStudioModelInterface,
		nullptr,	// HUD_ChatInputPosition,
		nullptr,	// HUD_GetPlayerTeam
		ClientFactory,	// pfnGetClientFactory
		HUD_GetRenderInterface,	// Xash3D pfnGetRenderInterface
		nullptr,	// Xash3D pfnClipMoveToEntity
		IN_ClientTouchEvent,	// SDL Xash pfnTouchEvent
		nullptr,	// SDL Xash pfnMoveEvent
		nullptr,	// SDL Xash pfnLookEvent
		HUD_OnGUI,	// SDL Xash pfnOnGUI
		CL_OnPrecache, // CSMoE ext
	};

	*pcldll_func = cldll_func;
}

#else

void DLLEXPORT IN_MouseEvent_CL( int mstate );
void DLLEXPORT IN_ActivateMouse_CL( void );
void DLLEXPORT IN_DeactivateMouse_CL( void );
void DLLEXPORT V_CalcRefdef_CL( struct ref_params_s *pparams );
int DLLEXPORT CL_IsThirdPerson_CL( void );

extern "C" void DLLEXPORT F(void *pv) {
	cldll_func_t *pcldll_func = reinterpret_cast<cldll_func_t *>(pv);

	static cldll_func_t cldll_func = {
			Initialize,
			HUD_Init,
			HUD_VidInit,
			HUD_Redraw,
			HUD_UpdateClientData,
			HUD_Reset,
			HUD_PlayerMove,
			HUD_PlayerMoveInit,
			HUD_PlayerMoveTexture,
			IN_ActivateMouse_CL,
			IN_DeactivateMouse_CL,
			IN_MouseEvent_CL,
			IN_ClearStates,
			IN_Accumulate,
			CL_CreateMove,
			CL_IsThirdPerson_CL,
			CL_CameraOffset,
			KB_Find,
			CAM_Think,
			V_CalcRefdef_CL,
			HUD_AddEntity,
			HUD_CreateEntities,
			HUD_DrawNormalTriangles,
			HUD_DrawTransparentTriangles,
			HUD_StudioEvent,
			HUD_PostRunCmd,
			HUD_Shutdown,
			HUD_TxferLocalOverrides,
			HUD_ProcessPlayerState,
			HUD_TxferPredictionData,
			Demo_ReadBuffer,
			HUD_ConnectionlessPacket,
			HUD_GetHullBounds,
			HUD_Frame,
			HUD_Key_Event,
			HUD_TempEntUpdate,
			HUD_GetUserEntity,
			HUD_VoiceStatus,
			HUD_DirectorMessage,
			HUD_GetStudioModelInterface,
			nullptr,	// HUD_ChatInputPosition,
			nullptr,	// HUD_GetPlayerTeam
			ClientFactory,	// pfnGetClientFactory
			HUD_GetRenderInterface,	// Xash3D pfnGetRenderInterface
			nullptr,	// Xash3D pfnClipMoveToEntity
			IN_ClientTouchEvent,	// SDL Xash pfnTouchEvent
			nullptr,	// SDL Xash pfnMoveEvent
			nullptr,	// SDL Xash pfnLookEvent
#ifdef XASH_IMGUI
			HUD_OnGUI,	// SDL Xash pfnOnGUI
#else
            nullptr,
#endif
			CL_OnPrecache, // CSMoE ext
	};

	*pcldll_func = cldll_func;
}

}

using namespace cl;

typedef struct dllexport_s
{
	const char *name;
	void *func;
} dllexport_t;

static dllexport_t switch_client_exports[] = {
	{ "F", (void*)F },
	{ "Initialize", (void*)Initialize },
	{ "HUD_VidInit", (void*)HUD_VidInit },
	{ "HUD_Init", (void*)HUD_Init },
	{ "HUD_Shutdown", (void*)HUD_Shutdown },
	{ "HUD_Redraw", (void*)HUD_Redraw },
	{ "HUD_UpdateClientData", (void*)HUD_UpdateClientData },
	{ "HUD_Reset", (void*)HUD_Reset },
	{ "HUD_PlayerMove", (void*)HUD_PlayerMove },
	{ "HUD_PlayerMoveInit", (void*)HUD_PlayerMoveInit },
	{ "HUD_PlayerMoveTexture", (void*)HUD_PlayerMoveTexture },
	{ "HUD_ConnectionlessPacket", (void*)HUD_ConnectionlessPacket },
	{ "HUD_GetHullBounds", (void*)HUD_GetHullBounds },
	{ "HUD_Frame", (void*)HUD_Frame },
	{ "HUD_PostRunCmd", (void*)HUD_PostRunCmd },
	{ "HUD_Key_Event", (void*)HUD_Key_Event },
	{ "HUD_AddEntity", (void*)HUD_AddEntity },
	{ "HUD_CreateEntities", (void*)HUD_CreateEntities },
	{ "HUD_StudioEvent", (void*)HUD_StudioEvent },
	{ "HUD_TxferLocalOverrides", (void*)HUD_TxferLocalOverrides },
	{ "HUD_ProcessPlayerState", (void*)HUD_ProcessPlayerState },
	{ "HUD_TxferPredictionData", (void*)HUD_TxferPredictionData },
	{ "HUD_TempEntUpdate", (void*)HUD_TempEntUpdate },
	{ "HUD_DrawNormalTriangles", (void*)HUD_DrawNormalTriangles },
	{ "HUD_DrawTransparentTriangles", (void*)HUD_DrawTransparentTriangles },
	{ "HUD_GetUserEntity", (void*)HUD_GetUserEntity },
	{ "Demo_ReadBuffer", (void*)Demo_ReadBuffer },
	{ "CAM_Think", (void*)CAM_Think },
	{ "CL_IsThirdPerson", (void*)CL_IsThirdPerson_CL },
	{ "CL_CameraOffset", (void*)CL_CameraOffset },
	{ "CL_CreateMove", (void*)CL_CreateMove },
	{ "IN_ActivateMouse", (void*)IN_ActivateMouse_CL },
	{ "IN_DeactivateMouse", (void*)IN_DeactivateMouse_CL },
	{ "IN_MouseEvent", (void*)IN_MouseEvent_CL },
	{ "IN_Accumulate", (void*)IN_Accumulate },
	{ "IN_ClearStates", (void*)IN_ClearStates },
	{ "V_CalcRefdef", (void*)V_CalcRefdef_CL },
	{ "KB_Find", (void*)KB_Find },
	{ "HUD_GetStudioModelInterface", (void*)HUD_GetStudioModelInterface },
	{ "HUD_DirectorMessage", (void*)HUD_DirectorMessage },
	{ "HUD_VoiceStatus", (void*)HUD_VoiceStatus },
	{ "IN_ClientMoveEvent", (void*)IN_ClientMoveEvent}, // Xash3D ext
	{ "IN_ClientLookEvent", (void*)IN_ClientLookEvent}, // Xash3D ext
#ifdef XASH_IMGUI
	{ "HUD_OnGUI", (void*)HUD_OnGUI },
#endif
	{ "CL_OnPrecache", (void*)CL_OnPrecache },
	{ NULL, NULL },
};

extern "C" int dll_register( const char *name, dllexport_t *exports );

extern "C" int switch_installdll_client( void )
{
	return dll_register( "client", switch_client_exports );
}
#endif