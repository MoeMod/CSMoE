// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#include "egttypes.h"

typedef int audio_open( int freq, int chans, int * fragsize );
typedef void audio_close(void);
typedef int audio_ready(void);
typedef int audio_write( const void * data, int length );
typedef int audio_check_settings( int * freq, int * chans, int * latency_ms, int * poll_ms );

typedef struct audio_funcs
{
  audio_open  * open;
  audio_close * close;
  audio_ready * ready;
  audio_write * write;
  audio_check_settings * check;
} audio_funcs;

RADEXPFUNC BINKSNDOPEN RADEXPLINK BinkOpenSSMix( void * low_level_func_ptrs, U32 freq, U32 chans );
