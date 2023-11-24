// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

// thread functions for Linux
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "rrThreads2.h" 
#include "rrAtomics.h"
#include "rrCpu.h"

#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sched.h>
#if defined __RADQNX__ || defined __RADEMSCRIPTEN__
#include <errno.h>
#else
#include <sys/errno.h>
#include <sys/syscall.h>
#endif
#include <sys/time.h>
#include <sys/resource.h>

#if defined(__RADANDROID__) && __ANDROID_API__ < 21
#define RAD_OLD_ANDROID 1
#endif

#ifdef RAD_OLD_ANDROID
// expect HAVE_PTHREAD_COND_TIMEDWAIT_MONOTONIC
// pthread_cond_timedwait_monotonic_np
// addition in BIONIC , now deprecated in Android >= 21 and all AArch64

// NOTE(fg): it gets even more fun
// Google accidentally removed this for old API levels in NDK rels 15 and 15b,
// then put it back in 15c, then accidentally (?) removed it again in 16:
//
//   https://stackoverflow.com/questions/44580542
//
// so, er, going with the original patch:
//
//   https://android-review.googlesource.com/c/platform/bionic/+/420945
//
// and the suggestion from the StackOverflow thread, declare it ourselves.
#ifndef HAVE_PTHREAD_COND_TIMEDWAIT_MONOTONIC
int pthread_cond_timedwait_monotonic_np(pthread_cond_t*, pthread_mutex_t*, const struct timespec*);
#endif
#endif

typedef struct rriThread
{
  thread_function2 * function;
  void * user_data;
  char short_name[32];
  pthread_t handle;
  int pri;
  U32 ret;
  U32 core_num;
  S32 hard_affin; // not set on posix platforms
#ifdef STADIA
  int id;
#endif
} rriThread;

// =================
//   priotity stuff
#if defined(__RADMAC__) || defined(__RADIPHONE__)

// macos lets you have higher and lower priorities with no special rights

#include <mach/thread_policy.h>
#include <mach/thread_act.h>

static int low_pri, high_pri;
static int tinit = 0;

static void init_priority_stuff()
{
  if (!tinit)
  {
    tinit = 1;
    low_pri = sched_get_priority_min( SCHED_OTHER );
    high_pri = sched_get_priority_max( SCHED_OTHER );
  }
}

static int getcurpri()
{
  struct sched_param sp={0};
  int policy = 0;

  init_priority_stuff();
  if ( pthread_getschedparam( pthread_self(), &policy, &sp ) )
    sp.sched_priority = (low_pri+high_pri+1)/2;

  return sp.sched_priority;
}

static void set_current_thread_priority( int pri )
{
  struct sched_param sp;
  sp.sched_priority = pri;
  pthread_setschedparam( pthread_self(), SCHED_OTHER, &sp );
}

static void set_current_thread_core( U32 core_num )
{
  // get the thread on a reasonable cpu
  struct thread_affinity_policy mypolicy = { core_num };

  // this call is just a hint - it can still move around
  thread_policy_set(
    pthread_mach_thread_np( pthread_self() ),
    THREAD_AFFINITY_POLICY,
    (thread_policy_t) &mypolicy,
    1
  );
}

#define PLATFORM_HIGHEST_PRIORITY high_pri
#define PLATFORM_LOWEST_PRIORITY  low_pri
#define PLATFORM_GET_CURRENT_THREAD_PRIORITY (getcurpri())

#elif defined(__RADLINUX__)

// linux lets you set lower priorities, but higher priority require superuser

#define PLATFORM_HIGHEST_PRIORITY -20
#define PLATFORM_LOWEST_PRIORITY  19
#define PLATFORM_GET_CURRENT_THREAD_PRIORITY (getpriority(PRIO_PROCESS,syscall(SYS_gettid)))
#define set_current_thread_priority( pri ) setpriority( PRIO_PROCESS, syscall(SYS_gettid), pri )
#define init_priority_stuff()

static void set_current_thread_core( U32 core_num )
{
  cpu_set_t set;
 
  // get the thread on a reasonable cpu
  CPU_ZERO( &set );
  CPU_SET( core_num, &set );
  pthread_setaffinity_np( pthread_self(), sizeof( cpu_set_t ), &set );
}

