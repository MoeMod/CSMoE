// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#ifndef __RADRTL_ATOMICS_H__
#define __RADRTL_ATOMICS_H__

#include "rrCore.h"

RADDEFSTART

//Note on atomic ops : types usually have to be aligned to their size to ensure atomicity in all cases.
//This is sometimes enforced by assert in rrAtomics.

//The best way to do this is RAD_ALIGN with __RRPTRBYTES__ or __RRTWOPTRBYTES__

#define rrAtomicsVersion 11
#define rrAtVN( name ) RR_STRING_JOIN( name, rrAtomicsVersion )
#define rrAtVNe( name, extra ) RR_STRING_JOIN3( name, extra, rrAtomicsVersion )

#define rrAtomicMemoryBarrierFull rrAtVN(rrAtomicMemoryBarrierFull)
#define rrAtomicLoadAcquire32 rrAtVN(rrAtomicLoadAcquire32)
#define rrAtomicLoadAcquire64 rrAtVN(rrAtomicLoadAcquire64)
#define rrAtomicStoreRelease32 rrAtVN(rrAtomicStoreRelease32)
#define rrAtomicStoreRelease64 rrAtVN(rrAtomicStoreRelease64)
#define rrAtomicCmpXchg32 rrAtVN(rrAtomicCmpXchg32)
#define rrAtomicCmpXchg64 rrAtVN(rrAtomicCmpXchg64)
#define rrAtomicCAS32 rrAtVN(rrAtomicCAS32)
#define rrAtomicCAS64 rrAtVN(rrAtomicCAS64)
#define rrAtomicExchange32 rrAtVN(rrAtomicExchange32)
#define rrAtomicExchange64 rrAtVN(rrAtomicExchange64)
#define rrAtomicAddExchange32 rrAtVN(rrAtomicAddExchange32)
#define rrAtomicAddExchange64 rrAtVN(rrAtomicAddExchange64)


// full read/write barrier :
RADDEFFUNC void RADLINK rrAtomicMemoryBarrierFull( void );

//-----------------------------------------------------------

// return *ptr; // gaurantees later reads do not move before this
U32 rrAtomicLoadAcquire32(U32 const volatile * ptr);
U64 rrAtomicLoadAcquire64(U64 const volatile * ptr);

// *ptr = val; // gaurantees earlier stores do not move afterthis
void rrAtomicStoreRelease32(U32 volatile * ptr,U32 val);
void rrAtomicStoreRelease64(U64 volatile * ptr,U64 val);

//-----------------------------------------------------------
// rr memory convention assumes that all the Exchange type functions are Acq-Rel (Acquire & Release)

// Windows style CmpXchg 
// atomically { oldVal = *pDestVal; if ( *pDestVal == compareVal ) { *pDestVal = newVal; } return oldVal; }
U32 rrAtomicCmpXchg32(U32 volatile * pDestVal, U32 newVal, U32 compareVal);
U64 rrAtomicCmpXchg64(U64 volatile * pDestVal, U64 newVal, U64 compareVal);

// CAS like C++11
// NOTE : Cmpx and CAS have different argument order convention, a little confusing
// atomically { if ( *pDestVal == *pOldVal ) { *pDestVal = newVal; return true; } else { *pOldVal = *pDestVal; return false; } }
// strong CAS, i.e. no spurious failures (if false is returned, that really does mean we took the "else" branch and not
// that something else went wrong)
rrbool rrAtomicCAS32(U32 volatile * pDestVal, U32 * pOldVal, U32 newVal);
rrbool rrAtomicCAS64(U64 volatile * pDestVal, U64 * pOldVal, U64 newVal);

// atomically { oldVal = *pDestVal; *pDestVal = newVal; return oldVal; };
U32 rrAtomicExchange32(U32 volatile * pDestVal, U32 newVal);
U64 rrAtomicExchange64(U64 volatile * pDestVal, U64 newVal);

// atomically { old = *pDestVal; *pDestVal += incVal; return old; }
U32 rrAtomicAddExchange32(U32 volatile * pDestVal, S32 incVal);
U64 rrAtomicAddExchange64(U64 volatile * pDestVal, S64 incVal);

