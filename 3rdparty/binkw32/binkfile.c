// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#include "egttypes.h"
#include "rrCore.h"
#include "bink.h"

#define BINK_USE_TELEMETRY  // comment out, if you don't have Telemetry

extern BINKFILEOPEN  binkosopen;  // returns 1 on success
extern BINKFILEREAD  binkosread;
extern BINKFILESEEK  binkosseek;
extern BINKFILECLOSE binkosclose;

// defined variables that are accessed from the Bink IO structure
typedef struct BINKFILE
{
  U64 StartFile;
  U64 FileSize;
  U64 FileBufPos;
  U64 ReadErrorPos;
  UINTa FileUserData[2];
  U64 volatile BufPos;
  U64 volatile FileIOPos;
  U64 volatile BufEmpty;
  U8* volatile BufBack;
  U8* Buffer;
  U8* BufEnd;
  S32 volatile HaveSeekedBack;
  U32 DontClose;
} BINKFILE;

#define BASEREADSIZE ( 128 * 1024 )
#define READALIGNMENT 4096

#ifdef BINK_USE_TELEMETRY
#include "binktm.h"
#endif

// functions that callback into the Bink library for service stuff
#define SUSPEND_CB( bio ) { if ( bio->suspend_callback ) bio->suspend_callback( bio );}
#define RESUME_CB( bio ) { if ( bio->resume_callback ) bio->resume_callback( bio );}
#define TRY_SUSPEND_CB( bio ) ( ( bio->try_suspend_callback == 0 ) ? 0 : bio->try_suspend_callback( bio ) )
#define IDLE_ON_CB( bio ) { if ( bio->idle_on_callback ) bio->idle_on_callback( bio );}
#define SIMULATE_CB( bio, amt, tmr ) { if ( bio->simulate_callback ) bio->simulate_callback( bio, amt, tmr );}
#define TIMER_CB( ) ( ( bio->timer_callback ) ? bio->timer_callback() : 0 )
#define FLIPENDIAN_CB( ptr, amt ) { if ( bio->flipendian_callback ) bio->flipendian_callback( ptr, amt );}
#define LOCKEDADD_CB( ptr, amt ) { if ( bio->lockedadd_callback ) bio->lockedadd_callback( ptr, amt ); else *(ptr)+=(amt); }
#define MEMCPY_CB( d,s, amt ) { if ( bio->memcpy_callback ) bio->memcpy_callback( d,s,amt ); }

#ifdef BINK_USE_TELEMETRY
static U64 tmspanopen = 0;
static U64 tmspanIOopen = 0;
#endif

static U64 perform_io(BINKIO * bio, S32 foreground);

//reads from the header
static U64 RADLINK BinkFileReadHeader( BINKIO * bio, S64 offset, void* dest, U64 size )
{
  U64 amt, temp;
  BINKFILE * BF = (BINKFILE*) bio->iodata;

  SUSPEND_CB( bio );

  if ( offset != -1 )
  {
    if ( BF->FileIOPos != (U64) offset )
    {
      binkosseek( BF->FileUserData, offset + BF->StartFile );
      BF->FileIOPos = offset;
    }
  }

  #ifdef BINK_USE_TELEMETRY
    tmEnter( tm_mask, TMZF_STALL, "%s %s", "Bink IO", "read header" );
  #endif
  
  amt = binkosread( BF->FileUserData, dest, size );

  #ifdef BINK_USE_TELEMETRY
      tmLeave( tm_mask );
  #endif

  if ( amt != size )
    bio->ReadError=1;

  BF->FileIOPos += amt;
  BF->FileBufPos = BF->FileIOPos;

  FLIPENDIAN_CB( dest, amt );
  
  // get the end of the file from the header
  if ( ( offset == 0 ) && ( amt >= 8 ) )
    BF->FileSize = ((U32*)dest)[1] + 8;

  temp = ( BF->FileSize - BF->FileBufPos );
  bio->CurBufSize = ( temp < bio->BufSize ) ? temp : bio->BufSize;

  RESUME_CB( bio );

  return( amt );
}

