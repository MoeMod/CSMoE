// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#ifndef __RADRTL_THREADS2_H__
#define __RADRTL_THREADS2_H__

#include "rrCore.h"

#define rrThreadVersion 23
#define rrTVN( name ) RR_STRING_JOIN( name, rrThreadVersion )

RADDEFSTART

struct rriThread2;
struct rriMutex2;
struct rriEvent2;
struct rriSemaphore2;

typedef struct rrThread2
{
#if defined(__RADBIGTHREADSTRUCT__)
  U8 data[ 512 - sizeof( void* ) ];
#else
  U8 data[ 256 - sizeof( void* ) ];
#endif
  struct rriThread2 * i;
} rrThread2;

typedef struct rrMutex2
{
#if defined(__RADMAC__) || defined(__RADIOS__) || defined(__RADLINUX__)
  U8 data[ 192 - sizeof( void* ) ];
#else
  U8 data[ 128 - sizeof( void* ) ];
#endif
  struct rriMutex2 * i;
} rrMutex2;

typedef struct rrSemaphore2
{
#if defined(__RADMAC__) || defined(__RADIOS__) 
  U8 data[ 192 - sizeof( void* ) ];
#else
  U8 data[ 128 - sizeof( void* ) ];
#endif
  struct rriSemaphore2 * i;
} rrSemaphore2;

#define rriThread    rriThread2
#define rriMutex     rriMutex2
#define rriSemaphore rriSemaphore2

#define rrThread    rrThread2
#define rrMutex     rrMutex2
#define rrSemaphore rrSemaphore2

// type of function for user thread main
typedef U32 (RADLINK thread_function2)( void * user_data );


#define RR_WAIT_INFINITE (~(U32)(0))

// values that can be used for thread priority, 0 means do nothing
struct rrThreadPriority2s; 
typedef struct rrThreadPriority2s * rrThreadPriority2;
#define rrThreadLowest               ((rrThreadPriority2)(SINTa)0x1)
#define rrThreadLow                  ((rrThreadPriority2)(SINTa)0x2)
#define rrThreadNormal               ((rrThreadPriority2)(SINTa)0x3)
#define rrThreadHigh                 ((rrThreadPriority2)(SINTa)0x4)
#define rrThreadHighest              ((rrThreadPriority2)(SINTa)0x5)
#define rrThreadSameAsCalling        ((rrThreadPriority2)(SINTa)0x6) //set to calling thread level (which is the thread creation default)

// or you can make a priority higher or lower with rrThreadPriorityAdd directly...
#define rrThreadOneLowerThanCalling  rrThreadPriorityAdd(rrThreadSameAsCalling,-1)
#define rrThreadOneHigherThanCalling rrThreadPriorityAdd(rrThreadSameAsCalling, 1)

// or you can even specify native platform priorities with this macro/function:
#define rrThreadNativePriority(val) rrThreadNativePriorityHelper( (SINTa)(val) );


struct rrThreadCoreIndexs; 
typedef struct rrThreadCoreIndexs * rrThreadCoreIndex;

// core assignment flags, 0 means don't set
//   use these two macros to specify a specify core:
#define rrThreadCore(v) ((rrThreadCoreIndex)((((UINTa)(v))<<8)+0xf0)) // just use a specific hardware core
#define rrThreadCoreByPrecedence(v) ((rrThreadCoreIndex)((((UINTa)(v))<<16)+0xfff0)) // order: real cores (in reverse), then hyperthread cores

// wrap a rrThreadCore or rrThreadCoreByPrecedence with this macro, and it will use a hard afffinity, if the platform has it
#define rrThreadHardAffinity(v) ((rrThreadCoreIndex)(((UINTa)(v))|(SINTa)0xf2)) // 2 is the hard flag


#if defined(__RADCANALLOCTHREADSTACK__)
// this really means *must* allocate stack
#define RR_CAN_ALLOC_THREAD_STACK  
#endif


// mutex type - this really only matters on Windows before Vista now
#define rrMutexFullLocksOnly        0  // don't need timeouts with this mutex
#define rrMutexDontNeedTimeouts     rrMutexFullLocksOnly
#define rrMutexNeedOnlyZeroTimeouts 1  // only need to be able to call rrMutexLockTimeout with 0 ms (test - usually fast)
#define rrMutexNeedFullTimeouts     2  // need to be able to call rrMutexLockTimeout with any ms (usually system call)

#ifndef RR_THREAD_TYPES_ONLY

#if defined(RR_CAN_ALLOC_THREAD_STACK)
#define rrThreadCreateWithStack rrTVN(rrThreadCreateWithStack)
#else
#define rrThreadCreate rrTVN(rrThreadCreate)
#endif
#define rrThreadDestroy rrTVN(rrThreadDestroy)

#define rrMutexCreate rrTVN(rrMutexCreate)
#define rrMutexLock rrTVN(rrMutexLock)
#define rrMutexLockTimeout rrTVN(rrMutexLockTimeout)
#define rrMutexUnlock rrTVN(rrMutexUnlock)
#define rrMutexDestroy rrTVN(rrMutexDestroy)

#define rrSemaphoreCreate rrTVN(rrSemaphoreCreate)
#define rrSemaphoreDestroy rrTVN(rrSemaphoreDestroy)
#define rrSemaphoreDecrementOrWait rrTVN(rrSemaphoreDecrementOrWait)
#define rrSemaphoreIncrement rrTVN(rrSemaphoreIncrement)

