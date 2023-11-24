// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#include "binkproj.h"

#if defined( USE_XA27 )
  #include <windows.h>
  #include <xaudio27.h>
#elif defined( USE_XA28 )
  #include <windows.h>
  #include <xaudio28.h>
  #define OUR_XAUDIO2  "xaudio2_8.dll"
#elif defined( USE_XA29 )
  #include <windows.h>
  #include <xaudio29.h>
  #define OUR_XAUDIO2 "xaudio2_9redist.dll"
  #define OUR_OTHER_XAUDIO2 "xaudio2_9.dll"
#else
  #include <xaudio2.h>
#endif

#define SEGMENT_NAME "xa2"
#include "binksegment.h"
#include "binkcomm.h"

#ifndef __RADMEMH__
#include "radmem.h"
#endif

#include "rrstring.h"

#ifndef __RADMATHH__
#include "radmath.h"
#endif


static IXAudio2 * XA2 = 0;
static IXAudio2MasteringVoice * Master = 0;
static int we_alloced = 0;

#define CHUNK_COUNT 16
#define TOTAL_BUFFER_MS 1000  // must be at least 800

//
// Sound channel descriptor and access macro
//

typedef struct SNDCHANNELXA      // Kept in bs->snddata array, must not exceed 128 bytes in size!
{
  HBINK bink;
  S32 vol;
  S32 pan;
  S32 status;
  S32 paused;    // 1 if paused, 0 if playing
  S32 fill_frag;
  S32 last_frag;
  U32 submitted;
  U32 preloadsize;
  U32 maxpreload;
  U32 bufsize;
  U32 mbins;
  IXAudio2SourceVoice * SourceVoice;
  U8* SoundBuffer;
  S32 has_mixspks;
  F32 vols[ 8 * 2 ];
} SNDCHANNELXA;

#ifdef __RAD64__
#define BS ( ( SNDCHANNELXA * )  ( ( ( (UINTa) & ( bs->snddata[ 0 ] ) ) + 7 ) & ~7 ) )
#else
#define BS ( ( SNDCHANNELXA * )    ( & ( bs->snddata[ 0 ] ) ) )
#endif

#define BS_PRELOADING 0
#define BS_STREAMING  1

static void XASoundPlay( struct BINKSND * bs, S32 index, U32 len )
{
  if ( BS->SourceVoice )
  {
    XAUDIO2_BUFFER buf = {0};

    buf.AudioBytes = len;
    buf.pAudioData = (BYTE*) ( BS->SoundBuffer + index * BS->bufsize );
    BS->SourceVoice->SubmitSourceBuffer( &buf );
  }
}

static int XASoundInit( struct BINKSND * bs )
{
  U32 rate = ( bs->freq * 16 * bs->chans ) / 8;
  WAVEFORMATEX fmt;

  if ( XA2 == 0 )
    return( 0 );

  BS->bufsize = ( ( ( rate * TOTAL_BUFFER_MS ) / ( 1000 * CHUNK_COUNT ) ) + 31 ) & ~31;
  BS->preloadsize = ( ( ( rate * TOTAL_BUFFER_MS ) / ( 1000 * 2 ) ) + 31 ) & ~31;
  if ( BS->preloadsize > BS->maxpreload)
    BS->preloadsize = BS->maxpreload;

  BS->status = BS_PRELOADING;
  BS->submitted = 0;

  BS->last_frag = -1;
  BS->fill_frag = -1;

  // create sound buffers and memory
  if ( BS->SoundBuffer == 0 )
  {
    BS->SoundBuffer = (U8*) radmalloc( BS->bufsize * CHUNK_COUNT );
    if ( BS->SoundBuffer == 0 )
      return( 0 );
  }

  rrmemset( BS->SoundBuffer, 0, BS->bufsize * CHUNK_COUNT );

  fmt.wFormatTag = WAVE_FORMAT_PCM;
  fmt.nChannels = (U8) bs->chans;
  fmt.wBitsPerSample = (U16) 16;
  fmt.nSamplesPerSec = bs->freq;
  fmt.nAvgBytesPerSec = bs->freq * ( (U32) ( fmt.wBitsPerSample >> 3 ) ) * ( (U32) fmt.nChannels );
  fmt.nBlockAlign = (U16) ( ( (U32) ( fmt.wBitsPerSample >> 3 ) ) * ( (U32) fmt.nChannels ) );
  fmt.cbSize = 0;

  BS->SourceVoice = 0;  
  if ( XA2->CreateSourceVoice( &BS->SourceVoice, &fmt, XAUDIO2_VOICE_NOPITCH, 1.0f, 0, 0, 0 ) != S_OK )
  {
    radfree( BS->SoundBuffer );
    return( 0 );
  }
  return( 1 );
}


