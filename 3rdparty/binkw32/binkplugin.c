// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#include "bink.h"

#include "binkplugin.h"
#include <stddef.h> 
#include <string.h> 

#include "binkpluginplatform.h"

#include "binkpluginos.h"
#include "binktextures.h"
#include "binkpluginGPUAPIs.h"

// redefine for telemetry if necessary
#define tmenter(str) 
#define tmleave()

#if 0
  #ifdef __RADWIN__
    #include <windows.h>
    #define API_LOG(...)  { char buf[512]; wsprintf(buf,__VA_ARGS__ ); OutputDebugString( buf ); }
  #else
    #include "stdio.h"
    #define API_LOG(...)  printf(__VA_ARGS__ )
  #endif
#else
  #define API_LOG(...) 
#endif

enum STATE
{
  ALLOC_TEXTURES = 0,
  NO_TEXTURES = 1,  // textures couldn't be allocated
  IDLE = 2,
  LOCKED = 3,
  DECOMPRESSING = 4,
  DO_NEXT = 5,      
  PAUSED = 6,
  IN_RESET = 7, // dx9 only
  AT_END = 8,
};

#define TRIPLE_BUFFERING 3
#define MAX_OVERLAYS_PER_FRAME 16
#define MAX_OVERLAY_DRAWS (TRIPLE_BUFFERING*MAX_OVERLAYS_PER_FRAME)  // triple buffering and 16 draws each

typedef struct DRAWINFO
{
  U64 frame;
  void * to_texture;

  F32 Ax,Ay,Bx,By,Cx,Cy;
  S32 depth;
  U32 draw_flags;

  U32 to_texture_width;
  U32 to_texture_height;
  U32 to_texture_format;
} DRAWINFO;

typedef struct radMutex
{
  U8 data[ 192 ];
} radMutex;

struct BINKPLUGIN
{
  radMutex mutex;  
  HBINK b;
  BINKTEXTURES * textures;
  struct BINKPLUGIN * next;
  U32 state;
  U32 loops;
  S32 closing;  
  U32 skips;
  S32 paused_frame;
  U32 num_tracks;
  S32 tracks[ 9 ];
  S32 finished_one;
  U32 first_goto_frame;
  U32 goto_frame;
  S32 one_more_frame;
  U32 goto_time;
  U32 snd_track_type;
  F32 alpha;
  S32 draw_flags;
  U32 format_idx;
 
  DRAWINFO draws[MAX_OVERLAY_DRAWS]; 
};

static S32 ginit = 0;
static S32 gsuccessful_init = 0;

static S32 cinit = 0;
static S32 csuccessful_init = 0;

#ifndef NO_THREADS
static S32 cdo_async = 1;
static S32 async_core_start = 0;
static U32 num_cpus = 0;
static U32 threads[8];
static S32 thread_count;
#endif

static radMutex listlock;

static BINKPLUGIN * all = 0;
static char pierr[256];
char BinkPIPath[512];
static void * gdevice;
static BINKPLUGININITINFO ginfo;
void * gcontext;
#define NOGRAPHICSAPI 32
static U32 gapitype = NOGRAPHICSAPI; //
static S32 use_gpu = 0;
static S32 check_gpu = 0;
static S32 currently_reset = 0;
static S32 bottom_up_render_targets = 0;

#ifdef BINK_NDA_SPEAKER_DEFAULT
static U32 limit_speakers_to = BINK_NDA_SPEAKER_DEFAULT;
#else
static U32 limit_speakers_to = 2;
#endif

#define PB_WINDOW 32
static U32 processbinks_cnt;
static U32 processbinks_pos;
static U32 processbinks_times[PB_WINDOW];

static U64 drawvideo_frame = 1; // 0 is special, start after it

// only used on d3d12,metal,vulkan,ndas right now
BINKPLUGINFRAMEINFO screen_data; 

#if defined(__RADWIN__)
static S32 using_xaudio = 0;
#endif

#ifdef _MSC_VER
#define BP_TRAP() __debugbreak()
#else
#define BP_TRAP() __builtin_trap()
#endif

BINKPLUGINGPUAPIFUNCS gpuapi[] =
{
#ifndef BUILDING_FOR_UNREAL_ONLY
  BinkGLAPI,
  BinkD3D9API,
  BinkD3D11API,
  BinkD3D12API,
  BinkMetalAPI,
  BinkVulkanAPI,
  BinkNDAAPI,
#else
  BinkRHIAPI,
#endif
};

//========================================================

static void alloc_list_lock( void )
{
  pBinkUtilMutexCreate( &listlock, 0 );
}

static void free_list_lock( void )
{
  if ( pBinkUtilMutexDestroy )
    pBinkUtilMutexDestroy( &listlock );
}

static void lock_list( void )
{
  pBinkUtilMutexLock( &listlock );
}

static void unlock_list( void )
{
  pBinkUtilMutexUnlock( &listlock );
}

static void * RADLINK ourmalloc( U64 bytes )
{
  return osmalloc( bytes );
}

static void RADLINK ourfree( void * ptr )
{
  osfree( ptr );
}

void RADLINK ourstrcpy(char * dest, char const * src )
{
  for(;;)
  {
    char c = *src++;
    *dest++=c;
    if (c==0) break;
  }
}

// only small memcpys
void RADLINK ourmemcpy(void * dest, void const * src, int bytes )
{
  unsigned char *d=(unsigned char*)dest;
  unsigned char *s=(unsigned char*)src;
  while(bytes)
  {
    *d++=*s++;
    --bytes;
  }
}

// only small memsets
void RADLINK ourmemset(void * dest, char val, int bytes )
{
  unsigned char *d=(unsigned char*)dest;
  while(bytes)
  {
    *d++=val;
    --bytes;
  }
}

PLUG_IN_FUNC_DEF( void ) BinkPluginSetError( char const * err )
{
  API_LOG("BinkPluginSetError(%s)\n", err);
  oserr( err );
  ourstrcpy( pierr, err );
}

PLUG_IN_FUNC_DEF( void ) BinkPluginSetPath( char const * path )
{
  API_LOG("BinkPluginSetPath(%s)\n", path);
  ourstrcpy( BinkPIPath, path );
}

PLUG_IN_FUNC_DEF( void ) BinkPluginAddError( char const * err )
{
  char * pe;
  API_LOG("BinkPluginAddError(%s)\n", err);
  oserr( err );
  pe = pierr;
  while( pe[0] ) ++pe;
  ourstrcpy( pe, err );
}

static S32 init_graphics( void )
{
  void * context;

  context = 0;
  if ( !gpuapi[gapitype].setup( gdevice, &ginfo, use_gpu, &context ) )
  {
    BinkPluginSetError( "Could not create shaders." );
    return 0;
  }

  if ( context )
    gcontext = context;
  return 1;
}  
#if defined(__RADWIN__) || defined(__RADNT__) || defined(USE_UTF16)

#ifndef USE_UTF16
#define USE_UTF16
#endif

S32 RADLINK binkdefopen(UINTa * user_data, const char * fn, U64 * opt_out_filesize);
U64 RADLINK binkdefread(UINTa * user_data, void * dest,U64 bytes);
void RADLINK binkdefseek(UINTa * user_data, U64 pos);
void RADLINK binkdefclose(UINTa * user_data);

static int strspace( char const * in )
{
  char const * i = in;
  while(*i++) ;
  return (int)( i - in );  // includes null 
}


static void str8to16(unsigned short * out, char const * in )
{
  while(*in) *out++ = *in++;
  *out = 0;
}

#define chartype unsigned short

#else

#define chartype char

#endif


