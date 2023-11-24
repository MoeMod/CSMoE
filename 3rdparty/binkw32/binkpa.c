// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

// based on the simple api source
#include "bink.h"
#include <pulse/pulseaudio.h>
#include <pulse/thread-mainloop.h>

// @cdep pre $addcompilerincludedir( $ProjectsBase/shared/sound )

static pa_threaded_mainloop *mainloop = 0;
static pa_context *context = 0;
static pa_stream *stream = 0;
static pa_buffer_attr bufinfo;


#define BINKPA_LIST \
PA( pa_stream_state_t, pa_stream_get_state,  pa_stream *   p ) \
PA( int,  pa_stream_connect_playback ,  pa_stream *   s, const char *    dev, const pa_buffer_attr *    attr,  pa_stream_flags_t   flags, const pa_cvolume *    volume, pa_stream *   sync_stream  ) \
PA( void, pa_stream_set_latency_update_callback, pa_stream *   p, pa_stream_notify_cb_t   cb, void *    userdata ) \
PA( void, pa_stream_set_write_callback, pa_stream *   p, pa_stream_request_cb_t    cb, void *    userdata ) \
PA( void, pa_stream_set_state_callback, pa_stream *   s, pa_stream_notify_cb_t   cb,  void *    userdata ) \
PA( pa_stream* , pa_stream_new,  pa_context *    c, const char *    name,  const pa_sample_spec *    ss, const pa_channel_map *    map ) \
PA( size_t,  pa_stream_writable_size, pa_stream *   p ) \
PA( int,  pa_stream_write, pa_stream *   p, const void *    data,  size_t    nbytes, pa_free_cb_t    free_cb, int64_t   offset, pa_seek_mode_t    seek ) \
PA( void, pa_stream_unref, pa_stream *   s ) \
PA( void, pa_threaded_mainloop_wait  ,   pa_threaded_mainloop *    m ) \
PA( void, pa_threaded_mainloop_unlock  ,   pa_threaded_mainloop *    m ) \
PA( int, pa_threaded_mainloop_start  ,   pa_threaded_mainloop *    m ) \
PA( void, pa_threaded_mainloop_stop  ,   pa_threaded_mainloop *    m ) \
PA( void, pa_threaded_mainloop_lock  ,   pa_threaded_mainloop *    m ) \
PA( void, pa_threaded_mainloop_free  ,   pa_threaded_mainloop *    m ) \
PA( pa_mainloop_api*, pa_threaded_mainloop_get_api   ,   pa_threaded_mainloop *    m ) \
PA( pa_threaded_mainloop*, pa_threaded_mainloop_new  ,   void      ) \
PA( void, pa_threaded_mainloop_signal  ,   pa_threaded_mainloop *    m, int   wait_for_accept ) \
PA( void, pa_context_unref   ,   pa_context *    c ) \
PA( pa_context_state_t, pa_context_get_state   ,   pa_context *    c ) \
PA( void, pa_context_disconnect  ,   pa_context *    c ) \
PA( int, pa_context_connect  ,   pa_context *    c, const char *    server, pa_context_flags_t    flags, const pa_spawn_api *    api ) \
PA( void, pa_context_set_state_callback  ,   pa_context *    c,  pa_context_notify_cb_t    cb, void *    userdata ) \
PA( pa_context*, pa_context_new  ,   pa_mainloop_api *   mainloop, const char *    name ) 


typedef void voidfunc(void);

#ifdef _MSC_VER

#define PAAPI __cdecl
#include <windows.h>

static void * PALoad( void )
{
  return (void *)LoadLibraryA("libpulse-0.dll");
}

static voidfunc * PAGetAddr( void * handle, char const * proc )
{
  return (voidfunc*) GetProcAddress( (HMODULE)handle, proc );
}

#else

#include <stddef.h>
#include <stdlib.h>
#include <dlfcn.h>

#define PAAPI

static void * PALoad( void )
{
#ifdef STADIA 
  // Newer SDK/Devkit versions changed the SO filename to libpulse.so.0, older ones used libpulse.so. 
  void *h = dlopen( "libpulse.so.0", RTLD_LAZY | RTLD_GLOBAL );
  if(!h) 
      h = dlopen( "libpulse.so", RTLD_LAZY | RTLD_GLOBAL );
  return h;
#else
  return dlopen( "libpulse.so", RTLD_LAZY | RTLD_GLOBAL );
#endif
}

static voidfunc * PAGetAddr( void * handle, char const * proc )
{
  return (voidfunc*) dlsym( handle, proc );
}

#endif

#ifdef STADIA 

typedef struct gpstatus 
{
  U32 code;
  U32 token;
} gpstatus;

#define STADIA_LIST \
PA( U64, GgpGetGameAudioEndpoint_v1, gpstatus * s ) \
PA( U64, GgpAudioEndpointGetPulseAudioDeviceName_v1, U64 endpoint, char * out, U64 out_len, gpstatus * s ) \

