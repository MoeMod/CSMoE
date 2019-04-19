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
#include "vgui_parser.h"

#include "entity_state.h"
#include "usercmd.h"
#include "ref_params.h"
#include "cl_entity.h"
#include "cdll_exp.h"

extern "C"
{
#include "pmtrace.h"
#include "pm_shared.h"
}

cl_enginefunc_t gEngfuncs = { };
render_api_t gRenderAPI = { };
mobile_engfuncs_t gMobileAPI = { };
CHud gHUD;
int g_iXash = 0; // indicates a buildnum
int g_iMobileAPIVersion = 0;

void InitInput (void);
void Game_HookEvents( void );
void IN_Commands( void );
void Input_Shutdown (void);

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
	Localize_Free();
}


/*
================================
HUD_GetHullBounds

  Engine calls this to enumerate player collision hulls, for prediction.  Return 0 if the hullnumber doesn't exist.
================================
*/
int DLLEXPORT HUD_GetHullBounds( int hullnumber, float *mins, float *maxs )
{
	int iret = 0;

	switch ( hullnumber )
	{
	case 0:				// Normal player
		Vector(-16, -16, -36).CopyToArray(mins);
		Vector(16, 16, 36).CopyToArray(maxs);
		iret = 1;
		break;
	case 1:				// Crouched player
		Vector(-16, -16, -18).CopyToArray(mins);
		Vector(16, 16, 18).CopyToArray(maxs);
		iret = 1;
		break;
	case 2:				// Point based hull
		Vector(0, 0, 0).CopyToArray(mins);
		Vector(0, 0, 0).CopyToArray(maxs);
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
F

For GoldSrc CS1.6 with VGUI2 to export client funcs.
No need for Xash3D.
========================
*/
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
		nullptr,	// pfnGetClientFactory
		HUD_GetRenderInterface,	// Xash3D pfnGetRenderInterface
		nullptr,	// Xash3D pfnClipMoveToEntity
		IN_ClientTouchEvent,	// SDL Xash pfnTouchEvent
		IN_ClientMoveEvent,	// SDL Xash pfnMoveEvent
		IN_ClientLookEvent	// SDL Xash pfnLookEvent
	};

	*pcldll_func = cldll_func;
}