#elif defined(__RADANDROID__) || defined(__RADQNX__)

// linux lets you set lower priorities, but higher priority require superuser

#define PLATFORM_HIGHEST_PRIORITY -20
#define PLATFORM_LOWEST_PRIORITY  19
#define PLATFORM_GET_CURRENT_THREAD_PRIORITY (getpriority(PRIO_PROCESS,gettid()))
#define set_current_thread_priority( pri ) setpriority( PRIO_PROCESS, gettid(), pri )
#define init_priority_stuff()

static void set_current_thread_core( U32 core_num )
{
#ifndef __RADQNX__ // Qnx doesn't provide this function
  int mask = 1 << core_num;
  syscall(__NR_sched_setaffinity, gettid(), sizeof(mask), &mask);
#endif
}

#elif defined(__RADEMSCRIPTEN__)

#define PLATFORM_HIGHEST_PRIORITY -20
#define PLATFORM_LOWEST_PRIORITY  19
#define PLATFORM_GET_CURRENT_THREAD_PRIORITY 0
#define set_current_thread_priority( pri ) 
#define init_priority_stuff()

static void set_current_thread_core( U32 core_num )
{
}

#else

// when porting, you can just start with do nothing macros here...
#error platform priorities macros need to be defined

#endif

#include "rrthreads2_priorities.inl"
// =================



static void * rad_thread_shim( void * param )
{
  rrThread * rr;
  rriThread * t;

  rr = (rrThread*) param;
  t = rr->i;

#ifdef STADIA
  t->id = syscall(SYS_gettid);
#endif

  // set the priority and core if they asked
  if ( is_good_platform_priority( t->pri ) )
    set_current_thread_priority( t->pri );
  if ( is_good_core_num( t->core_num ) )
    set_current_thread_core( t->core_num );

  // run the user thread
  t->ret = t->function( t->user_data );

  return 0;
}

RADDEFFUNC rrbool RADLINK rrThreadCreate(
                                      rrThread * rr, 
                                      thread_function2 * function,  
                                      void * user_data,
                                      U32 stack_size,
                                      rrThreadPriority2 priority,   // to do nothing, pass 0
                                      rrThreadCoreIndex core_index, // to do nothing, pass 0
                                      char const * name
                                      )
{
  rriThread * thread;
  pthread_attr_t tattr;
  int ret;

  RR_ASSERT_ALWAYS_NO_SHIP( stack_size != 0 );

  init_priority_stuff();

  thread = (rriThread*) ( ( ( (UINTa) rr->data ) + 15 ) & ~15 );
  
  // setup the info
  rr->i = thread;
  thread->function = function;
  thread->user_data = user_data;
  limit_thread_name( thread->short_name, sizeof(thread->short_name), name );
  thread->pri = rad_to_platform_pri( priority ); // set in the thread itself
  rad_to_platform_core( core_index, &thread->core_num, &thread->hard_affin );

#ifdef STADIA
  stack_size = ( stack_size + (1024*1024-1) ) & ~(1024*1024-1);
#endif
 
  pthread_attr_init( &tattr );
  pthread_attr_setstacksize( &tattr, stack_size );
  ret = pthread_create( &thread->handle, &tattr, rad_thread_shim, rr );
  pthread_attr_destroy( &tattr );

  if ( ret )
    return 0;

  return 1;
}


// waits for the thread to shutdown and return
RADDEFFUNC U32 RADLINK rrThreadDestroy( rrThread * rr )
{
  rriThread * thread;
  void * ret;

  thread = rr->i;
  
  pthread_join( thread->handle, &ret );

  return thread->ret;
}


RADDEFFUNC void RADLINK rrThreadGetOSThreadID( rrThread * rr, void * out )
{
  rriThread * thread;
  thread = rr->i;
#ifdef STADIA
  ((int*)out)[ 0 ] = thread->id;
#else
  ((pthread_t*)out)[ 0 ] = thread->handle;
#endif
}


typedef struct timespec WaitUntilTimeType;

