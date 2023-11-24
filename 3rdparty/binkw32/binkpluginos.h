// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#include "bink.h"

#include "binkpluginplatform.h"

RADEXPFUNC U32 RADEXPLINK BinkUtilCPUs(void);
RADEXPFUNC S32 RADEXPLINK BinkUtilMutexCreate(void* rr,S32 need_timeout);
RADEXPFUNC void RADEXPLINK BinkUtilMutexDestroy(void* rr);
RADEXPFUNC void RADEXPLINK BinkUtilMutexLock(void* rr);
RADEXPFUNC void RADEXPLINK BinkUtilMutexUnlock(void* rr);
RADEXPFUNC S32 RADEXPLINK BinkUtilMutexLockTimeOut(void* rr, S32 timeout);
RADEXPFUNC U32 RADEXPLINK RADTimerRead( void );

#define DO_PROCS() \
  ProcessProc( 12, HBINK,  BinkOpenWithOptions,        char const *name,BINK_OPEN_OPTIONS const * boo,U32 flags ) \
  ProcessProc(  4, void,   BinkClose,                  HBINK bnk ) \
  ProcessProc(  0, char *, BinkGetError,               void ) \
  ProcessProc(  4, S32,    BinkDoFrame,                HBINK bnk ) \
  ProcessProc( 12, S32,    BinkDoFrameAsyncMulti,      HBINK bnk, U32 * threads, S32 thread_count ) \
  ProcessProc(  8, S32,    BinkDoFrameAsyncWait,       HBINK bnk, S32 us ) \
  ProcessProc(  8, S32,    BinkStartAsyncThread,       S32 thread_num, void const * param ) \
  ProcessProc(  4, S32,    BinkRequestStopAsyncThread, S32 thread_num ) \
  ProcessProc(  4, S32,    BinkWaitStopAsyncThread,    S32 thread_num ) \
  ProcessProc( 12, void,   BinkGoto,                   HBINK bnk,U32 frame,S32 flags ) \
  ProcessProc(  4, void,   BinkNextFrame,              HBINK bnk ) \
  ProcessProc(  8, S32,    BinkPause,                  HBINK bnk,S32 pause ) \
  ProcessProc( 12, U32,    BinkGetKeyFrame,            HBINK bnk,U32 frame,S32 flags ) \
  ProcessProc(  4, S32,    BinkWait,                   HBINK bnk ) \
  ProcessProc(  8, void,   BinkSetWillLoop,            HBINK bnk,S32 onoff ) \
  ProcessProc(  8, S32,    BinkSetSoundSystem,         BINKSNDSYSOPEN open, UINTa param ) \
  ProcessProc( 12, S32,    BinkSetSoundSystem2,        BINKSNDSYSOPEN2 open, UINTa param1, UINTa param2 ) \
  ProcessProc(  8, void,   BinkGetFrameBuffersInfo,    HBINK bink, BINKFRAMEBUFFERS * fbset ) \
  ProcessProc(  8, void,   BinkRegisterFrameBuffers,   HBINK bink, BINKFRAMEBUFFERS * fbset ) \
  ProcessProc(  8, S32,    BinkGetGPUDataBuffersInfo,  HBINK bink, BINKGPUDATABUFFERS * gpu ) \
  ProcessProc(  8, void,   BinkRegisterGPUDataBuffers, HBINK bink, BINKGPUDATABUFFERS * gpu ) \
  ProcessProc(  8, S32,    BinkControlBackgroundIO,    HBINK bnk,U32 control ) \
  ProcessProc( 12, void,   BinkSetVolume,              HBINK bnk, U32 trackid, S32 volume ) \
  ProcessProc( 20, void,   BinkSetSpeakerVolumes,      HBINK bnk, U32 trackid, U32 * indexes, S32 * volumes, U32 total ) \
  ProcessProc(  0, U32,    BinkUtilCPUs,               void ) \
  ProcessProc(  8, S32,    BinkUtilMutexCreate,        void*,S32 need_timeout ) \
  ProcessProc(  4, void,   BinkUtilMutexDestroy,       void* ) \
  ProcessProc(  4, void,   BinkUtilMutexLock,          void* ) \
  ProcessProc(  4, void,   BinkUtilMutexUnlock,        void* ) \
  ProcessProc(  8, S32,    BinkUtilMutexLockTimeOut,   void*,S32 timeout ) \
  ProcessProc(  0, U32,    RADTimerRead,               void ) \
  ProcessProc(  8, void,   BinkSetMemory,              BINKMEMALLOC a, BINKMEMFREE f ) \
  ProcessProc(  4, S32,    BinkShouldSkip,             HBINK bink ) \
  ProcessProc(  8, S32,    BinkSetSoundOnOff,          HBINK bink, S32 onoff ) \
  ProcessProc(  0, void,   BinkFreeGlobals,            void ) \
  ProcessProc( 12, S32,    BinkAllocateFrameBuffers,   HBINK bp, BINKFRAMEBUFFERS * set, U32 minimum_alignment ) \
  ProcessProc( 16, void,   BinkSetOSFileCallbacks,     BINKFILEOPEN o, BINKFILEREAD r, BINKFILESEEK s, BINKFILECLOSE c ) \
  ProcessProc(  4, void,   BinkService,                HBINK bink ) \
  ProcessProc( 28, S32,    BinkCopyToBuffer,           HBINK bink, void* dest, S32 destpitch, U32 destheight, U32 destx, U32 desty, U32 flags  ) \
  ProcessProc(  8, S32,    BinkGetPlatformInfo,        S32 bink_platform_enum, void * output_ptr ) \
  ProcessProc(  8, S32,    BinkLoadSubtitles,          HBINK bink, char const * srt_file ) \
  ProcessProc( 16, char const *, BinkCurrentSubtitle,  HBINK bink, U32 * iterate, U32 * opt_out_start_ms, U32 * opt_out_end_ms ) \