// reads a frame (BinkIdle might be running from another thread)

//   This function operates in essentially two modes:
//     1) Normal streaming mode, where as soon as we consume
//        data from Bink, that buffer range is free to be reused.
//        Basically, we only get any caching by seeking forward
//        in the stream into a position that is in our buffer.
//     2) Or, when the IO buffer is as large as the file, we just
//        keep adding to the buffer until the entire bink is
//        completely cached. Once cached, you can hop around 
//        anywhere.  If you jump forward, before the data is
//        cached, we just restart, caching from that point,
//        and if you loop around, then we restart the cache
//        again, and then the movie reads in normally (and
//        will then be eventually fully cached).
static U64  RADLINK BinkFileReadFrame( BINKIO * bio, 
                                       U32 framenum, 
                                       S64 offset,
                                       void* dest,
                                       U64 size )
{
  S32 suspended = 0;
  U64 amt, tamt = 0;
  U64 cpy;
  U32 skip_past_bytes = 0;
  void* odest = dest;
  BINKFILE * BF = (BINKFILE*) bio->iodata;
  
  if ( bio->ReadError )
    return( 0 );

  #ifdef BINK_USE_TELEMETRY
    if ( tmRunning() )
    {
      if ( framenum <= 1 )
      {
        if ( tmspanopen )
        {
          tmEndTimeSpan( tm_mask, tmspanopen, 0, "Start of file IO" );
          tmspanopen = 0;
        }
        if ( tmspanIOopen )
        {
          tmEndTimeSpan( tm_mask, tmspanIOopen, 0, "Start of file IO" );
          tmspanIOopen = 0;
        }
        tmspanIOopen = (((U64)((UINTa)'BINK'))<<32)^(U64)(UINTa)bio;
        tmBeginTimeSpan( tm_mask, tmspanIOopen, 0, "Bink IO active" );
      }
      tmEnter( tm_mask, TMZF_NONE, "%s %s", "Bink IO", "read frame" );
    }
  #endif

  if ( offset != -1 )
  {
    // are we big enough to fit in memory?
    if ( bio->BufSize >= BF->FileSize )
    {
      // when the file fits into the buffer, read pos is filebufpos+bufpos
      if ( ( BF->FileBufPos + BF->BufPos ) != (U64) offset )
      {
        if ( ( (U64) offset >= BF->FileBufPos ) && ( (U64) offset <= BF->FileIOPos ) )
        {
          amt = offset-BF->FileBufPos;
          BF->BufPos = amt;
          bio->CurBufSize = BF->FileSize - BF->BufPos;
          bio->CurBufUsed = BF->FileIOPos - BF->FileBufPos - BF->BufPos;
        }
        else
        {
          skip_past_bytes = ( (U32)offset ) & ( BASEREADSIZE - 1 );
          offset -= skip_past_bytes;
          // after adjusting, do we still need to seek? (small gotos ahead without clearing)
          // otherwise, we just reset everything (the common case)
          if ( ( BF->FileBufPos + BF->BufPos ) != (U64) offset )
          {
            suspended = 1;
            #ifdef BINK_USE_TELEMETRY
              tmEnter( tm_mask, TMZF_STALL, "%s %s", "Bink IO", "seek to new position" );
            #endif
            SUSPEND_CB( bio );
            goto reset;
          }
        }
      }
    }
    else
    {
      // when streaming, filebufpos is always the read pos
      if ( BF->FileBufPos != (U64) offset )
      {
        if ( bio->ReadError )
          return( 0 );

        suspended = 1;
        #ifdef BINK_USE_TELEMETRY
          tmEnter( tm_mask, TMZF_STALL, "%s %s", "Bink IO", "seek to new position" );
        #endif
        SUSPEND_CB( bio );

        if ( ( (U64) offset > BF->FileBufPos ) && ( (U64) offset <= BF->FileIOPos ) )
        {
          amt = offset-BF->FileBufPos;
          BF->FileBufPos = offset;
          BF->BufEmpty += amt;
          bio->CurBufUsed -= amt;
          BF->BufPos += amt;
          if ( BF->BufPos >= bio->BufSize )
            BF->BufPos -= bio->BufSize;
        }
        else
        {
         reset: 
          // jumped out of buffer, reset everything
          skip_past_bytes = ((U32)offset) & ( BASEREADSIZE - 1 );
          offset -= skip_past_bytes;
          binkosseek( BF->FileUserData, offset + BF->StartFile );
          BF->FileIOPos = offset;
          BF->FileBufPos = offset;

          BF->BufEmpty = bio->BufSize;
          bio->CurBufUsed = 0;
          BF->BufPos = 0;
          BF->BufBack = BF->Buffer;
        }

        #ifdef BINK_USE_TELEMETRY
          tmLeave( tm_mask );
        #endif
      }
    }
  }

  // copy from background buffer
 getrest:

  cpy = bio->CurBufUsed;
  if ( ( skip_past_bytes ) && ( cpy ) )
  {
    U64 adj = cpy;
    if ( adj > skip_past_bytes )
      adj = skip_past_bytes;
    skip_past_bytes = 0;
    LOCKEDADD_CB( &BF->BufPos, adj );
    LOCKEDADD_CB( &bio->CurBufUsed, -(S64)adj );
    cpy = bio->CurBufUsed;
    if ( bio->BufSize < BF->FileSize )
    {
      BF->FileBufPos += adj;
      LOCKEDADD_CB( &BF->BufEmpty, adj );
    }
  }

  if ( cpy )
  {
    if ( cpy > size )
      cpy = size;

    size -= cpy;
    tamt += cpy;

    if ( bio->BufSize >= BF->FileSize )
    {
      // whole file fits in buffer, never wraps
      MEMCPY_CB( dest, BF->Buffer + BF->BufPos, cpy );
      dest = ( (U8*) dest ) + cpy;
      LOCKEDADD_CB( &BF->BufPos, cpy );
      LOCKEDADD_CB( &bio->CurBufUsed, -(S64)cpy );

      bio->CurBufSize = BF->FileSize - BF->BufPos;
    }
    else
    {
      U64 front;

      BF->FileBufPos += cpy;

      front = bio->BufSize - BF->BufPos;
      if ( front <= cpy )
      {
        MEMCPY_CB( dest, BF->Buffer + BF->BufPos, front );
        dest = ( (U8*) dest ) + front;
        BF->BufPos = 0;
        cpy -= front;

        LOCKEDADD_CB( &bio->CurBufUsed, -(S64)front );
        LOCKEDADD_CB( &BF->BufEmpty, front );

        if ( cpy == 0 )
          goto skipwrap;
      }
      
      MEMCPY_CB( dest, BF->Buffer + BF->BufPos, cpy );
      dest = ( (U8*) dest ) + cpy;

      LOCKEDADD_CB( &BF->BufPos, cpy );
      LOCKEDADD_CB( &bio->CurBufUsed, -(S64)cpy );
      LOCKEDADD_CB( &BF->BufEmpty, cpy );

      bio->CurBufSize = ( BF->FileSize >= BF->FileBufPos ) ? (BF->FileSize - BF->FileBufPos) : 0;
      if ( bio->CurBufSize > bio->BufSize )
        bio->CurBufSize = bio->BufSize;
      else if ( bio->CurBufSize < bio->CurBufUsed )
        bio->CurBufSize = bio->CurBufUsed;

     skipwrap:;
  
     if ( BF->FileBufPos >= BF->ReadErrorPos )
       bio->ReadError = 1;
    }
  }

  if ( ( size ) || ( skip_past_bytes ) )
  {
    if ( !bio->ReadError )
    {
      perform_io( bio, 1 );
      goto getrest;
    }
  }

  if ( suspended )
    RESUME_CB( bio );

  FLIPENDIAN_CB( odest, tamt );

  #ifdef BINK_USE_TELEMETRY
    tmLeave( tm_mask );
  #endif

  return( tamt );
}