static void * GGPLoad( void )
{
  return dlopen( "libggp.so", RTLD_LAZY | RTLD_GLOBAL );
}

#else

#define STADIA_LIST

#endif

#define PA(ret, name, ...) typedef ret PAAPI name##proc(__VA_ARGS__); static name##proc * p##name;
BINKPA_LIST
STADIA_LIST
#undef PA


static void * pahandle;
static char const * errfunc = 0;

static int load_extensions( void )
{
  if ( pahandle == 0 )
  {
    pahandle = PALoad();

#define PA(ret, name, ...) p##name = (name##proc *) PAGetAddr(pahandle, #name); if ( p##name == 0 ) { errfunc = #name; return 0;}
    BINKPA_LIST
#ifdef STADIA 
    pahandle = GGPLoad();
#endif
    STADIA_LIST
#undef PA

  }
  return 1;
}

static void context_state_cb(pa_context *c, void *userdata) 
{
  if (c)
    switch (ppa_context_get_state(c)) 
    {
        case PA_CONTEXT_READY:
        case PA_CONTEXT_TERMINATED:
        case PA_CONTEXT_FAILED:
            ppa_threaded_mainloop_signal(mainloop, 0);
            break;

        case PA_CONTEXT_UNCONNECTED:
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME:
            break;
    }
}

static void stream_state_cb(pa_stream *s, void * userdata) 
{
    switch (ppa_stream_get_state(s)) 
    {

        case PA_STREAM_READY:
        case PA_STREAM_FAILED:
        case PA_STREAM_TERMINATED:
            ppa_threaded_mainloop_signal(mainloop, 0);
            break;

        case PA_STREAM_UNCONNECTED:
        case PA_STREAM_CREATING:
            break;
    }
}

static void stream_request_cb(pa_stream *s, size_t length, void *userdata) 
{
  if (mainloop)
    ppa_threaded_mainloop_signal(mainloop, 0);
}

static void stream_latency_update_cb(pa_stream *s, void *userdata) 
{
  if (mainloop)
    ppa_threaded_mainloop_signal(mainloop, 0);
}


static int bink_pulseaudio_open( int freq, int chans, int * fragsize )
{
    pa_sample_spec ss;
    pa_channel_map map;
    #ifdef STADIA
      char dev_buffer[128];
      char * pa_dev_name = 0;
    #else
      #define pa_dev_name 0  
    #endif 

    if ( !load_extensions() )
      return 0;

    mainloop = ppa_threaded_mainloop_new();
    if ( !mainloop )
        return 0;

    context = ppa_context_new(ppa_threaded_mainloop_get_api(mainloop), "Bink");
    if ( !context )
    {
      freemainloop:
      ppa_threaded_mainloop_stop(mainloop);
      ppa_threaded_mainloop_free( mainloop );
      mainloop = 0;
      return 0;
    }

    ppa_context_set_state_callback(context, context_state_cb, 0);

    if (ppa_context_connect(context, 0, 0, NULL) < 0) 
    {
        freecontext:
        ppa_context_unref(context);
        context = 0;
        goto freemainloop;
    }

    ppa_threaded_mainloop_lock(mainloop);

    if (ppa_threaded_mainloop_start(mainloop) < 0)
    {
        unlockerr:
        ppa_context_disconnect(context);
        ppa_threaded_mainloop_unlock(mainloop);
        goto freecontext;
    }

    // wait for the context to get ready
    for (;;) 
    {
        pa_context_state_t state;
        state = ppa_context_get_state(context);
        if (state == PA_CONTEXT_READY)
            break;
        if (!PA_CONTEXT_IS_GOOD(state)) 
          goto unlockerr;
        ppa_threaded_mainloop_wait(mainloop);
    }

    ss.format = PA_SAMPLE_S16LE;
    ss.channels = (char)chans;
    ss.rate = freq;
    map.channels = ss.channels;
    map.map[0]=PA_CHANNEL_POSITION_FRONT_LEFT;
    map.map[1]=PA_CHANNEL_POSITION_FRONT_RIGHT;
    map.map[2]=PA_CHANNEL_POSITION_FRONT_CENTER;
    map.map[3]=PA_CHANNEL_POSITION_LFE;
    map.map[4]=PA_CHANNEL_POSITION_REAR_LEFT;
    map.map[5]=PA_CHANNEL_POSITION_REAR_RIGHT;
    map.map[6]=PA_CHANNEL_POSITION_SIDE_LEFT;
    map.map[7]=PA_CHANNEL_POSITION_SIDE_RIGHT;

    stream = ppa_stream_new(context, "BinkAudio", &ss, &map);
    if ( !stream ) 
      goto unlockerr;

    ppa_stream_set_state_callback(stream, stream_state_cb, 0);
    ppa_stream_set_write_callback(stream, stream_request_cb, 0);
    ppa_stream_set_latency_update_callback(stream, stream_latency_update_cb, 0);

    #ifdef STADIA
      {
        gpstatus status; status.code = 0; 
        U64 ep = pGgpGetGameAudioEndpoint_v1( &status );
        if ( status.code == 0 )
        {
          status.code = 0;
          if ( pGgpAudioEndpointGetPulseAudioDeviceName_v1( ep, dev_buffer, 128, &status) )
          {
            if ( status.code == 0 )
              pa_dev_name = dev_buffer;
          }
        }
      }
    #endif

    // mixsize is 8 ms
    bufinfo.minreq = ( ( freq * 8 ) / 1000 ) * sizeof(short) * chans;
    bufinfo.maxlength = bufinfo.minreq * 17; // can oversend by one mixsize
    bufinfo.tlength = bufinfo.maxlength;
    bufinfo.prebuf = bufinfo.minreq * 8;
    bufinfo.fragsize = 0xffffffff;

    if ( ppa_stream_connect_playback(stream, pa_dev_name, &bufinfo,
                                    PA_STREAM_NOFLAGS, 
                                    0, 0) < 0 )
      goto unlockerr;

    // now wait for the stream to get ready, gad
    for (;;) 
    {
        pa_stream_state_t state;
        state = ppa_stream_get_state(stream);
        if (state == PA_STREAM_READY)
            break;
        if (!PA_STREAM_IS_GOOD(state))
          goto unlockerr; 
        ppa_threaded_mainloop_wait(mainloop);
    }

    // ok, everything loaded, return sucess
    ppa_threaded_mainloop_unlock(mainloop);
    *fragsize = bufinfo.minreq;
    return 1;
}