static void XASoundPause( struct BINKSND * bs )
{
  if ( BS->SourceVoice )
    BS->SourceVoice->Stop( 0, XAUDIO2_COMMIT_NOW );
}


static void XASoundResume( struct BINKSND * bs )
{
  if ( BS->SourceVoice )
    BS->SourceVoice->Start( 0, XAUDIO2_COMMIT_NOW );
}


static void XASoundVolume( struct BINKSND * bs )
{
  if ( bs->chans > 1 )
  {
    F32 vols[ 2 ];
    if ( BS->pan <= 32767 )
    {
      vols[0] = ( ( (F32) BS->vol ) / 32768.0f );
      vols[1] = vols[0] * ( ( (F32) BS->pan ) / 32768.0f );
    }
    else
    {
      vols[1] = ( ( (F32) BS->vol ) / 32768.0f );
      vols[0] = vols[1] * ( ( (F32) ( 65536 - BS->pan ) ) / 32768.0f );
    }

    if ( BS->SourceVoice )
      BS->SourceVoice->SetChannelVolumes( 2, vols, XAUDIO2_COMMIT_NOW );
  }
  else
  {
    F32 vol;

    vol = ( ( (F32) BS->vol ) / 32768.0f );

    if ( BS->SourceVoice )
      BS->SourceVoice->SetChannelVolumes( 1, &vol, XAUDIO2_COMMIT_NOW );
  }
}


static void XASoundShutdown( struct BINKSND * bs )
{
  if ( BS->SourceVoice )
  {
    BS->SourceVoice->DestroyVoice();
    BS->SourceVoice = 0;
  }
}

/**********************************

    returns a one if a buffer range is ready
    if the mixer system starved, then record a sound dropout in bs->SoundDroppedOut

**********************************/

static S32 RADLINK Ready( struct BINKSND * bs )
{
  S32 i;

  if ( ( BS->paused ) || ( bs->OnOff == 0 ) || ( BS->SourceVoice == 0 ) )
  {
      return( 0 );
  }
  if ( BS->status != BS_PRELOADING )
  {
    XAUDIO2_VOICE_STATE state;

    state.BuffersQueued = CHUNK_COUNT;
    BS->SourceVoice->GetState( &state );

    if ( state.BuffersQueued == 0 )
    {
      bs->SoundDroppedOut = 1;
      BS->status = BS_PRELOADING;
      BS->submitted = 0;

      BS->last_frag = -1;
      BS->fill_frag = -1;
      return( 0 );
    }

    if ( state.BuffersQueued == CHUNK_COUNT )
      return( 0 );
  }

  i = ( BS->last_frag + 1 ) % CHUNK_COUNT;

  BS->fill_frag = i;
  return( 1 );
}


// this function is only called if ready has returned a non-zero value
static S32 RADLINK Lock( struct BINKSND * bs, U8 * * addr, U32 * len )
{
  if (BS->fill_frag==-1)
  {
      return 0;
  }

  *addr = &BS->SoundBuffer[BS->fill_frag*BS->bufsize];
  *len = BS->bufsize;
  return 1;
}


