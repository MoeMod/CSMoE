// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#ifndef __BINKPLUGINH__
#define __BINKPLUGINH__

#include "egttypes.h"

#ifdef BUILDING_FOR_UNREAL_ONLY

// this is for static libs in unreal
#ifdef __cplusplus
#define PLUG_IN_FUNC_DEC( ret ) extern "C" ret 
#define PLUG_IN_FUNC_DEF( ret ) extern "C" ret 
#else
#define PLUG_IN_FUNC_DEC( ret ) extern ret
#define PLUG_IN_FUNC_DEF( ret ) ret
#endif

#endif

#ifndef PLUG_IN_FUNC_DEC

// normal DLLs if we aren't doing static libs
#define PLUG_IN_FUNC_DEC( ret ) RADEXPFUNC ret RADEXPLINK
#define PLUG_IN_FUNC_DEF( ret ) RADDEFFUNC ret RADLINK

#endif

#define BP_STRING_JOIN(arg1, arg2)              BP_STRING_JOIN_DELAY(arg1, arg2)
#define BP_STRING_JOIN_DELAY(arg1, arg2)        BP_STRING_JOIN_IMMEDIATE(arg1, arg2)
#define BP_STRING_JOIN_IMMEDIATE(arg1, arg2)    arg1 ## arg2


enum BINKPLUGINAPI
{
#ifndef BUILDING_FOR_UNREAL_ONLY
  BinkGL = 0,
  BinkD3D9 = 1,
  BinkD3D11 = 2,
  BinkD3D12 = 3,
  BinkMetal = 4,
  BinkVulkan = 5,
  BinkNDA = 6,     // any nda gpu api
#else
  BinkRHI = 0,     // Unreal RHI api
#endif
};

// some platforms need allocators for cpu/gpu memory
typedef void * BinkPluginAlloc_t( UINTa bytes, U32 alignment );
typedef void   BinkPluginFree_t( void * ptr ); 
typedef void * BinkPluginGPUAlloc_t( UINTa bytes, U32 alignment );
typedef void   BinkPluginGPUFree_t( void * ptr, UINTa bytes ); 

typedef struct BINKPLUGININITINFO
{
  // D3D12 = ID3D12Device*
  // Vulkan = VkPhysicalDevice
  // Metal = id< MTLDevice >
  void * queue;

  // Vulkan only = VkPhysicalDevice
  void * physical_device; 
  
  // Vulkan only = formats for the sdr and hdr formats
  U32 sdr_and_hdr_render_target_formats[2];

  // general CPU allocators (can be zero on most platforms)
  BinkPluginAlloc_t * alloc;
  BinkPluginFree_t * free;

  // gpu allocators (only used on some platforms
  BinkPluginGPUAlloc_t * gpu_alloc;
  BinkPluginGPUFree_t * gpu_free;
} BINKPLUGININITINFO;


// turn the plug in system on and off (these functions touch the graphics API)

// device: 
// D3D12 = ID3D12Device*
// Vulkan = VkPhysicalDevice
// Metal = id< MTLDevice >
// Vulkan = logical device
// NVNdevice*
PLUG_IN_FUNC_DEC( S32 ) BinkPluginInit( void * device, BINKPLUGININITINFO * info, U32 graphics_api ); 
PLUG_IN_FUNC_DEC( void ) BinkPluginShutdown( void );                     

// spins through all binks, advancing frames and using gpu 
//   this function will hit the gpu on D3D9 and GPUAssisted (not plain GL and D3D11)
//   try to take less than ms_to_process (gotos will take longer, depending on goto time)
PLUG_IN_FUNC_DEC( void ) BinkPluginProcessBinks( S32 ms_to_process ); 

// spins through all binks, drawing everything, you can tell it what you want to draw
//   if you want to draw the rendertargets at a different moment within the frame
PLUG_IN_FUNC_DEC( void ) BinkPluginDraw( S32 draw_overlays, S32 draw_to_render_textures ); 

