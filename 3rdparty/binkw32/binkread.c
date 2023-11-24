// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#include "binkproj.h"

#include "rrstring.h"

#define SEGMENT_NAME "read"
#include "binksegment.h"

#include "binkacd.h"
#ifndef NO_BINK10_SUPPORT
#include "expand.h"
#endif
#ifdef INC_BINK2
#include "expand2.h" // @cdep ignore
// @cdep pre $if( $bink2on, $requires(expand2.h), )
#endif
#include "binkcomm.h"
#include "radmath.h"
#include "binkmarkers.h"

#include "rrAtomics.h"

#include "rrThreads2.h" 

// Some platforms only let threads run on one core, just set a default
#ifdef __RADSINGLECORETHREAD__
#define BINK_SND_CORE rrThreadCore(__RADSINGLECORETHREAD__)
#define BINK_IO_CORE  rrThreadCore(__RADSINGLECORETHREAD__)
#else
#define BINK_SND_CORE 0
#define BINK_IO_CORE  0
#endif

#ifdef RR_CAN_ALLOC_THREAD_STACK
  #define alloc_rr_thread_stack( bink, stack_ptr, size ) { stack_ptr = (rrThread*) bpopmalloc( 0, bink, size ); }
  #define dealloc_rr_thread_stack( stack_ptr ) { popfree( stack_ptr ); stack_ptr = 0; }
#else
  #define alloc_rr_thread_stack( bink, stack_ptr, size ) 
  #define dealloc_rr_thread_stack( v_rr_thread ) 
#endif

#ifndef NO_BINK_BLITTERS
#include "blitter.h"
#endif
#include "popmal.h"

#include "cpu.h"

#include "binktm.h"

#ifdef __RADNT__
#include "rrcpu.h"
#endif

static U32 unique_count = 0;

#if defined( __RAD_NDA_PLATFORM__ )
  // @cdep pre $AddPlatformInclude
  #include RR_PLATFORM_PATH_STR( __RAD_NDA_PLATFORM__, _binkread.h )
#endif


//===================================
#define OSFRAMEREADALIGN 128

#ifdef RADUSETM3
  #define bpopmalloc( p,b,y ) bpopmalloci( p,b,y, __FILE__, __LINE__ )
  void* bpopmalloci(void * pmbuf, HBINK bink,U64 amt, char const * info, U32 line)
#else
  void* bpopmalloc(void * pmbuf, HBINK bink,U64 amt);
  void* bpopmalloc(void * pmbuf, HBINK bink,U64 amt)
#endif
{
  bink->totalmem+=(popmalloctotal(pmbuf)+amt);
#ifdef RADUSETM3
  return(popmalloci(pmbuf,amt,info,line));
#else
  return(popmalloc(pmbuf,amt));
#endif
}
//===================================


#if !defined( DLL_FOR_GENERIC_OS ) && !defined( RADUTIL ) && !defined(NO_THREADS)
  #define BINK_ASYNC
  #define USE_THREADS
  #define USE_IO_THREAD
  #define USE_SND_THREAD
#endif


//===================================

#if defined( BINK_ASYNC )

extern void (*bink_async_wait)(HBINK bink);

static void wait_for_asyncs( HBINK bink )
{
  if ( bink_async_wait )
    bink_async_wait( bink );
}

extern int RAD_platform_info( int thread_num, void * output );

static S32 get_async_thread( int num, void * output )
{
  if ( RAD_platform_info( num, output ) )
    return 1;
  return 0;
}

#else

#define wait_for_asyncs(b)
#define get_async_thread(n,v) 0

#endif

//===================================




//===================================

#ifdef USE_THREADS

static S32 volatile  bink_mutex_init = 0;
static HBINK volatile bink_first = 0;
static HBINK volatile bink_io_last = 0;

static rrMutex bink_list;
static rrMutex bink_snd_global;
static rrMutex bink_io_global;

static U32 volatile global_lock = 0;

// only used until bink mutex is created
static void quick_global_lock( void )
{
  U32 times = 0;
  for(;;)
  {
    U32 old = rrAtomicAddExchange32( &global_lock, 1 );
    if ( old == 0 )
      return;
    rrAtomicAddExchange32( &global_lock, -1 );
    rrThreadSpinHyperYield();
    if ( ++times >= 512 )
    {
      times = 0;
      rrThreadSleep( 1 );
    }
  }
}

static void quick_global_unlock( void )
{
  rrAtomicAddExchange32( &global_lock, -1 );
}

static void setup_bink_mutex( void )
{
  quick_global_lock();
  if ( !bink_mutex_init )
  {
    if ( !rrMutexCreate( &bink_list, rrMutexNeedFullTimeouts ) )
      RR_BREAK();
    if ( !rrMutexCreate( &bink_snd_global, rrMutexNeedOnlyZeroTimeouts ) )
      RR_BREAK();
    if ( !rrMutexCreate( &bink_io_global, rrMutexNeedOnlyZeroTimeouts ) )
      RR_BREAK();
    tmLockName( tm_mask, &bink_list, "Bink list mutex" );
    tmLockName( tm_mask, &bink_snd_global, "Bink global sound mutex" );
    tmLockName( tm_mask, &bink_io_global, "Bink global io mutex" );

    bink_mutex_init = 1;
  }
  quick_global_unlock();
}

static void cleanup_bink_mutex( void )
{
  quick_global_lock();
  if ( bink_mutex_init )
  {
    rrMutexDestroy( &bink_list );
    rrMutexDestroy( &bink_io_global );
    rrMutexDestroy( &bink_snd_global );
    bink_mutex_init = 0;
  }
  quick_global_unlock();
}

#if !defined(RADUSETM3)
  #define our_lock_timeout(m,ms,str) rrMutexLockTimeout( m, ms )
  #define our_lock(m,str) { rrMutexLock(m); }
  #define our_unlock(m,str) { rrMutexUnlock(m); }
  #define our_lock_timeout_help(m,ms,str,f,l) rrMutexLockTimeout( m, ms )
  #define our_lock_help(m,str,f,l) { rrMutexLock(m); }
  #define our_unlock_help(m,f,l) { rrMutexUnlock(m); }
#else
  static void RADNOINLINE our_lock_help( rrMutex * m, char const * name, char const * file, U32 line )
  {
    tmStartWaitForLockEx( tm_mask, TMZF_STALL, m, file, line, name );
    rrMutexLock(m);
    tmEndWaitForLock( tm_mask );
    tmAcquiredLockEx( tm_mask, 0, m, file, line, name );
  }

  static void RADNOINLINE our_unlock_help( rrMutex * m, char const * file, U32 line )
  {
    rrMutexUnlock( m );
    tmReleasedLockEx( tm_mask, m, file, line );
  }

  static S32 RADNOINLINE our_lock_timeout_help( rrMutex * m, U32 ms, char const * name, char const * file, U32 line )
  {
    tmStartWaitForLockEx( tm_mask, TMZF_STALL, m, file, line, name );
    if ( rrMutexLockTimeout( m, ms ) )
    {
      tmEndWaitForLock( tm_mask );
      tmAcquiredLockEx( tm_mask, 0, m, file, line, name );
      return 1;
    }
    else
    {
      tmEndWaitForLock( tm_mask );
      return 0;
    }
  }

  #define our_lock_timeout(m,ms,str) our_lock_timeout_help( m, ms, str, __FILE__, __LINE__ )
  #define our_lock(m,str) our_lock_help( m,str,__FILE__,__LINE__ )
  #define our_unlock(m,str) our_unlock_help( m,__FILE__,__LINE__ )
#endif

static void place_in_bink_list( HBINK bink )
{
  setup_bink_mutex();
    
  our_lock( &bink_list, "Bink list mutex" );
  {
    HBINK b;
    b = bink_first;
    for( ; ; )
    {
      if ( b == 0 )
      {
        bink->next_bink = bink_first;
        bink_first = bink;
        break;
      }
      if ( b == bink )
        break;
      b = b->next_bink;
    }
  }
  our_unlock( &bink_list, "Bink list mutex" );
}

static void remove_from_bink_list( HBINK bink )
{
  if ( bink->in_bink_list )
  {
    our_lock( &bink_list, "Bink list mutex" );

    if ( bink_first == bink )
    {
      bink_first = bink_first->next_bink;
    }
    else
    {
      HBINK b;
      b = bink_first;
      while ( b )
      {
        if ( b->next_bink == bink )
        {
          b->next_bink = bink->next_bink;
          break;
        }
        b = b->next_bink;
      }
    }
    if (bink_io_last == bink)
      bink_io_last = 0;

    our_unlock( &bink_list, "Bink list mutex" );

    bink->in_bink_list = 0;
  }
}

static void lock_two_mutexes( rrMutex * m1, char const * str1, rrMutex * m2, char const * str2 )
{
  for(;;)
  {
    our_lock( m1, str1 );
    if ( our_lock_timeout( m2, 0, str2 ) )
      return;
    our_unlock( m1, str1 );

    our_lock( m2, str2 );
    if ( our_lock_timeout( m1, 0, str1 ) )
      return;
    our_unlock( m2, str2 );
  }
}

static void lock_io_global_help(S32 l)
{    
  if ( bink_mutex_init )
    our_lock_help( &bink_io_global, "Bink global IO mutex", __FILE__,l );
}

static void unlock_io_global_help(S32 l)
{    
  if ( bink_mutex_init )
    our_unlock_help( &bink_io_global, __FILE__,l );
}

static void lock_snd_global_help(S32 l)
{  
  if ( bink_mutex_init )
    our_lock_help( &bink_snd_global, "Bink global sound mutex", __FILE__,l );
}

static void unlock_snd_global_help(S32 l)
{
  if ( bink_mutex_init )
    our_unlock_help( &bink_snd_global, __FILE__,l );
}

#define lock_io_global() lock_io_global_help(__LINE__)
#define unlock_io_global() unlock_io_global_help(__LINE__)

#define lock_snd_global() lock_snd_global_help(__LINE__)
#define unlock_snd_global() unlock_snd_global_help(__LINE__)

#else  // no threads

#define remove_from_bink_list( bink )

#define setup_bink_mutex()
#define lock_two_mutexes(...)

#define lock_io_global()
#define unlock_io_global()

#define lock_snd_global()
#define unlock_snd_global()

#define our_lock_timeout(m,ms,str) 1
#define our_unlock(m,str) {}
#define our_lock(m,str) {}

#endif

//===================================



//===================================

#ifdef USE_IO_THREAD

static S32 volatile bink_io_thread_init = 0;
static S32 volatile bink_io_setup_count = 0;
// bink_io_last moved up the file.

#ifdef RR_CAN_ALLOC_THREAD_STACK
static rrThread * stack_for_io_thread;
#endif

static rrThread bink_rr_thread;
static S32 volatile bink_io_close = 0;
static rrSemaphore bink_io_sema;

static U64 bink_do_pri_io( HBINK b )
{
  U64 ret = 0;
  
  if (b->bio.ReadError)
    b->ReadError=1;

  if ( ( b->bio.ReadError ) || ( b->closing ) )
    return 0;

  if ( b->needio ) 
  {
    ret = b->bio.ReadFrame(&b->bio,b->FrameNum-1,b->compframeoffset,b->compframe,b->compframesize);
    b->lastfileread = b->compframeoffset + b->compframesize;
    b->needio = 0;

    if (b->bio.ReadError)
      b->ReadError=1;
  }

  return ret;
}

static U64 bink_do_io( HBINK b )
{
  U64 ret = 0;
  
  if ( ( b->bio.ReadError ) || ( b->closing ) )
    return 0;

  if ( b->bio.BGControl )
  {
    if ( b->bio.BGControl( &b->bio, 0 ) & BINKBGIOSUSPEND )
      return 0;
  }

  if ( b->preloadptr == 0 )
    ret = b->bio.Idle(&b->bio);

  return ret;
}

#define READ_PERCENT( per ) ( ( per * 128 ) / 100 )

static S32 GET_READ_PERCENT( HBINK bink )
{
  U64 used, size;

  // all errors return 101% full
  if ( bink == 0 )
    return READ_PERCENT( 101 );

  used = bink->bio.CurBufUsed;
  size = bink->bio.CurBufSize;

  if ( size == 0 )
    return READ_PERCENT( 101 );
  if ( bink->bio.ReadError )
    return READ_PERCENT( 101 );
  if ( bink->closing )
    return READ_PERCENT( 101 );
  if ( bink->bio.BGControl )
  {
    if ( bink->bio.BGControl( &bink->bio, 0 ) & BINKBGIOSUSPEND )
    {
      return READ_PERCENT( 101 );
    }
  }
  
  // max of 100% full
  if ( used >= size )
    return READ_PERCENT( 100 );
  
  return  (S32)( ( used * 128 ) / size );
}

static U32 RADLINK Bink_io_thread( void * user_data )
{
  tmThreadName( tm_mask, TMZF_NONE, "Bink IO" );
  for(;;)
  {  
    S32 did_io = 0;

    tmEnter( tm_mask, TMZF_IDLE, "No IO to do" );
    rrSemaphoreDecrementOrWait( &bink_io_sema, RR_WAIT_INFINITE );
    tmLeave( tm_mask );

walk_list_again:
    while( ( !bink_io_close ) && ( bink_mutex_init ) )
    {
      S32 last_per;
      S32 has_need_io = 0;
      HBINK b;
      HBINK bl;
            
      our_lock( &bink_list, "Bink list mutex" );
      bl = bink_io_last;
      if ( ( bl ) && ( bl->needio ) && ( !bl->ReadError ) && ( ( bl->OpenFlags & BINKNOTHREADEDIO ) == 0 ) )
      {
        S32 locked_io = 0;
        ++has_need_io;

        tmEnter( tm_mask, TMZF_STALL, "Waiting for both io mutexes" );
        locked_io = our_lock_timeout( (rrMutex*) bl->pri_io_mutex, 1, "Bink instance priority io mutex" );
        if ( locked_io )
        {
          locked_io = our_lock_timeout( (rrMutex*) bl->io_mutex, 0, "Bink instance io mutex" );
          if ( !locked_io )
            our_unlock( (rrMutex*) bl->pri_io_mutex, "Bink instance priority io mutex" );
        }
        tmLeave( tm_mask );

        our_unlock( &bink_list, "Bink list mutex" );
        
         
        if ( locked_io )
        {
          // do we have to do a priority io on the last read bink
          if ( bl->needio )
          {
            if ( bink_do_pri_io( bl ) )
              did_io = 1;
            --has_need_io;
          }

          our_unlock( (rrMutex*) bl->io_mutex, "Bink instance io mutex" );
          our_unlock( (rrMutex*) bl->pri_io_mutex, "Bink instance priority io mutex" );
        }
      }
      else
      {
        our_unlock( &bink_list, "Bink list mutex" );
      }
        
      // do we have to do any other priority ios
      our_lock( &bink_list, "Bink list mutex" );
      b = bink_first;
      while ( b )
      {
        if ( ( b->needio ) && ( !b->ReadError ) && ( ( b->OpenFlags & BINKNOTHREADEDIO ) == 0 ) )
        {
          S32 bll = 1;
          S32 locked_io = 0;
          ++has_need_io;

          tmEnter( tm_mask, TMZF_STALL, "Waiting for both io mutexes" );
          locked_io = our_lock_timeout( (rrMutex*) b->pri_io_mutex, 1, "Bink instance priority io mutex" );
          if ( locked_io )
          {
            locked_io = our_lock_timeout( (rrMutex*) b->io_mutex, 0, "Bink instance io mutex" );
            if ( !locked_io )
              our_unlock( (rrMutex*) b->pri_io_mutex, "Bink instance priority io mutex" );
          }
          tmLeave( tm_mask );

          if ( locked_io )
          {
            our_unlock( &bink_list, "Bink list mutex" );

            if ( b->needio )
            {
              if ( bink_do_pri_io( b ) )
              {
                did_io = 1;
                bink_io_last = b;
              }
              --has_need_io;
            }
      
            bll = our_lock_timeout( &bink_list, 0, "Bink list mutex" );

            our_unlock( (rrMutex*) b->io_mutex, "Bink instance io mutex" );
            our_unlock( (rrMutex*) b->pri_io_mutex, "Bink instance priority io mutex" );

            if ( bll == 0 )
              goto walk_list_again;
          }
        }
        
        b = b->next_bink;
      }
      our_unlock( &bink_list, "Bink list mutex" );
      
      // if we haven't done an priority read, then background read
      if ( !did_io )
      {
        S32 locked = 1;

        our_lock( &bink_list, "Bink list mutex" );
        bl = bink_io_last;
        if ( ( bl ) && ( !bl->ReadError ) && ( ( bl->OpenFlags & BINKNOTHREADEDIO ) == 0 ) )
        {
          if ( our_lock_timeout( (rrMutex*) bl->io_mutex, 0, "Bink instance io mutex" ) )
          {
            locked = 0;
            our_unlock( &bink_list, "Bink list mutex" );

            if ( !bl->needio )
            {
              if ( bink_do_io( bl ) )
                did_io = 1;
              our_unlock( (rrMutex*) bl->io_mutex, "Bink instance io mutex" );
            }
            else
            {
              our_unlock( (rrMutex*) bl->io_mutex, "Bink instance io mutex" );
              goto walk_list_again;
            }
          }
        }
        if ( locked )
          our_unlock( &bink_list, "Bink list mutex" );
      }

      // check if we should switch to other binks
      if ( !has_need_io )
      {
        S32 locked = 1;
        S32 small_per = READ_PERCENT( 101 );
        HBINK sb = bink_io_last;

        our_lock( &bink_list, "Bink list mutex" );
        last_per = GET_READ_PERCENT( bink_io_last );
        
        b = bink_first;
        while ( b )
        {
          S32 this_per = GET_READ_PERCENT( b );
         
          if ( ( this_per < small_per ) && ( ( b->OpenFlags & BINKNOTHREADEDIO ) == 0 ) && ( !b->needio ) )
          {
            small_per = this_per;
            sb = b;
          }
          b = b->next_bink;
        }
        
        // should we switch?
        if ( ( ( last_per > READ_PERCENT( 80 ) ) && ( small_per < READ_PERCENT( 20 ) ) ) || 
             ( ( last_per > READ_PERCENT( 95 ) ) && ( small_per < READ_PERCENT( 50 ) ) ) || 
               ( small_per < READ_PERCENT( 10 ) ) )
        {

          bink_io_last = sb;
          bl = sb;
          
          if ( ( bl ) && ( !bl->ReadError ) && ( ( bl->OpenFlags & BINKNOTHREADEDIO ) == 0 ) )
          {
            if ( our_lock_timeout( (rrMutex*) bl->io_mutex, 0, "Bink instance io mutex" ) )
            {
              locked = 0;
              our_unlock( &bink_list, "Bink list mutex" );
              if ( !bl->needio )
              {
                if ( bink_do_io( bl ) )
                  did_io = 1;
                our_unlock( (rrMutex*) bl->io_mutex, "Bink instance io mutex" );
              }
              else
              {
                our_unlock( (rrMutex*) bl->io_mutex, "Bink instance io mutex" );
                goto walk_list_again;
              }
            }
          }
        }
        if ( locked )
          our_unlock( &bink_list, "Bink list mutex" );
      }

      if ( ( !did_io ) && ( !has_need_io ) )
        break;

      // for the next loop around
      did_io = 0;
    }

    if ( bink_io_close )
      return 1;
  }    
}