// called when the buffer has been filled
static S32  RADLINK Unlock (struct BINKSND * bs,U32 filled)
{
  if ( BS->fill_frag == -1 )
  {
    return 0;
  }

  BS->last_frag = BS->fill_frag;
  BS->fill_frag = -1;
  BS->submitted += filled;

  if ( BS->status == BS_PRELOADING )
  {
    if ( BS->submitted >= BS->preloadsize )
    {
      S32 i;

      BS->status = BS_STREAMING;
      for( i = 0 ; i <= BS->last_frag ; i++ )
        XASoundPlay( bs, i, BS->bufsize );

      if ( BS->SourceVoice )
      {
        // start all the voices only when we have loaded the last one
        if ( bs == &BS->bink->bsnd[BS->bink->playingtracks-1] )
        {
          bs = BS->bink->bsnd;
          for( i = BS->bink->playingtracks ; i ; i-- )
          {
            BS->SourceVoice->Start( 0, XAUDIO2_COMMIT_NOW );
            ++bs;
          }
          bs = 0; // cause a crash, if we later try to access.
        }
      }
    }
  }
  else
  {
    XASoundPlay( bs, BS->last_frag, filled );
  }

  return( 1 );
}

// set the volume 0=quiet, 32767=max, 65536=double volume (not always supported)
static void RADLINK Volume ( struct BINKSND * bs, S32 volume )
{
  if ( volume >= 65536 )
    volume = 65535;
  BS->vol = mult64anddiv( ( volume + 1 ), 32767, 32768 );
  XASoundVolume( bs );
}

// set the pan 0=left, 32767=middle, 65536=right
static void RADLINK Pan( struct BINKSND * bs, S32 pan )
{
  if ( pan > 65536 )
    pan = 65536;
  BS->pan = pan;
  XASoundVolume( bs );
}


// called to turn the sound on or off
static S32  RADLINK SetOnOff( struct BINKSND * bs, S32 status )
{
  if ( ( status == 1 ) && ( bs->OnOff == 0 ) )
  {

    if ( XASoundInit( bs ) )
    {
      XASoundVolume( bs );
    
      if ( BS->has_mixspks )
        if ( BS->SourceVoice ) 
          BS->SourceVoice->SetOutputMatrix( 0, bs->chans, BS->mbins, BS->vols, XAUDIO2_COMMIT_NOW );

      //
      // Set initial status
      //

      bs->OnOff = 1;
    }

  }
  else if ( ( status == 0 ) && ( bs->OnOff == 1 ) )
  {
    XASoundShutdown( bs );
    bs->OnOff = 0;
  }

  return( bs->OnOff );
}


// pauses/resumes the playback
static S32  RADLINK Pause( struct BINKSND * bs, S32 status )  // 1=pause, 0=resume, -1=resume and clear
{
  if ( status )
  {
    XASoundPause( bs );
    BS->paused = 1;
  }
  else
  {
    XASoundResume( bs );
    BS->paused = 0;
  }
  return( BS->paused );
}


// set the spk vol values - volumes are LRLRLR... for stereo and just VVVV... for mono
//    currently, total will always be 8
static void RADLINK SpeakerVolumes( struct BINKSND * bs, F32 * volumes, U32 total )
{
  if ( total > 8 )
    total = 8;
      
  BS->has_mixspks = 1;
  rrmemsetzero( BS->vols, 4 * 8 * 2 );
  rrmemcpy( BS->vols, volumes, 4 * total * 2 );
  
  if ( total > BS->mbins )
    BS->mbins = total;

  if ( BS->SourceVoice ) 
    BS->SourceVoice->SetOutputMatrix( 0, bs->chans, BS->mbins, BS->vols, XAUDIO2_COMMIT_NOW );
}


// closes this channel

static void RADLINK Close( struct BINKSND * bs )
{
  XASoundShutdown( bs );

  if ( BS->SoundBuffer )
  {
    radfree( BS->SoundBuffer );
    BS->SoundBuffer = 0;
  }
}