// turn on and off IO for all Binks - if the buffer runs *completely* out, we still hit the disc
PLUG_IN_FUNC_DEC( void ) BinkPluginIOPause( S32 IO_on );

// Tell Bink to try to use GPU-assisted mode (once on, always on)
PLUG_IN_FUNC_DEC( void ) BinkPluginTurnOnGPUAssist( void );

// used to specify the sounds to open at playback
enum BINKPLUGINSNDTRACK
{
  BinkSndNone               = 0, // don't open any sound tracks snd_track_start not used
  BinkSndSimple             = 1, // based on filename, OR simply mono or stereo sound in track snd_track_start (default speaker spread)
  BinkSndLanguageOverride   = 2, // mono or stereo sound in track 0, language track at snd_track_start
  BinkSnd51                 = 3, // 6 mono tracks in tracks snd_track_start[0..5]
  BinkSnd51LanguageOverride = 4, // 6 mono tracks in tracks 0..5, center language track at snd_track_start
  BinkSnd71                 = 5, // 8 mono tracks in tracks snd_track_start[0..7]
  BinkSnd71LanguageOverride = 6, // 8 mono tracks in tracks 0..7, center language track at snd_track_start
};

typedef struct BINKPLUGIN BINKPLUGIN;

// used to specify the how the video should be buffered
enum BINKPLUGINBUFFERING
{
  BinkStream                = 0, // stream the movie off the media during playback (caches about 1 second of video)
  BinkPreloadAll            = 1, // loads the whole movie into memory at Open time (will block)
  BinkStreamUntilResident   = 2, // streams the movie into a memory buffer as big as the movie, so it will be preloaded eventually)
};


// open and close a Bink file
PLUG_IN_FUNC_DEC( BINKPLUGIN * ) BinkPluginOpen( char const * name, U32 snd_track_type, S32 snd_track_start, U32 buffering_type, U64 file_byte_offset );
// Open a Bink file with a UTF-16 filename (Warning: Only works on Win32/64 platforms)
PLUG_IN_FUNC_DEC( BINKPLUGIN * ) BinkPluginOpenUTF16( unsigned short const * name, U32 snd_track_type, S32 snd_track_start, U32 buffering_type, U64 file_byte_offset );
// Close a Bink 
PLUG_IN_FUNC_DEC( void ) BinkPluginClose( BINKPLUGIN * bnk );

// function to preload a Bink completely into memory (so it can be played by multiple Binks)
//   To use these function, just call them with a name and file offset, and if it's a Bink,
//   the entire Bink file will be loaded into memory.  If you then call BinkPluginOpen with
//   the same parameters, it will just read out of this memory, instead of re-loading it.
//   64 Binks can be preloaded, and you shouldn't load too big of a file (these are
//   are blocking calls).
PLUG_IN_FUNC_DEC( S32 ) BinkPluginPreload( char const * name, U64 file_byte_offset );
PLUG_IN_FUNC_DEC( void ) BinkPluginUnload( char const * name, U64 file_byte_offset );

PLUG_IN_FUNC_DEC( S32 ) BinkPluginPreloadUTF16( unsigned short const * name, U64 file_byte_offset );
PLUG_IN_FUNC_DEC( void ) BinkPluginUnloadUTF16( unsigned short const * name, U64 file_byte_offset );


// set the path to load the library binaries from
PLUG_IN_FUNC_DEC( void ) BinkPluginSetPath( char const * path );

// error stuff
PLUG_IN_FUNC_DEC( char const * ) BinkPluginError( void );
PLUG_IN_FUNC_DEC( void ) BinkPluginSetError( char const * err );
PLUG_IN_FUNC_DEC( void ) BinkPluginAddError( char const * err );

