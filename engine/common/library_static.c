/*
 Copyright (C) 2018 fgsfds, modified for switch the switch port by mats

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
#include "library.h"
#include "library_static.h"
#include "server.h"

typedef struct dll_s
{
	const char *name;
	int refcnt;
	dllexport_t *exp;
	struct dll_s *next;
} dll_t;

static dll_t *dll_list;
char lasterror[1024] = "";

static void *LS_dlfind( const char *name )
{
	dll_t *d = NULL;
	for( d = dll_list; d; d = d->next )
		if( !Q_strcmp( d->name, name ) )
			break;
	return d;
}

static const char *LS_dlname( void *handle )
{
	dll_t *d = NULL;
	// iterate through all dll_ts to check if the handle is actually in the list
	// and not some bogus pointer from god knows where
	for( d = dll_list; d; d = d->next ) if( d == handle ) break;
	return d ? d->name : NULL;
}

void *LS_dlopen( const char *name, int flag )
{
	dll_t *d = LS_dlfind( name );
	if( d ) d->refcnt++;
	else Com_PushLibraryError("LS_dlopen(): unknown dll name");
	return d;
}

void *LS_dlsym( void *handle, const char *symbol )
{
	if( !handle || !symbol ) { Com_PushLibraryError("LS_dlsym(): NULL args"); return NULL; }
	if( !LS_dlname( handle ) ) { Com_PushLibraryError("LS_dlsym(): unknown handle"); return NULL; }
	dll_t *d = handle;
	if( !d->refcnt ) { Com_PushLibraryError("LS_dlsym(): call LS_dlopen() first"); return NULL; }
	dllexport_t *f = NULL;
	for( f = d->exp; f && f->func; f++ )
		if( !Q_strcmp( f->name, symbol ) )
			break;

	if( f && f->func )
	{
		return f->func;
	}
	else
	{
		Com_PushLibraryError("LS_dlsym(): symbol not found in dll");
		return NULL;
	}
}

int LS_dlclose( void *handle )
{
	if( !handle ) { Com_PushLibraryError("LS_dlclose(): NULL arg"); return -1; }
	if( !LS_dlname( handle ) ) { Com_PushLibraryError("LS_dlclose(): unknown handle"); return -2; }
	dll_t *d = handle;
	if( !d->refcnt ) { Com_PushLibraryError("LS_dlclose(): call LS_dlopen() first"); return -3; }
	d->refcnt--;
	return 0;
}

char *LS_dlerror( void );
#if 0
int LS_dladdr( const void *addr, Dl_info *info )
{
	dll_t *d = NULL;
	dllexport_t *f = NULL;
	for( d = dll_list; d; d = d->next )
		for( f = d->exp; f && f->func; f++ )
			if( f->func == addr ) goto for_end;
	for_end:
	if( d && f && f->func )
	{
		if( info )
		{
			info->dli_fbase = d;
			info->dli_sname = f->name;
			info->dli_saddr = addr;
		}
		return 1;
	}
	return 0;
}
#endif
// export registering api for all dlls //

int dll_register( const char *name, dllexport_t *exports )
{
	if( !name || !exports ) return -1;
	if( LS_dlfind( name ) ) return -2; // already registered
	dll_t *new = calloc( 1, sizeof( dll_t ) );
	if( !new ) return -3;
	new->name = name;
	new->exp = exports;
	new->next = dll_list;
	dll_list = new;
    return 0;
}

void *Com_LoadLibrary( const char *dllname, int build_ordinals_table )
{
	return LS_dlopen(dllname, RTLD_NOW);
}

void *Com_GetProcAddress( void *hInstance, const char *name )
{
	return LS_dlsym( hInstance, name );
}

const char *Com_NameForFunction( void *hInstance, void *function )
{
	return NULL;
}

void *Com_FunctionFromName( void *hInstance, const char *pName )
{
	return LS_dlsym( hInstance, pName );
}

void *Com_FunctionFromName_SR( void *hInstance, const char *pName )
{
#ifdef XASH_ALLOW_SAVERESTORE_OFFSETS
	if( !Q_memcmp( pName, "ofs:",4 ) )
		return (void*)((intptr_t)svgame.dllFuncs.pfnGameInit + (ptrdiff_t)Q_atoi(pName + 4));
#endif
	return Com_FunctionFromName( hInstance, pName );
}

void Com_FreeLibrary( void *hInstance )
{
	LS_dlclose( hInstance );
}

void Com_PushLibraryError( const char *error )
{
	Q_strncat( lasterror, error, sizeof( lasterror ) );
	Q_strncat( lasterror, "\n", sizeof( lasterror ) );
}

void Com_ResetLibraryError()
{
	lasterror[0] = '\0';
}

const char *Com_GetLibraryError()
{
	return lasterror;
}
