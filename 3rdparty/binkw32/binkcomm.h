// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#ifndef __BINKCOMMH__
#define __BINKCOMMH__

#ifndef __RADRR_COREH__
  #include "rrCore.h"
#endif

// this ugliness is for cross platform alloca
#include <stdlib.h>
#ifndef alloca
#if defined(_MSC_VER)
  #include <malloc.h>
#elif defined(__GNUC__)
  #include <alloca.h>
#endif
#endif
#define radalloca(amt) ((void*)((((UINTa)alloca((amt)+15))+15)&~15))

#include "bink.h"


//blocks: S=skip, P=pattern, O=original, 1=1color, 2=2color, M=motioned, Z=Zero, U=multires

#define BTS  0
#define BTU  1
#define BTZ  2

#define BTP  3
#define BTM  4
#define BTD  5

#define BT1  6
#define BTMD 7
#define BT2  8
#define BTO  9

#define BTLAST 9
#define BTESCAPE 15

// sound format macros

#define GETTRACKFORMAT(val) (((val)>>31) || (((val)>>28)&1))
#define GETTRACKFREQ(val) ((val)&0xffff)
#define GETTRACKBITS(val) (((((val)>>30)&1)<<3)+8)
#define GETTRACKCHANS(val) ((((val)>>29)&1)+1)
#define GETTRACKNEWFORMAT(openflags,val) ((openflags&BINKAUDIO2)?BINKAC20:(((((val)>>31)==0) && (((val)>>28)&1))))

U32 get_slice_range( BINKSLICES * slices, U32 per,U32 perdiv );

#ifdef INC_BINK2
  void clear_seam( BINKSLICES * slices, void * seam );
#else
  #define clear_seam(...)
#endif

typedef void RADLINK BinkFreeGlobalsCallback( void );

// define to force markers to be inserted in the bitstream
// #define INSERTMARKERS

#endif
