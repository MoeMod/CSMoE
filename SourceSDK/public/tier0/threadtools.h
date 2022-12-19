//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A collection of utility classes to simplify thread handling, and
//			as much as possible contain portability problems. Here avoiding 
//			including windows.h.
//
//=============================================================================

#ifndef THREADTOOLS_H
#define THREADTOOLS_H

#include "tier0/type_traits.h"

#include <limits.h>
#if defined( __arm__ ) || defined( __arm64__ ) || defined( __aarch64__ )
#include <atomic>
#endif

#include "tier0/platform.h"
#include "tier0/dbg.h"
#include "tier0/vcrmode.h"
#include "tier0/vprof_telemetry.h"

#ifdef PLATFORM_WINDOWS_PC
#include <intrin.h>
#endif

#ifdef POSIX
#include <pthread.h>
#include <errno.h>
#include <sched.h>
#define WAIT_OBJECT_0 0
#define WAIT_TIMEOUT 0x00000102
#define WAIT_FAILED -1
#define THREAD_PRIORITY_HIGHEST 2
#endif

#if defined( _WIN32 )
#pragma once
#pragma warning(push)
#pragma warning(disable:4251)
#endif

// #define THREAD_PROFILER 1

#ifndef _RETAIL
#define THREAD_MUTEX_TRACING_SUPPORTED
#if defined(_WIN32) && defined(_DEBUG)
#define THREAD_MUTEX_TRACING_ENABLED
#endif
#endif

#ifdef _WIN32
typedef void *HANDLE;
#endif

// Start thread running  - error if already running
enum ThreadPriorityEnum_t
{
#if defined( PLATFORM_PS3 )
	TP_PRIORITY_NORMAL  = 1001,
	TP_PRIORITY_HIGH = 100,
	TP_PRIORITY_LOW = 2001,
	TP_PRIORITY_DEFAULT = 1001
#error "Need PRIORITY_LOWEST/HIGHEST"
#elif defined( PLATFORM_LINUX )
    // We can use nice on Linux threads to change scheduling.
    // pthreads on Linux only allows priority setting on
    // real-time threads.
    // NOTE: Lower numbers are higher priority, thus the need
    // for TP_IS_PRIORITY_HIGHER.
	TP_PRIORITY_DEFAULT = 0,
	TP_PRIORITY_NORMAL = 0,
	TP_PRIORITY_HIGH = -10,
	TP_PRIORITY_LOW = 10,
	TP_PRIORITY_HIGHEST = -20,
	TP_PRIORITY_LOWEST = 19,
#else  // PLATFORM_PS3
	TP_PRIORITY_DEFAULT = 0,	//	THREAD_PRIORITY_NORMAL
	TP_PRIORITY_NORMAL = 0,	//	THREAD_PRIORITY_NORMAL
	TP_PRIORITY_HIGH = 1,	//	THREAD_PRIORITY_ABOVE_NORMAL
	TP_PRIORITY_LOW = -1,	//	THREAD_PRIORITY_BELOW_NORMAL
	TP_PRIORITY_HIGHEST = 2,	//	THREAD_PRIORITY_HIGHEST
	TP_PRIORITY_LOWEST = -2,	//	THREAD_PRIORITY_LOWEST 
#endif // PLATFORM_PS3
};

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

const unsigned TT_INFINITE = 0xffffffff;

#ifndef NO_THREAD_LOCAL

#ifndef THREAD_LOCAL
#ifdef _WIN32
#define THREAD_LOCAL __declspec(thread)
#elif POSIX
#define THREAD_LOCAL __thread
#endif
#endif

#endif // NO_THREAD_LOCAL

#ifdef PLATFORM_64BITS
typedef uint64 ThreadId_t;
#else
typedef uint32 ThreadId_t;
#endif

//-----------------------------------------------------------------------------
//
// Simple thread creation. Differs from VCR mode/CreateThread/_beginthreadex
// in that it accepts a standard C function rather than compiler specific one.
//
//-----------------------------------------------------------------------------
FORWARD_DECLARE_HANDLE( ThreadHandle_t );
typedef uintp (*ThreadFunc_t)( void *pParam );

PLATFORM_OVERLOAD ThreadHandle_t CreateSimpleThread( ThreadFunc_t, void *pParam, ThreadId_t *pID, unsigned stackSize = 0 );
PLATFORM_INTERFACE ThreadHandle_t CreateSimpleThread( ThreadFunc_t, void *pParam, unsigned stackSize = 0 );
PLATFORM_INTERFACE bool ReleaseThreadHandle( ThreadHandle_t );


//-----------------------------------------------------------------------------

PLATFORM_INTERFACE void ThreadSleep(unsigned duration = 0);
PLATFORM_INTERFACE ThreadId_t ThreadGetCurrentId();
PLATFORM_INTERFACE ThreadHandle_t ThreadGetCurrentHandle();
PLATFORM_INTERFACE int ThreadGetPriority( ThreadHandle_t hThread = NULL );
PLATFORM_INTERFACE bool ThreadSetPriority( ThreadHandle_t hThread, int priority );
inline		 bool ThreadSetPriority( int priority ) { return ThreadSetPriority( NULL, priority ); }
PLATFORM_INTERFACE bool ThreadInMainThread();
PLATFORM_INTERFACE void DeclareCurrentThreadIsMainThread();

// NOTE: ThreadedLoadLibraryFunc_t needs to return the sleep time in milliseconds or TT_INFINITE
typedef int (*ThreadedLoadLibraryFunc_t)(); 
PLATFORM_INTERFACE void SetThreadedLoadLibraryFunc( ThreadedLoadLibraryFunc_t func );
PLATFORM_INTERFACE ThreadedLoadLibraryFunc_t GetThreadedLoadLibraryFunc();

#if defined( _WIN32 ) && !defined( _WIN64 ) && !defined( _X360 )
extern "C" unsigned long __declspec(dllimport) __stdcall GetCurrentThreadId();
#define ThreadGetCurrentId GetCurrentThreadId
#endif

inline void ThreadPause()
{
#if defined( PLATFORM_WINDOWS_PC ) && ( defined( _M_IX86 ) || defined( _M_X64 ) )
	// Intrinsic for __asm pause; from <intrin.h>
	_mm_pause();
#elif defined( PLATFORM_WINDOWS_PC ) && ( defined( _M_ARM ) || defined( _M_ARM64 ) )
	__yield();
#elif POSIX && ( defined( __i386__ ) || defined( __x86_64__ ) )
	__asm __volatile( "pause" );
#elif defined( _X360 )
#elif defined(__arm__) || defined(__arm64__) || defined(__aarch64__)
	sched_yield();
#else
#error "implement me"
#endif
}

PLATFORM_INTERFACE bool ThreadJoin( ThreadHandle_t, unsigned timeout = TT_INFINITE );
// If you're not calling ThreadJoin, you need to call ThreadDetach so pthreads on Linux knows it can
//	free the memory for this thread. Otherwise you wind up leaking threads until you run out and
//	CreateSimpleThread() will fail.
PLATFORM_INTERFACE void ThreadDetach( ThreadHandle_t );

PLATFORM_INTERFACE void ThreadSetDebugName( ThreadId_t id, const char *pszName );
inline		 void ThreadSetDebugName( const char *pszName ) { ThreadSetDebugName( (ThreadId_t)-1, pszName ); }

PLATFORM_INTERFACE void ThreadSetAffinity( ThreadHandle_t hThread, int nAffinityMask );

//-----------------------------------------------------------------------------

enum ThreadWaitResult_t
{
	TW_FAILED = 0xffffffff, // WAIT_FAILED
	TW_TIMEOUT = 0x00000102, // WAIT_TIMEOUT
};

#ifdef _WIN32
PLATFORM_INTERFACE int ThreadWaitForObjects( int nEvents, const HANDLE *pHandles, bool bWaitAll = true, unsigned timeout = TT_INFINITE );
inline int ThreadWaitForObject( HANDLE handle, bool bWaitAll = true, unsigned timeout = TT_INFINITE ) { return ThreadWaitForObjects( 1, &handle, bWaitAll, timeout ); }
#endif

//-----------------------------------------------------------------------------
//
// Interlock methods. These perform very fast atomic thread
// safe operations. These are especially relevant in a multi-core setting.
//
//-----------------------------------------------------------------------------

#ifdef _WIN32
#define NOINLINE
#elif POSIX
#define NOINLINE __attribute__ ((noinline))
#endif

// ThreadMemoryBarrier is a fence/barrier sufficient for most uses. It prevents reads
// from moving past reads, and writes moving past writes. It is sufficient for
// read-acquire and write-release barriers. It is not a full barrier and it does
// not prevent reads from moving past writes -- that would require a full __sync()
// on PPC and is significantly more expensive.
#if defined( _X360 ) || defined( _PS3 )
	#define ThreadMemoryBarrier() __lwsync()

#elif defined(_MSC_VER)
	// Prevent compiler reordering across this barrier. This is
	// sufficient for most purposes on x86/x64.

	#if _MSC_VER < 1500
		// !KLUDGE! For VC 2005
		// http://connect.microsoft.com/VisualStudio/feedback/details/100051
		#pragma intrinsic(_ReadWriteBarrier)
	#endif
	#define ThreadMemoryBarrier() _ReadWriteBarrier()
