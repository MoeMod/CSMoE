// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#include "binkproj.h"

#include "binkcomm.h"
#include "rrCpu.h"

#define SEGMENT_NAME "asyncthread"
#include "binksegment.h"

S32 LowBinkDoFrameAsync( HBINK bink, U32 work_todo );

#include "binktm.h"
#include "binkthread.h"

extern char * RAD_thread_error;

RADEXPFUNC S32 RADEXPLINK BinkStartAsyncThread( S32 thread_num, void const * param )
{
  return RAD_start_thread( thread_num );
}

RADEXPFUNC S32 RADEXPLINK BinkRequestStopAsyncThread( S32 thread_num )
{
  return RAD_stop_thread( thread_num );
}

RADEXPFUNC S32 RADEXPLINK BinkRequestStopAsyncThreadsMulti( U32* threads, S32 thread_count )
{
  S32 i, ret;
  ret = 0;
  for ( i = 0 ; i < thread_count ; i++ )
    ret |= RAD_stop_thread( threads[i] );
  return ret;
}

RADEXPFUNC S32 RADEXPLINK BinkWaitStopAsyncThread( S32 thread_num )
{
  return RAD_wait_stop_thread( thread_num );
}

RADEXPFUNC S32 RADEXPLINK BinkWaitStopAsyncThreadsMulti( U32* threads, S32 thread_count )
{
  S32 i, ret;
  ret = 0;
  for ( i = 0 ; i < thread_count ; i++ )
    ret |= RAD_wait_stop_thread( threads[i] );
  return ret;
}

void RAD_async_main( int thread_num )
{
  for(;;)
  {
    UINTa buf[ 2 ];  
    S32 r;
    
    tmEnter( tm_mask, TMZF_IDLE, "Waiting for async work" );
    r = RAD_receive_at_client( thread_num, -1, buf, sizeof( buf[ 0 ] ) );
    tmLeave( tm_mask );

    if ( r )
    {
      if ( buf[ 0 ] == 0 )
        return;

      buf[ 1 ] = LowBinkDoFrameAsync( (HBINK) ( buf[ 0 ] & ~255 ), (U32) ( buf[ 0 ] & 255 ) );

      r = 0;
      while ( !RAD_send_to_host( thread_num, buf, sizeof( buf ) ) )
      {
        RAD_wait_for_host_room( thread_num );
      }
    }
  }
}

extern HBINK start_do_frame( HBINK bink, U32 timer );
extern S32 end_do_frame( HBINK bink, U32 timer );
RRSTRIPPABLEPUB void (*bink_async_wait)(HBINK bink) = 0;
#ifdef RADUSETM3
  #define bpopmalloc( p,b,y ) bpopmalloci( p,b,y, __FILE__, __LINE__ )
  extern void* bpopmalloci(void * pmbuf, HBINK bink,U32 amt, char const * info, U32 line);
#else
  extern void* bpopmalloc( void * base, HBINK bink, U32 bytes );
#endif

static void close_wait( HBINK bink );

#define BINKRUNNING 256
#define BINKNOTDONE 512

// return 1 if got one
static int read_done_binks( int thread, S32 wait, HBINK current_bink )
{
  UINTa incoming[ 2 ];
  int got_one = 0;

  while ( RAD_receive_at_host( thread, wait, incoming, sizeof( incoming ) ) )
  {
    HBINK completed_bink;

    completed_bink = (HBINK) ( incoming[ 0 ] & ~255 );

    if ( completed_bink != current_bink )
      RAD_blip_for_host( completed_bink->async_in_progress[ incoming[ 0 ] & 15 ] & ~(BINKRUNNING | BINKNOTDONE) );

    completed_bink->async_in_progress[ incoming[ 0 ] & 15 ] &= ~BINKRUNNING;

    got_one = 1;
    wait = 0;
  }

  return got_one;
}


