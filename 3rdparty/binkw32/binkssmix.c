// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#include "binkproj.h"

#include "string.h"

#include <math.h>

#include "radtimer.h"

#ifndef __RADMEMH__
#include "radmem.h"
#endif

#include "rrstring.h"

#include "binkcomm.h"

#include "binkssmix.h"

// Sound channel descriptor and access macro
typedef struct SNDCHANNEL        // Kept in bs->snddata array, must not exceed 128 bytes in size!
{
  F32 vol;
  F32 pan;
  F32 vols[ 8*2 ];
  S32 lastl, lastr;
  U32 adv_fract;
  U32 last_fract, prev_last_fract;
  S32 status;
  S32 paused;    // 1 if paused, 0 if playing
  HBINK bink;
  struct BINKSND * prev;
  struct BINKSND * next;
} SNDCHANNEL;

#define SC( v ) ( (SNDCHANNEL *)(v)->snddata )

static struct BINKSND * list = 0;

static audio_funcs * dev = 0;
static U32 opened = 0;
static U32 playing = 0;
static U32 last_time = 0;
static int frequency;
static int number_of_channels;
static int latency_ms;
static int poll_ms;
static S32 * mix_buf;
static U32 buf_size;

static void to_s16()
{
  S32 * RADRESTRICT mix = mix_buf;
  S32 const * mix_end = (S32*) ( ((char*)mix_buf) + buf_size );
  S16 * RADRESTRICT out = (S16*) mix_buf;
  while( mix < mix_end )
  {
    S32 s;

    s = *mix++ >> 10;
    if ( s > 32767 ) s = 32767;
    if ( s < -32767 ) s = -32767;
    *out++ = (S16)s;
  }
}

//#include <android/log.h>
//#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "radstdout", __VA_ARGS__))
//#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "radstdout", __VA_ARGS__))
#define LOGI(...)