#elif defined(GNUC)
	// Prevent compiler reordering across this barrier. This is
	// sufficient for most purposes on x86/x64.
	// http://preshing.com/20120625/memory-ordering-at-compile-time
	#define ThreadMemoryBarrier() asm volatile("" ::: "memory")
#else
	#error Every platform needs to define ThreadMemoryBarrier to at least prevent compiler reordering
#endif

#if defined(_WIN32) && !defined(_X360)
	#if ( _MSC_VER >= 1310 )
		#define USE_INTRINSIC_INTERLOCKED
	#endif
#endif

#ifdef USE_INTRINSIC_INTERLOCKED
extern "C"
{
	long __cdecl _InterlockedIncrement(volatile long*);
	long __cdecl _InterlockedDecrement(volatile long*);
	long __cdecl _InterlockedExchange(volatile long*, long);
	long __cdecl _InterlockedExchangeAdd(volatile long*, long);
	long __cdecl _InterlockedCompareExchange(volatile long*, long, long);
}

#pragma intrinsic( _InterlockedCompareExchange )
#pragma intrinsic( _InterlockedDecrement )
#pragma intrinsic( _InterlockedExchange )
#pragma intrinsic( _InterlockedExchangeAdd ) 
#pragma intrinsic( _InterlockedIncrement )

inline int32 ThreadInterlockedIncrement( int32 volatile *p )										{ Assert( (size_t)p % 4 == 0 ); return (int32)_InterlockedIncrement((long volatile*)p); }
inline int32 ThreadInterlockedDecrement( int32 volatile *p )										{ Assert( (size_t)p % 4 == 0 ); return (int32)_InterlockedDecrement( (long volatile *)p ); }
inline int32 ThreadInterlockedExchange( int32 volatile *p, int32 value )							{ Assert( (size_t)p % 4 == 0 ); return (int32)_InterlockedExchange( (long volatile *)p, (long)value ); }
inline int32 ThreadInterlockedExchangeAdd( int32 volatile *p, int32 value )						{ Assert( (size_t)p % 4 == 0 ); return (int32)_InterlockedExchangeAdd( (long volatile *)p, (long)value ); }
inline int32 ThreadInterlockedCompareExchange( int32 volatile *p, int32 value, int32 comperand )	{ Assert( (size_t)p % 4 == 0 ); return (int32)_InterlockedCompareExchange( (long volatile *)p, (long)value, (long)comperand ); }
inline bool ThreadInterlockedAssignIf( int32 volatile *p, int32 value, int32 comperand )			{ Assert( (size_t)p % 4 == 0 ); return ( _InterlockedCompareExchange( (long volatile *)p, (long)value, (long)comperand ) == (long)comperand ); }
#else
PLATFORM_INTERFACE int32 ThreadInterlockedIncrement( int32 volatile * );
PLATFORM_INTERFACE int32 ThreadInterlockedDecrement( int32 volatile * );
PLATFORM_INTERFACE int32 ThreadInterlockedExchange( int32 volatile *, int32 value );
PLATFORM_INTERFACE int32 ThreadInterlockedExchangeAdd( int32 volatile *, int32 value );
PLATFORM_INTERFACE int32 ThreadInterlockedCompareExchange( int32 volatile *, int32 value, int32 comperand );
PLATFORM_INTERFACE bool ThreadInterlockedAssignIf( int32 volatile *, int32 value, int32 comperand );
#endif

inline unsigned ThreadInterlockedExchangeSubtract( int32 volatile *p, int32 value )	{ return ThreadInterlockedExchangeAdd( (int32 volatile *)p, -value ); }

#if defined( USE_INTRINSIC_INTERLOCKED ) && !defined( PLATFORM_64BITS )
#define TIPTR()
inline void *ThreadInterlockedExchangePointer( void * volatile *p, void *value )							{ return (void *)ThreadInterlockedExchange( reinterpret_cast<intp volatile *>(p), reinterpret_cast<intp>(value) ); }
inline void *ThreadInterlockedCompareExchangePointer( void * volatile *p, void *value, void *comperand )	{ return (void *)ThreadInterlockedCompareExchange( reinterpret_cast<intp volatile *>(p), reinterpret_cast<intp>(value), reinterpret_cast<intp>(comperand) ); }
inline bool ThreadInterlockedAssignPointerIf( void * volatile *p, void *value, void *comperand )			{ return (ThreadInterlockedCompareExchange( reinterpret_cast<intp volatile *>(p), reinterpret_cast<intp>(value), reinterpret_cast<intp>(comperand) ) == reinterpret_cast<intp>(comperand) ); }
#else
PLATFORM_INTERFACE void *ThreadInterlockedExchangePointer( void * volatile *, void *value ) NOINLINE;
PLATFORM_INTERFACE void *ThreadInterlockedCompareExchangePointer( void * volatile *, void *value, void *comperand ) NOINLINE;
PLATFORM_INTERFACE bool ThreadInterlockedAssignPointerIf( void * volatile *, void *value, void *comperand ) NOINLINE;
#endif

inline void const *ThreadInterlockedExchangePointerToConst( void const * volatile *p, void const *value )							{ return ThreadInterlockedExchangePointer( const_cast < void * volatile * > ( p ), const_cast < void * > ( value ) );  }
inline void const *ThreadInterlockedCompareExchangePointerToConst( void const * volatile *p, void const *value, void const *comperand )	{ return ThreadInterlockedCompareExchangePointer( const_cast < void * volatile * > ( p ), const_cast < void * > ( value ), const_cast < void * > ( comperand ) ); }
inline bool ThreadInterlockedAssignPointerToConstIf( void const * volatile *p, void const *value, void const *comperand )			{ return ThreadInterlockedAssignPointerIf( const_cast < void * volatile * > ( p ), const_cast < void * > ( value ), const_cast < void * > ( comperand ) ); }

#if defined( PLATFORM_64BITS )
#if defined (_WIN32) 
#if defined( _M_ARM ) || defined( _M_ARM64 )
typedef int32x4_t int128;
inline int128 int128_zero() { return vdupq_n_s32(0); }
#else
typedef __m128i int128;
inline int128 int128_zero()	{ return _mm_setzero_si128(); }
#endif
#else
typedef __int128_t int128;
#define int128_zero() int128()
#endif

PLATFORM_INTERFACE bool ThreadInterlockedAssignIf128( volatile int128 *pDest, const int128 &value, const int128 &comperand ) NOINLINE;

#endif

PLATFORM_INTERFACE int64 ThreadInterlockedIncrement64( int64 volatile * ) NOINLINE;
PLATFORM_INTERFACE int64 ThreadInterlockedDecrement64( int64 volatile * ) NOINLINE;
PLATFORM_INTERFACE int64 ThreadInterlockedCompareExchange64( int64 volatile *, int64 value, int64 comperand ) NOINLINE;
PLATFORM_INTERFACE int64 ThreadInterlockedExchange64( int64 volatile *, int64 value ) NOINLINE;
PLATFORM_INTERFACE int64 ThreadInterlockedExchangeAdd64( int64 volatile *, int64 value ) NOINLINE;
PLATFORM_INTERFACE bool ThreadInterlockedAssignIf64(volatile int64 *pDest, int64 value, int64 comperand ) NOINLINE;

inline uint32 ThreadInterlockedExchangeSubtract( uint32 volatile *p, uint32 value )	{ return ThreadInterlockedExchangeAdd( (int32 volatile *)p, value ); }
inline uint32 ThreadInterlockedIncrement( uint32 volatile *p )	{ return ThreadInterlockedIncrement( (int32 volatile *)p ); }
inline uint32 ThreadInterlockedDecrement( uint32 volatile *p )	{ return ThreadInterlockedDecrement( (int32 volatile *)p ); }
inline uint32 ThreadInterlockedExchange( uint32 volatile *p, uint32 value )	{ return ThreadInterlockedExchange( (int32 volatile *)p, value ); }
inline uint32 ThreadInterlockedExchangeAdd( uint32 volatile *p, uint32 value )	{ return ThreadInterlockedExchangeAdd( (int32 volatile *)p, value ); }
inline uint32 ThreadInterlockedCompareExchange( uint32 volatile *p, uint32 value, uint32 comperand )	{ return ThreadInterlockedCompareExchange( (int32 volatile *)p, value, comperand ); }
inline bool ThreadInterlockedAssignIf( uint32 volatile *p, uint32 value, uint32 comperand )	{ return ThreadInterlockedAssignIf( (int32 volatile *)p, value, comperand ); }

