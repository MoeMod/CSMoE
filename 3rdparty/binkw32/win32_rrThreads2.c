// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

// thread functions for Win32, Win64, and consoles

#include "rrcore.h" 
#include "rrThreads2.h" 
#include "rrAtomics.h"
#include "rrCPU.h"

#if defined(__RADPPC__)
  
  #undef S8
  #define NOD3D
  #include <xtl.h>
  #include <PPCIntrinsics.h>
  #define S8 signed char
  #define _T(x) x

#elif defined(__RADCONSOLE__)

   #define _WIN32_WINNT 0x0A00
   #include <windows.h>
   #include <synchapi.h>
   #include <processthreadsapi.h>
   #include <libloaderapi.h>

#else  // Windows 32 or 64
  #pragma warning(disable : 4201)
  
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #include <tchar.h>
  #include <windows.h>
  #include <mmsystem.h>
#endif

#if defined( __RADNT__)
#define HAS_OPTIONAL_CONDITION_VARS
#elif (defined(__RADCONSOLE__) || defined(WINDOWS_UWP) ) && !defined( __RADPPC__ )
#define HAS_NATIVE_CONDITION_VARS
#define NEW_WINDOWS_APIS
#endif   

#if defined(WINDOWS_UWP) || defined( __RADPPC__ )
  #define NO_ADD_VECTORED_EXCEPTION
#else 
  #define HAS_AFFINITY_MASK
#endif

typedef BOOL WINAPI InitializeCriticalSectionAndSpinCountFunc(
  LPCRITICAL_SECTION lpCriticalSection,
  DWORD dwSpinCount
);

typedef BOOL WINAPI TryEnterCriticalSectionFunc(
  LPCRITICAL_SECTION lpCriticalSection
);

typedef DWORD WINAPI SetThreadIdealProcessorFunc(
  HANDLE hThread,
  DWORD dwIdealProcessor
);

typedef BOOL WINAPI IsDebuggerPresentFunc( void );

typedef HRESULT WINAPI SetThreadDescriptionFunc(
  HANDLE hThread,
  PCWSTR lpThreadDescription
);

#if defined(HAS_OPTIONAL_CONDITION_VARS)

   // VC6:
   #if defined(_MSC_VER) && _MSC_VER <= 1200
   typedef struct _RTL_CONDITION_VARIABLE {                    
           PVOID Ptr;                                       
   } RTL_CONDITION_VARIABLE, *PRTL_CONDITION_VARIABLE;      
   #define RTL_CONDITION_VARIABLE_INIT {0}                 
   #define RTL_CONDITION_VARIABLE_LOCKMODE_SHARED  0x1     

   typedef RTL_CONDITION_VARIABLE CONDITION_VARIABLE, *PCONDITION_VARIABLE;
   #endif

typedef BOOL WINAPI SleepConditionVariableCSFunc(
  PCONDITION_VARIABLE ConditionVariable,
  PCRITICAL_SECTION   CriticalSection,
  DWORD               dwMilliseconds
);

typedef VOID WINAPI InitializeConditionVariableFunc(
  PCONDITION_VARIABLE ConditionVariable
);

typedef VOID WINAPI WakeConditionVariableFunc(
  PCONDITION_VARIABLE ConditionVariable
);

typedef VOID WINAPI WakeAllConditionVariableFunc(
  PCONDITION_VARIABLE ConditionVariable
);

static SleepConditionVariableCSFunc * ourSleepConditionVariableCS = 0;
static InitializeConditionVariableFunc * ourInitializeConditionVariable = 0;
static WakeConditionVariableFunc * ourWakeConditionVariable = 0;
static WakeAllConditionVariableFunc * ourWakeAllConditionVariable = 0;