typedef struct BINKPLUGININFO
{
  U64 BufferSize;
  U64 BufferUsed;
  U32 Width;
  U32 Height;
  U32 Frames;
  U32 FrameNum;
  U32 TotalFrames;
  U32 FrameRate;
  U32 FrameRateDiv;
  U32 LoopsRemaining;
  S32 ReadError;
  S32 TexturesError;
  U32 SndTrackType;          // BINKPLUGINSNDTRACK in use
  U32 NumTracksRequested;    // Num tracks
  U32 NumTracksOpened;
  U32 SoundDropOuts;
  U32 SkippedFrames;
  U32 PlaybackState;         // 0 = playing, 1 = paused, 2 = gotoing, 3 = at end
  F32 ProcessBinksFrameRate; // rate at which ProcessBinks is getting called (over last 32 processes)
  F32 Alpha;
} BINKPLUGININFO;

// get playback info
PLUG_IN_FUNC_DEC( void ) BinkPluginInfo( BINKPLUGIN * bnk, BINKPLUGININFO * info );

// call one of these functions every frame
PLUG_IN_FUNC_DEC( S32 ) BinkPluginScheduleToTexture( BINKPLUGIN * bnk, F32 x0, F32 y0, F32 x1, F32 y1, S32 depth, void * render_target_texture, U32 render_target_width, U32 render_target_height );
PLUG_IN_FUNC_DEC( S32 ) BinkPluginScheduleOverlay( BINKPLUGIN * bnk, F32 x0, F32 y0, F32 x1, F32 y1, S32 depth );
// call when everything is scheduled
PLUG_IN_FUNC_DEC( void ) BinkPluginAllScheduled( void );

// Pause playback
PLUG_IN_FUNC_DEC( void ) BinkPluginPause( BINKPLUGIN * bnk, S32 pause_frame ); // 0 = resume, or framenumber to pause on, or -1 to pause immediately

// start async jumping to frame (may draw same frame for long time until it finishes - can monitor with GetInfo)
//   goto_frame is dest frame (fast if key frame). Use 0 to cancel a previous goto at the current position
//   ms_per_process is how long Bink can spend decompressing in BinkPluginProcessBinks for this goto, use -1 for infinite wait
PLUG_IN_FUNC_DEC( void ) BinkPluginGoto( BINKPLUGIN * bnk, S32 goto_frame, S32 ms_per_process ); 

// set overall volume
PLUG_IN_FUNC_DEC( void ) BinkPluginVolume( BINKPLUGIN * bnk, F32 vol ); // 0 to 1.0

// set speaker volume
//   BinkSndSimple = count must be 2 (l/r)
//   BinkSndLanguageOverride = count must be 3 (l,r)/language
//   BinkSnd51 = count must be 6 (front l/r),center,sub,(rear l/r),
//   BinkSnd51LanguageOverride = 7 (front l/r),center,sub,(rear l/r),language
//   BinkSnd71 = count must be 8 (front l/r),center,sub,(read l/r),(surr l/r)
//   BinkSnd71LanguageOverride = 9 (front l/r),center,sub,(read l/r),(surr l/r), lang
PLUG_IN_FUNC_DEC( void ) BinkPluginSpeakerVolumes( BINKPLUGIN * bnk, F32 * vols, U32 count ); // 0 to 1.0 

// turn on/off video looping, loops == 0, infinite
PLUG_IN_FUNC_DEC( void ) BinkPluginLoop( BINKPLUGIN * bnk, U32 loops );

// sets HDR settings state used by BinkPluginScheduleToTexture and BinkPluginScheduleOverlay.
//   tonemap = 0 (disabled), 1 (enabled), 2 (ST2084 PQ)
//   exposure is a scaling factor that happens before tonemapping (1.0=normal, <1.0 darken, >1.0 brighten)
//   output_luma = scales the tonemapped output to output this value as its maximum. 
//     For HDR displays, set this to the max luma of the display. Typically 1000 nits to 2000 nits.
PLUG_IN_FUNC_DEC( void ) BinkPluginSetHdrSettings( BINKPLUGIN * bnk, U32 tonemap, F32 exposure, S32 output_nits );