inline uint64 ThreadInterlockedIncrement64( uint64 volatile *p )	{ return ThreadInterlockedIncrement64( (int64 volatile *)p ); }
inline uint64 ThreadInterlockedDecrement64( uint64 volatile *p )	{ return ThreadInterlockedDecrement64( (int64 volatile *)p ); }
inline uint64 ThreadInterlockedCompareExchange64( uint64 volatile *p, uint64 value, uint64 comperand )	{ return ThreadInterlockedCompareExchange64( (int64 volatile *)p, value, comperand ); }
inline uint64 ThreadInterlockedExchange64( uint64 volatile *p, uint64 value )	{ return ThreadInterlockedExchange64( (int64 volatile *)p, value ); }
inline uint64 ThreadInterlockedExchangeAdd64( uint64 volatile *p, uint64 value )	{ return ThreadInterlockedExchangeAdd64( (int64 volatile *)p, value ); }
inline bool ThreadInterlockedAssignIf64( uint64 volatile *p, uint64 value, uint64 comperand )	{ return ThreadInterlockedAssignIf64( (int64 volatile *)p, value, comperand ); }

//inline int ThreadInterlockedExchangeSubtract( int volatile *p, int value )	{ return ThreadInterlockedExchangeAdd( (int32 volatile *)p, value ); }
//inline int ThreadInterlockedIncrement( int volatile *p )	{ return ThreadInterlockedIncrement( (int32 volatile *)p ); }
//inline int ThreadInterlockedDecrement( int volatile *p )	{ return ThreadInterlockedDecrement( (int32 volatile *)p ); }
//inline int ThreadInterlockedExchange( int volatile *p, int value )	{ return ThreadInterlockedExchange( (int32 volatile *)p, value ); }
//inline int ThreadInterlockedExchangeAdd( int volatile *p, int value )	{ return ThreadInterlockedExchangeAdd( (int32 volatile *)p, value ); }
//inline int ThreadInterlockedCompareExchange( int volatile *p, int value, int comperand )	{ return ThreadInterlockedCompareExchange( (int32 volatile *)p, value, comperand ); }
//inline bool ThreadInterlockedAssignIf( int volatile *p, int value, int comperand )	{ return ThreadInterlockedAssignIf( (int32 volatile *)p, value, comperand ); }

//-----------------------------------------------------------------------------
// Access to VTune thread profiling
//-----------------------------------------------------------------------------
#if defined(_WIN32) && defined(THREAD_PROFILER)
PLATFORM_INTERFACE void ThreadNotifySyncPrepare(void *p);
PLATFORM_INTERFACE void ThreadNotifySyncCancel(void *p);
PLATFORM_INTERFACE void ThreadNotifySyncAcquired(void *p);
PLATFORM_INTERFACE void ThreadNotifySyncReleasing(void *p);
#else
#define ThreadNotifySyncPrepare(p)		((void)0)
#define ThreadNotifySyncCancel(p)		((void)0)
#define ThreadNotifySyncAcquired(p)		((void)0)
#define ThreadNotifySyncReleasing(p)	((void)0)
#endif

//-----------------------------------------------------------------------------
// Encapsulation of a thread local datum (needed because THREAD_LOCAL doesn't
// work in a DLL loaded with LoadLibrary()
//-----------------------------------------------------------------------------

#ifndef NO_THREAD_LOCAL

#if defined(_LINUX) && !defined(OSX)
// linux totally supports compiler thread locals, even across dll's.
#define PLAT_COMPILER_SUPPORTED_THREADLOCALS 1
#define CTHREADLOCALINTEGER( typ ) __thread int
#define CTHREADLOCALINT __thread int
#define CTHREADLOCALPTR( typ ) __thread typ *
#define CTHREADLOCAL( typ ) __thread typ
#define GETLOCAL( x ) ( x )
#endif // _LINUX && !OSX

#if defined(WIN32) || defined(OSX)
#ifndef __AFXTLS_H__ // not compatible with some Windows headers
#define CTHREADLOCALINT CThreadLocalInt<int>
#define CTHREADLOCALINTEGER( typ ) CThreadLocalInt<typ>
#define CTHREADLOCALPTR( typ ) CThreadLocalPtr<typ>
#define CTHREADLOCAL( typ ) CThreadLocal<typ>
#define GETLOCAL( x ) ( x.Get() )
#endif
#endif // WIN32 || OSX

#endif // NO_THREAD_LOCALS

#ifndef __AFXTLS_H__ // not compatible with some Windows headers
#ifndef NO_THREAD_LOCAL

class PLATFORM_CLASS CThreadLocalBase
	{
public:
		CThreadLocalBase();
		~CThreadLocalBase();

		void * Get() const;
		void   Set(void *);

private:
#ifdef _WIN32
	uint32 m_index;
#elif POSIX
		pthread_key_t m_index;
#endif
	};

	//---------------------------------------------------------

#ifndef __AFXTLS_H__

	template <class T>
	class CThreadLocal : public CThreadLocalBase
	{
	public:
		CThreadLocal()
		{
#ifdef PLATFORM_64BITS
            COMPILE_TIME_ASSERT( sizeof(T) <= sizeof(void *) );
#else
            COMPILE_TIME_ASSERT( sizeof(T) == sizeof(void *) );
#endif
		}

		T Get() const
		{
#ifdef PLATFORM_64BITS
            void *pData = CThreadLocalBase::Get();
            return *reinterpret_cast<T*>( &pData );
#else
    #ifdef COMPILER_MSVC
		#pragma warning ( disable : 4311 )
	#endif
			return reinterpret_cast<T>( CThreadLocalBase::Get() );
	#ifdef COMPILER_MSVC
		#pragma warning ( default : 4311 )
	#endif
#endif
		}

		void Set(T val)
		{
#ifdef PLATFORM_64BITS
            void* pData = 0;
            *reinterpret_cast<T*>( &pData ) = val;
            CThreadLocalBase::Set( pData );
#else
    #ifdef COMPILER_MSVC
		#pragma warning ( disable : 4312 )
	#endif
			CThreadLocalBase::Set( reinterpret_cast<void *>(val) );
	#ifdef COMPILER_MSVC
		#pragma warning ( default : 4312 )
	#endif
#endif
		}
	};

#endif

	//---------------------------------------------------------

template <class T = intp>
	class CThreadLocalInt : public CThreadLocal<T>
	{
	public:
	CThreadLocalInt()
	{
		COMPILE_TIME_ASSERT( sizeof(T) >= sizeof(int) );
	}

	operator int() const { return (int)this->Get(); }
	int	operator=( int i ) { this->Set( (intp)i ); return i; }

	int operator++()					{ T i = this->Get(); this->Set( ++i ); return (int)i; }
	int operator++(int)				{ T i = this->Get(); this->Set( i + 1 ); return (int)i; }

	int operator--()					{ T i = this->Get(); this->Set( --i ); return (int)i; }
	int operator--(int)				{ T i = this->Get(); this->Set( i - 1 ); return (int)i; }
	};


	//---------------------------------------------------------

	template <class T>
	class CThreadLocalPtr : private CThreadLocalBase
	{
	public:
		CThreadLocalPtr() {}

	operator const void *() const          					{ return (T *)Get(); }
		operator void *()                      					{ return (T *)Get(); }

	operator const T *() const							    { return (T *)Get(); }
	operator const T *()          							{ return (T *)Get(); }
		operator T *()											{ return (T *)Get(); }

		T *			operator=( T *p )							{ Set( p ); return p; }

		bool        operator !() const							{ return (!Get()); }
		bool		operator==( const void *p ) const			{ return (Get() == p); }
		bool		operator!=( const void *p ) const			{ return (Get() != p); }

		T *  		operator->()								{ return (T *)Get(); }
		T &  		operator *()								{ return *((T *)Get()); }

	const T *   operator->() const							{ return (T *)Get(); }
	const T &   operator *() const							{ return *((T *)Get()); }

	const T &	operator[]( int i ) const					{ return *((T *)Get() + i); }
		T &			operator[]( int i )							{ return *((T *)Get() + i); }

	private:
		// Disallowed operations
		CThreadLocalPtr( T *pFrom );
		CThreadLocalPtr( const CThreadLocalPtr<T> &from );
		T **operator &();
		T * const *operator &() const;
		void operator=( const CThreadLocalPtr<T> &from );
		bool operator==( const CThreadLocalPtr<T> &p ) const;
		bool operator!=( const CThreadLocalPtr<T> &p ) const;
	};

#endif // NO_THREAD_LOCAL
#endif // !__AFXTLS_H__

//-----------------------------------------------------------------------------
//
// A super-fast thread-safe integer A simple class encapsulating the notion of an 
// atomic integer used across threads that uses the built in and faster 
// "interlocked" functionality rather than a full-blown mutex. Useful for simple 
// things like reference counts, etc.
//
//-----------------------------------------------------------------------------

template <typename T>
class CInterlockedIntT
{
public:
	CInterlockedIntT() : m_value( 0 ) 				{ COMPILE_TIME_ASSERT( sizeof(T) == sizeof(int) ); }
	CInterlockedIntT( T value ) : m_value( value ) 	{}

	T GetRaw() const				{ return m_value; }

	operator T() const				{ return m_value; }