static void init_cond_var_funcs( void )
{
  if ( ourSleepConditionVariableCS == 0 )
  {
    HANDLE kernel32Handle = GetModuleHandleA("KERNEL32");
    if (kernel32Handle == 0)
      goto err;

    ourSleepConditionVariableCS = (SleepConditionVariableCSFunc*) GetProcAddress( kernel32Handle, "SleepConditionVariableCS" );
    if ( ourSleepConditionVariableCS == 0 )
    {
      err:
      ourSleepConditionVariableCS = (SleepConditionVariableCSFunc*) (UINTa) 1;
      return;
    }
    ourInitializeConditionVariable = (InitializeConditionVariableFunc*) GetProcAddress( kernel32Handle, "InitializeConditionVariable" );
    if ( ourInitializeConditionVariable == 0 ) goto err;
    ourWakeConditionVariable = (WakeConditionVariableFunc*) GetProcAddress( kernel32Handle, "WakeConditionVariable" );
    if ( ourWakeConditionVariable == 0 ) goto err;
    ourWakeAllConditionVariable = (WakeAllConditionVariableFunc*) GetProcAddress( kernel32Handle, "WakeAllConditionVariable" );
    if ( ourWakeAllConditionVariable == 0 ) goto err;
  }
}

#else

#define ourSleepConditionVariableCS SleepConditionVariableCS
#define ourInitializeConditionVariable InitializeConditionVariable
#define ourWakeConditionVariable WakeConditionVariable
#define ourWakeAllConditionVariable WakeAllConditionVariable
#define init_cond_var_funcs()

#endif
    
#if defined( __RADNT__ )

typedef MMRESULT WINAPI timeBeginPeriodT( UINT p );
typedef MMRESULT WINAPI timeEndPeriodT( UINT p );

static timeBeginPeriodT * timeBeginPeriodp = 0;
static timeEndPeriodT * timeEndPeriodp = 0;

static InitializeCriticalSectionAndSpinCountFunc * icsasc = 0;
static TryEnterCriticalSectionFunc * tecs = 0;
static SetThreadIdealProcessorFunc * stip = 0;
static IsDebuggerPresentFunc * idp = 0;
static SetThreadDescriptionFunc * std = 0;
static S32 set_threadnames = 0;

static void get_winmm_funcs( void )
{
  HMODULE l;

  l = LoadLibraryA( "winmm.dll" );
  timeBeginPeriodp = (timeBeginPeriodT*)GetProcAddress( l, "timeBeginPeriod" );
  timeEndPeriodp = (timeEndPeriodT*)GetProcAddress( l, "timeEndPeriod" );
}

#else

#define tecs TryEnterCriticalSection

#endif

typedef struct rriThread
{
  thread_function2 * function;
  void * user_data;
  char short_name[32];
  HANDLE handle;
  U32 ret;
  DWORD id;   // windows id
} rriThread;

static U32 active_rrthreads = 0;
static S32 number_cores = -1;

#pragma pack(push,8)
typedef struct THREADNAME_INFO
{
  DWORD dwType;
  LPCSTR szName;
  DWORD dwThreadID;
  DWORD dwFlags;
} THREADNAME_INFO;
#pragma pack(pop)


#define PLATFORM_HIGHEST_PRIORITY THREAD_PRIORITY_HIGHEST
#define PLATFORM_LOWEST_PRIORITY  THREAD_PRIORITY_LOWEST
#define PLATFORM_GET_CURRENT_THREAD_PRIORITY (GetThreadPriority(GetCurrentThread()))

#define PLATFORM_NORMAL_PRIORITY  THREAD_PRIORITY_NORMAL
#define PLATFORM_LOW_PRIORITY     THREAD_PRIORITY_BELOW_NORMAL
#define PLATFORM_HIGH_PRIORITY    THREAD_PRIORITY_ABOVE_NORMAL

#include "rrthreads2_priorities.inl"


static LONG WINAPI handle_bad( struct _EXCEPTION_POINTERS * info )
{
  if ( info->ExceptionRecord->ExceptionCode == 0x406d1388 )
  {
    return EXCEPTION_CONTINUE_EXECUTION;
  }
  return EXCEPTION_CONTINUE_SEARCH;
}

