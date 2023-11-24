// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#include "binkplugin.h"
#include "binkpluginos.h"

#ifdef RADUSETM3
#include "telemetryplugin.h"
#endif

#include <windows.h>

#ifndef __RADNT__
#define DO_DIRECTSOUND_PROC()
#endif

#ifdef BUILDING_FOR_UNREAL_ONLY

//#define ProcessProc( bytes, ret, name, ...) extern ret name(__VA_ARGS__);
#define ProcessProc( bytes, ret, name, ...) typedef ret RADLINK RR_STRING_JOIN( name, Proc )(__VA_ARGS__); RR_STRING_JOIN( name, Proc ) * RR_STRING_JOIN( p, name ) = name;
  DO_PROCS()
  DO_WIN_PROCS()
#undef ProcessProc

#else

#ifdef __RADWINRT__
  #define LoadLib(name) LoadPackagedLibrary(name,0)
  #ifdef __RAD64__
    #define bink1name L"binkwinrt_x64.uni10.dll"
    #define bink2name L"bink2winrt_x64.uni10.dll"
  #else
    #define bink1name L"binkwinrt_x86.uni10.dll"
    #define bink2name L"bink2winrt_x86.uni10.dll"
  #endif
#else
  #define LoadLib(name) LoadLibraryW(name)
  #ifdef __RAD64__
    #define bink1name L"binkw64.dll"
    #define bink2name L"bink2w64.dll"
  #else
    #define bink1name L"binkw32.dll"
    #define bink2name L"bink2w32.dll"
  #endif
#endif

#define ProcessProc( bytes, ret, name, ... ) RR_STRING_JOIN( name, Proc ) * RR_STRING_JOIN( p, name );
  DO_PROCS()
  DO_WIN_PROCS()
  DO_DIRECTSOUND_PROC()
#undef ProcessProc

static HINSTANCE dllinst;

static void mystrcpyw( wchar_t * d, wchar_t * s )
{
  while(*s) *d++=*s++;
  *d=0;
}

S32 dynamically_load_procs( void )
{
  HMODULE l;
  wchar_t dll[ MAX_PATH ];
  wchar_t * fn;

  if ( BinkPIPath[0] )
  {
    char const * d = BinkPIPath;
    fn = dll;
    do
    {
      *fn++ = *d++;
    } while ( d[0] );

    // add a slash, if we don't have one.
    if ( ( d[-1] != '/' ) && ( d[-1] != '\\' ) )
      *fn++ = '\\';
  }
  else
  {
    #ifdef __RADWINRT__
      fn = dll;
    #else 
      wchar_t * d;

      dll[0] = 0;
      GetModuleFileNameW( dllinst, dll, MAX_PATH - 1 );

      fn = dll;
      d = dll;
      while ( d[0] )
      {
        if ( ( d[0] == '\\' ) || ( d[0] == '/' ) ) fn = d + 1;
        ++d;
      }
    #endif
  }
  
  mystrcpyw( fn, bink2name );
  l = LoadLib( dll );
  if ( l == 0 )
  {
    mystrcpyw( fn, bink1name );
    l = LoadLib( dll );
    if ( l == 0 )
    {
      BinkPluginSetError( "Couldn't load Bink DLL." );
      return 0;
    }
  }

#define STRINGDELAY( v ) #v
#define STRINGIZE( v ) STRINGDELAY( v )

#ifdef __RAD64__
  #define ProcessProc( num, ret, name, ... ) {RR_STRING_JOIN( p, name ) = ( RR_STRING_JOIN( name, Proc )* ) GetProcAddress( l, STRINGIZE( name ) ); if ( RR_STRING_JOIN( p, name ) == 0 ) return 0;};
#else
  #define ProcessProc( num, ret, name, ... ) {RR_STRING_JOIN( p, name ) = ( RR_STRING_JOIN( name, Proc )* ) GetProcAddress( l, STRINGIZE( RR_STRING_JOIN( RR_STRING_JOIN( RR_STRING_JOIN( _, name ), @ ), num ) ) ); if ( RR_STRING_JOIN( p, name ) == 0 ) return 0;};
#endif
    DO_PROCS()
    DO_DIRECTSOUND_PROC()
    DO_WIN_PROCS()

  #undef ProcessProc

  return 1;
}