//returns the size of the recommended buffer
static U64 RADLINK BinkFileGetBufferSize( BINKIO * bio, U64 size )
{
  size = READALIGNMENT + ( ( size + ( BASEREADSIZE - 1 ) ) / BASEREADSIZE ) * BASEREADSIZE;
  if ( size < ( BASEREADSIZE * 2 + READALIGNMENT ) )
    size = BASEREADSIZE * 2 + READALIGNMENT;
  return( size );
}


//sets the address and size of the background buffer
static void RADLINK BinkFileSetInfo( BINKIO * bio,
                                     void * buff,
                                     U64 size,
                                     U64 filesize,
                                     U32 simulate )
{
  BINKFILE * BF = (BINKFILE*) bio->iodata;
  U8 * buf;
  S64 size_align;

  SUSPEND_CB( bio );

  buf = (void*) ( ( ( (UINTa) buff ) + (READALIGNMENT-1) ) & ~(READALIGNMENT-1) );
  size_align = size - ( (U8*) buf - (U8*) buff );
  size_align = ( size_align / BASEREADSIZE ) * BASEREADSIZE;
  if ( size_align <= 0 )
    buf = buff;
  else
    size = size_align;
  BF->Buffer = (U8*) buf;
  BF->BufPos = 0;
  BF->BufBack = (U8*) buf;
  BF->BufEnd =( (U8*) buf ) + size;
  bio->BufSize = size;
  BF->BufEmpty = size;
  bio->CurBufUsed = 0;
  bio->CurBufSize = size;
  BF->FileSize = filesize;

  RESUME_CB( bio );
}