static void GetWaitTime( WaitUntilTimeType * time, U32 ms )
{
  U64 nanos;

  #if defined(__RADMAC__) || defined(__RADIPHONE__)
    time->tv_sec=0; time->tv_nsec=0;
  #else   
    clock_gettime(CLOCK_MONOTONIC,time);
  #endif

  nanos = (U64) time->tv_nsec + (U64) ms * 1000 * 1000;

  while ( nanos >= 1000000000 )
  {
    nanos -= 1000000000;
    time->tv_sec++;
  }
  time->tv_nsec = (long)nanos;
 }   


//=======================================================

// CB 09-12-2016 : rriCondVar - internal CondVar used by Mutex & Semaphore

typedef struct rriCondVar
{
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
} rriCondVar;

static rrbool  rriCondVarCreate( rriCondVar * p )
{
    int err;

    /*
    pthread_mutexattr_t mattr;
    pthread_mutexattr_init( &mattr );
    //pthread_mutexattr_settype( &mattr, PTHREAD_MUTEX_NORMAL ); // non-recursive !!
    pthread_mutexattr_settype( &mattr, PTHREAD_MUTEX_ERRORCHECK ); // non-recursive !!
    //perhaps use PTHREAD_MUTEX_ERRORCHECK  in debug builds
    */
  
    err = pthread_mutex_init(&(p->mutex),NULL /*attributes*/);
    RR_ASSERT( err == 0 ); // temp
    if ( err != 0 )
        return 0;

//#ifdef __RADLINUX__
#if !defined(RAD_OLD_ANDROID) && !defined(__RADMAC__) && !defined(__RADIPHONE__)
    pthread_condattr_t cattr;
    pthread_condattr_init(&cattr);
    clockid_t cid = CLOCK_MONOTONIC;
    err = pthread_condattr_setclock(&cattr,cid);
    RR_ASSERT( err == 0 ); // temp
    if ( err != 0 )
        return 0;
        
    err = pthread_cond_init(&(p->cond),&cattr);
    pthread_condattr_destroy(&cattr);
#else

    // RAD_OLD_ANDROID doesn't have condattr_setclock
    //  but it does have timedwait_monotonic_np
    //  so we will still use the MONOTONIC clock

    err = pthread_cond_init(&(p->cond),NULL);
#endif

    RR_ASSERT( err == 0 ); // temp
    if ( err != 0 )
    {
        pthread_mutex_destroy(&(p->mutex));
        return 0;
    }

    return 1;
}

static void   rriCondVarDestroy( rriCondVar * p )
{
    pthread_mutex_destroy(&(p->mutex));
    pthread_cond_destroy(&(p->cond));
}

static void   rriCondVarLock( rriCondVar * p )
{
    int ret = pthread_mutex_lock( &p->mutex );  
    RR_ASSERT( ret == 0 );
    RR_UNUSED_VARIABLE(ret);
}

static void  rriCondVarUnlock( rriCondVar * p )
{
    int ret = pthread_mutex_unlock( &p->mutex );    
    RR_ASSERT( ret == 0 );
    RR_UNUSED_VARIABLE(ret);
}

static void rriCondVarUnlockWaitLock( rriCondVar * p )
{
    // no timeout

    int ret = pthread_cond_wait(&(p->cond),&(p->mutex));
    RR_ASSERT( ret == 0 );
    RR_UNUSED_VARIABLE(ret);

  // I got a signal
    // unlike other pthread waits, cond var does not break
    //  out of the wait on interrupts (never returns EINTR)

}


// returns 1 if signalled, 0 if timed out
static rrbool rriCondVarUnlockWaitUntilLock( rriCondVar * p, WaitUntilTimeType * waitUntil )
{
    int ret;

    #ifdef __RADMAC__
    ret = pthread_cond_timedwait_relative_np(&(p->cond),&(p->mutex),waitUntil);
    #elif defined(RAD_OLD_ANDROID)
    ret = pthread_cond_timedwait_monotonic_np(&(p->cond),&(p->mutex),waitUntil);
    #else
    ret = pthread_cond_timedwait(&(p->cond),&(p->mutex),waitUntil);
    #endif
    
    if ( ret == ETIMEDOUT )
    {
        // note mutex lock is still reacquired here
        return 0; // expected failure due to timeout
    }

    // I got a signal
    // unlike other pthread waits, cond var does not break
    //  out of the wait on interrupts (never returns EINTR)

    RR_ASSERT( ret == 0 );
    return 1;
}

