// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#ifndef __RADRR_COREH__
  #include "rrCore.h"
#endif

#include "radmem.h"

//#define SHOWMEMORY // undefine to outputdebugstring all allocs and frees

//#define __RADMALDEBUG__

#if defined(__RADMALDEBUG__) && !defined(DLL_FOR_GENERIC_OS)

#include "rad_except.h"  // @cdep ignore

  RADDEFFUNC void RADLINK RADSetMemory(RADMEMALLOC a,RADMEMFREE f)
  {
  }

  #ifdef __RADFINAL__
    #error "You have debug mallocs turned on!"
  #endif

  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>

#ifdef TELEMEMORY
  RADDEFFUNC void * RADLINK radmalloci(U64 size, char const * info, U32 line)
#else
  RADDEFFUNC void * RADLINK radmalloc(U64 size)
#endif
  {
     static unsigned int pagesize=0;
     void* p;
     U64 s;

     // get the page size, if we haven't yet
     if (pagesize==0)
     {
       SYSTEM_INFO si;
       GetSystemInfo(&si);
       pagesize=si.dwPageSize;
     }

     // we need room for the size, 8 bytes to hide the original pointer and a
     //   validation dword, and enough data to completely fill one page
     s=(size+16+(pagesize-1))&~(pagesize-1);

#ifdef __RAD32__
     rrassert( size <= 0xffffffff );
#endif     

     // allocate the size plus a page (for the guard)
     p=VirtualAlloc(0,(UINTa)(s+pagesize),MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);

     if (p)
     {
       DWORD oldprot;
       void* orig=p;

       // protect the final page
       VirtualProtect(((char*)p)+s,pagesize,PAGE_NOACCESS,&oldprot);

       // now move the returned pointer so that it bumps right up against the
       //   the next (protected) page (this may result in unaligned return
       //   addresses - pre-align the sizes if you always want aligned ptrs)
       p=((char*)p)+(s-size);

       // hide the validation value and the original pointer (which we'll
       //   need used for freeing) right behind the returned pointer
       ((unsigned int*)p)[-1]=0x98765432;
       ((void**)p)[-2]=orig;

       return(p);
     }

     return 0;
  }

  RADDEFFUNC void RADLINK radfree(void * ptr)
  {
    if (ptr)
    {
      int err=0;

      // is this one of our allocations?
      RAD_try
      {
        if (((((unsigned int*)ptr)[-1])!=0x98765432) || ((((void**)ptr)[-2])==0))
        {
          err=1;
        }
      }
      RAD_except ( default )
      {
        err=1;
      }
      RAD_end

      if (err)
      {
        MessageBox(0,"Bad pointer or double freed memory.","Memory free error",MB_OK|MB_ICONSTOP);
      }
      else
      {

        // back up to find the original pointer
        void* p=((void**)ptr)[-2];

        // clear the validation value and the original pointer
        ((unsigned int*)ptr)[-1]=0;
        ((void**)ptr)[-2]=0;

        // now free the pages
        if (p)
          VirtualFree(p,0,MEM_RELEASE);

      }
    }
  }

#else 


  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>

  static HANDLE ourheap = 0;

  #if !defined( DLL_FOR_GENERIC_OS )  

  RADDEFFUNC void * RADLINK radmalloc_init(void)
  {
    if ( ourheap == 0 )
    {
      ourheap = HeapCreate( 0, 256, 0 );
    }
    return (void*)ourheap;
  }
  
  #endif

  static void* ourmalloc( U64 bytes )
  {
    void* ptr;
    if ( ourheap == 0 )
    {
      #if defined( DLL_FOR_GENERIC_OS )  
        return 0;
      #else
        if ( radmalloc_init() == 0 )
          return 0;
      #endif
    }

    #if !defined(__RAD64__)
      if ( bytes > 0xffffffff )
        return 0;
      ptr = HeapAlloc( ourheap, 0, (U32)bytes );
    #else
      ptr = HeapAlloc( ourheap, 0, bytes );
    #endif

#ifdef SHOWMEMORY
    { 
      char buf[ 128 ];
      wsprintf( buf, "allocated %p bytes %I64d\n", ptr, bytes );
      OutputDebugString( buf );
    }
#endif

    return ptr;
  }

  static void ourfree( void * ptr )
  {
#ifdef SHOWMEMORY
    { 
      char buf[ 128 ];
      wsprintf( buf, "freeing   %p\n", ptr );
      OutputDebugString( buf );
    }
#endif
    HeapFree( ourheap, 0, ptr );
  }

  #define radmalrad ourmalloc
  #define radfrrad ourfree

  #include "radmem.inl"

#endif
