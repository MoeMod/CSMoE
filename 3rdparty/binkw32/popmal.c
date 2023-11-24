// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#ifndef __RADRR_COREH__
  #include "rrCore.h"
#endif

#ifndef __RADMEMH__
#include "radmem.h"
#endif

#ifdef __RADMAC__
  #include <memory.h>
#endif

#define SEGMENT_NAME "popmal"
#include "binksegment.h"

#include "popmal.h"

typedef struct PM
{
  void * * * ptrs;
  void * * * pushptr;
  U64 * amt;
  U64 * pushamt;
  U64 pushtot;
  U32 pushcur;
  U32 cursize;
} PM;


RADDEFFUNC void RADLINK pushmallocinit(void* base,U32 num_ptrs)
{
  PM * p = (PM *)base;
  typedef char blah[ ( PushMallocBytesForXPtrs( 0 ) >= sizeof(PM) ) ? 1 : -1 ];

  p->ptrs = (void * * *) ( ( (char*) base ) + PushMallocBytesForXPtrs( 0 ) );
  p->amt = (U64 *) ( ( (char*) p->ptrs ) + ( num_ptrs * sizeof(void*) ) );
  p->pushtot = 0;
  p->pushcur = 0;
  p->pushptr = p->ptrs;
  p->pushamt = p->amt;
  p->cursize = num_ptrs;
}

RADDEFFUNC void RADLINK pushmalloc(void* base, void * ptr,U64 amt)
{
  PM * p = (PM *)base;
  if ( p->cursize == p->pushcur )
  {
    RR_BREAK();
  }

#ifndef SEPARATEMALLOCS
  {
    U64 last,next;

    amt=Round32(amt);
    last=((p->pushtot/32)&31)+1;
    next=(amt/32)&31;

    // make sure the up 32 separate mallocs use distinct sets
    amt+=(((32+last-next)&31)*32);
  }
#endif

  p->pushtot+=amt;
  p->pushamt[p->pushcur]=amt;
  p->pushptr[p->pushcur++]=(void * *)ptr;
}

RADDEFFUNC void RADLINK pushmalloco(void* base, void * ptr,U64 amt)
{
  PM * p = (PM *)base;
  pushmalloc(base, ptr, amt);
  p->pushamt[p->pushcur-1]|=1;// mark it as special
}

RADDEFFUNC U64 RADLINK popmalloctotal( void* base )
{
  PM * p = (PM *)base;
  if ( p == 0 ) return 0;
  return( p->pushtot );
}

#ifdef SEPARATEMALLOCS

#ifdef __RADFINAL__
#error "You have separate mallocs turned on!"
#endif

#ifdef RADUSETM3
RADDEFFUNC void * RADLINK popmalloci(void* base, U64 amt, char const * info, U32 line)
#else
RADDEFFUNC void * RADLINK popmalloc(void* base, U64 amt)
#endif
{
  void * ptr;
  UINTa * table;
  U32 i;
  PM * p = (PM *)base;
  U32 extra;

  extra = sizeof(UINTa)+sizeof(void*)+sizeof(void*)+sizeof(UINTa);
  if ( p )
    extra += ( p->pushcur * sizeof(void*) );
  extra = (extra+15) & ~15;

  ptr=radmalloc(amt+extra);
  table = (UINTa *)ptr;
  table[ 0 ] = 1;
  table[ 1 ] = (UINTa)table;

  ptr = ( (U8 *) ptr ) + extra;
  ((UINTa*)ptr)[-1] = 0x97537531;
  ((UINTa*)ptr)[-2] = (UINTa)table;

  if ( p )
  {
    table[ 0 ] = 1 + p->pushcur;
    p->pushtot=0;
    for(i=0;i<p->pushcur;i++)
    {
      void * np;
#ifdef RADUSETM3
      np=radmalloci(p->pushamt[i]&~1,info,line);
#else
      np=radmalloc(p->pushamt[i]&~1);
#endif

      if ( p->pushamt[i]&1 )
        (*(void**)(((U8*)ptr)+((UINTa)p->pushptr[i])))=np;
      else
        (*(p->pushptr[i]))=np;

      table[ i + 1 ] = (UINTa)np;
    }
    table[ i + 1 ] = (UINTa)table;
    p->pushcur=0;
  }
  return(ptr);
}

RADDEFFUNC void RADLINK popfree(void * ptr)
{
  UINTa * table;
  UINTa i, n;
  if ( ((UINTa*)ptr)[-1] != 0x97537531 )
  {
     RR_BREAK();
     return;
  }
  ((UINTa*)ptr)[-1] = 0;
  table = ((UINTa**)ptr)[-2];
  n = table[0];
  table[0]=0;
  for(i=0;i<n;i++)
  {
    radfree( (void*)table[i+1] );
  }
}

#else

#ifdef RADUSETM3
RADDEFFUNC void * RADLINK popmalloci(void* base, U64 amt, char const * info, U32 line)
#else
RADDEFFUNC void * RADLINK popmalloc(void* base, U64 amt)
#endif
{
  PM * p = (PM *)base;
  void * ptr;

  amt=Round32(amt);

  if ( p == 0 )
  {
    return radmalloc( amt );
  }

#ifdef RADUSETM3
  ptr=radmalloci(p->pushtot+amt,info,line);
#else
  ptr=radmalloc(p->pushtot+amt);
#endif
  
  p->pushtot=0;
  if (ptr) {
    U32 i;
    U8 * np;

    np=((U8 *)ptr)+amt;
    for(i=0;i<p->pushcur;i++) 
    {
      if ( p->pushamt[i]&1 )
        (*(void**)(((U8*)ptr)+((UINTa)p->pushptr[i])))=np;
      else
        (*(p->pushptr[i]))=np;
      np=np+(p->pushamt[i]&~1);
    }


  }
  p->pushcur=0;
  return(ptr);
}

#endif

