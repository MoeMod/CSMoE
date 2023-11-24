// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#define _GNU_SOURCE

#include "binkplugin.h"
#include "binkpluginos.h"

#ifdef RADUSETM3
#include "telemetryplugin.h"
#endif

#ifdef BUILDING_FOR_UNREAL_ONLY

//#define ProcessProc( bytes, ret, name, ...) extern ret name(__VA_ARGS__);
#define ProcessProc( bytes, ret, name, ...) RR_STRING_JOIN( name, Proc ) * RR_STRING_JOIN( p, name ) = name;
  DO_PROCS()
#if defined(__RADANDROID__)
  DO_ANDROID_PROCS()
#endif
#if defined(__RADLINUX__)
  DO_LINUX_PROCS()
#endif
#undef ProcessProc

#else

#include <stddef.h>
#include <stdlib.h>
#include <dlfcn.h>


#ifdef __RADANDROID__
#include <android/log.h>
#ifdef __RAD64__
  #define Bink2SharedLib "libbink2androidarm64.so" 
  #define Bink1SharedLib "libbinkandroidarm64.so" 
#elif defined( __RADX86__ )
  #define Bink2SharedLib "libbink2androidx86.so" 
  #define Bink1SharedLib "libbinkandroidx86.so" 
#else
  #define Bink2SharedLib "libbink2android.so" 
  #define Bink1SharedLib "libbinkandroid.so" 
#endif
#else  
#ifdef STADIA
  #define Bink2SharedLib "libBink2Stadia.so" 
  #define Bink1SharedLib "libBinkStadia.so" 
#else
#ifdef __RAD64__
  #define Bink2SharedLib "libBink2x64.so" 
  #define Bink1SharedLib "libBinkx64.so" 
#else
  #define Bink2SharedLib "libBink2.so" 
  #define Bink1SharedLib "libBink.so" 
#endif
#endif
#endif

#define ProcessProc( bytes, ret, name, ... ) RR_STRING_JOIN( name, Proc ) * RR_STRING_JOIN( p, name );
  DO_PROCS()
#if defined(__RADANDROID__)
  DO_ANDROID_PROCS()
#endif
#if defined(__RADLINUX__)
  DO_LINUX_PROCS()
#endif
#undef ProcessProc

S32 dynamically_load_procs( void )
{
  void * l;
  char dll[ 1024 ];
  char * fn;

  if ( BinkPIPath[0] )
  {
    char const * d = BinkPIPath;
    fn = dll;
    do
    {
      *fn++ = *d++;
    } while ( d[0] );

    // add a slash, if we don't have one.
    if ( ( d[-1] != '/' ) && ( d[-1] != '\\' ) )
      *fn++ = '/';
  }
  else
  {
    char const * s;
    char * d;
    Dl_info dl_info;
    dl_info.dli_fname = "";
    
    if ( dladdr( dynamically_load_procs, &dl_info) == 0 )
      dl_info.dli_fname = "";  

    s = dl_info.dli_fname;
    fn = dll;
    d = dll;
    d[0] = 0;
    while ( s[0] )
    {
      d[0] = s[0];
      if ( ( d[0] == '\\' ) || ( d[0] == '/' ) ) fn = d + 1;
      ++s;
      ++d;
    }
  }

  ourstrcpy( fn, Bink2SharedLib );
  l = dlopen( dll, RTLD_LAZY | RTLD_GLOBAL );
  if ( l == 0 )
  {
    ourstrcpy( fn, Bink1SharedLib );
    l = dlopen( dll, RTLD_LAZY | RTLD_GLOBAL );
    if ( l == 0 )
    {
      BinkPluginSetError( "Couldn't load Bink shared object." );
      return 0;
    }
  }

#define STRINGDELAY( v ) #v
#define STRINGIZE( v ) STRINGDELAY( v )

#define ProcessProc( num, ret, name, ... ) {RR_STRING_JOIN( p, name ) = ( RR_STRING_JOIN( name, Proc )* ) dlsym( l, STRINGIZE( name ) ); if ( RR_STRING_JOIN( p, name ) == 0 ) { BinkPluginSetError( "Error finding: " STRINGIZE(name)); return 0; } };
  DO_PROCS()
#if defined(__RADANDROID__)
  DO_ANDROID_PROCS()
#endif
#if defined(__RADLINUX__)
  DO_LINUX_PROCS()
#endif
#undef ProcessProc

  return 1;
}

#endif

#include <malloc.h>

static BinkPluginAlloc_t * Alloc = 0;
static BinkPluginFree_t * Free = 0; 

void * osmalloc( U64 bytes )
{
  #if !defined(__RAD64__)
    if ( bytes > 0xffffffff )
      return 0;
  #endif

  if (Alloc)
    return Alloc( (UINTa)bytes, 16 );

  #if !defined(__RAD64__)
    return memalign( 16, (U64) bytes );
  #else
    return memalign( 16, bytes );
  #endif
}

void osfree( void * ptr )
{
  if (Free)
  {
    Free( ptr );
    return;
  }

  free( ptr );
}

void osmemoryreset( void )
{
#if !defined(__RADANDROID__)
  malloc_trim( 0 );
#endif
}

void set_mem_allocators( BINKPLUGININITINFO * info )
{
  if(info) 
  {
    Alloc = info->alloc;
    Free = info->free;
  }
}

#ifdef nofprintf

void oserr( char const * err )
{
}

#else

#include <stdio.h>

void oserr( char const * err )
{
  // uncomment for debugging
#ifdef __RADANDROID__
//  __android_log_print(ANDROID_LOG_INFO, "Bink", "%s", err);
#else
//  fprintf( stderr, "Bink: %s", err );
#endif
}

#endif
