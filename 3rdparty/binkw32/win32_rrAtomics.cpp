// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

// Copyright 2008-2013 RAD Game Tools

#include "rrAtomics.h"

#if defined(__RADPPC__)
  #undef S8
  #define NOD3D
  #include <xtl.h>
  #define S8 signed char
#else
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  
  #include <windows.h> 
  
  #ifndef __RADARM__
    #ifdef _MSC_VER
      #pragma warning(disable:4995)
    #endif
    
    #if defined(_MSC_VER) && _MSC_VER < 1300
      #pragma warning(disable:4035)
      #include <mmintrin.h>
      #include <mmsystem.h>
    #else
      #include <emmintrin.h>
    #endif
  #endif

  #if defined(_MSC_VER) && _MSC_VER > 1500
    #include <intrin.h>  // for _InterlockedCompareExchange128
  #endif
#endif


// add replacements for the intrinsics for losers using vc6 and 2003
#if !defined(__RADPPC__) && !defined(__RAD64__)
  #if _MSC_VER <= 1500
    // for msvc6 and vsnet2003
    extern "C" long __cdecl _InterlockedCompareExchange(volatile long *p, long n, long p_compare);
    extern "C" long __cdecl _InterlockedExchangeAdd(volatile long *p, long n);
    extern "C" long __cdecl _InterlockedExchange(volatile long *p, long n);
    extern "C" __int64 __cdecl _InterlockedCompareExchange64(volatile __int64 *dest, __int64 newv, __int64 p_compare);

    #pragma intrinsic(_InterlockedCompareExchange)
    #pragma intrinsic(_InterlockedExchangeAdd)
    #pragma intrinsic(_InterlockedExchange)

    #if _MSC_VER < 1400
    extern "C" __int64 __cdecl _InterlockedCompareExchange64(volatile __int64 *dest, __int64 newv, __int64 p_compare)
    {
      __asm
      {
        mov eax,dword ptr [p_compare]
        mov edx,dword ptr [p_compare+4]
        mov ebx,dword ptr [newv]
        mov ecx,dword ptr [newv+4]
        mov esi,dword ptr [dest]
        lock cmpxchg8b qword ptr [esi]
        // eax:edx = *dest, returned as int64
      }
    }
    #else
    #pragma intrinsic(_InterlockedCompareExchange64)
    #endif

    #if _MSC_VER < 1300
      // for msvc6
      #define ULONG_PTR DWORD
    #else
      // for vsnet2003
    #endif
  #endif
#endif


/*************************************************************************************************/
// get intrinsics in global namespace :


extern "C"
{
	// these are the *optimizer* barriers, not memory barriers :
	#if _MSC_VER >= 1400
	#define HAS_READ_BARRIER
	void _ReadBarrier(void); // ReadBarrier is VC2005
	#endif
	
	void _WriteBarrier(void);
	void _ReadWriteBarrier(void);
}

// _ReadBarrier, _WriteBarrier, and _ReadWriteBarrier
// these are just *compiler* memory operation barriers, they are NOT fences :

#if defined(_MSC_VER) && _MSC_VER < 1300
   // VC6 has no link optimization, so no worries about inlining
   // cross-file and needing compiler read/write barriers
   #define CompilerReadBarrier()
   #define CompilerWriteBarrier()
   #define CompilerReadWriteBarrier()
#else
   #pragma intrinsic(_WriteBarrier)
   #pragma intrinsic(_ReadWriteBarrier)

   #define CompilerReadWriteBarrier	_ReadWriteBarrier
   #define CompilerWriteBarrier	_WriteBarrier

   #ifdef HAS_READ_BARRIER
   #pragma intrinsic(_ReadBarrier)	// ReadBarrier is VC2005
   #define CompilerReadBarrier		_ReadBarrier	
   #else
   #define CompilerReadBarrier		_ReadWriteBarrier
   #endif
#endif

#ifdef __RADARM__
// FG: source = VC++ 2012 "xatomic.h" (impl. level for std::atomic)
// "_MemoryBarrier"
// Windows MemoryBarrier() is a full system barrier which is stronger
// than this, which is an "Inner sharable" barrier.
#define AtomicMemoryBarrier()    __dmb(_ARM_BARRIER_ISH) 
#endif

/*************************************************************************************************/

RADDEFFUNC void RADLINK rrAtomicMemoryBarrierFull( void )
{
  #ifdef MemoryBarrier
    // MemoryBarrier is in Windows.h on Vista or newer
    MemoryBarrier();
  #elif defined(__RADX64__)
    _mm_mfence();
  #else  
    volatile long temp;
    __asm lock xchg temp,eax;
  #endif
}

/*************************************************************************************************/

U32 rrAtomicLoadAcquire32(U32 volatile const * ptr)
{
	RR_ASSERT(	(((UINTa)ptr)&3) == 0 );
#if defined(__RADX86__)
	// on x86, loads are Acquire :
	CompilerReadBarrier();
	U32 ret = *((volatile U32 *)ptr);
	CompilerReadBarrier();
#elif defined(__RADARM__)
  // FG: source = "xatomic.h", _Load_acquire_4)
  U32 ret = __iso_volatile_load32((const volatile __int32 *)ptr);
  AtomicMemoryBarrier();
#endif
	return ret;
}

/**
BTW there's one extra nasty issue with this.
In 32 bit builds, 64-bit values on the stack are NOT aligned to 8 bytes by the compiler automatically.  This can cause atomicity failures.
I try to have asserts about required alignment in rrAtomics, but it's something to be aware of.
I believe that statics/globals are aligned properly, which is what people normally use as atomic values
**/