// sets Alpha settings state used by BinkPluginScheduleToTexture and BinkPluginScheduleOverlay.
//   alpha_value is just a constant blend value for entire video frame. 1 (default) opaque, 0 fully transparent.
PLUG_IN_FUNC_DEC( void ) BinkPluginSetAlphaSettings( BINKPLUGIN * bnk, F32 alpha_value );

// Only used with vulkan. Indexes into the formats specified on device init. See BINKPLUGINVULKANDEVICE 
PLUG_IN_FUNC_DEC( void ) BinkPluginSetRenderTargetFormat( BINKPLUGIN * bnk, U32 format_idx );

enum BINKPLUGINDRAWFLAGS {
  // Decodes sRGB in the shader when drawing. Use this for example when rendering a 8-bit movie to a HDR texture format.
  BinkDrawDecodeSRGB = 0x80000000,
};

// sets draw_flags state used by BinkPluginScheduleToTexture and BinkPluginScheduleOverlay.
//   draw_flags is a bitmask of different options you can set when drawing. see BINKPLUGINDRAWFLAGS enum
PLUG_IN_FUNC_DEC( void ) BinkPluginSetDrawFlags( BINKPLUGIN * bnk, S32 draw_flags );

// limit speakers to certain number of speakers
PLUG_IN_FUNC_DEC( void ) BinkPluginLimitSpeakers( U32 speaker_count );  // set to 1=mono max, 2=stereo max, 3=2.1 max, 4=4.0 max, 6=5.1 max, 8=7.1 max

// for windows only, use directsound (otherwise uses Xaudio2) call before first open
PLUG_IN_FUNC_DEC( S32 ) BinkPluginWindowsUseXAudioDevice( char const * strstr_device_name );  // name has to match some part of the windows device name "Rift" for example (or 0 for default)
PLUG_IN_FUNC_DEC( S32 ) BinkPluginWindowsUseDirectSound( void );

// for D3D9 windows only (call before and after device reset to reset GPU video textures)
PLUG_IN_FUNC_DEC( void ) BinkPluginWindowsD3D9BeginReset( void );
PLUG_IN_FUNC_DEC( void ) BinkPluginWindowsD3D9EndReset( void );

PLUG_IN_FUNC_DEC( S32 ) BinkPluginGetPlatformInfo( U32 bink_platform_enum, void * output_ptr );

// attach a subtitle file to a bink
PLUG_IN_FUNC_DEC( S32 ) BinkPluginLoadSubtitles( BINKPLUGIN * bink, char const * srt_name );
// attach a subtitle file to a bink, filename is UTF-16 - subtitle file contents are still UTF8 or ascii
PLUG_IN_FUNC_DEC( S32 ) BinkPluginLoadSubtitlesUTF16( BINKPLUGIN * bink, unsigned short const * srt_name );
// get current subtitle
PLUG_IN_FUNC_DEC( char const * ) BinkPluginCurrentSubtitle( BINKPLUGIN * bink, U32 * iterate );


/// Bink Image API ///

typedef struct BINKIMAGE  BINKIMAGE;

enum BINKIMAGEFORMAT {
  BinkSurface32BGRA   =  5,
  BinkSurface32RGBA   =  6,
  BinkSurface565      = 10,
  BinkSurface32ARGB   = 12,
};

// open and close a Bink file as an image
PLUG_IN_FUNC_DEC( BINKIMAGE * ) BinkPluginOpenImage( char const * name, U32 *width, U32 *height, U64 file_byte_offset );
// Open a Bink file as an image with a UTF-16 filename (Warning: Only works on Win32/64 platforms)
PLUG_IN_FUNC_DEC( BINKIMAGE * ) BinkPluginOpenImageUTF16( unsigned short const * name, U32 *width, U32 *height, U64 file_byte_offset );
// Read a Bink image into memory
PLUG_IN_FUNC_DEC( void ) BinkPluginReadImage( BINKIMAGE * bnki, void *dest, S32 destpitch, U32 format_flags );
// Close a Bink image
PLUG_IN_FUNC_DEC( void ) BinkPluginCloseImage( BINKIMAGE * bnki );