static void RADLINK rr_set_thread_name( HANDLE thread_handle, DWORD thread_id, char const * thread_name )
{
#if !defined(_MSC_VER) || _MSC_VER > 1200

#if defined( __RADNT__ )
  // if we're on windows, see if isdebuggerpresent, 
  //    if so, call it, and only name the threads if it is present

  if ( (UINTa) std <= 1 )
  {
    if ( std == 0 )
    {
      HANDLE kernel32Handle = GetModuleHandle(_T("KERNEL32"));
      if (kernel32Handle != 0)
      {
        std = (SetThreadDescriptionFunc*) GetProcAddress( kernel32Handle, "SetThreadDescription" );
        if ( std )
          goto found_std;
      }
      std = (SetThreadDescriptionFunc*) (UINTa) 1;
    }
  } 
  else
  {
    WCHAR name[128];
    WCHAR * n;
    char const * t;

   found_std:
    n = name;
    t = thread_name;
    while( ( *t ) && ( (n-name) < 128 ) )
      *n++ = *t++;
    *n = 0;

    std( thread_handle, name );
  }
  
  if ( (UINTa) idp <= 1 )
  {
    if ( idp == 0 )
    {
      HANDLE kernel32Handle = GetModuleHandle(_T("KERNEL32"));
      if (kernel32Handle != 0)
      {
        idp = (IsDebuggerPresentFunc*) GetProcAddress( kernel32Handle, "IsDebuggerPresent" );
        if ( idp )
          goto found_idp;
      }
      idp = (IsDebuggerPresentFunc*) (UINTa) 1;
    }
  }
 found_idp: 
  if ( !idp() ) 
    return;
#endif
  {
    static U32 volatile raising = 0;
    
    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = thread_name;
    info.dwThreadID = thread_id;
    info.dwFlags = 0;

    for(;;)
    {
      if ( rrAtomicCmpXchg32( &raising, 1, 0 ) == 0 )
      {
  #if defined(NO_ADD_VECTORED_EXCEPTION)
        __try {
  #else      
        // use on windows and consoles
        void * flt; 
        flt = AddVectoredExceptionHandler( 1, &handle_bad );
  #endif
        RaiseException( 0x406d1388, 0, sizeof(info)/sizeof(DWORD), (ULONG_PTR *) &info);

  #if defined(NO_ADD_VECTORED_EXCEPTION)
        } __except( GetExceptionCode()==0x406D1388 ? EXCEPTION_CONTINUE_EXECUTION : EXCEPTION_EXECUTE_HANDLER ) {}
  #else      
        RemoveVectoredExceptionHandler( flt );
  #endif      

        raising = 0;
        break;
      } 
      Sleep(1);
    }
  }
#endif // _MSC_VER
}

static DWORD WINAPI win32_shim( LPVOID param )
{
  rrThread * rr;
  rriThread * t;

  rr = (rrThread*) param;
  t = rr->i;

  // set the thread name in the debugger (more reliable to do this on the thread itself)
  if ( t->short_name[0] ) 
  {
#if defined( __RADNT__ ) 
    if ( set_threadnames != -1 )
#endif
    {
      rr_set_thread_name( t->handle, t->id, t->short_name );
    }
  }

  // run the user thread
  t->ret = t->function( t->user_data );

  return 0;
}

// stick an rrthread directly on a specific cpu
static void set_cpu_core( HANDLE handle, rrThreadCoreIndex index )
{
  U32 core_num;
  S32 hard_affin;

  if ( rad_to_platform_core( index, &core_num, &hard_affin ) )
  {
    #if defined(__RADPPC__)

      XSetThreadProcessor( handle, core_num );
  
    #elif defined(NEW_WINDOWS_APIS)

      {
        PROCESSOR_NUMBER PN = {0};
        PN.Number =  (BYTE)core_num;
        SetThreadIdealProcessorEx(handle, &PN, 0);
      }

      #if defined( RAD_USE_6_CPUS_ONLY )
      // reset to only run on first six cpus
      SetThreadAffinityMask( handle, 63 );
      #endif

    #else

      // on win32, try setthreadideal first, and then the affinitymask
      if ( (UINTa) stip <= 1 )
      {
        if ( stip == 0 )
        {
          HANDLE kernel32Handle = GetModuleHandle(_T("KERNEL32"));
          if (kernel32Handle != 0)
          {
            stip = (SetThreadIdealProcessorFunc*) GetProcAddress( kernel32Handle, "SetThreadIdealProcessor" );
            if ( stip )
              goto found;
          }
          // missing flag
          stip = (SetThreadIdealProcessorFunc*) (UINTa) 1;
        }

        // didn't find setideal, use set affinity
        hard_affin = 1;
      }
      else
      {
       found:
        // set it with SetThreadIdealProcessor
         stip( handle, core_num );
      }
    #endif

    #if defined( HAS_AFFINITY_MASK )
      if ( hard_affin )
        SetThreadAffinityMask( handle, (UINTa) ( 1 << core_num ) );
    #endif
  }
}