U64 rrAtomicLoadAcquire64(U64 volatile const * src)
{
	RR_ASSERT(	(((UINTa)src)&7) == 0 );
#if defined(__RADX64__)
	// on x86, loads are Acquire :
	CompilerReadBarrier();
	U64 ret = *((volatile U64 *)src);
	CompilerReadBarrier();
	return ret;
#elif defined(__RADX86__)
  U64 outv;
  __asm
  {
    mov eax,[src]
    fild qword ptr [eax]
    fistp qword ptr [outv]
  }
  return outv;
#elif defined(__RADARM__)
  U64 ret = __iso_volatile_load64((const volatile __int64 *)src);
  AtomicMemoryBarrier();
  return ret;
#endif
}

// *ptr = val; // gaurantees earlier stores do not move afterthis
void rrAtomicStoreRelease32(U32 volatile * ptr,U32 val)
{
	RR_ASSERT(	(((UINTa)ptr)&3) == 0 );
#if defined(__RADX86__)
	// on x86, stores are Release :
	CompilerWriteBarrier();
	*((volatile U32 *)ptr) = val;
	CompilerWriteBarrier();
#elif defined(__RADARM__)
  AtomicMemoryBarrier();
  __iso_volatile_store32((volatile __int32 *)ptr, val);
#endif
}

void rrAtomicStoreRelease64(U64 volatile * dest,U64 val)
{
	RR_ASSERT(	(((UINTa)dest)&7) == 0 );
#if defined(__RADX64__)
	// on x86, stores are Release :
	CompilerWriteBarrier();
	*((volatile U64 *)dest) = val;
	CompilerWriteBarrier();
#elif defined(__RADX86__)
	// on 32 bit windows
	// trick from Intel TBB : (atomic_support.asm)
	//	see http://www.niallryan.com/node/137
	// FPU can store 64 bits atomically
	__asm
	{
		mov eax, dest
		fild qword ptr [val]
		fistp qword ptr [eax]
	}
#elif defined(__RADARM__)
  AtomicMemoryBarrier();
  __iso_volatile_store64((volatile __int64 *)dest, val);
#endif
}

// Windows style CmpXchg 
U32 rrAtomicCmpXchg32(U32 volatile * pDestVal, U32 newVal, U32 compareVal)
{
	RR_ASSERT(	(((UINTa)pDestVal)&3) == 0 );
	return _InterlockedCompareExchange((LONG *)pDestVal,(LONG)newVal,(LONG)compareVal);
}

U64 rrAtomicCmpXchg64(U64 volatile * pDestVal, U64 newVal, U64 compareVal)
{
	RR_ASSERT(	(((UINTa)pDestVal)&7) == 0 );
	return _InterlockedCompareExchange64((__int64 *)pDestVal,(__int64)newVal,(__int64)compareVal);
}

// CAS like C++0x
// atomically { if ( *pDestVal == *pOldVal ) { *pDestVal = newVal; } else { *pOldVal = *pDestVal; } }
rrbool rrAtomicCAS32(U32 volatile * pDestVal, U32 * pOldVal, U32 newVal)
{
	U32 old = rrAtomicCmpXchg32(pDestVal,newVal,*pOldVal);
	if ( *pOldVal == old ) return true;
	*pOldVal = old;
	return false;
}
rrbool rrAtomicCAS64(U64 volatile * pDestVal, U64 * pOldVal, U64 newVal)
{
	U64 old = rrAtomicCmpXchg64(pDestVal,newVal,*pOldVal);
	if ( *pOldVal == old ) return true;
	*pOldVal = old;
	return false;
}

// atomically { *pOldVal = *pDestVal; *pDestVal = newVal; return *pOldVal; };
U32 rrAtomicExchange32(U32 volatile * pDestVal, U32 newVal)
{
	return _InterlockedExchange((LONG *)pDestVal,newVal);
}

U64 rrAtomicExchange64(U64 volatile * pDestVal, U64 newVal)
{
#if defined(_WIN64) || defined(__RADARM__)
	// InterlockedExchange64 requires Vista
	return InterlockedExchange64((__int64 *)pDestVal,newVal);
#else
	
	U64 oldVal = rrAtomicLoadAcquire64(pDestVal);
	while ( ! rrAtomicCAS64(pDestVal,&oldVal,newVal) )
	{
	}
	
	return oldVal;
	
#endif	
}

// atomically { old = *pDestVal; *pDestVal += incVal; return old; }
U32 rrAtomicAddExchange32(U32 volatile * pDestVal, S32 incVal)
{
	return _InterlockedExchangeAdd((LONG *)pDestVal,incVal);
}

U64 rrAtomicAddExchange64(U64 volatile * pDestVal, S64 incVal)
{
#if defined(_WIN64) || defined(__RADARM__)
	return _InterlockedExchangeAdd64((__int64 *)pDestVal,incVal);
#else
	
	U64 oldVal = rrAtomicLoadAcquire64(pDestVal);
	for(;;)
	{
		U64 newVal = oldVal + incVal;
		if ( rrAtomicCAS64(pDestVal,&oldVal,newVal) )
			return oldVal;
	}
	
#endif
}

/*

// ppc :
               
  asm
  long         // <- Zero on failure, one on success (r3).
AtomicCAS(
  long prev,   // -> Previous value (r3).
  long next,   // -> New value (r4).
  void *addr ) // -> Location to update (r5).
{
retry:
  lwarx  r6, 0, r5 // current = *addr;
  cmpw   r6, r3    // if( current != prev )
  bne    fail      // goto fail;
  stwcx. r4, 0, r5 // if( reservation == addr ) *addr = next;
  bne-   retry     // else goto retry;
  li      r3, 1    // Return true.
  blr              // We're outta here.
fail:
  stwcx. r6, 0, r5 // Clear reservation.
  li     r3, 0     // Return false.
  blr              // We're outta here.
}

*/