// open a bink channel
static S32 RADLINK XAudio2Open( struct BINKSND * bs, U32 freq, S32 chans, U32 flags, HBINK bink )
{
  U32 i;

  radassert( sizeof( bs->snddata ) >= sizeof( SNDCHANNELXA ) );

  //
  // Init sound descriptor
  //

  rrmemsetzero( bs, sizeof( *bs ) );

  bs->SoundDroppedOut = 0;

  bs->freq  = freq;
  bs->chans = chans;
#ifdef __RADBIGENDIAN__  
  BS->mbins = 6;
#elif defined(__RADCONSOLE__)
  BS->mbins = 8;
#elif defined(XAUDIO2_VOICE_NOSAMPLESPLAYED)  // xaudio 2.8+
  XAUDIO2_VOICE_DETAILS d;
  Master->GetVoiceDetails( &d );
  BS->mbins = d.InputChannels;
#else
  XAUDIO2_DEVICE_DETAILS d;
  XA2->GetDeviceDetails( 0, &d );
  BS->mbins = d.OutputFormat.Format.nChannels;
#endif

  // save for Bink
  bink->limit_speakers = BS->mbins;

  i = bs->freq * ( 16 >> 3 ) * bs->chans;

  if ( ( bink->FrameRate ) && ( bink->FrameRateDiv ) )
    BS->maxpreload = ( U32 ) ( ( (U64) bink->Frames * (U64) bink->FrameRateDiv * (U64) i ) / (U64) bink->FrameRate );
  else
    BS->maxpreload = i;

  //
  // Calculate size of secondary buffer based on preferences
  //
  // Round fragment size to next higher power of 2 for maximum compatibility
  //


  //
  // Set initial values
  //

  BS->bink = bink;
  BS->vol =  32767;
  BS->pan =  32767;

  //
  // Set initial status
  //

  BS->paused    = 0;
  bs->OnOff     = 1;

 //
 // Set up the function calls
 //

  bs->Ready    = Ready;
  bs->Lock     = Lock;
  bs->Unlock   = Unlock;
  bs->Volume   = Volume;
  bs->Pan      = Pan;
  bs->Pause    = Pause;
  bs->SetOnOff = SetOnOff;
  bs->Close    = Close;
  bs->SpeakerVols  = SpeakerVolumes;

 //
 // Return success
 //

  if ( XASoundInit( bs ) == 0 )
    return( 0 );

  //
  // Set preferred amount of data to be passed 
  //

  bs->BestSize = BS->bufsize;

  bs->Latency = 0;

  return(1);
}

#ifdef USE_XA27
  #define BinkOpenXAudio2 BinkOpenXAudio27
#endif

#ifdef USE_XA28
  #define BinkOpenXAudio2 BinkOpenXAudio28

  static GUID const IID_IXAudio2_28 = { 0x60d8dac8, 0x5aa1, 0x4e8e, { 0xb5, 0x97, 0x2f, 0x5e, 0x28, 0x83, 0xd4, 0x84 } };

  extern "C" int IsXAudio28( UINTa param1, UINTa param2 )
  {
    HRESULT hr;
    IUnknown * xa2 = (IUnknown *)param1;
    IUnknown *pXAudio28 = NULL;

    hr = xa2->QueryInterface( IID_IXAudio2_28, (void **) &pXAudio28 );
    if ( SUCCEEDED( hr ) )
    {
      pXAudio28->Release();
      return 1;
    }
    return 0;
  }
#endif

#ifdef USE_XA29

  #ifndef BUILDING_FOR_UNREAL_ONLY
    #define BinkOpenXAudio2 BinkOpenXAudio29
  #endif

  static GUID const IID_IXAudio2_29 = { 0x2B02E3CF, 0x2E0B, 0x4ec3, { 0xbe, 0x45, 0x1b, 0x2a, 0x3f, 0xe7, 0x21, 0x0d } };

  extern "C" int IsXAudio29( UINTa param1, UINTa param2 )
  {
    HRESULT hr;
    IUnknown * xa2 = (IUnknown *)param1;
    IUnknown *pXAudio29 = NULL;

    hr = xa2->QueryInterface( IID_IXAudio2_29, (void **) &pXAudio29 );
    if ( SUCCEEDED( hr ) )
    {
      pXAudio29->Release();
      return 1;
    }
    return 0;
  }
#endif