RADEXPFUNC S32  RADEXPLINK BinkDoFrameAsyncMulti(HBINK bink, U32* ithreads, S32 thread_count )
{
  S32 i;
  U32 started = 0;
  U32 threads[ RR_MAX_CPUS ];
  U32 empty[ RR_MAX_CPUS ];
  U32 * t;

  if ( bink == 0 )
    return 0;
  
  bink = start_do_frame( bink, RADTimerRead() );
  if ( bink == 0 )
    return( 0 );

  {
    U32 all;
    all=0;
    for( i = 0 ; i < 8; i++)
      all|=bink->async_in_progress[ i ];
    if ( all )
    {
      BinkSetError( "There is already an async decompression in progress on this HBINK." );
      return 0;
    }
  }

  // copy thread array and see how empty each queue is
  for( i = 0 ; i < thread_count ; i++ )
  {
    threads[i] = ithreads[i];
    empty[i] = RAD_get_client_empty( threads[i] );
  }

  // now sort so that we assign to the emptiest thread first
  for( i = 1 ; i < thread_count ; i++ )
  {
    S32 j;
    U32 th, e; 
    
    th = threads[i];
    e = empty[i];
    j = i - 1;
    for(;;)
    {
      if ( empty[j] < e )
      {
        threads[j+1] = threads[j];
        empty[j+1] = empty[j];
      } 
      else
      {
        threads[j+1] = th;
        empty[j+1] = e;
        break;
      }
      --j;
      if ( j < 0 )
      {
        threads[0] = th;
        empty[0] = e;
        break;
      }
    }
  }

  bink_async_wait = close_wait;
  
  t = threads;
  for( i = 0 ; i < thread_count ; i++ )
  {
    U32 r;
    UINTa buf;

    r = get_slice_range( &bink->slices, 1, thread_count );
  
    if ( r == 0 )
      goto done;

    if ( bink->async_in_progress[ r&15 ] )
      goto done;

    buf = ( (UINTa) bink ) | r;

    for(;;)
    {
      if ( RAD_send_to_client( *t, &buf, sizeof( buf ) ) )
      {
        bink->async_in_progress[ r&15 ] = *t | BINKRUNNING | BINKNOTDONE;
        started |= bink->async_in_progress[ r&15 ];
        break;
      }
      else
      {
        read_done_binks( *t, -1, bink );
      }
    }

    if ( RAD_thread_error )
      BinkSetError( RAD_thread_error );

    ++t;
  }  

  // if we started something
 done: 
  if ( started )
    return( 1 );

  return( 0 );
}


RADEXPFUNC S32 RADEXPLINK BinkDoFrameAsync(HBINK bink, U32 thread_one, U32 thread_two )
{
  if ( thread_one == thread_two )
  {
    return BinkDoFrameAsyncMulti( bink, &thread_one, 1 );
  }
  else
  {
    U32 threads[2];
    threads[0] = thread_one;
    threads[1] = thread_two;

    return BinkDoFrameAsyncMulti( bink, threads, 2 );
  }
}


RADEXPFUNC S32 RADEXPLINK BinkDoFrameAsyncWait( HBINK bink, S32 us )
{
  U32 i, st, all;

  if ( bink == 0 )
    return 1;
  
  all=0;
  for( i = 0 ; i < 8; i++)
    all|=bink->async_in_progress[ i ];
  if ( all == 0 )
    return 1;

  st = 0;
  if ( us > 0 )
    st = RADTimerRead();

  for(;;)
  {
    for ( i = 0 ; i < 8 ; i++ )
    {
      if ( bink->async_in_progress[ i ] & BINKRUNNING )
      {
        read_done_binks( bink->async_in_progress[ i ] & ~(BINKRUNNING | BINKNOTDONE), us, bink );
  
        if ( RAD_thread_error )
          BinkSetError( RAD_thread_error );
      }
    }

    all=0;
    for( i = 0 ; i < 8; i++)
      all|=bink->async_in_progress[ i ];

    if ( all & BINKRUNNING )
    {
      if ( us == 0 )
        return 0;
        
      if ( us > 0 )
      {
        if ( ( RADTimerRead() - st ) >= (U32) (us/1000) )
          return 0;
      }
    }
    else
    {
      break;
    }
  }

  // clear
  for( i = 0 ; i < 8; i++)
    bink->async_in_progress[ i ] = 0;

  end_do_frame( bink, RADTimerRead() );

  return( 1 );
}

static void close_wait( HBINK bink )
{
  BinkDoFrameAsyncWait( bink, -1 );
}