//close the io structure
static void RADLINK BinkFileClose( BINKIO * bio )
{
  BINKFILE * BF = (BINKFILE*) bio->iodata;

  SUSPEND_CB( bio );

  #ifdef BINK_USE_TELEMETRY
  if ( tmspanIOopen )
  {
    tmEndTimeSpan( tm_mask, tmspanIOopen, 0, "Bink IO closed" );
    tmspanIOopen = 0;
  }
  #endif

  if ( BF->DontClose == 0 )
    binkosclose( BF->FileUserData );

  RESUME_CB( bio );
}


static U64 perform_io(BINKIO * bio, S32 foreground)
{
  BINKFILE * BF = (BINKFILE*) bio->iodata;
  U64 amt = 0;
  U64 filesizeleft;
  U32 timer;
  S32 Working = bio->Working;

  if ( BF->ReadErrorPos != 0xffffffffULL )
    return( 0 );

  if (( bio->Suspended ) && (!foreground))
    return( 0 );

  if ( TRY_SUSPEND_CB( bio ) )
  {
    filesizeleft = ( BF->FileSize - BF->FileIOPos );

    // if we are out of data to read, and we are in loop mode, seek backwards with a dummy read
    if ( ( filesizeleft == 0 ) && ( !BF->HaveSeekedBack ) && ( bio->bink->OpenFlags & BINKWILLLOOP ) )
    {
      char dummy_buf[ 96 ]; // padded for wii/nds

      BF->HaveSeekedBack = 1;
      bio->bink->OpenFlags &= ~BINKWILLLOOP;

      bio->DoingARead = 1;

      #ifdef BINK_USE_TELEMETRY
        tmspanopen = (((U64)((UINTa)'BNKL'))<<32)^(U64)(UINTa)bio;
        tmBeginTimeSpan( tm_mask, tmspanopen, 0, "Bink seek back to start window" );

        tmEnter( tm_mask, TMZF_NONE, "%s %s", "Bink IO", "seek to file start" );
      #endif

      binkosseek( BF->FileUserData, ( bio->bink->frameoffsets[ 0 ] & ~1 ) - 32 + BF->StartFile );
      amt = binkosread( BF->FileUserData, dummy_buf, 32 );
      if ( amt != 32 )
        bio->ReadError = 1;

      #ifdef BINK_USE_TELEMETRY
        tmLeave( tm_mask );
      #endif

      bio->DoingARead = 0;

      amt = 0;
    }

    // do a background IO, if we have the room
    if ( ( ( BF->BufEmpty >= BASEREADSIZE ) || ( BF->BufEmpty >= filesizeleft ) ) && ( filesizeleft ) )
    {
      U64 toreadamt;
      U64 align;

      toreadamt = BASEREADSIZE;
      if ( filesizeleft < toreadamt )
        toreadamt = filesizeleft;

      timer = TIMER_CB();

      // this alignment only handled the initial alignment
      //   at the front of the file (where we don't want
      //   to seek back to get the header).
      align = BF->FileIOPos & ( BASEREADSIZE - 1 );
      if ( ( align > toreadamt ) || ( toreadamt == filesizeleft ) )
        align = 0;

      // this assert makes sure we are only aligning on frame 1
      rrassert( ( align == 0 ) || ( BF->FileIOPos == ( bio->bink->frameoffsets[ 0 ] & ~1 ) ) ); 
  
      rrassert( ( align == 0 ) || ( ( bio->CurBufUsed == 0 ) && ( BF->FileIOPos == BF->FileBufPos ) && ( (BF->Buffer+BF->BufPos)==BF->BufBack ) ) );

      if ( align )
      {
        if ( bio->BufSize >= BF->FileSize )
          BF->FileBufPos -= align;
        LOCKEDADD_CB( &BF->BufPos,align );
      }

      toreadamt = toreadamt - align;

      if ( BF->HaveSeekedBack )
      {
        bio->bink->OpenFlags &= ~BINKWILLLOOP;
        BF->HaveSeekedBack = 0;
      }

      #ifdef BINK_USE_TELEMETRY
        tmEnter( tm_mask, TMZF_NONE, "%s %s", "Bink IO", ( ( Working ) || ( bio->Working ) )?"doing background read":"doing foreground read" );
      #endif

      bio->DoingARead = 1;
      amt = binkosread( BF->FileUserData, (void*) ( BF->BufBack + align ), toreadamt );
      bio->DoingARead = 0;

      // sleeps for the simulation time (only necessary for debugging)
      SIMULATE_CB( bio, amt, timer );
      
      if ( amt != toreadamt )
      {
        U64 errpos = BF->FileBufPos+bio->CurBufUsed+amt;
        if ( errpos < BF->ReadErrorPos )
          BF->ReadErrorPos = errpos;
      }

      if ( amt )
      {
        bio->BytesRead += amt;
        BF->FileIOPos += amt;
        BF->BufBack += BASEREADSIZE;  // the background buffer ptr *ALWAYS* advances by a full base amount 
                                      //   because we will only read less than a buffer on the first read
                                      //   (when we need to align), and the last read (where we won't look
                                      //   at the extra data).  this pointer thereby always stays aligned
                                      
        if ( BF->BufBack >= BF->BufEnd )
          BF->BufBack = BF->Buffer;

        LOCKEDADD_CB( &BF->BufEmpty, -(S32) amt );
        LOCKEDADD_CB( &bio->CurBufUsed, amt );

        if ( bio->CurBufUsed > bio->BufHighUsed )
          bio->BufHighUsed = bio->CurBufUsed;

        timer = TIMER_CB() - timer;
        bio->TotalTime += timer;
        if ( ( Working ) || ( bio->Working ) )
          bio->ThreadTime += timer;
        else
          bio->IdleTime += timer;
      }
    
      #ifdef BINK_USE_TELEMETRY
        tmLeave( tm_mask );
      #endif
    }
    else
    {
      // if we can't fill anymore, then set the max size to the current size
      bio->CurBufSize = bio->CurBufUsed;
    }

    RESUME_CB( bio );
  }
  else
  {
    // if we're in idle in the background thread, do a sleep to give it more time
    IDLE_ON_CB( bio ); // let the callback run
    amt = (U64)(S64)-1;
  }

  return( amt );
}