	bool operator!() const			{ return ( m_value == 0 ); }
	bool operator==( T rhs ) const	{ return ( m_value == rhs ); }
	bool operator!=( T rhs ) const	{ return ( m_value != rhs ); }


#if defined( __arm__ ) || defined( __arm64__ ) || defined( __aarch64__ )
    CInterlockedIntT( const CInterlockedIntT &rhs ) : m_value( rhs ) 	{}
    CInterlockedIntT &operator=( const CInterlockedIntT &rhs ) { m_value.store(rhs.m_value.load()); return *this; }
	T operator++()					{ return m_value.fetch_add(1) + 1; }
	T operator++(int)				{ return m_value.fetch_add(1); }

	T operator--()					{ return m_value.fetch_sub(1) - 1; }
	T operator--(int)				{ return m_value.fetch_sub(1); }

	bool AssignIf( T conditionValue, T newValue )	{ return m_value.compare_exchange_strong(conditionValue, newValue); }

	T operator=( T newValue )		{ m_value.store(newValue); return newValue; }

	void operator+=( T add )		{ m_value.fetch_add(add); }
#else
    T operator++()					{ return (T)ThreadInterlockedIncrement( (int32 *)&m_value ); }
	T operator++(int)				{ return operator++() - 1; }

	T operator--()					{ return (T)ThreadInterlockedDecrement( (int32 *)&m_value ); }
	T operator--(int)				{ return operator--() + 1; }

	bool AssignIf( T conditionValue, T newValue )	{ return ThreadInterlockedAssignIf( (int32 *)&m_value, (int32)newValue, (int32)conditionValue ); }

	T operator=( T newValue )		{ ThreadInterlockedExchange((int32 *)&m_value, newValue); return m_value; }

	void operator+=( T add )		{ ThreadInterlockedExchangeAdd( (int32 *)&m_value, (int32)add ); }
#endif
	void operator-=( T subtract )	{ operator+=( -subtract ); }
	void operator*=( T multiplier )	{ 
		T original, result; 
		do 
		{ 
			original = m_value; 
			result = original * multiplier; 
		} while ( !AssignIf( original, result ) );
	}
	void operator/=( T divisor )	{ 
		T original, result; 
		do 
		{ 
			original = m_value; 
			result = original / divisor;
		} while ( !AssignIf( original, result ) );
	}

	T operator+( T rhs ) const		{ return m_value + rhs; }
	T operator-( T rhs ) const		{ return m_value - rhs; }

private:
#if defined( __arm__ ) || defined( __arm64__ ) || defined( __aarch64__ )
    std::atomic<T> m_value;
#else
	volatile T m_value;
#endif
};

typedef CInterlockedIntT<int> CInterlockedInt;
typedef CInterlockedIntT<unsigned> CInterlockedUInt;

//-----------------------------------------------------------------------------

template <typename T>
class CInterlockedPtr
{
public:
	CInterlockedPtr() : m_value( 0 ) 				{}
	CInterlockedPtr( T *value ) : m_value( value ) 	{}

	operator T *() const			{ return m_value; }

	bool operator!() const			{ return ( m_value == 0 ); }
	bool operator==( T *rhs ) const	{ return ( m_value == rhs ); }
	bool operator!=( T *rhs ) const	{ return ( m_value != rhs ); }
#if defined( __arm__ ) || defined( __arm64__ ) || defined( __aarch64__ )
    CInterlockedPtr( const CInterlockedPtr &rhs ) : m_value( rhs ) 	{}
    CInterlockedPtr &operator=( const CInterlockedPtr &rhs ) { m_value.store(rhs.m_value.load()); return *this; }
    T *operator++()					{ return m_value.fetch_add(1) + 1; }
    T *operator++(int)				{ return m_value.fetch_add(1); }

    T *operator--()					{ return m_value.fetch_sub(1) - 1; }
    T *operator--(int)				{ return m_value.fetch_sub(1); }

    bool AssignIf( T *conditionValue, T *newValue )	{ return m_value.compare_exchange_strong(conditionValue, newValue); }

    T *operator=( T *newValue )		{ m_value.store(newValue); return newValue; }

    void operator+=( int add )		{ m_value.fetch_add(add); }
#else
#if defined( PLATFORM_64BITS )
	T *operator++()					{ return ((T *)ThreadInterlockedExchangeAdd64( (int64 *)&m_value, sizeof(T) )) + 1; }
	T *operator++(int)				{ return (T *)ThreadInterlockedExchangeAdd64( (int64 *)&m_value, sizeof(T) ); }

	T *operator--()					{ return ((T *)ThreadInterlockedExchangeAdd64( (int64 *)&m_value, -sizeof(T) )) - 1; }
	T *operator--(int)				{ return (T *)ThreadInterlockedExchangeAdd64( (int64 *)&m_value, -sizeof(T) ); }

	bool AssignIf( T *conditionValue, T *newValue )	{ return ThreadInterlockedAssignPointerToConstIf( (void const **) &m_value, (void const *) newValue, (void const *) conditionValue ); }

	T *operator=( T *newValue )		{ ThreadInterlockedExchangePointerToConst( (void const **) &m_value, (void const *) newValue ); return newValue; }

	void operator+=( int add )		{ ThreadInterlockedExchangeAdd64( (int64 *)&m_value, add * sizeof(T) ); }
#else
	T *operator++()					{ return ((T *)ThreadInterlockedExchangeAdd( (intp*)&m_value, sizeof(T) )) + 1; }
	T *operator++(int)				{ return (T *)ThreadInterlockedExchangeAdd( (intp*)&m_value, sizeof(T) ); }

	T *operator--()					{ return ((T *)ThreadInterlockedExchangeAdd( (intp*)&m_value, -sizeof(T) )) - 1; }
	T *operator--(int)				{ return (T *)ThreadInterlockedExchangeAdd( (intp*)&m_value, -sizeof(T) ); }

	bool AssignIf( T *conditionValue, T *newValue )	{ return ThreadInterlockedAssignPointerToConstIf( (void const **) &m_value, (void const *) newValue, (void const *) conditionValue ); }

	T *operator=( T *newValue )		{ ThreadInterlockedExchangePointerToConst( (void const **) &m_value, (void const *) newValue ); return newValue; }

	void operator+=( int add )		{ ThreadInterlockedExchangeAdd( (intp *)&m_value, (intp)(add * sizeof(T)) ); }
#endif
#endif

	void operator-=( int subtract )	{ operator+=( -subtract ); }

	T *operator+( int rhs ) const		{ return m_value + rhs; }
	T *operator-( int rhs ) const		{ return m_value - rhs; }
	T *operator+( unsigned rhs ) const	{ return m_value + rhs; }
	T *operator-( unsigned rhs ) const	{ return m_value - rhs; }
	size_t operator-( T *p ) const		{ return m_value - p; }
	size_t operator-( const CInterlockedPtr<T> &p ) const	{ return m_value - p.m_value; }

private:
#if defined( __arm__ ) || defined( __arm64__ ) || defined( __aarch64__ )
    std::atomic<T *> m_value;
#else
	T * volatile m_value;
#endif
};

//-----------------------------------------------------------------------------
// 
// Platform independent verification that multiple threads aren't getting into the same code at the same time. 
// Note: This is intended for use to identify problems, it doesn't provide any sort of thread safety.
// 
//-----------------------------------------------------------------------------
class ReentrancyVerifier
{
public:
	inline ReentrancyVerifier(CInterlockedInt* counter, int sleepTimeMS)
	: mCounter(counter)
	{
		Assert(mCounter != NULL);

		if (++(*mCounter) != 1) {
			DebuggerBreakIfDebugging_StagingOnly();
		}

		if (sleepTimeMS > 0)
		{
			ThreadSleep(sleepTimeMS);
		}
	}

	inline ~ReentrancyVerifier()
	{
		if (--(*mCounter) != 0) {
			DebuggerBreakIfDebugging_StagingOnly();
		}
	}

private:
	CInterlockedInt* mCounter;
};


//-----------------------------------------------------------------------------
//
// Platform independent for critical sections management
//
//-----------------------------------------------------------------------------

class PLATFORM_CLASS CThreadMutex
{
public:
	CThreadMutex();
	~CThreadMutex();

	//------------------------------------------------------
	// Mutex acquisition/release. Const intentionally defeated.
	//------------------------------------------------------
	void Lock();
	void Lock() const		{ (const_cast<CThreadMutex *>(this))->Lock(); }
	void Unlock();
	void Unlock() const		{ (const_cast<CThreadMutex *>(this))->Unlock(); }

	bool TryLock();
	bool TryLock() const	{ return (const_cast<CThreadMutex *>(this))->TryLock(); }

	//------------------------------------------------------
	// Use this to make deadlocks easier to track by asserting
	// when it is expected that the current thread owns the mutex
	//------------------------------------------------------
	bool AssertOwnedByCurrentThread();

