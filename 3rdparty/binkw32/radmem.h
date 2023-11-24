// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#ifndef __RADMEMH__
  #define __RADMEMH__

  RADDEFSTART

  #if defined( __RADNT__ ) || defined( __RADWINRT__ )
    RADDEFFUNC void * RADLINK radmalloc_init(void);
  #else
    #define radmalloc_init() 0
  #endif

  typedef void  * (RADLINK  * RADMEMALLOC) (U64 bytes);
  typedef void   (RADLINK  * RADMEMFREE)  (void   * ptr);

// Granny has it's own version of these to prevent link collisions.  Note that Granny
// doesn't define SetMemory, since if you need to hijack Granny's allocation, it goes
// through GrannySetAllocator.
#if ( defined( BUILDING_GRANNY ) && BUILDING_GRANNY ) || ( defined( BUILDING_GRANNY_STATIC ) && BUILDING_GRANNY_STATIC )
  #define radmalloc g_radmalloc
  #define radfree   g_radfree

  RADDEFFUNC void * RADLINK g_radmalloc(U64 numbytes);
  RADDEFFUNC void RADLINK g_radfree(void * ptr);
#else
  RADDEFFUNC void RADLINK RADSetMemory(RADMEMALLOC a,RADMEMFREE f);

  #ifdef RADUSETM3
    #define radmalloc( b ) radmalloci( b, __FILE__, __LINE__ )
    RADDEFFUNC void * RADLINK radmalloci(U64 numbytes,char const * info, U32 line );
  #else
    RADDEFFUNC void * RADLINK radmalloc(U64 numbytes);
  #endif

    RADDEFFUNC void RADLINK radfree(void * ptr);
#endif

  RADDEFEND

// @cdep pre $pullinradmemsource


#endif
