// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#include "egttypes.h"

#define DECLARING_THREADS
#include "binkthread.h"

#include "rrAtomics.h"
#include "rrThreads2.h" 
#include "rrCpu.h" 
#include "popmal.h" 

#define BINK_ASYNC_PRI     rrThreadNormal

#if defined(__RADCONSOLE__)
  #define BINK_ASYNC_CORE(n) rrThreadCore(n)
#else
  #define BINK_ASYNC_CORE(n) 0
#endif

#define SEGMENT_NAME "asyncRR"
#include "binksegment.h"

#include "rrstring.h"

#define QUEUE_SIZE 256

#define WrapOffset(ofs) ( (ofs) & (QUEUE_SIZE-1) )
#define AmountFull(wp,rp) ( (wp)-(rp) )
#define AmountLeft(wp,rp) ( QUEUE_SIZE - AmountFull(wp,rp) )

typedef struct queue_data
{
  rrSemaphore added;
  rrSemaphore removed;  // this acts more like an event
  rrMutex block;
  U32 removed_count;
  U32 count;
  U32 read_ofs;
  U32 write_ofs;
  S32 init;
  char data[ QUEUE_SIZE ];
} queue_data;

typedef struct thread_data
{
  rrThread thread_p;
  char thread_name[16];

  queue_data to_client;
  queue_data to_host;
} thread_data;

static U8 loaded_on[RR_MAX_CPUS] = {0};  // 0=not init,1=init,2=closing

RRSTRIPPABLE thread_data * bink_threads[ RR_MAX_CPUS ]={0};

RRSTRIPPABLEPUB char * RAD_thread_error = 0;

static int create_queue( queue_data * q )
{
  q->init = 0;
  if ( !rrSemaphoreCreate( &q->added, 0, 0x7fffffff ) )
  {
    RAD_thread_error = "CreateSemaphore failed.";
    return 0;
  }

  if ( !rrSemaphoreCreate( &q->removed, 1, 0x7fffffff ) )
  {
    RAD_thread_error = "CreateSemaphore failed.";
   err:
    rrSemaphoreDestroy( &q->added );
    return 0;
  }

  if ( !rrMutexCreate( &q->block, rrMutexFullLocksOnly ) )
  {
    RAD_thread_error = "CreateMutex failed.";
    rrSemaphoreDestroy( &q->removed );
    goto err;
  }

  q->read_ofs = 0;
  q->write_ofs = 0;
  q->init = 1;
  q->count = 0;
  q->removed_count = 1;
  return 1;
}


static int destroy_queue( queue_data * q )
{
  if ( q->init )
  {
    rrSemaphoreDestroy( &q->added );
    rrSemaphoreDestroy( &q->removed );
    rrMutexDestroy( &q->block );
    q->init = 0;

    return 1;
  }
  return 0;
}

#include "binktm.h"

#if defined(_MSC_VER)
#pragma warning(disable: 4702)
#endif

static U32 RADLINK thread_proc( void * p )
{
  tmThreadName( tm_mask, 0, bink_threads[ (SINTa)p ]->thread_name );
  RAD_async_main( (int)(SINTa) p );
  return 1;
}

static int create_thread( thread_data * t, int thread_num )
{
  rrmemcpy( t->thread_name, "BinkAsy0", 9 );
  t->thread_name[ 7 ] = (char) ( thread_num + '0' );
  
  #if defined(RR_CAN_ALLOC_THREAD_STACK)
  if ( !rrThreadCreateWithStack( &t->thread_p, thread_proc, (void*)(SINTa)thread_num, 96*1024, (t+1),  // stack comes right after the thread_data
                                 BINK_ASYNC_PRI, BINK_ASYNC_CORE( thread_num ), t->thread_name ) )
  #else
  if ( !rrThreadCreate( &t->thread_p, thread_proc, (void*)(SINTa)thread_num, 96*1024, 
                       BINK_ASYNC_PRI, BINK_ASYNC_CORE( thread_num ), t->thread_name ) )
  #endif
  {
    RAD_thread_error = "CreateThread failed.";
    return 0;
  }

  return 1;
}

static unsigned int queue_empty( queue_data * q )
{
  return AmountLeft( q->write_ofs, q->read_ofs );
}