static S32 setup_io_thread( HBINK bink )
{
  S32 ret = 0;
  
  lock_io_global();
  
  if ( bink_io_thread_init )
  {
    ++bink_io_setup_count;
    ret = 1;
  }
  else
  {
    bink_io_close = 0;
  
    if ( rrSemaphoreCreate( &bink_io_sema, 0, 16384 ) )
    {
      #if defined(RR_CAN_ALLOC_THREAD_STACK)
      alloc_rr_thread_stack( bink, stack_for_io_thread, 32*1024 );
      if ( rrThreadCreateWithStack( &bink_rr_thread, Bink_io_thread, 0, 32*1024, stack_for_io_thread,
                                    rrThreadPriorityAdd(rrThreadNormal,1), BINK_IO_CORE, "Bink IO" ) )
      #else
      if ( rrThreadCreate( &bink_rr_thread, Bink_io_thread, 0, 32*1024, 
                           rrThreadPriorityAdd(rrThreadNormal,1), BINK_IO_CORE, "Bink IO" ) )
      #endif  
      {
        ++bink_io_setup_count;
        bink_io_thread_init = 1;
        ret = 1;
      }
      else
      {
        dealloc_rr_thread_stack( stack_for_io_thread );
        rrSemaphoreDestroy( &bink_io_sema );
      }
    }
  }
 
  unlock_io_global();

  return ret;
}

static void close_io_thread( void )
{
  lock_io_global();

  if ( bink_io_thread_init )
  {
    bink_io_close = 1;
    rrSemaphoreIncrement( &bink_io_sema, 4 );
    rrThreadDestroy( &bink_rr_thread );
    dealloc_rr_thread_stack( stack_for_io_thread );

    rrSemaphoreDestroy( &bink_io_sema );

    bink_io_setup_count = 0;
    bink_io_thread_init = 0;
  }

  unlock_io_global();
}

static void check_io_thread_close( S32 on_zero_close )
{
  lock_io_global();
  
  --bink_io_setup_count;
  
  if ( ( bink_io_setup_count == 0 ) && ( on_zero_close ) )
    close_io_thread();

  unlock_io_global();
}

static void wake_io_thread( void )
{
  if ( bink_io_thread_init )
    rrSemaphoreIncrement( &bink_io_sema, 1 );
}

static void RADLINK bink_suspend_io(struct BINKIO * Bnkio)
{
  HBINK b = (HBINK)(((char*)Bnkio)-(SINTa)&((HBINK)0)->bio);
  if ( b->OpenFlags & BINKNOTHREADEDIO )
    return;
  our_lock( (rrMutex*) b->io_mutex, "Bink instance io mutex" );
}


static S32  RADLINK bink_try_suspend_io(struct BINKIO * Bnkio)
{
  HBINK b = (HBINK)(((char*)Bnkio)-(SINTa)&((HBINK)0)->bio);

  if ( b->OpenFlags & BINKNOTHREADEDIO )
    return 1;
  
  return our_lock_timeout( (rrMutex*) b->io_mutex, 0, "Bink instance io mutex" );
}


static void RADLINK bink_resume_io(struct BINKIO * Bnkio)
{
  HBINK b = (HBINK)(((char*)Bnkio)-(SINTa)&((HBINK)0)->bio);
  if ( b->OpenFlags & BINKNOTHREADEDIO )
    return;
  our_unlock( (rrMutex*) b->io_mutex, "Bink instance io mutex" );
}


static void RADLINK bink_idle_on_io(struct BINKIO * Bnkio)
{
  HBINK b = (HBINK)(((char*)Bnkio)-(SINTa)&((HBINK)0)->bio);

  if ( b->OpenFlags & BINKNOTHREADEDIO )
    return;
  
  if ( our_lock_timeout( (rrMutex*) b->io_mutex, 1, "Bink instance io mutex" ) )
    our_unlock( (rrMutex*) b->io_mutex, "Bink instance io mutex" );
}

static void init_io_service_for_bink( HBINK bp )
{
  bp->bio.suspend_callback = bink_suspend_io;
  bp->bio.resume_callback = bink_resume_io;
  bp->bio.idle_on_callback = bink_idle_on_io;

  if ( ( ( bp->OpenFlags & BINKNOTHREADEDIO ) == 0 ) && ( bp->preloadptr == 0 ) )
  {
    RR_COMPILER_ASSERT( sizeof( bp->io_mutex ) >= sizeof( rrMutex ) );
    
    if ( rrMutexCreate( (rrMutex*) bp->io_mutex, rrMutexNeedFullTimeouts ) )
    {
       if ( !setup_io_thread( bp ) )
       {
         rrMutexDestroy( (rrMutex*) bp->io_mutex );
         bp->OpenFlags |= BINKNOTHREADEDIO; // fail
       }
       else
       {
         tmLockName( tm_mask, bp->io_mutex, "Bink instance io mutex" );
       }
    }
    else
    {
      bp->OpenFlags |= BINKNOTHREADEDIO; // fail
    }

    if ( ! ( bp->OpenFlags & BINKNOTHREADEDIO ) )
    {
      place_in_bink_list( bp );
      bp->in_bink_list = 1;
    }
       
  }

  if ( rrMutexCreate( (rrMutex*) bp->pri_io_mutex, rrMutexNeedFullTimeouts ) == 0 )
    RR_BREAK();  // have to have this mutex
  tmLockName( tm_mask, bp->pri_io_mutex, "Bink instance priority io mutex" );

  bink_io_last = bp;
}

static void close_io_service_for_bink( HBINK bink )
{
  if ( ( bink->OpenFlags & BINKNOTHREADEDIO ) == 0 )
    rrMutexDestroy( (rrMutex*) bink->io_mutex );

  if ( ( bink->OpenFlags & BINKFILEDATALOADONLY ) == 0 )
    rrMutexDestroy( (rrMutex*) bink->pri_io_mutex );

  if ( ( bink->OpenFlags & BINKNOTHREADEDIO ) == 0 )
    check_io_thread_close( ( bink->OpenFlags & BINKDONTCLOSETHREADS ) == 0 );
}

static void free_io_globals(void)
{
  S32 lt = 0;
  if ( bink_mutex_init )
  {
    lt = 1;
    lock_io_global();
  }

  if ( ( bink_io_setup_count == 0 ) && ( bink_io_thread_init ) )
   close_io_thread();

  if ( ( lt ) && ( bink_mutex_init ) )
    unlock_io_global();

  cleanup_bink_mutex();
}

static void sync_io_service( HBINK bink )
{
  // blip the io thread
  if ( ( bink->OpenFlags & BINKNOTHREADEDIO ) == 0 )
  {
    lock_two_mutexes( &bink_list, "Bink list mutex", (rrMutex*) bink->io_mutex, "Bink instance io mutex" );
    if ( bink_io_last == bink )
      bink_io_last = 0;
    our_unlock( (rrMutex*) bink->io_mutex, "Bink instance io mutex" );
    our_unlock( &bink_list, "Bink list mutex" );
  }

  if ( bink_io_last == bink )
    bink_io_last = 0;
}

static S32 get_io_thread( void * output )
{
  if ( bink_io_thread_init )
  {
    rrThreadGetOSThreadID( &bink_rr_thread, output );
    return 1;
  }
  return 0;
}  

#else

// when not using background IO, always ok
static S32  RADLINK bink_try_suspend_io(struct BINKIO * Bnkio)
{
  return 1;
}

#define init_io_service_for_bink( bp ) { bp->OpenFlags |= BINKNOTHREADEDIO; }
#define wake_io_thread() {}
#define close_io_service_for_bink( bink )

#define free_io_globals()
#define sync_io_service(b)

#define get_io_thread(v) 0

#endif

//===================================


//===================================

#ifdef USE_SND_THREAD

static S32 volatile bink_snd_thread_init = 0;
static S32 volatile bink_snd_setup_count = 0;

#ifdef RR_CAN_ALLOC_THREAD_STACK
static void * stack_for_snd_thread;
#endif
static rrThread bink_snd_rr_thread;

static S32 volatile bink_snd_close = 0;
static rrSemaphore bink_snd_sema;
static U32 volatile snd_wait_time = 13;

static void set_default_snd_wait_time( void )
{
  snd_wait_time = 13; 
}

static void set_min_snd_wait_time( U32 ms )
{
  if ( ms < snd_wait_time )
    snd_wait_time = ms;
}

static void set_idle_snd_wait_time( void )
{
  snd_wait_time = RR_WAIT_INFINITE; 
}

#ifdef __RADIPHONE__
RADEXPFUNC void RADEXPLINK BinkIOSPostAudioInterruptEnd( void )
{
  if ( ( !bink_snd_close ) && ( bink_mutex_init ) )
  {
    our_lock( &bink_list, "Bink list mutex" );

    void bink_coreaudio_ios_resume();    
    bink_coreaudio_ios_resume();

    our_unlock( &bink_list, "Bink list mutex" );
  }
}
#endif // __RADIPHONE__

static void RADLINK checksound(BINK*b);

static U32 pump_sound( void )
{
walk_list_again:
  if ( ( !bink_snd_close ) && ( bink_mutex_init ) )
  {
    HBINK b;
    BINKSNDUNLOCK do_global_mix = 0;
    
    // do we have to do any snd
    our_lock( &bink_list, "Bink list mutex" );
    b = bink_first;
    while ( b ) 
    {
      if ( ( !b->closing ) && ( b->playingtracks ) )
      {
        if ( our_lock_timeout( (rrMutex*) b->snd_mutex, 0, "Bink instance sound mutex" ) )
        {
          S32 bll = 1;

          our_unlock( &bink_list, "Bink list mutex" );
          
          set_default_snd_wait_time();
          
          if ( ( !b->closing ) && ( b->playingtracks ) )
          {
            if ( ( b->bsnd[0].ThreadServiceType == 0 ) || ( b->bsnd[0].ThreadServiceType == 2 ) )
            {
              tmEnter( tm_mask, TMZF_NONE, "Sound mix" );
              checksound(b);
              tmLeave( tm_mask );
            } 
            else if ( b->bsnd[0].ThreadServiceType < 0 )
            {
              do_global_mix = b->bsnd[0].Unlock;
              set_min_snd_wait_time( -b->bsnd[0].ThreadServiceType );
            }

          }

          bll = our_lock_timeout( &bink_list, 2, "Bink list mutex" );

          our_unlock( (rrMutex*) b->snd_mutex, "Bink instance sound mutex" );
          
          if ( bll == 0 )
            goto walk_list_again;
        }
        else
        {
          set_min_snd_wait_time( 2 );
        }
      }
      b = b->next_bink;
    }

    // release the list lock
    our_unlock( &bink_list, "Bink list mutex" );

    // actually do the global mix, and release
    if ( do_global_mix )
    {
      if ( our_lock_timeout( &bink_snd_global, 0, "Bink global sound mutex" ) )
      {
        tmEnter( tm_mask, TMZF_NONE, "Global sound mix" );
        do_global_mix( 0, 0 );
        tmLeave( tm_mask );
        unlock_snd_global();
      }
      else
      {
        set_min_snd_wait_time( 2 );
      }
    }

  }

  return ( bink_snd_close ) ? 1 : 0 ;
}


static U32 RADLINK Bink_snd_thread( void * user_data )
{
  tmThreadName( tm_mask, 0, "Bink Snd" );
  for(;;)
  {  
    tmEnter( tm_mask, TMZF_IDLE, "Sound mix delay" );
    rrSemaphoreDecrementOrWait( &bink_snd_sema, snd_wait_time );
    tmLeave( tm_mask );

    if ( pump_sound() )
      return 1;
  }    
}

static S32 setup_snd_thread( HBINK bink )
{
  S32 ret = 0;
  
  lock_snd_global();

  if ( bink_snd_thread_init )
  {
    ++bink_snd_setup_count;
    ret = 1;
  }
  else
  {
    bink_snd_close = 0;

    if ( rrSemaphoreCreate( &bink_snd_sema, 0, 16384 ) )
    {
      #if defined(RR_CAN_ALLOC_THREAD_STACK)
      alloc_rr_thread_stack( bink, stack_for_snd_thread, 32*1024 );
      if ( rrThreadCreateWithStack( &bink_snd_rr_thread, Bink_snd_thread, 0, 32*1024, stack_for_snd_thread,
                                    rrThreadPriorityAdd(rrThreadNormal,2), BINK_SND_CORE, "Bink Snd" ) )
      #else
      if ( rrThreadCreate( &bink_snd_rr_thread, Bink_snd_thread, 0, 32*1024, 
                           rrThreadPriorityAdd(rrThreadNormal,2), BINK_SND_CORE, "Bink Snd" ) )
      #endif
      {
        ++bink_snd_setup_count;
        bink_snd_thread_init = 1;
        ret = 1;
      }
      else
      {
        dealloc_rr_thread_stack( stack_for_snd_thread );
        rrSemaphoreDestroy( &bink_snd_sema );
      }
    }
  }
 
  unlock_snd_global();

  return ret;
}

static void close_snd_thread( void )
{
  lock_snd_global();

  if ( bink_snd_thread_init )
  {
    bink_snd_close = 1;
    rrSemaphoreIncrement( &bink_snd_sema, 4 );
    rrThreadDestroy( &bink_snd_rr_thread );
    dealloc_rr_thread_stack( stack_for_snd_thread );

    rrSemaphoreDestroy( &bink_snd_sema );
    bink_snd_setup_count = 0;
    bink_snd_thread_init = 0;
  }

  unlock_snd_global();
}

static void check_snd_thread_close( S32 on_zero_close )
{
  lock_snd_global();
  
  --bink_snd_setup_count;
  
  if ( bink_snd_setup_count == 0 )
  {
    if ( on_zero_close )
      close_snd_thread();
    else
      set_idle_snd_wait_time();
  }

  unlock_snd_global();
}

static S32 bink_try_suspend_snd( HBINK b )
{
  if ( ( b->playingtracks ) && ( ( b->bsnd[0].ThreadServiceType <= 0 ) || ( b->bsnd[0].ThreadServiceType == 2 ) ) )
    return our_lock_timeout( (rrMutex*)b->snd_mutex, 0, "Bink instance sound mutex" );
  
  return 0;
}

static void bink_suspend_snd( HBINK b )
{
  if ( ( b->playingtracks ) && ( ( b->bsnd[0].ThreadServiceType <= 0 ) || ( b->bsnd[0].ThreadServiceType == 2 ) ) )
    our_lock( (rrMutex*)b->snd_mutex, "Bink instance sound mutex"  );
}

static void bink_resume_snd( HBINK b )
{
  if ( ( b->playingtracks ) && ( ( b->bsnd[0].ThreadServiceType <= 0 ) || ( b->bsnd[0].ThreadServiceType == 2 ) ) )
    our_unlock( (rrMutex*) b->snd_mutex, "Bink instance sound mutex" );
}

static void init_sound_service_for_bink( HBINK bp )
{
  if ( bp->bsnd[0].ThreadServiceType <= 0 )
  {
    RR_COMPILER_ASSERT( sizeof( bp->snd_mutex ) >= sizeof( rrMutex ) );
    
    if ( rrMutexCreate( (rrMutex*) bp->snd_mutex, rrMutexNeedOnlyZeroTimeouts ) )
    {
      tmLockName( tm_mask, bp->snd_mutex, "Bink instance sound mutex" );
      
      // clear the sound wait time
      if ( bink_first == 0 )
        set_default_snd_wait_time();

      if ( !setup_snd_thread( bp ) )
      {
        rrMutexDestroy( (rrMutex*) bp->snd_mutex );
        bp->bsnd[0].ThreadServiceType = 1;
      }
      else
      {
        place_in_bink_list( bp );
        bp->in_bink_list = 1;
      } 

      if ( bp->bsnd[0].ThreadServiceType < 0 )
        set_min_snd_wait_time( -bp->bsnd[0].ThreadServiceType );
    }
    else
    {
      bp->bsnd[0].ThreadServiceType = 1;
    }
  }
  else if ( bp->bsnd[0].ThreadServiceType == 2 )
  {
    if ( rrMutexCreate( (rrMutex*) bp->snd_mutex, rrMutexNeedOnlyZeroTimeouts ) )
    {
      tmLockName( tm_mask, bp->snd_mutex, "Bink instance sound mutex" );

      place_in_bink_list( bp );
      bp->in_bink_list = 1;
    }
    else
    {
      place_in_bink_list( bp );
      bp->in_bink_list = 1;
    } 
  }
}

static void wake_snd_thread( HBINK bp )
{
  if ( ( bp->playingtracks ) && ( bink_snd_thread_init ) && ( bp->bsnd[0].ThreadServiceType <= 0 ) )
    rrSemaphoreIncrement( &bink_snd_sema, 1 );
}
 
static void close_snd_service_for_bink( HBINK bink )
{
  if ( ( bink->playingtracks ) && ( ( bink->bsnd[0].ThreadServiceType <= 0 ) || ( bink->bsnd[0].ThreadServiceType == 2 ) ) )
    rrMutexDestroy( (rrMutex*) bink->snd_mutex );
  if ( ( bink->playingtracks ) && ( bink->bsnd[0].ThreadServiceType <= 0 ) )
    check_snd_thread_close( ( bink->OpenFlags & BINKDONTCLOSETHREADS ) == 0 );
}

static void free_snd_globals(void)
{
  S32 lt = 0;
  if ( bink_mutex_init )
  {
    lt = 1;
    lock_snd_global();
  }

  if ( ( bink_snd_setup_count == 0 ) && ( bink_snd_thread_init ) )
   close_snd_thread();

  if ( ( lt ) && ( bink_mutex_init ) )
    unlock_snd_global();
}

static void sync_snd_service( HBINK bink )
{
  // blip the snd thread
  if ( ( bink->playingtracks ) && ( ( bink->bsnd[0].ThreadServiceType <= 0 ) || ( bink->bsnd[0].ThreadServiceType == 2 ) ) )
  {
    lock_two_mutexes( &bink_list, "Bink list mutex", (rrMutex*) bink->snd_mutex, "Bink instance sound mutex" );
    our_unlock( (rrMutex*) bink->snd_mutex, "Bink instance sound mutex" );
    our_unlock( &bink_list, "Bink list mutex" );
  }
}

static S32 get_snd_thread( void * output )
{
  if ( bink_snd_thread_init )
  {
    rrThreadGetOSThreadID( &bink_snd_rr_thread, output );
    return 1;
  }
  return 0;
}

#else

#define bink_try_suspend_snd(s) 1
#define bink_suspend_snd(s)
#define bink_resume_snd(s)

#define init_sound_service_for_bink(b)
#define wake_snd_thread(b)
#define close_snd_service_for_bink(b)

#define free_snd_globals()
#define sync_snd_service(b)
#define get_snd_thread(v) 0

#endif

//===================================


//===================================

static BINKSNDSYSOPEN  sysopen=0;
static BINKSNDSYSOPEN2 sysopen2=0;
static BINKSNDOPEN sndopen=0;
static U32 volatile numopensounds=0;


RADEXPFUNC S32  RADEXPLINK BinkSetSoundSystem(BINKSNDSYSOPEN open, UINTa param)
{
  BINKSNDOPEN so;

  if (open==0)
    return(0);

  setup_bink_mutex();
  lock_snd_global();

  if ((sysopen2==0) && (sysopen==0))
    sysopen=open;

  if (sysopen!=open)
  {
    if (numopensounds==0)
      sysopen=open;
    else
    {
      unlock_snd_global();
      return(0);
    }
  }

  so=sysopen(param);

  sndopen=so;

  unlock_snd_global();
  return( (sndopen)?1:0 );
}

