// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

// This file will make rrmutex and rrsemaphore from rrcondvar
// must define before including:

// the thread id type:

//#define MUTEX_TID_TYPE  pthread_t

// the thread id getter:

//#define MUTEX_TID_GET() pthread_self()

// the interal mutex structure with at least these fields:

//typedef struct rriMutex
//{
//    rriCondVar  cv;
//    U64         atomic_owner_and_flag; // this is the mutex gate ; atomic!
//    U32         count;  // recursion count - protected by this mutex
//    U32         type;
//} rriMutex;

// the interal semaphore structure with at least these fields:

//typedef struct rriSemaphore
//{
//    rriCondVar  cv;
//    U32         count;
//    U32         type;
//} rriSemaphore;

// you can wrap the functions names by overriding the DECLARE_NAME and/or 
//     DECLARE_FUNCTION macros:
#ifndef DECLARE_NAME
#define DECLARE_NAME( name ) name
#endif
#ifndef DECLARE_FUNCTION
#define DECLARE_FUNCTION( ret, name ) RADDEFFUNC ret RADLINK DECLARE_NAME(name)
#endif

// you can play up to 16-bits in the low half of the U32 type fields in both structs

// And, finally, you need to define a typedef and a function to create the times
//   for the timeout functions - these should return the timespan type and range
//   that your implementation of rriCondVarUnlockWaitUntilLock() requires:
// For example on linux:
//   typedef struct timespec WaitUntilTimeType;
//   static void GetWaitTime( WaitUntilTimeType * time, U32 ms );



/**

CB 09-12-2016

Mutex from CondVar

atomic fast path for un-contended case

the actual mutex gate variable is "owner"

the "count" is the recursion count and is not atomic; it is protected by this mutex

the cv is used for signalling

----------------

    rriCondVar  cv;
    U32         atomic_owner_and_flag; // this is the mutex gate ; atomic!
    U32         count;  // recursion count - protected by this mutex

atomic_owner_and_flag = owner thread ID (63 bits) + 1 bit WAITERS flag

-> atomic_owner_and_flag is the mutex for the user code

the condvar also internally has a pthread_mutex 

-> it does not protect any variables! but it is important in a subtle way

"count" is the recursive lock count on this thread

-> it is protected by this mutex (the "owner" variable)

Mutex Lock & Unlock must provide a memory barrier.  This is done with the atomic ops on atomic_owner_and_flag.
That is, atomic_owner_and_flag is the variable used for the mutex happens-before relationships.

----------------

The waiters flag is used to optimize the case of a single thread locking a mutex and then
unlocking it with no contention.  In that case we do an atomic CAS to Lock
and an atomic Exchange to Unlock, and no threading primitives are used. (eg. the condvar is not touched)

----------------

Two subtle moments :

1. The Unlock must do a Broadcast (not a Signal).  The purpose of this is just to reset the WAITERS flag in
a correct race-free way.

Thread A owns the mutex
Thread B & C are waiting on it.  WAITERS flag is set.
Thread A unlocks the mutex, setting owner to 0 (WAITERS flag cleared).
It must broadcast so that :
Thread B wakes up and grabs the mutex, owning it, WAITER flag unset.
Thread C wakes up and tries to lock the mutex, sets WAITER flag and goes to sleep.
Otherwise you could wind up with a thread waiting on the mutex, but waiters flag not set (deadlock)

Obviously this is not ideal for performance if you care about the case of many threads being parked on one mutex.

2. Missed wakeup / double-checked lock pattern

The WAITERS flag uses a kind of preparewait-wait pattern ; this takes some care.

The bad case goes like this :

A owns the mutex
B tries to get the mutex can't CAS to acquire it, sets WAITERS flag
A unlocks the mutex, sees & clears WAITERS flag, signals the CV
B goes into the wait on the CV
-> B is deadlocked because CV has already been signalled, won't be signalled again

The lockfree way of thinking about this is like this :

the mutex unlock (that sets owner to 0 and observes the WAITERS flag) is an "event"
You should only go into a wait if that event has not occurred between your observation of "owner" and the wait

That is,

preparewait <- registers event count
CAS to try to acquire "owner" again
wait <- only wait if event count hasn't changed

We only want to wait when this invariant is true : the mutex is owned by someone else and the WAITERS flag is set.
If we ever go into a wait when WAITERS is not set (so the signal may have already been done) that's bad bad.

The alternative way to do this (now used here) is just to use the CondVar Mutex to block that time scope.
I've chosen this because in the semantics of condvar, you have to take that lock anyway to signal it, so may as
well make use of it.

What's happening here is the CondVar mutex is not actually protecting any variables.  It is protecting certain
thread interleaving patterns from being possible.

preparewait <- CV mutex lock
CAS to try to acquire "owner" again
wait <- only wait if event count hasn't changed, it must not be because CV protects EC

Specificially, preparewait is replaced with CV lock.  I think of this as protected the "event count", even though the
event count does not actually exist as a variable.  Imagine that there was an "event count" variable.  When the mutex
unlocks and does its signal it increments event count.  

**/