#if (!defined( USE_XA27 ) && !defined( USE_XA28 ) && !defined(__RADWINRT__) && !defined(__RADBIGENDIAN__) )  // win32, win64, no winrt

#include <Mmdeviceapi.h>

static const CLSID rCLSID_MMDeviceEnumerator = { 0xbcde0395, 0xe52f, 0x467c,{ 0x8e, 0x3d, 0xc4, 0x57, 0x92, 0x91, 0x69, 0x2e } };
static const IID rIID_IMMDeviceEnumerator = { 0xa95664d2, 0x9614, 0x4f35,{ 0xa7, 0x46, 0xde, 0x8d, 0xb6, 0x36, 0x17, 0xe6 } };
static const PROPERTYKEY IDevice_FriendlyName = { { 0xa45c254e, 0xdf1c, 0x4efd,{ 0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0, } }, 14 };

static WCHAR * copywide( WCHAR * o, WCHAR const * i )
{
  while( *i )   
    *o++ = *i++;
  *o = 0;
  return o;
}

RADEXPFUNC S32 RADEXPLINK BinkFindXAudio2WinDevice( void * wchar_out, char const * match )
{
  S32 fnd = 0;
  WCHAR * out = (WCHAR*)wchar_out;
  
  out[0] = 0;
  
  #if defined(__RADNT__) 
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
  #endif

  IMMDeviceEnumerator *pEnumerator = NULL;
  if ( CoCreateInstance( rCLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, rIID_IMMDeviceEnumerator, (void**)&pEnumerator ) != S_OK )
    return 0;

  IMMDeviceCollection *pCollection = NULL;
  if ( pEnumerator->EnumAudioEndpoints( eRender, DEVICE_STATE_ACTIVE, &pCollection ) == S_OK )
  {
    UINT count = 0;
    pCollection->GetCount( &count );

    for ( ULONG i = 0 ; i < count ; i++ )
    {
      // Get pointer to endpoint number i.
      IMMDevice * pEndpoint = NULL;
      pCollection->Item( i, &pEndpoint );
      if ( pEndpoint )
      {
        LPWSTR pwszID = NULL;
      
        // Get the endpoint ID string.
        pEndpoint->GetId( &pwszID );
        if ( pwszID )
        {
          PROPVARIANT varName;
          
          // Initialize container for property value.
          PropVariantInit( &varName );

          IPropertyStore *pProps = NULL;
          pEndpoint->OpenPropertyStore( STGM_READ, &pProps );
          if ( pProps )
          {
            // Get the endpoint's friendly-name property.
            if ( pProps->GetValue( IDevice_FriendlyName, &varName ) == S_OK )
            {
              // look for match
              WCHAR const * a = varName.pwszVal;
              while( *a )
              {
                WCHAR const * ac = a;
                char const * b = match;
                for(;;)
                {
                  if ( *b == 0 )
                  {
                    fnd = 1;
                    out = copywide( out, L"\\\\?\\SWD#MMDEVAPI#" );
                    out = copywide( out, pwszID );
                    out = copywide( out, L"#{E6327CAD-DCEC-4949-AE8A-991E976A79D2}" );
                    goto stop;
                  }
                  if ( ( *ac == 0 ) || ( *ac != *b ) )
                    break;
                  ++ac;
                  ++b;
                }
                ++a;
              }
             stop:;
            }
            
            pProps->Release();
          }

          PropVariantClear(&varName);
          CoTaskMemFree(pwszID);
        }

        pEndpoint->Release();
      }
      if ( fnd ) break;
    }

    pCollection->Release();
  }

  pEnumerator->Release();

  return fnd;
}

#endif

#if defined(__RADCONSOLE__) && !defined(__RADBIGENDIAN__)

#elif (defined(__RADNT__) && defined(XAUDIO2_VOICE_NOSAMPLESPLAYED))  // if 2.8 on windows desktop, dyload!

extern "C" BinkFreeGlobalsCallback * BinkFreeGlobals1;

