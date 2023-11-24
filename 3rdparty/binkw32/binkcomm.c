// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#include "binkcomm.h"

#include "bink.h"

#include "cpu.h"

#include "rrAtomics.h"

#define SEGMENT_NAME "comm"
#include "binksegment.h"

U32 get_slice_range( BINKSLICES * slices, U32 per,U32 perdiv )
{
  U32 s, r;

  r = ((slices->slice_count*per)+(perdiv-1))/perdiv;
  if ( r == 0 )
    r = 1;

  s = rrAtomicAddExchange32( &slices->slice_inc, r );

  if ( s >= slices->slice_count )
    return 0;

  if ( ( s + r ) > slices->slice_count )
    r = slices->slice_count - s;

  return s | ( r << 4 );
}

#ifdef INC_BINK2
  void clear_seam( BINKSLICES * slices, void * seam )
  {
    if (seam)
    {
      U32 i;
      U8 * ptr = (U8*)seam;
      for( i = 0 ; i < ( slices->slice_count - 1 ) ; i++ )
      {
        ((U32*)ptr)[0]=0;
        ptr += slices->slice_pitch;
      }
    }
  }
#endif


#ifdef INC_BINK2
#ifndef __RADFINAL__
#define STACHE_INCLUDE_IMPLEMENTATION 1

#include "mymoustache.h"
#endif
#endif