RADDEFFUNC rrbool RADLINK rrThreadCreate( rrThread * rr, 
                                          thread_function2 * function,  
                                          void * user_data,
                                          U32 stack_size,
                                          rrThreadPriority2 priority,
                                          rrThreadCoreIndex core_assign,
                                          char const * name )
{
  rriThread * thread;
  U32 count;
  int win_pri;

  RR_COMPILER_ASSERT( ( sizeof ( rrThread ) >= ( sizeof( rriThread ) + sizeof(void*) + 15 ) ) );  // make sure our rrthread isn't too big

  RR_ASSERT_ALWAYS_NO_SHIP( stack_size != 0 );

  thread = (rriThread*) ( ( ( (UINTa) rr->data ) + 15 ) & ~15 );
  rr->i = thread;

#if defined( __RADNT__ )
  if ( set_threadnames == 0 )
  {
    char buf[ 16 ];
    set_threadnames = 1;

    if ( GetEnvironmentVariableA( "RAD_NO_SET_THREAD_NAMES", buf, 16 ) )
    {
      if ( buf[ 0 ] == '1' )
        set_threadnames = -1;
    }
  }
#endif

  init_cond_var_funcs();

  // setup the info
  thread->function = function;
  thread->user_data = user_data;
  limit_thread_name( thread->short_name, sizeof(thread->short_name), name );
  thread->handle = CreateThread( 0, stack_size, win32_shim, rr, CREATE_SUSPENDED, &thread->id );  //jkr: assert non-zero stacksize?

  win_pri = rad_to_platform_pri( priority );
  if ( is_good_platform_priority( win_pri ) )
    SetThreadPriority( thread->handle, win_pri );

  count = rrAtomicAddExchange32( &active_rrthreads, 1 );

  set_cpu_core( thread->handle, core_assign );

  #if defined( __RADNT__ )
    if ( count == 0 )
    {
      /*
        This function affects a global Windows setting. Windows uses the lowest value
        (that is, highest resolution) requested by any process. Setting a higher
        resolution can improve the accuracy of time-out intervals in wait functions.
        However, it can also reduce overall system performance, because the thread
        scheduler switches tasks more often. High resolutions can also prevent the CPU
        power management system from entering power-saving modes. Setting a higher
        resolution does not improve the accuracy of the high-resolution performance counter.
      */
      if ( timeBeginPeriodp == 0 )
        get_winmm_funcs();
      if ( timeBeginPeriodp )
        timeBeginPeriodp( 1 );
    }
  #endif

  // start the thread (we created it suspended)
  ResumeThread( thread->handle );

  return 1;
}


// waits for the thread to shutdown and return
RADDEFFUNC U32 RADLINK rrThreadDestroy( rrThread * rr )
{
  rriThread * thread;

  thread = rr->i;
  
  if ( WaitForSingleObject( thread->handle, INFINITE ) == WAIT_OBJECT_0 )
  {
    S32 old;
    CloseHandle( thread->handle );

    thread->handle = INVALID_HANDLE_VALUE;

    old = rrAtomicAddExchange32( &active_rrthreads, -1 );
    #if defined( __RADNT__ ) 
      if ( old == 1 )
      {
        if ( timeEndPeriodp )
          timeEndPeriodp( 1 );
      }
    #endif
  }

  return thread->ret;
}

RADDEFFUNC void RADLINK rrThreadSetCores( rrThread * rr, U64 bitmask )
{
  #if defined(HAS_AFFINITY_MASK)
    rriThread * thread;
    thread = rr->i;

    SetThreadAffinityMask( thread->handle, (DWORD_PTR)bitmask );  
  #endif
}