#if defined(USE32BITATOMICS)
  #define AtomicExchange  rrAtomicExchange32
  #define AtomicCAS rrAtomicCAS32
  #define AtomicLoadAcquire rrAtomicLoadAcquire32
  #define ATYPE U32
  #define MUTEX_OWNER_WAITERS_FLAG    (0x80000000)
  #define MUTEX_OWNER_MASK            (0x7FFFFFFF)
#else
  #define AtomicExchange  rrAtomicExchange64
  #define AtomicCAS rrAtomicCAS64
  #define AtomicLoadAcquire rrAtomicLoadAcquire64
  #define ATYPE U64
  #if defined(_MSC_VER) && _MSC_VER <= 1200  // VC6
    #define MUTEX_OWNER_WAITERS_FLAG    (0x8000000000000000ui64)
    #define MUTEX_OWNER_MASK            (0x7FFFFFFFFFFFFFFFui64)
  #else
    #define MUTEX_OWNER_WAITERS_FLAG    (0x8000000000000000ull)
    #define MUTEX_OWNER_MASK            (0x7FFFFFFFFFFFFFFFull)
  #endif
#endif

#define TYPE_CONDVAR 0x30000

DECLARE_FUNCTION(rrbool, rrMutexCreate)( rrMutex * rr, S32 mtype )
{
    rriMutex * p;

    RR_COMPILER_ASSERT( ( sizeof ( rrMutex ) >= ( sizeof( rriMutex ) + sizeof(void*) + 15 ) ) );
    
    p = (rriMutex*) ( ( ( (UINTa) rr->data ) + 15 ) & ~15 );
    rr->i = p;

    RR_ASSERT( ( ((UINTa)rr->i) + sizeof(*(rr->i)) ) <= ( ((UINTa)rr) + sizeof(rr->data) ) );
    
    p->count = 0;
    p->atomic_owner_and_flag = 0;   
    p->type = 0;
    
    if ( ! rriCondVarCreate(&p->cv) )
        return 0;
    
    p->type = TYPE_CONDVAR;
    return 1;
}


DECLARE_FUNCTION(void,rrMutexDestroy)( rrMutex * rr )
{
    rriMutex * rri;

    RR_ASSERT( rr != 0 );
    rri = rr->i;
    RR_ASSERT( (rri->type&0xffff0000) == TYPE_CONDVAR );

    // ? could assert p->owner == 0

    rriCondVarDestroy(&rri->cv);
    rri->type = 0;
}

DECLARE_FUNCTION(void, rrMutexUnlock)( rrMutex * rr )
{
    rriMutex * rri;

    RR_ASSERT( rr != 0 );
    rri = rr->i;
    RR_ASSERT( (rri->type&0xffff0000) == TYPE_CONDVAR );

    // I own the mutex
    // so I can touch p->count without doing anything
    
    // owner thread must be me :
    RR_ASSERT( (rri->atomic_owner_and_flag&MUTEX_OWNER_MASK) == (UINTa)MUTEX_TID_GET() );
    RR_ASSERT( rri->count >= 1 );
    
    rri->count --;
    
    if ( rri->count == 0 )
    {
        // releasing it for reals

        // stuff a 0 in owner and get old value :
        ATYPE old_owner = AtomicExchange(&rri->atomic_owner_and_flag,0);
    
        RR_ASSERT( (old_owner&MUTEX_OWNER_MASK) == (UINTa)MUTEX_TID_GET() );

        if ( old_owner & MUTEX_OWNER_WAITERS_FLAG )
        {
            // need to signal
            // must be a broadcast so that extra waiters will re-set the flag
            rriCondVarLock(&rri->cv);
            rriCondVarBroadcastUnlock(&rri->cv);
        }
    }
}