static void do_a_mix( struct BINKSND * bs )
{
  SNDCHANNEL * BS = SC( bs );

  if ( ( !BS->paused ) && ( bs->OnOff ) )
  {
    S32 * RADRESTRICT mix = mix_buf;
    S32 const * mix_end = (S32*) ( ((char*)mix_buf) + buf_size );
    U8 * RADRESTRICT i = bs->sndreadpos;
    U8 * RADRESTRICT e = bs->sndend;
    U32 prev_fract = BS->prev_last_fract;
    U32 fract = BS->last_fract;
    S32 lastsl = BS->lastl;
    U32 adv_fract = BS->adv_fract;

LOGI("doing a mix: %p (%d) [%p %p %d] [%p %p %d]\n",bs,number_of_channels,i,e,(S32)(e-i),mix,mix_end,(S32)(mix_end-mix) );

    if ( number_of_channels == 2 )
    {
      S32 l,r;

      {
        F32 t;
        t = BS->vol * BS->vols[ 0 ];
        if ( BS->pan > 0.5f )
          t *= 2.0f - (  BS->pan * 2.0f );
        t *= 1024.0f;
        l = (S32)t;

        t = BS->vol * BS->vols[ ( bs->chans == 1 ) ? 1 : 2 ];
        if ( BS->pan < 0.5f )
          t *= ( BS->pan * 2.0f );
        t *= 1024.0f;
        r = (S32)t;
      }

      if ( bs->chans == 1 )
      {
        S32 s = ( (S16*) i )[0] - (lastsl>>16);
        while( mix < mix_end )
        {
          U32 whole;
          S32 a;

          a = ( (S32) ( lastsl + ( s * prev_fract ) ) ) >> 16;
          
          mix[ 0 ] += ( a * l );
          mix[ 1 ] += ( a * r );
          
          mix += 2;
          prev_fract = fract;
          fract += adv_fract;
          whole = fract >> 16;
          if ( whole )
          {
            lastsl = s + ( lastsl >> 16 );
            fract &= 0xffff;
            i += whole + whole;
            if ( i >= e ) i = bs->sndbuf + ( i - e );
            s = ((S32)( (S16*) i )[0]) - lastsl;
            lastsl <<= 16;
          }
        }
      }
      else
      {
        S32 lastsr = BS->lastr;
        S32 sl = ( (S16*) i )[0] - (lastsl>>16);
        S32 sr = ( (S16*) i )[1] - (lastsr>>16);
        while( mix < mix_end )
        {
          U32 whole;
          S32 al, ar;

          al = ( (S32) ( lastsl + ( sl * prev_fract ) ) ) >> 16;
          ar = ( (S32) ( lastsr + ( sr * prev_fract ) ) ) >> 16;
          
          mix[ 0 ] += ( al * l );
          mix[ 1 ] += ( ar * r );
          mix += 2;
          
          fract += adv_fract;
          whole = fract >> 16;
          if ( whole )
          {
            lastsl = sl + ( lastsl >> 16 );
            lastsr = sr + ( lastsr >> 16 );
            fract &= 0xffff;
            i += (whole*4);
            if ( i >= e ) i = bs->sndbuf + ( i - e );
            sl = ((S32)( (S16*) i )[0]) - lastsl;
            sr = ((S32)( (S16*) i )[1]) - lastsr;
            lastsl <<= 16;
            lastsr <<= 16;
          }
        }
        BS->lastr = lastsr;
      }
    }
    else if ( number_of_channels == 6 )
    {
      if ( bs->chans == 1 )
      {
        S32 fl,fr,c,su,rl,rr;
        S32 s = ( (S16*) i )[0] - (lastsl>>16);

        {
          F32 t;
          t = BS->vol * BS->vols[ 0 ];
          if ( BS->pan > 0.5f )
            t *= 2.0f - (  BS->pan * 2.0f );
          t *= 1024.0f;
          fl = (S32)t;

          t = BS->vol * BS->vols[ 1 ];
          if ( BS->pan < 0.5f )
            t *= ( BS->pan * 2.0f );
          t *= 1024.0f;
          fr = (S32)t;

          t = BS->vol * BS->vols[ 2 ];
          t *= 1024.0f;
          c = (S32)t;

          t = BS->vol * BS->vols[ 3 ];
          t *= 1024.0f;
          su = (S32)t;

          t = BS->vol * BS->vols[ 4 ];
          if ( BS->pan > 0.5f )
            t *= 2.0f - (  BS->pan * 2.0f );
          t *= 1024.0f;
          rl = (S32)t;

          t = BS->vol * BS->vols[ 5 ];
          if ( BS->pan < 0.5f )
            t *= ( BS->pan * 2.0f );
          t *= 1024.0f;
          rr = (S32)t;
        }

        while( mix < mix_end )
        {
          U32 whole;
          S32 a;

          a = ( (S32) ( lastsl + ( s * prev_fract ) ) ) >> 16;
          
          mix[ 0 ] += ( a * fl );
          mix[ 1 ] += ( a * fr );
          mix[ 2 ] += ( a * c );
          mix[ 3 ] += ( a * su );
          mix[ 4 ] += ( a * rl );
          mix[ 5 ] += ( a * rr );
          
          mix += 6;
          prev_fract = fract;
          fract += adv_fract;
          whole = fract >> 16;
          if ( whole )
          {
            lastsl = s + ( lastsl >> 16 );
            fract &= 0xffff;
            i += whole + whole;
            if ( i >= e ) i = bs->sndbuf + ( i - e );
            s = ((S32)( (S16*) i )[0]) - lastsl;
            lastsl <<= 16;
          }
        }
      }
      else
      {
        S32 fl,fr,cl,cr,sul,sur,rl,rr;
        S32 lastsr = BS->lastr;
        S32 sl = ( (S16*) i )[0] - (lastsl>>16);
        S32 sr = ( (S16*) i )[1] - (lastsr>>16);

        {
          F32 t;
          t = BS->vol * BS->vols[ 0 ];
          if ( BS->pan > 0.5f )
            t *= 2.0f - (  BS->pan * 2.0f );
          t *= 1024.0f;
          fl = (S32)t;

          t = BS->vol * BS->vols[ 3 ];
          if ( BS->pan < 0.5f )
            t *= ( BS->pan * 2.0f );
          t *= 1024.0f;
          fr = (S32)t;

          t = BS->vol * BS->vols[ 4 ];
          t *= 1024.0f;
          cl = (S32)t;

          t = BS->vol * BS->vols[ 5 ];
          t *= 1024.0f;
          cr = (S32)t;

          t = BS->vol * BS->vols[ 6 ];
          t *= 1024.0f;
          sul = (S32)t;

          t = BS->vol * BS->vols[ 7 ];
          t *= 1024.0f;
          sur = (S32)t;

          t = BS->vol * BS->vols[ 8 ];
          if ( BS->pan > 0.5f )
            t *= 2.0f - (  BS->pan * 2.0f );
          t *= 1024.0f;
          rl = (S32)t;

          t = BS->vol * BS->vols[ 11 ];
          if ( BS->pan < 0.5f )
            t *= ( BS->pan * 2.0f );
          t *= 1024.0f;
          rr = (S32)t;
        }

        while( mix < mix_end )
        {
          U32 whole;
          S32 al, ar;

          al = ( (S32) ( lastsl + ( sl * prev_fract ) ) ) >> 16;
          ar = ( (S32) ( lastsr + ( sr * prev_fract ) ) ) >> 16;
          
          mix[ 0 ] += ( al * fl );
          mix[ 1 ] += ( ar * fr );
          mix[ 2 ] += ( al * cl ) + ( ar * cr );
          mix[ 3 ] += ( al * sul ) + ( ar * sur );
          mix[ 4 ] += ( al * rl );
          mix[ 5 ] += ( ar * rr );
          mix += 6;
          
          fract += adv_fract;
          whole = fract >> 16;
          if ( whole )
          {
            lastsl = sl + ( lastsl >> 16 );
            lastsr = sr + ( lastsr >> 16 );
            fract &= 0xffff;
            i += (whole*4);
            if ( i >= e ) i = bs->sndbuf + ( i - e );
            sl = ((S32)( (S16*) i )[0]) - lastsl;
            sr = ((S32)( (S16*) i )[1]) - lastsr;
            lastsl <<= 16;
            lastsr <<= 16;
          }
        }
        BS->lastr = lastsr;
      }
    }
    else if ( number_of_channels == 8 )
    {
      if ( bs->chans == 1 )
      {
        S32 fl,fr,c,su,rl,rr,il,ir;
        S32 s = ( (S16*) i )[0] - (lastsl>>16);

        {
          F32 t;
          t = BS->vol * BS->vols[ 0 ];
          if ( BS->pan > 0.5f )
            t *= 2.0f - (  BS->pan * 2.0f );
          t *= 1024.0f;
          fl = (S32)t;

          t = BS->vol * BS->vols[ 1 ];
          if ( BS->pan < 0.5f )
            t *= ( BS->pan * 2.0f );
          t *= 1024.0f;
          fr = (S32)t;

          t = BS->vol * BS->vols[ 2 ];
          t *= 1024.0f;
          c = (S32)t;

          t = BS->vol * BS->vols[ 3 ];
          t *= 1024.0f;
          su = (S32)t;

          t = BS->vol * BS->vols[ 4 ];
          if ( BS->pan > 0.5f )
            t *= 2.0f - (  BS->pan * 2.0f );
          t *= 1024.0f;
          rl = (S32)t;

          t = BS->vol * BS->vols[ 5 ];
          if ( BS->pan < 0.5f )
            t *= ( BS->pan * 2.0f );
          t *= 1024.0f;
          rr = (S32)t;

          t = BS->vol * BS->vols[ 6 ];
          if ( BS->pan > 0.5f )
            t *= 2.0f - (  BS->pan * 2.0f );
          t *= 1024.0f;
          il = (S32)t;

          t = BS->vol * BS->vols[ 7 ];
          if ( BS->pan < 0.5f )
            t *= ( BS->pan * 2.0f );
          t *= 1024.0f;
          ir = (S32)t;
        }

        while( mix < mix_end )
        {
          U32 whole;
          S32 a;

          a = ( (S32) ( lastsl + ( s * prev_fract ) ) ) >> 16;
          
          mix[ 0 ] += ( a * fl );
          mix[ 1 ] += ( a * fr );
          mix[ 2 ] += ( a * c );
          mix[ 3 ] += ( a * su );
          mix[ 4 ] += ( a * rl );
          mix[ 5 ] += ( a * rr );
          mix[ 6 ] += ( a * il );
          mix[ 7 ] += ( a * ir );
          
          mix += 8;
          prev_fract = fract;
          fract += adv_fract;
          whole = fract >> 16;
          if ( whole )
          {
            lastsl = s + ( lastsl >> 16 );
            fract &= 0xffff;
            i += whole + whole;
            if ( i >= e ) i = bs->sndbuf + ( i - e );
            s = ((S32)( (S16*) i )[0]) - lastsl;
            lastsl <<= 16;
          }
        }
      }
      else
      {
        S32 fl,fr,cl,cr,sul,sur,rl,rr,il,ir;
        S32 lastsr = BS->lastr;
        S32 sl = ( (S16*) i )[0] - (lastsl>>16);
        S32 sr = ( (S16*) i )[1] - (lastsr>>16);

        {
          F32 t;
          t = BS->vol * BS->vols[ 0 ];
          if ( BS->pan > 0.5f )
            t *= 2.0f - (  BS->pan * 2.0f );
          t *= 1024.0f;
          fl = (S32)t;

          t = BS->vol * BS->vols[ 3 ];
          if ( BS->pan < 0.5f )
            t *= ( BS->pan * 2.0f );
          t *= 1024.0f;
          fr = (S32)t;

          t = BS->vol * BS->vols[ 4 ];
          t *= 1024.0f;
          cl = (S32)t;

          t = BS->vol * BS->vols[ 5 ];
          t *= 1024.0f;
          cr = (S32)t;

          t = BS->vol * BS->vols[ 6 ];
          t *= 1024.0f;
          sul = (S32)t;

          t = BS->vol * BS->vols[ 7 ];
          t *= 1024.0f;
          sur = (S32)t;

          t = BS->vol * BS->vols[ 8 ];
          if ( BS->pan > 0.5f )
            t *= 2.0f - (  BS->pan * 2.0f );
          t *= 1024.0f;
          rl = (S32)t;

          t = BS->vol * BS->vols[ 11 ];
          if ( BS->pan < 0.5f )
            t *= ( BS->pan * 2.0f );
          t *= 1024.0f;
          rr = (S32)t;

          t = BS->vol * BS->vols[ 12 ];
          if ( BS->pan > 0.5f )
            t *= 2.0f - (  BS->pan * 2.0f );
          t *= 1024.0f;
          il = (S32)t;

          t = BS->vol * BS->vols[ 15 ];
          if ( BS->pan < 0.5f )
            t *= ( BS->pan * 2.0f );
          t *= 1024.0f;
          ir = (S32)t;
        }

        while( mix < mix_end )
        {
          U32 whole;
          S32 al, ar;

          al = ( (S32) ( lastsl + ( sl * prev_fract ) ) ) >> 16;
          ar = ( (S32) ( lastsr + ( sr * prev_fract ) ) ) >> 16;
          
          mix[ 0 ] += ( al * fl );
          mix[ 1 ] += ( ar * fr );
          mix[ 2 ] += ( al * cl ) + ( ar * cr );
          mix[ 3 ] += ( al * sul ) + ( ar * sur );
          mix[ 4 ] += ( al * rl );
          mix[ 5 ] += ( ar * rr );
          mix[ 6 ] += ( al * il );
          mix[ 7 ] += ( ar * ir );
          mix += 8;
          
          fract += adv_fract;
          whole = fract >> 16;
          if ( whole )
          {
            lastsl = sl + ( lastsl >> 16 );
            lastsr = sr + ( lastsr >> 16 );
            fract &= 0xffff;
            i += (whole*4);
            if ( i >= e ) i = bs->sndbuf + ( i - e );
            sl = ((S32)( (S16*) i )[0]) - lastsl;
            sr = ((S32)( (S16*) i )[1]) - lastsr;
            lastsl <<= 16;
            lastsr <<= 16;
          }
        }
        BS->lastr = lastsr;
      }
    }
 
    BS->prev_last_fract = prev_fract;
    BS->lastl = lastsl;
    BS->last_fract = fract;
    bs->sndreadpos = i;
   }
}