#define rrThreadSleep rrTVN(rrThreadSleep)
#define rrThreadSpinHyperYield rrTVN(rrThreadSpinHyperYield)
#define rrThreadYieldToAny rrTVN(rrThreadYieldToAny)
#define rrThreadYieldNotLower rrTVN(rrThreadYieldNotLower)

#define rrThreadGetOSThreadID rrTVN(rrThreadGetOSThreadID)
#define rrThreadNativePriorityHelper rrTVN(rrThreadNativePriorityHelper)
#define rrThreadPriorityAdd rrTVN(rrThreadPriorityAdd)
#define rrThreadSetCores rrTVN(rrThreadSetCores)

// create a new thread
// NOTE: stack_size must not be 0 and will trigger an assert otherwise
// 
// If you want to set the stack to something that is based on OS specific minimums, here's
// the current information as of 9/19/2012:
//
// Windows: Threads inherit stack size of parent executable unless otherwise specified.  Default
//        is 1MB unless overridden by /STACKSIZE.  Granularity is typically 64K.
// iOS:   Primary thread has 1MB stack by default, child threads are 512K.
// OS X:  pthread_attr_setstacksize must be at least PTHREAD_STACK_MIN (defined in <limits.h> and
//        currently 8K).  Default process stack size is 8MB, but child threads have a default stack
//        size of 512K (using pthreads) or 4K (using Carbon MPTasks).
// Linux: pthread_attr_setstacksize must be at least PTHREAD_STACK_MIN (16K) and on some systems
//        it must be a multiple of the system page size.  The default stack on Linux/x86-32 is 2MB,
//        and the max stack space is 8MB typically (verify with ulimit -s).

#if defined(RR_CAN_ALLOC_THREAD_STACK)

    //
    // Some platforms *require* that the stack be allocated by the user and passed to the 
    // thread creation.
    //
    RADDEFFUNC rrbool RADLINK rrThreadCreateWithStack(
                                                rrThread* rr, 
                                                thread_function2* function, 
                                                void* user_data, 
                                                U32 stack_size,
                                                void * stack_ptr,
                                                rrThreadPriority2 priority,   // to do nothing, pass 0
                                                rrThreadCoreIndex core_index, // to do nothing, pass 0
                                                char const * name
                                                );

#else // RR_CAN_ALLOC_THREAD_STACK

    RADDEFFUNC rrbool RADLINK rrThreadCreate(
                                          rrThread * rr, 
                                          thread_function2 * function,  
                                          void * user_data,
                                          U32 stack_size,
                                          rrThreadPriority2 priority,   // to do nothing, pass 0
                                          rrThreadCoreIndex core_index, // to do nothing, pass 0
                                          char const * name
                                          );

#endif // RR_CAN_ALLOC_THREAD_STACK


// destroys the thread - it will wait until the thread has exited and closes handles to the os
//   returns the value returned from the user thread function
RADDEFFUNC U32 RADLINK rrThreadDestroy( rrThread * handle );

// Mutex stuff
RADDEFFUNC rrbool RADLINK rrMutexCreate( rrMutex * rr, S32 mutex_type );
RADDEFFUNC void   RADLINK rrMutexLock( rrMutex * handle);
RADDEFFUNC rrbool RADLINK rrMutexLockTimeout( rrMutex * handle, U32 ms );
RADDEFFUNC void   RADLINK rrMutexUnlock( rrMutex * handle );
RADDEFFUNC void   RADLINK rrMutexDestroy( rrMutex * handle );


// Semaphore stuff
RADDEFFUNC rrbool RADLINK rrSemaphoreCreate( rrSemaphore * rr, S32 initialCount, S32 maxCount );
RADDEFFUNC void   RADLINK rrSemaphoreDestroy( rrSemaphore * rr );
RADDEFFUNC rrbool RADLINK rrSemaphoreDecrementOrWait( rrSemaphore * rr, U32 ms );
RADDEFFUNC void   RADLINK rrSemaphoreIncrement( rrSemaphore * rr, S32 cnt );


// Sleep stuff

// sleeps for the specified number of ms
//   behavior Sleep(0) is undefined and may vary by platform - don't do it
RADDEFFUNC void RADLINK rrThreadSleep( U32 ms );

// rrThreadSpinHyperYield should be called in spin loops to yield hyper-threads
RADDEFFUNC void RADLINK rrThreadSpinHyperYield( void );

// adds to native priorities
RADDEFFUNC rrThreadPriority2 RADLINK rrThreadNativePriorityHelper( SINTa val );
RADDEFFUNC rrThreadPriority2 RADLINK rrThreadPriorityAdd( rrThreadPriority2 v, S32 upordown ); // upordown: pos = higher, neg = lower

// optional function to set affinities across cores
RADDEFFUNC void RADLINK rrThreadSetCores( rrThread * thread, U64 bitmask );


// functions to operate on the native thread handles

// returns the thread id on windows, pthread_t on mac, etc...
RADDEFFUNC void RADLINK rrThreadGetOSThreadID( rrThread * thread, void * out_ph );

#endif // RR_THREADS_TYPES_ONLY

RADDEFEND

// @cdep pre $rrThreadsAddSource

#endif