RADDEFFUNC void RADLINK rrThreadGetOSThreadID( rrThread * rr, void * out )
{
  rriThread * thread;
  thread = rr->i;

  ((DWORD*)out)[ 0 ] = thread->id;
}

RADDEFFUNC void RADLINK rrThreadSleep( U32 ms )
{
#if !defined( __RADCONSOLE__ )
  {
    int p = (1<<30);

    // on windows, if the wait time is short enough to be
    //   close to the granuarity of the thread scheduler,
    //   then bump up the thread priority before you sleep,
    //   so the scheduler will wake you up more accurately.
    if ( ms <= 30 && ms != 0 )
    {
      p = GetThreadPriority( GetCurrentThread() );
      SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_HIGHEST );
      //SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL+5 );
    }
#endif

  Sleep( ms );

#if !defined( __RADCONSOLE__ )
    if ( p != (1<<30) )
    {
      SetThreadPriority( GetCurrentThread(), p );
    }
  }
#endif
}


// rrThreadSpinHyperYield should be called in spin loops to yield hyper-threads
RADDEFFUNC void RADLINK rrThreadSpinHyperYield( void )
{
  // on hyperthreaded procs need this :
  //  on non-HT, it improves performance too
  #if defined(YieldProcessor)||defined(__RADARM__)
    YieldProcessor();
  #else
    _mm_pause();
  #endif  
}

typedef struct rriCondVar
{
  CRITICAL_SECTION   mutex;
#if defined(HAS_NATIVE_CONDITION_VARS) || defined(HAS_OPTIONAL_CONDITION_VARS)
  CONDITION_VARIABLE cond;
#endif
} rriCondVar;

typedef struct rriMutex
{
    rriCondVar  cv;
    U64         atomic_owner_and_flag; // this is the mutex gate ; atomic!
    U32         count;  // recursion count - protected by this mutex
    U32         type;

    HANDLE      m;      // for pre-vista windows only
} rriMutex;

typedef struct rriSemaphore
{
    rriCondVar  cv;
    U32         count;
    U32         type;

    HANDLE sema;      // for pre-vista windows only
} rriSemaphore;

#if defined(HAS_NATIVE_CONDITION_VARS) || defined(HAS_OPTIONAL_CONDITION_VARS)

typedef DWORD WaitUntilTimeType;

static void GetWaitTime( WaitUntilTimeType * time, U32 ms )
{
  *time = ms;
}

//=======================================================

// CB 09-12-2016 : rriCondVar - internal CondVar used by Mutex & Semaphore

static rrbool  rriCondVarCreate( rriCondVar * p )
{
  init_cond_var_funcs();
  if ( (UINTa) ourSleepConditionVariableCS <= 1 )
    return 0;

  InitializeCriticalSection( &(p->mutex) );
  ourInitializeConditionVariable( &(p->cond) );
  return 1;
}

static void   rriCondVarDestroy( rriCondVar * p )
{
  DeleteCriticalSection( &(p->mutex) );
}

static void   rriCondVarLock( rriCondVar * p )
{
  EnterCriticalSection( &p->mutex );  
}

static void  rriCondVarUnlock( rriCondVar * p )
{
  LeaveCriticalSection( &p->mutex );    
}

// returns 1 if signalled, 0 if timed out
static rrbool rriCondVarUnlockWaitUntilLock( rriCondVar * p, WaitUntilTimeType * waitUntil )
{
  for (;;)
  {
    BOOL ret;

    ret = ourSleepConditionVariableCS( &(p->cond), &(p->mutex), *waitUntil );
    if ( ret )
      return 1;

    if ( ret == 0 )
    {
      if ( GetLastError() == ERROR_TIMEOUT )
      {
        // note mutex lock is still reacquired here
        return 0; // expected failure due to timeout
      }
    }
  }
}

static void rriCondVarUnlockWaitLock( rriCondVar * p )
{
  WaitUntilTimeType wait = INFINITE;
  
  rriCondVarUnlockWaitUntilLock( p, &wait );
}