static int add_to_queue( queue_data * q, void * data, unsigned int length )
{
  U32 amt, len;
  U32 wo;

  rrMutexLock( &q->block );

  amt = AmountLeft( q->write_ofs, q->read_ofs );
  
  if ( amt < length )
  {
    RAD_thread_error = "Async queue full.";
    rrMutexUnlock( &q->block );
    return 0;
  }

  len = length;
  wo = WrapOffset( q->write_ofs );
  amt = QUEUE_SIZE - wo;

  if ( amt <= len )
  {
    rrmemcpy( q->data + wo, data, amt );
    wo = 0;
    data = ( (char*) data ) + amt;
    len -= amt;
  }

  rrmemcpy( q->data + wo, data, len );
  q->write_ofs += length;

  rrSemaphoreIncrement( &q->added, 1 );
  rrAtomicAddExchange32( &q->count, 1 );

  rrMutexUnlock( &q->block );

  return 1;
}


static int receive_from_queue( queue_data * q, int us, void * out_data, unsigned int length )
{
  U32 amt, len;
  U32 ro;

  if ( q->init == 0 )
  {
    RAD_thread_error = "Broken async queue.";
    return 0;
  }

  if ( !rrSemaphoreDecrementOrWait( &q->added, ( us < 0 ) ? RR_WAIT_INFINITE : ( ( (U32) us + 999 ) / 1000 ) ) )
    return 0;

  rrMutexLock( &q->block );
  rrAtomicAddExchange32( &q->count, -1 );
  
  amt = AmountFull( q->write_ofs, q->read_ofs );

  if ( amt < length )  // read as much as we can
    length = amt;

  len = length;
  ro = WrapOffset( q->read_ofs );
  amt = QUEUE_SIZE - ro;

  if ( amt <= len )
  {
    rrmemcpy( out_data, q->data + ro, amt );
    ro = 0;
    out_data = ( (char*) out_data ) + amt;
    len -= amt;
  }

  rrmemcpy( out_data, q->data + ro, len );
  q->read_ofs += length;

  if ( rrAtomicLoadAcquire32( &q->removed_count ) == 0 )
  {
    rrAtomicAddExchange32( &q->removed_count, 1 );
    rrSemaphoreIncrement( &q->removed, 1 );
  }

  rrMutexUnlock( &q->block );

  return length;
}

static void wait_for_a_queue_remove( queue_data * q, S32 wait )
{
  rrSemaphoreDecrementOrWait( &q->removed, wait );
  rrAtomicAddExchange32( &q->removed_count, -1 );
}

static int blip_other_queue( queue_data * q )
{
  if ( q->init == 0 )
  {
    RAD_thread_error = "Broken async queue.";
    return 0;
  }
  if ( rrAtomicLoadAcquire32( &q->count ) < 16 )
  {
    rrAtomicAddExchange32( &q->count, 1 );
    rrSemaphoreIncrement( &q->added, 1 );
  }
  return 1;
}

#if defined(__RADCONSOLE__)
static void reset_affinities()
{
  int i;
  U64 ld, b;

  ld = 0;
  b = 1;
  for( i = 0 ; i < RR_MAX_CPUS ; i++ )
  {
    if ( loaded_on[i] )
      ld |= b;
    b <<= 1;
  }

  for( i = 0 ; i < RR_MAX_CPUS ; i++ )
    if ( loaded_on[i] )
      rrThreadSetCores( &bink_threads[i]->thread_p, ld );
}
#endif

static int wait_thread_exit( thread_data * t )
{
  rrThreadDestroy( &t->thread_p );
  return 1;
}

int RAD_start_thread( int thread_num )
{
  RAD_thread_error = 0;

  if ( thread_num >= RR_MAX_CPUS )
  {
    RAD_thread_error = "Out of range thread number.";
    return 0;
  }
  
  if ( loaded_on[ thread_num ] )
  {
    RAD_thread_error = "Already loaded on this thread number.";
    return 0;
  }

  bink_threads[thread_num] = popmalloc( 0, 
    sizeof( *bink_threads[0] )
    #if defined(RR_CAN_ALLOC_THREAD_STACK)
      + 96 * 1024
    #endif
    );

  if ( bink_threads[thread_num] == 0 )
    return 0;

  if ( !create_queue( &bink_threads[thread_num]->to_client ) )
  {
   err0: 
    popfree( bink_threads[thread_num] );
    bink_threads[thread_num] = 0;
    return 0;
  }
  
  if ( !create_queue( &bink_threads[thread_num]->to_host ) )
  {
   err1:
    destroy_queue( &bink_threads[thread_num]->to_client );
    goto err0;
  }

  if ( !create_thread( bink_threads[thread_num], thread_num ) )  
  {
    destroy_queue( &bink_threads[thread_num]->to_host );
    goto err1;
  }

  loaded_on[ thread_num ] = 1;
  
  #if defined(__RADCONSOLE__)
  // when you create new threads, make sure all decode threads
  //    are spread over all of the specified cores
  reset_affinities();
  #endif

  return 1;
}