	//------------------------------------------------------
	// Enable tracing to track deadlock problems
	//------------------------------------------------------
	void SetTrace( bool );

private:
	// Disallow copying
	CThreadMutex( const CThreadMutex & );
	CThreadMutex &operator=( const CThreadMutex & );

#if defined( _WIN32 )
	// Efficient solution to breaking the windows.h dependency, invariant is tested.
#ifdef _WIN64
	#define TT_SIZEOF_CRITICALSECTION 40	
#else
#ifndef _X360
	#define TT_SIZEOF_CRITICALSECTION 24
#else
	#define TT_SIZEOF_CRITICALSECTION 28
#endif // !_XBOX
#endif // _WIN64
	byte m_CriticalSection[TT_SIZEOF_CRITICALSECTION];
#elif defined(POSIX)
	pthread_mutex_t m_Mutex;
	pthread_mutexattr_t m_Attr;
#else
#error
#endif

#ifdef THREAD_MUTEX_TRACING_SUPPORTED
	// Debugging (always here to allow mixed debug/release builds w/o changing size)
	uint	m_currentOwnerID;
	uint16	m_lockCount;
	bool	m_bTrace;
#endif
};

//-----------------------------------------------------------------------------
//
// An alternative mutex that is useful for cases when thread contention is 
// rare, but a mutex is required. Instances should be declared volatile.
// Sleep of 0 may not be sufficient to keep high priority threads from starving 
// lesser threads. This class is not a suitable replacement for a critical
// section if the resource contention is high.
//
//-----------------------------------------------------------------------------

#if !defined(THREAD_PROFILER)

class CThreadFastMutex
{
public:
	CThreadFastMutex()
	  :	m_ownerID( 0 ),
	  	m_depth( 0 )
	{
	}

private:
	FORCEINLINE bool TryLockInline( const uintp threadId ) volatile
	{
#if PLATFORM_64BITS
        if ( threadId != m_ownerID && !ThreadInterlockedAssignIf64( &m_ownerID, threadId, 0 ) )
#else
		if ( threadId != m_ownerID && !ThreadInterlockedAssignIf( &m_ownerID, threadId, 0 ) )
#endif
			return false;

		ThreadMemoryBarrier();
		++m_depth;
		return true;
	}

	bool TryLock( const uintp threadId ) volatile
	{
		return TryLockInline( threadId );
	}

	PLATFORM_CLASS void Lock( const uintp threadId, unsigned nSpinSleepTime ) volatile;

public:
	bool TryLock() volatile
	{
#ifdef _DEBUG
		if ( m_depth == INT_MAX )
			DebuggerBreak();

		if ( m_depth < 0 )
			DebuggerBreak();
#endif
		return TryLockInline( ThreadGetCurrentId() );
	}

#ifndef _DEBUG 
	FORCEINLINE 
#endif
	void Lock( unsigned int nSpinSleepTime = 0 ) volatile
	{
		const uintp threadId = ThreadGetCurrentId();

		if ( !TryLockInline( threadId ) )
		{
			ThreadPause();
			Lock( threadId, nSpinSleepTime );
		}
#ifdef _DEBUG
		if ( m_ownerID != ThreadGetCurrentId() )
			DebuggerBreak();

		if ( m_depth == INT_MAX )
			DebuggerBreak();

		if ( m_depth < 0 )
			DebuggerBreak();
#endif
	}

#ifndef _DEBUG
	FORCEINLINE 
#endif
	void Unlock() volatile
	{
#ifdef _DEBUG
		if ( m_ownerID != ThreadGetCurrentId() )
			DebuggerBreak();

		if ( m_depth <= 0 )
			DebuggerBreak();
#endif

		--m_depth;
		if ( !m_depth )
		{
			ThreadMemoryBarrier();
#if PLATFORM_64BITS
            ThreadInterlockedExchange64( &m_ownerID, 0 );
#else
            ThreadInterlockedExchange( &m_ownerID, 0 );
#endif
    	}
    }

#ifdef WIN32
	bool TryLock() const volatile							{ return (const_cast<CThreadFastMutex *>(this))->TryLock(); }
	void Lock(unsigned nSpinSleepTime = 1 ) const volatile	{ (const_cast<CThreadFastMutex *>(this))->Lock( nSpinSleepTime ); }
	void Unlock() const	volatile							{ (const_cast<CThreadFastMutex *>(this))->Unlock(); }
#endif
	// To match regular CThreadMutex:
	bool AssertOwnedByCurrentThread()	{ return true; }
	void SetTrace( bool )				{}

    uintp GetOwnerId() const			{ return m_ownerID;	}
	int	GetDepth() const				{ return m_depth; }
private:
	volatile uintp m_ownerID;
	int				m_depth;
};

#ifdef COMPILER_CLANG
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wunused-private-field"
#endif // Q_CC_CLANG

class ALIGN128 CAlignedThreadFastMutex : public CThreadFastMutex
{
public:
	CAlignedThreadFastMutex()
	{
		Assert( (size_t)this % 128 == 0 && sizeof(*this) == 128 );
	}

private:
	uint8 pad[128-sizeof(CThreadFastMutex)];
} ALIGN128_POST;

#ifdef COMPILER_CLANG
#  pragma clang diagnostic pop
#endif

#else
typedef CThreadMutex CThreadFastMutex;
#endif

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

class CThreadNullMutex
{
public:
	static void Lock()				{}
	static void Unlock()			{}

	static bool TryLock()			{ return true; }
	static bool AssertOwnedByCurrentThread() { return true; }
	static void SetTrace( bool b )	{}

	static uintp GetOwnerId() 		{ return 0;	}
	static int	GetDepth() 			{ return 0; }
};

//-----------------------------------------------------------------------------
//
// A mutex decorator class used to control the use of a mutex, to make it
// less expensive when not multithreading
//
//-----------------------------------------------------------------------------

template <class BaseClass, bool *pCondition>
class CThreadConditionalMutex : public BaseClass
{
public:
	void Lock()				{ if ( *pCondition ) BaseClass::Lock(); }
	void Lock() const 		{ if ( *pCondition ) BaseClass::Lock(); }
	void Unlock()			{ if ( *pCondition ) BaseClass::Unlock(); }
	void Unlock() const		{ if ( *pCondition ) BaseClass::Unlock(); }

	bool TryLock()			{ if ( *pCondition ) return BaseClass::TryLock(); else return true; }
	bool TryLock() const 	{ if ( *pCondition ) return BaseClass::TryLock(); else return true; }
	bool AssertOwnedByCurrentThread() { if ( *pCondition ) return BaseClass::AssertOwnedByCurrentThread(); else return true; }
	void SetTrace( bool b ) { if ( *pCondition ) BaseClass::SetTrace( b ); }
};

//-----------------------------------------------------------------------------
// Mutex decorator that blows up if another thread enters
//-----------------------------------------------------------------------------

template <class BaseClass>
class CThreadTerminalMutex : public BaseClass
{
public:
	bool TryLock()			{ if ( !BaseClass::TryLock() ) { DebuggerBreak(); return false; } return true; }
	bool TryLock() const 	{ if ( !BaseClass::TryLock() ) { DebuggerBreak(); return false; } return true; }
	void Lock()				{ if ( !TryLock() ) BaseClass::Lock(); }
	void Lock() const 		{ if ( !TryLock() ) BaseClass::Lock(); }

};

//-----------------------------------------------------------------------------
//
// Class to Lock a critical section, and unlock it automatically
// when the lock goes out of scope
//
//-----------------------------------------------------------------------------

template <class MUTEX_TYPE = CThreadMutex>
class CAutoLockT
{
public:
	FORCEINLINE CAutoLockT( MUTEX_TYPE &lock, const char* pMutexName, const char* pFilename, int nLineNum, uint64 minReportDurationUs )
	: m_lock( const_cast< typename V_remove_const< MUTEX_TYPE >::type & >( lock ) )
	, m_pMutexName( pMutexName )
	, m_pFilename( pFilename )
	, m_nLineNum( nLineNum )
	, m_bOwned( true )
	{
		tmTryLockEx( TELEMETRY_LEVEL0, &m_uLockMatcher, minReportDurationUs, pFilename, nLineNum, &m_lock, pMutexName );
		m_lock.Lock();
		tmEndTryLockEx( TELEMETRY_LEVEL0, m_uLockMatcher, pFilename, nLineNum, &m_lock, TMLR_SUCCESS );
		tmSetLockStateEx( TELEMETRY_LEVEL0, pFilename, nLineNum, &m_lock, TMLS_LOCKED, pMutexName );
	}

	FORCEINLINE CAutoLockT<MUTEX_TYPE>( CAutoLockT<MUTEX_TYPE> && rhs )
	: m_lock( const_cast< typename V_remove_const< MUTEX_TYPE >::type &>( rhs.m_lock ) )
	{
		m_pMutexName = rhs.m_pMutexName;
		m_pFilename = rhs.m_pFilename;
		m_nLineNum = rhs.m_nLineNum;
		#ifdef RAD_TELEMETRY_ENABLED
			m_uLockMatcher = rhs.m_uLockMatcher;
		#endif
		m_bOwned = true;
		rhs.m_bOwned = false;
	}

	FORCEINLINE ~CAutoLockT()
	{
		if ( m_bOwned ) 
		{
			m_lock.Unlock();
			tmSetLockStateEx( TELEMETRY_LEVEL0, m_pFilename, m_nLineNum, &m_lock, TMLS_RELEASED, m_pMutexName );
		}
	}

private:
	typename V_remove_const< MUTEX_TYPE >::type &m_lock;
	const char* m_pMutexName;
	const char* m_pFilename;
	int m_nLineNum;
	bool m_bOwned;	 // Did owenership of the lock pass to another instance?

#ifdef RAD_TELEMETRY_ENABLED
	TmU64 m_uLockMatcher;
#endif