static rrbool rriMutexSpinningLockTimeout( rriMutex * rri, ATYPE owner, ATYPE me, U32 ms )
{
    // 100 spins + 2 yields ?
    {
      #define spin_count  102
      int spins;
      for(spins=0;spins<spin_count;spins++)
      {
          // give up some CPU ?
          // would be nice to have increasing backoff here :
          rrThreadSpinHyperYield();
          if ( spins >= 100 )
          {
              RR_ASSERT( ms > 0 ); // don't do this yield for a trylock
              rrThreadSpinHyperYield();
          }
      
          if ( owner == 0 )
          {
              if ( AtomicCAS(&rri->atomic_owner_and_flag,&owner,me) )
              {
                  // I got it, I own the mutex now
                  RR_ASSERT( rri->count == 0 );
                  rri->count = 1;
                  return 1;
              }
              // owner was reloaded by CAS
          }
          else
          {
              // reload owner but don't bother trying to CAS
              owner = AtomicLoadAcquire(&rri->atomic_owner_and_flag);
          }
      }
      #undef spin_count
    }

    {
        WaitUntilTimeType wtime;
        int initedTime = 0;
        
        // first grab CV lock :
        // this lock doesn't actually protect any variables
        // what it does is prevent a missed wakeup
        // the unlocker can't fire a signal until I go into my Wait
        rriCondVarLock(&rri->cv);
                
        for(;;)
        {
            // owner cannot become me in this loop :
            RR_ASSERT( (owner&MUTEX_OWNER_MASK) != me );
        
            if ( owner == 0 )
            {
                if ( AtomicCAS(&rri->atomic_owner_and_flag,&owner,me) )
                {
                    // I got it, I own the mutex now
                    RR_ASSERT( rri->count == 0 );
                    rri->count = 1;
                    break;
                }
                // owner was reloaded, loop
            }
            else
            {
                // I can't aquire the mutex.  Wait.
                // this is a slow path and we don't really care
                
                // now attempt to CAS to turn on the waiters flag :
                // do the CAS even if owner already had waiters flag
                //  this ensures it didn't change between grabbing the cv lock and waiting
                
                if ( AtomicCAS(&rri->atomic_owner_and_flag,&owner,owner|MUTEX_OWNER_WAITERS_FLAG) )
                {
                    // owner flag is set, prepared to wait :
                
                    if ( ms == RR_WAIT_INFINITE )
                    {
                        rriCondVarUnlockWaitLock(&rri->cv);
                    }
                    else
                    {
                        if ( !initedTime)
                        {
                          initedTime = 1;
                          GetWaitTime( &wtime, ms );
                        }

                        if ( ! rriCondVarUnlockWaitUntilLock(&rri->cv,&wtime) )
                        {
                            // timed out
                            rriCondVarUnlock(&rri->cv);
                            return 0;
                        }
                    }
                    
                    // got a signal, retry
                    
                    // reload owner :
                    owner = AtomicLoadAcquire(&rri->atomic_owner_and_flag);
                }
                else
                {
                    // CAS failed, owner was reloaded
                }
            }
        }
    }
    
    rriCondVarUnlock(&rri->cv);               
    return 1;
}


DECLARE_FUNCTION(rrbool,rrMutexLockTimeout)( rrMutex * rr, U32 ms )
{
    rriMutex * rri;
    ATYPE me, owner;

    RR_ASSERT( rr != 0 );
    rri = rr->i;
    RR_ASSERT( (rri->type&0xffff0000) == TYPE_CONDVAR );

    me = (ATYPE)(UINTa) MUTEX_TID_GET();
    RR_ASSERT( me != 0 );
    RR_ASSERT( (me&MUTEX_OWNER_MASK) == me );

    // try the fast common uncontended case first
    owner = 0;
    if ( RAD_LIKELY( AtomicCAS(&rri->atomic_owner_and_flag,&owner,me) ) )
    {
      // I got it, I own the mutex now
      RR_ASSERT( rri->count == 0 );
      rri->count = 1;
      return 1;
    }
    // owner was reloaded by CAS

    // try the other simple case of I already own this
    if ( (owner&MUTEX_OWNER_MASK) == me )
    {
        // no race possible here; owner is me
        
        // recursive lock, I already had it
        RR_ASSERT( rri->count >= 1 );
        rri->count ++;
        return 1;
    }

    // if it's a trylock and we failed, we'redon
    if ( ms == 0 )
    {
        // just a TryLock , don't wait
        return 0;
    }

    return rriMutexSpinningLockTimeout( rri, owner, me, ms );
}

DECLARE_FUNCTION(void,rrMutexLock)( rrMutex * rr )
{
    rriMutex * rri;
    ATYPE me;
    ATYPE owner;

    RR_ASSERT( rr != 0 );
    rri = rr->i;
    RR_ASSERT( (rri->type&0xffff0000) == TYPE_CONDVAR );

    me = (ATYPE)(UINTa) MUTEX_TID_GET();
    RR_ASSERT( me != 0 );
    RR_ASSERT( (me&MUTEX_OWNER_MASK) == me );

    // try the fast common uncontended case first
    owner = 0;
    if ( RAD_LIKELY( AtomicCAS(&rri->atomic_owner_and_flag,&owner,me) ) )
    {
      // I got it, I own the mutex now
      RR_ASSERT( rri->count == 0 );
      rri->count = 1;
      return;
    }
    // owner was reloaded by CAS

    // try the other simple case of I already own this
    if ( (owner&MUTEX_OWNER_MASK) == me )
    {
        // no race possible here; owner is me
        
        // recursive lock, I already had it
        RR_ASSERT( rri->count >= 1 );
        rri->count ++;
        return;
    }

    rriMutexSpinningLockTimeout( rri, owner, me, RR_WAIT_INFINITE );
}