static int check_thread_num( int thread_num )
{
  if ( (U32) thread_num >= (U32) RR_MAX_CPUS )
  {
    RAD_thread_error = "Out of range thread number.";
    return 0;
  }

  if ( loaded_on[ thread_num ] == 0 )
  {
    RAD_thread_error = "Thread number not started.";
    return 0;
  }

  return 1;
}  

int RAD_platform_info( int thread_num, void * output )
{
  if ( check_thread_num( thread_num ) == 0 )
    return 0;
  
  rrThreadGetOSThreadID( &bink_threads[ thread_num ]->thread_p, output );
  return 1;
}

static int iRAD_send( queue_data * queue, int thread_num, void * data, U32 length )
{
  RAD_thread_error = 0;

  if ( check_thread_num( thread_num ) == 0 )
    return 0;

  if ( !add_to_queue( queue, data, length ) )
    return 0;
    
  return 1;
}

unsigned int RAD_get_client_empty( int thread_num )
{
  return queue_empty( &bink_threads[thread_num]->to_client );
}

int RAD_send_to_client( int thread_num, void * data, U32 length )
{
  return iRAD_send( &bink_threads[thread_num]->to_client, thread_num, data, length );
}

int RAD_send_to_host( int thread_num, void * data, U32 length )
{
  return iRAD_send( &bink_threads[thread_num]->to_host, thread_num, data, length );
}

void RAD_wait_for_host_room( int thread_num )
{
  wait_for_a_queue_remove( &bink_threads[thread_num]->to_host, RR_WAIT_INFINITE );
}

static int iRAD_receive( queue_data * queue, int thread_num, int us, void * out_data, U32 length )
{
  RAD_thread_error = 0;

  if ( loaded_on[ thread_num ] != 2 )
    if ( check_thread_num( thread_num ) == 0 )
      return 0;

  if ( ! receive_from_queue( queue, us, out_data, length ) )
    return 0;

  return 1;
}

int RAD_receive_at_client( int thread_num, int us, void * out_data, U32 length )
{
  return iRAD_receive( &bink_threads[thread_num]->to_client, thread_num, us, out_data, length );
}

int RAD_receive_at_host( int thread_num, int us, void * out_data, U32 length )
{
  return iRAD_receive( &bink_threads[thread_num]->to_host, thread_num, us, out_data, length );
}

static int iRAD_blip_queue( struct queue_data * queue, int thread_num )
{
  if ( check_thread_num( thread_num ) == 0 )
    return 0;

  if ( ! blip_other_queue( queue ) )
    return 0;
    
  return 1;
}

int RAD_blip_for_host( int thread_num )
{
  return iRAD_blip_queue( &bink_threads[thread_num]->to_host, thread_num );
}

int RAD_stop_thread( int thread_num )
{
  void * data = 0;
  
  RAD_thread_error = 0;

  if ( check_thread_num( thread_num ) == 0 )
    return 0;

  // send a zero command down
  if ( !add_to_queue( &bink_threads[thread_num]->to_client, &data, sizeof( data ) ) )
    return 0;

  loaded_on[ thread_num ] = 2;

  return 1;
}


int RAD_wait_stop_thread( int thread_num )
{
  RAD_thread_error = 0;
  
  if ( (U32) thread_num >= (U32) RR_MAX_CPUS )
  {
    RAD_thread_error = "Out of range thread number.";
    return 0;
  }

  if ( loaded_on[ thread_num ] != 2 )
  {
    RAD_thread_error = "Wait not queued for this thread.";
    return 0;
  }

  if ( ! wait_thread_exit( bink_threads[thread_num] ) )
    return 0;

  destroy_queue( &bink_threads[thread_num]->to_client );
  destroy_queue( &bink_threads[thread_num]->to_host );

  popfree( bink_threads[thread_num] );
  bink_threads[thread_num] = 0;

  loaded_on[ thread_num ] = 0;

  return 1;
}