static S32 plugin_init()
{
  S32 i,j;

  if ( cinit )
  {
    //set err to already init
    return csuccessful_init;
  }
  cinit = 1;
  csuccessful_init = 0;

  API_LOG("plugin_init();\n");

  plugin_load_funcs();

  alloc_list_lock();

  pBinkSetMemory( ourmalloc, ourfree );

#if defined(USE_UTF16)
  pBinkSetOSFileCallbacks( binkdefopen, binkdefread, binkdefseek, binkdefclose );
#endif

#ifndef NO_THREADS
  // create decompression threads
  num_cpus = pBinkUtilCPUs();
  thread_count = ( num_cpus > 4 ) ? 4 : num_cpus;

  j = 0;
  for( i = async_core_start ; i < thread_count+async_core_start ; i++ )
  {
    if ( pBinkStartAsyncThread( i, 0 ) )
      threads[ j++ ] = i;
  }
  thread_count = j;

  if ( j == 0 )
  {
    BinkPluginSetError( pBinkGetError() );
    return 0;
  }
#endif

#if defined(__RADWIN__)  // windows, winrt, etc
  if (pBinkSetSoundSystem2(pBinkOpenXAudio2,0,0) )
  {
    using_xaudio = 1;
  }
  else
  {
#if defined(__RADNT__) && !defined(BUILDING_FOR_UNREAL_ONLY)
    BinkPluginSetError("XAudio failed, switching to DirectSound.");
    if ( pBinkSetSoundSystem(pBinkOpenDirectSound,0) == 0 )
      BinkPluginSetError("XAudio and DirectSound both failed.");
#endif
  }
#endif

#if defined(__RADANDROID__)
  pBinkSetSoundSystem2(pBinkOpenSLES,48000,limit_speakers_to);
#endif

#if defined(__RADLINUX__) 
  pBinkSetSoundSystem2(pBinkOpenPulseAudio,48000,limit_speakers_to);
#endif

#if defined(__RADMAC__) || defined(__RADIPHONE__)
  pBinkSetSoundSystem2(pBinkOpenCoreAudio,48000,limit_speakers_to);
#endif

#ifdef BINK_NDA_SOUND
  BINK_NDA_SOUND(limit_speakers_to); 
#endif

  csuccessful_init = 1;
  return 1;
}

extern void set_mem_allocators( BINKPLUGININITINFO * dev );

PLUG_IN_FUNC_DEF( S32 ) BinkPluginInit( void * device, BINKPLUGININITINFO * info, U32 graphics_api )
{
  API_LOG("BinkPluginInit(%p, %p%i);\n", device, info, graphics_api);
  if ( ginit )
  {
    //set err to already init
    return gsuccessful_init;
  }

  if ( info )
    ginfo = *info;
  else
    memset(&ginfo,0,sizeof(ginfo));

  set_mem_allocators( &ginfo );

  if ( !plugin_init() )
    return 0;

  ginit = 1;
  gsuccessful_init = 0;

  gapitype = graphics_api;
  gdevice = device;

  if ( init_graphics() == 0 )
    return 0;

  gsuccessful_init = 1;
  return 1;
}


PLUG_IN_FUNC_DEF( void ) BinkPluginTurnOnGPUAssist( void )
{
  API_LOG("BinkPluginTurnOnGPUAssist();\n");
  use_gpu = 1;
  check_gpu = 1;
}

PLUG_IN_FUNC_DEF( void ) BinkPluginShutdown( void )
{
  S32 i;
  API_LOG("BinkPluginShutdown();\n");

  if ( ( cinit == 0 ) && ( ginit == 0 ) )
  {
    //set err to not inited
    return;
  }
 
  // mark as closed
  while( all )
  {
    lock_list();
    {
      BINKPLUGIN * bp;
      bp = all;
      while( bp )
      {
        bp->closing = 1;
        bp = bp->next;
      }
    }
    unlock_list();
    // close
    BinkPluginProcessBinks( -1 );
  }

  if ( all != 0 )
  {
    //set err to plugins still alloced
    return;
  }

  cinit = ginit = 0;

#ifndef NO_THREADS
  // signal them all to stop
  if (pBinkRequestStopAsyncThread)
    for( i = 0 ; i < thread_count ; i++ )
      pBinkRequestStopAsyncThread( threads[i] );

  // now wait
  if (pBinkWaitStopAsyncThread)
    for( i = 0 ; i < thread_count ; i++ )
      pBinkWaitStopAsyncThread( threads[i] );

  thread_count = 0;
#endif

  gpuapi[gapitype].shutdown();
  
  gapitype = NOGRAPHICSAPI;
  use_gpu = 0;

#if defined(__RADWIN__)
  if ( using_xaudio )
  {
    if ( pBinkSetSoundSystem )
      pBinkSetSoundSystem2(pBinkOpenXAudio2,(UINTa)-1,0);
    using_xaudio = 0;
  }
#endif

  if (pBinkFreeGlobals)
    pBinkFreeGlobals();

  free_list_lock();

  osmemoryreset();
}


PLUG_IN_FUNC_DEF( char const * ) BinkPluginError( void )
{
  API_LOG("'%s' = BinkPluginError()\n", pierr);
  return pierr;
}


static void alloc_bink_lock( BINKPLUGIN * bp )
{
  pBinkUtilMutexCreate( &bp->mutex, 1 );
}


static void free_bink_lock( BINKPLUGIN * bp )
{
  pBinkUtilMutexDestroy( &bp->mutex );
}


static void lock_bink( BINKPLUGIN * bp )
{
  pBinkUtilMutexLock( &bp->mutex );
}


static void unlock_bink( BINKPLUGIN * bp )
{
  pBinkUtilMutexUnlock( &bp->mutex );
}


static S32 try_lock_bink( BINKPLUGIN * bp, S32 timeout )
{
  return pBinkUtilMutexLockTimeOut( &bp->mutex, timeout ) ? 1 : 0;
}


static void start_bink_frame( BINKPLUGIN * bp )
{
  tmenter("Start_Bink_texture_update");
  Start_Bink_texture_update( bp->textures );
  tmleave();
  bp->state = LOCKED;
  tmenter("BinkDoFrame");
#ifndef NO_THREADS
  if(cdo_async)
    pBinkDoFrameAsyncMulti( bp->b, threads, thread_count ); 
  else
#endif
    pBinkDoFrame( bp->b ); 
  tmleave();
  bp->state = DECOMPRESSING;
}

static S32 time_left( U32 start, S32 ms, U32 wait_amt )
{
  S32 wait;
  if ( ms <= 0 )
    wait = ms;
  else
  {
    U32 d = pRADTimerRead() - start;
    if ( d >= (U32) ms )
      wait = 0;
    else
      wait = wait_amt;
  }
  return wait;
}

static volatile U32 using_gpu = 0; // always incremented under lock

static void handle_goto( HBINK b, U32 goto_frame )
{
  if ( goto_frame != 0xffffffff ) // make sure not a cancel
  {
    U32 keyframe = pBinkGetKeyFrame( b, goto_frame, BINKGETKEYPREVIOUS );
    if ( ( (U32)goto_frame < b->FrameNum ) || ( keyframe > (U32) ( b->FrameNum + ( ( b->FrameRate + ( ( b->FrameRateDiv + 1 ) / 2 ) ) / b->FrameRateDiv ) ) ) ) // farther than a second in the future? use goto 
    {
      pBinkGoto( b, keyframe, BINKGOTOQUICK );
    }
  }
}