static void rriCondVarSignalUnlock( rriCondVar * p )
{
  ourWakeConditionVariable( &p->cond );  
  LeaveCriticalSection( &p->mutex );    
}
 
static void rriCondVarBroadcastUnlock( rriCondVar * p )
{
  ourWakeAllConditionVariable( &p->cond );   
  LeaveCriticalSection( &p->mutex );    
}


//================================================================

#define MUTEX_TID_TYPE  DWORD   
#define MUTEX_TID_GET() GetCurrentThreadId()

#if defined(HAS_OPTIONAL_CONDITION_VARS)
#define DECLARE_NAME( name ) RR_STRING_JOIN( cv, name )
#define DECLARE_FUNCTION( ret, name ) static ret DECLARE_NAME( name )
#endif

#include "primativesoncondvars.inl"

//=============================================================================

#endif

#if !defined(HAS_NATIVE_CONDITION_VARS)

#define TYPE_MUTEX        0x10000
#define TYPE_CRITSEC      0x20000
#define MASK_ONLY         0xffff
#define NO_TYPE           (~MASK_ONLY)

RADDEFFUNC rrbool RADLINK rrMutexCreate( rrMutex * rr, S32 mtype )
{
  rriMutex * p;
  int spins;

  RR_COMPILER_ASSERT( ( sizeof ( rrMutex ) >= ( sizeof( rriMutex ) + sizeof(void*) + 15 ) ) );  // make sure our rrmutex isn't too big

  p = (rriMutex*) ( ( ( (UINTa) rr->data ) + 15 ) & ~15 );
  p->type = 0;
  rr->i = p;

#if defined(HAS_OPTIONAL_CONDITION_VARS)
  // try the condvar version first
  if ( DECLARE_NAME(rrMutexCreate)( rr, mtype ) )
  {
    p->type |= ( (int) mtype );
    return 1;
  }
#endif

  if ( mtype == rrMutexNeedFullTimeouts )
  {
#if defined( __RADNT__)
   use_mutex:
#endif
    // use a normal windows mutex
    p->m = CreateMutex( 0, 0, 0 );

    p->type = TYPE_MUTEX | ( (int) mtype );
    return 1;
  }

  spins = rrGetSpinCount();
    
#if !defined( __RADNT__ ) 
  InitializeCriticalSectionAndSpinCount( &p->cv.mutex, spins );
#else
  // see if we have tryentercriticalsection on win32
  if ( (UINTa) tecs <= 1 )
  {
    if ( tecs == 0 )
    {
      HANDLE kernel32Handle = GetModuleHandle(_T("KERNEL32"));
      if (kernel32Handle != 0)
      {
        tecs = (TryEnterCriticalSectionFunc*) GetProcAddress( kernel32Handle, "TryEnterCriticalSection" );
        if ( tecs )
          goto have_tecs;
      }
      tecs = (TryEnterCriticalSectionFunc*) (UINTa) 1;
    }

    // we don't have tryenter, so if we need timeouts, then we have to use a mutex
    if ( mtype != rrMutexFullLocksOnly )
      goto use_mutex;
  }

 have_tecs:

  // we can use critical sections...

  // spincount is way better on real multicore systems ; has no affect on single core
  if ( (UINTa) icsasc <= 1 )
  {
    if ( icsasc == 0 )
    {
      HANDLE kernel32Handle = GetModuleHandle(_T("KERNEL32"));
      if (kernel32Handle != 0)
      {
        icsasc = (InitializeCriticalSectionAndSpinCountFunc*) GetProcAddress( kernel32Handle, "InitializeCriticalSectionAndSpinCount" );
        if ( icsasc )
          goto found;
      }
      icsasc = (InitializeCriticalSectionAndSpinCountFunc*) (UINTa) 1;
    }

    // no spincount - use the old version
    InitializeCriticalSection( &p->cv.mutex );
  }
  else
  {
   found:
    icsasc( &p->cv.mutex, spins );
  }
#endif

  p->type = TYPE_CRITSEC | ( (int) mtype );

  return 1;
}