static void rriCondVarSignalUnlock( rriCondVar * p )
{
    int ret = pthread_cond_signal( &p->cond );  
    RR_ASSERT( ret == 0 );
    
    ret = pthread_mutex_unlock( &p->mutex );    
    RR_ASSERT( ret == 0 );
    RR_UNUSED_VARIABLE(ret);
}
 
static void rriCondVarBroadcastUnlock( rriCondVar * p )
{
    int ret = pthread_cond_broadcast( &p->cond );   
    RR_ASSERT( ret == 0 );
    
    ret = pthread_mutex_unlock( &p->mutex );    
    RR_ASSERT( ret == 0 );
    RR_UNUSED_VARIABLE(ret);
}


//================================================================

// a few options to identify current thread :

#if defined(__RADANDROID__) 

//gettid() is defined in <unistd.h> in Bionic.

#define MUTEX_TID_TYPE  pid_t   
#define MUTEX_TID_GET() gettid()

#elif defined(__RADQNX__)

#define MUTEX_TID_TYPE  pid_t   
#define MUTEX_TID_GET() gettid()
#define USE32BITATOMICS

#elif defined(__RADMAC__) 

#define MUTEX_TID_TYPE  mach_port_t            
#define MUTEX_TID_GET() pthread_mach_thread_np(pthread_self())

#elif defined(__RADLINUX__) 

// gettid for Linux - NOT BSD/Apple

#define MUTEX_TID_TYPE  long            
//#define MUTEX_TID_GET()   syscall(224)
#define MUTEX_TID_GET() syscall(SYS_gettid)

// __NR_gettid
// SYS_gettid

#ifdef __RADARM__ // don't have 64-bit locks for arm linux yet
#define USE32BITATOMICS
#endif

#elif 0  // some linuxes...

// getthreadid apparently exists in some unixes and is faster than pthread_self
#define MUTEX_TID_TYPE  pthread_id_np_t 
#define MUTEX_TID_GET() pthread_getthreadid_np()

#elif defined(__RADEMSCRIPTEN__)

#define MUTEX_TID_TYPE  pthread_t
#define MUTEX_TID_GET() pthread_self()
#define USE32BITATOMICS

#else

// pthread_self is the most portable way (any POSIX) but may be slow

#define MUTEX_TID_TYPE  pthread_t
#define MUTEX_TID_GET() pthread_self()

#endif

typedef struct rriMutex
{
    rriCondVar  cv;
    #ifdef USE32BITATOMICS
    U32       atomic_owner_and_flag; // this is the mutex gate ; atomic!
    #else
    U64       atomic_owner_and_flag; // this is the mutex gate ; atomic!
    #endif
    U32         count;  // recursion count - protected by this mutex
    U32         type;
} rriMutex;

typedef struct rriSemaphore
{
    rriCondVar  cv;
    U32         count;
    U32         type;
} rriSemaphore;

#include "primativesoncondvars.inl"

//=============================================================================

RADDEFFUNC void RADLINK rrThreadSleep( U32 ms )
{
  usleep( ms * 1000 );
}

#if defined __RADARM__ || defined __EMSCRIPTEN__

RADDEFFUNC void RADLINK rrThreadSpinHyperYield( void )
{
  ;
}

#elif defined __RADX86__ || defined __RADX64__ 

RADDEFFUNC void         RADLINK rrThreadSpinHyperYield( void )
{
  // on hyperthreaded procs need this :
  //  benign on non-HT
  __asm__("pause");
}

#else 

#error rrThreadSpinHyperYield not implemented

#endif

RR_COMPILER_ASSERT( ( sizeof ( rrThread ) >= ( sizeof( rriThread ) + sizeof(void*) + 15 ) ) );  // make sure our rrthread isn't too big
RR_COMPILER_ASSERT( ( sizeof ( rrMutex ) >= ( sizeof( rriMutex ) + sizeof(void*) + 15 ) ) );  // make sure our rrmutex isn't too big
RR_COMPILER_ASSERT( ( sizeof ( rrSemaphore ) >= ( sizeof( rriSemaphore ) + sizeof(void*) + 15 ) ) );