//=================================================================

/**

Totally straightforward Semaphore from CondVar

The "count" is protected by the cv mutex.

Decrement waits on count using the cv
Increment signals changes to count using the cv

@@ some of the radrtl thread functions try to be non-crashing NOPS on uninitialized structures or
after failed Creates.  I think that's terrible and should be removed.  (eg. get rid of this GOODSEMA nonsense)

**/

#define TYPE_SEMA_CONDVAR 0x20000

//=============================================================================

DECLARE_FUNCTION(rrbool, rrSemaphoreCreate)( rrSemaphore * rr, S32 initialCount, S32 maxCount )
{
  RR_COMPILER_ASSERT( ( sizeof ( rrSemaphore ) >= ( sizeof( rriSemaphore ) + sizeof(void*) + 15 ) ) );
  rriSemaphore *rri;

  rri = (rriSemaphore*) ( ( ( (UINTa) rr->data ) + 15 ) & ~15 );
  rr->i = rri;

  rri->type = 0;
  // @@ maxCount not supported
  rri->count = initialCount;
  if ( ! rriCondVarCreate(&rri->cv) )
      return 0;
  rri->type = TYPE_SEMA_CONDVAR;
  return 1;
}

DECLARE_FUNCTION(void,rrSemaphoreDestroy)( rrSemaphore * rr )
{
  rriSemaphore * rri;
  
  RR_ASSERT( rr );
  rri = rr->i;

  if ( (rri->type&0xffff0000) == TYPE_SEMA_CONDVAR ) 
  {
    //RR_ASSERT( sem->count == 0 ); //  ??
    rriCondVarDestroy(&rri->cv);
    rri->type = 0;
  }
}

DECLARE_FUNCTION(rrbool,rrSemaphoreDecrementOrWait)( rrSemaphore * rr, U32 ms )
{
  rriSemaphore * rri;
  
  RR_ASSERT( rr );
  rri = rr->i;

  RR_ASSERT( (rri->type&0xffff0000) == TYPE_SEMA_CONDVAR );
    
  if ( ms == RR_WAIT_INFINITE )
  {
    rriCondVarLock(&rri->cv);
    while ( rri->count == 0 )
    {
        rriCondVarUnlockWaitLock(&rri->cv);
    }
    rri->count--;
    rriCondVarUnlock(&rri->cv);
    return 1;
  }
  else if ( ms == 0 ) // trylock
  {
    rrbool ret = 0;
    rriCondVarLock(&rri->cv);
    if ( rri->count > 0 )
    {
        rri->count--;
        ret = 1;
    }
    rriCondVarUnlock(&rri->cv);
    return ret;
  }
  else
  {
    // timed wait :

    WaitUntilTimeType wtime;
    GetWaitTime( &wtime, ms );

    rriCondVarLock(&rri->cv);
    // "count" is protected by the cv mutex
    while ( rri->count == 0 )
    {
        if ( ! rriCondVarUnlockWaitUntilLock(&rri->cv,&wtime) )
        {
            // timed out

            // this should never succeed except in very rare races
            // @@ could remove this check here :            
            rrbool ret = 0;
            if ( rri->count > 0 )
            {
                rri->count--;
                ret = 1;
            }
    
            rriCondVarUnlock(&rri->cv);
            return ret;
        }
        // non-timeout wakeup; loop
    }
    rri->count--;
    rriCondVarUnlock(&rri->cv);
    
    return 1;
  }
}

DECLARE_FUNCTION(void,rrSemaphoreIncrement)( rrSemaphore * rr, S32 count )
{
  rriSemaphore * rri;
  
  RR_ASSERT( rr );
  rri = rr->i;

  RR_ASSERT( (rri->type&0xffff0000) == TYPE_SEMA_CONDVAR );
  RR_ASSERT( count >= 1 );
  if ( count <= 0 ) return;

  rriCondVarLock(&rri->cv);
  rri->count += count;
  if ( count == 1 )
      rriCondVarSignalUnlock(&rri->cv);
  else
      rriCondVarBroadcastUnlock(&rri->cv);
}

