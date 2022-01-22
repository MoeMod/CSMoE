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
#if !defined ( EVENT_APIH )
#define EVENT_APIH
#ifdef _WIN32
#ifndef __MINGW32__
#pragma once
#endif /* not __MINGW32__ */
#endif

#define EVENT_API_VERSION 1

typedef struct event_api_s
{
	int		version;
	void	( *EV_PlaySound ) ( int ent, const vec3_t origin, int channel, const char *sample, float volume, float attenuation, int fFlags, int pitch );
	void	( *EV_StopSound ) ( int ent, int channel, const char *sample );
	int		( *EV_FindModelIndex )( const char *pmodel );
	int		( *EV_IsLocal ) ( int playernum );
	int		( *EV_LocalPlayerDucking ) ( void );
	void	( *EV_LocalPlayerViewheight ) ( vec3_t_ref );
	void	( *EV_LocalPlayerBounds ) ( int hull, vec3_t_ref mins, vec3_t_ref maxs );
	int		( *EV_IndexFromTrace) ( struct pmtrace_s *pTrace );
	struct physent_s *( *EV_GetPhysent ) ( int idx );
	void	( *EV_SetUpPlayerPrediction ) ( int dopred, int bIncludeLocalClient );
	void	( *EV_PushPMStates ) ( void );
	void	( *EV_PopPMStates ) ( void );
	void	( *EV_SetSolidPlayers ) (int playernum);
	void	( *EV_SetTraceHull ) ( int hull );
	void	( *EV_PlayerTrace ) ( const vec3_t start, const vec3_t end, int traceFlags, int ignore_pe, struct pmtrace_s *tr );
	void	( *EV_WeaponAnimation ) ( int sequence, int body );
	unsigned short ( *EV_PrecacheEvent ) ( int type, const char* psz );
	void	( *EV_PlaybackEvent ) ( int flags, const struct edict_s *pInvoker, unsigned short eventindex, float delay, const vec3_t origin, const vec3_t angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2 );
	const char *( *EV_TraceTexture ) ( int ground, const vec3_t vstart, const vec3_t vend );
	void	( *EV_StopAllSounds ) ( int entnum, int entchannel );
	void    ( *EV_KillEvents ) ( int entnum, const char *eventname );

	// Xash3D extension
	unsigned short(*EV_IndexForEvent)(const char *name);
	const char *(*EV_EventForIndex)(unsigned short index);
	void(*EV_PlayerTraceExt)(const vec3_t start, const vec3_t end, int traceFlags, int(*pfnIgnore)(struct physent_s *pe), struct pmtrace_s *tr);
	const char *(*EV_SoundForIndex)(int index);
	struct msurface_s *(*EV_TraceSurface)(int ground, const vec3_t vstart, const vec3_t vend);

	// CSMoE extension
	unsigned short(*EV_GetCurrentEventIndex)(void);
} event_api_t;

#endif