static void process_binks( S32 ms, S32 when_idle_reset )
{
  BINKPLUGIN * bp;  
  U32 start;
  API_LOG("process_binks(%i, %i)\n", ms, when_idle_reset);

  tmenter("process_binks");

  if ( currently_reset )
  {
    tmleave();
    return;
  }

  // if gpu is flipped on, do it
  if ( check_gpu )
  {
    check_gpu = 0;
    init_graphics();
  }

  start = pRADTimerRead();


  lock_list();
  ++using_gpu;
  if ( using_gpu != 1 )
    BP_TRAP();

  // Save state
  tmenter("statepush");
  if ( gpuapi[gapitype].statepush )
    gpuapi[gapitype].statepush();
  tmleave();

  // time the process call rate - only record a new frame if it is 
  //   at least 8 ms since the last one (half 60 Hz rate)
  if ( ( start - processbinks_times[ processbinks_pos ] ) > 8 )
  {
    ++processbinks_cnt;
    processbinks_pos = ( processbinks_pos + 1 ) & ( PB_WINDOW - 1 );
    processbinks_times[ processbinks_pos ] = start;
  }

  bp = all;
  while( bp )
  {
    U32 wait;

    wait = time_left( start, ms, 1 );
    tmenter("try_lock_bink");
    if ( try_lock_bink( bp, wait ) )
    {
      tmleave();
      // if this is closing, shut it down
      if ( bp->closing )
      {
        BINKPLUGIN * par;

        // wait until compress
        if ( bp->state == DECOMPRESSING )
        {
          unlock_list();  // unlock as we may be entering a pause

          // if there is a async going on and it's not finished, close it the next frame
          wait = time_left( start, ms, 1000 );
#ifndef NO_THREADS
          tmenter("BinkDoFrameAsyncWait");
          if (cdo_async && !pBinkDoFrameAsyncWait(bp->b, wait))
            goto cont;
          tmleave();
#endif
          bp->state = LOCKED;

          lock_list();
        }

        // remove bp from the all list (while it is locked)
        if ( all == bp )
        {
          all = bp->next;
          par = 0;
        }
        else
        {
          par = all;
          while ( par )
          {
            if ( par->next == bp )
            {
              par->next = bp->next;
              break;
            }
            par = par->next;
          }
        }

        unlock_list();  // unlock now that it's removed from the list

        // unlock the bink textures
        if ( bp->state == LOCKED )
        {
          tmenter("Finish_Bink_texture_update");
          Finish_Bink_texture_update( bp->textures );
          tmleave();
          bp->finished_one = 1;
          bp->state = IDLE;
        }

        // if we have textures, free them
        if ( ( bp->state != ALLOC_TEXTURES ) && ( bp->state != NO_TEXTURES ) )
        {
          Free_Bink_textures( bp->textures );
          bp->state = NO_TEXTURES;
        }

        // free the bink
        if ( bp->b )
        {
          pBinkClose( bp->b );
          bp->b = 0;
        }

        // unlock the bink, free the mutex, delete the plug-in and advance
        unlock_bink( bp );
        free_bink_lock( bp );
        ourfree( bp );
        lock_list();
        // find the new next bink ptr (rescan if list has changed)
        if ( par == 0 )
        {
          bp = all;
        }
        else
        {
          BINKPLUGIN * f;
          f = all;
          while( f )
          {
            if ( f == par )
              break;
            f = f->next;
          }
          bp = ( f == 0 ) ? 0 : f->next;
        }
      }
      else if ( gsuccessful_init )
      {
         U32 start_bink_time = pRADTimerRead();

        unlock_list();  // unlock list

        // allocate textures, if we haven't
        if ( bp->state == ALLOC_TEXTURES )
        {
          // start with error state
          bp->state = NO_TEXTURES;

          tmenter("createtextures");
          bp->textures = gpuapi[ gapitype ].createtextures( bp->b );
          tmleave();

          if ( bp->textures )
          {
            bp->state = IDLE;
            if ( !when_idle_reset )
              start_bink_frame( bp );
          }
        }

        // see if the decompression has finished, advance frame
        if ( bp->state == DECOMPRESSING )
        {
          wait = time_left( start, ms, 1000 );
          tmenter("BinkDoFrameAsyncWait");
#ifndef NO_THREADS
          if (!cdo_async || pBinkDoFrameAsyncWait(bp->b, wait))
#endif
          {
            bp->state = DO_NEXT;
          }
          tmleave();
        }

        // handle moving to next frame
        if ( bp->state == DO_NEXT )
        {
          tmenter("DO_NEXT");
          if ( bp->first_goto_frame )
          {
            handle_goto( bp->b, bp->goto_frame );
            bp->first_goto_frame = 0;
          }
          else
          {
           do_next:
            // handle looping
            if ( bp->b->FrameNum == bp->b->Frames ) 
            {
              if ( bp->loops > 1 )
              {
                --bp->loops;
              }
              else if ( bp->loops == 1 )
              {
                bp->state = AT_END;
                goto do_unlock;
              }
            }
            
            if ( bp->loops != 1 )
              pBinkSetWillLoop( bp->b, 1 );

            pBinkNextFrame( bp->b );
          }
          tmleave();
          bp->state = LOCKED;
        }

        // if the texture is locked, unlock it
        if ( bp->state == LOCKED )
        {
          bp->state = IDLE;
         do_unlock: 
          tmenter("Finish_Bink_texture_update");
          Finish_Bink_texture_update( bp->textures );
          tmleave();
          bp->finished_one = 1;
        }
 
        if ( ( bp->state == IN_RESET ) && ( !when_idle_reset ) )
        {
          After_Reset_Bink_textures( bp->textures );
          bp->state = IDLE;
        }

        // if we're idle, start the next frame
        if ( ( bp->state == IDLE ) || ( bp->first_goto_frame ) )
        {
          if ( when_idle_reset )
          {
            Before_Reset_Bink_textures( bp->textures );
            bp->state = IN_RESET;
          }
          else
          {
            // check for pause
            if ( bp->paused_frame )
            {
              if ( ( bp->paused_frame < 0 ) || ( (U32)bp->paused_frame == bp->b->FrameNum ) )
              {
                bp->paused_frame = 0;
                bp->state = PAUSED;
                pBinkPause( bp->b, 1 ); 
              }
            }

            if ( bp->first_goto_frame )
            {
              handle_goto( bp->b, bp->goto_frame );
              bp->first_goto_frame = 0;
            }
   
            if ( bp->goto_frame )
            {
              if ( ( bp->goto_frame == bp->b->FrameNum ) || ( bp->goto_frame == 0xffffffff ) ) // if we hit the goto, or the cancel frame numb
              {
                bp->one_more_frame = 0;
                if ( ( bp->goto_frame != bp->b->LastFrameNum ) && ( bp->goto_frame != 0xffffffff ) ) // if we need to do one more, and we aren't cancelling
                  bp->one_more_frame = 1;
                else
                  pBinkSetSoundOnOff( bp->b, 1 );
                bp->goto_frame = 0;
              }
              else
              {
                // keep reading ahead
                start_bink_frame( bp );
#ifndef NO_THREADS
                tmenter("BinkDoFrameAsyncWait");
                if(cdo_async)
                  pBinkDoFrameAsyncWait( bp->b, -1 );
                tmleave();
#endif
                bp->state = DO_NEXT;
                if ( ( pRADTimerRead() - start_bink_time ) <= bp->goto_time )
                  goto do_next;
                goto cont;
              }
            }


            tmenter("BinkWait");
            if ( ( !pBinkWait( bp->b ) ) || ( bp->goto_frame ) || ( bp->one_more_frame ) )
            {
              tmleave();

              tmenter("start_bink_frame");
              start_bink_frame( bp );
              tmleave();

              // do we need to skip a frame?
              if ( pBinkShouldSkip( bp->b ) )
              {
                ++bp->skips;
                // when we skip, we block until we are done, and then re-run the state machine from nextframe
#ifndef NO_THREADS
                tmenter("BinkDoFrameAsyncWait");
                if(cdo_async)
                  pBinkDoFrameAsyncWait( bp->b, -1 );
                tmleave();
#endif
                bp->state = DO_NEXT;
                goto do_next;
              }
              
              if ( bp->one_more_frame )
              {
                bp->one_more_frame = 0;
#ifndef NO_THREADS
                pBinkDoFrameAsyncWait( bp->b, -1 );
#endif
                bp->state = DO_NEXT;
                pBinkSetSoundOnOff( bp->b, 1 );
              }
            }
            else
            {
              tmleave();
            }
          }
        }

        // unlock bink, lock the list and advance
       cont: 
        unlock_bink( bp );
        lock_list();
        bp = bp->next;
      }
    }
    else
    {
      tmleave();
    }
  }
  
  // Reset GPU state
  tmenter("statepop");
  if ( gpuapi[gapitype].statepop )
    gpuapi[gapitype].statepop();
  tmleave();

  --using_gpu;
  if ( using_gpu != 0 )
    BP_TRAP();

  unlock_list();
}


PLUG_IN_FUNC_DEF( void ) BinkPluginProcessBinks( S32 ms_to_process )
{
  API_LOG("BinkPluginProcessBinks(%i)\n", ms_to_process);
  if ( !csuccessful_init ) // don't check gsuccess, because checked in process
    return;
  process_binks( ms_to_process, 0 );
}