typedef struct BINKPLUGINFRAMEINFO
{
  // (Metal), id<MTLCommandBuffer>
  // (GNM), Gnmx::LightweightGfxContext *
  // (NVN), NVNcommandBuffer *
  // (AGC), DrawCommandBuffer *
  void * cmdBuf; 

  // (D3D12), swap buffer ptr. On D3D12, must have the flags D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
  // (Metal), id<MTLTexture>
  // (Vulkan), VkImage
  void * screen_resource; 
  
  // (D3D12), D3D12_RESOURCE_STATES, what the screen resource starts in and is restored back to.
  // (Vulkan), VkImageLayout, what layout the screen starts in and is restored back to.
  U32 screen_resource_state; 
  
  // width and height of the screen resource
  U32 width, height;
  
  // (Vulkan), screen is sdr or hdr (formats set in INITINFO above)
  U32 sdr_or_hdr; // on vulkan, 0=sdr screen, 1=hdr screen
} BINKPLUGINFRAMEINFO;

PLUG_IN_FUNC_DEC( void ) BinkPluginSetPerFrameInfo( BINKPLUGINFRAMEINFO * info );  

// dynamic linking typedefs - you can use DO_BINKPLUGIN_PROCS() to create them and such
#define DO_BINKPLUGIN_BINKONLY_PROCS() \
  ProcessProc(  8, S32,            BinkPluginInit,                  void *device, BINKPLUGININITINFO * info, U32 graphics_api ) \
  ProcessProc(  0, void,           BinkPluginShutdown,              void ) \
  ProcessProc( 24, BINKPLUGIN *,   BinkPluginOpen,                  char const * name, U32 snd_track_type, S32 snd_track_start, U32 buffering_type, U64 file_byte_offset ) \
  ProcessProc( 24, BINKPLUGIN *,   BinkPluginOpenUTF16,             unsigned short const * name, U32 snd_track_type, S32 snd_track_start, U32 buffering_type, U64 file_byte_offset ) \
  ProcessProc(  4, void,           BinkPluginClose,                 BINKPLUGIN * bnk ) \
  ProcessProc(  8, void,           BinkPluginInfo,                  BINKPLUGIN * bnk, BINKPLUGININFO *info ) \
  ProcessProc( 12, void,           BinkPluginGoto,                  BINKPLUGIN * bnk, S32 goto_frame, S32 ms_per_process ) \
  ProcessProc(  4, void,           BinkPluginProcessBinks,          S32 ms_to_process ) \
  ProcessProc(  4, void,           BinkPluginLimitSpeakers,         U32 num_speakers ) \
  ProcessProc(  8, void,           BinkPluginLoop,                  BINKPLUGIN * bnk, U32 loops ) \
  ProcessProc(  8, void,           BinkPluginPause,                 BINKPLUGIN * bnk, S32 pause_frame ) \
  ProcessProc(  8, void,           BinkPluginDraw,                  S32 draw_overlays, S32 draw_to_render_textures ) \
  ProcessProc(  4, void,           BinkPluginSetPerFrameInfo,       BINKPLUGINFRAMEINFO * info ) \
  ProcessProc( 36, S32,            BinkPluginScheduleToTexture,     BINKPLUGIN *bnk, F32 x0, F32 y0, F32 x1, F32 y1, S32 depth, void * render_target_texture, U32 render_target_width, U32 render_target_height ) \
  ProcessProc( 24, S32,            BinkPluginScheduleOverlay,       BINKPLUGIN *bnk, F32 x0, F32 y0, F32 x1, F32 y1, S32 depth ) \
  ProcessProc(  0, void,           BinkPluginAllScheduled,          void ) \
  ProcessProc(  4, void,           BinkPluginIOPause,               S32 IO_on ) \
  ProcessProc(  0, void,           BinkPluginTurnOnGPUAssist,       void ) \
  ProcessProc(  0, char const *,   BinkPluginError,                 void ) \
  ProcessProc(  8, void,           BinkPluginVolume,                BINKPLUGIN * bnk, F32 vol ) \
  ProcessProc( 12, void,           BinkPluginSpeakerVolumes,        BINKPLUGIN * bnk, F32 * vols, U32 count ) \
  ProcessProc( 16, void,           BinkPluginSetHdrSettings,        BINKPLUGIN * bnk, U32 tonemap, F32 exposure, S32 output_nits ) \
  ProcessProc(  8, void,           BinkPluginSetAlphaSettings,      BINKPLUGIN * bnk, F32 alpha_value ) \
  ProcessProc(  8, void,           BinkPluginSetRenderTargetFormat, BINKPLUGIN * bnk, U32 format ) \
  ProcessProc(  4, void,           BinkPluginSetPath,               char const * path ) \
  ProcessProc(  8, S32,            BinkPluginGetPlatformInfo,       U32 bink_platform_enum, void * output_ptr ) \
  ProcessProc(  8, void,           BinkPluginSetDrawFlags,          BINKPLUGIN * bnk, S32 draw_flags ) \
  ProcessProc(  8, S32,            BinkPluginPreload,               char const * name, U64 file_byte_offset ) \
  ProcessProc(  8, void,           BinkPluginUnload,                char const * name, U64 file_byte_offset ) \
  ProcessProc(  8, S32,            BinkPluginPreloadUTF16,          unsigned short const * name, U64 file_byte_offset ) \
  ProcessProc(  8, void,           BinkPluginUnloadUTF16,           unsigned short const * name, U64 file_byte_offset ) \
  ProcessProc(  8, S32,            BinkPluginLoadSubtitles,         BINKPLUGIN * bnk, char const * srt_name ) \
  ProcessProc(  8, S32,            BinkPluginLoadSubtitlesUTF16,    BINKPLUGIN * bnk, unsigned short const * srt_name ) \
  ProcessProc(  8, char const *,   BinkPluginCurrentSubtitle,       BINKPLUGIN * bnk, U32 * iterate ) \

