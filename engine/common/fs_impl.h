#pragma once

#if defined __ANDROID__
#include <android/asset_manager.h>
#endif

#define FILE_BUFF_SIZE		2048

typedef struct file_s
{
	xe::fs::fd_t	handle;			// file descriptor
	xe::fs::off_t	real_length;		// uncompressed file size (for files opened in "read" mode)
	xe::fs::off_t	position;			// current position in the file
	xe::fs::off_t	offset;			// offset into the package (0 if external file)
	int		ungetc;			// single stored character from ungetc, cleared to EOF when read
	time_t		filetime;			// pak, wad or real filetime
						// Contents buffer
	xe::fs::off_t	buff_ind, buff_len;		// buffer current index and length
	byte		buff[FILE_BUFF_SIZE];	// intermediate buffer
#if defined __ANDROID__
	AAsset* asset;
#endif
} file_t;

typedef struct packfile_s
{
	char		name[56];
	xe::fs::off_t	offset;
	xe::fs::off_t	realsize;	// real file size (uncompressed)
} packfile_t;

typedef struct pack_s
{
	char		filename[MAX_SYSPATH];
	xe::fs::fd_t		handle;
	int		numfiles;
	time_t		filetime;	// common for all packed files
	packfile_t* files;
} pack_t;