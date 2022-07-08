/*
libmpg.h - compact version of famous library mpg123
Copyright (C) 2017 Uncle Mike

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef LIBMPG_H
#define LIBMPG_H

// error codes
#define MP3_ERR		-1
#define MP3_OK		0
#define MP3_NEED_MORE	1

#define OUTBUF_SIZE		8192	// don't change!

typedef struct
{
	long	rate;		// num samples per second (e.g. 11025 - 11 khz)
	int	channels;		// num channels (1 - mono, 2 - stereo)
	int	playtime;		// stream size in milliseconds
} wavinfo_t;

// custom stdio
/* A little hack to help MSVC not having ssize_t. */
#ifdef _MSC_VER
#include <stddef.h>
typedef ptrdiff_t mpg123_ssize_t;
#else
typedef ssize_t mpg123_ssize_t;
#endif
typedef mpg123_ssize_t(*pfread)( void *handle, void *buf, size_t count );
typedef off_t (*pfseek)( void *handle, off_t offset, int whence );

extern void *create_decoder( int *error );
extern int feed_mpeg_header( void *mpg, const char *data, size_t bufsize, long streamsize, wavinfo_t *sc );
extern int feed_mpeg_stream( void *mpg, const char *data, size_t bufsize, char *outbuf, size_t *outsize );
extern int open_mpeg_stream( void *mpg, void *file, pfread f_read, pfseek f_seek, wavinfo_t *sc );
extern int read_mpeg_stream( void *mpg, char *outbuf, size_t *outsize  );
extern int get_stream_pos( void *mpg );
extern int set_stream_pos( void *mpg, int curpos );
extern void close_decoder( void *mpg );
const char* get_error(void* mh);


#endif//LIBMPG_H