#if defined(__RADWIN__)
  #define DO_WIN_PROCS() \
    ProcessProc(  8, S32,    BinkFindXAudio2WinDevice,   void * wchar_out, char const * strstr_device_name ) \
    ProcessProc(  8, BINKSNDOPEN, BinkOpenXAudio2,       UINTa param1, UINTa param2 ) 
#endif

#if defined(__RADNT__)
  #define DO_DIRECTSOUND_PROC() \
    ProcessProc(  4, BINKSNDOPEN, BinkOpenDirectSound,   UINTa param ) 
#endif

#define DO_CA_PROCS() \
  ProcessProc(  8, BINKSNDOPEN, BinkOpenCoreAudio, UINTa freq, UINTa num_speakers )

#define DO_ANDROID_PROCS() \
  ProcessProc(  8, BINKSNDOPEN, BinkOpenSLES, UINTa freq, UINTa num_speakers )

#define DO_LINUX_PROCS() \
  ProcessProc(  8, BINKSNDOPEN, BinkOpenPulseAudio, UINTa freq, UINTa num_speakers )

#if !defined(BUILDING_FOR_UNREAL_ONLY) && ( (!defined( __RADINSTATICLIB__)) || defined( BINK_PLUGIN_DYNAMIC ) )

#define ProcessProc( bytes, ret, name, ...) typedef ret RADLINK RR_STRING_JOIN( name, Proc )(__VA_ARGS__); extern RR_STRING_JOIN( name, Proc ) * RR_STRING_JOIN( p, name );

S32 dynamically_load_procs( void );
#define plugin_load_funcs() if ( dynamically_load_procs() == 0 ) { BinkPluginAddError( "\nCould not load procs." ); return 0; }
    
#else // statically loaded

#ifdef __cplusplus
#define NAMEDEC "C"
#else
#define NAMEDEC
#endif

#define ProcessProc( bytes, ret, name, ...) typedef ret RADLINK RR_STRING_JOIN( name, Proc )(__VA_ARGS__); extern NAMEDEC RR_STRING_JOIN( name, Proc ) * RR_STRING_JOIN( p, name );

#define plugin_load_funcs()

#endif

  DO_PROCS()
  #if defined(__RADWIN__)
    DO_WIN_PROCS()
  #endif  
  #if defined(__RADNT__)
    DO_DIRECTSOUND_PROC()
  #endif
  #if defined(__RADANDROID__)
    DO_ANDROID_PROCS()
  #endif  
  #if defined(__RADLINUX__)
    DO_LINUX_PROCS()
  #endif  
  #if defined(DO_NDA_PROCS)
    DO_NDA_PROCS()
  #endif
  #if defined(__RADMAC__) || defined(__RADIPHONE__)
    DO_CA_PROCS()
  #endif
#undef ProcessProc

extern char BinkPIPath[512];

void oserr( char const * err ); // show err

RADDEFFUNC void RADLINK ourstrcpy(char * dest, char const * src );
RADDEFFUNC void RADLINK ourmemcpy(void * dest, void const * src, int bytes );
RADDEFFUNC void RADLINK ourmemset(void * dest, char val, int bytes );

// memory functions
void * osmalloc( U64 bytes );
void osfree( void * ptr );
void osmemoryreset( void );

