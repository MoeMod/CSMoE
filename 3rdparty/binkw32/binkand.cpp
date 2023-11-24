// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>

#include <android/asset_manager.h>

#include "egttypes.h"


// Rad timer read routine
RADEXPFUNC U32 RADEXPLINK RADTimerRead( void )
{
  struct timeval _time;

  gettimeofday( &_time, NULL );

  return (U32) ( ( _time.tv_sec * 1000000LL + _time.tv_usec ) / 1000 );
}

#include "defread.h"

// File routines
static AAssetManager* s_AssetManager;

RADDEFFUNC void RADEXPLINK BinkSetAssetManager(void* asset_manager) 
{ 
  s_AssetManager = (AAssetManager*)asset_manager; 
}

RADDEFFUNC S32  RADLINK binkdefopen (UINTa * user_data, const char * fn, U64 * out_filesize)
{
  int f = 0;
  f = open( fn, O_RDONLY );
  if ( f != -1 )
  {
    user_data[0] = f;
    user_data[1] = 1;

    if ( out_filesize )
    {
      *out_filesize = 
      #ifdef __RAD64__
      lseek64( f, 0, SEEK_END );
      lseek64( f, 0, SEEK_SET );
      #else
      lseek( f, 0, SEEK_END );
      lseek( f, 0, SEEK_SET );
      #endif
    }

    return 1;
  }
  else
  {
    if (s_AssetManager != 0)
    {
      AAsset* asset;
      asset = AAssetManager_open(s_AssetManager, fn, AASSET_MODE_STREAMING);
      if ( asset )
      {
        user_data[0] = (UINTa)asset;
        user_data[1] = 2;

        if ( out_filesize )
        {
          *out_filesize = 
          #ifdef __RAD64__
          AAsset_seek64( asset, 0, SEEK_END );
          AAsset_seek64( asset, 0, SEEK_SET );
          #else
          AAsset_seek( asset, 0, SEEK_END );
          AAsset_seek( asset, 0, SEEK_SET );
          #endif
        }

        return 1;
      }
    }
  }

  return 0;
}

RADDEFFUNC void RADLINK binkdefclose(UINTa * user_data)
{
  if ( user_data[1] == 1 )
  {
    close( (int) user_data[0] );
    user_data[0] = 0;
    user_data[1] = 0;
  }
  else if ( user_data[1] == 2 )
  {
    AAsset_close( (AAsset*) user_data[0] );
    user_data[0] = 0;
    user_data[1] = 0;
  }
}

RADDEFFUNC U64  RADLINK binkdefread (UINTa * user_data, void * dest, U64 bytes)
{
  if ( user_data[1] == 1 )
  {
    U64 amt64;

    amt64 = 0;
    while( bytes )
    {
      size_t b;
      size_t amt;

      b = ( bytes > ( 512*1024*1024 ) ) ? (512*1024*1024) : ((U32)bytes);
      amt = read( (int) user_data[0], dest, b );
      bytes -= amt;
      dest = (U8*)dest + amt;
      amt64 += amt;

      if (amt != b )
        break;
    }

    return amt64;
  }
  else if ( user_data[1] == 2 )
  {
    AAsset* A = (AAsset*) user_data[0] ;
    U64 amt64;

    amt64 = 0;
    while( bytes ) 
    {
      size_t b;
      size_t amt;
      #ifdef __RAD64__
      U64 remn = AAsset_getRemainingLength64( A );
      #else
      U32 remn = AAsset_getRemainingLength( A );
      #endif

      b = ( bytes > ( 512*1024*1024 ) ) ? (512*1024*1024) : ((U32)bytes);
      if ( (U32)b > remn ) b = remn;

      amt = AAsset_read( A, dest, b );
      if ((ptrdiff_t)amt < 0)
        return amt64;  // error

      bytes -= amt;
      dest = (U8*)dest + amt;
      amt64 += amt;

      if (amt != b )
        break;
    }

    return amt64;
  }

  return 0;
}

RADDEFFUNC void  RADLINK binkdefseek (UINTa * user_data, U64 pos)
{
  if ( user_data[1] == 1 )
  {
    #ifdef __RAD64__
    lseek64( (int) user_data[0], pos, SEEK_SET );
    #else
    lseek( (int) user_data[0], (U32)pos, SEEK_SET );
    #endif
  }
  else if ( user_data[1] == 2 )
  {
    #ifdef __RAD64__
    AAsset_seek64( (AAsset*) user_data[0], pos, SEEK_SET );
    #else
    AAsset_seek( (AAsset*) user_data[0], (U32) pos, SEEK_SET );
    #endif
  }
}