// since this code copies directly out of the bink tracks, we don't use
//   ready, or lock (everything gets sucked out in the callback made through "unlock")
static S32  RADLINK Unlock (struct BINKSND * z,U32 filled)  // parameters ignored
{
  if ( dev->ready() )
  {
    struct BINKSND * bs;
    int mix=0;
    U32 t = RADTimerRead();
   
    // walk the list
    bs = list;
    while ( bs )
    {
      SNDCHANNEL * BS =  SC( bs );
      S32 sndamt;
      
      sndamt = (S32) ( bs->sndwritepos - bs->sndreadpos );
      if ( sndamt < 0 )
        sndamt += bs->sndbufsize;
    
      if ( sndamt > bs->BestSize )
      {
        if ( playing == 0 )
        {
          last_time = RADTimerRead();         
          playing = 1;
        }

        if ( mix == 0)
          rrmemsetzerobig( mix_buf, buf_size );
        mix = 1;
        do_a_mix( bs );
      }
      else
      {
        if ( ( t - last_time ) > 50 ) // if we are 50 ms late, assume we skipped
          if ( ( !BS->paused ) && ( bs->OnOff ) )
              bs->SoundDroppedOut=1;
      }

      bs = BS->next;
    }
    
    if ( mix )
    {
      to_s16();
      if ( !dev->write( mix_buf, buf_size / 2 ) )
        playing = 0;
      last_time = RADTimerRead();
    }
  }

  return( 0 );
}

