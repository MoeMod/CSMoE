// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#include "rrAtomics.h"

/**

NOTE :

http://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html


these are much better than the old __sync builtins, which never clearly defined their memory semantics.


Some weirdness about them though : they always exist in the compiler, you will never get a compile failure.  Instead if they don't have impls on your platform, you will get a link failure.


The default -march is very conservative about not providing these (for example due to the early AMD chips that were not full x64), so you have to set a pretty recent -march.


I haven't done the full binary search, but I found that -march=core2 gives you the 128 bit atomics on x64.


Oh, they all map from something like
__atomic_add_fetch
to
__atomic_add_fetch_4
__atomic_add_fetch_8
__atomic_add_fetch_16


which you are free to call directly to disambiguate which one you wanted.

**/


/*************************************************************************************************/

extern "C"
{

void rrAtomicMemoryBarrierFull( void )
{
	__atomic_thread_fence(__ATOMIC_SEQ_CST);
}

U32 rrAtomicLoadAcquire32(U32 volatile const * ptr)
{
	RR_ASSERT_ALIGNED(ptr);
	
	return __atomic_load_n((U32 volatile *)ptr,__ATOMIC_ACQUIRE);
}

typedef U64 U64aligned __attribute__((aligned (8)));

U64 rrAtomicLoadAcquire64(U64 volatile const * ptr)
{
	RR_ASSERT_ALIGNED(ptr);

	U64 ret = __atomic_load_n((U64aligned volatile *)ptr,__ATOMIC_ACQUIRE);
		
	return ret;
}

// *ptr = val; // gaurantees earlier stores do not move afterthis
void rrAtomicStoreRelease32(U32 volatile * ptr,U32 val)
{
	RR_ASSERT_ALIGNED(ptr);
	
	__atomic_store_n(ptr,val,__ATOMIC_RELEASE);
}

void rrAtomicStoreRelease64(U64 volatile * ptr,U64 val)
{
	RR_ASSERT_ALIGNED(ptr);
		
	__atomic_store_n((U64aligned volatile * )ptr,val,__ATOMIC_RELEASE);
}


// Windows style CmpXchg 
U32 rrAtomicCmpXchg32(U32 volatile * pDestVal, U32 newVal, U32 compareVal)
{
	RR_ASSERT_ALIGNED(pDestVal);
	
	U32 old = compareVal;
	bool ok = __atomic_compare_exchange_n(pDestVal,&old,newVal,false,__ATOMIC_ACQ_REL,__ATOMIC_ACQUIRE);
	RR_UNUSED_VARIABLE(ok);
	RR_ASSERT( (old == compareVal) == ok );
	return old;
}

U64 rrAtomicCmpXchg64(U64 volatile * pDestVal, U64 newVal, U64 compareVal)
{
	RR_ASSERT_ALIGNED(pDestVal);
	
	U64 old = compareVal;
	bool ok = __atomic_compare_exchange_n((U64aligned volatile * )pDestVal,&old,newVal,false,__ATOMIC_ACQ_REL,__ATOMIC_ACQUIRE);
	RR_UNUSED_VARIABLE(ok);
	RR_ASSERT( (old == compareVal) == ok );
	return old;
}

// CAS like C++0x
// atomically { if ( *pDestVal == *pOldVal ) { *pDestVal = newVal; } else { *pOldVal = *pDestVal; } }
rrbool rrAtomicCAS32(U32 volatile * pDestVal, U32 * pOldVal, U32 newVal)
{
	RR_ASSERT_ALIGNED(pDestVal);
	RR_ASSERT_ALIGNED(pOldVal);
	
	bool ok = __atomic_compare_exchange_n(pDestVal,pOldVal,newVal,false,__ATOMIC_ACQ_REL,__ATOMIC_ACQUIRE);
	return ok;
}

rrbool rrAtomicCAS64(U64 volatile * pDestVal, U64 * pOldVal, U64 newVal)
{
	RR_ASSERT_ALIGNED(pDestVal);
	RR_ASSERT_ALIGNED(pOldVal);
	
	bool ok = __atomic_compare_exchange_n((U64aligned volatile * )pDestVal,pOldVal,newVal,false,__ATOMIC_ACQ_REL,__ATOMIC_ACQUIRE);
	return ok;
}

// atomically { *pOldVal = *pDestVal; *pDestVal = newVal; return *pOldVal; };
U32 rrAtomicExchange32(U32 volatile * pDestVal, U32 newVal)
{
	RR_ASSERT_ALIGNED(pDestVal);
	
	return __atomic_exchange_n(pDestVal,newVal,__ATOMIC_ACQ_REL);
}

U64 rrAtomicExchange64(U64 volatile * pDestVal, U64 newVal)
{
	RR_ASSERT_ALIGNED(pDestVal);
	
	return __atomic_exchange_n((U64aligned volatile * )pDestVal,newVal,__ATOMIC_ACQ_REL);
}

// atomically { old = *pDestVal; *pDestVal += incVal; return old; }
U32 rrAtomicAddExchange32(U32 volatile * pDestVal, S32 incVal)
{
	RR_ASSERT_ALIGNED(pDestVal);
	
	return __atomic_fetch_add(pDestVal,incVal,__ATOMIC_ACQ_REL);
}

U64 rrAtomicAddExchange64(U64 volatile * pDestVal, S64 incVal)
{
	RR_ASSERT_ALIGNED(pDestVal);
	
	return __atomic_fetch_add((U64aligned volatile * )pDestVal,incVal,__ATOMIC_ACQ_REL);
}

#ifdef __RADATOMIC128__

ATOMIC128 rrAtomicLoadAcquire128(ATOMIC128 const volatile * const ptr)
{
	RR_ASSERT_ALIGNED(ptr);
	
	return __atomic_load_n((ATOMIC128 volatile *)ptr,__ATOMIC_ACQUIRE);
}

void rrAtomicStoreRelease128(ATOMIC128 volatile * const ptr,ATOMIC128 val)
{
	RR_ASSERT_ALIGNED(ptr);

	__atomic_store_n(ptr,val,__ATOMIC_RELEASE);
}

rrbool rrAtomicCAS128(ATOMIC128 volatile * pDestVal, ATOMIC128 * pOldVal, ATOMIC128 const * const pNewVal)
{
	RR_ASSERT_ALIGNED(pDestVal);
	RR_ASSERT_ALIGNED(pOldVal);
	RR_ASSERT_ALIGNED(pNewVal);

	bool ok = __atomic_compare_exchange(pDestVal,pOldVal,(volatile ATOMIC128 *)pNewVal,false,__ATOMIC_ACQ_REL,__ATOMIC_ACQUIRE);
	return ok;
}

void rrAtomicExchange128(ATOMIC128 volatile * pDestVal, ATOMIC128 * pOldVal, ATOMIC128 const * const pNewVal)
{
	RR_ASSERT_ALIGNED(pDestVal);
	RR_ASSERT_ALIGNED(pOldVal);
	RR_ASSERT_ALIGNED(pNewVal);

	__atomic_exchange(pDestVal,(ATOMIC128 *)pNewVal,pOldVal,__ATOMIC_ACQ_REL);
}

#endif

}
