#pragma once

#include <new>

typedef struct mempool_s mempool_t;

//
// zone.c
//
#ifdef XASH_MEMDBG

#define Mem_Alloc( pool, size ) LLM_FUNCTION_MANGLE(Mem_Alloc)( pool, size LLM_EXTRA_ARG_GEN )
#define Mem_ZeroAlloc( pool, size ) LLM_FUNCTION_MANGLE(Mem_ZeroAlloc)( pool, size LLM_EXTRA_ARG_GEN )
#define Mem_AlignedAlloc( pool, size, alignment ) LLM_FUNCTION_MANGLE(Mem_AlignedAlloc)( pool, size, alignment LLM_EXTRA_ARG_GEN )
#define Mem_Realloc( pool, ptr, size ) LLM_FUNCTION_MANGLE(Mem_Realloc)( pool, ptr, size LLM_EXTRA_ARG_GEN )
#define Mem_Free( mem ) LLM_FUNCTION_MANGLE(Mem_Free)( mem LLM_EXTRA_ARG_GEN )
#define Mem_AllocPool( name ) LLM_FUNCTION_MANGLE(Mem_AllocPool)( name LLM_EXTRA_ARG_GEN )
#define Mem_AllocSubPool( backend, name ) LLM_FUNCTION_MANGLE(Mem_AllocSubPool)( backend, name LLM_EXTRA_ARG_GEN )
#define Mem_FreePool( pool ) LLM_FUNCTION_MANGLE(Mem_FreePool)( pool LLM_EXTRA_ARG_GEN )
#define Mem_EmptyPool( pool ) LLM_FUNCTION_MANGLE(Mem_EmptyPool)( pool LLM_EXTRA_ARG_GEN )

#define LLM_FUNCTION_MANGLE(f) _##f
#define LLM_EXTRA_ARG_DEF , const char *filename, int fileline
#define LLM_EXTRA_ARG_FWD , filename, fileline
#define LLM_EXTRA_ARG_GEN , __FILE__, __LINE__
#else
#define LLM_FUNCTION_MANGLE(f) f
#define LLM_EXTRA_ARG_DEF
#define LLM_EXTRA_ARG_FWD
#define LLM_EXTRA_ARG_GEN
#endif

void *LLM_FUNCTION_MANGLE(Mem_Realloc)( mempool_t *poolptr, void *memptr, size_t size LLM_EXTRA_ARG_DEF );
void *LLM_FUNCTION_MANGLE(Mem_Alloc)( mempool_t *poolptr, size_t size LLM_EXTRA_ARG_DEF );
void *LLM_FUNCTION_MANGLE(Mem_ZeroAlloc)( mempool_t *poolptr, size_t size LLM_EXTRA_ARG_DEF );
void *LLM_FUNCTION_MANGLE(Mem_AlignedAlloc)( mempool_t *poolptr, size_t size, size_t alignment LLM_EXTRA_ARG_DEF );
mempool_t *LLM_FUNCTION_MANGLE(Mem_AllocPool)( const char *name LLM_EXTRA_ARG_DEF );
mempool_t *LLM_FUNCTION_MANGLE(Mem_AllocSubPool)( mempool_t *backend, const char *name LLM_EXTRA_ARG_DEF );
void LLM_FUNCTION_MANGLE(Mem_FreePool)( mempool_t **poolptr LLM_EXTRA_ARG_DEF );
void LLM_FUNCTION_MANGLE(Mem_EmptyPool)( mempool_t *poolptr LLM_EXTRA_ARG_DEF );
void LLM_FUNCTION_MANGLE(Mem_Free)( void *data LLM_EXTRA_ARG_DEF );

#ifdef XASH_OPERATOR_NEW_DELETE
template<class T, class...Args> T *LLM_FUNCTION_MANGLE(Mem_New)(mempool_t *pool, Args &&...args)
{
    void *res;
    if constexpr(alignof(T) > alignof(std::max_align_t))
        res = LLM_FUNCTION_MANGLE(Mem_AlignedAlloc)( pool, sizeof(T), alignof(T) LLM_EXTRA_ARG_GEN );
    else
        res = LLM_FUNCTION_MANGLE(Mem_Alloc)( pool, sizeof(T) LLM_EXTRA_ARG_GEN );
    return ::new(res) T(std::forward<Args>(args)...);
}

template<class T> void Mem_Delete(T *p)
{
    if(p)
    {
        p->~T();
        LLM_FUNCTION_MANGLE(Mem_Free)(p);
    }
}
#endif

// better memcpy for size >= 16K
void *Mem_VirtualCopy(void *dest, const void *src, size_t size);
#ifdef XASH_OPERATOR_NEW_DELETE
void *operator new(std::size_t size, mempool_t *poolptr);
void *operator new[](std::size_t size, mempool_t *poolptr);
void operator delete(void *ptr, mempool_t *poolptr);
void operator delete[](void *ptr, mempool_t *poolptr);

void* operator new(std::size_t count);
void operator delete(void* ptr) noexcept;
#endif