// set the volume 0=quiet, 32767=max, 65536=double volume (not always supported)
static void RADLINK Volume ( struct BINKSND * bs, S32 volume )
{
  SNDCHANNEL * BS = SC( bs );

  if ( volume >= 65536 )
    volume = 65535;

  BS->vol = powf( (F32)volume / 32767.0f, 10.0f / 6.0f );
}

// set the pan 0=left, 32767=middle, 65536=right
static void RADLINK Pan( struct BINKSND * bs, S32 pan )
{
  SNDCHANNEL * BS = SC( bs );

  if ( pan > 65536 )
    pan = 65536;

  BS->pan = (F32) pan / 65536.0f;
}


// set the spk vol values - volumes are LRLRLR... for stereo and just VVVV... for mono
//    currently, total will always be 8
static void RADLINK SpeakerVolumes( struct BINKSND * bs, F32 * volumes, U32 total )
{
  SNDCHANNEL * BS = SC( bs );

  if ( total > 8 )
    total = 8;

  rrmemsetzero( BS->vols, 4 * 8 * 2 );
  rrmemcpy( BS->vols, volumes, 4 * total * 2 );
}


// called to turn the sound on or off
static S32  RADLINK SetOnOff( struct BINKSND * bs, S32 status )
{
  SNDCHANNEL * BS = SC( bs );

  if ( status == 1 )
  {
    if ( ( BS->prev == 0 ) && ( BS->next == 0 ) && ( opened == 0 ) )
    {
      int outsize;
      if ( !dev->open( frequency, number_of_channels, &outsize ) )
      {
        bs->OnOff = 0;
        return 0; // still off
      }
      outsize *= 2;
      if ( outsize != buf_size )
      {
        radfree( mix_buf );
        buf_size = outsize;
        mix_buf = radmalloc( buf_size );
        if ( mix_buf == 0 )
        {
          dev->close();
          bs->OnOff = 0;
          return 0; // still off
        }
      }
      opened = 1;
    }
  }
/*
  else
  {
    always leave hardware running open since reopening audio on some platforms is problematic
    close by calling BinkFreeGlobals()
    if ( ( BS->prev == 0 ) && ( BS->next == 0 ) )
    {
      if ( opened )
      {
        dev->close();
        opened = 0;
        playing = 0;
      }
    }
  }
  */

  bs->OnOff = status;

  return( bs->OnOff );
}