RADDEFFUNC void RADLINK rrMutexDestroy( rrMutex * rr )
{
  U32 type;
  rriMutex * p;

  p = rr->i;

  type = p->type;
 
#if defined(HAS_OPTIONAL_CONDITION_VARS)
  if ( ( type & NO_TYPE ) == TYPE_CONDVAR )
  {
    DECLARE_NAME(rrMutexDestroy)( rr );
  } 
  else 
#endif // else falls through
  if ( ( type & NO_TYPE ) == TYPE_MUTEX )
  {
    CloseHandle( p->m );
    p->m = INVALID_HANDLE_VALUE;
  }
  else if ( ( type & NO_TYPE ) == TYPE_CRITSEC )
  {
    DeleteCriticalSection( &p->cv.mutex );
  }
  p->type = 0;
}


RADDEFFUNC void RADLINK rrMutexLock( rrMutex * rr )
{
  U32 type;
  rriMutex * p;

  p = rr->i;

  type = p->type;

#if defined(HAS_OPTIONAL_CONDITION_VARS)
  if ( ( type & NO_TYPE ) == TYPE_CONDVAR )
  {
    DECLARE_NAME(rrMutexLock)( rr );
  } 
  else 
#endif // else falls through
  if ( ( type & NO_TYPE ) == TYPE_MUTEX )
  {
    WaitForSingleObject( p->m, INFINITE );
  }
  else if ( ( type & NO_TYPE ) == TYPE_CRITSEC )
  {
    EnterCriticalSection( &p->cv.mutex );
  }
  else
  {
    RR_ASSERT(0);
  }
}


RADDEFFUNC void RADLINK rrMutexUnlock( rrMutex * rr )
{
  U32 type;
  rriMutex * p;

  p = rr->i;

  type = p->type;

#if defined(HAS_OPTIONAL_CONDITION_VARS)
  if ( ( type & NO_TYPE ) == TYPE_CONDVAR )
  {
    DECLARE_NAME(rrMutexUnlock)( rr );
  } 
  else 
#endif // else falls through
  if ( ( type & NO_TYPE ) == TYPE_MUTEX )
  {
    ReleaseMutex( p->m );
  }
  else if ( ( type & NO_TYPE ) == TYPE_CRITSEC )
  {
    LeaveCriticalSection( &p->cv.mutex );
  }
  else
  {
    RR_ASSERT(0);
  }
}


RADDEFFUNC rrbool RADLINK rrMutexLockTimeout( rrMutex * rr, U32 ms )
{
  U32 type;
  rriMutex * p;

  p = rr->i;
  
  // fail lock of uninitialized var
  if ( ( p == 0 ) && ( ms != RR_WAIT_INFINITE ) )
    return 0;

  type = p->type;

  // you tried to use a non-infinite timeout, when you said you didn't need them!
  RR_ASSERT( !( ( ( type & MASK_ONLY ) == (int) rrMutexDontNeedTimeouts ) && ( ms != RR_WAIT_INFINITE ) ) );

  // you tried to use a non-zero timeout, when you said you only need zero tests
  RR_ASSERT( !( ( ( type & MASK_ONLY ) == (int) rrMutexNeedOnlyZeroTimeouts ) && ( ms != 0 ) && ( ms != RR_WAIT_INFINITE ) ) );

#if defined(HAS_OPTIONAL_CONDITION_VARS)
  if ( ( type & NO_TYPE ) == TYPE_CONDVAR )
  {
    return DECLARE_NAME(rrMutexLockTimeout)( rr, ms );
  } 
  else 
#endif // else falls through
  if ( ( type & NO_TYPE ) == TYPE_MUTEX )
  {
    switch ( WaitForSingleObject( p->m, ( ms == RR_WAIT_INFINITE ) ? INFINITE : ms ) )
    {
      case WAIT_OBJECT_0:
      case WAIT_ABANDONED:
        return 1;

      case WAIT_TIMEOUT:
      default:
         return 0;
    }
  }
  else if ( ( type & NO_TYPE ) == TYPE_CRITSEC )
  {
    // infinite wait always works with critical sections
    if ( ms == RR_WAIT_INFINITE )
    {
      EnterCriticalSection( &p->cv.mutex );
      return 1;
    }

    RR_ASSERT( ms == 0 );

    // try a lock
    return tecs( &p->cv.mutex ) ? 1 : 0;
  }
  
  RR_ASSERT(0);
  return 0;
}

