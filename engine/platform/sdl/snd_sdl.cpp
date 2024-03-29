/*
s_backend.c - sound hardware output
Copyright (C) 2009 Uncle Mike

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "common.h"
#if XASH_SOUND == SOUND_SDL

#include "sound.h"
#include "voice.h"

#include <SDL.h>

#define SAMPLE_16BIT_SHIFT 1
#define SECONDARY_BUFFER_SIZE 0x10000

/*
=======================================================================
Global variables. Must be visible to window-procedure function
so it can unlock and free the data block after it has been played.
=======================================================================
*/
static int sdl_dev;
static SDL_AudioDeviceID in_dev;
static SDL_AudioFormat sdl_format;

//static qboolean	snd_firsttime = true;
//static qboolean	primary_format_set;

void SDL_SoundCallback( void *userdata, Uint8 *stream, int len )
{
	int size    = dma.samples << 1;
	int pos     = dma.samplepos << 1;
	int wrapped = pos + len - size;

	if( wrapped < 0 )
	{
		memcpy( stream, dma.buffer + pos, len );
		dma.samplepos += len >> 1;
	}
	else
	{
		int remaining = size - pos;
		memcpy( stream, dma.buffer + pos, remaining );
		memcpy( stream + remaining, dma.buffer, wrapped );
		dma.samplepos = wrapped >> 1;
	}
}

/*
==================
SNDDMA_Init

Try to find a sound device to mix for.
Returns false if nothing is found.
==================
*/
qboolean SNDDMA_Init( void *hInst )
{
	SDL_AudioSpec desired, obtained;
	int samplecount;

	if( SDL_Init( SDL_INIT_AUDIO ) )
	{
		MsgDev( D_ERROR, "Audio: SDL: %s \n", SDL_GetError( ) );
		return false;
	}

#ifdef __linux__
	setenv( "PULSE_PROP_application.name", GI->title, 1 );
	setenv( "PULSE_PROP_media.role", "game", 1 );
#endif

	Q_memset( &desired, 0, sizeof( desired ) );
	desired.freq     = SOUND_DMA_SPEED;
	desired.format   = AUDIO_S16LSB;
	desired.samples  = 1024;
	desired.channels = 2;
	desired.callback = SDL_SoundCallback;

	sdl_dev = SDL_OpenAudioDevice( NULL, 0, &desired, &obtained, 0 );

	if( !sdl_dev )
	{
		Con_Printf( "Couldn't open SDL audio: %s\n", SDL_GetError( ) );
		return false;
	}

	if( obtained.format != AUDIO_S16LSB )
	{
		Con_Printf( "SDL audio format %d unsupported.\n", obtained.format );
		goto fail;
	}

	if( obtained.channels != 1 && obtained.channels != 2 )
	{
		Con_Printf( "SDL audio channels %d unsupported.\n", obtained.channels );
		goto fail;
	}

	dma.format.speed    = obtained.freq;
	dma.format.channels = obtained.channels;
	dma.format.width    = 2;
	samplecount = s_samplecount->integer;
	if( !samplecount )
		samplecount = 0x8000;
	dma.samples         = samplecount * obtained.channels;
	dma.buffer          = (byte*)Z_Malloc( dma.samples * 2 );
	dma.samplepos       = 0;
	dma.sampleframes    = dma.samples / dma.format.channels;

    sdl_format = obtained.format;

	Con_Printf( "Using SDL audio driver: %s @ %d Hz\n", SDL_GetCurrentAudioDriver( ), obtained.freq );

	SDL_PauseAudioDevice( sdl_dev, 0 );

	dma.initialized = true;
	return true;

fail:
	SNDDMA_Shutdown( );
	return false;
}

/*
==============
SNDDMA_GetDMAPos

return the current sample position (in mono samples read)
inside the recirculating dma buffer, so the mixing code will know
how many sample are required to fill it up.
===============
*/
int SNDDMA_GetDMAPos( void )
{
	return dma.samplepos;
}