// pauses/resumes the playback
static S32  RADLINK Pause( struct BINKSND * bs, S32 status )  // 1=pause, 0=resume, -1=resume and clear
{
  SNDCHANNEL * BS = SC( bs );

  BS->paused = status;
  return( BS->paused );
}


// closes this channel

static void RADLINK Close( struct BINKSND * bs )
{
  SNDCHANNEL * BS = (SNDCHANNEL *) bs->snddata;

  bs->SetOnOff( bs, 0 );

  if ( list == bs )
  {
    list = BS->next;
  }

  if ( BS->prev )
  {
    SC(BS->prev)->next = BS->next;
  }

  if ( BS->next )
  {
    SC(BS->next)->prev = BS->prev;
  }

  /*
  Don't close by default, do it in BinkFreeGlobals
  if ( list == 0 )
  {
    if ( opened )
    {
      dev->close();
      opened = 0;
    }
    if ( mix_buf )
    {
      radfree( mix_buf );
      mix_buf = 0;
    }
  }
  */
}

extern BinkFreeGlobalsCallback * BinkFreeGlobals1;

static void RADLINK BinkSSMixGlobalClose()
{
  if ( opened )
  {
    dev->close();
    opened = 0;
  }
  if ( mix_buf )
  {
    radfree( mix_buf );
    mix_buf = 0;
  }

  // reset it
  BinkFreeGlobals1 = 0;
}


