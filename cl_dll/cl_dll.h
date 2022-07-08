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
//  cl_dll.h
//

// 4-23-98  JOHN
#pragma once
#ifndef CL_DLL_H
#define CL_DLL_H
//
//  This DLL is linked by the client when they first initialize.
// This DLL is responsible for the following tasks:
//		- Loading the HUD graphics upon initialization
//		- Drawing the HUD graphics every frame
//		- Handling the custum HUD-update packets
//
#include "basetypes.h"

typedef int (*pfnUserMsgHook)(const char *pszName, int iSize, void *pbuf);

#include "math.h"

#include "../dlls/vector.h" // dlls/vector.h
namespace cl {
    using namespace moe;
}

#include "../engine/cdll_int.h"
#include "../dlls/cdll_dll.h"

#include "exportdef.h"

#include "render_api.h"
#include "mobility_int.h"
#include "fs_int.h"

typedef struct local_state_s local_state_t;
typedef struct usercmd_s usercmd_t;
typedef struct entity_state_s entity_state_t;
typedef struct clientdata_s clientdata_t;
typedef struct playermove_s playermove_t;
typedef struct cl_entity_s cl_entity_t;
typedef struct mstudioevent_s mstudioevent_t;
typedef struct r_studio_interface_s r_studio_interface_t;
typedef struct engine_studio_api_s engine_studio_api_t;
typedef struct ref_params_s ref_params_t;
typedef struct tempent_s tempent_t;
typedef struct kbutton_s kbutton_t;
typedef struct netadr_s netadr_t;
typedef struct weapon_data_s weapon_data_t;

struct ImGuiContext;

namespace cl {

extern "C"
{
int        DLLEXPORT Initialize( cl_enginefunc_t *pEnginefuncs, int iVersion );
int        DLLEXPORT HUD_VidInit( void );
void       DLLEXPORT HUD_Init( void );
int        DLLEXPORT HUD_Redraw( float flTime, int intermission );
int        DLLEXPORT HUD_UpdateClientData( client_data_t *cdata, float flTime );
void       DLLEXPORT HUD_Reset( void );
void       DLLEXPORT HUD_PlayerMove( playermove_t *ppmove, int server );
void       DLLEXPORT HUD_PlayerMoveInit( playermove_t *ppmove );
char       DLLEXPORT HUD_PlayerMoveTexture( char *name );
int        DLLEXPORT HUD_ConnectionlessPacket( const netadr_t *net_from, const char *args, char *response_buffer, int *response_buffer_size );
int        DLLEXPORT HUD_GetHullBounds( int hullnumber, vec3_t_ref mins, vec3_t_ref maxs );
void       DLLEXPORT HUD_Frame( double time );
void       DLLEXPORT HUD_VoiceStatus( int entindex, qboolean bTalking );
void       DLLEXPORT HUD_DirectorMessage( int iSize, void *pbuf );
int        DLLEXPORT HUD_GetRenderInterface( int version, render_api_t *renderfuncs, render_interface_t *callback );
int        DLLEXPORT HUD_MobilityInterface( mobile_engfuncs_t *mobileapi );
void       DLLEXPORT HUD_PostRunCmd( local_state_t *from, local_state_t *to, usercmd_t *cmd, int runfuncs, double time, unsigned int random_seed );
int        DLLEXPORT HUD_AddEntity( int type, cl_entity_t *ent, const char *modelname );
void       DLLEXPORT HUD_CreateEntities( void );
void       DLLEXPORT HUD_StudioEvent( const mstudioevent_t *event, const cl_entity_t *entity );
void       DLLEXPORT HUD_TxferLocalOverrides( entity_state_t *state, const clientdata_t *client );
void       DLLEXPORT HUD_ProcessPlayerState( entity_state_t *dst, const entity_state_t *src );
void       DLLEXPORT HUD_TxferPredictionData( entity_state_t *ps, const entity_state_t *pps, clientdata_t *pcd, const clientdata_t *ppcd, weapon_data_t *wd, const weapon_data_t *pwd );
void       DLLEXPORT HUD_TempEntUpdate( double frametime, double client_time, double cl_gravity, tempent_t **ppTempEntFree, tempent_t **ppTempEntActive, int ( *Callback_AddVisibleEntity )( cl_entity_t *pEntity ), void ( *Callback_TempEntPlaySound )( tempent_t *pTemp, float damp ) );
void       DLLEXPORT HUD_Shutdown( void );
int        DLLEXPORT HUD_Key_Event( int eventcode, int keynum, const char *pszCurrentBinding );
int        DLLEXPORT HUD_GetStudioModelInterface( int version, r_studio_interface_t **ppinterface, engine_studio_api_t *pstudio );
void       DLLEXPORT HUD_DrawNormalTriangles( void );
void       DLLEXPORT HUD_DrawTransparentTriangles( void );
void       DLLEXPORT CAM_Think( void );
int        DLLEXPORT CL_IsThirdPerson( void );
void       DLLEXPORT CL_CameraOffset( vec3_t_ref ofs );
void       DLLEXPORT CL_CreateMove( float frametime, usercmd_t *cmd, int active );
void       DLLEXPORT IN_ActivateMouse( void );
void       DLLEXPORT IN_DeactivateMouse( void );
void       DLLEXPORT IN_MouseEvent( int mstate );
void       DLLEXPORT IN_Accumulate( void );
void       DLLEXPORT IN_ClearStates( void );
void       DLLEXPORT V_CalcRefdef( ref_params_t *pparams );
void       DLLEXPORT Demo_ReadBuffer( int size, unsigned char *buffer );
cl_entity_t DLLEXPORT *HUD_GetUserEntity( int index );
kbutton_t   DLLEXPORT *KB_Find( const char *name );
int        DLLEXPORT IN_ClientTouchEvent(int type_id, int fingerID, float x, float y, float dx, float dy);
void       DLLEXPORT IN_ClientMoveEvent( float forwardmove, float sidemove );
void       DLLEXPORT IN_ClientLookEvent( float relyaw, float relpitch );
void       DLLEXPORT HUD_OnGUI( ImGuiContext *context );
}

extern cl_enginefunc_t gEngfuncs;
extern render_api_t gRenderAPI;
extern mobile_engfuncs_t gMobileAPI;
extern fs_api_t gFileSystemAPI;
extern int g_iXash; // indicates buildnum
extern int g_iMobileAPIVersion; // indicates version. 0 if no mobile API
}

#endif // CL_DLL_H
