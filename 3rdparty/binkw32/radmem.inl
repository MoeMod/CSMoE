// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

static RADMEMALLOC usermalloc=0;
static RADMEMFREE  userfree=0;

RADDEFFUNC void RADLINK RADSetMemory(RADMEMALLOC a,RADMEMFREE f)
{
  usermalloc=a;
  userfree=f;
}

#ifdef RADUSETM3

#define g_tm_api g_radtm_api
#include "rad_tm.h"
extern tm_api * g_radtm_api;
extern U32 tm_mask;

RADDEFFUNC void * RADLINK radmalloci(U64 numbytes, char const * info, U32 line)
#else
RADDEFFUNC void * RADLINK radmalloc(U64 numbytes)
#endif
{
  U8 * temp;
  U8 i;
  U8 typ;

  if ((numbytes==0) || (numbytes==0xffffffff))
    return(0);

  if (usermalloc)
  {
    temp=(U8 *)usermalloc(numbytes+64);
    if (temp==0)
      goto cont;
    if (((SINTa)(UINTa)temp)==-1)
      return(0);
    typ=3;
  }
  else
  {
   cont:
#ifdef DLL_FOR_GENERIC_OS
    return 0;
#else    
    temp=(U8 *)radmalrad(numbytes+64);
    if (temp==0)
      return(0);
    typ=0;
#endif
  }

  i=(U8)(64-((UINTa)temp&31));
  temp+=i;
  temp[-1]=i;
  temp[-2]=typ;

  if (typ==3)                              // Save the function to free the memory
    *((RADMEMFREE *)&temp[-16])=userfree;

#ifdef RADUSETM3
  tmAllocEx( tm_mask,info,line, temp, numbytes, "Bink alloc" );
#endif

  return(temp);
}


RADDEFFUNC void RADLINK radfree(void * ptr)
{
  if (ptr)
  {

#ifdef RADUSETM3
  tmFree( tm_mask, ptr );
#endif

    if (((U8 *)ptr)[-2]==3)
    {
      RADMEMFREE f=*((RADMEMFREE *)&(((U8 *)ptr)[-16]));
      f(((U8 *)ptr)-((U8 *)ptr)[-1] );
    }
#ifndef DLL_FOR_GENERIC_OS
    else
      radfrrad( ((U8 *)ptr)-((U8 *)ptr)[-1] );
#endif
  }
}
