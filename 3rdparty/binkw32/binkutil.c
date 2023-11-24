// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#include "binkproj.h"

#include "rrCpu.h"
#include "rrThreads2.h" // @cdep ignore 
#include "popmal.h"

RADEXPFUNC U32 RADEXPLINK BinkUtilCPUs(void)
{
  return rrGetTotalCPUs();
}

RADEXPFUNC S32 RADEXPLINK BinkUtilMutexCreate(rrMutex* rr,S32 need_timeout)
{
  return rrMutexCreate( rr, (need_timeout) ? rrMutexNeedFullTimeouts : rrMutexFullLocksOnly );
}

RADEXPFUNC void RADEXPLINK BinkUtilMutexDestroy(rrMutex* rr)
{
  rrMutexDestroy( rr );
}

RADEXPFUNC void RADEXPLINK BinkUtilMutexLock(rrMutex* rr)
{
  rrMutexLock( rr );
}

RADEXPFUNC void RADEXPLINK BinkUtilMutexUnlock(rrMutex* rr)
{
  rrMutexUnlock( rr );
}

RADEXPFUNC S32 RADEXPLINK BinkUtilMutexLockTimeOut(rrMutex* rr, S32 timeout)
{
  return rrMutexLockTimeout( rr, timeout );
}
