// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#if !(BUILDING_GRANNY || BUILDING_GRANNY_STATIC)

#ifndef __RADRR_COREH__
  #include "rrCore.h"
#endif

#include "radmem.h"
#include <stdlib.h>

#ifdef __RADMACH__
  #include <Carbon/Carbon.h>
#else
  #include <codefragments.h>
#endif

static RADMEMALLOC usermalloc=0;
static RADMEMFREE  userfree=0;

RADDEFFUNC void RADEXPLINK RADSetMemory(RADMEMALLOC a,RADMEMFREE f)
{
  usermalloc=a;
  userfree=f;
}

typedef Ptr pascal newptrsystype( Size logicalsize );

// These functions insure that we are aligned on a 16-byte boundary

RADDEFFUNC void* RADEXPLINK radmalloc(U64 numbytes64)
{
  U8 *p=0;
  U8 d;
  Handle h;
  OSErr e;
  U8 typ=0;
  static newptrsystype* newptrsys = 0;

  if (numbytes64==0)
    return 0;

  if (usermalloc)
  {
    p=(U8*)usermalloc(numbytes64+64);
    if (p==0)
      goto cont;
    if (((SINTa)(UINTa)p)==-1)
      return(0);
    typ=3;
  }
  else
  {
    U32 numbytes;
   cont:
    #ifdef __RAD64__
      p=(U8*)malloc(numbytes64+64);
      typ=1;
    #else
    if ( numbytes64 > 0xffffffff )
      return 0;
    numbytes = (U32) numbytes64;
    if (MaxBlock()>(numbytes+256000))
      p=(U8*)NewPtr(numbytes+64);                   // try app memory if it won't use it up
    if (!p) {
      h=TempNewHandle(numbytes+64,&e);            // try multifinder memory
      if (e==noErr) {
        TempHLock(h,&e);
        if (e==noErr) {
          typ=2;
          p=(U8*)*h;
        }
      }
      if (!p) 
      {
        if ( newptrsys == 0 )
        {
          CFragConnectionID conn_id;
          char* main_addr;
          Str255 error;
          CFragSymbolClass sym_class;

          //
          // Dynamically try to find NewPtrSys
          //

          if ( GetSharedLibrary( "\pInterfaceLib",
                                 kPowerPCCFragArch,
                                 kLoadCFrag,
                                 &conn_id,
                                 &main_addr,
                                 ( unsigned char* ) &error ) == noErr )
          {
            if ( FindSymbol( conn_id,
                             "\pNewPtrSys",
                             ( char** )&newptrsys,
                             &sym_class ) != noErr )
            {
              //
              // Mark the function address with an invalid, non-zero value
              //
              * ( U32 * ) &newptrsys = 1;
            }
          }
        }

        //
        // is the function present? if so, call it...
        //
         
        if ( ( ( U32 ) newptrsys ) > 1 )
          p=(U8*)newptrsys(numbytes+64);             // try sys memory
  
        if (!p)
          return 0;
        typ=1;
      }
    }  
    #endif  
  }

  d=64-((UINTa)p&31);
  p+=d;
  p[-1]=d;
  p[-2]=typ;

  if (typ==2)                                   // Save the multifinder handle
    *((Handle*)&p[-16])=h;
  else if (typ==3)                              // Save the function to free the memory
    *((RADMEMFREE*)&p[-16])=userfree;

  return p;
}

RADDEFFUNC void RADEXPLINK radfree(void* ptr)
{
  Handle h;
  OSErr e;
  U8 *p;
	RADMEMFREE f;
	
	if (!ptr) return;
	
	p=(U8*)ptr;
	switch (p[-2])
	{
	  case 3:
	    f=*((RADMEMFREE*)&p[-16]);
	    f((char*)(p-p[-1]));
	    break;
#ifdef __RAD64__
    default:  
      free((char*)(p-p[-1]));
#else      
    case 2:                              // if multifinder memory
      h=*((Handle*)&p[-16]);
      TempHUnlock( h, &e);
      TempDisposeHandle(h,&e);
      break;
	  default:
    	DisposePtr((char*)(p-p[-1]));
    	break;
#endif      
  }
}

#else

char RadmemGrannyLibtoolPacifier;

#endif // !(BUILDING_GRANNY || BUILDING_GRANNY_STATIC)
