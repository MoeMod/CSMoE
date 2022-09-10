/*
snd_main.c - load & save various sound formats
Copyright (C) 2010 Uncle Mike

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "soundlib.h"

// global sound variables
sndlib_t	sound;

static void Sound_Reset( void )
{
	// reset global variables
	sound.width = sound.rate = 0;
	sound.channels = sound.loopstart = 0;
	sound.samples = sound.flags = 0;
	sound.type = WF_UNKNOWN;

	sound.wav = NULL;
	sound.size = 0;
}

static wavdata_t *SoundPack( void )
{
	wavdata_t	*pack = (wavdata_t *)Mem_ZeroAlloc( host.soundpool, sizeof( wavdata_t ));

	pack->buffer = sound.wav;
	pack->width = sound.width;
	pack->rate = sound.rate;
	pack->type = sound.type;
	pack->size = sound.size;
	pack->loopStart = sound.loopstart;
	pack->samples = sound.samples;
	pack->channels = sound.channels;
	pack->flags = sound.flags;

	return pack;
}

/*
================
FS_LoadSound

loading and unpack to wav any known sound
================
*/
wavdata_t *FS_LoadSound( const char *filename, const byte *buffer, size_t size )
{
	const char	*ext = FS_FileExtension( filename );
	string		path, loadname;
	qboolean		anyformat = true;
	fs_offset_t		filesize = 0;
	const loadwavfmt_t	*format;
	const byte	*f;

	Sound_Reset(); // clear old sounddata
	Q_strncpy( loadname, filename, sizeof( loadname ));

	if( Q_stricmp( ext, "" ))
	{
		// we need to compare file extension with list of supported formats
		// and be sure what is real extension, not a filename with dot
		for( format = sound.loadformats; format && format->formatstring; format++ )
		{
			if( !Q_stricmp( format->ext, ext ))
			{
				FS_StripExtension( loadname );
				anyformat = false;
				break;
			}
		}
	}

	// HACKHACK: skip any checks, load file from buffer
	if( filename[0] == '#' && buffer && size ) goto load_internal;

	// now try all the formats in the selected list
	for( format = sound.loadformats; format && format->formatstring; format++)
	{
		if( anyformat || !Q_stricmp( ext, format->ext ))
		{
			Q_sprintf( path, format->formatstring, loadname, "", format->ext );
			f = FS_MapFile( path, &filesize, false );
			if( f && filesize > 0 )
			{
				if( format->loadfunc( path, f, (size_t)filesize ))
				{
                    FS_MapFree(f, filesize); // release buffer
					return SoundPack(); // loaded
				}
				else FS_MapFree(f, filesize); // release buffer
			}
		}
	}

load_internal:
	for( format = sound.loadformats; format && format->formatstring; format++ )
	{
		if( anyformat || !Q_stricmp( ext, format->ext ))
		{
			if( buffer && size > 0  )
			{
				if( format->loadfunc( loadname, buffer, size ))
					return SoundPack(); // loaded
			}
		}
	}

	if( !sound.loadformats || sound.loadformats->ext == NULL )
		MsgDev( D_NOTE, "FS_LoadSound: soundlib offline\n" );
	else if( filename[0] != '#' )
		MsgDev( D_WARN, "FS_LoadSound: couldn't load \"%s\"\n", loadname );

	return NULL;
}

/*
================
Sound_FreeSound

free WAV buffer
================
*/
void FS_FreeSound( wavdata_t *pack )
{
	if( pack )
	{
		if( pack->buffer ) Mem_Free( pack->buffer );
		Mem_Free( pack );
	}
	else MsgDev( D_WARN, "FS_FreeSound: trying to free NULL sound\n" );
}

/*
================
FS_OpenStream

open and read basic info from sound stream 
================
*/
stream_t *FS_OpenStream( const char *filename )
{
	const char	*ext = FS_FileExtension( filename );
	string		path, loadname;
	qboolean		anyformat = true;
	const streamfmt_t	*format;
	stream_t		*stream;

	Sound_Reset(); // clear old streaminfo
	Q_strncpy( loadname, filename, sizeof( loadname ));

	if( Q_stricmp( ext, "" ))
	{
		// we needs to compare file extension with list of supported formats
		// and be sure what is real extension, not a filename with dot
		for( format = sound.streamformat; format && format->formatstring; format++ )
		{
			if( !Q_stricmp( format->ext, ext ))
			{
				FS_StripExtension( loadname );
				anyformat = false;
				break;
			}
		}
	}

	// now try all the formats in the selected list
	for( format = sound.streamformat; format && format->formatstring; format++)
	{
		if( anyformat || !Q_stricmp( ext, format->ext ))
		{
			Q_sprintf( path, format->formatstring, loadname, "", format->ext );
			if(( stream = format->openfunc( path )) != NULL )
			{
				stream->format = format;
				return stream; // done
			}
		}
	}

	if( !sound.streamformat || sound.streamformat->ext == NULL )
		MsgDev( D_NOTE, "FS_OpenStream: soundlib offline\n" );
	else MsgDev( D_NOTE, "FS_OpenStream: couldn't open \"%s\"\n", loadname );

	return NULL;
}

/*
================
FS_StreamInfo

get basic stream info
================
*/
wavdata_t *FS_StreamInfo( stream_t *stream )
{
	static wavdata_t	info;

	if( !stream ) return NULL;

	// fill structure
	info.loopStart = -1;
	info.rate = stream->rate;
	info.width = stream->width;
	info.channels = stream->channels;
	info.flags = SOUND_STREAM; 
	info.size = stream->size;
	info.buffer = NULL;
	info.samples = 0;	// not actual for streams
	info.type = stream->type;

	return &info;
}

/*
================
FS_ReadStream

extract stream as wav-data and put into buffer, move file pointer
================
*/
int FS_ReadStream( stream_t *stream, int bytes, void *buffer )
{
	if( !stream || !stream->format || !stream->format->readfunc )
		return 0;

	if( bytes <= 0 || buffer == NULL )
		return 0;

	return stream->format->readfunc( stream, bytes, buffer );
}

/*
================
FS_GetStreamPos

get stream position (in bytes)
================
*/
int FS_GetStreamPos( stream_t *stream )
{
	if( !stream || !stream->format || !stream->format->getposfunc )
		return -1;

	return stream->format->getposfunc( stream );
}

/*
================
FS_SetStreamPos

set stream position (in bytes)
================
*/
int FS_SetStreamPos( stream_t *stream, int newpos )
{
	if( !stream || !stream->format || !stream->format->setposfunc )
		return -1;

	return stream->format->setposfunc( stream, newpos );
}

/*
================
FS_FreeStream

close sound stream
================
*/
void FS_FreeStream( stream_t *stream )
{
	if( !stream || !stream->format || !stream->format->freefunc )
		return;

	stream->format->freefunc( stream );
}
