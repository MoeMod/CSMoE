// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#ifndef __RADRR_COREH__
  #include "rrCore.h"
#endif
#include "radmath.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <mmsystem.h>

RADEXPFUNC U32 RADEXPLINK RADTimerRead(void)
{
  static U64 qpfirst;
  static U64 qpfreq;
  static S32 noprof=1;
  static U32 lasttime;
  static U32 lastrad;
  static U32 radadj=0;

  U64 count;
  if (noprof)
  {
    noprof=0;
    
    {
      char buf[ 16 ];
      if ( GetEnvironmentVariable( "RAD_USE_TGT", buf, 16 ) )
      {
        if ( buf[ 0 ] == '1' )
          goto tgt;
      }
    }
   
    if (QueryPerformanceFrequency((LARGE_INTEGER*)&qpfreq))
    {
      QueryPerformanceCounter((LARGE_INTEGER*)&qpfirst);
      lastrad=0;
      lasttime=GetTickCount();
      return( 0 );
    }
    else
    {
     tgt:
      radadj = timeGetTime();
      lastrad = 0;
      qpfreq=0;
    }
  }

  if (qpfreq)
  {
    U32 ret, time;
    S32 deltatime,deltarad;

    // read the new times...
    QueryPerformanceCounter((LARGE_INTEGER*)&count);
    time = GetTickCount();
    ret = radadj + ( (U32)(((count-qpfirst)*((LONGLONG)1000))/qpfreq) );

    // see how much each has moved
    deltatime = time - lasttime;
    deltarad = ret - lastrad;

    // check the query against GetTickCount to make sure it hasn't
    //   jumped ahead...

    if ( abs( deltatime-deltarad ) > 200 )
    {
      deltatime -= deltarad;
      radadj += deltatime;
      ret += deltatime;
    }

    // now check to see if it went backwards?
    if ( ( ( U32 ) ( ret - lastrad ) ) > 0xc0000000 )
    {
      // yup, just return the old timer value then
      return( lastrad );
    }

    lasttime = time;
    lastrad = ret;
    
    return( ret );
  }
  else
  {
    U32 ret;

    ret = timeGetTime() - radadj;

    // now check to see if it went backwards?
    if ( ( ( U32 ) ( ret - lastrad ) ) > 0xc0000000 )
    {
      // yup, just return the old timer value then
      return( lastrad );
    }
    
    lastrad = ret;

    return( ret );
  }
}