//tells the io system that idle time is occurring (can be called from another thread)
static U64 RADLINK BinkFileIdle(BINKIO * bio)
{
  return perform_io(bio,0);
}

//close the io structure
static S32 RADLINK BinkFileBGControl( BINKIO * bio, U32 control )
{
  if ( control & BINKBGIOSUSPEND )
  {
    #ifdef BINK_USE_TELEMETRY
    if ( tmspanIOopen )
    {
      tmEndTimeSpan( tm_mask, tmspanIOopen, 0, "Bink IO suspended" );
      tmspanIOopen = 0;
    }
    tmspanIOopen = (((U64)((UINTa)'BINK'))<<32)^(U64)(UINTa)bio;
    tmBeginTimeSpan( tm_mask, tmspanIOopen, 0, "Bink IO suspended" );
    #endif

    if ( bio->Suspended == 0 )
    {
      bio->Suspended = 1;
    }
    if ( control & BINKBGIOWAIT )
    {
      SUSPEND_CB( bio );
      RESUME_CB( bio );
    }
  }
  else if ( control & BINKBGIORESUME )
  {
    if ( bio->Suspended == 1 )
    {
      bio->Suspended = 0;
    }

    #ifdef BINK_USE_TELEMETRY
    if ( tmspanIOopen )
    {
      tmEndTimeSpan( tm_mask, tmspanIOopen, 0, "Bink IO resumed" );
      tmspanIOopen = 0;
    }
    tmspanIOopen = (((U64)((UINTa)'BINK'))<<32)^(U64)(UINTa)bio;
    tmBeginTimeSpan( tm_mask, tmspanIOopen, 0, "Bink IO resumed" );
    #endif

    if ( control & BINKBGIOWAIT )
    {
      BinkFileIdle( bio );
    }
  }

  if ( control & BINKBGIOTRYWAIT )
  {
    if ( TRY_SUSPEND_CB( bio ) )
    {
      RESUME_CB( bio );
      return 1;
    }
    return 0;
  }

  return( bio->Suspended );
}