static void draw_videos( int draw_to_textures, int draw_overlays )
{
  BINKPLUGIN * bp;  
  U64 cnt;
  S32 i;
  S32 do_clear;
  U32 draw_total;
  U32 draw_cnt = 0;
  S32 begun_drawing=0;

  API_LOG("draw_videos(%i, %i)\n", draw_to_textures, draw_overlays);

  tmenter("draw_videos");

  lock_list();

  // prevent re-entrancy  
  ++using_gpu;
  if ( using_gpu != 1 )
    BP_TRAP();
  
  // find the smallest overlay numbered set to draw
  bp = all;
  cnt = 0;
  while( bp )
  {
    for( i = 0 ; i < MAX_OVERLAY_DRAWS ; i++ )
    {
      lock_bink( bp );
      if ( bp->draws[i].frame )
      {
        if ( ( cnt == 0 ) || ( cnt > bp->draws[i].frame ) )
        {
          if ( ( ( bp->draws[i].to_texture ) && ( draw_to_textures ) ) || 
               ( ( bp->draws[i].to_texture == 0 ) && ( draw_overlays ) ) )
          {
            cnt = bp->draws[i].frame;
          }
        }
      }
      unlock_bink( bp );
    }
    bp = bp->next;
  }

  if ( cnt )
  {
    void * texture = 0;
    U32 rtw=0, rth=0, rtfmt=0, rtdraw_flags=0;

    // now, draw everything of that numbered set
    for(;;)
    {
      // now find the first render texture (or screen) that match our cnt
      //   we need to draw all of the textures of each render_texture/screen at once for speed
      bp = all;
      while( bp )
      {
        lock_bink( bp );
        for( i = 0 ; i < MAX_OVERLAY_DRAWS ; i++ )
        {
          if ( bp->draws[i].frame == cnt )
          {
            if ( ( ( bp->draws[i].to_texture ) && ( draw_to_textures ) ) || 
                 ( ( bp->draws[i].to_texture == 0 ) && ( draw_overlays ) ) )
            {
              texture = bp->draws[i].to_texture;
              rtw =  bp->draws[i].to_texture_width;
              rth =  bp->draws[i].to_texture_height;
              rtfmt = bp->draws[i].to_texture_format;
              rtdraw_flags = bp->draws[i].draw_flags;
              unlock_bink( bp );
              goto found;
            }
          }
        }
        unlock_bink( bp );
        bp = bp->next;
      }

      // if fell out of loop, nothing left to draw at this numbered set
      break;

     found:

      // Count the total number of draws we *will* be doing below
      {
        draw_total = 0;
        bp = all;
        while( bp )
        {
          lock_bink( bp );
          for( i = 0 ; i < MAX_OVERLAY_DRAWS ; i++ )
            if ( bp->draws[i].frame == cnt && bp->draws[i].to_texture == texture )
              ++draw_total;
          unlock_bink( bp );
          bp = bp->next;
        }
      }

      // unlock the list while we create a render target/start rendering
      unlock_list();

      // start the drawing if we haven't done so yet
      if ( !begun_drawing )
      {
        // start drawing if we have anything that we're going to render
        gpuapi[gapitype].begindraw();

        begun_drawing = 1;
      }

      // now start the render target if necessary
      if ( texture )
      {
        do_clear = draw_total == 1 ? 0 : 1;
        ++draw_cnt; // keep track of how many textures have we used

        gpuapi[ gapitype ].selectrendertarget( texture, rtw, rth, do_clear, rtfmt, -1 );
      }
      else
      {
        // we're drawing the overlays, so select the screen
        if ( gpuapi[gapitype].selectscreenrendertarget )
          gpuapi[gapitype].selectscreenrendertarget( screen_data.screen_resource, screen_data.width, screen_data.height, screen_data.sdr_or_hdr, screen_data.screen_resource_state );
      }


  
      lock_list();

      for(;;)
      {
        BINKPLUGIN * low = 0;  
        S32 low_depth = 0x7fffffff;
        S32 low_i = 0;

        // find the lowest depth at this texture
        bp = all;
        while( bp )
        {
          lock_bink( bp );

          for( i = 0 ; i < MAX_OVERLAY_DRAWS ; i++ )
          {
            if ( bp->draws[i].frame == cnt )
            {
              if ( bp->draws[i].depth <= low_depth )
              {
                if ( bp->draws[i].to_texture == texture )
                {
                  // unlock the previous low, if there is one
                  if ( ( low ) && ( low != bp ) )
                    unlock_bink( low );

                  low_depth = bp->draws[i].depth;
                  low = bp;
                  low_i = i;
                }
              }
            }
          }

          // if this is not a low, unlock it
          if ( low != bp )
            unlock_bink( bp );

          bp = bp->next;
        }

        // if we didn't find anything, this rendertarget is done
        if ( low == 0 )
          break;

        // at this point low is locked and so is the list, unlock the list while we draw
        unlock_list();

        // draw if we have at least one frame done
        if ( low->finished_one && !low->closing )
        {
          F32 Ax = low->draws[low_i].Ax;
          F32 Ay = low->draws[low_i].Ay;
          F32 Bx = low->draws[low_i].Bx;
          F32 By = low->draws[low_i].By;
          F32 Cx = low->draws[low_i].Cx;
          F32 Cy = low->draws[low_i].Cy;

          // if drawing to a texture, are we in bottom_up land (usually for unity)
          if ( texture )
          {
#ifndef BUILDING_FOR_UNREAL_ONLY
            if ( gapitype == BinkGL )
            {
              if ( !bottom_up_render_targets )
                goto flip;
            }
            else
#endif
            {
              if ( bottom_up_render_targets ) 
              { 
#ifndef BUILDING_FOR_UNREAL_ONLY
               flip:
#endif
                Ay = 1.0f - Ay; 
                By = 1.0f - By; 
                Cy = 1.0f - Cy; 
              }
            }
          }
          Set_Bink_draw_corners( low->textures, Ax,Ay, Bx,By, Cx,Cy );
          Set_Bink_alpha_settings( low->textures, low->alpha, (draw_total == 1 && texture ? 2 : 0) | rtdraw_flags );
#ifndef BUILDING_FOR_UNREAL_ONLY
          if(gapitype == BinkVulkan) 
          {
            // make a copy so we are not modifying the user's data
            BINKAPICONTEXTVULKAN ctx = *(BINKAPICONTEXTVULKAN*)gcontext;
            ctx.format_idx = rtfmt;
            //todo!
            Draw_Bink_textures( low->textures, 0, &ctx );
          } 
          else 
#endif
          {
            Draw_Bink_textures( low->textures, 0, gcontext );
          }
        }

        // clear the draw trigger and target
        low->draws[ low_i ].frame = 0;
        low->draws[ low_i ].to_texture = 0;

        // unlock the bink we drew, relock the list and loop around to find the next depth
        unlock_bink( low );
        lock_list();
      }

      gpuapi[ gapitype ].clearrendertarget();
    }
  }

  if ( begun_drawing ) 
    gpuapi[gapitype].enddraw();
  
  --using_gpu;
  if ( using_gpu != 0 )
    BP_TRAP();

  unlock_list();

  tmleave();
}


PLUG_IN_FUNC_DEF( void ) BinkPluginDraw( S32 draw_overlays, S32 draw_to_render_textures )
{
  API_LOG("BinkPluginDraw(%i, %i)\n", draw_overlays, draw_to_render_textures);
  if ( ( !csuccessful_init ) || ( !gsuccessful_init ) )
    return;
  draw_videos( draw_overlays, draw_to_render_textures );
}


PLUG_IN_FUNC_DEF( void ) BinkPluginIOPause( S32 IO_on )
{
  BINKPLUGIN * bp;  
  API_LOG("BinkPluginIOPause(%i)\n", IO_on);
  
  if ( !plugin_init() )
    return;

  lock_list();
  bp = all;
  while( bp )
  {
    pBinkControlBackgroundIO( bp->b, ( IO_on ) ? BINKBGIOSUSPEND : BINKBGIORESUME );
    bp = bp->next;
  }
  unlock_list();
}

static U8 mask21[3*2] =
{
  0,0,0,
  0,1,0+(1<<4),
};

static U8 mask51[6*4] = 
{
  0,0,0,0,0,0,
  0,1,0+(1<<4),0+(1<<4)+0,0,1,
  0,1,2,0+(1<<4),0,1,
  0,1,0+(1<<4),0+(1<<4),2,3,
};

static U8 mask71[8*6] = 
{
  0,0,0,0,0,0,0,0,
  0,1,0+(1<<4),0+(1<<4),0,1,0,1,
  0,1,2,0+(1<<4),0,1,0,1,
  0,1,0+(1<<4),0+(1<<4),2,3,2,3,
  0,1,2,3,4,5,4,5,
  0,1,2,3,4,5,4,5,
};