/*
==============
SNDDMA_GetSoundtime

update global soundtime
===============
*/
int SNDDMA_GetSoundtime( void )
{
	static int buffers, oldsamplepos;
	int samplepos, fullsamples;

	fullsamples = dma.samples / 2;

	// it is possible to miscount buffers
	// if it has wrapped twice between
	// calls to S_Update.  Oh well.
	samplepos = SNDDMA_GetDMAPos( );

	if( samplepos < oldsamplepos )
	{
		buffers++; // buffer wrapped

		if( paintedtime > 0x40000000 )
		{
			// time to chop things off to avoid 32 bit limits
			buffers     = 0;
			paintedtime = fullsamples;
			S_StopAllSounds( );
		}
	}

	oldsamplepos = samplepos;

	return ( buffers * fullsamples + samplepos / 2 );
}

/*
==============
SNDDMA_BeginPainting

Makes sure dma.buffer is valid
===============
*/
void SNDDMA_BeginPainting( void )
{
	SDL_LockAudio( );
}

/*
==============
SNDDMA_Submit

Send sound to device if buffer isn't really the dma buffer
Also unlocks the dsound buffer
===============
*/
void SNDDMA_Submit( void )
{
	SDL_UnlockAudio( );
}

/*
==============
SNDDMA_Shutdown

Reset the sound device for exiting
===============
*/
void SNDDMA_Shutdown( void )
{
	Con_Printf( "Shutting down audio.\n" );
	dma.initialized = false;

	if( sdl_dev )
	{
		SDL_PauseAudioDevice( sdl_dev, 1 );
#ifndef __EMSCRIPTEN__
		SDL_CloseAudioDevice( sdl_dev );
		SDL_CloseAudio( );
#endif
	}

#ifndef __EMSCRIPTEN__
	if( SDL_WasInit( SDL_INIT_AUDIO ) )
		SDL_QuitSubSystem( SDL_INIT_AUDIO );
#endif

	if( dma.buffer )
	{
		Mem_Free( dma.buffer );
		dma.buffer = NULL;
	}
}

/*
===========
S_PrintDeviceName
===========
*/
void S_PrintDeviceName( void )
{
	Msg( "Audio: SDL (driver: %s)\n", SDL_GetCurrentAudioDriver( ) );
}

/*
===========
S_Activate
Called when the main window gains or loses focus.
The window have been destroyed and recreated
between a deactivate and an activate.
===========
*/
void S_Activate( qboolean active )
{
	SDL_PauseAudioDevice( sdl_dev, !active );
}


/*
===========
SDL_SoundInputCallback
===========
*/
void SDL_SoundInputCallback( void *userdata, Uint8 *stream, int len )
{
    int size;

    size = min( len, sizeof( voice.buffer ) - voice.buffer_pos );
    SDL_memset( voice.buffer + voice.buffer_pos, 0, size );
    SDL_MixAudioFormat( voice.buffer + voice.buffer_pos, stream, sdl_format, size, SDL_MIX_MAXVOLUME );
    voice.buffer_pos += size;
}

/*
===========
VoiceCapture_Init
===========
*/
qboolean VoiceCapture_Init( void )
{
    SDL_AudioSpec wanted, spec;

    SDL_zero( wanted );
    wanted.freq = voice.samplerate;
    wanted.format = AUDIO_S16LSB;
    wanted.channels = voice.channels;
    wanted.samples = voice.frame_size / voice.width;
    wanted.callback = SDL_SoundInputCallback;

    in_dev = SDL_OpenAudioDevice( NULL, SDL_TRUE, &wanted, &spec, 0 );

    if( !in_dev )
    {
        Con_Printf( "VoiceCapture_Init: error creating capture device (%s)\n", SDL_GetError() );
        return false;
    }

    Con_Printf( "VoiceCapture_Init: capture device creation success (%i: %s)\n", in_dev, SDL_GetAudioDeviceName( in_dev, SDL_TRUE ) );
    return true;
}

/*
===========
VoiceCapture_RecordStart
===========
*/
qboolean VoiceCapture_RecordStart( void )
{
    if(!in_dev)
    {
        if ( !VoiceCapture_Init() )
        {
            return false;
        }
    }

    SDL_PauseAudioDevice( in_dev, SDL_FALSE );

    return true;
}

/*
===========
VoiceCapture_RecordStop
===========
*/
void VoiceCapture_RecordStop( void )
{
    if(in_dev)
    {
        SDL_PauseAudioDevice( in_dev, SDL_TRUE );
        SDL_CloseAudioDevice( in_dev );
        in_dev = false;
    }
}

#endif // XASH_SOUND == SOUND_SDL
