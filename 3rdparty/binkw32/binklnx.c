// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#include <time.h>

#include "egttypes.h"

RADEXPFUNC U32 RADEXPLINK RADTimerRead( void )
{
  struct timespec ts;
  clock_gettime( CLOCK_MONOTONIC, &ts );
  return (U32) ( ( ((U64)(U32)ts.tv_sec) * 1000LL ) + (U64)(((U32)ts.tv_nsec+500000)/1000000) );
}