static S32 get_limit_speaker_index( S32 orig_speaker_index, U8 * masks, S32 total_tracks, S32 limit_to )
{
  S32 speaker_index = 0;
  S32 cnt = 0;
  
  if ( ( total_tracks == 3 ) && ( limit_to == 4 ) )   // if 4 out, but handed 3 tracks, use stereo routing
    limit_to = 2;
  
  while( orig_speaker_index )
  {
    S32 si;
    si = orig_speaker_index&15;
    if ( total_tracks > limit_to )
      si = masks[ total_tracks * ( limit_to - 1 ) + si ];
    si<<=cnt;
    cnt+=4;
    orig_speaker_index >>= 4;
    speaker_index |= si;
  }

  return speaker_index;
}

static void set_speakers( BINKPLUGIN * bp, S32 speaker_index, S32 track_index, F32 * track_volumes )
{
  S32 vols[8];
  U32 ind[8];
  S32 cnt = 1;

  vols[0] = (S32)(track_volumes[track_index]*32768.0f);
  
  ind[0] = speaker_index & 15;
  for(;;)
  {
    speaker_index >>= 4;
    if ( speaker_index == 0 )
      break;
    ind[ cnt ] = speaker_index;
    vols[ cnt ] = vols[0];
    ++cnt;
  }

  pBinkSetSpeakerVolumes( bp->b, bp->tracks[track_index], ind, vols, cnt );
}

static void set_volumes( BINKPLUGIN * bp, F32 * trk_vols )
{
  S32 i, si;

  switch ( bp->num_tracks )
  {
    case 2: // one mono or stereo track, and one language track
      // handle the language track, and then fall down into the one track case
      si = get_limit_speaker_index( 2, mask21, 3, limit_speakers_to );
      set_speakers( bp, si, 1, trk_vols );
      // fall through
    case 1:  // one mono or stereo track
      si = get_limit_speaker_index( 0+(1<<4), mask21, 3, limit_speakers_to );
      set_speakers( bp, si, 0, trk_vols );
      break;
    
    case 7:  // 5+1 and a language track
      // handle the language track, and then fall down into the 5.1 case
      si = get_limit_speaker_index( 2, mask51, 6, limit_speakers_to );
      set_speakers( bp, si, 6, trk_vols );
      // fall through
    case 6: // 5+1
      for( i = 0 ; i < 6 ; i++ )
      {
        si = get_limit_speaker_index( i, mask51, 6, limit_speakers_to );
        set_speakers( bp, si, i, trk_vols );
      }
      break;
    case 9: // 7+1 and a language track
      // handle the language track, and then fall down into the 7.1 case
      si = get_limit_speaker_index( 2, mask71, 8, limit_speakers_to );
      set_speakers( bp, si, 8, trk_vols );
      // fall through
    case 8: // 7+1
      for( i = 0 ; i < 8 ; i++ )
      {
        si = get_limit_speaker_index( i, mask71, 8, limit_speakers_to );
        set_speakers( bp, si, i, trk_vols );
      }
      break;
  }
}

static int has_string( chartype const * big, char const * has )
{
  while( has[0] )
  {
    if ( has[0] != big[0] )
      return 0;
    ++has;
    ++big;
  }
  return 1;
}

static int snd_override( chartype const * name, U32 * snd_track_type )
{
  for(;;)
  {
    if ( name[0] == 0 )
      return 0;
    ++name;
    if ( name[-1] == '_' )
    {
      if ( has_string( name, "51." ) )
      {
        *snd_track_type = BinkSnd51;
        return 1;
      }
      if ( has_string( name, "51L." ) )
      {
        *snd_track_type = BinkSnd51LanguageOverride;
        return 1;
      }
      if ( has_string( name, "71." ) )
      {
        *snd_track_type = BinkSnd71;
        return 1;
      }
      if ( has_string( name, "71L." ) )
      {
        *snd_track_type = BinkSnd71LanguageOverride;
        return 1;
      }
    }
  }
}

#define MAXPRELOADS 64

typedef struct BINKPRELOADDATA
{
  U64 offset;
  chartype name[1024-8];
  BINK b;
} BINKPRELOADDATA;

static BINKPRELOADDATA * predata[ MAXPRELOADS ];

static int strequal( chartype const * ab, chartype const * bb )
{
  for(;;)
  {
    if ( ab[0] == 0 )
    {
      if ( bb[0] == 0 )
        return 1;
      else
        return 0;
    }
    if ( ab[0] != bb[0] )
      return 0;
    ++ab;
    ++bb;
  }
}

static void strcopier( chartype * out, chartype const * in )
{
  while(*in) *out++ = *in++;
  *out = 0;
}

static int find_preloaded_name( chartype const *name, U64 file_byte_offset )
{
  int i;
  for( i = 0 ; i < MAXPRELOADS ; i++ )
  {
    if ( predata[ i ] )
    {
      if ( predata[ i ]->offset == file_byte_offset )
        if ( strequal( (chartype*)predata[ i ]->name, name ) )
          return i;
    }
  }
  return -1;
}

static int find_empty_preload_spot()
{
  int i;
  for( i = 0 ; i < MAXPRELOADS ; i++ )
  {
    if ( predata[i] == 0 )
      return i;
  }
  return -1;
}

static int preload_bink( chartype const * name, U64 file_byte_offset )
{
  HBINK lf;
  int i;
  BINK_OPEN_OPTIONS o;

  if ( !plugin_init() )
    return 0;

  i = find_empty_preload_spot();
  if ( i == -1 )
  {
    BinkPluginSetError( "No preload slots left!" );
    return 0;
  }

  o.FileOffset = file_byte_offset;
  o.TotTracks = 0;
  o.TrackNums = 0;
  
  lf = pBinkOpenWithOptions( (void*)name, &o, BINKFILEDATALOADONLY|BINKFILEOFFSET|BINKSNDTRACK|BINKNOTHREADEDIO );
  if ( lf == 0 )
  {
    BinkPluginSetError( pBinkGetError() );
    return 0;
  }

  if ( lf->frameoffsets )
  {
    // old Bink DLL!
    BinkPluginSetError( "Using old Bink SDK library without preload support!" );
    pBinkClose( lf );
    return 0;
  }

  predata[ i ] = (BINKPRELOADDATA*)(((char*)lf)-1024);
  predata[ i ]->offset = file_byte_offset;
  strcopier( predata[ i ]->name, name );  // BINKFILEDATALOADONLY allocates 1024 bytes in front of the bink header for user data

  return 1;
}

static int unload_bink( chartype const * name, U64 file_byte_offset )
{
  int p = find_preloaded_name( name, file_byte_offset );
  if ( p >= 0 )
  {
    pBinkClose( &predata[ p ]->b );
    predata[ p ] = 0;
    return 1;
  }
  return 0;
}

PLUG_IN_FUNC_DEF( S32 ) BinkPluginPreloadUTF16( unsigned short const * name, U64 file_byte_offset )
{
  API_LOG("BinkPluginPreloadUTF16(%p, %llu)\n", name, file_byte_offset);
#if defined(USE_UTF16) 
  return preload_bink( name, file_byte_offset );
#else
  BinkPluginSetError("BinkPluginPreloadUTF16 not supported on this platform.");
  return 0;
#endif
}

#include "rrMem.h"

PLUG_IN_FUNC_DEF( S32 ) BinkPluginPreload( char const * name, U64 file_byte_offset )
{
  API_LOG("BinkPluginPreload(%s, %llu)\n", name, file_byte_offset);
#if defined(USE_UTF16)
{
  chartype *nameW = (chartype*)alloca(strspace(name)*2);
  str8to16(nameW,name);
  return preload_bink( nameW, file_byte_offset );
}
#else
  return preload_bink( name, file_byte_offset );
#endif
}

PLUG_IN_FUNC_DEF( void ) BinkPluginUnloadUTF16( unsigned short const * name, U64 file_byte_offset )
{
  API_LOG("BinkPluginUnloadUTF16(%p, %llu)\n", name, file_byte_offset);
#if defined(USE_UTF16)
  unload_bink( name, file_byte_offset );
#else
  BinkPluginSetError("BinkPluginUnloadUTF16 not supported on this platform.");
#endif
}

PLUG_IN_FUNC_DEF( void ) BinkPluginUnload( char const * name, U64 file_byte_offset )
{
  API_LOG("BinkPluginUnload(%s, %llu)\n", name, file_byte_offset);
#if defined(USE_UTF16)
{
  chartype *nameW = (chartype*)alloca(strspace(name)*2);
  str8to16(nameW,name);
  unload_bink( nameW, file_byte_offset );
}
#else
  unload_bink( name, file_byte_offset );
#endif
}