#ifdef BUILDING_FOR_UNREAL_ONLY

#define DO_BINKPLUGIN_PROCS() \
  DO_BINKPLUGIN_BINKONLY_PROCS() \

#else

#define DO_BINKPLUGIN_PROCS() \
  DO_BINKPLUGIN_BINKONLY_PROCS() \
  ProcessProc( 16, BINKIMAGE *,    BinkPluginOpenImage,             char const * name, U32 *width, U32 *height, U64 file_byte_offset ) \
  ProcessProc( 16, BINKIMAGE *,    BinkPluginOpenImageUTF16,        unsigned short const * name, U32 *width, U32 *height, U64 file_byte_offset ) \
  ProcessProc( 16, void,           BinkPluginReadImage,             BINKIMAGE * bnki, void *dest, S32 destpitch, U32 format_flags ) \
  ProcessProc(  4, void,           BinkPluginCloseImage,            BINKIMAGE * bnki ) \

#endif

#define DO_BINKPLUGIN_WIN_PROCS() \
  ProcessProc(  4, S32,            BinkPluginWindowsUseXAudioDevice, char const * strstr_device_name ) \
  ProcessProc(  0, S32,            BinkPluginWindowsUseDirectSound,  void ) \

#define ProcessProc( bytes, ret, name, ...) typedef ret BP_STRING_JOIN( name, Proc )(__VA_ARGS__); extern BP_STRING_JOIN( name, Proc ) * BP_STRING_JOIN( p, name );
  DO_BINKPLUGIN_PROCS()
  DO_BINKPLUGIN_WIN_PROCS()
#undef ProcessProc

#endif