typedef HRESULT WINAPI xacreatetype( IXAudio2 ** p, UINT32 Flags, XAUDIO2_PROCESSOR pro );
static HMODULE xadll = 0;
static xacreatetype * xacreate = 0;

static void RADLINK BinkXaudioGlobalClose()
{
  if ( we_alloced )
  {
    if ( XA2 )
    {
      Master->DestroyVoice();
      XA2->Release();
      XA2 = 0;
      Master = 0;
    }
    we_alloced = 0;
  }

  FreeLibrary( xadll );
  xadll = 0;
  xacreate = 0;

#if defined(__RADNT__) 
  CoUninitialize();
#endif
  
  // reset it
  BinkFreeGlobals1 = 0;
}

#endif

// param = 0, we allocate the xaudio voice
// param = (UINTa)-1, we free any previously allocated xaudio device
// param otherwise, IXAudio2 device that you allocated
RADEXPFUNC BINKSNDOPEN RADEXPLINK BinkOpenXAudio2( UINTa param1, UINTa param2 )
{
  if ( we_alloced )
  {
    if ( XA2 )
    {
      Master->DestroyVoice();
      XA2->Release();
      XA2 = 0;
      Master = 0;
    }
    we_alloced = 0;
  }

  if ( ( param1 ) && ( param1 != (UINTa)-2 ) )  // -2 means param 2 has device name
  {
    if ( param1 == (UINTa)-1 )
    {
      return 0;
    }

    XA2 = (IXAudio2 *) param1;
    Master = (IXAudio2MasteringVoice *) param2;
    we_alloced = 0;
    return( XAudio2Open );
  }
  else
  {
    IXAudio2 * xa2 = 0;
    IXAudio2MasteringVoice * master = 0;

#if defined(__RADCONSOLE__) && !defined(__RADBIGENDIAN__)

    XAudio2Create( &xa2, 0x0400, XAUDIO2_DEFAULT_PROCESSOR );

#elif (defined(__RADNT__) && defined(XAUDIO2_VOICE_NOSAMPLESPLAYED))  // if 2.8 on windows desktop, dyload!
  

    if ( xadll == 0 )
    {
    #if defined(__RADNT__) 
      CoInitializeEx(NULL, COINIT_MULTITHREADED);
    #endif
      xadll = LoadLibrary( OUR_XAUDIO2 );

      #if defined(OUR_OTHER_XAUDIO2) 
      if ( xadll == 0 )
        xadll = LoadLibrary( OUR_OTHER_XAUDIO2 );
      #endif
    }

    if ( xadll )
      if ( xacreate == 0 )
        xacreate = (xacreatetype *)GetProcAddress( xadll, "XAudio2Create" );

    if ( xacreate == 0 )
      return 0; 

    xacreate( &xa2, 0, XAUDIO2_DEFAULT_PROCESSOR );

    // setup callback to free audio at BinkFree
    BinkFreeGlobals1 = BinkXaudioGlobalClose;

#else

    #if defined(__RADNT__) 
      CoInitializeEx(NULL, COINIT_MULTITHREADED);
    
      // dummy xaudio load - prevents a crash at shutdown time
      //  https://blogs.msdn.microsoft.com/chuckw/2015/10/09/known-issues-xaudio-2-7/
      LoadLibrary( "XAudio2_7.DLL" );
    #endif

    // Xaudio 2.7      
    XAudio2Create( &xa2, 0, XAUDIO2_DEFAULT_PROCESSOR );

#endif

    if ( xa2 )
    {
#if defined( USE_XA27 ) || defined( __RADBIGENDIAN__ )
      xa2->CreateMasteringVoice( &master ); 
#else
      xa2->CreateMasteringVoice( &master, XAUDIO2_DEFAULT_CHANNELS, XAUDIO2_DEFAULT_SAMPLERATE, 0, ( param1 != (UINTa)-2 ) ? 0 : ((LPCWSTR)param2) ); 
#endif
      if ( master == 0 )
      {
        xa2->Release();
        return 0;
      }
      XA2 = xa2;
      Master = master;
      we_alloced = 1;
      return( XAudio2Open );
    }

  }
  return( 0 );
}