static BINKPLUGIN *plugin_open( chartype const * name, U32 snd_track_type, S32 snd_track_start, U32 buffering_type, U64 file_byte_offset ) 
{
  BINKPLUGIN * bp;
  BINK_OPEN_OPTIONS o;
  S32 i;
  U32 flags = 0;

  if ( !plugin_init() )
    return 0;

  bp = ourmalloc( sizeof(*bp) );
  if ( bp == 0 )
  {
    BinkPluginSetError( "Out of memory." );
    return 0;
  }
  ourmemset( bp, 0, sizeof(*bp) );
  
  o.FileOffset = file_byte_offset;
 again: 
  switch ( snd_track_type )
  {
    case BinkSndNone:
      bp->tracks[0] = 0;
      bp->num_tracks = 0;
      break;
    case BinkSndSimple:
      if ( snd_override( name, &snd_track_type ) )
        goto again;
      bp->tracks[0] = snd_track_start;
      bp->num_tracks = 1;
      break;
    case BinkSndLanguageOverride:
      bp->tracks[0] = 0;
      bp->tracks[1] = snd_track_start;
      bp->num_tracks = 2;
      break;
    case BinkSnd51:
      for( i = 0 ; i < 6 ; i++ )
        bp->tracks[ i ] = snd_track_start + i;
      bp->num_tracks = 6;
      break;
    case BinkSnd51LanguageOverride:
      for( i = 0 ; i < 6 ; i++ )
        bp->tracks[ i ] = i;
      bp->tracks[ 6 ] = snd_track_start;
      bp->num_tracks = 7;
      break;
    case BinkSnd71:
      for( i = 0 ; i < 8 ; i++ )
        bp->tracks[ i ] = snd_track_start + i;
      bp->num_tracks = 8;
      break;
    case BinkSnd71LanguageOverride:
      for( i = 0 ; i < 8 ; i++ )
        bp->tracks[ i ] = i;
      bp->tracks[ 8 ] = snd_track_start;
      bp->num_tracks = 9;
      break;
    default:
      BinkPluginSetError( "Bad sound track type specified." );
      ourfree( bp );
      return 0;
  }
  o.TotTracks = bp->num_tracks;
  o.TrackNums = bp->tracks;

  switch ( buffering_type )
  {
    case BinkStream:
      break;
    case BinkPreloadAll:
      flags |= BINKPRELOADALL;
      break;
    case BinkStreamUntilResident:
      flags |= BINKIOSIZE;
      o.IOBufferSize = 0x7fffffffffffffffULL;
      break;
  }

#ifndef BUILDING_FOR_UNREAL_ONLY
  if ( ( gapitype == BinkD3D12 ) || ( gapitype == BinkVulkan ) )
    flags |= BINKUSETRIPLEBUFFERING;
#endif

  #ifdef FORCE_TRIPLE_BUFFERING
    flags |= BINKUSETRIPLEBUFFERING;
  #endif


  // see if we have previously preloaded this file
  {
    int p = find_preloaded_name( name, file_byte_offset );
    if ( p >= 0 )
    {
      name = predata[p]->b.preloadptr;
      flags |= BINKFROMMEMORY;
      o.FileOffset = 0;
    }
  }

  bp->b = pBinkOpenWithOptions( (char*)name, &o, BINKALPHA|BINKFILEOFFSET|BINKSNDTRACK|BINKDONTCLOSETHREADS|BINKNOFRAMEBUFFERS|flags );
  if ( bp->b == 0 )
  {
    BinkPluginSetError( pBinkGetError() );
    ourfree( bp );
    return 0;
  }

  // only warn if there are sound tracks at all in the bink
  if ( ( bp->b->NumTracks != (S32)bp->num_tracks ) && ( bp->b->NumTracks ) )
  {
    BinkPluginSetError( "Mismatched number of tracks opened." );
  }

  bp->state = ALLOC_TEXTURES;
  bp->loops = 1;
  bp->alpha = 1;

  {
    F32 vols[9]={1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f};
    set_volumes( bp, vols );
  }

  alloc_bink_lock( bp );

  lock_list();

  // insert at end of list
  bp->next = 0;
  if ( all == 0 )
  {
    all = bp;
  }
  else
  {
    BINKPLUGIN * f;
    f = all;
    for(;;)
    {
      if ( f->next == 0 )
        break;
      f = f->next;
    }
#if defined(_DEBUG) && defined(_MSC_VER)
    if ( ( f==0 ) || ( f->next ) ) __debugbreak(); 
#endif
    f->next = bp;
  }
  unlock_list();

  return bp;
}

PLUG_IN_FUNC_DEF( BINKPLUGIN * ) BinkPluginOpenUTF16( unsigned short const * name, U32 snd_track_type, S32 snd_track_start, U32 buffering_type, U64 file_byte_offset )
{
  API_LOG("BinkPluginOpenUTF16(%p, %i, %i, %i, %llu)\n", name, snd_track_type, snd_track_start, buffering_type, file_byte_offset);
#if defined(USE_UTF16)
  return plugin_open(name, snd_track_type, snd_track_start, buffering_type, file_byte_offset);
#else
  BinkPluginSetError("BinkPluginOpenUTF16 not supported on this platform.");
  return 0;
#endif
}

PLUG_IN_FUNC_DEF( BINKPLUGIN * ) BinkPluginOpen( char const * name, U32 snd_track_type, S32 snd_track_start, U32 buffering_type, U64 file_byte_offset )
{
  API_LOG("BinkPluginOpen(%s, %i, %i, %i, %llu)\n", name, snd_track_type, snd_track_start, buffering_type, file_byte_offset);
#if defined(USE_UTF16)
{
  chartype *nameW = (unsigned short*)alloca(strspace(name)*2);
  str8to16(nameW,name);
  return plugin_open(nameW, snd_track_type, snd_track_start, buffering_type, file_byte_offset);
}
#else
  return plugin_open(name, snd_track_type, snd_track_start, buffering_type, file_byte_offset);
#endif
}


PLUG_IN_FUNC_DEF( void ) BinkPluginClose( BINKPLUGIN * bp )
{
  API_LOG("BinkPluginClose(%p)\n", bp);
  if ( !plugin_init() )
    return;

  if ( bp )
  {
    lock_bink( bp );
    bp->closing = 1;
    unlock_bink( bp );
  }
}


PLUG_IN_FUNC_DEF( void ) BinkPluginInfo( BINKPLUGIN * bp, BINKPLUGININFO * info )
{
  API_LOG("BinkPluginInfo(%p, %p)\n", bp, info);
  if ( !plugin_init() )
    return;

  if ( bp )
  {
    lock_bink( bp );
    if ( bp->b )
    {
      info->Width = bp->b->Width;
      info->Height = bp->b->Height;
      info->Frames = bp->b->Frames;
      info->FrameNum = bp->b->LastFrameNum;
      info->TotalFrames = bp->b->playedframes;
      info->FrameRate = bp->b->FrameRate;
      info->FrameRateDiv = bp->b->FrameRateDiv;
      info->LoopsRemaining = bp->loops;
      info->ReadError = bp->b->ReadError;
      info->TexturesError = ( bp->state == NO_TEXTURES ) ? 1 : 0 ;
      info->SndTrackType = bp->snd_track_type;
      info->NumTracksRequested = bp->num_tracks;
      info->NumTracksOpened = bp->b->NumTracks;
      info->BufferSize = bp->b->bio.CurBufSize;
      info->BufferUsed = bp->b->bio.CurBufUsed;
      info->SoundDropOuts = bp->b->soundskips;
      info->SkippedFrames = bp->skips;
      info->Alpha = bp->alpha;
      switch ( bp->state )
      {
        case AT_END:
          info->PlaybackState = 3;
          break;
        case PAUSED:
          info->PlaybackState = 1;
          break;
        default:
          info->PlaybackState = 0;
          break;
      }
      // set goto state
      if ( ( bp->goto_frame ) || ( bp->first_goto_frame ) )
        info->PlaybackState = 2;
    }
    unlock_bink( bp );

    lock_list();
    if ( processbinks_cnt < PB_WINDOW )
      info->ProcessBinksFrameRate = 0;
    else
      info->ProcessBinksFrameRate = (((F32)PB_WINDOW)*1000.0f) / (F32)(processbinks_times[processbinks_pos]-processbinks_times[(processbinks_pos+PB_WINDOW+1)&(PB_WINDOW-1)]);
    unlock_list();
  }
}