RADEXPFUNC S32  RADEXPLINK BinkSetSoundSystem2(BINKSNDSYSOPEN2 open, UINTa param1, UINTa param2 )
{
  BINKSNDOPEN so;

  if (open==0)
    return(0);

  setup_bink_mutex();
  lock_snd_global();

  if ((sysopen2==0) && (sysopen==0))
    sysopen2=open;

  if (sysopen2!=open)
  {
    if (numopensounds==0)
      sysopen2=open;
    else
    {
      unlock_snd_global();
      return(0);
    }
  }

  so=sysopen2(param1,param2);

  sndopen=so;

  unlock_snd_global();
  return( (sndopen)?1:0 );
}

static void dosilencefillorclamp(BINKSND*b)
{
  S32 sndamt;

  sndamt = (S32) ( b->sndwritepos - b->sndreadpos );
  if ( sndamt < 0 )
    sndamt += b->sndbufsize;
  
  sndamt &= ~63;
  
  if ( sndamt < b->sndprime )  // no enough sound primed?
  {
    // prime up some silence
    U32 amt = b->sndprime - sndamt;

    b->sndreadpos -= amt;
    if ( b->sndreadpos < b->sndbuf )
    {
      U32 amtl = (U32) (b->sndbuf - b->sndreadpos );
      b->sndreadpos += b->sndbufsize;
      rrmemsetzerobig( b->sndbuf, amt-amtl );
      rrmemsetzerobig( b->sndreadpos, amtl );
    }
    else
    {
      rrmemsetzerobig( b->sndreadpos, amt );
    }
  }
  else if ( sndamt > b->sndprime ) // do we have too much sound primed?
  {
    // shrink the buffer amount
    U32 amt = sndamt - b->sndprime;
    b->sndreadpos += amt;

    if ( b->sndreadpos > ( b->sndbuf + b->sndbufsize ) )
      b->sndreadpos -= b->sndbufsize;
  }
}


#define ServiceSound(b) {if ((b->playingtracks) && ( b->bsnd[0].ThreadServiceType == 1 ) ) checksound(b);}

static void RADLINK checksound(BINK*b)
{
  if ( (!(b->bsnd[b->playingtracks-1].Ready)) || ( b->playedframes == 0 ) )
    return;

  if ( bink_try_suspend_snd( b ) )
  {
    U32 i;

    for( i = 0 ; i < b->playingtracks ; i++ )
    {
      BINKSND * bsnd;

      bsnd = &b->bsnd[i];
      
      // check the audio status
      for(;;)
      {
        U8* in;
        U32 insize,left;

        S32 sndamt;
        if (!(bsnd->Ready(bsnd)))
          break;

        sndamt = (S32) ( bsnd->sndwritepos - bsnd->sndreadpos );
        if ( sndamt < 0 )
          sndamt += bsnd->sndbufsize;

        if (
             ( sndamt <= bsnd->BestSize ) 
           &&
             (
               ( (S32) b->FrameNum <= (S32) bsnd->sndendframe )
             || 
               ( !sndamt )
             )
           )
          break;

        if (bsnd->Lock(bsnd,&in,&insize))
        {
          if (insize>(U32)sndamt)
            insize=sndamt;

          // Make sure we handle any alignment requirements people have
          if ((S32)b->FrameNum<=(S32)bsnd->sndendframe)
            insize&=(bsnd->BestSizeMask);

          left=(U32)(bsnd->sndend-bsnd->sndreadpos);
          if (left<insize)
          {
            if (left)
            {
              rrmemmovebig(in,bsnd->sndreadpos,left);
              in+=left;
            }
            rrmemmovebig(in,bsnd->sndbuf,insize-left);
            bsnd->sndreadpos=bsnd->sndbuf+(insize-left);
          }
          else
          {
            rrmemmovebig(in,bsnd->sndreadpos,insize);
            bsnd->sndreadpos+=insize;
          }

          bsnd->Unlock(bsnd,insize);
        }
        else
        {
          break;
        }
      }
    }
    bink_resume_snd( b );
  }
}

//===================================

static void inittimer( BINK * b )
{
  // set up the initial time, if necessary
  if ( b->startsynctime == 0 )
  {
    b->startsynctime = RADTimerRead();
    b->startsyncframe = b->playedframes - 1;
    b->paused_sync_diff = 0;
  }
}

void check_for_pending_io( HBINK bink );
void check_for_pending_io( HBINK bink )
{
  if ( bink->needio )
  {
    tmEnter( tm_mask, TMZF_STALL, "Wait for frame IO to complete" );

    our_lock( (rrMutex*) bink->pri_io_mutex, "Bink instance priority io mutex" );

    if ( bink->needio ) // check again after lock
    {
      bink->bio.ReadFrame(&bink->bio,bink->compframenum,bink->compframeoffset,bink->compframe,bink->compframesize);
      bink->lastfileread = bink->compframeoffset + bink->compframesize;
      bink->needio = 0;
      if (bink->bio.ReadError)
        bink->ReadError=1;
    }

    our_unlock( (rrMutex*) bink->pri_io_mutex, "Bink instance priority io mutex" );
    tmLeave( tm_mask );

    wake_io_thread();
  }
}

static void GotoFrame(BINK* b,U32 frame)
{
  if (frame)
    --frame;

  b->slices.slice_inc = 0;
  b->skipped_status_this_frame = 0;  //0=not checked this frame, 1=no skip, 2=skip
  ServiceSound(b);

  b->LastFrameNum=b->FrameNum;

  if (b->ReadError)
  {
    b->FrameNum=frame+1;
    return;
  }
  
  check_for_pending_io( b );

  if ( frame == 0 )
  {
    U32 t = RADTimerRead();

    if ( b->lastresynctime == 0 )
      b->lastresynctime = t;

    if ( ( t - b->lastresynctime ) > 300000 )  // resync every 5 minutes
    {
      b->lastresynctime = t;
      b->doresync = 1;
    }
  }

  b->compframenum = frame + 1;
  b->compframeoffset = b->frameoffsets[ frame ] & ~3;
  b->compframekey = ((U32)b->frameoffsets[ frame ]) & 1;
  b->compframesize = (U32) ( ( b->frameoffsets[ frame + 1 ] & ~3 ) - b->compframeoffset );
  
  if (b->preloadptr)
    b->compframe = ((U8 *)b->preloadptr) + ( b->compframeoffset - (b->frameoffsets[0]&~3) );
  else
  {
    U32 readtoalign;
    readtoalign = OSFRAMEREADALIGN - (((U32)b->compframeoffset)&(OSFRAMEREADALIGN-1));
    b->compframe = (void*) ( ((((UINTa)b->alloccompframe)+OSFRAMEREADALIGN+OSFRAMEREADALIGN)&~(OSFRAMEREADALIGN-1)) - readtoalign);

    if ( ( b->compframeoffset < b->lastfileread ) || ( ( b->compframeoffset + b->compframesize ) > ( b->lastfileread + b->bio.CurBufUsed ) ) )
    {
      // schedule to do io on background thread
      b->needio = 1;

      wake_io_thread();
    }
    else
    {
      // data for IO is in cache, so we don't need to schedule it
      b->needio = 1;
      check_for_pending_io( b );
    }
  }

  ServiceSound(b);

#ifndef NTELEMETRY
  if ( tmRunning() )
  {
    U32 frames;
    
    frames=(b->runtimeframes-1);
    if (frames>b->FrameNum)
      frames=b->FrameNum-1;
    if (frames==0)
      frames=1;

    #ifndef NTELEMETRY
    {
      tmPlot( tm_mask, TMPT_MEMORY, 0, (F32)b->bio.CurBufSize, "Bink %d/%s/%s (%s)", (U32)b->bink_unique, "IO buffer","Size","ratemem" );
      tmPlot( tm_mask, TMPT_MEMORY, 0, (F32)b->bio.CurBufUsed, "Bink %d/%s/%s (%s)", (U32)b->bink_unique, "IO buffer","In Use","ratemem" );
      tmPlot( tm_mask, TMPT_PERCENTAGE_DIRECT, 0, (F32)b->bio.CurBufUsed/(F32)(b->bio.CurBufSize+1.0f), "Bink %d/%s/%s", (U32)b->bink_unique, "IO buffer", "Percent Full" );
      if ( (S32)b->FrameNum > (S32)frames )
        tmPlot( tm_mask, TMPT_MEMORY, 0, (F32)(((b->frameoffsets[b->FrameNum]&~3)-(b->frameoffsets[b->FrameNum-frames]&~3))*b->fileframerate/frames*b->fileframeratediv), "Bink %d/%s (%s)", (U32)b->bink_unique, "Data rate","ratemem" ); 
    }
    #endif
  }
#endif
  
  b->FrameNum=frame+1;
}

//error buffer
static char binkerr[256];

#define BinkClearError() *binkerr=0
#define BinkErrorIsCleared() (*binkerr==0)

//sets the BinkError
RADEXPFUNC void RADEXPLINK BinkSetError(const char * err)
{
  char * d;
  int i = 0;
  d = binkerr;
  if ( err == 0 )
    err = "Bink Error";
  for(;;)
  {
    *d = *err;
    if ( ++i == 255 ) 
    {
      d[0]=0;
      break;
    }
    if ( *err == 0)
      break;
    ++d;
    ++err;
  }
}

//gets the BinkError
RADEXPFUNC char * RADEXPLINK BinkGetError(void)
{
  return(binkerr);
}

static U64 gFileOffset=0;
static S32 gForceRate=-1;
static S32 gForceRateDiv;
static S64 gIOBufferSize=-1;
static S32 gSimulate=-1;
#define MAXTRACKS 32
static U32 gTotTracks=1;
static S32 gTrackNums[MAXTRACKS]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static BINKIOOPEN gUserOpen=0;


RADEXPFUNC void RADEXPLINK BinkSetFrameRate(U32 forcerate,U32 forceratediv)
{
  gForceRate=forcerate;
  gForceRateDiv=forceratediv;
}


RADEXPFUNC void RADEXPLINK BinkSetFileOffset(U64 offset)
{
  gFileOffset = offset;
}


RADEXPFUNC void RADEXPLINK BinkSetIOSize(U64 iosize)
{
  gIOBufferSize=iosize;
}


RADEXPFUNC void RADEXPLINK BinkSetIO(BINKIOOPEN io)
{
  gUserOpen=io;
}


RADEXPFUNC void RADEXPLINK BinkSetSimulate(U32 simulate)
{
  gSimulate=simulate;
}


RADEXPFUNC void RADEXPLINK BinkSetSoundTrack(U32 total_tracks, U32 * tracks)
{
  U32 i;

  if ( total_tracks > MAXTRACKS )
    total_tracks = MAXTRACKS;

  gTotTracks = total_tracks;

  for( i = 0 ; i < total_tracks ; i++ )
  {
    gTrackNums[i] = tracks[i];
  }
}


static U64 high1secrate(U32 num, U64 * ofs, U32* ofs32,U32 over,U32* frame,S32* allkeys,U64 * size)
{
  S32 i,last;
  U64 largest=0;
  U64 adjust=0;
  U32 fnum=0;
  S32 ak=1;

  // copy into 64-bit
  for(i=num;i>=0;i--)
  {
    ofs[i]=ofs32[i];
  }

  // now expand wrapped-32s to 64s
  for(i=0;i<(S32)num;i++)
  {
    U64 ofsi;
    S64 cur;

    ofsi = ofs[i];
    ofs[i] += adjust;

    // keep track of all-keys
    ak &= (((S32)ofsi)&1);
    
    cur=(ofs[i+1]&~3)-(ofsi&~3);  // &2 == future tweakable

    // check for bad frames sizes 
    if ( cur > 0x10000000 )
      return (U64)(S64)-1;    

    if ( cur <= 0 )
      adjust += 0x100000000ULL;
  }
  ofs[num]+=adjust;
  *size += adjust;

  last = num-over;
  if ( last < 0 )
    last = 0;

  // find largest data rate
  for(i=0;i<last;i++)
  {
    U64 cur;

    cur=(ofs[over+i]&~3)-(ofs[i]&~3);  // &2 == future tweakable

    if (cur>largest)
    {
      largest=cur;
      fnum=i;
    }
  }

  if (largest==0)
    largest=((ofs[num]&~3)-(ofs[0]&~3))*over/num;

  *allkeys=ak;

  *frame=fnum;
  return largest;
}

//#define BINKLOG
#ifdef BINKLOG

#ifdef __RADFINAL__
#error "You have logging turned on!"
#endif

static void BinkLog(char* str)
{
  DWORD wrote;
  HANDLE h=CreateFile("BINKLOG.TXT",GENERIC_WRITE,0,0,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_WRITE_THROUGH,0);
  SetFilePointer(h,0,0,FILE_END);
  WriteFile(h,str,radstrlen(str),&wrote,0);
  WriteFile(h,"\r\n",2,&wrote,0);
  FlushFileBuffers(h);
  CloseHandle(h);
}

#else

#define BinkLog(str)

#endif


#if !defined(DLL_FOR_GENERIC_OS)

static void RADLINK bink_simulate_io(struct BINKIO * Bnkio, U64 amt, U32 timer )
{
  HBINK b = (HBINK)(((char*)Bnkio)-(SINTa)&((HBINK)0)->bio);
  U32 rt;
  U64 wait;
  U32 t;
  
  if ( b->simulate <= 0 )
    return;
  
  wait = ( amt * 1000 / b->simulate );
  if ( wait > 0xffffffff )
    t = 0xffffffff;
  else
    t = (U32) wait;

  rt = RADTimerRead();

  b->adjustsim += t - ( (U32) ( rt - timer ) );

  while ( b->adjustsim > 0 )
  {
    rrThreadSleep( 1 );
    t = RADTimerRead();
    if ( t - rt )
    {
      b->adjustsim -= ( t - rt );
      rt = t;
    }
  }
}

#endif

static U32 RADLINK bink_timer( void )
{
  return RADTimerRead();
}

#ifdef __RADBIGENDIAN__

static void RADLINK bink_flipendian( void* dest, U64 size )
{
  U64 s = ( size + 3 ) / 4;
  U32 * RADRESTRICT d = dest;

  #ifdef fliptype
  fliptype v0,v1,v2,v3,sh;
  static RAD_ALIGN(U8,fb[16],16) = { 3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12 };
  loadvec( fb, sh );
  #endif

  tmEnter( tm_mask, TMZF_NONE, "Byte swap" );

  #ifdef fliptype
  // if using vector flips, pre-align dest address
  while ( ( (UINTa) d ) & 15 )
  {
    U32 v;
    if ( s == 0 )
      break;
      
    v = loadflip( d, 0 );
    *d++ = v;
    --s;
  }
  
  while( s >= 4*4 )
  {
    loadvec( d, v0 );
    loadvec( d+4, v1 );
    loadvec( d+8, v2 );
    loadvec( d+12, v3 );
    shufvec( v0, v0, v0, sh );
    shufvec( v1, v1, v1, sh );
    shufvec( v2, v2, v2, sh );
    shufvec( v3, v3, v3, sh );
    storevec( d, v0 );
    storevec( d+4, v1 );
    storevec( d+8, v2 );
    storevec( d+12, v3 );
    d += 16; // 4 vecs
    s -= 16; // 4 vecs
  }
  #endif

  while ( s-- )
  {
#ifdef loadflip
    U32 v = loadflip( d, 0 );
    *d++ = v;
#else
    U32 v = *d;
    *d++ = ( v >> 24 ) | ( ( v >> 8 ) & 0xff00 ) | ( ( v << 8 ) & 0xff0000 ) | ( v << 24 );
#endif
  }
  tmLeave( tm_mask );
}

RADEXPFUNC void RADEXPLINK BinkFlipEndian32( void * buffer, U64 bytes )
{
  bink_flipendian( buffer, bytes );
}

#endif

static U64 RADLINK bink_lockedadd( U64 volatile* dest, S64 amt )
{
  return rrAtomicAddExchange64( dest, amt );
}

static void RADLINK bink_memcpy( void * d, void const *s, U64 amt )
{
  while ( amt )
  {
    U32 b;
    b = ( amt > 0x80000000 ) ? 0x80000000 : (U32) amt;
    rrmemcpy( d, s, b );
    d = ((U8*)d) + b;
    s = ((U8*)s) + b;
    amt -= b;
  }
}

RADEXPFUNC void RADEXPLINK BinkGetFrameBuffersInfo( HBINK bink, BINKFRAMEBUFFERS * set )
{
  if ( ( bink == 0 ) || (set == 0 ) )
    return;
  
  set->FrameNum = 0;
  if ( is_binkv2_or_later( bink->marker ) )
  {
    // NOTE: if you change this, make sure BinkGetGPUDataBuffersInfo matches!
    set->YABufferWidth = ( bink->Width + 31 ) & ~31;
    set->YABufferHeight = ( bink->Height + 15 ) & ~15;
    set->cRcBBufferWidth = set->YABufferWidth / 2;
    set->cRcBBufferHeight = set->YABufferHeight / 2;
  }
  else
  {
    set->YABufferWidth = ( bink->Width + 7 ) & ~7;
    set->YABufferHeight = ( bink->Height + 7 ) & ~7;
    set->cRcBBufferWidth = ( ( ( bink->Width + 1 ) / 2 ) + 7 ) & ~7;
    set->cRcBBufferHeight = ( ( ( bink->Height + 1 ) / 2 ) + 7 ) & ~7;
  }

  set->TotalFrames = ( bink->Frames == 1 ) ? 1 : ( (bink->OpenFlags & BINKUSETRIPLEBUFFERING) ? 3 : 2 );

  if ( bink->FrameBuffers )
  {
    rrmemcpy( &set->Frames[ 0 ], &bink->FrameBuffers->Frames[ 0 ], sizeof( BINKFRAMEPLANESET )*BINKMAXFRAMEBUFFERS );
  }
  else
  {
    S32 i;
    rrmemset( &set->Frames[ 0 ], 0, sizeof( BINKFRAMEPLANESET )*BINKMAXFRAMEBUFFERS );    
    for( i = 0 ; i < set->TotalFrames ; i++ )
    {
      set->Frames[ i ].YPlane.Allocate = 1;
      set->Frames[ i ].cRPlane.Allocate = 1;
      set->Frames[ i ].cBPlane.Allocate = 1;
      set->Frames[ i ].APlane.Allocate = ( bink->OpenFlags & BINKALPHA ) ? 1 : 0;
      set->Frames[ i ].HPlane.Allocate = ( bink->OpenFlags & BINKHDR ) ? 1 : 0;
    }
  }
}

RADEXPFUNC void RADEXPLINK BinkRegisterFrameBuffers( HBINK bink, BINKFRAMEBUFFERS * set )
{
  if ( ( bink == 0 ) || (set == 0 ) )
    return;

  bink->OpenFlags &= ~BINKGPU;

  bink->FrameBuffers = set;

#if defined( NDA_CHECK_MEM )
  NDA_CHECK_MEM( set );
#endif    

}

