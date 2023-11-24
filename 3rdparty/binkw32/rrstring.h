// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

// Copyright 2008-2013 RAD Game Tools

// This file is equivalent to the old radmemutils. since it offers
// both strcpy-ish and memcpy-ish functions, and the C string.h
// defines both sets of those as well, this is called rrstring.h.

// The implementations are derived from the old radmemutils as
// well, with Watcom removed, and without actually providing the
// appropriate implementations for all the consoles--those need
// to be added as time passes, if necessary.

#ifndef __RADRR_STRINGH__
#define __RADRR_STRINGH__

#include "rrCore.h"

#include <string.h>   

RADDEFSTART

//// most platforms use the following definitions: those that don't undefine these...
//// this makes it SLIGHTLY harder to determine what each platform does,
//// but reduces the filesize by 2x and makes it easier to understand the
//// *whole* file.

// string functions
#define rrstrcat   strcat
#define rrstrcpy   strcpy
#define rrstrlen   strlen
#define rrstrcmp   strcmp
#define rrstricmp  stricmp
#define rrstrchr   strchr
RADDEFFUNC char * rrstpcpy(char* dest,char const * src); // like strcpy, but returns a pointer to the end of the string (so you can nest)

// memory functions
#define rrmemmovebig memmove // use for large copies (>512 bytes) - can overlay
#define rrmemmove  memmove   // overlap - unknown direction

#define rrmemcpy   memcpy    // smallish copies - no overlap
#define rrmemcpydb memmove   // overlap - destination address bigger than source
#define rrmemcpyds memmove   // overlap - destination address smaller than source

#define rrmemsetbig  memset  // use for large sets (>512 bytes)
#define rrmemset   memset
#define rrmemsetzero(d,c) memset(d,0,c) // use for small zero clears
#define rrmemsetzerobig(d,c) memset(d,0,c) // use for large zero (>512 bytes)

#define rrmemcmpbig  memcmp  // use for large compares (>512 bytes)
#define rrmemcmp   memcmp    // smallish compares

//// most platforms use the following definitions

RADDEFFUNC void rrmemset16(void* dest,U16 value,U32 count_w);
RADDEFFUNC void rrmemset32(void* dest,U32 value,U32 count_w);

//// most platforms use the following functions:

// UTF-16 string functions
RADDEFFUNC U16 *rrwcscpy(U16 *dst, const U16 *src); // returns ptr to null terminator of copied string
RADDEFFUNC U32 rrwcslen(const U16 *string);
RADDEFFUNC S32 rrwcscmp(const U16 *str1, const U16 *str2);
RADDEFFUNC S32 rrwcsncmp(const U16 *str1, const U16 *str2, U32 n);
RADDEFFUNC U16 *rrwcschr(U16 *str, U16 chr); // doesn't handle chars outside basic multilingual plane
RADDEFFUNC U16 *rrwcsrchr(U16 *str, U16 chr); // doesn't handle chars outside basic multilingual plane
RADDEFFUNC S32 rrwcstol(U16 *str, U16 **endptr, S32 base); // base must be given explicitly, we don't parse "0blah" or "0xblah"

// platform specific defines

#if defined(__RADX64__)

   #undef rrstricmp
   #define rrstricmp _stricmp

   #if defined(_MSC_VER)
      #define rrstrupr _strupr
      #define rrstrlwr _strlwr
      #define rrstru32(s) ((U32)atol(s))
      #define rrstru64(s) ((U32)_atoi64(s))

      #pragma intrinsic(memset,memcpy)
   #endif

   #undef rrmemset16
   #define rrmemset16 rrmemset16_inline
   static void __inline rrmemset16_inline(void* dest,U16 value,U32 count_w)
   {
      U32 v = value | ( ( (U32) value ) << 16 );
      U32 s = count_w >> 1;
      U32 * RADRESTRICT d = (U32*)dest;
      while ( s )
      {
         --s;
         *d++=v;
      }
      if ( count_w & 1 )
      {
         *(U16*)d = value;
      }
   }

#else // windows

   #if defined(_MSC_VER) && defined(__RADX86__) && !defined(__RAD64__)

      #undef rrmemcpydb
      #undef rrstricmp
      
      #define rrstrupr _strupr
      #define rrstrlwr _strlwr
      #define rrstru32(s) ((U32)atol(s))
      #define rrstru64(s) ((U32)_atoi64(s))

      #pragma warning(push)
      #pragma warning( disable : 4035)
      RADDEFFUNC void *memset ( void*, int, unsigned int );
      RADDEFFUNC void *memcpy ( void*, const void*, unsigned int );
      #pragma intrinsic(memset,memcpy)

      static void __inline rrmemcpydb( void const * dest, void const * src, U32 bytes )
      {
         __asm
         {
            mov ecx,[bytes]
            mov edi,[dest]
            mov esi,[src]
            std
            mov edx,ecx
            lea esi,[esi+ecx-4]
            lea edi,[edi+ecx-4]
            shr ecx,2
            rep movsd
            and edx,3
            jz dne
            add esi,3
            add edi,3
            mov ecx,edx
            rep movsb
           dne:
            cld
         }
      }

      static int __inline rrstricmp( void const * s1, void const * s2 )
      {
         __asm
         {
            mov edi,dword ptr [s1]
            mov esi,dword ptr [s2]

            mov eax,1 //skips the first matched check

           zc:
            cmp eax,0
            je matched

           lp:
            movzx eax,byte ptr [edi]
            movzx edx,byte ptr [esi]
            inc esi
            inc edi
            cmp eax,edx
            je zc

            cmp eax,'a'
            jb c1
            cmp eax,'z'
            ja c1
            sub eax,32
           c1:
            cmp edx,'a'
            jb c2
            cmp edx,'z'
            ja c2
            sub edx,32
            c2:

            sub eax,edx
            je lp

            cdq
            lea eax,[edx+edx+1]

           matched:
         }
      }
      
      #define rrmemset16 rrmemset16_inline 
      static void __inline rrmemset16_inline(void* dest,U16 value,U32 count_w) {
         __asm {
            mov edi,[dest]
            mov ax,[value] 
            mov ecx,[count_w]
            shl eax,16 
            cld 
            mov ax,[value] 
            mov bl,cl 
            shr ecx,1 
             rep stosd
            mov cl,bl 
            and cl,1 
            rep stosw
         }
      }

      #define rrmemset32 rrmemset32_inline
      static void __inline rrmemset32_inline(void* dest,U32 value,U32 count_d) {
         __asm {
             mov edi,[dest]
             mov eax,[value]
             mov ecx,[count_d]
             cld
             rep stosd
         }
      }

      #pragma warning(pop)

   #endif //  defined(_MSC_VER)

#endif // else

RADDEFEND 

#endif // __RADRR_STRINGH__