static S32 RADLINK Open( struct BINKSND * bs, U32 freq, S32 chans, U32 flags, HBINK bink )
{
  SNDCHANNEL * BS = SC( bs );

  rrmemsetzero( bs, sizeof( *bs ) );

  bs->SoundDroppedOut = 0;
  bs->ThreadServiceType = -poll_ms; // negative numbers means Bink will call Unlock once per mix (at that 5 ms deltas)

  bs->freq  = freq;
  bs->chans = chans;

  BS->bink = bink;
  BS->vol =  1.0f;
  BS->pan =  0.5f;

  BS->vols[0] = 1.0f;
  BS->vols[1] = 1.0f;
  if ( number_of_channels >= 6 )
  {
    BS->vols[2] = 1.0f;
    BS->vols[3] = 1.0f;
    BS->vols[4] = 1.0f;
    BS->vols[5] = 1.0f;
    if ( number_of_channels >= 8 )
    {
      BS->vols[6] = 1.0f;
      BS->vols[7] = 1.0f;
    }
  }
 

  // Set initial status
  BS->paused    = 0;
  bs->OnOff     = 1;

  // Set up the function calls
  bs->Unlock   = Unlock;
  bs->Volume   = Volume;
  bs->Pan      = Pan;
  bs->Pause    = Pause;
  bs->SetOnOff = SetOnOff;
  bs->Close    = Close;
  bs->SpeakerVols  = SpeakerVolumes;

  bs->Latency = latency_ms;

  // fill in the mixing info structure
  BS->adv_fract = (U32) ( ( (U64) 65536 * (U64) bs->freq ) / (U64) frequency );
  
  BS->prev = 0;
  BS->next = list;

  if ( opened == 0 )
  {
    int size;
    if ( !dev->open( frequency, number_of_channels, &size ) )
      return( 0 );
    // our buf_size is in S32's 
    buf_size = size*2;
    mix_buf = radmalloc( buf_size );
    if ( mix_buf == 0 )
    {
      dev->close();
      return 0;
    }
    opened = 1;

    // setup callback to free audio at BinkFree
    BinkFreeGlobals1 = BinkSSMixGlobalClose;
  }

  if ( list == 0 )
  {
    list = bs;
  }
  else
  {
    SC(list)->prev = bs;
    list = bs;
  }

  bs->BestSize = (U32) ( ( (U64)buf_size * (U64)chans * (U64) bs->freq ) / ( (U64)number_of_channels * (U64) frequency ) );
  
  return( 1 );
}

RADEXPFUNC BINKSNDOPEN RADEXPLINK BinkOpenSSMix( void * low_level_func_ptrs, U32 freq, U32 chans )
{
  if ( list == 0 )
  {
    if ( ( freq > 48000 ) || ( freq <= 500 ) )
      freq = 48000;

    if ( chans > 6 )
      chans = 8;
    else if ( chans > 2 )
      chans = 6;
    else
      chans = 2;

    dev = (audio_funcs*) low_level_func_ptrs;
    frequency = freq;
    number_of_channels = chans;

    // TODO - handle channel changes for platforms that don't re-allocate the sound hardware (ps4/p45)
    
    if ( dev->check( &frequency, &number_of_channels, &latency_ms, &poll_ms ) )
      return( &Open );          
  }

  return 0;
}