RADEXPFUNC void RADEXPLINK BinkRegisterGPUDataBuffers( HBINK bink, BINKGPUDATABUFFERS * gpu )
{
#ifndef SUPPORT_BINKGPU
  BinkSetError("BinkGPU not supported.");
  return;
#else
  if ( bink == 0 )
    return;

  if ( bink->FrameBuffers )
  {
    return;
  }

  if ( !is_binkv22_or_later( bink->marker ) )
  {
    return;
  }

  bink->OpenFlags |= BINKGPU;
  bink->gpu = gpu;
#endif
}

static U32 ac_max_buffer_size( U32 num_macroblocks, U32 num_8x8s )
{
    U32 max_bytes_per_block32 =
        (U32)sizeof(U32) + // offset table entry
        (U32)sizeof(U32) + // flags
        num_8x8s * (
              (U32)sizeof(U64) + // AC nonzero mask
              64*(U32)sizeof(S16) // AC coeffs
             );
    
    return num_macroblocks * max_bytes_per_block32;
}

RADEXPFUNC S32 RADEXPLINK BinkGetGPUDataBuffersInfo( HBINK bink, BINKGPUDATABUFFERS * gpu )
{
#ifndef SUPPORT_BINKGPU
  BinkSetError("BinkGPU not supported.");
  return 0;
#else
  U32 num_macroblocks;
  U32 full_ac_size, sub_ac_size;
  if ( bink == 0 )
    return 0;

  if ( bink->FrameBuffers )
  {
    BinkSetError("Registered GPU buffers, while you have CPU buffers registered!");
    return 0;
  }

  if ( !is_binkv22_or_later( bink->marker ) )
  {
    BinkSetError("BinkGPU is only supported for Bink 2.2 and later files.");
    return 0;
  }

  gpu->BufferWidth = ( bink->Width + 31 ) & ~31;
  gpu->BufferHeight = ( bink->Height + 15 ) & ~15;
  gpu->ScanOrderTable = bink_zigzag_scan_order;

  num_macroblocks = ( ( bink->Width + 31 ) / 32 ) * ( ( bink->Height + 31 ) / 32 );
  full_ac_size = ac_max_buffer_size( num_macroblocks, 16 ); // AC size of a full-size plane
  sub_ac_size = ac_max_buffer_size( num_macroblocks, 4 ); // AC size of a subsampled plane

  gpu->AcBufSize[ BINKGPUPLANE_Y ] = full_ac_size;
  gpu->AcBufSize[ BINKGPUPLANE_CR ] = sub_ac_size;
  gpu->AcBufSize[ BINKGPUPLANE_CB ] = sub_ac_size;
  gpu->AcBufSize[ BINKGPUPLANE_A ] = ( bink->OpenFlags&BINKALPHA ) ? full_ac_size : 0;
  gpu->AcBufSize[ BINKGPUPLANE_H ] = ( bink->OpenFlags&BINKHDR ) ? full_ac_size : 0;

  return 1;
#endif  
}


#ifdef DUMPVOLUMES
#include <stdio.h>
#endif

RADEXPFUNC S32 RADEXPLINK BinkAllocateFrameBuffers( HBINK bp, BINKFRAMEBUFFERS * set, U32 minimum_alignment )
{
  S32 i, any;
  S32 ysubwidth, csubwidth;
  char pmbuf[ PushMallocBytesForXPtrs( 32 ) ];

  pushmallocinit( pmbuf, 32 );

  if ( bp->allocatedframebuffers )
    return 0;
  
#ifdef INC_BINK2
  if ( is_binkv2_or_later( bp->marker ) )
  {
    if ( minimum_alignment < RR_CACHE_LINE_SIZE )
      minimum_alignment = RR_CACHE_LINE_SIZE;
    
    ysubwidth = ( set->YABufferWidth + (minimum_alignment-1) ) & ~(minimum_alignment-1);
    csubwidth = ( set->cRcBBufferWidth + (minimum_alignment-1) ) & ~(minimum_alignment-1);
  }
  else
#endif
  {
    if ( minimum_alignment < 16 )
      minimum_alignment = 16;

    ysubwidth = set->YABufferWidth;
    csubwidth = ( set->cRcBBufferWidth + 15 ) & ~15;
  }

  // if we are using system memory buffering, we never need triple buffering...
  if ( set->TotalFrames == 3 )
    set->TotalFrames = 2;

  any = 0;
  for ( i = 0 ; i < set->TotalFrames ; i++ )
  {
    if ( set->Frames[ i ].YPlane.Allocate )
    {
      pushmalloc( pmbuf, &set->Frames[ i ].YPlane.Buffer, 
                  ( ysubwidth * set->YABufferHeight ) );
      set->Frames[ i ].YPlane.BufferPitch = ysubwidth;
      any = 1;
    }
    if ( set->Frames[ i ].cRPlane.Allocate )
    {
      pushmalloc( pmbuf, &set->Frames[ i ].cRPlane.Buffer, 
                  ( csubwidth * set->cRcBBufferHeight ) );
      set->Frames[ i ].cRPlane.BufferPitch = csubwidth;
      any = 1;
    }
    if ( set->Frames[ i ].cBPlane.Allocate )
    {
      pushmalloc( pmbuf, &set->Frames[ i ].cBPlane.Buffer, 
                  ( csubwidth * set->cRcBBufferHeight ) );
      set->Frames[ i ].cBPlane.BufferPitch = csubwidth;
      any = 1;
    }
    if ( set->Frames[ i ].APlane.Allocate )
    {
      pushmalloc( pmbuf, &set->Frames[ i ].APlane.Buffer, 
                  ( ysubwidth * set->YABufferHeight ) );
      set->Frames[ i ].APlane.BufferPitch = ysubwidth;
      any = 1;
    }
    if ( set->Frames[ i ].HPlane.Allocate )
    {
      pushmalloc( pmbuf, &set->Frames[ i ].HPlane.Buffer, 
                  ( ysubwidth * set->YABufferHeight ) );
      set->Frames[ i ].HPlane.BufferPitch = ysubwidth;
      any = 1;
    }
  }
  
  for ( ; i < BINKMAXFRAMEBUFFERS ; i++ )
  {
    set->Frames[ i ].YPlane.Buffer = 0;
    set->Frames[ i ].cRPlane.Buffer = 0;
    set->Frames[ i ].cBPlane.Buffer = 0;
    set->Frames[ i ].YPlane.Buffer = 0;
  }
    
  if ( any )
  {
    bp->allocatedframebuffers = (BINKFRAMEBUFFERS*)bpopmalloc( pmbuf, bp, sizeof( *set ) );
    if ( bp->allocatedframebuffers == 0 )
      return 0;
  }
  return 1;
}

// Y'CrCb matrices for Bink. Supported color spaces have
// the form:
//
// [r]    [y_scale  cr_r    0 ]   [Y']   [r_bias]
// [g]  = [y_scale  cr_g  cb_g] * [Cr] + [g_bias]
// [b]    [y_scale   0    cb_b]   [Cb]   [b_bias]
//
// The colorspace constants store four 4-vectors used by
// the pixel shaders, in this order:
//
//   float4 cr_column; // Cr column of above matrix (padded with 0)
//   float4 cb_column; // Cb column of above matrix (padded with 0)
//   float4 bias;      // (r_bias, g_bias, b_bias, 0)
//   float4 y_scale;   // (y_scale, y_scale, y_scale, 0)
static F32 const bink1_colorspace_consts[16] = 
{
  1.595794678f, -0.813476563f,             0, 0.0f,
             0, -0.391448975f,  2.017822266f, 0.0f,
  -0.87406939f,  0.531782323f, -1.085910693f, 1.0f,
  1.164123535f,  1.164123535f,  1.164123535f, 0.0f 
};

static F32 const bink2_colorspace_consts[16] = 
{
        1.402f,    -0.71414f,            0, 0.0f,
             0,    -0.34414f,       1.772f, 0.0f,
  -0.70374902f,  0.53121506f, -0.88947451f, 1.0f,
          1.0f,         1.0f,         1.0f, 0.0f 
};

#if defined(__RADWINRT__) && defined(__RAD64__)
#define CheckPlatform IsUWPPlatformOK
RADEXPFUNC S32 RADEXPLINK RADLINK IsUWPPlatformOK();
#endif

RADDEFFUNC S32 RADLINK BinkFileOpen(BINKIO * bio,const char * name, U32 flags);