	// Disallow copying
	CAutoLockT<MUTEX_TYPE>( const CAutoLockT<MUTEX_TYPE> & );
	CAutoLockT<MUTEX_TYPE> &operator=( const CAutoLockT<MUTEX_TYPE> & );

	// No move assignment because no default construction.
	CAutoLockT<MUTEX_TYPE> &operator=( CAutoLockT<MUTEX_TYPE> && );
};

typedef CAutoLockT<CThreadMutex> CAutoLock;

template < typename MUTEX_TYPE >
inline CAutoLockT<MUTEX_TYPE> make_auto_lock( MUTEX_TYPE& lock, const char* pMutexname, const char* pFilename, int nLineNum, int nMinReportDurationUs = 1 )
{
	return CAutoLockT<MUTEX_TYPE>( lock, pMutexname, pFilename, nLineNum, nMinReportDurationUs );
}

//---------------------------------------------------------

#define AUTO_LOCK( mutex ) \
	auto UNIQUE_ID = make_auto_lock( mutex, #mutex, __FILE__, __LINE__ );

#define AUTO_LOCK_D( mutex, minDurationUs ) \
	auto UNIQUE_ID = make_auto_lock( mutex, #mutex, __FILE__, __LINE__, minDurationUs );

#define LOCAL_THREAD_LOCK_( tag ) \
	; \
	static CThreadFastMutex autoMutex_##tag; \
	AUTO_LOCK( autoMutex_##tag )

#define LOCAL_THREAD_LOCK() \
	LOCAL_THREAD_LOCK_(_)

//-----------------------------------------------------------------------------
//
// Base class for event, semaphore and mutex objects.
//
//-----------------------------------------------------------------------------

class PLATFORM_CLASS CThreadSyncObject
{
public:
	~CThreadSyncObject();

	//-----------------------------------------------------
	// Query if object is useful
	//-----------------------------------------------------
	bool operator!() const;

	//-----------------------------------------------------
	// Access handle
	//-----------------------------------------------------
#ifdef _WIN32
	operator HANDLE() { return GetHandle(); }
	const HANDLE GetHandle() const { return m_hSyncObject; }
#endif
	//-----------------------------------------------------
	// Wait for a signal from the object
	//-----------------------------------------------------
	bool Wait( uint32 dwTimeout = TT_INFINITE );

protected:
	CThreadSyncObject();
	void AssertUseable();

#ifdef _WIN32
	HANDLE m_hSyncObject;
	bool m_bCreatedHandle;
#elif defined(POSIX)
	pthread_mutex_t	m_Mutex;
	pthread_cond_t	m_Condition;
	bool m_bInitalized;
	int m_cSet;
	bool m_bManualReset;
	bool m_bWakeForEvent;
#else
#error "Implement me"
#endif

private:
	CThreadSyncObject( const CThreadSyncObject & );
	CThreadSyncObject &operator=( const CThreadSyncObject & );
};


//-----------------------------------------------------------------------------
//
// Wrapper for unnamed event objects
//
//-----------------------------------------------------------------------------

#if defined( _WIN32 )

//-----------------------------------------------------------------------------
//
// CThreadSemaphore
//
//-----------------------------------------------------------------------------

class PLATFORM_CLASS CThreadSemaphore : public CThreadSyncObject
{
public:
	CThreadSemaphore(long initialValue, long maxValue);

	//-----------------------------------------------------
	// Increases the count of the semaphore object by a specified
	// amount.  Wait() decreases the count by one on return.
	//-----------------------------------------------------
	bool Release(long releaseCount = 1, long * pPreviousCount = NULL );

private:
	CThreadSemaphore(const CThreadSemaphore &);
	CThreadSemaphore &operator=(const CThreadSemaphore &);
};


//-----------------------------------------------------------------------------
//
// A mutex suitable for out-of-process, multi-processor usage
//
//-----------------------------------------------------------------------------

class PLATFORM_CLASS CThreadFullMutex : public CThreadSyncObject
{
public:
	CThreadFullMutex( bool bEstablishInitialOwnership = false, const char * pszName = NULL );

	//-----------------------------------------------------
	// Release ownership of the mutex
	//-----------------------------------------------------
	bool Release();

	// To match regular CThreadMutex:
	void Lock()							{ Wait(); }
	void Lock( unsigned timeout )		{ Wait( timeout ); }
	void Unlock()						{ Release(); }
	bool AssertOwnedByCurrentThread()	{ return true; }
	void SetTrace( bool )				{}

private:
	CThreadFullMutex( const CThreadFullMutex & );
	CThreadFullMutex &operator=( const CThreadFullMutex & );
};
#endif


class PLATFORM_CLASS CThreadEvent : public CThreadSyncObject
{
public:
	CThreadEvent( bool fManualReset = false );
#ifdef WIN32
	CThreadEvent( HANDLE hHandle );
#endif
	//-----------------------------------------------------
	// Set the state to signaled
	//-----------------------------------------------------
	bool Set();

	//-----------------------------------------------------
	// Set the state to nonsignaled
	//-----------------------------------------------------
	bool Reset();

	//-----------------------------------------------------
	// Check if the event is signaled
	//-----------------------------------------------------
	bool Check();

	bool Wait( uint32 dwTimeout = TT_INFINITE );

private:
	CThreadEvent( const CThreadEvent & );
	CThreadEvent &operator=( const CThreadEvent & );
};

// Hard-wired manual event for use in array declarations
class CThreadManualEvent : public CThreadEvent
{
public:
	CThreadManualEvent()
	 :	CThreadEvent( true )
	{
	}
};

inline int ThreadWaitForEvents( int nEvents, CThreadEvent * const *pEvents, bool bWaitAll = true, unsigned timeout = TT_INFINITE )
{
#ifdef POSIX
  Assert( nEvents == 1);
  if ( pEvents[0]->Wait( timeout ) )
	  return WAIT_OBJECT_0;
  else
	return WAIT_TIMEOUT;
#else
	HANDLE handles[64];
	for ( int i = 0; i < min( nEvents, (int)ARRAYSIZE(handles) ); i++ )
		handles[i] = pEvents[i]->GetHandle();
	return ThreadWaitForObjects( nEvents, handles, bWaitAll, timeout );
#endif
}

//-----------------------------------------------------------------------------
//
// CThreadRWLock
//
//-----------------------------------------------------------------------------

class PLATFORM_CLASS CThreadRWLock
{
public:
	CThreadRWLock();

	void LockForRead();
	void UnlockRead();
	void LockForWrite();
	void UnlockWrite();

	void LockForRead() const { const_cast<CThreadRWLock *>(this)->LockForRead(); }
	void UnlockRead() const { const_cast<CThreadRWLock *>(this)->UnlockRead(); }
	void LockForWrite() const { const_cast<CThreadRWLock *>(this)->LockForWrite(); }
	void UnlockWrite() const { const_cast<CThreadRWLock *>(this)->UnlockWrite(); }

private:
	void WaitForRead();

#ifdef WIN32
	CThreadFastMutex m_mutex;
#else
	CThreadMutex m_mutex;	
#endif
	CThreadEvent m_CanWrite;
	CThreadEvent m_CanRead;

	int m_nWriters;
	int m_nActiveReaders;
	int m_nPendingReaders;
};

//-----------------------------------------------------------------------------
//
// CThreadSpinRWLock
//
//-----------------------------------------------------------------------------

class ALIGN8 PLATFORM_CLASS CThreadSpinRWLock
{
public:
	CThreadSpinRWLock()	{ Assert( (intp)this % 8 == 0 ); memset( this, 0, sizeof( *this ) ); }

	bool TryLockForWrite();
	bool TryLockForRead();

	void LockForRead();
	void UnlockRead();
	void LockForWrite();
	void UnlockWrite();

	bool TryLockForWrite() const { return const_cast<CThreadSpinRWLock *>(this)->TryLockForWrite(); }
	bool TryLockForRead() const { return const_cast<CThreadSpinRWLock *>(this)->TryLockForRead(); }
	void LockForRead() const { const_cast<CThreadSpinRWLock *>(this)->LockForRead(); }
	void UnlockRead() const { const_cast<CThreadSpinRWLock *>(this)->UnlockRead(); }
	void LockForWrite() const { const_cast<CThreadSpinRWLock *>(this)->LockForWrite(); }
	void UnlockWrite() const { const_cast<CThreadSpinRWLock *>(this)->UnlockWrite(); }

private:
	struct LockInfo_t
		{
		union {
#if PLATFORM_64BITS
			struct {
				uint64	m_writerId;
				int64	m_nReaders;
			};
			int128 i;
			// MoeMod: not actually need 64bit, just fill padding here, and fix strict aliasing bug
#else
			struct {
				uintp	m_writerId;
				int		m_nReaders;
			};
			int64 i;
#endif
		};
		};

	bool AssignIf( const LockInfo_t &newValue, const LockInfo_t &comperand );
	bool TryLockForWrite( const uintp threadId );
	void SpinLockForWrite( const uintp threadId );

	volatile LockInfo_t m_lockInfo;
	CInterlockedInt m_nWriters;
} ALIGN8_POST;

//-----------------------------------------------------------------------------
//
// A thread wrapper similar to a Java thread.
//
//-----------------------------------------------------------------------------

class PLATFORM_CLASS CThread
{
public:
	CThread();
	virtual ~CThread();

	//-----------------------------------------------------

	const char *GetName();
	void SetName( const char * );

	size_t CalcStackDepth( void *pStackVariable )		{ return ((byte *)m_pStackBase - (byte *)pStackVariable); }

	//-----------------------------------------------------
	// Functions for the other threads
	//-----------------------------------------------------

	// Start thread running  - error if already running
	virtual bool Start( unsigned nBytesStack = 0 );

	// Returns true if thread has been created and hasn't yet exited
	bool IsAlive();

	// This method causes the current thread to wait until this thread
	// is no longer alive.
	bool Join( unsigned timeout = TT_INFINITE );

#ifdef _WIN32
	// Access the thread handle directly
	HANDLE GetThreadHandle();
	uint GetThreadId();
#elif defined( LINUX )
	uint GetThreadId();
#endif

	//-----------------------------------------------------

	int GetResult();

	//-----------------------------------------------------
	// Functions for both this, and maybe, and other threads
	//-----------------------------------------------------

	// Forcibly, abnormally, but relatively cleanly stop the thread
	void Stop( int exitCode = 0 );

	// Get the priority
	int GetPriority() const;

	// Set the priority
	bool SetPriority( int );

	// Request a thread to suspend, this must ONLY be called from the thread itself, not the main thread
	// This suspend variant causes the thread in question to suspend at a known point in its execution
	// which means you don't risk the global deadlocks/hangs potentially caused by the raw Suspend() call
	void SuspendCooperative();

	// Resume a previously suspended thread from the Cooperative call
	void ResumeCooperative();

	// wait for a thread to execute its SuspendCooperative call 
	void BWaitForThreadSuspendCooperative();

#ifndef LINUX
	// forcefully Suspend a thread
	unsigned int Suspend();

	// forcefully Resume a previously suspended thread
	unsigned int Resume();
#endif

	// Force hard-termination of thread.  Used for critical failures.
	bool Terminate( int exitCode = 0 );

	//-----------------------------------------------------
	// Global methods
	//-----------------------------------------------------

	// Get the Thread object that represents the current thread, if any.
	// Can return NULL if the current thread was not created using
	// CThread
	static CThread *GetCurrentCThread();

	// Offer a context switch. Under Win32, equivalent to Sleep(0)
#ifdef Yield
#undef Yield
#endif
	static void Yield();

	// This method causes the current thread to yield and not to be
	// scheduled for further execution until a certain amount of real
	// time has elapsed, more or less.
	static void Sleep( unsigned duration );

protected:

	// Optional pre-run call, with ability to fail-create. Note Init()
	// is forced synchronous with Start()
	virtual bool Init();

	// Thread will run this function on startup, must be supplied by
	// derived class, performs the intended action of the thread.
	virtual int Run() = 0;

	// Called when the thread is about to exit, by the about-to-exit thread.
	virtual void OnExit();

	// Called after OnExit when a thread finishes or is killed. Not virtual because no inherited classes
	// override it and we don't want to change the vtable from the published SDK version.
	void Cleanup();

	bool WaitForCreateComplete( CThreadEvent *pEvent );

	// "Virtual static" facility
	typedef unsigned (__stdcall *ThreadProc_t)( void * );
	virtual ThreadProc_t GetThreadProc();
	virtual bool IsThreadRunning();

	CThreadMutex m_Lock;

#ifdef WIN32
	ThreadHandle_t GetThreadID() const { return (ThreadHandle_t)m_hThread; }
#else
	ThreadId_t GetThreadID() const { return (ThreadId_t)m_threadId; }
#endif

private:
	enum Flags
	{
		SUPPORT_STOP_PROTOCOL = 1 << 0
	};

	// Thread initially runs this. param is actually 'this'. function
	// just gets this and calls ThreadProc
	struct ThreadInit_t
	{
		CThread *     pThread;
		CThreadEvent *pInitCompleteEvent;
		bool *        pfInitSuccess;
	};

	static unsigned __stdcall ThreadProc( void * pv );

	// make copy constructor and assignment operator inaccessible
	CThread( const CThread & );
	CThread &operator=( const CThread & );

#ifdef _WIN32
	HANDLE 	m_hThread;
	ThreadId_t m_threadId;
#elif defined(POSIX)
	pthread_t m_threadId;
#endif
	CInterlockedInt m_nSuspendCount;
	CThreadEvent m_SuspendEvent;
	CThreadEvent m_SuspendEventSignal;
	int		m_result;
	char	m_szName[32];
	void *	m_pStackBase;
	unsigned m_flags;
};

//-----------------------------------------------------------------------------
//
// A helper class to let you sleep a thread for memory validation, you need to handle
//	 m_bSleepForValidate in your ::Run() call and set m_bSleepingForValidate when sleeping
//
//-----------------------------------------------------------------------------
class PLATFORM_CLASS CValidatableThread : public CThread
{
public:
	CValidatableThread()
	{
		m_bSleepForValidate = false;
		m_bSleepingForValidate = false;
	}

#ifdef DBGFLAG_VALIDATE
	virtual void SleepForValidate() { m_bSleepForValidate = true; }
	bool BSleepingForValidate() { return m_bSleepingForValidate; }
	virtual void WakeFromValidate() { m_bSleepForValidate = false; }
#endif
protected:
	bool m_bSleepForValidate;
	bool m_bSleepingForValidate;
};

//-----------------------------------------------------------------------------
// Simple thread class encompasses the notion of a worker thread, handing
// synchronized communication.
//-----------------------------------------------------------------------------


// These are internal reserved error results from a call attempt
enum WTCallResult_t
{
	WTCR_FAIL			= -1,
	WTCR_TIMEOUT		= -2,
	WTCR_THREAD_GONE	= -3,
};

class CFunctor;
class PLATFORM_CLASS CWorkerThread : public CThread
{
public:
	CWorkerThread();

	//-----------------------------------------------------
	//
	// Inter-thread communication
	//
	// Calls in either direction take place on the same "channel."
	// Seperate functions are specified to make identities obvious
	//
	//-----------------------------------------------------

	// Master: Signal the thread, and block for a response
	int CallWorker( unsigned, unsigned timeout = TT_INFINITE, bool fBoostWorkerPriorityToMaster = true, CFunctor *pParamFunctor = NULL );

	// Worker: Signal the thread, and block for a response
	int CallMaster( unsigned, unsigned timeout = TT_INFINITE );

	// Wait for the next request
	bool WaitForCall( unsigned dwTimeout, unsigned *pResult = NULL );
	bool WaitForCall( unsigned *pResult = NULL );

	// Is there a request?
	bool PeekCall( unsigned *pParam = NULL, CFunctor **ppParamFunctor = NULL );

	// Reply to the request
	void Reply( unsigned );

	// Wait for a reply in the case when CallWorker() with timeout != TT_INFINITE
	int WaitForReply( unsigned timeout = TT_INFINITE );

	// If you want to do WaitForMultipleObjects you'll need to include
	// this handle in your wait list or you won't be responsive
	CThreadEvent &GetCallHandle();
	// Find out what the request was
	unsigned GetCallParam( CFunctor **ppParamFunctor = NULL ) const;

	// Boost the worker thread to the master thread, if worker thread is lesser, return old priority
	int BoostPriority();

protected:
#ifndef _WIN32
#define __stdcall
#endif
	typedef uint32 (__stdcall *WaitFunc_t)( int nEvents, CThreadEvent * const *pEvents, int bWaitAll, uint32 timeout );
	
	int Call( unsigned, unsigned timeout, bool fBoost, WaitFunc_t = NULL, CFunctor *pParamFunctor = NULL );
	int WaitForReply( unsigned timeout, WaitFunc_t );

private:
	CWorkerThread( const CWorkerThread & );
	CWorkerThread &operator=( const CWorkerThread & );

	CThreadEvent	m_EventSend;
	CThreadEvent	m_EventComplete;

	unsigned        m_Param;
	CFunctor		*m_pParamFunctor;
	int				m_ReturnVal;
};


// a unidirectional message queue. A queue of type T. Not especially high speed since each message
// is malloced/freed. Note that if your message class has destructors/constructors, they MUST be
// thread safe!
template<class T> class CMessageQueue
{
	CThreadEvent SignalEvent;								// signals presence of data
	CThreadMutex QueueAccessMutex;

	// the parts protected by the mutex
	struct MsgNode
	{
		MsgNode *Next;
		T Data;
	};

	MsgNode *Head;
	MsgNode *Tail;

public:
	CMessageQueue( void )
	{
		Head = Tail = NULL;
	}

	// check for a message. not 100% reliable - someone could grab the message first
	bool MessageWaiting( void ) 
	{
		return ( Head != NULL );
	}

	void WaitMessage( T *pMsg )
	{
		for(;;)
		{
			while( ! MessageWaiting() )
				SignalEvent.Wait();
			QueueAccessMutex.Lock();
			if (! Head )
			{
				// multiple readers could make this null
				QueueAccessMutex.Unlock();
				continue;
			}
			*( pMsg ) = Head->Data;
			MsgNode *remove_this = Head;
			Head = Head->Next;
			if (! Head)										// if empty, fix tail ptr
				Tail = NULL;
			QueueAccessMutex.Unlock();
			delete remove_this;
			break;
		}
	}

	void QueueMessage( T const &Msg)
	{
		MsgNode *new1=new MsgNode;
		new1->Data=Msg;
		new1->Next=NULL;
		QueueAccessMutex.Lock();
		if ( Tail )
		{
			Tail->Next=new1;
			Tail = new1;
		}
		else
		{
			Head = new1;
			Tail = new1;
		}
		SignalEvent.Set();
		QueueAccessMutex.Unlock();
	}
};


//-----------------------------------------------------------------------------
//
// CThreadMutex. Inlining to reduce overhead and to allow client code
// to decide debug status (tracing)
//
//-----------------------------------------------------------------------------

#ifdef _WIN32
typedef struct _RTL_CRITICAL_SECTION RTL_CRITICAL_SECTION;
typedef RTL_CRITICAL_SECTION CRITICAL_SECTION;

#ifndef _X360
extern "C"
{
	void __declspec(dllimport) __stdcall InitializeCriticalSection(CRITICAL_SECTION *);
	void __declspec(dllimport) __stdcall EnterCriticalSection(CRITICAL_SECTION *);
	void __declspec(dllimport) __stdcall LeaveCriticalSection(CRITICAL_SECTION *);
	void __declspec(dllimport) __stdcall DeleteCriticalSection(CRITICAL_SECTION *);
};
#endif

//---------------------------------------------------------

inline void CThreadMutex::Lock()
{
#ifdef THREAD_MUTEX_TRACING_ENABLED
		uint thisThreadID = ThreadGetCurrentId();
		if ( m_bTrace && m_currentOwnerID && ( m_currentOwnerID != thisThreadID ) )
		Msg( "Thread %u about to wait for lock %p owned by %u\n", ThreadGetCurrentId(), (CRITICAL_SECTION *)&m_CriticalSection, m_currentOwnerID );
	#endif

	VCRHook_EnterCriticalSection((CRITICAL_SECTION *)&m_CriticalSection);

	#ifdef THREAD_MUTEX_TRACING_ENABLED
		if (m_lockCount == 0)
		{
			// we now own it for the first time.  Set owner information
			m_currentOwnerID = thisThreadID;
			if ( m_bTrace )
			Msg( "Thread %u now owns lock %p\n", m_currentOwnerID, (CRITICAL_SECTION *)&m_CriticalSection );
		}
		m_lockCount++;
	#endif
}

//---------------------------------------------------------

inline void CThreadMutex::Unlock()
{
	#ifdef THREAD_MUTEX_TRACING_ENABLED
		AssertMsg( m_lockCount >= 1, "Invalid unlock of thread lock" );
		m_lockCount--;
		if (m_lockCount == 0)
		{
			if ( m_bTrace )
			Msg( "Thread %u releasing lock %p\n", m_currentOwnerID, (CRITICAL_SECTION *)&m_CriticalSection );
			m_currentOwnerID = 0;
		}
	#endif
	LeaveCriticalSection((CRITICAL_SECTION *)&m_CriticalSection);
}

//---------------------------------------------------------

inline bool CThreadMutex::AssertOwnedByCurrentThread()
{
#ifdef THREAD_MUTEX_TRACING_ENABLED
	if (ThreadGetCurrentId() == m_currentOwnerID)
		return true;
	AssertMsg3( 0, "Expected thread %u as owner of lock %p, but %u owns", ThreadGetCurrentId(), (CRITICAL_SECTION *)&m_CriticalSection, m_currentOwnerID );
	return false;
#else
	return true;
#endif
}

//---------------------------------------------------------

inline void CThreadMutex::SetTrace( bool bTrace )
{
#ifdef THREAD_MUTEX_TRACING_ENABLED
	m_bTrace = bTrace;
#endif
}

//---------------------------------------------------------

#elif defined(POSIX)

inline CThreadMutex::CThreadMutex()
{
	// enable recursive locks as we need them
	pthread_mutexattr_init( &m_Attr );
	pthread_mutexattr_settype( &m_Attr, PTHREAD_MUTEX_RECURSIVE );
	pthread_mutex_init( &m_Mutex, &m_Attr );
}

//---------------------------------------------------------

inline CThreadMutex::~CThreadMutex()
{
	pthread_mutex_destroy( &m_Mutex );
}

//---------------------------------------------------------

inline void CThreadMutex::Lock()
{
	pthread_mutex_lock( &m_Mutex );
}

//---------------------------------------------------------

inline void CThreadMutex::Unlock()
{
	pthread_mutex_unlock( &m_Mutex );
}

//---------------------------------------------------------

inline bool CThreadMutex::AssertOwnedByCurrentThread()
{
	return true;
}

//---------------------------------------------------------

inline void CThreadMutex::SetTrace(bool fTrace)
{
}

#endif // POSIX

//-----------------------------------------------------------------------------
//
// CThreadRWLock inline functions
//
//-----------------------------------------------------------------------------

inline CThreadRWLock::CThreadRWLock()
:	m_CanRead( true ),
	m_nWriters( 0 ),
	m_nActiveReaders( 0 ),
	m_nPendingReaders( 0 )
{
}

inline void CThreadRWLock::LockForRead()
{
	m_mutex.Lock();
	if ( m_nWriters)
	{
		WaitForRead();
	}
	m_nActiveReaders++;
	m_mutex.Unlock();
}

inline void CThreadRWLock::UnlockRead()
{
	m_mutex.Lock();
	m_nActiveReaders--;
	if ( m_nActiveReaders == 0 && m_nWriters != 0 )
	{
		m_CanWrite.Set();
	}
	m_mutex.Unlock();
}


//-----------------------------------------------------------------------------
//
// CThreadSpinRWLock inline functions
//
//-----------------------------------------------------------------------------

inline bool CThreadSpinRWLock::AssignIf( const LockInfo_t &newValue, const LockInfo_t &comperand )
{
#if PLATFORM_64BITS
    COMPILE_TIME_ASSERT(sizeof(LockInfo_t) == 16);
	return ThreadInterlockedAssignIf128( (int128 *)&m_lockInfo, *((int128 *)&newValue), *((int128 *)&comperand) );
#else
    COMPILE_TIME_ASSERT(sizeof(LockInfo_t) == 8);
	return ThreadInterlockedAssignIf64( (int64 *)&m_lockInfo, *((int64 *)&newValue), *((int64 *)&comperand) );
#endif
}

inline bool CThreadSpinRWLock::TryLockForWrite( const uintp threadId )
{
	// In order to grab a write lock, there can be no readers and no owners of the write lock
	if ( m_lockInfo.m_nReaders > 0 || ( m_lockInfo.m_writerId && m_lockInfo.m_writerId != threadId ) )
	{
		return false;
	}

	static const LockInfo_t oldValue = { 0, 0 };
	LockInfo_t newValue = { threadId, 0 };
	const bool bSuccess = AssignIf( newValue, oldValue );
#if defined(_X360)
	if ( bSuccess )
	{
		// X360TBD: Serious perf implications. Not Yet. __sync();
	}
#endif
	return bSuccess;
}

inline bool CThreadSpinRWLock::TryLockForWrite()
{
	m_nWriters++;
	if ( !TryLockForWrite( ThreadGetCurrentId() ) )
	{
		m_nWriters--;
		return false;
	}
	return true;
}

inline bool CThreadSpinRWLock::TryLockForRead()
{
	if ( m_nWriters != 0 )
	{
		return false;
	}
	// In order to grab a write lock, the number of readers must not change and no thread can own the write
	LockInfo_t oldValue;
	LockInfo_t newValue;

		oldValue.m_nReaders = m_lockInfo.m_nReaders;
		oldValue.m_writerId = 0;
		newValue.m_nReaders = oldValue.m_nReaders + 1;
		newValue.m_writerId = 0;

	const bool bSuccess = AssignIf( newValue, oldValue );
#if defined(_X360)
	if ( bSuccess )
	{
		// X360TBD: Serious perf implications. Not Yet. __sync();
	}
#endif
	return bSuccess;
}

inline void CThreadSpinRWLock::LockForWrite()
{
	const uintp threadId = ThreadGetCurrentId();

	m_nWriters++;

	if ( !TryLockForWrite( threadId ) )
	{
		ThreadPause();
		SpinLockForWrite( threadId );
	}
}

// read data from a memory address
template<class T> FORCEINLINE T ReadVolatileMemory( T const *pPtr )
{
	volatile const T * pVolatilePtr = ( volatile const T * ) pPtr;
	return *pVolatilePtr;
}

//-----------------------------------------------------------------------------

#if defined( _WIN32 )
#pragma warning(pop)
#endif

#endif // THREADTOOLS_H