BOOL WINAPI DllMain(
  _In_ HINSTANCE hinstDLL,
  _In_ DWORD     fdwReason,
  _In_ LPVOID    lpvReserved
)
{
  dllinst = hinstDLL;
  return 1;
}

#endif

static HANDLE ourheap = 0;

static BinkPluginAlloc_t * Alloc = 0;
static BinkPluginFree_t * Free = 0; 

void * osmalloc( U64 bytes )
{
  void * p, * ap;

  #if !defined(__RAD64__)
    if ( bytes > 0xffffff00 )
      return 0;
  #endif

  if (Alloc)
    return Alloc( (UINTa)bytes, 16 );

  if ( ourheap == 0 )
  {
    ourheap = HeapCreate( 0, 256, 0 );
    if ( ourheap == 0 )
    {
      return( 0 );
    }
  }
  #if !defined(__RAD64__)
    p = HeapAlloc( ourheap, 0, (U32)bytes+16+15 );
  #else
    p = HeapAlloc( ourheap, 0, bytes+16+15 );
  #endif
  ap = (void*) (((((UINTa)p)+16)+15)&~15);
  ((void**)ap)[-1]=p;
  return ap;
}

void osfree( void * ptr )
{
  if ( ptr )
  {
    void * op;

    if (Free)
    {
      Free( ptr );
      return;
    }

    op = ((void**)ptr)[-1];
    if ( op )
    {
      ((void**)ptr)[-1] = 0;
      HeapFree( ourheap, 0, op );
    }
  }
}

void osmemoryreset( void )
{
  if ( ourheap )
  {
    HeapDestroy( ourheap );
    ourheap = 0;
  }
}

void set_mem_allocators( BINKPLUGININITINFO * info )
{
  if(info) 
  {
    Alloc = info->alloc;
    Free = info->free;
  }
}

void oserr( char const * err )
{
#if !defined(__RADWINRT__) // don't call outputdebugstring on winrt
  OutputDebugString( "Bink: " );
  OutputDebugString( err );
#endif
}

S32 RADLINK binkdefopen(UINTa * user_data, const char * fn, U64 * optional_out_filesize)
{
  HANDLE h;

#if defined(__RADWINRT__) // don't call outputdebugstring on winrt
  h = CreateFile2((wchar_t*)fn, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, 0);
#else
  h = CreateFileW((wchar_t*)fn, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0);
#endif
  if ( h == INVALID_HANDLE_VALUE )
    return 0;

  if ( optional_out_filesize )
  {
    LARGE_INTEGER l;
    if ( !GetFileSizeEx( h, &l ) )       
    {
      CloseHandle(h);
      return 0;
    }
    *optional_out_filesize = l.QuadPart;
  }


  *user_data = (UINTa)h;
  return 1;
}

U64 RADLINK binkdefread(UINTa * user_data, void * dest,U64 bytes)
{
  HANDLE h;
  DWORD amt;
  U64 amt64;

  h = (HANDLE)*user_data;
  amt64 = 0;
  while( bytes )
  {
    DWORD b;
    BOOL res;

    b = ( bytes > ( 512*1024*1024 ) ) ? (512*1024*1024) : ((U32)bytes);
    amt = 0;
    res = ReadFile( h,dest,b,&amt,0);

    bytes -= amt;
    dest = (U8*)dest + amt;
    amt64 += amt;
    if (!res)
      break;
    if ( ( amt == 0 ) && ( b ) ) // file is smaller than the header claims it is
      break;
  }

  return amt64;
}

void RADLINK binkdefseek(UINTa * user_data, U64 pos)
{
  HANDLE h;
  U32 posb,post;

  h = (HANDLE)*user_data;

  // this api is nuts
  post = (U32) ( pos >> 32 );
  posb = SetFilePointer( h, (U32) pos, (PLONG)&post, FILE_BEGIN );
}

void RADLINK binkdefclose(UINTa * user_data)
{
  HANDLE h;

  h = (HANDLE)*user_data;
  CloseHandle( h );
  *user_data = (UINTa)INVALID_HANDLE_VALUE;
}