//opens a Bink File
static HBINK bink_open( char const * name, BINK_OPEN_OPTIONS const * boo, U32 flags )
{
  BINK b;
  HBINK bp;
  U32 basizes[ MAXTRACKS ];
  U8 * baaudiomem = 0;
  U32 tottracks = 1;
  S32 deftrk = 0;
  S32 * tracknums = &deftrk;
  U32 * alignbink;
  U32 * fo32;
  
  BINKHEADER h;
  U32 sim;
  U32 j;
  char pmbuf[ PushMallocBytesForXPtrs( 32 ) ];

  BINKIOOPEN fileopen=&BinkFileOpen;

  CPU_check( 0, 0 );
#ifdef INC_BINK2
  ExpandBink2GlobalInit();
#endif

  pushmallocinit( pmbuf, 32 );

  if ( flags & BINKSNDTRACK )
  {
    tottracks = boo->TotTracks;
    tracknums = boo->TrackNums;
  }

  BinkLog("In BinkOpen");

#if defined(CheckPlatform)
  if ( !CheckPlatform() )
  {
    BinkSetError("Incorrect platform.");
    return 0;
  }
#endif  

  rrmemsetzero(&b,sizeof(b));

  b.timeopen=RADTimerRead();

  BinkClearError();

  if (flags&BINKFROMMEMORY)
    rrmemcpy(&h,name,sizeof(h));
  else
  {

    if ((flags&BINKIOPROCESSOR) && (boo->UserOpen))
      fileopen=boo->UserOpen;
    
    if ( fileopen == 0 )
    {
      if (BinkErrorIsCleared())
        BinkSetError("No file IO provider installed.");
      return 0;
    }

    rrmemsetzero( &b.bio, sizeof( BINKIO ) );
    b.bio.timer_callback = bink_timer;
    b.bio.lockedadd_callback = bink_lockedadd;
    b.bio.memcpy_callback = bink_memcpy;
    #ifdef __RADBIGENDIAN__
      b.bio.flipendian_callback = bink_flipendian;
    #endif

    setup_bink_mutex();

    if (flags&BINKFILEOFFSET)
      b.FileOffset=boo->FileOffset;

    lock_io_global();

    if (fileopen(&b.bio,name,flags)==0)
    {
      unlock_io_global();
      
      if (BinkErrorIsCleared())
        BinkSetError("Error opening file.");
      return(0);
    }
    
    unlock_io_global();

    b.bio.ReadHeader(&b.bio,0,&h,sizeof(h));
  }

  if ( (S32)h.Height < 0 ) h.Height += 0x7f000000; // future tweakable

#ifdef INC_BINK2
  if ( ( !is_bink( h.Marker ) )
#else
  if ( ( !is_binkv1(h.Marker) )
#endif
   || (b.bio.ReadError) )
  {
    BinkSetError("Not a Bink file.");
   closebio:
    if (!(flags&BINKFROMMEMORY))
    {
      lock_io_global();
      
      b.bio.Close(&b.bio);

      unlock_io_global();
    }
    return(0);
  }

  h.NumTracks &= ~0x40000000; // future tweakable

  if (h.Frames==0)
  {
    BinkSetError("The file doesn't contain any compressed frames yet.");
    goto closebio;
  }

  if ( (S32)h.Width < 0 ) h.Width = -(S32)h.Width; // future tweakable

  if ( ( h.InternalFrames < h.Frames ) ||
       ( h.InternalFrames > 1000000 ) ||  // probably no one has more than a million frames
       //( h.LargestFrameSize > h.Size ) ||
       ( h.NumTracks > 256 ) ||
       //( h.Size < ( h.InternalFrames * ( 4 * ( h.NumTracks + 1 ) ) ) ) ||
       ( h.Width > 32767 ) ||
       ( h.Height > 32767 ) ||
       ( h.FrameRate == 0 ) ||
       ( h.FrameRateDiv == 0 ) )
  {
    BinkSetError("The file has a corrupt header.");
    goto closebio;
  }    
    
  b.Width=h.Width;
  b.Height=h.Height;
  b.marker = h.Marker;

  b.BinkType=h.Flags;

  if (flags&BINKGPU)
  {
    flags|=BINKNOFRAMEBUFFERS;
    flags&=~BINKGPU; // this flag is set when you register gpu textures
  }

  b.OpenFlags=flags|(h.Flags&(BINKGRAYSCALE|BINKAUDIO2|BINKYCRCBNEW|BINK_SLICES_MASK));

  if ((h.Flags&BINKALPHA)==0) // if file has no alpha, then don't set alpha flag
    b.OpenFlags&=~BINKALPHA;

  if ((h.Flags&BINKHDR)==0) // if file has no hdr, then don't set hdr flag
    b.OpenFlags&=~BINKHDR;
  else
    b.OpenFlags|=BINKHDR;

  if (is_binkv23d_or_later( h.Marker ) || is_binkv11_or_later_but_not_v2( h.Marker ) )
  {
    b.opt_bytes = 4 + ( h.Flags >> 24 ) * 4;
    if ( b.OpenFlags & BINKHDR )
      b.opt_bytes += 6 * sizeof(S16); // max nits, then 4 chroma scales
    else if (h.Flags&BINKCUSTOMCOLORSPACE)
      b.opt_bytes += 12 * sizeof(S16);
  }

  if (is_binkv1_old_format(h.Marker))
      b.OpenFlags|=BINKOLDFRAMEFORMAT;

  b.Frames=h.Frames;
  b.InternalFrames=h.InternalFrames;

  if ((flags&BINKFRAMERATE) && (boo->ForceRate!=-1) )
  {
    b.FrameRate=boo->ForceRate;
    b.FrameRateDiv=boo->ForceRateDiv;
  }
  else
  {
    b.FrameRate=h.FrameRate;
    b.FrameRateDiv=h.FrameRateDiv;
  }

  b.fileframerate=h.FrameRate;
  b.fileframeratediv=h.FrameRateDiv;

  b.Size=h.Size;
  b.NumTracks=h.NumTracks;
  b.LargestFrameSize=h.LargestFrameSize;
  b.runtimeframes=(b.fileframerate+(b.fileframeratediv/2))/b.fileframeratediv;
  if (b.runtimeframes==0)
    b.runtimeframes=1;
//  b.runtimemoveamt=(b.runtimeframes-1)*4;

  // flag for just loading the data
  if (flags&BINKFILEDATALOADONLY)
  {
    alignbink = (U32*)bpopmalloc( pmbuf, &b, sizeof(BINK) + h.Size + 8 + 256 + 1024 + 1024 );
    if ( alignbink == 0 ) 
      goto nomem;
    
    bp=(HBINK)((((UINTa)alignbink)+256+1024)&(~(UINTa)255));
    ((U32*)bp)[-1]=(U32)(((UINTa)bp)-((UINTa)alignbink)); // store the amount to back up
    rrmemcpy(bp,&b,sizeof(b));
    bp->bio.bink=bp; // set the bink pointer in the IO structure
    
    bp->preloadptr=(void*)((((UINTa)(bp+1))+15)&~15);
    bp->OpenFlags|=BINKNOTHREADEDIO|BINKPRELOADALL;
    bp->bio.SetInfo(&bp->bio,0,0,bp->Size+8,0);
    bp->bio.ReadHeader(&bp->bio,0,bp->preloadptr,bp->Size+8);
    lock_io_global();
    bp->bio.Close(&bp->bio);
    unlock_io_global();
    bp->bio.ForegroundTime=0;
    return bp;
  }


  pushmalloc(pmbuf, &b.rtframetimes,4*(b.runtimeframes));
  pushmalloc(pmbuf, &b.rtadecomptimes,4*(b.runtimeframes));
  pushmalloc(pmbuf, &b.rtvdecomptimes,4*(b.runtimeframes));
  pushmalloc(pmbuf, &b.rtblittimes,4*(b.runtimeframes));
  pushmalloc(pmbuf, &b.rtreadtimes,4*(b.runtimeframes));
  pushmalloc(pmbuf, &b.rtidlereadtimes,4*(b.runtimeframes));
  pushmalloc(pmbuf, &b.rtthreadreadtimes,4*(b.runtimeframes));

#ifdef INC_BINK2
  if ( is_binkv2_or_later(h.Marker) )
  {
    pushmalloc(pmbuf, &b.seam, 4+((b.Width+31)/32)*4*(SliceMaskToCount(b.OpenFlags)-1) );
  }
#endif

  { RR_COMPILER_ASSERT( (sizeof(BINKSND)&15)==0 ); }
  pushmalloc(pmbuf, &b.bsnd,(tottracks?tottracks:1)*sizeof(BINKSND));
  pushmalloc(pmbuf, &b.trackindexes,tottracks*4);
  
  if ( is_binkv1(h.Marker) )
  {
#ifdef NO_BINK10_SUPPORT    
    BinkSetError("No Bink 1 support.");
    goto closebio;
#else
    ExpandBundleSizes( &b.bunp, ( ( b.Width + 15 ) & ~15 ) );
#endif  
  }

  if ((flags&BINKFROMMEMORY)==0)
  {
    pushmalloc(pmbuf, &b.opt_ptr, b.opt_bytes );
    pushmalloc(pmbuf, &b.tracksizes, OSFRAMEREADALIGN + OSFRAMEREADALIGN + 4*b.NumTracks + 4*b.NumTracks + 4*b.NumTracks + 8*(b.InternalFrames+1) );
  }
  else
  {
    pushmalloc(pmbuf, &b.frameoffsets, 8*(b.InternalFrames+1)+8 );
  }
  
  alignbink = (U32*)bpopmalloc( pmbuf, &b, sizeof(BINK) + 256 );
  if ( alignbink == 0 ) 
  {
   nomem: 
    BinkSetError("Out of memory.");
   memerr:
    goto closebio;
  }

  bp=(HBINK)((((UINTa)alignbink)+256)&(~(UINTa)255));
  ((U32*)bp)[-1]=(U32)(((UINTa)bp)-((UINTa)alignbink)); // store the amount to back up

  rrmemcpy(bp,&b,sizeof(b));

  bp->bio.bink=bp; // set the bink pointer in the IO structure

  bp->rtadecomptimes[0]=0;
  bp->rtvdecomptimes[0]=0;
  bp->rtblittimes[0]=0;
  bp->rtreadtimes[0]=0;
  bp->rtidlereadtimes[0]=0;
  bp->rtthreadreadtimes[0]=0;

  if (flags&BINKFROMMEMORY)
  {
    bp->opt_ptr = (char*)name+sizeof(h);
    bp->tracksizes=(U32*)(name+sizeof(h)+bp->opt_bytes);
    bp->tracktypes=(U32*)(name+sizeof(h)+4*bp->NumTracks+bp->opt_bytes);
    bp->trackIDs=(S32*)(name+sizeof(h)+8*bp->NumTracks+bp->opt_bytes);
    fo32=(U32*)(name+sizeof(h)+12*bp->NumTracks+bp->opt_bytes);
  }
  else
  {
    if ( bp->opt_bytes )
      bp->bio.ReadHeader(&bp->bio,-1,bp->opt_ptr,bp->opt_bytes);

    bp->tracksizes = (U32*) ( ( ( ( (UINTa)bp->tracksizes ) + OSFRAMEREADALIGN + OSFRAMEREADALIGN ) & ~(OSFRAMEREADALIGN-1) ) - OSFRAMEREADALIGN + sizeof(h) );
    bp->bio.ReadHeader(&bp->bio,-1,bp->tracksizes,4*b.NumTracks + 4*b.NumTracks + 4*b.NumTracks + 4*(b.InternalFrames+1));
    bp->tracktypes   = &(((U32*)bp->tracksizes)[b.NumTracks]);
    bp->trackIDs     = &(((S32*)bp->tracktypes)[b.NumTracks]);
    fo32 = &(((U32*)bp->trackIDs)  [b.NumTracks]);
    bp->frameoffsets = (U64*)((((UINTa)fo32)+7)&~7);
    if ( bp->bio.ReadError ) 
    {
      BinkSetError("Error reading Bink header.");
      goto memerr1;
     }
  }
  
  if ( h.Flags & BINKHDR )
  {
    S32 i;
    S16 * cs;

    cs = (S16*) bp->opt_ptr;
    bp->opt_ptr = ((U8*)bp->opt_ptr) + 6 * sizeof(S16);
    bp->opt_bytes -= 6 * sizeof(S16);

    for( i = 0 ; i < 5 ; i++ )
      bp->ColorSpace[i] = ((F32)cs[i])*(1.0f/32767.0f);
    bp->ColorSpace[5] = (F32)cs[5]; // nits (non-scaled)
  }
  else if ( h.Flags & BINKCUSTOMCOLORSPACE )
  {
    // pull the colorspace from the opt_bytes
    S32 t,i;
    S16 * cs;
    cs = (S16*) bp->opt_ptr;
    bp->opt_ptr = ((U8*)bp->opt_ptr) + 12 * sizeof(S16);
    bp->opt_bytes -= 12 * sizeof(S16);
    
    t=0;
    for(i=0;i<16;i++)    
      bp->ColorSpace[i] = ((i&3)==3) ? 0.0f : (((F32)cs[t++])*(1.0f/4096.0f)); 
  }
  else
  {
    // set videowide colorspace
    rrmemcpy( bp->ColorSpace, ( bp->OpenFlags & BINKYCRCBNEW ) ? bink2_colorspace_consts : bink1_colorspace_consts, sizeof( bp->ColorSpace ) );
    if (is_binkv1_flipped_RB_chroma(h.Marker))
    {
      F32 t;
      // flip the chroma rows
      t=bp->ColorSpace[0]; bp->ColorSpace[0]=bp->ColorSpace[4]; bp->ColorSpace[4]=t;
      t=bp->ColorSpace[1]; bp->ColorSpace[1]=bp->ColorSpace[5]; bp->ColorSpace[5]=t;
      t=bp->ColorSpace[2]; bp->ColorSpace[2]=bp->ColorSpace[6]; bp->ColorSpace[6]=t;
    }
  }


#ifdef INC_BINK2
  setup_slices( bp->marker, bp->OpenFlags, bp->Width, bp->Height, &bp->slices );
#else
  bp->slices.slice_count=2;
#endif

  bp->Highest1SecRate=high1secrate(bp->Frames,bp->frameoffsets,fo32,bp->runtimeframes,&bp->Highest1SecFrame,&bp->allkeys,&bp->Size);
  if ( bp->Highest1SecRate == (U64) -1 )
  {
    BinkSetError("This file has bad frame size data.");
    goto memerr1;
  }
  
  if ( ( flags & BINKNOFRAMEBUFFERS ) == 0 )
  {
    BINKFRAMEBUFFERS set;
    
    BinkGetFrameBuffersInfo( bp, &set );
    
    if ( !BinkAllocateFrameBuffers( bp, &set, 0 ) )
    {
     memerr1mess:
      BinkSetError("Out of memory.");
     memerr1:
      popfree( ((U8*)bp)-((U32*)bp)[-1] );
      goto memerr;
    }

    bp->FrameBuffers=bp->allocatedframebuffers;
    rrmemcpy( bp->FrameBuffers, &set, sizeof( set ) );
  }  

  if ((flags&BINKIOSIZE) && (boo->IOBufferSize!=-1) && (boo->IOBufferSize!=0))
  {
    bp->iosize=boo->IOBufferSize;
  } else
    bp->iosize=bp->Highest1SecRate;

  if ((flags&BINKSIMULATE) && (boo->Simulate!=-1) && (boo->Simulate!=0))
  {
    sim=boo->Simulate;
  } else
    sim=0;
  bp->simulate = sim;

  if (flags&BINKFROMMEMORY)
  {
    bp->preloadptr=(void*)(name+(bp->frameoffsets[0]&~3));
    bp->OpenFlags |= BINKNOTHREADEDIO;
  }
  else
  {
    bp->iosize=bp->bio.GetBufferSize(&bp->bio,bp->iosize);
    
    if (((U64)bp->iosize+(U64)(256*1024))>=(U64)bp->Size)
    {
      bp->iosize = bp->Size + 16; // let the whole thing read into memory
    }
    
    if (flags&BINKPRELOADALL)
    {
      U64 all=(bp->Size+8)-(bp->frameoffsets[0]&~3); // size doesn't include first two dwords

      bp->preloadptr=bpopmalloc(pmbuf, bp,all + 1024); // extra 1024 to detect overruns
      if (bp->preloadptr==0)
      {
        if ( bp->FrameBuffers )
          popfree( bp->FrameBuffers );
        goto memerr1mess;
      }
      bp->OpenFlags|=BINKNOTHREADEDIO;

      bp->bio.SetInfo(&bp->bio,0,0,bp->Size+8,sim);
      bp->bio.ReadHeader(&bp->bio,(bp->frameoffsets[0]&~3),bp->preloadptr,all);
      
      lock_io_global();
      
      bp->bio.Close(&bp->bio);
      
      unlock_io_global();
      bp->bio.ForegroundTime=0;
    }
    else
    {
      pushmalloc(pmbuf, &bp->alloccompframe,bp->LargestFrameSize + OSFRAMEREADALIGN + OSFRAMEREADALIGN + 1024); // extra 1024 to detect overruns
      bp->ioptr=(U8*)bpopmalloc(pmbuf, bp,bp->iosize);

      if (bp->ioptr==0)
      {
        if ( bp->FrameBuffers )
          popfree( bp->FrameBuffers );
        goto memerr1mess;
      }

      bp->bio.SetInfo(&bp->bio,bp->ioptr,bp->iosize,bp->Size+8,sim);
    }
  }

  bp->FrameNum=~0U;

  if (bp->FrameRate)
    bp->twoframestime=mult64anddiv(2000,bp->FrameRateDiv,bp->FrameRate);
  else
    bp->twoframestime=2000;

  bp->videoon=1;

  bp->timeopen=RADTimerRead()-bp->timeopen;

  // set default videoscale
  bp->bsnd[0].VideoScale=1<<16;
  {
    U32 totalbamem = 0;
    // get the track numbers to attempt to play
    bp->playingtracks=0;
    if ((bp->NumTracks) && (tottracks))
    {

      #ifdef DUMPVOLUMES
        char buf[128];
        sprintf(buf,"told to play: %d number of tracks in file: %d\n",tottracks,bp->NumTracks);
        OutputDebugString(buf);
        if (tottracks)
        {
          U32 i;
          OutputDebugString("Tracks told to play: ");
          for(i=0;i<tottracks;i++)
          {
            sprintf(buf,"%d ",tracknums[i]);
            OutputDebugString(buf);
          }
          OutputDebugString("\n");
        }
      #endif

      for( j = 0; j < tottracks ; j++ )
      {
        S32 i;

        // skip duplicates
        for( i = 0 ; i < (S32)j ; i++ )
        {
          if ( tracknums[i]==tracknums[j] )
            goto skip_dupe;
        }

        // find the track number
        for(i=0;i<(S32)bp->NumTracks;i++)
        {
          if (bp->trackIDs[i]==tracknums[j])
          {
            U32 n = bp->playingtracks++;
            bp->trackindexes[ n ]=i;
            basizes[ n ] = BinkAudioDecompressMemory( GETTRACKFREQ(bp->tracktypes[i]), GETTRACKCHANS(bp->tracktypes[i]), GETTRACKNEWFORMAT(bp->OpenFlags,bp->tracktypes[i]) );
            totalbamem += ((((5*bp->tracksizes[i])>>2)+255)&~255) + basizes[ n ];
            break;
          }
        }

        skip_dupe:;
      }
    }
    if ( totalbamem )
    {
      baaudiomem = (U8*) bpopmalloc( pmbuf, bp, totalbamem );
      bp->binkaudiomem = baaudiomem;
      if ( baaudiomem == 0 )
        bp->playingtracks = 0;
    }
  }

  gTotTracks=1;
  gTrackNums[0]=0;

  //now try to open the file
  j=0;
  while( j < bp->playingtracks )
  {
    bp->bsnd[j].sndbuf=0;

    // does the track contain sound?
    if (GETTRACKFORMAT(bp->tracktypes[bp->trackindexes[j]]))
    {
      if (sndopen)
      {
        U32 freq;
        U32 ofreq=GETTRACKFREQ(bp->tracktypes[bp->trackindexes[j]]);

        if ( ofreq <= 256000 )
        {
          U32 sndsize=((((5*bp->tracksizes[bp->trackindexes[j]])>>2)+255)&~255);
          U32 sndchans=GETTRACKCHANS(bp->tracktypes[bp->trackindexes[j]]);

          freq = ofreq;

          if ((b.FrameRate!=0) && (b.FrameRateDiv!=0))
          {
            freq=(S32)(((F64)(S32)ofreq*(F64)(S32)b.FrameRate*(F64)(S32)b.fileframeratediv) /
                       ((F64)(S32)b.FrameRateDiv*(F64)(S32)b.fileframerate) );
          }

          if ( freq > 256000 )
            freq = 256000;

          if ( sndsize <= ( ( 2 * ofreq * 16 * sndchans ) / 8 ) ) // never more than 2 second of audio buffers
          {
            S32 sopened;

            lock_snd_global();

            sopened = sndopen(&bp->bsnd[j],freq,
                               sndchans,
                               bp->OpenFlags,bp);

            unlock_snd_global();
                               
            if ( sopened )
            {
              bp->bsnd[j].orig_freq = ofreq;
              
              // Default BestSizeMask to 0xffffffff if it wasn't set in sndopen.
              if (bp->bsnd[j].BestSizeMask == 0)
                bp->bsnd[j].BestSizeMask = (U32)~0;

              bp->bsnd[j].sndbufsize=sndsize;

              bp->bsnd[j].sndbuf = baaudiomem;
              baaudiomem += bp->bsnd[j].sndbufsize;

              {
                S32 latency;

                latency = 750 - (S32) bp->bsnd[j].Latency;
                if (latency < 0 )
                  latency = 0;
                  
                bp->bsnd[j].sndend=bp->bsnd[j].sndbuf+bp->bsnd[j].sndbufsize;
                bp->bsnd[j].sndwritepos=bp->bsnd[j].sndbuf;
                bp->bsnd[j].sndreadpos=bp->bsnd[j].sndbuf;

                bp->bsnd[j].sndprime=mult64anddiv(freq*sndchans*2,
                                                  latency,
                                                  1000)&~63;

                // the prime amount should always be at least 16 bytes smaller than the sndbuf size
                if ( ( (S32) bp->bsnd[j].sndbufsize - bp->bsnd[j].sndprime ) < 16 )
                  bp->bsnd[j].sndprime=bp->bsnd[j].sndbufsize - 16;

                bp->bsnd[j].sndcomp=(UINTa)BinkAudioDecompressOpen(baaudiomem,
                                                                   GETTRACKFREQ(bp->tracktypes[bp->trackindexes[j]]),
                                                                   sndchans,
                                                                   GETTRACKNEWFORMAT(bp->OpenFlags,bp->tracktypes[bp->trackindexes[j]]));
                                                                   
                lock_snd_global();

                if ( bp->bsnd[j].sndcomp == 0 )
                {
                  bp->bsnd[j].Close(&bp->bsnd[j]);

                  bp->bsnd[j].sndbuf = 0;
                }
                
                unlock_snd_global();

                baaudiomem += basizes[ j ];
                bp->bsnd[j].sndendframe=bp->Frames-((bp->fileframerate*3)/(bp->fileframeratediv*4));
              }
            }
          }
        }
      }
    }
    

    // set the default video scale
    if (bp->bsnd[j].VideoScale==0)
      bp->bsnd[j].VideoScale=1<<16;

    // if the sound did not start properly
    if (bp->bsnd[j].sndbuf==0)
    {
      --bp->playingtracks;
      rrmemcpyds( &bp->trackindexes[j], &bp->trackindexes[j+1], (bp->playingtracks-j)*4 );
    }
    else
    {
      ++j;
    }
  }

  // start the sound callback
  if ( bp->playingtracks )
  {
    bp->soundon=1;

    lock_snd_global();

    numopensounds += bp->playingtracks;

    unlock_snd_global();

    init_sound_service_for_bink( bp );
  }

  bp->bio.try_suspend_callback = bink_try_suspend_io;

#if !defined(DLL_FOR_GENERIC_OS)
  bp->bio.simulate_callback = bink_simulate_io;
#endif

  // start the io callback
  init_io_service_for_bink( bp );

  // if we are doing background loading, don't preload the buffer
  if ( ( bp->OpenFlags & BINKNOTHREADEDIO ) == 0 )
  {
    flags |= BINKNOFILLIOBUF;
  }

  if (bp->preloadptr==0) 
  {
    if ((flags&BINKNOFILLIOBUF)==0)
    {
      if ( bp->bio.Idle( &bp->bio ) )
      {
        while (bp->bio.Idle(&bp->bio)) {};
      }
    }
    wake_io_thread();
  }

  bp->lastfileread = 0xf0000000;
  GotoFrame(bp,1);

  // blip the sound thread
  wake_snd_thread( bp );

  bp->bink_unique = rrAtomicAddExchange32( &unique_count, 1 );

  BinkLog("Out BinkOpen");

  return(bp);
}

RADEXPFUNC HBINK RADEXPLINK BinkOpen(char const * name,U32 flags)
{
  BINK_OPEN_OPTIONS boo;

  boo.FileOffset=gFileOffset;
  boo.ForceRate=gForceRate;
  boo.ForceRateDiv=gForceRateDiv;
  boo.IOBufferSize=gIOBufferSize;
  boo.Simulate=gSimulate;
  boo.TotTracks=gTotTracks;
  boo.TrackNums=gTrackNums;
  boo.UserOpen=gUserOpen;

  gFileOffset=0;
  gForceRate=-1;
  gIOBufferSize=-1;
  gSimulate=-1;
  gUserOpen=0;
  
  return bink_open( name, &boo, flags );
}

RADEXPFUNC HBINK RADEXPLINK BinkOpenWithOptions(char const * name, BINK_OPEN_OPTIONS const * boo,U32 flags)
{
  return bink_open( name, boo, flags );
}  

static S32 shouldskip( HBINK bink, S32 onlyreportskip )
{
  S32 timer, waittime, curtime;

  inittimer(bink);

  if ( bink->FrameRate == 0 ) 
    goto noskip;
  
  timer = RADTimerRead();
  waittime=mult64andshift(mult64anddiv((bink->playedframes-bink->startsyncframe)*1000,bink->FrameRateDiv,bink->FrameRate),bink->bsnd[0].VideoScale,16);
  curtime = timer - bink->startsynctime;
  curtime -= waittime;

  if ( curtime >= 0 )
  {
    if ( curtime > (S32) bink->twoframestime )
    {
      if (bink->playingtracks==0)
      {
        //if we're way too late and we don't have sound, reset the timer
        bink->startsynctime = timer;
        bink->startsyncframe = ( bink->playedframes - 1 );
        bink->paused_sync_diff = 0;
        goto noskip;
      }

      if ( curtime > 725 ) //have we exhausted our sound buffer
        bink->very_delayed = 1;

      // don't skip more than 4 frames in a row
      if ( bink->skipped_in_a_row >= 4 )
      {
        bink->skipped_in_a_row = 0;
        goto noskip;
      }
      ++bink->skipped_in_a_row;

      if ( !onlyreportskip )
      {
        bink->skipped_status_this_frame = 2;  //0=not checked this frame, 1=no skip, 2=skip
        bink->skippedlastblit=1;
        ++bink->skippedblits;
      }

      return( 1 );
    }
  }

 noskip:
  bink->skipped_status_this_frame = 1;  //0=not checked this frame, 1=no skip, 2=skip
  return(0);
}


#ifndef NO_BINK_BLITTERS

RADEXPFUNC S32 RADEXPLINK BinkCopyToBuffer(HBINK bink,void* dest,S32 destpitch,U32 destheight,U32 destx,U32 desty, U32 flags)
{
  return( BinkCopyToBufferRect( bink, dest, destpitch, destheight, destx, desty, 0, 0, bink->Width, bink->Height, flags) );
}

#endif

/*
static void unsharp( U8* o, BINKFRAMEBUFFERS * b )
{
  U8 * yt, * yb, * ym;
  U32 h = b->YABufferHeight;
  U32 x,y;
  U32 w = b->YABufferWidth;
  U32 p=b->Frames[0].YPlane.BufferPitch;

  yt = (U8*) b->Frames[b->FrameNum].YPlane.Buffer;
  ym = yt + p;
  yb = yt + p*2;
  o += p;

  for(y=2;y<h;y++)
  {
    ++yt;
    ++ym;
    ++yb;
    ++o;
    for(x=2;x<w;x++)
    {
      S32 a = (ym[0]*8-yt[0]-yb[0]-ym[-1]-ym[1])/4;
      if (a>255) a = 255; else if ( a<0 ) a= 0;
      o[0]=(U8)a;
      ++yt;
      ++ym;
      ++yb;
      ++o;
    }
    yt -= (w-1);
    ym -= (w-1);
    yb -= (w-1);
    o -= (w-1);
    yt += p;
    ym += p;
    yb += p;
    o += p;
  }
}
*/

#ifndef NO_BINK_BLITTERS

RADEXPFUNC S32 RADEXPLINK BinkCopyToBufferRect(HBINK bink,void* dest,S32 destpitch,U32 destheight,U32 destx,U32 desty,U32 srcx, U32 srcy, U32 srcw, U32 srch, U32 flags)
{
  RAD_ALIGN(BLITPARAMS, blitpara, 16);
  U32 ret=0;

  if (bink==0)
    return(0);

  if (dest==0)
    return(0);
    
  if (bink->FrameBuffers==0)
    return(0);

  if ( bink->OpenFlags & BINKHDR )
    return 0;

  BinkLog("In BinkCopy");

  if ( srcx >= bink->Width )
  {
    srcx = bink->Width;
  }

  if ( srcy >= bink->Height )
  {
    srcy = bink->Height;
  }

  if ( ( srcx + srcw ) > bink->Width )
  {
    srcw = bink->Width - srcx;
  }

  if ( ( srcy + srch ) > bink->Height )
  {
    srch = bink->Height - srcy;
  }

  if ( ( srcw == 0 ) || ( srch == 0 ) || ( bink->skipped_status_this_frame == 2 ) )
  {
    return( ( bink->skipped_status_this_frame == 2 ) ? 1 : 0 );  //0=not checked this frame, 1=no skip, 2=skip
  }

  if ( bink->startblittime == (U32)-1 )
    bink->startblittime=RADTimerRead();

  bink->bio.Working=1;

  flags |= bink->OpenFlags & ( BINKGRAYSCALE | BINKYCRCBNEW | BINKYAINVERT );
  if ( bink->FrameBuffers->Frames[ 0 ].APlane.Buffer == 0 )
    flags &= ~BINKYAINVERT;

  {
    U32 blitflags = ( bink->FrameBuffers->Frames[ 0 ].APlane.Buffer != 0 ) ? BLITFLAG_HASALPHA : 0;
    if ( flags & BINKYAINVERT ) blitflags |= BLITFLAG_SWAP_YA;
    if ( flags & BINKGRAYSCALE ) blitflags |= BLITFLAG_GRAYSCALE;

    BinkBlit_init( &blitpara, bink->ColorSpace, flags & BINKSURFACEMASK, blitflags );
  }

  if (destpitch<0)
  {
    dest=((U8*)dest)+((-destpitch)*(destheight-desty-1));
    desty=0;
  }

  // see if we should skip
  if ((bink->playingtracks) && (bink->FrameRate) && (bink->Paused==0) && (bink->soundon) && (bink->skipped_status_this_frame==0) ) // 0 means we haven't checked, 1 means we don't skip, 2 won't happen because we exitted above
  {
    if ((flags&BINKNOSKIP) || (bink->OpenFlags&BINKNOSKIP) || (bink->FrameNum>=bink->Frames) )
    {
      // if no skip is on, or we are at the last frame, just report the current skip status (and don't skip)
      if ( shouldskip( bink, 1 ) )
        ret = 1;
    }
    else
    {
      // otherwise exit
      if ( shouldskip( bink, 0 ) )
      {
        bink->bio.Working=0;
        return(1);
      }
    }
    bink->skippedlastblit=0;
  }

//  only do io in binknextframe and binkwait from now on

  tmEnter( tm_mask, TMZF_NONE, "YUV to RGB " );

  bink->lastblitflags=flags;

  BinkBlit_blit( &blitpara, dest,destx,desty, destpitch,
                 &bink->FrameBuffers->Frames[bink->FrameBuffers->FrameNum],
                 bink->Width, bink->Height,
                 srcx,srcy, srcw,srch );

  tmLeave( tm_mask );

  BinkLog("Out BinkCopy");

  bink->bio.Working=0;

  return(ret);
}

#endif

/*
#include <stdio.h>

#ifdef __RADNT__
#include <windows.h>
#endif
*/


/*#ifdef __RADNT__
#include <windows.h> 
#endif
static void fill_random( void * add, UINTa amt )
{
  UINTa i;
static U8 r=1;

#ifdef __RADNT__
 if ((GetAsyncKeyState(VK_SHIFT)&0x8000000) || (GetAsyncKeyState(VK_CONTROL)&0x8000000))
#endif

  for ( i = 0 ; i < amt ; i++ )
  {
    ((U8*)add)[i]^= r;

  }
  ++r;
}
*/

static void timeblitframe(HBINK bink,U32 timer)
{
  if ((bink->startblittime) && (bink->startblittime!=(U32)-1))
  {
    bink->timeblit+=(timer-bink->startblittime);
    bink->startblittime=0;
  }
}


HBINK start_do_frame( HBINK bink, U32 timer );
HBINK start_do_frame( HBINK bink, U32 timer )
{
  if (bink==0)
    return(0);

  if (bink->lastdecompframe==bink->FrameNum)
    return(0);

  if (bink->bio.ReadError)
    bink->ReadError=1;

  if (bink->ReadError)
    return(0);

  timeblitframe( bink, timer );

  if ( bink->doresync )
  {
    bink->doresync = 0;
    if ( bink->soundon )
    {
      BinkSetSoundOnOff( bink, 0 );
      BinkSetSoundOnOff( bink, 1 );
    }
    bink->startsynctime = 0;
    bink->paused_sync_diff = 0;
  }
    
  if ( bink->Paused )
  {
    // if we advance while paused, reset our sync
    bink->startsynctime = 0;
    bink->paused_sync_diff = 0;
  }    

  bink->skipped_status_this_frame = 0;  //0=not checked this frame, 1=no skip, 2=skip

  clear_seam( &bink->slices, bink->seam );

  bink->bio.Working=1;

  BinkLog("In BinkDo");

  bink->changepercent = 0;

  return( bink );
}

S32 end_do_frame( HBINK bink, U32 timer );
S32 end_do_frame( HBINK bink, U32 timer )
{
  S32 oldindex;
  
#if defined(INC_BINK2)
    if ( bink->seam && !bink->gpu && bink->videoon ) 
    {
      ExpandBink2SplitFinish( bink->FrameBuffers, &bink->slices, ( bink->BinkType&BINKALPHA )?1:0, ( bink->BinkType&BINKHDR )?1:0, bink->compframekey, bink->seam );
    }
#endif

  if ( bink->FrameBuffers )
  {
   // advance the count
    S32 i = bink->FrameBuffers->FrameNum + 1;
    if ( i >= bink->FrameBuffers->TotalFrames )
      i = 0;
    bink->FrameBuffers->FrameNum = i;
  }

  oldindex = bink->rtindex;
  --bink->rtindex;
  if ( bink->rtindex < 0 )
    bink->rtindex = bink->runtimeframes - 1;

  if ( bink->lastfinisheddoframe )
  {
    U32 delta = timer - bink->lastfinisheddoframe;

    if (delta>bink->slowestframetime)
    {
      bink->slowest2frametime=bink->slowestframetime;
      bink->slowest2frame=bink->slowestframe;
      bink->slowestframetime=delta;
      bink->slowestframe=bink->lastdecompframe;
    }
    else
    {
      if (timer>bink->slowest2frametime)
      {
        bink->slowest2frametime=delta;
        bink->slowest2frame=bink->lastdecompframe;
      }
    }
  }

  bink->lastfinisheddoframe=timer;

  bink->rtframetimes[bink->rtindex]=timer;
  bink->rtvdecomptimes[bink->rtindex]=bink->timevdecomp;
  bink->rtadecomptimes[bink->rtindex]=bink->timeadecomp;
  bink->rtblittimes[bink->rtindex]=bink->timeblit;
  bink->rtreadtimes[bink->rtindex]=bink->bio.ForegroundTime;
  bink->rtidlereadtimes[bink->rtindex]=bink->bio.IdleTime;
  bink->rtthreadreadtimes[bink->rtindex]=bink->bio.ThreadTime;

  if (bink->firstframetime==0)
  {
    bink->rtframetimes[ oldindex ]=timer-mult64anddiv(1000,bink->fileframeratediv,bink->fileframerate);
    bink->firstframetime=timer;
    bink->bio.ThreadTime=0;
    bink->bio.IdleTime=0;
  }
  else
  {
    ServiceSound(bink);
  }

  ++bink->playedframes;

  inittimer(bink);

  bink->startblittime=(U32)-1;

  bink->lastdecompframe=bink->FrameNum;
  bink->LastFrameNum=bink->FrameNum;
  bink->FrameChangePercent = bink->changepercent;
  
  BinkLog("Out BinkDo");

  bink->bio.Working=0;
/*
{
  FILE * f;
  f=fopen("frames.bin","ab+");
  fwrite(((char*)bink->YPlane[bink->PlaneNum]),1,bink->YWidth*bink->YHeight,f);
  fwrite(((char*)bink->APlane[bink->PlaneNum]),1,bink->YWidth*bink->YHeight,f);
  fwrite(((char*)bink->YPlane[bink->PlaneNum])+bink->YWidth*bink->YHeight,1,bink->YWidth/2*bink->YHeight/2*2,f);
  fclose(f);
}  
*/

  return( 0 );  // we never currently skip
}

static void decompress_sound( HBINK bink, S32 j, U32 amt, U8* addr, void * compframe, U8* addrend )
{
  U8 const * saddrend;

//#ifdef __RADNT__
//#define DEBUGSOUND
//#endif

#ifdef DEBUGSOUND
  #ifdef __RADFINAL__
    #error "You have sound debug turned on!"
  #endif
  U8* taddr = popmalloc( amt );
  U8 const * saddr = taddr;
  U32 decomp=*((U32*)addr);
  rrmemcpy(taddr,addr,amt);
#else
  U8 const * saddr=addr;
  U32 decomp=*((U32*)addr);
#endif

  decomp &= ~15;

  saddr+=4;
  saddrend = saddr + amt - 4;

  if ( decomp )
  {
    HBINKAUDIODECOMP sndcomp;

    BINKSND * bsnd = &bink->bsnd[j];
    sndcomp = (HBINKAUDIODECOMP)bsnd->sndcomp;

    // figure out how much data we already have in the buffer

    while (decomp)
    {
      U32 left;
      S32 in_sndbuf;
      BINKAC_OUT_RINGBUF out;
      BINKAC_IN in;

      if ( ( saddr > addrend ) || ( saddr < (U8*) compframe ) || ( saddrend > addrend ) ) return;

      // how much is queued up?
      in_sndbuf = (S32) ( bsnd->sndwritepos - bsnd->sndreadpos );
      if ( in_sndbuf < 0 )
        in_sndbuf += bsnd->sndbufsize;

      // how much room left
      left = ( bsnd->sndbufsize - in_sndbuf ) & ~63;

      // always leave at least 16 bytes unused in the sndbuffer (when write==read, buffer is empty)
      if ( left > 16 ) 
        left -= 16;

      out.outptr = bsnd->sndwritepos;
      out.outstart = bsnd->sndbuf;
      out.outend = bsnd->sndend;
      out.outlen = left;
      out.eatfirst = 0;

      in.inptr = saddr;
      in.inend = saddrend;

      BinkAudioDecompress( sndcomp,&out,&in );
      // update compressed ptr
      saddr = in.inptr;

      // update output ptrs and amount left to decomp
      if ( decomp < out.decoded_bytes )
      {
        // this means we're on a final movie frame, and we had one
        //   full chunk of sound to decode, so we clamp the output
        //   to the movie frame's limit
        bsnd->sndwritepos += decomp;
        if ( bsnd->sndwritepos > bsnd->sndend ) bsnd->sndwritepos -= bsnd->sndbufsize;
        decomp = 0;
      }
      else
      {
        bsnd->sndwritepos = out.outptr;
        decomp -= out.decoded_bytes;
      }
    }
  }
#ifdef DEBUGSOUND
  popfree( taddr );
#endif
}


S32 LowBinkDoFrameAsync( HBINK bink, U32 work_flags )
{
  S32 i = 0;
  U8* addr;
  BINKFRAMEBUFFERS * lframebuffers; 
  U8 * compframe;
  S32 * trackindexes;
  S32 do_them;


  U8* addrend;
  U32 audio_time, video_time;

  if ( bink->closing )
    return 0;

  if ( work_flags & BINKDOFRAMESTART )
  {
    bink = start_do_frame( bink, RADTimerRead() );
    if ( bink == 0 )
      return( 0 );
  }

  lframebuffers = bink->FrameBuffers;
  trackindexes = bink->trackindexes;

  compframe = (U8*)bink->compframe;
  addr = compframe;
  addrend = addr + bink->compframesize;

  if ( bink->closing )
    return 0;

  tmEnter( tm_mask, TMZF_IDLE, "Waiting for priority IO to decompress" );
  check_for_pending_io( bink );
  tmLeave( tm_mask );
  
  if ( ( bink->ReadError ) || ( bink->closing ) )
    return 0;

  tmEnter( tm_mask, TMZF_NONE, "Decompress" );

  audio_time = RADTimerRead();

  tmEnter( tm_mask, TMZF_NONE, "Decompress audio" );
  
  do_them = 0;
  if ( ( ( work_flags & 15 ) + ( ( work_flags >> 4 ) & 15 ) ) >= bink->slices.slice_count )  // is this the last part of the frame
    do_them = 1;
        

  // do sound
  for(i=0;i<bink->NumTracks;i++)
  {
    S32 j;
    U32 amt;

    if ( ( addr > addrend ) || ( addr < (U8*) compframe ) ) break;

    amt=*((U32*)addr);
    addr+=4;
    
    // are we playing this track?
    if ( do_them )
    {
      for( j = 0 ; j < (S32)bink->playingtracks; j++ )
      {
        if (trackindexes[j]==i)
          goto found;
      }
    }
    j=-1;
   found:

    // decompress sound here
    if ((j != -1) && (amt))
    {
      decompress_sound( bink, j, amt, addr, compframe, addrend );
    }

    addr+=amt;
  }

  tmLeave( tm_mask );

  video_time = RADTimerRead();
  audio_time = video_time - audio_time;

  if ( (bink->videoon) && ( lframebuffers || bink->gpu ) && ( (work_flags & (15<<4)) ) )
  {
    // turn on and off the compression flags based on the work parameters

    i = bink->OpenFlags;
#if defined(INC_BINK2)
  if ( is_binkv2_or_later( bink->marker ) ) 
  {
    // bink 2 ignores these flags anyway
    i &= ~BINKGRAYSCALE;
    i &= ~BINKNOYPLANE;
  }
  else // carries over into next ifdef
#endif
  {
    // start by doing nothing 
    i |= BINKNOYPLANE;
    i |= BINKGRAYSCALE;
    i &= ~BINKALPHA;

    // does this overlap the middle section? 
    if ( ( ( work_flags & 15 ) + ( ( work_flags >> 4 ) & 15 ) ) >= 2 )  // is this the last part of the frame
    {
      i &= ~BINKGRAYSCALE; // turn on cr/cb
      i |= (bink->OpenFlags&BINKALPHA); // turn on alpha, if present
    }
    
    // does this start at zero?
    if ( ( work_flags & 15 ) == 0 )
    {
      i &= ~BINKNOYPLANE;  // turn on y plane
    }

    // if nothing to do, just return
    if ( ( i & (BINKNOYPLANE|BINKGRAYSCALE) ) == (BINKNOYPLANE|BINKGRAYSCALE) )
      goto skip_decode;
  }

  tmEnter( tm_mask, TMZF_NONE, "Decompress video" );
 
    if ( ( ! ( ( addr >= addrend ) || ( addr < compframe ) ) ) && ( lframebuffers || bink->gpu ) )
    {
#if defined(INC_BINK2)
      if ( is_binkv2_or_later( bink->marker ) )
      {
  
  #if defined(INC_BINK2) && defined(__RADNT__)
    if ( ! ( CPU_can_use( CPU_SSE2 ) ) )
      goto no_sse2;
  #endif

        i = ExpandBink2( lframebuffers, addr, ( bink->BinkType&BINKALPHA )?1:0, ( bink->BinkType&BINKHDR )?1:0, bink->compframekey, addrend, &bink->slices, work_flags&255, bink->seam, bink->gpu );

#if defined(INC_BINK2) && defined(__RADNT__)
        no_sse2:;
#endif

      }
      else
#endif
#ifdef NO_BINK10_SUPPORT
        i = 0;
#else
        i = ExpandBink( lframebuffers,
          addr,
          bink->compframekey,
          addrend,
          &bink->bunp,
          i,
          bink->BinkType,
          bink->marker
          );
#endif
    }

    tmLeave( tm_mask );
  }

 skip_decode:

 tmLeave( tm_mask );

  bink->timeadecomp += (U32) audio_time;
  audio_time = RADTimerRead();   // audio_time is just now the current time
  video_time = audio_time - video_time;
  bink->timevdecomp += (U32) video_time;

  if ( ((U32) i) > bink->changepercent )
    bink->changepercent = i;

  if ( work_flags & BINKDOFRAMEEND )
  {
    S32 ret;      
    tmEnter( tm_mask, TMZF_NONE, "End video" );
    ret = end_do_frame( bink, audio_time );
    tmLeave( tm_mask );
    return( ret );
  }
  else
    return( 1 );
}

RADEXPFUNC S32 RADEXPLINK BinkDoFramePlane( HBINK bink, U32 which_planes )
{
  S32 ret = 0;
  static U8 type_to_count[6]={ 0, 24, 12, 8, 6, 3 };  // in 24ths of a frame

  if ( which_planes & BINKDOFRAMESTART )
  {
    LowBinkDoFrameAsync( bink, BINKDOFRAMESTART );
    ret = 1;
  }
  
  if ( which_planes & 255 )
  {
    U32 r, c;
    if ( bink->OpenFlags & BINKOLDFRAMEFORMAT )
      c = 24;
    else
      c = type_to_count[which_planes&255];

    r = get_slice_range( &bink->slices, c, 24 );

    if ( r )
    {
      LowBinkDoFrameAsync( bink, r );
      ret = 1;
    }
  }

  if ( which_planes & BINKDOFRAMEEND )
  {
    LowBinkDoFrameAsync( bink, BINKDOFRAMEEND );
    ret = 1;
  }
  
  return ret; 
}

RADEXPFUNC S32  RADEXPLINK BinkDoFrame(HBINK bink)
{
  U32 r = get_slice_range( &bink->slices, 1,1 );
  if ( r )
    return LowBinkDoFrameAsync( bink, r|BINKDOFRAMESTART|BINKDOFRAMEEND );
  else
    return 0;
}


RADEXPFUNC S32 RADEXPLINK BinkShouldSkip( HBINK bink )
{
  if ( ( bink == 0 ) || ( bink->ReadError ) || ( bink->Paused ) || ( bink->playingtracks == 0 ) || ( bink->soundon == 0 ) || ((bink->FrameNum+2)>=bink->Frames) )
    return( 0 );

  if ( bink->skipped_status_this_frame )  //0=not checked this frame, 1=no skip, 2=skip
    return( bink->skipped_status_this_frame - 1 );  // 0=noskip, 1=skip

  return( shouldskip( bink, 0 ) );
}

static S32 async_in_progress( HBINK bink )
{
  U32 i;
  U32 all=0;
  for( i = 0 ; i < 8 ; i++ )
    all|=bink->async_in_progress[ i ];
  return ( all ) ? 1 : 0;
}

RADEXPFUNC void RADEXPLINK BinkNextFrame(HBINK bink)
{
  S32 skipped=0;

  BinkLog("In BinkNext");

  if (bink==0)
    return;

  if ( async_in_progress( bink ) )
    return;
 
  if ((bink->playingtracks) && (bink->FrameRate) && (bink->Paused==0) && (bink->soundon) ) 
  {
    if ( bink->skipped_status_this_frame == 0 )  //0=not checked this frame, 1=no skip, 2=skip
      shouldskip( bink, 0 );

    // clear here and binkgoto, in case we don't actually have to do the binkgoto
    bink->skipped_status_this_frame = 0;  //0=not checked this frame, 1=no skip, 2=skip

    bink->bio.Working=0;

    if ( bink->very_delayed )
    {
      skipped = 1;
      bink->very_delayed = 0;
    }
    else
    {
      U32 i;

      //
      // See if the sound skipped
      //

      for( i = 0 ; i < bink->playingtracks ; i++ )
      {
        if (bink->bsnd[i].SoundDroppedOut)
        {
          bink->bsnd[i].SoundDroppedOut=0;

          if ((bink->FrameNum>1) && ((S32)bink->FrameNum<=(S32)bink->bsnd[i].sndendframe))
          {
            skipped = 1;
          }
        }
      }
    }

    if ( skipped )
    {
      bink->soundskips++;
      BinkSetSoundOnOff( bink, 0 );
      bink->startsynctime=0;
      bink->paused_sync_diff = 0;
      BinkSetSoundOnOff( bink, 1 );
    }
  }

  bink->bio.Working=1;

  ServiceSound(bink);

  timeblitframe(bink,RADTimerRead());

  GotoFrame(bink,(bink->FrameNum>=bink->Frames)?1:(bink->FrameNum+1));
  BinkLog("Out BinkNext");

  bink->bio.Working=0;
}

RADEXPFUNC U32 RADEXPLINK BinkGetKeyFrame(HBINK bnk,U32 frame,S32 flags)
{
  S32 i;
  U32 t;

  if (bnk==0)
    return(0);
  
  if ( frame == 0 )
    frame = 1;
  
  if (((flags&BINKGETKEYNOTEQUAL)==0) && (bnk->frameoffsets[frame-1]&1))
    return(frame);

  switch (flags&127)
  {
    case BINKGETKEYPREVIOUS:
      i=frame-2;
      while (i>=1)
      {
        if (bnk->frameoffsets[i]&1)
          break;
        --i;
      }
      return(i+1);
    case BINKGETKEYNEXT:
      t=frame;
      while (t<bnk->Frames)
      {
        if (bnk->frameoffsets[t++]&1)
          return(t);
      }
      break;
    case BINKGETKEYCLOSEST:
      i=frame-2;
      t=frame;
      for(;;)
      {
        if (i>=0)
        {
          if (bnk->frameoffsets[i]&1)
            return(i+1);
          if (t<bnk->Frames)
            if (bnk->frameoffsets[t++]&1)
              return(t);
        }
        else
        {
          if (t<bnk->Frames)
          {
            if (bnk->frameoffsets[t++]&1)
              return(t);
          }
          else
            return(0);
        }
        --i;
      }
// never get here      break;
  }

  return(0);
}


// goto possibilities:
//  1) GOTOQUICK - just jumps directly to frame
//  2) GOTOQUICK|GOTOWITHSOUND - sound off, fill with silence, jump directly
//     to frame-3/4 of second, turn off video and skip forward until the
//     frame making sure any sound data will always fit, turn on video, turn on sound
//  3) nothing - jumps to the nearest key frame, returns if that was the current frame,
//     otherwise sound off, fill with silence, decode to current frame making sure
//     any sound data will always fit, turn on sound
//  4) GOTOWITHSOUND - sound off, fill with silence, get the nearest key frame,
//     if the nearest is later than 3/4 of second earlier then turn off video
//     and jump to 3/4 of a second back, skip forward until the frame making
//     sure the audio will fit, if you turned off the video and hit the nearest
//     keyframe then turn the video back on and continue playing forward, sound on

RADEXPFUNC void RADEXPLINK BinkGoto(HBINK bnk,U32 frame,S32 flags)
{
  U32 key_frame, sound_frame, goto_frame;
  S32 save_snd, save_vid;

  if (bnk==0)
    return;

  if ( async_in_progress( bnk ) )
    return;

  if ( frame == 0 )
    frame = 1;
  else if ( frame > bnk->Frames )
    frame = bnk->Frames;


  bnk->skipped_status_this_frame = 0;  //0=not checked this frame, 1=no skip, 2=skip

  bnk->bio.Working=1;

  if (frame>bnk->Frames)
    frame=bnk->Frames;

  if ( ( flags & BINKGOTOQUICKSOUND ) || ( !bnk->playingtracks ) )
  {
    sound_frame = frame;
  }
  else
  {
    sound_frame = ( bnk->fileframerate + ( bnk->fileframeratediv - 1 ) ) / bnk->fileframeratediv;
    if ( sound_frame >= frame )
      sound_frame = 1;
    else
      sound_frame = frame - sound_frame;
  }

  if (bnk->FrameNum!=frame)
  {
    // if not a quick goto or if we need sound
    if ( ( ( flags & BINKGOTOQUICK ) == 0 ) || ( ( flags & BINKGOTOQUICKSOUND ) == 0 ) )
    {
      key_frame = (flags&BINKGOTOQUICK) ? frame : BinkGetKeyFrame(bnk,frame,BINKGETKEYPREVIOUS);
      save_vid = 0;
      if ( key_frame > sound_frame )
      {
        goto_frame = sound_frame;
        if ((frame<bnk->FrameNum) || (goto_frame>bnk->FrameNum) || ( ( flags & BINKGOTOQUICKSOUND ) == 0 ) )
        {
          save_vid = bnk->videoon;
          BinkSetVideoOnOff(bnk,0);
        }
      }
      else
      {
        goto_frame = key_frame;
      }

      if ((frame<bnk->FrameNum) || (goto_frame>bnk->FrameNum))
      {
        GotoFrame(bnk,goto_frame);
        if (goto_frame==frame)
        {
          bnk->bio.Working=0;
          if ( bnk->soundon )
          {
            BinkSetSoundOnOff(bnk,0);
            BinkSetSoundOnOff(bnk,1);
          }
          return;
        }
      }

      // only stop the sound if we did a goto
      bink_suspend_snd( bnk );

      save_snd = bnk->soundon;
      if ( save_snd )
        BinkSetSoundOnOff(bnk,0);

      if (bnk->FrameNum!=bnk->lastdecompframe)
      {
        BinkDoFrame(bnk);
        // set this because we aren't going to blit this frame
        bnk->skippedlastblit = 1;
      }

      BinkNextFrame(bnk);
      while (bnk->FrameNum!=frame)
      {
        if (bnk->needio)
        {
          our_lock( (rrMutex*) bnk->pri_io_mutex, "Bink instance priority io mutex" );

          if ( bnk->needio )
            bnk->bio.Idle(&bnk->bio);
          our_unlock( (rrMutex*) bnk->pri_io_mutex, "Bink instance priority io mutex" );
        }

        // if we hit our key frame (once we got our sound), restart the video
        if ( ( save_vid ) && ( bnk->FrameNum == key_frame ) )
        {
          BinkSetVideoOnOff( bnk, 1 );
          save_vid = 0;
        }

        BinkDoFrame(bnk);
        // set this because we aren't going to blit this frame
        bnk->skippedlastblit = 1;
        BinkNextFrame(bnk);
      }

      bnk->startsynctime = 0;
      bnk->paused_sync_diff = 0;

      if ( save_vid )
      {
        BinkSetVideoOnOff( bnk, 1 );
      }

      bink_resume_snd( bnk );

      if ( save_snd )
        BinkSetSoundOnOff(bnk, 1 );
    }
    else
    {
      GotoFrame(bnk,frame);
    }
  }

  bnk->bio.Working=0;
}

BinkFreeGlobalsCallback * BinkFreeGlobals1 = 0;
BinkFreeGlobalsCallback * BinkFreeGlobals2 = 0;

RADEXPFUNC void RADEXPLINK BinkFreeGlobals( void )
{
  if ( BinkFreeGlobals1 )
    BinkFreeGlobals1();
  if ( BinkFreeGlobals2 )
    BinkFreeGlobals2();

  free_snd_globals();
  free_io_globals();
}

RADEXPFUNC void RADEXPLINK BinkClose(HBINK bink)
{
  U32 i;
  
  BinkLog("In BinkClose");
  if (bink==0)
    return;

  bink->closing = 1;
  
  BinkPause(bink,1);

  wait_for_asyncs(bink);

  remove_from_bink_list( bink );
  sync_io_service( bink );
  sync_snd_service( bink );

  if ((bink->OpenFlags&BINKFILEDATALOADONLY)==0)
  {
    our_lock( (rrMutex*) bink->pri_io_mutex, "Bink instance priority io mutex" );
    bink->needio = 0;  // cancel priority io request
    our_unlock( (rrMutex*) bink->pri_io_mutex, "Bink instance priority io mutex" );
  }

#ifdef USE_THREADS
  if (bink_io_last == bink)
    bink_io_last = 0;
#endif
  
  lock_snd_global();

  // close the audio if we are playing any
  for( i = 0 ; i < bink->playingtracks ; i++ )
  {
    bink->bsnd[i].Close(&bink->bsnd[i]);
    --numopensounds;
  }

  unlock_snd_global();

  if (bink->binkaudiomem)
     popfree(bink->binkaudiomem);

  // free all the memory pointers
  if (bink->preloadptr)
  {
    if ((bink->OpenFlags&(BINKFROMMEMORY|BINKFILEDATALOADONLY))==0)
      popfree(bink->preloadptr);
  }
  else
  {
    lock_io_global();

    bink->bio.Close(&bink->bio);

    unlock_io_global();

    popfree(bink->ioptr);
  }

  close_io_service_for_bink( bink );
  close_snd_service_for_bink( bink );

  if (bink->allocatedframebuffers)
    popfree(bink->allocatedframebuffers);

  if (bink->subimage)
    popfree(bink->subimage);

  rrmemsetzero(bink,sizeof(BINK));
  popfree( ((U8*)bink)-((U32*)bink)[-1] );

  BinkLog("Out BinkClose");
}

RADEXPFUNC S32  RADEXPLINK BinkGetPlatformInfo( S32 bink_platform_enum, void * output )
{
  switch( bink_platform_enum )
  {
    case BINKPLATFORMSOUNDTHREAD:
      return get_snd_thread( output );

    case BINKPLATFORMIOTHREAD:
     return get_io_thread( output );

#if defined(RADPLATFORMSUBMITTHREADGET) && defined(USE_SND_THREAD)
    case BINKPLATFORMSUBMITTHREADCOUNT:
    {
      *(U32*)output = 1;
      return 1;
    }
    case BINKPLATFORMSUBMITTHREADS:
    {
      UINTa* output_ptr = (UINTa*)output;
      our_lock( &bink_list, "Bink list mutex" );
      output_ptr[0] = RADPLATFORMSUBMITTHREADGET();
      our_unlock( &bink_list, "Bink list mutex" );
      return 1;
    }
#endif

    case BINKPLATFORMASYNCTHREADS+0:
    case BINKPLATFORMASYNCTHREADS+1:
    case BINKPLATFORMASYNCTHREADS+2:
    case BINKPLATFORMASYNCTHREADS+3:
    case BINKPLATFORMASYNCTHREADS+4:
    case BINKPLATFORMASYNCTHREADS+5:
    case BINKPLATFORMASYNCTHREADS+6:
    case BINKPLATFORMASYNCTHREADS+7:
      return get_async_thread( bink_platform_enum - BINKPLATFORMASYNCTHREADS, output );

  }

  return 0;
}


RADEXPFUNC S32 RADEXPLINK BinkWait(HBINK bink)
{
  S32 waittime,curtime,timer,synctime;

  if (bink==0)
    return(0);

  if (((bink->playedframes==0) && (!bink->Paused)) || (bink->ReadError))
    return(0);

  // we we have background IO and we still need data, keep waiting
  if ( ( ( bink->OpenFlags & BINKNOTHREADEDIO ) == 0 ) && ( bink->needio ) )
    return 1;

  synctime=bink->startsynctime;
  if (synctime==0)
  {
    inittimer(bink);
    synctime=bink->startsynctime;
  }

  ServiceSound(bink);
  timer=RADTimerRead();

  timeblitframe(bink,timer);

  if ((bink->Paused) || ((bink->playingtracks) && (bink->soundon==0)))
    goto idle;

  if (bink->FrameRate==0)
    return(0);

  waittime = mult64andshift(mult64anddiv((bink->playedframes-bink->startsyncframe)*1000,bink->FrameRateDiv,bink->FrameRate),bink->bsnd[0].VideoScale,16);
  curtime = timer - synctime;
  curtime -= waittime;

  if ( curtime >= 0 )
  {
    if ( ( bink->playingtracks == 0 ) && ( curtime > (S32) (bink->twoframestime*2) ) )
      bink->startsynctime=0;
    return(0);
  }

 idle:

  if (bink->preloadptr==0)
  {
    if ( bink->OpenFlags & BINKNOTHREADEDIO )
    {
      // don't do a read, if they have turned off background
      if ( bink->bio.BGControl )
      {
        if ( bink->bio.BGControl( &bink->bio, 0 ) & BINKBGIOSUSPEND )
          return 1;
      }

      bink->bio.Idle(&bink->bio);
    }  
  }

  return(1);
}


RADEXPFUNC S32 RADEXPLINK BinkPause( HBINK bink, S32 pause )
{
  U32 timer;
  U32 i;
  S32 reset_start_sync = 0;
  U32 original_pause;
  
  if ( bink == 0 )
    return( 0 );

  if ( bink->Paused == pause )
    return( pause );

  timer=RADTimerRead();
  timeblitframe( bink, timer );
  bink->lastfinisheddoframe=0;

  original_pause = bink->Paused;
  bink->Paused=pause;
  
  bink_suspend_snd( bink );

  for( i = 0 ; i < bink->playingtracks ; i++ )
    bink->bsnd[ i ].Pause( &bink->bsnd[ i ], pause );
  
  timer=RADTimerRead();

  // snapshot the pause time
  if ( pause == 0 )
  {
    if ( original_pause )
    {
      reset_start_sync = 1;
    }
  }
  else
  {
    // if no sound, just reset sync time completely
    if ( bink->playingtracks == 0 )
      bink->startsynctime = bink->paused_sync_diff = 0;

    if ( !original_pause ) 
    {
      // save the current delta between start sync and the timer (so 
      //   we can restore this delta when we resume)
      if ( bink->startsynctime )
        bink->paused_sync_diff = timer - bink->startsynctime;
      else
        bink->paused_sync_diff = 0;
    }
  }

  bink_resume_snd( bink );

  ServiceSound( bink );

  if ( reset_start_sync )
  {
    bink->startsynctime = timer - bink->paused_sync_diff;
  }

  wake_io_thread();

  return( bink->Paused );
}


RADEXPFUNC void RADEXPLINK BinkGetSummary(HBINK bink,BINKSUMMARY* sum)
{
  U32 timer;
  U64 size;

  if ((bink) && (sum))
  {

    timer=RADTimerRead();
    timeblitframe(bink,timer);

    rrmemsetzero(sum,sizeof(*sum));
    sum->FrameRate=bink->FrameRate;
    sum->FrameRateDiv=bink->FrameRateDiv;
    sum->SkippedBlits=bink->skippedblits;
    sum->SoundSkips=bink->soundskips;
    sum->FileFrameRate=bink->fileframerate;
    sum->FileFrameRateDiv=bink->fileframeratediv;
    sum->TotalFrames=bink->Frames;
    sum->TotalPlayedFrames=bink->playedframes;
    sum->TotalTime=RADTimerRead()-bink->firstframetime;
    sum->TotalOpenTime=bink->timeopen;
    sum->TotalAudioDecompTime=bink->timeadecomp;
    sum->TotalVideoDecompTime=bink->timevdecomp;
    sum->TotalBlitTime=bink->timeblit;
    sum->HighestMemAmount+=bink->totalmem;
    sum->TotalIOMemory=bink->iosize;
    if ( bink->bio.TotalTime < 1000 )
      sum->TotalReadSpeed=bink->bio.BytesRead*(1000/(bink->bio.TotalTime+1));
    else
      sum->TotalReadSpeed=(bink->bio.BytesRead*1000/bink->bio.TotalTime);
    sum->TotalReadTime=bink->bio.ForegroundTime;
    sum->TotalIdleReadTime=bink->bio.IdleTime;
    sum->TotalBackReadTime=bink->bio.ThreadTime;

    size=(bink->Size+8-(bink->frameoffsets[0]&~3));
    sum->AverageDataRate=(U32)(size*(U64)bink->fileframerate/((U64)bink->fileframeratediv*(U64)bink->Frames));
    sum->AverageFrameSize=(U32)(size/bink->Frames);

    sum->Highest1SecRate=bink->Highest1SecRate;
    sum->Highest1SecFrame=bink->Highest1SecFrame+1;

    sum->Width=bink->Width;
    sum->Height=bink->Height;

    sum->SlowestFrameTime=bink->slowestframetime;
    sum->Slowest2FrameTime=bink->slowest2frametime;

    sum->SlowestFrameNum=bink->slowestframe;
    sum->Slowest2FrameNum=bink->slowest2frame;

    sum->TotalIOMemory=bink->bio.BufSize;
    sum->HighestIOUsed=bink->bio.BufHighUsed;

  }
}


RADEXPFUNC void RADEXPLINK BinkGetRealtime(HBINK bink,BINKREALTIME * run,U32 frames)
{
  U32 timer;
  U32 rtframes;

  if ( ( bink == 0 ) || ( run == 0 ) )
    return;
  
  timer=RADTimerRead();
  timeblitframe(bink,timer);

  if ((frames==0) || (frames>=bink->runtimeframes))
    frames=(bink->runtimeframes-1);

  if (frames>bink->FrameNum)
  {
    frames=bink->FrameNum-1;
  }

  if (frames==0)
  {
    frames=1;
  }

  run->FrameNum=bink->LastFrameNum;
  run->FrameRate=bink->FrameRate;
  run->FrameRateDiv=bink->FrameRateDiv;

  run->ReadBufferSize=bink->bio.CurBufSize;
  run->ReadBufferUsed=bink->bio.CurBufUsed;
  run->FramesDataRate=(((bink->frameoffsets[bink->FrameNum]&~3)-(bink->frameoffsets[bink->FrameNum-frames]&~3))*(U64)bink->fileframerate)/((U64)frames*bink->fileframeratediv);

  rtframes = frames + bink->rtindex;
  if ( rtframes >= bink->runtimeframes )
    rtframes -= bink->runtimeframes;

  run->Frames=frames;
  run->FramesTime=bink->rtframetimes[bink->rtindex]-bink->rtframetimes[rtframes];
  if (run->FramesTime==0)
    run->FramesTime=1;

  run->FramesVideoDecompTime=bink->rtvdecomptimes[bink->rtindex]-bink->rtvdecomptimes[rtframes];
  run->FramesAudioDecompTime=bink->rtadecomptimes[bink->rtindex]-bink->rtadecomptimes[rtframes];
  run->FramesBlitTime=bink->rtblittimes[bink->rtindex]-bink->rtblittimes[rtframes];
  run->FramesReadTime=bink->rtreadtimes[bink->rtindex]-bink->rtreadtimes[rtframes];
  run->FramesIdleReadTime=bink->rtidlereadtimes[bink->rtindex]-bink->rtidlereadtimes[rtframes];
  run->FramesThreadReadTime=bink->rtthreadreadtimes[bink->rtindex]-bink->rtthreadreadtimes[rtframes];
}


RADEXPFUNC S32  RADEXPLINK BinkGetRects(HBINK bnk)
{
  if (bnk==0)
    return(0);

  // We used to do fine-grained dirty tracking. Not anymore.
  bnk->FrameRects[0].Left=0;
  bnk->FrameRects[0].Top=0;
  bnk->FrameRects[0].Width=bnk->Width;
  bnk->FrameRects[0].Height=bnk->Height;
  bnk->NumRects=1;

  return( 1 );
}


RADEXPFUNC void RADEXPLINK BinkService(HBINK bink)
{
  if ( bink )
  {
    ServiceSound(bink);

    if (bink->preloadptr==0)
    {
      if ( bink->OpenFlags & BINKNOTHREADEDIO )
      {
        bink->bio.Idle(&bink->bio);
      }
    }
  }
}


static S32 idtoindex(HBINK bink,U32 trackid)
{
  S32 i;
  for( i = 0 ; i < (S32)bink->playingtracks ; i++ )
  {
    if (bink->trackIDs[bink->trackindexes[i]]==(S32)trackid)
      return( i );
  }
  return( -1 );
}

RADEXPFUNC void RADEXPLINK BinkSetVolume(HBINK bnk,U32 trackid,S32 volume)
{
  #ifdef DUMPVOLUMES
    char buf[256]; 
    sprintf(buf,"Volume set: id: %d volume: %d\n",trackid,volume );
    OutputDebugString(buf);
  #endif

  if (bnk)
  {
    if (bnk->playingtracks)
    {
      S32 j = idtoindex(bnk, trackid);
      if ( j != -1 )
      {
        if (bnk->bsnd[j].Volume)
          bnk->bsnd[j].Volume(&bnk->bsnd[j],volume);
      }
    }
  }
}


RADEXPFUNC void RADEXPLINK BinkSetSpeakerVolumes(HBINK bnk,U32 trackid,U32 * spks,S32 * volumes, U32 total)
{
  static const float one = 1.0f;
  static const float two = 2.0f;
  static const float half = 0.5f;

  #ifdef DUMPVOLUMES
    char buf[256]; 
    sprintf(buf,"Speaker Total: %d\n",total );
    OutputDebugString(buf);
    if (spks)
    {
      U32 t;
      OutputDebugString("Speakers: " );
      for(t=0;t<total;t++)
      {
        sprintf(buf,"%d ",spks[t] );
        OutputDebugString( buf );
      }
      OutputDebugString( "\n" );
    }
    if (volumes)
    {
      U32 t;
      OutputDebugString("Volumes: " );
      for(t=0;t<total;t++)
      {
        sprintf(buf,"%d ",volumes[t] );
        OutputDebugString( buf );
      }
      OutputDebugString( "\n" );
    }
  #endif

  if (bnk)
  {
    if (bnk->playingtracks)
    {
      S32 j = idtoindex(bnk, trackid);
      if ( j != -1 )
      {
        if (bnk->bsnd[j].SpeakerVols)
        {
          U32 i;
          F32 fvols[ 8 ];
          F32 vols[ 8 * 2 ];
          U32 chs, chm;
        
          if ( total > 8 )
            total = 8;
        
          // convert the volumes to float
          if ( volumes )
          {
            for( i = 0 ; i < total ; i++ )
              fvols[ i ] = ( volumes[ i ] >= 65536 ) ? two : ( ( (F32) volumes[ i ] ) * ( 1.0f / 32768.0f ) );
          }
          else
          {
            for( i = 0 ; i < total ; i++ )
              fvols[ i ] = one;
          }
          if ( total == 0 )
            fvols[ 0 ] = one;
        
          // clear out the speaker matrix
          for( i = 0 ; i < ( 8 * 2 ) ; i++ )
            vols[ i ] = 0.0f;
        
          chs = 0;
          if ( bnk->bsnd[j].chans == 2 )
            chs = 1;
          chm = bnk->bsnd[j].chans - 1;
        
          // if we don't have any direct spk assignments,
          //   replicate the volumes out based on how many total
          //   indexes they set.  So, if spks is 0, and you only
          //   set two volumes, then replicate the stereo pairs
          //   across the stereo sets
        
          if ( spks == 0 )
          {
            S32 fr, rl, rr, sl, sr;
              
            fr = 0; rl = 0; rr = 0; sr = 0; sl = 0;
            if ( total >= 2 )
            {
              fr = 1; 
              rr = 1;
              sr = 1;
              
              if ( total >= 5 )
              {
                rl = 4;
                rr = 4;
                sl = 4;
                sr = 4;
                
                if ( total >= 6 )
                {
                  rr = 5;
                  sr = 5;
        
                  if ( total >= 7 )
                  {
                    sl = 6;
                    sr = 6;
                    
                    if ( total >= 8 )
                      sr = 7;
                  }
                }
              }
            }
            
            // vols = [ source_chans * dest_index + src_index ]
            //   or vols = [ ( dest_index << source_chan_shift ) + src_index ]
            
            vols[ ( 0 << chs ) + 0 ] = fvols[ 0 ];
            vols[ ( 1 << chs ) + chm ] = fvols[ fr ];
        
            if ( chm == 1 ) // if stereo
            {
              fvols[ 2 ] *= half;
              fvols[ 3 ] *= half;
        
              // if playing stereo, merge levels for LR to center and sub
              vols[ ( 2 << 1 ) + 1 ] = fvols[ 2 ];
              vols[ ( 3 << 1 ) + 0 ] = fvols[ 3 ];
            }
        
            vols[ ( 2 << chs ) + 0 ] = fvols[ 2 ];
            vols[ ( 3 << chs ) + chm ] = fvols[ 3 ];
        
            vols[ ( 4 << chs ) + 0 ] = fvols[ rl ];
            vols[ ( 5 << chs ) + chm ] = fvols[ rr ];
                
            vols[ ( 6 << chs ) + 0 ] = fvols[ sl ];
            vols[ ( 7 << chs ) + chm ] = fvols[ sr ];
          
          }
          else
          {
            // copy the floats into the matrix based on the spks
            for( i = 0 ; i < total ; i++ )
            {
              vols[ ( spks[ i ] << chs ) + ( i & chm ) ] = fvols[ i ];
            }
          }

          if ( total < 8 )
            total = 8;
          
          //  can set limit_speakers to 8 to override platform default
          if ( bnk->limit_speakers )
          {
            if ( bnk->limit_speakers < total )
              total = bnk->limit_speakers;
          }

          bnk->bsnd[j].SpeakerVols( &bnk->bsnd[j], vols, total );
        }
      }
    }
  }
}


RADEXPFUNC void RADEXPLINK BinkSetPan(HBINK bnk,U32 trackid,S32 pan)
{
  if (bnk)
  {
    if (bnk->playingtracks)
    {
      S32 j = idtoindex(bnk, trackid);
      if ( j != -1 )
      {
        if (bnk->bsnd[j].Pan)
          bnk->bsnd[j].Pan(&bnk->bsnd[j],pan);
      }
    }
  }
}


RADEXPFUNC U32  RADEXPLINK BinkGetTrackType(HBINK bnk,U32 trackindex)
{
  if (bnk)
    return( bnk->tracktypes[trackindex] );
  return(0);
}

RADEXPFUNC U32  RADEXPLINK BinkGetTrackMaxSize(HBINK bnk,U32 trackindex)
{
  if (bnk)
    return( bnk->tracksizes[trackindex] + 512 );
  return(0);
}

RADEXPFUNC U32  RADEXPLINK BinkGetTrackID(HBINK bnk,U32 trackindex)
{
  if (bnk)
    return( bnk->trackIDs[trackindex] );
  return(0);
}

RADEXPFUNC HBINKTRACK RADEXPLINK BinkOpenTrack(HBINK bnk,U32 trackindex)
{
  HBINKTRACK bnkt;
  void * sndcomp;
  U32 size;
  char pmbuf[ PushMallocBytesForXPtrs( 8 ) ];
  pushmallocinit( pmbuf, 8 );

  if (bnk)
  {

    if (trackindex>=(U32)bnk->NumTracks)
      return(0);

    // does the track contain sound?
    if (!(GETTRACKFORMAT(bnk->tracktypes[trackindex])))
      return(0);

    size=BinkAudioDecompressMemory(GETTRACKFREQ(bnk->tracktypes[trackindex]),
                                   GETTRACKCHANS(bnk->tracktypes[trackindex]),
                                   GETTRACKNEWFORMAT(bnk->OpenFlags,bnk->tracktypes[trackindex]));

    pushmalloc(pmbuf, &sndcomp,size);
    bnkt=(HBINKTRACK)bpopmalloc(pmbuf,bnk,sizeof(BINKTRACK));

    sndcomp=BinkAudioDecompressOpen(sndcomp,
                                    GETTRACKFREQ(bnk->tracktypes[trackindex]),
                                    GETTRACKCHANS(bnk->tracktypes[trackindex]),
                                    GETTRACKNEWFORMAT(bnk->OpenFlags,bnk->tracktypes[trackindex]));
    if (sndcomp==0)
    {
      popfree( bnkt );
      return(0);
    }
    
    rrmemsetzero(bnkt,sizeof(BINKTRACK));

    bnkt->bink=bnk;
    bnkt->sndcomp=(UINTa)sndcomp;

    bnkt->Frequency=GETTRACKFREQ(bnk->tracktypes[trackindex]);
    bnkt->Channels=GETTRACKCHANS(bnk->tracktypes[trackindex]);
    bnkt->MaxSize=8192+((bnk->tracksizes[trackindex]+3)&~3);

    bnkt->MaxSize += ( BinkAudioDecompressOutputSize( (HBINKAUDIODECOMP)bnkt->sndcomp ) / 16 );
      
    bnkt->trackindex=trackindex;

    return(bnkt);

  }
  return(0);
}


RADEXPFUNC void RADEXPLINK BinkCloseTrack(HBINKTRACK bnkt)
{
  if (bnkt)
  {
    bnkt->sndcomp=0;
    popfree(bnkt);
  }
}

RADEXPFUNC U32  RADEXPLINK BinkGetTrackData(HBINKTRACK bnkt,void * dest,U32 destlen)
{
  if ( (bnkt) && (dest) )
  {
    S32 i;
    U8 const * addr=(U8*)bnkt->bink->compframe;
    U8 const * addrend=addr + bnkt->bink->compframesize;

    check_for_pending_io( bnkt->bink );
    if ( bnkt->bink->ReadError )
      return 0;

    // do sound
    for(i=0;i<bnkt->bink->NumTracks;i++)
    {
      U8 const * saddrend;
      U32 amt=*((U32*)addr);
      addr+=4;

      saddrend = addr + amt;
      if ( ( addr > addrend ) || ( addr < (U8*)bnkt->bink->compframe ) || ( saddrend > addrend ) ) break;

      // decompress sound here
      if ((i==bnkt->trackindex) && (amt))
      {
        U8 const * taddrend;
        U32 tot=0;
        U32 decomp=*((U32*)addr);
        addr+=4;

        taddrend = addr + amt;

        while (decomp)
        {
          BINKAC_OUT_RINGBUF out;
          BINKAC_IN in;

          out.outptr = dest;
          out.outstart = dest; 
          out.outend = (S16*)(( (U8*)dest ) + destlen);
          out.outlen = destlen;
          out.eatfirst = 0;

          in.inptr = addr; 
          in.inend = taddrend;

          BinkAudioDecompress((HBINKAUDIODECOMP)bnkt->sndcomp,&out,&in);
          addr = in.inptr;

          if (out.outlen>decomp)
            out.outlen=decomp;

          decomp-=out.outlen;

          dest=((U8*)dest)+out.outlen;
          tot+=out.outlen;

        }

        return(tot);
      }

      addr+=amt;
    }

  }

  return(0);
}


RADEXPFUNC S32  RADEXPLINK BinkSetVideoOnOff(HBINK bnk,S32 onoff)
{
  if (bnk)
    bnk->videoon=onoff;

  return(onoff);
}

RADEXPFUNC S32  RADEXPLINK BinkSetSoundOnOff(HBINK bink,S32 onoff)
{
  S32 state=0;

  if (bink)
  {
    S32 suspended = 0;
    U32 i;
    S32 soundon = bink->soundon;

    // if no sound, just reset the sound sync
    if ( bink->playingtracks == 0 )
      bink->startsynctime = bink->paused_sync_diff = 0;

    for ( i = 0 ; i < bink->playingtracks ; i++ )
    {
      if (bink->bsnd[i].SetOnOff)
      {
        S32 ret;

        if ( !suspended )
        {
          if ( ( bink->bsnd[0].ThreadServiceType <= 0 ) || ( bink->bsnd[0].ThreadServiceType == 2 ) )
          {
            if ( bink->bsnd[0].ThreadServiceType < 0 )
            {
              suspended=1;
              lock_snd_global();
            }
            else
            {
              suspended=2;
              our_lock( (rrMutex*) bink->snd_mutex, "Bink instance sound mutex" );
            }
          }
        }

        ret=bink->bsnd[i].SetOnOff(&bink->bsnd[i],onoff?1:0);

        if (ret)
          state=1;

        if ((ret==0) && (soundon))
        {
          // we clear the buffer on every soundonoff, but we may only want to do this if
          //    the goto amount is really different - if skipping ten frames forward,
          //    we should only kill ten frames of sound - that would have to record
          //    the start onoff and the end onoff and delta (and it would have to have
          //    changes made in BinkGoto, so that the start and end frames worked
          //  This might need to be another SoundOnOff completely, so that this function
          //    always clears the buffer first.  For now, let's not make drastic changes.
          bink->bsnd[i].sndwritepos=bink->bsnd[i].sndbuf;
          bink->bsnd[i].sndreadpos=bink->bsnd[i].sndbuf;

          bink->soundon=0;
        }
        else
          if ((ret) && (soundon==0))
          {
            // if we are about to decompress frame 1, don't fill anything (leave empty from
            //    the previous call to turn off the sound).
            if ( ( bink->FrameNum != 1 ) || ( bink->lastdecompframe == bink->FrameNum ) )
              dosilencefillorclamp( &bink->bsnd[i] );

            // do on
            bink->soundon=1;

            bink->startsynctime = 0;
            bink->paused_sync_diff = 0;
          }
      }
    }

    if (suspended)
    {
      if ( suspended == 1 )
      {
        unlock_snd_global();
      }
      else
      {
        our_unlock( (rrMutex*) bink->snd_mutex, "Bink instance sound mutex" );
      }
    }
  }

  return(state);
}

RADEXPFUNC void RADEXPLINK BinkSetMemory(BINKMEMALLOC a,BINKMEMFREE f)
{
  RADSetMemory( a, f );
}

RADEXPFUNC void RADEXPLINK BinkSetWillLoop(HBINK bink,S32 onoff)
{
  if ( bink )
  {
    if ( onoff )
      bink->OpenFlags |=  BINKWILLLOOP;
    else
      bink->OpenFlags &= ~BINKWILLLOOP;
  }
}

RADEXPFUNC S32 RADEXPLINK BinkControlBackgroundIO(HBINK bink,U32 control)
{
  if ( ( bink ) && ( bink->bio.BGControl ) )
  {
    S32 ret;
    ret = bink->bio.BGControl( &bink->bio,control);
    wake_io_thread();
    return ret;
  }

  return( -1 );
}

RADEXPFUNC void * RADEXPLINK BinkUtilMalloc(U64 bytes)
{
  return popmalloc( 0, bytes );
}

RADEXPFUNC void RADEXPLINK BinkUtilFree(void * ptr)
{
  popfree( ptr );
}

RADEXPFUNC void RADEXPLINK BinkUtilSoundGlobalLock(void)
{
  lock_snd_global();
}

RADEXPFUNC void RADEXPLINK BinkUtilSoundGlobalUnlock(void)
{
  unlock_snd_global();
}

#include "defread.h"

BINKFILEOPEN  binkosopen = binkdefopen;  // returns 1 on success
BINKFILEREAD  binkosread = binkdefread;
BINKFILESEEK  binkosseek = binkdefseek;
BINKFILECLOSE binkosclose = binkdefclose;

RADEXPFUNC void RADEXPLINK BinkSetOSFileCallbacks( BINKFILEOPEN o, BINKFILEREAD r, BINKFILESEEK s, BINKFILECLOSE c )
{
  if ( ( o == 0 ) || ( r == 0 ) || ( s == 0 ) || ( c == 0 ) )
  {
    binkosopen = binkdefopen;  
    binkosread = binkdefread;
    binkosseek = binkdefseek;
    binkosclose = binkdefclose;
  }
  else
  {
    binkosopen =  o;
    binkosread =  r;
    binkosseek =  s;
    binkosclose = c;
  }
}

RADDEFFUNC S32  RADLINK binkdefopen (UINTa * user_data, const char * fn, U64 * optional_out_file_size);
RADDEFFUNC U64  RADLINK binkdefread (UINTa * user_data, void * dest, U64 bytes);
RADDEFFUNC void RADLINK binkdefseek (UINTa * user_data, U64 pos);
RADDEFFUNC void RADLINK binkdefclose(UINTa * user_data);

#define STB_SUB_IMPLEMENTATION
#include "stb_subtitle.h"

RADEXPFUNC S32 RADEXPLINK BinkLoadSubtitles( HBINK bink, char const * srt_file )
{
  STB_SRT_SUB * sub;
  stb_srt_error err;
  U64 filesize, fileread;
  UINTa fileinfo[2];

  bink->subvalid = 0;

  // if they want to force free the subtitle memory
  if ( srt_file == 0 )
  {
    popfree( bink->subimage );
    bink->subimage = 0;
    bink->subimagesize = 0;
    return 0;
  }

  if ( !binkosopen( fileinfo, srt_file, &filesize ) )
  {
    BinkSetError("Error opening file.");
    return 0;
  }

  // only support 1 MB srt files
  if ( filesize >= ( 1 << 20 ) )
  {
   memerr: 
    binkosclose( fileinfo );
    BinkSetError("Out of memory.");
    return 0;
  }

  // if we have an image ptr, see if it's big enough, otherwise free it
  if ( bink->subimage )
  {
    if ( filesize > bink->subimagesize )
    {
      popfree( bink->subimage );
      bink->subimage = 0;
      bink->subimagesize = 0;
    }
  }

  // alloc a pointer if we don't have one
  if ( bink->subimage == 0 )
  {
    bink->subimage = bpopmalloc( 0, bink, filesize );
    if (bink->subimage == 0 )
      goto memerr;
    bink->subimagesize = (U32) filesize;
  }

  // read the file
  fileread = binkosread( fileinfo, bink->subimage, filesize );
  binkosclose( fileinfo );

  // bad read
  if ( filesize != fileread )
  {
    BinkSetError( "Error reading file." );
    return 0;
  }

  // try to parse now
  sub = stb_srt_parse_image( bink->subimage, ( (char*)bink->subimage ) + filesize, &err );
  if ( !sub )
  {
    if ( err == stb_srt_utf16_not_supported )
      BinkSetError("Error: UTF16 subtitles not supported.");
    else
      BinkSetError("Error: couldn't parse subtitles file.");
    return 0;
  }

  bink->subvalid = sub;
  return 1;
}

RADEXPFUNC char const * RADEXPLINK BinkCurrentSubtitle( HBINK bink, U32 * iterate, U32 * opt_out_start_ms, U32 * opt_out_end_ms )
{
  if ( ( bink ) && ( bink->subvalid ) )
  {
    STB_SRT_SUB * sub;
    char const * str;
    U32 ms;

    sub = (STB_SRT_SUB *) bink->subvalid;
    ms = mult64andshift(mult64anddiv(bink->FrameNum*1000,bink->FrameRateDiv,bink->FrameRate),bink->bsnd[0].VideoScale,16);
    str = stb_srt_find_subtitle( sub, ms, iterate, opt_out_start_ms, opt_out_end_ms );
    return str;
  }
  return 0;
}

RADEXPFUNC char const * RADEXPLINK BinkGetSubtitleByIndex( HBINK bink, U32 index, U32 * opt_out_start_ms, U32 * opt_out_end_ms )
{
  if ( ( bink ) && ( bink->subvalid ) )
  {
    STB_SRT_SUB * sub;
    char const * str;

    sub = (STB_SRT_SUB *) bink->subvalid;
    str = stb_srt_get_subtitle( sub, index, opt_out_start_ms, opt_out_end_ms );
    return str;
  }
  return 0;
}
