// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#include "rrCpu.h"
#include <unistd.h>
#include <pthread.h>
#include <sched.h>

#ifdef __RADQNX__
#include <sys/syspage.h>
#endif

#if defined(__RADMAC__)|| defined(__RADIPHONE__)
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

#if defined(__RADEMSCRIPTEN__)
#include <emscripten/threading.h>
#endif

static S32 cpus = -1;

RADDEFFUNC S32 RADLINK rrGetTotalCPUs( void )
{
  if ( cpus == -1 )
  {
    #if defined(__RADANDROID__)
      cpus = sysconf(_SC_NPROCESSORS_ONLN);
    #elif defined(__RADQNX__)
      cpus = _syspage_ptr->num_cpu;
    #elif defined(__RADIPHONE__)
      size_t count;
      int mib[4];
      size_t len = sizeof(count); 
      mib[0] = CTL_HW;
      mib[1] = HW_AVAILCPU; 
      sysctl(mib, 2, &count, &len, NULL, 0);
      if( count < 1 ) 
      {
        mib[1] = HW_NCPU;
        sysctl( mib, 2, &count, &len, NULL, 0 );
        if( count < 1 )
          count = 1;
      }  
      cpus = (int) count;
    #elif defined(__RADMAC__)
      int count;
      size_t size = sizeof(count);
      if (sysctlbyname("hw.ncpu", &count, &size, NULL, 0))
        count = 1;
      cpus = count;
    #elif defined(__RADLINUX__)
      cpus = sysconf(_SC_NPROCESSORS_CONF);
    #elif defined(__RADEMSCRIPTEN__)
      cpus = emscripten_num_logical_cores();
    #else
      #error posix rrcpus problem
    #endif

    if ( cpus > RR_MAX_CPUS )
      cpus = RR_MAX_CPUS;
    if ( cpus <= 0 )
      cpus = 1;
  }

  return cpus;
}

RADDEFFUNC S32 RADLINK rrGetSpinCount( void )
{
  if ( rrGetTotalCPUs() == 1 )
    return 1;
  else
    return 500;
}