// attach a subtitle file to a bink
PLUG_IN_FUNC_DEF( S32 ) BinkPluginLoadSubtitles( BINKPLUGIN * bp, char const * srt_name )
{
  S32 ret = 0;

  API_LOG("BinkPluginLoadSubtitles(%p)\n", bp);
  if ( !plugin_init() )
    return 0;

#if defined(USE_UTF16)
  chartype *nameW = (unsigned short*)alloca(strspace(srt_name)*2);
  str8to16(nameW,srt_name);
#endif

  if ( bp )
  {
    lock_bink( bp );
    if ( bp->b )
    {
      ret = pBinkLoadSubtitles( bp->b, 
#if defined(USE_UTF16)
        (char const * )nameW
#else
        srt_name
#endif
      );
    }
    unlock_bink( bp );
  }

  return ret;
}

// attach a subtitle file to a bink, filename is UTF-16 - subtitle file contents are still UTF8 or ascii
PLUG_IN_FUNC_DEF( S32 ) BinkPluginLoadSubtitlesUTF16( BINKPLUGIN * bp, unsigned short const * srt_name )
{
#if defined(USE_UTF16)
  S32 ret = 0;
  API_LOG("BinkPluginLoadSubtitlesUTF16(%p)\n", bp);

  if ( !plugin_init() )
    return 0;

  if ( bp )
  {
    lock_bink( bp );
    if ( bp->b )
    {
      ret = pBinkLoadSubtitles( bp->b, (char const*)srt_name );
    }
    unlock_bink( bp );
  }

  return ret;
#else
  BinkPluginSetError("BinkPluginLoadSubtitlesUTF16 not supported on this platform.");
  return 0;
#endif
}

// get current subtitle
PLUG_IN_FUNC_DEF( char const * ) BinkPluginCurrentSubtitle( BINKPLUGIN * bp, U32 * iterate )
{
  char const * str;

  API_LOG("BinkPluginCurrentSubtitle(%p, %p)\n", bp, iterate);
  if ( !plugin_init() )
    return 0;

  str = 0;
  if ( bp )
  {
    lock_bink( bp );
    if ( bp->b )
    {
      str = pBinkCurrentSubtitle( bp->b, iterate, 0, 0 );
    }
    unlock_bink( bp );
  }

  return str;
}




PLUG_IN_FUNC_DEF( S32 ) BinkPluginScheduleToTexture( BINKPLUGIN * bp, F32 x0, F32 y0, F32 x1, F32 y1, S32 depth, void * render_target_texture, U32 render_target_width, U32 render_target_height )
{
  S32 i;
  API_LOG("BinkPluginScheduleToTexture(%p, %f, %f, %f, %f, %i, %p %i, %i)\n", bp, x0, y0, x1, y1, depth, render_target_texture, render_target_width, render_target_height);
  
  if ( !plugin_init() )
    return 0;

  if ( ( bp ) && ( render_target_texture ) )
  {
    U64 frame;

    lock_list();

    frame = drawvideo_frame;

    lock_bink( bp );
  
    // find spot
    for( i = 0 ; i < MAX_OVERLAY_DRAWS ; i++ )
      if ( bp->draws[i].frame == 0 )
        break;
    
    // did we find a spot?
    if ( i < MAX_OVERLAY_DRAWS )
    {
      bp->draws[i].Ax = x0;
      bp->draws[i].Ay = y0;
      bp->draws[i].Bx = x1;
      bp->draws[i].By = y0;
      bp->draws[i].Cx = x0;
      bp->draws[i].Cy = y1;
      bp->draws[i].depth = depth;
      bp->draws[i].to_texture = render_target_texture;
      bp->draws[i].to_texture_width = render_target_width;
      bp->draws[i].to_texture_height = render_target_height;
      bp->draws[i].to_texture_format = bp->format_idx;
      bp->draws[i].draw_flags = bp->draw_flags;
      bp->draws[i].frame = frame;
    }

    unlock_bink( bp );
    
    unlock_list();
  }
  return 1;
}


PLUG_IN_FUNC_DEF( S32 ) BinkPluginScheduleOverlay( BINKPLUGIN * bp, F32 x0, F32 y0, F32 x1, F32 y1, S32 depth )
{
  S32 i;
  API_LOG("BinkPluginScheduleOverlay(%p, %f, %f, %f, %f, %i)\n", bp, x0, y0, x1, y1, depth);
  
  if ( !plugin_init() )
    return 0;

  if ( bp )
  {
    U64 frame;

    lock_list();

    frame = drawvideo_frame;

    lock_bink( bp );
  
    // find spot
    for( i = 0 ; i < MAX_OVERLAY_DRAWS ; i++ )
      if ( bp->draws[i].frame == 0 )
        break;
    
    // did we find a spot?
    if ( i < MAX_OVERLAY_DRAWS )
    {
      bp->draws[i].Ax = x0;
      bp->draws[i].Ay = y0;
      bp->draws[i].Bx = x1;
      bp->draws[i].By = y0;
      bp->draws[i].Cx = x0;
      bp->draws[i].Cy = y1;
      bp->draws[i].depth = depth;
      bp->draws[i].draw_flags = bp->draw_flags;
      bp->draws[i].frame = frame;
    }

    unlock_bink( bp );
    
    unlock_list();
  }
  return 1;
}

PLUG_IN_FUNC_DEF( void ) BinkPluginAllScheduled( void )
{
  API_LOG("BinkPluginAllScheduled()\n");
  if ( !plugin_init() )
    return;

  lock_list();
  if(drawvideo_frame == (U64)-1) {
    drawvideo_frame = 1;
  } else {
    ++drawvideo_frame;
  }
  unlock_list();
}

PLUG_IN_FUNC_DEF( void ) BinkPluginPause( BINKPLUGIN * bp, S32 pause_frame )
{
  API_LOG("BinkPluginPause(%p, %i)\n", bp, pause_frame);
  if ( !plugin_init() )
    return;

  if ( bp )
  {
    lock_bink( bp );
    if ( bp->b )
    {
      bp->paused_frame = pause_frame;
      if ( pause_frame == 0 )
      {
        if ( bp->b->Paused ) 
          pBinkPause( bp->b, 0 ); // resume if unpausing
        if ( bp->state == PAUSED )
          bp->state = IDLE;
      }
    }
    unlock_bink( bp );
  }
}


PLUG_IN_FUNC_DEF( void ) BinkPluginGoto( BINKPLUGIN * bp, S32 goto_frame, S32 ms_per_process )
{
  API_LOG("BinkPluginGoto(%p, %i, %i)\n", bp, goto_frame, ms_per_process);
  if ( !plugin_init() )
    return;

  if ( bp )
  {
    lock_bink( bp );
    if ( bp->b )
    {
      if ( (U32)goto_frame > bp->b->Frames )
        goto_frame = bp->b->Frames;
      if( goto_frame == 0 )
        goto_frame = 0xffffffff;  // signal to stop a previous goto
      else
        pBinkSetSoundOnOff( bp->b, 0 ); // stop audio, if this isn't a cancel

      bp->first_goto_frame = 1;
      bp->goto_frame = goto_frame;
      bp->goto_time = ms_per_process;
    }
    unlock_bink( bp );
  }
}


PLUG_IN_FUNC_DEF( void ) BinkPluginVolume( BINKPLUGIN * bp, F32 vol )
{
  API_LOG("BinkPluginVolume(%p, %f)\n", bp, vol);
  if ( !plugin_init() )
    return;

  if ( bp )
  {
    lock_bink( bp );
    if ( bp->b )
    {
      U32 i;
      for( i = 0 ; i < bp->num_tracks ; i++ )
        pBinkSetVolume( bp->b, bp->tracks[i], (S32) (32768.0*vol) );
    }
    unlock_bink( bp );
  }
}


PLUG_IN_FUNC_DEF( void ) BinkPluginSpeakerVolumes( BINKPLUGIN * bp, F32 * vols, U32 count )
{
  API_LOG("BinkPluginSpeakerVolumes(%p, %p, %i)\n", bp, vols, count);
  if ( !plugin_init() )
    return;

  if ( bp )
  {
    lock_bink( bp );
    if ( bp->b )
    {
      if ( bp->num_tracks != count )
      {
        BinkPluginSetError( "Wrong track count used in SpeakerVolumes." );
      }
      else
      {
        set_volumes( bp, vols );
      }
    }
    unlock_bink( bp );
  }
}