static void bink_pulseaudio_close(void) 
{
    if (mainloop)
        ppa_threaded_mainloop_stop(mainloop);
  
    if (stream)
        ppa_stream_unref(stream);

    if (context) 
    {
        ppa_context_disconnect(context);
        ppa_context_unref(context);
    }

    if (mainloop)
        ppa_threaded_mainloop_free(mainloop);

    mainloop = 0;
    stream = 0;
    context = 0;
}

static int pulseok()
{
  if ( context == 0 )
    return 0;
  if ( !PA_CONTEXT_IS_GOOD(ppa_context_get_state(context)) )  // TODO  
    return 0;
  if ( ppa_context_get_state(context) == PA_CONTEXT_FAILED )
    return 0;
  if ( stream == 0 )
    return 0;
  if ( !PA_STREAM_IS_GOOD(ppa_stream_get_state(stream)) )
    return 0;
  if ( ppa_stream_get_state(stream) == PA_STREAM_FAILED )
    return 0;
  return 1;
}

static int get_pulse_writeable()
{
  int l = ppa_stream_writable_size(stream);
  if ( l >= bufinfo.minreq ) return bufinfo.minreq;
  return 0;
}

static int bink_pulseaudio_ready()
{
    int amt;
    if ( mainloop == 0 )
      return 0;

    ppa_threaded_mainloop_lock(mainloop);
    if ( !pulseok() )
    {
      fail:
      ppa_threaded_mainloop_unlock(mainloop);
      return 0;
    }

    amt = get_pulse_writeable();
    if ( amt == -1 ) goto fail;

    ppa_threaded_mainloop_unlock(mainloop);
    return (int) amt;
}

static int bink_pulseaudio_write( const void * data, int length ) 
{
    if ( mainloop == 0 )
      return 0;

    ppa_threaded_mainloop_lock(mainloop);
    if ( !pulseok() )
    {
      fail:
      ppa_threaded_mainloop_unlock(mainloop);
      return 0;
    }

    while ( length > 0 ) 
    {
        int amt;

        amt = get_pulse_writeable();
        if ( amt == -1 ) goto fail;
        if ( amt == 0 )
        {
          ppa_threaded_mainloop_wait(mainloop);
          if ( !pulseok() ) return -1;
        }

        if (amt > length)
            amt = length;

        if ( ppa_stream_write(stream, data, amt, NULL, 0LL, PA_SEEK_RELATIVE) < 0 )
          goto fail;

        data = (char const*) data + amt;
        length -= amt;
    }

    ppa_threaded_mainloop_unlock(mainloop);
    return 1;
}

static int bink_pulseaudio_check( int * freq, int * chans, int * latency_ms, int * poll_ms )
{
  *latency_ms = 8*8; // prebuf size
  *poll_ms = 7;
  return 1;
}

#include "binkssmix.h"

static audio_funcs funcs = { bink_pulseaudio_open, bink_pulseaudio_close, bink_pulseaudio_ready, bink_pulseaudio_write, bink_pulseaudio_check };

RADEXPFUNC BINKSNDOPEN RADEXPLINK BinkOpenPulseAudio( UINTa freq, UINTa speakers )
{
  return BinkOpenSSMix( &funcs, (U32)freq, (U32)speakers );
}

