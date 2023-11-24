// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#include "binkproj.h"

void radmemset16(void* dest,U16 value,U32 size)
{
  U32 v=((U32)value)|(((U32)value)<<16);
  U32*d=(U32*)dest;
  U32 s=size/2;
  while(s--)
  {
    *d++=v;
  }
  if (size&1)
    *((U16*)d)=(U16)v;
}

#include <mach/mach.h>
#include <mach/mach_time.h>

U32 RADTimerRead()
{
    static mach_timebase_info_data_t timebase;
    static U64 starttime = 0;
    if (starttime == 0)
    {
        mach_timebase_info(&timebase);
        starttime = mach_absolute_time();
    }

    U64 time = mach_absolute_time();
    time -= starttime;
    
    time = (time * timebase.numer) / timebase.denom;
    time /= 1000000; // convert to microseconds.
    return (U32)time;
}