PLUG_IN_FUNC_DEF( void ) BinkPluginLoop( BINKPLUGIN * bp, U32 loops )
{
  API_LOG("BinkPluginLoop(%p, %i)\n", bp, loops);
  if ( !plugin_init() )
    return;

  if ( bp )
  {
    lock_bink( bp );
    if ( bp->b )
    {
      bp->loops = loops;
      if ( bp->state == AT_END )
      {
        bp->state = DO_NEXT;
      }
    }
    unlock_bink( bp );
  }
}

PLUG_IN_FUNC_DEF( void ) BinkPluginSetHdrSettings( BINKPLUGIN * bp, U32 tonemap, F32 exposure, S32 output_nits )
{
  API_LOG("BinkPluginSetHdrSettings(%p, %i, %f, %i)\n", bp, tonemap, exposure, output_nits);
  if ( !plugin_init() )
    return;

  if ( bp )
  {
    lock_bink( bp );
    if ( bp->b && bp->textures )
    {
      Set_Bink_hdr_settings( bp->textures, tonemap, exposure, output_nits );
    }
    unlock_bink( bp );
  }
}

PLUG_IN_FUNC_DEF( void ) BinkPluginSetAlphaSettings( BINKPLUGIN * bp, F32 alpha )
{
  API_LOG("BinkPluginSetAlphaSettings(%p, %f)\n", bp, alpha);
  if ( !plugin_init() )
    return;

  if ( bp )
  {
    lock_bink( bp );
    bp->alpha = alpha;
    unlock_bink( bp );
  }
}

PLUG_IN_FUNC_DEF( void ) BinkPluginSetDrawFlags( BINKPLUGIN * bp, S32 draw_flags )
{
  API_LOG("BinkPluginSetDrawFlags(%p, %x)\n", bp, draw_flags);
  if ( !plugin_init() )
    return;

  if ( bp )
  {
    lock_bink( bp );
    bp->draw_flags = draw_flags;
    unlock_bink( bp );
  }
}

PLUG_IN_FUNC_DEF( void ) BinkPluginSetRenderTargetFormat( BINKPLUGIN * bp, U32 format_idx )
{
  API_LOG("BinkPluginSetRenderTargetFormat(%p, %i)\n", bp, format_idx);
  if ( !plugin_init() )
    return;

  if ( bp )
  {
    lock_bink( bp );
    bp->format_idx = format_idx;
    unlock_bink( bp );
  }
}

PLUG_IN_FUNC_DEF( S32 ) BinkPluginGetPlatformInfo( U32 bink_platform_enum, void * output_ptr )
{
  API_LOG("BinkPluginGetPlatformInfo(%i, %p)\n", bink_platform_enum, output_ptr);
  if ( !plugin_init() )
    return 0;

  return pBinkGetPlatformInfo( bink_platform_enum, output_ptr );
}

PLUG_IN_FUNC_DEF( void ) BinkPluginLimitSpeakers( U32 speaker_count )
{
  API_LOG("BinkPluginLimitSpeakers(%i)\n", speaker_count);

  if ( (( speaker_count >= 1 ) && ( speaker_count <= 4 )) || ( speaker_count == 6 ) || ( speaker_count == 8 ) )
  {
    if ( limit_speakers_to != speaker_count )
    {
      limit_speakers_to = speaker_count;

      if ( !plugin_init() )
        return;

      // reset audio system on android, and linux-a-likes
      #if defined(__RADANDROID__)
        if ( ( pBinkOpenSLES ) && ( pBinkSetSoundSystem2 ) )
          pBinkSetSoundSystem2(pBinkOpenSLES,48000,limit_speakers_to);
      #endif

      #if defined(__RADLINUX__)
        if ( ( pBinkOpenPulseAudio ) && ( pBinkSetSoundSystem2 ) )
          pBinkSetSoundSystem2(pBinkOpenPulseAudio,48000,limit_speakers_to);
      #endif
    
      #ifdef BINK_NDA_SOUND
        BINK_NDA_SOUND(limit_speakers_to); 
      #endif
    }
  }
}

#if defined(__RADWIN__)

PLUG_IN_FUNC_DEF( S32 ) BinkPluginWindowsUseXAudioDevice( char const * strstr_device_name )
{
  short buffer[512];
  short * b = 0;
  API_LOG("BinkPluginWindowsUseXAudioDevice(%s)\n", strstr_device_name);
  if ( !plugin_init() )
    return 0;
  
  if ( (strstr_device_name) && (strstr_device_name[0] != 0 ) )
    if ( pBinkFindXAudio2WinDevice( buffer, strstr_device_name ) )
      b = buffer;

  if ( pBinkSetSoundSystem2(pBinkOpenXAudio2,(UINTa)(SINTa)-2,(UINTa)b) )
  {
    using_xaudio = 1;
    return 1;
  }

  return 0;
}

#if defined(__RADNT__) && !defined(BUILDING_FOR_UNREAL_ONLY)

PLUG_IN_FUNC_DEF( S32 ) BinkPluginWindowsUseDirectSound( void )
{
  API_LOG("BinkPluginWindowsUseDirectSound()\n");
  if ( !plugin_init() )
    return 0;

  return pBinkSetSoundSystem(pBinkOpenDirectSound,0) ? 1 : 0;
}

#endif

#if PLATFORM_HAS_D3D9
PLUG_IN_FUNC_DEF( void ) BinkPluginWindowsD3D9BeginReset( void )
{
  API_LOG("BinkPluginWindowsD3D9BeginReset()\n");
  if ( !plugin_init() )
    return;

  if ( ( gapitype == BinkD3D9 ) && ( currently_reset == 0 ) )
  {
    // now release all the per-bink textures
    process_binks( -1, 1 );
    currently_reset = 1;
  }
}


PLUG_IN_FUNC_DEF( void ) BinkPluginWindowsD3D9EndReset( void )
{
  API_LOG("BinkPluginWindowsD3D9EndReset()\n");
  if ( !plugin_init() )
    return;

  if ( ( gapitype == BinkD3D9 ) && ( currently_reset == 1 ) )
  {
    currently_reset = 0;
    process_binks( 1, 0 );
  }
}
#endif
#endif

PLUG_IN_FUNC_DEF( void ) BinkPluginSetPerFrameInfo(BINKPLUGINFRAMEINFO * info)
{
  API_LOG("BinkPluginSetPerFrameInfo(%p)\n", info);
  screen_data = *(BINKPLUGINFRAMEINFO*)info;
#ifdef COPY_FROM_CONTEXT_TO_GLOBAL
  gcontext = screen_data.cmdBuf;
#endif
}

void indirectBinkGetFrameBuffersInfo( HBINK bink, BINKFRAMEBUFFERS * fbset )
{
  pBinkGetFrameBuffersInfo( bink, fbset );
}


void indirectBinkRegisterFrameBuffers( HBINK bink, BINKFRAMEBUFFERS * fbset )
{
  pBinkRegisterFrameBuffers( bink, fbset );
}

S32  indirectBinkGetGPUDataBuffersInfo( HBINK bink, BINKGPUDATABUFFERS * gpu )
{
  return pBinkGetGPUDataBuffersInfo( bink, gpu );
}

void indirectBinkRegisterGPUDataBuffers( HBINK bink, BINKGPUDATABUFFERS * gpu )
{
  pBinkRegisterGPUDataBuffers( bink, gpu );
}

void * indirectBinkUtilMalloc( U64 bytes )
{
  return ourmalloc( bytes );
}

void indirectBinkUtilFree( void * ptr )
{
  ourfree( ptr );
}

S32 indirectBinkAllocateFrameBuffers( HBINK bp, BINKFRAMEBUFFERS * set, U32 minimum_alignment )
{
  return pBinkAllocateFrameBuffers( bp, set, minimum_alignment );
}

#ifdef BUILDING_FOR_UNREAL_ONLY
// this dummy function keeps all the cpu blitters out of unreal, which we don't use
S32 RADLINK BinkCopyToBuffer( HBINK bink, void* dest, S32 destpitch, U32 destheight, U32 destx, U32 desty, U32 flags )
{
  return 0;
}
#else
#include "binkplugin_image.inl"  // image not used in unreal
#endif

#ifdef BUILDING_WITH_UNITY
#include "binkpluginunity.inl"
#endif

