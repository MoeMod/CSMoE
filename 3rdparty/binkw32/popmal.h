// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#ifndef __POPMALH__
#define __POPMALH__

#ifndef __RADRR_COREH__
  #include "rrCore.h"
#endif

#ifndef __RADMEMH__
#include "radmem.h"
#endif

#ifdef WRAP_PUBLICS
#define rfmerge3(name,add) name##add
#define rfmerge2(name,add) rfmerge3(name,add)
#define rfmerge(name)      rfmerge2(name,WRAP_PUBLICS)
#define pushmallocinit                    rfmerge(pushmallocinit)
#define pushmalloc                        rfmerge(pushmalloc)
#define pushmalloco                       rfmerge(pushmalloco)
#define popmalloctotal                    rfmerge(popmalloctotal)
#define popmalloc                         rfmerge(popmalloc)
#endif

#define Round32( num ) ( ( ( num ) + 31 ) & ~31 )

#define PushMallocBytesForXPtrs( X ) ( ( X * sizeof(void*) ) + ( X * sizeof(U64) ) + 64 )

RADDEFFUNC void RADLINK pushmallocinit(void * base,U32 num_ptrs);
RADDEFFUNC void RADLINK pushmalloc( void * base, void  * ptr, U64 amt );

// ptr is an offset within the final allocation that will be popped (for allocating ptrs within a structure that you allocate with popmalloc)
RADDEFFUNC void RADLINK pushmalloco(void* base, void * ptr,U64 amt);

RADDEFFUNC U64 RADLINK popmalloctotal( void * base );

#ifdef RADUSETM3
  #define popmalloc( base,b ) popmalloci( base, b, __FILE__, __LINE__ )
  RADDEFFUNC void  * RADLINK popmalloci( void * base, U64 amt, char const * info, U32 line );
#else
  RADDEFFUNC void  * RADLINK popmalloc( void * base, U64 amt );
#endif

//#define SEPARATEMALLOCS

#ifdef SEPARATEMALLOCS
RADDEFFUNC void RADLINK popfree(void  * ptr);
#else
#define popfree radfree
#endif

#endif
