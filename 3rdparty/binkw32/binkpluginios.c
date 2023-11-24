// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#define _GNU_SOURCE

#include "binkplugin.h"
#include "binkpluginos.h"

#ifdef RADUSETM3
#include "telemetryplugin.h"
#endif

#include <dlfcn.h>
#include <malloc/malloc.h>

#define ProcessProc( bytes, ret, name, ... ) \
  extern ret name(__VA_ARGS__); \
  RR_STRING_JOIN( name, Proc ) * RR_STRING_JOIN( p, name ) = name;
DO_PROCS()
DO_CA_PROCS()
#undef ProcessProc

static malloc_zone_t * ourheap = 0;

static BinkPluginAlloc_t * Alloc = 0;
static BinkPluginFree_t * Free = 0; 

void * osmalloc( U64 bytes )
{
  if (Alloc)
    return Alloc( (UINTa)bytes, 16 );

  if ( ourheap == 0 )
  {
    ourheap = malloc_create_zone( 256, 0 );
    if ( ourheap == 0 )
    {
      return( 0 );
    }
  }
  #ifdef __RAD64__
    return( malloc_zone_memalign( ourheap, 16, bytes ) );
  #else
    if ( bytes > 0xffffffff )
      return 0;
    return( malloc_zone_memalign( ourheap, 16, (U32)bytes ) );
  #endif
}

void osfree( void * ptr )
{
  if (Free)
  {
    Free( ptr );
    return;
  }

  malloc_zone_free( ourheap, ptr );
}

void osmemoryreset( void )
{
  if ( ourheap )
  {
    malloc_destroy_zone( ourheap );
    ourheap = 0;
  }
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
  fprintf( stderr, "Bink: %s", err );
}

#endif

