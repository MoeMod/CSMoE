/*
enginecallback.h - inline function define for engfuncs
Copyright (C) 2019 Moemod Hymei

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef ENGINECALLBACK_H
#define ENGINECALLBACK_H
#ifdef _WIN32
#pragma once
#endif

#include "event_flags.h"

// remove some shit from windows headers
#ifdef SERVER_EXECUTE
#undef SERVER_EXECUTE
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

extern enginefuncs_t g_engfuncs;

namespace engfunc {

inline int	GETPLAYERUSERID( edict_t *e ) { return (*g_engfuncs.pfnGetPlayerUserId)(e); }
inline int	PRECACHE_GENERIC( const char *s ) { return (*g_engfuncs.pfnPrecacheGeneric)(s); }
#ifdef CLIENT_DLL
inline int PRECACHE_MODEL(const char *s) { return 0; }
inline int PRECACHE_SOUND(const char *s) { return 0; }
inline void	SET_MODEL( edict_t *e, const char *m ) {  }
#else
inline int	PRECACHE_MODEL( const char *s ) { return (*g_engfuncs.pfnPrecacheModel)(s); }
inline int	PRECACHE_SOUND( const char *s ) { return (*g_engfuncs.pfnPrecacheSound)(s); }
inline void	SET_MODEL( edict_t *e, const char *m ) { return (*g_engfuncs.pfnSetModel)(e, m); }
#endif
inline int	MODEL_INDEX( const char *m ) { return (*g_engfuncs.pfnModelIndex)(m); }
inline int	MODEL_FRAMES( int modelIndex ) { return (*g_engfuncs.pfnModelFrames)(modelIndex); }
inline void	SET_SIZE( edict_t *e, const float *rgflMin, const float *rgflMax ) { return (*g_engfuncs.pfnSetSize)(e, rgflMin, rgflMax); }
inline void	CHANGE_LEVEL( const char *s1, const char *s2 ) { return (*g_engfuncs.pfnChangeLevel)(s1, s2); }
inline void	GET_SPAWN_PARMS( edict_t *ent ) { return (*g_engfuncs.pfnGetSpawnParms)(ent); }
inline void	SAVE_SPAWN_PARMS( edict_t *ent ) { return (*g_engfuncs.pfnSaveSpawnParms)(ent); }
inline float	VEC_TO_YAW( const float *rgflVector ) { return (*g_engfuncs.pfnVecToYaw)(rgflVector); }
inline void	VEC_TO_ANGLES( const float *rgflVectorIn, float *rgflVectorOut ) { return (*g_engfuncs.pfnVecToAngles)(rgflVectorIn, rgflVectorOut); }
inline void	MOVE_TO_ORIGIN( edict_t *ent, const float *pflGoal, float dist, int iMoveType ) { return (*g_engfuncs.pfnMoveToOrigin)(ent, pflGoal, dist, iMoveType); }
inline void	oldCHANGE_YAW( edict_t* ent ) { return (*g_engfuncs.pfnChangeYaw)(ent); }
inline void	CHANGE_PITCH( edict_t* ent ) { return (*g_engfuncs.pfnChangePitch)(ent); }
inline void	MAKE_VECTORS( const float *rgflVector ) { return (*g_engfuncs.pfnMakeVectors)(rgflVector); }
inline edict_t*	CREATE_ENTITY( void ) { return (*g_engfuncs.pfnCreateEntity)(); }
//inline void	REMOVE_ENTITY( edict_t* e ) { return (*g_engfuncs.pfnRemoveEntity)(e); }
//inline edict_t*	CREATE_NAMED_ENTITY( int className ) { return (*g_engfuncs.pfnCreateNamedEntity)(className); }
inline void	MAKE_STATIC( edict_t *ent ) { return (*g_engfuncs.pfnMakeStatic)(ent); }
inline int	ENT_IS_ON_FLOOR( edict_t *e ) { return (*g_engfuncs.pfnEntIsOnFloor)(e); }
inline int	DROP_TO_FLOOR( edict_t* e ) { return (*g_engfuncs.pfnDropToFloor)(e); }
inline int	WALK_MOVE( edict_t *ent, float yaw, float dist, int iMode ) { return (*g_engfuncs.pfnWalkMove)(ent, yaw, dist, iMode); }
inline void	SET_ORIGIN( edict_t *e, const float *rgflOrigin ) { return (*g_engfuncs.pfnSetOrigin)(e, rgflOrigin); }
inline void	EMIT_SOUND_DYN2( edict_t *entity, int channel, const char *sample, /*int*/float volume, float attenuation, int fFlags, int pitch ) { return (*g_engfuncs.pfnEmitSound)(entity, channel, sample, volume, attenuation, fFlags, pitch); }
inline void	BUILD_SOUND_MSG( edict_t *entity, int channel, const char *sample, /*int*/float volume, float attenuation, int fFlags, int pitch, int msg_dest, int msg_type, const float *pOrigin, edict_t *ed ) { return (*g_engfuncs.pfnBuildSoundMsg)(entity, channel, sample, volume, attenuation, fFlags, pitch, msg_dest, msg_type, pOrigin, ed); }
inline void	TRACE_LINE( const float *v1, const float *v2, int fNoMonsters, edict_t *pentToSkip, TraceResult *ptr ) { return (*g_engfuncs.pfnTraceLine)(v1, v2, fNoMonsters, pentToSkip, ptr); }
inline void	TRACE_TOSS( edict_t* pent, edict_t* pentToIgnore, TraceResult *ptr ) { return (*g_engfuncs.pfnTraceToss)(pent, pentToIgnore, ptr); }
inline int	TRACE_MONSTER_HULL( edict_t *pEdict, const float *v1, const float *v2, int fNoMonsters, edict_t *pentToSkip, TraceResult *ptr ) { return (*g_engfuncs.pfnTraceMonsterHull)(pEdict, v1, v2, fNoMonsters, pentToSkip, ptr); }
inline void	TRACE_HULL( const float *v1, const float *v2, int fNoMonsters, int hullNumber, edict_t *pentToSkip, TraceResult *ptr ) { return (*g_engfuncs.pfnTraceHull)(v1, v2, fNoMonsters, hullNumber, pentToSkip, ptr); }
inline void	TRACE_MODEL( const float *v1, const float *v2, int hullNumber, edict_t *pent, TraceResult *ptr ) { return (*g_engfuncs.pfnTraceModel)(v1, v2, hullNumber, pent, ptr); }
inline void	GET_AIM_VECTOR( edict_t* ent, float speed, float *rgflReturn ) { return (*g_engfuncs.pfnGetAimVector)(ent, speed, rgflReturn); }
inline void	SERVER_COMMAND( const char *str ) { return (*g_engfuncs.pfnServerCommand)(str); }
inline void	SERVER_EXECUTE( void ) { return (*g_engfuncs.pfnServerExecute)(); }
template<class...Args> void	CLIENT_COMMAND( edict_t* pEdict, const char *szFmt, Args &&...args ) { return (*g_engfuncs.pfnClientCommand)(pEdict, szFmt, std::forward<Args>(args)...); }
inline void	PARTICLE_EFFECT( const float *org, const float *dir, float color, float count ) { return (*g_engfuncs.pfnParticleEffect)(org, dir, color, count); }
inline void	LIGHT_STYLE( int style, const char *val ) { return (*g_engfuncs.pfnLightStyle)(style, val); }
inline int	DECAL_INDEX( const char *name ) { return (*g_engfuncs.pfnDecalIndex)(name); }
inline int	POINT_CONTENTS( const float *rgflVector ) { return (*g_engfuncs.pfnPointContents)(rgflVector); }
inline void	CRC32_INIT( CRC32_t *pulCRC ) { return (*g_engfuncs.pfnCRC32_Init)(pulCRC); }
inline void	CRC32_PROCESS_BUFFER( CRC32_t *pulCRC, void *p, int len ) { return (*g_engfuncs.pfnCRC32_ProcessBuffer)(pulCRC, p, len); }
inline void	CRC32_PROCESS_BYTE( CRC32_t *pulCRC, unsigned char ch ) { return (*g_engfuncs.pfnCRC32_ProcessByte)(pulCRC, ch); }
inline CRC32_t	CRC32_FINAL( CRC32_t pulCRC ) { return (*g_engfuncs.pfnCRC32_Final)(pulCRC); }
inline long	RANDOM_LONG( long lLow, long lHigh ) { return (*g_engfuncs.pfnRandomLong)(lLow, lHigh); }
inline float	RANDOM_FLOAT( float flLow, float flHigh ) { return (*g_engfuncs.pfnRandomFloat)(flLow, flHigh); }
inline void	ADD_SERVER_COMMAND( const char *cmd_name, void (*function) (void) ) { return (*g_engfuncs.pfnAddServerCommand)(cmd_name, function); }
inline qboolean	SET_CLIENT_LISTENING(int iReceiver, int iSender, qboolean bListen) { return (*g_engfuncs.pfnVoice_SetClientListening)(iReceiver, iSender, bListen); }
inline const char *GETPLAYERAUTHID		( edict_t *e ) { return (*g_engfuncs.pfnGetPlayerAuthId)(e); }
inline int	GET_FILE_SIZE( const char *filename ) { return (*g_engfuncs.pfnGetFileSize)(filename); }
inline unsigned int GET_APPROX_WAVE_PLAY_LEN( const char *filepath ) { return (*g_engfuncs.pfnGetApproxWavePlayLen)(filepath); }
inline int	IS_CAREER_MATCH( void ) { return (*g_engfuncs.pfnIsCareerMatch)(); }
inline int	GET_LOCALIZED_STRING_LENGTH( const char *label ) { return (*g_engfuncs.pfnGetLocalizedStringLength)(label); }
inline void	REGISTER_TUTOR_MESSAGE_SHOWN( int mid ) { return (*g_engfuncs.pfnRegisterTutorMessageShown)(mid); }
inline int	GET_TIMES_TUTOR_MESSAGE_SHOWN( int mid ) { return (*g_engfuncs.pfnGetTimesTutorMessageShown)(mid); }
inline int	ENG_CHECK_PARM( char *parm, char **ppnext ) { return (*g_engfuncs.CheckParm)(parm, ppnext); }
inline void MESSAGE_BEGIN(int msg_dest, int msg_type, const float *pOrigin = nullptr, edict_t *ed = nullptr) { (*g_engfuncs.pfnMessageBegin)(msg_dest, msg_type, pOrigin, ed); }
inline void	MESSAGE_END( void ) { return (*g_engfuncs.pfnMessageEnd)(); }
inline void	WRITE_BYTE( int iValue ) { return (*g_engfuncs.pfnWriteByte)(iValue); }
inline void	WRITE_CHAR( int iValue ) { return (*g_engfuncs.pfnWriteChar)(iValue); }
inline void	WRITE_SHORT( int iValue ) { return (*g_engfuncs.pfnWriteShort)(iValue); }
inline void	WRITE_LONG( int iValue ) { return (*g_engfuncs.pfnWriteLong)(iValue); }
inline void	WRITE_ANGLE( float flValue ) { return (*g_engfuncs.pfnWriteAngle)(flValue); }
inline void	WRITE_COORD( float flValue ) { return (*g_engfuncs.pfnWriteCoord)(flValue); }
inline void	WRITE_STRING( const char *sz ) { return (*g_engfuncs.pfnWriteString)(sz); }
inline void	WRITE_ENTITY( int iValue ) { return (*g_engfuncs.pfnWriteEntity)(iValue); }
inline void	CVAR_REGISTER( cvar_t *pCvar ) { return (*g_engfuncs.pfnCVarRegister)(pCvar); }
inline float	CVAR_GET_FLOAT( const char *szVarName ) { return (*g_engfuncs.pfnCVarGetFloat)(szVarName); }
inline const char* CVAR_GET_STRING( const char *szVarName ) { return (*g_engfuncs.pfnCVarGetString)(szVarName); }
inline void	CVAR_SET_FLOAT( const char *szVarName, float flValue ) { return (*g_engfuncs.pfnCVarSetFloat)(szVarName, flValue); }
inline void	CVAR_SET_STRING( const char *szVarName, const char *szValue ) { return (*g_engfuncs.pfnCVarSetString)(szVarName, szValue); }
inline cvar_t	*CVAR_GET_POINTER( const char *szVarName ) { return (*g_engfuncs.pfnCVarGetPointer)(szVarName); }
template<class...Args> void	ALERT( ALERT_TYPE atype, const char *szFmt, Args &&...args ) { return (*g_engfuncs.pfnAlertMessage)(atype, szFmt, std::forward<Args>(args)...); }
template<class...Args> void	ENGINE_FPRINTF( FILE *pfile, const char *szFmt, Args &&...args ) { return (*g_engfuncs.pfnEngineFprintf)(pfile, szFmt, std::forward<Args>(args)...); }
inline void*	ALLOC_PRIVATE( edict_t *pEdict, long cb ) { return (*g_engfuncs.pfnPvAllocEntPrivateData)(pEdict, cb); }
template<class T = void> T *GET_PRIVATE(edict_t *pent) { return pent ? static_cast<T *>(pent->pvPrivateData) : nullptr; }
inline void	FREE_PRIVATE( edict_t *pEdict ) { return (*g_engfuncs.pfnFreeEntPrivateData)(pEdict); }
//inline const char *STRING( int iString ) { return (*g_engfuncs.pfnSzFromIndex)(iString); }
inline int	ALLOC_STRING( const char *szValue ) { return (*g_engfuncs.pfnAllocString)(szValue); }
inline edict_t*	FIND_ENTITY_BY_STRING( edict_t *pEdictStartSearchAfter, const char *pszField, const char *pszValue ) { return (*g_engfuncs.pfnFindEntityByString)(pEdictStartSearchAfter, pszField, pszValue); }
inline int	GETENTITYILLUM( edict_t* pEnt ) { return (*g_engfuncs.pfnGetEntityIllum)(pEnt); }
inline edict_t*	FIND_ENTITY_IN_SPHERE( edict_t *pEdictStartSearchAfter, const float *org, float rad ) { return (*g_engfuncs.pfnFindEntityInSphere)(pEdictStartSearchAfter, org, rad); }
inline edict_t*	FIND_CLIENT_IN_PVS( edict_t *pEdict ) { return (*g_engfuncs.pfnFindClientInPVS)(pEdict); }
inline edict_t*	FIND_ENTITY_IN_PVS( edict_t *pplayer ) { return (*g_engfuncs.pfnEntitiesInPVS)(pplayer); }
inline void	EMIT_AMBIENT_SOUND( edict_t *entity, float *pos, const char *samp, float vol, float attenuation, int fFlags, int pitch ) { return (*g_engfuncs.pfnEmitAmbientSound)(entity, pos, samp, vol, attenuation, fFlags, pitch); }
inline void*	GET_MODEL_PTR( edict_t* pEdict ) { return (*g_engfuncs.pfnGetModelPtr)(pEdict); }
inline int	REG_USER_MSG( const char *pszName, int iSize ) { return (*g_engfuncs.pfnRegUserMsg)(pszName, iSize); }
inline void	GET_BONE_POSITION( const edict_t* pEdict, int iBone, float *rgflOrigin, float *rgflAngles ) { return (*g_engfuncs.pfnGetBonePosition)(pEdict, iBone, rgflOrigin, rgflAngles); }
inline unsigned long FUNCTION_FROM_NAME( const char *pName ) { return (*g_engfuncs.pfnFunctionFromName)(pName); }
inline const char *NAME_FOR_FUNCTION( unsigned long function ) { return (*g_engfuncs.pfnNameForFunction)(function); }
inline const char *TRACE_TEXTURE( edict_t *pTextureEntity, const float *v1, const float *v2 ) { return (*g_engfuncs.pfnTraceTexture)(pTextureEntity, v1, v2); }
inline void	CLIENT_PRINTF( edict_t* pEdict, PRINT_TYPE ptype, const char *szMsg ) { return (*g_engfuncs.pfnClientPrintf)(pEdict, ptype, szMsg); }
inline void	SERVER_PRINT( const char *szMsg ) { return (*g_engfuncs.pfnServerPrint)(szMsg); }
inline const char *CMD_ARGS( void ) { return (*g_engfuncs.pfnCmd_Args)(); }
inline const char *CMD_ARGV( int argc ) { return (*g_engfuncs.pfnCmd_Argv)(argc); }
inline int	CMD_ARGC( void ) { return (*g_engfuncs.pfnCmd_Argc)(); }
inline void	GET_ATTACHMENT( const edict_t *pEdict, int iAttachment, float *rgflOrigin, float *rgflAngles ) { return (*g_engfuncs.pfnGetAttachment)(pEdict, iAttachment, rgflOrigin, rgflAngles); }
inline void	SET_VIEW( const edict_t *pClient, const edict_t *pViewent ) { return (*g_engfuncs.pfnSetView)(pClient, pViewent); }
inline void	SET_CROSSHAIRANGLE( const edict_t *pClient, float pitch, float yaw ) { return (*g_engfuncs.pfnCrosshairAngle)(pClient, pitch, yaw); }
inline byte*	LOAD_FILE_FOR_ME( const char *filename, int *pLength ) { return (*g_engfuncs.pfnLoadFileForMe)(filename, pLength); }
inline void	FREE_FILE( void *buffer ) { return (*g_engfuncs.pfnFreeFile)(buffer); }
inline void	END_SECTION( const char *pszSectionName ) { return (*g_engfuncs.pfnEndSection)(pszSectionName); }
inline int	COMPARE_FILE_TIME( char *filename1, char *filename2, int *iCompare ) { return (*g_engfuncs.pfnCompareFileTime)(filename1, filename2, iCompare); }
inline void	GET_GAME_DIR( char *szGetGameDir ) { return (*g_engfuncs.pfnGetGameDir)(szGetGameDir); }
inline void	SET_CLIENT_MAXSPEED( const edict_t *pEdict, float fNewMaxspeed ) { return (*g_engfuncs.pfnSetClientMaxspeed)(pEdict, fNewMaxspeed); }
inline void	PLAYER_RUN_MOVE( edict_t *fakeclient, const float *viewangles, float forwardmove, float sidemove, float upmove, unsigned short buttons, byte impulse, byte msec ) { return (*g_engfuncs.pfnRunPlayerMove)(fakeclient, viewangles, forwardmove, sidemove, upmove, buttons, impulse, msec); }
inline int	NUMBER_OF_ENTITIES( void ) { return (*g_engfuncs.pfnNumberOfEntities)(); }
inline int	IS_MAP_VALID( const char *filename ) { return (*g_engfuncs.pfnIsMapValid)(filename); }
inline void	STATIC_DECAL( const float *origin, int decalIndex, int entityIndex, int modelIndex ) { return (*g_engfuncs.pfnStaticDecal)(origin, decalIndex, entityIndex, modelIndex); }
inline int	IS_DEDICATED_SERVER( void ) { return (*g_engfuncs.pfnIsDedicatedServer)(); }
inline unsigned short PRECACHE_EVENT( int type, const char*psz ) { return (*g_engfuncs.pfnPrecacheEvent)(type, psz); }
inline void	PLAYBACK_EVENT_FULL( int flags, const edict_t *pInvoker, unsigned short eventindex, float delay, float *origin, float *angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2 ) { return (*g_engfuncs.pfnPlaybackEvent)(flags, pInvoker, eventindex, delay, origin, angles, fparam1, fparam2, iparam1, iparam2, bparam1, bparam2); }
inline unsigned char *ENGINE_SET_PVS( float *org ) { return (*g_engfuncs.pfnSetFatPVS)(org); }
inline unsigned char *ENGINE_SET_PAS( float *org ) { return (*g_engfuncs.pfnSetFatPAS)(org); }
inline int	ENGINE_CHECK_VISIBILITY( const edict_t *entity, unsigned char *pset ) { return (*g_engfuncs.pfnCheckVisibility)(entity, pset); }
inline void	DELTA_SET	( struct delta_s *pFields, const char *fieldname ) { return (*g_engfuncs.pfnDeltaSetField)(pFields, fieldname); }
inline void	DELTA_UNSET( struct delta_s *pFields, const char *fieldname ) { return (*g_engfuncs.pfnDeltaUnsetField)(pFields, fieldname); }
inline void	DELTA_ADDENCODER( const char *name, void (*conditionalencode)( struct delta_s *pFields, const unsigned char *from, const unsigned char *to ) ) { return (*g_engfuncs.pfnDeltaAddEncoder)(name, conditionalencode); }
inline int	ENGINE_CURRENT_PLAYER( void ) { return (*g_engfuncs.pfnGetCurrentPlayer)(); }
inline int	ENGINE_CANSKIP( const edict_t *player ) { return (*g_engfuncs.pfnCanSkipPlayer)(player); }
inline int	DELTA_FINDFIELD( struct delta_s *pFields, const char *fieldname ) { return (*g_engfuncs.pfnDeltaFindField)(pFields, fieldname); }
inline void	DELTA_SETBYINDEX( struct delta_s *pFields, int fieldNumber ) { return (*g_engfuncs.pfnDeltaSetFieldByIndex)(pFields,fieldNumber); }
inline void	DELTA_UNSETBYINDEX( struct delta_s *pFields, int fieldNumber ) { return (*g_engfuncs.pfnDeltaUnsetFieldByIndex)(pFields, fieldNumber); }
inline const char *ENGINE_GETPHYSINFO( const edict_t *pClient ) { return (*g_engfuncs.pfnGetPhysicsInfoString)(pClient); }
inline void	ENGINE_SETGROUPMASK( int mask, int op ) { return (*g_engfuncs.pfnSetGroupMask)(mask, op); }
inline int	ENGINE_INSTANCE_BASELINE( int classname, struct entity_state_s *baseline ) { return (*g_engfuncs.pfnCreateInstancedBaseline)(classname, baseline); }
inline void	ENGINE_FORCE_UNMODIFIED( FORCE_TYPE type, float *mins, float *maxs, const char *filename ) { return (*g_engfuncs.pfnForceUnmodified)(type, mins, maxs, filename); }
inline void	PLAYER_CNX_STATS( const edict_t *pClient, int *ping, int *packet_loss ) { return (*g_engfuncs.pfnGetPlayerStats)(pClient, ping, packet_loss); }
inline edict_t	*CREATE_FAKE_CLIENT( const char *netname ) { return (*g_engfuncs.pfnCreateFakeClient)(netname); }
inline char*	GET_USERINFO( edict_t *e ) { return (*g_engfuncs.pfnGetInfoKeyBuffer)(e); }
inline char*	GET_INFO_BUFFER( edict_t *e ) { return (*g_engfuncs.pfnGetInfoKeyBuffer)(e); }
inline char*	GET_KEY_VALUE( char *infobuffer, const char *key ) { return (*g_engfuncs.pfnInfoKeyValue)(infobuffer, key); }
inline void	SET_KEY_VALUE( char *infobuffer, const char *key, const char *value ) { return (*g_engfuncs.pfnSetKeyValue)(infobuffer, key, value); }
inline void	SET_CLIENT_KEY_VALUE( int clientIndex, char *infobuffer, const char *key, const char *value ) { return (*g_engfuncs.pfnSetClientKeyValue)(clientIndex, infobuffer, key, value); }
inline void	REMOVE_KEY_VALUE( char *s, const char *key ) { return (*g_engfuncs.pfnInfo_RemoveKey)(s, key); }
inline void	SET_PHYSICS_KEY_VALUE( const edict_t *pClient, const char *key, const char *value ) { return (*g_engfuncs.pfnSetPhysicsKeyValue)(pClient, key, value); }

}

using namespace engfunc;

}


#endif //ENGINECALLBACK_H