//opens a normal filename into an io structure
RADDEFFUNC S32 RADLINK BinkFileOpen( BINKIO * bio, const char * name, U32 flags );
RADDEFFUNC S32 RADLINK BinkFileOpen( BINKIO * bio, const char * name, U32 flags )
{
  BINKFILE * BF = (BINKFILE*) bio->iodata;

  if ( flags & BINKFILEHANDLE )
  {
    BF->FileUserData[0] = (UINTa) name;
    BF->FileUserData[1] = 0;
    BF->DontClose = 1;
  }
  else
  {
    if ( !binkosopen( BF->FileUserData, name, 0 ) )
      return( 0 );
  }

  if ( flags & BINKFILEOFFSET )
  {
    BF->StartFile = ( (HBINK) ( ( ( char * ) bio ) - ( (UINTa)&((HBINK)0)->bio ) ) )->FileOffset;
    if ( BF->StartFile )
      binkosseek( BF->FileUserData, BF->StartFile );
  }

  BF->ReadErrorPos = 0xffffffffULL;
  bio->ReadHeader = BinkFileReadHeader;
  bio->ReadFrame = BinkFileReadFrame;
  bio->GetBufferSize = BinkFileGetBufferSize;
  bio->SetInfo = BinkFileSetInfo;
  bio->Idle = BinkFileIdle;
  bio->Close = BinkFileClose;
  bio->BGControl = BinkFileBGControl;

  return( 1 );
}