#define TYPE_SEMA_SIMPLE 0x10000

RADDEFFUNC rrbool RADLINK rrSemaphoreCreate( rrSemaphore * rr, S32 initialCount, S32 maxCount )
{
  rriSemaphore *p;

  RR_COMPILER_ASSERT( ( sizeof ( rrSemaphore ) >= ( sizeof( rriSemaphore ) + sizeof(void*) + 15 ) ) );

  p = (rriSemaphore*) ( ( ( (UINTa) rr->data ) + 15 ) & ~15 );
  rr->i = p;
  p->type = 0;

  RR_ASSERT( maxCount > 0 );

#if defined(HAS_OPTIONAL_CONDITION_VARS)
  // try condvar first
  if ( DECLARE_NAME(rrSemaphoreCreate)( rr, initialCount, maxCount ) )
    return 1;
#endif 

#if defined( NEW_WINDOWS_APIS )
  p->sema = CreateSemaphoreExW(0, initialCount, maxCount, NULL, 0, SEMAPHORE_ALL_ACCESS);
#else
  p->sema = CreateSemaphore( NULL, initialCount, maxCount, NULL );
#endif
  if ( p->sema != INVALID_HANDLE_VALUE )
  {
    p->type = TYPE_SEMA_SIMPLE;
    return 1;
  }

  return 0;
}

RADDEFFUNC void RADLINK rrSemaphoreDestroy( rrSemaphore * rr )
{
  U32 type;
  rriSemaphore * p;

  p = rr->i;
  type = p->type;

#if defined(HAS_OPTIONAL_CONDITION_VARS)
  if ( ( type & NO_TYPE ) == TYPE_SEMA_CONDVAR )
  {
    DECLARE_NAME(rrSemaphoreDestroy)( rr );
  } 
  else 
#endif // else falls through
  if ( ( type & NO_TYPE ) == TYPE_SEMA_SIMPLE )
  {
    CloseHandle( p->sema );
    p->sema = INVALID_HANDLE_VALUE;
  }
  p->type = 0;
}

RADDEFFUNC rrbool RADLINK rrSemaphoreDecrementOrWait( rrSemaphore * rr, U32 ms )
{
  U32 type;
  rriSemaphore * p;

  p = rr->i;
  type = p->type;

#if defined(HAS_OPTIONAL_CONDITION_VARS)
  if ( ( type & NO_TYPE ) == TYPE_SEMA_CONDVAR )
  {
    return DECLARE_NAME(rrSemaphoreDecrementOrWait)( rr, ms );
  } 
  else 
#endif // else falls through
  if ( ( type & NO_TYPE ) == TYPE_SEMA_SIMPLE )
  {
    switch ( WaitForSingleObject( p->sema, ( ms == RR_WAIT_INFINITE ) ? INFINITE : ms ) )
    {
      case WAIT_OBJECT_0:
        return 1;
      case WAIT_ABANDONED:
      case WAIT_TIMEOUT:
      default:
        return 0;
    }
  }
  else
  {
    RR_ASSERT(0);
    return 0;
  }
}

RADDEFFUNC void RADLINK rrSemaphoreIncrement( rrSemaphore * rr, S32 count )
{
  U32 type;
  rriSemaphore * p;

  p = rr->i;
  type = p->type;

#if defined(HAS_OPTIONAL_CONDITION_VARS)
  if ( ( type & NO_TYPE ) == TYPE_SEMA_CONDVAR )
  {
    DECLARE_NAME(rrSemaphoreIncrement)( rr, count );
  } 
  else 
#endif // else falls through
  if ( ( type & NO_TYPE ) == TYPE_SEMA_SIMPLE )
  {
    ReleaseSemaphore( p->sema, count, NULL );
  }
  else
  {
    RR_ASSERT(0);
  }
}

#endif