//-----------------------------------------------------------

//-----------------------------------------------------------
// make UINTa calls that select 32 or 64 :

#define rrAtomicAddExchange_3264    rrAtVNe(rrAtomicAddExchange,RAD_PTRBITS)
#define rrAtomicExchange_3264       rrAtVNe(rrAtomicExchange,RAD_PTRBITS)
#define rrAtomicCAS_3264            rrAtVNe(rrAtomicCAS,RAD_PTRBITS)
#define rrAtomicCmpXchg_3264        rrAtVNe(rrAtomicCmpXchg,RAD_PTRBITS)
#define rrAtomicStoreRelease_3264   rrAtVNe(rrAtomicStoreRelease,RAD_PTRBITS)
#define rrAtomicLoadAcquire_3264    rrAtVNe(rrAtomicLoadAcquire,RAD_PTRBITS)

//-----------------------------------------------------------
// Pointer aliases that go through UINTa :

#ifdef __cplusplus

#define rrAtomicLoadAcquirePointer rrAtVN(rrAtomicLoadAcquirePointer)
#define rrAtomicStoreReleasePointer rrAtVN(rrAtomicStoreReleasePointer)
#define rrAtomicCASPointer rrAtVN(rrAtomicCASPointer)
#define rrAtomicExchangePointer rrAtVN(rrAtomicExchangePointer)
#define rrAtomicCmpXchgPointer rrAtVN(rrAtomicCmpXchgPointer)

static RADINLINE void* rrAtomicLoadAcquirePointer(void* const volatile * ptr)
{
    return (void*)(UINTa) rrAtomicLoadAcquire_3264((UINTa const volatile *)ptr);
}
static RADINLINE void rrAtomicStoreReleasePointer(void* volatile * pDest,void* val)
{
    rrAtomicStoreRelease_3264((UINTa volatile *)pDest,(UINTa)(UINTa)val);
}
static RADINLINE rrbool rrAtomicCASPointer(void* volatile * pDestVal, void* * pOldVal, void* newVal)
{
    return rrAtomicCAS_3264((UINTa volatile *)pDestVal,(UINTa *)pOldVal,(UINTa)(UINTa)newVal);
}
static RADINLINE void* rrAtomicExchangePointer(void* volatile * pDestVal, void* newVal)
{
    return (void*)(UINTa) rrAtomicExchange_3264((UINTa volatile *)pDestVal,(UINTa)(UINTa)newVal);
}
static RADINLINE void* rrAtomicCmpXchgPointer(void* volatile * pDestVal, void* newVal, void* compVal)
{
    return (void*)(UINTa) rrAtomicCmpXchg_3264((UINTa volatile *)(pDestVal),(UINTa)(UINTa)(newVal),(UINTa)(UINTa)(compVal));
}

#else

#define rrAtomicLoadAcquirePointer(ptr)                 (void*)(UINTa) rrAtomicLoadAcquire_3264((UINTa const volatile *)(ptr))
#define rrAtomicStoreReleasePointer(pDest,val)          rrAtomicStoreRelease_3264((UINTa volatile *)(pDest),(UINTa)(UINTa)(val))
#define rrAtomicCASPointer(pDestVal,pOldVal,newVal)     rrAtomicCAS_3264((UINTa volatile *)(pDestVal),(UINTa *)(pOldVal),(UINTa)(UINTa)(newVal))
#define rrAtomicExchangePointer(pDestVal,newVal)        (void*)(UINTa) rrAtomicExchange_3264((UINTa volatile *)(pDestVal),(UINTa)(UINTa)(newVal))
#define rrAtomicCmpXchgPointer(pDestVal,newVal,compVal) (void*)(UINTa) rrAtomicCmpXchg_3264((UINTa volatile *)(pDestVal),(UINTa)(UINTa)(newVal),(UINTa)(UINTa)(compVal))

#endif

RADDEFEND

// @cdep pre $rrAtomicsAddSource

#endif 
