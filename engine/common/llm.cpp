/*
zone.c - zone memory allocation from DarkPlaces
Copyright (C) 2007 Uncle Mike

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "common.h"

#include <boost/pool/pool.hpp>
#include <boost/container/pmr/global_resource.hpp>
#include <boost/container/pmr/synchronized_pool_resource.hpp>
#include <boost/container/pmr/unsynchronized_pool_resource.hpp>
#include <boost/container/pmr/monotonic_buffer_resource.hpp>

#ifdef XASH_TBBMALLOC
#include <tbb/scalable_allocator.h>
#endif

// MoeMod : there is no std::pmr on ios, so we are using the boost one
namespace pmr = boost::container::pmr;

#define MEMHEADER_SENTINEL1	0xDEADF00D
#define MEMHEADER_SENTINEL2	0xDF

class IMemPool
{
public:
    virtual ~IMemPool() = default;
    virtual void *Alloc(size_t size, size_t align =  alignof(std::max_align_t)) = 0;
    virtual void Free(void *ptr) = 0;
    virtual void Empty() = 0;
};

struct MemPoolConfig
{
    std::string name;
#ifdef XASH_MEMDBG
    const char *filename;	// file name and line where Mem_AllocPool was called
    int fileline;
#endif
};

typedef struct mempool_s : IMemPool {} mempool_t;

#ifdef XASH_TBBMALLOC
class TBBMemPool : public mempool_t, public MemPoolConfig
{
public:
    TBBMemPool(const char *name LLM_EXTRA_ARG_DEF) : MemPoolConfig{ name LLM_EXTRA_ARG_FWD }, tbbpool(nullptr)
    {
        rml::MemPoolPolicy policy(raw_malloc, raw_free);
        auto error = pool_create_v1((intptr_t)this, &policy, &this->tbbpool);
        if(error != rml::POOL_OK)
        {
            throw std::bad_alloc();
        }
    }
    ~TBBMemPool() override
    {
        pool_destroy(tbbpool);
    }

    void *Alloc(size_t size, size_t align) override
    {
        return pool_aligned_malloc(tbbpool, size, align);
    }

    void Free(void *ptr) override
    {
        pool_free(tbbpool, ptr);
    }

    void Empty() override
    {
        pool_reset(tbbpool);
    }

private:
    static void *raw_malloc(std::intptr_t pool_id, std::size_t &bytes)
    {
        //auto that = reinterpret_cast<mempool_t *>(pool_id);
        return scalable_aligned_malloc(bytes, alignof(std::max_align_t));
    }

    static int raw_free(std::intptr_t pool_id, void* raw_ptr, std::size_t raw_bytes)
    {
        //auto that = reinterpret_cast<mempool_t *>(pool_id);
        scalable_free(raw_ptr);
        return 0;
    }

    rml::MemoryPool *tbbpool;
};
#endif

class MemPoolRes : public pmr::memory_resource
{
public:
    MemPoolRes(mempool_t *bk) : backendpool(bk) {}

    void* do_allocate(std::size_t size, std::size_t align) override
    {
        return backendpool->Alloc(size, align);
    }
    void do_deallocate(void* ptr, std::size_t bytes, std::size_t alignment) override
    {
        return backendpool->Free(ptr);
    }
    bool do_is_equal(const memory_resource& other) const noexcept override
    {
        return this == &other;
    }

    mempool_t *backendpool;
};

template<class T = pmr::unsynchronized_pool_resource>
class PmrMemPool : public mempool_t, public MemPoolConfig
{
public:
    PmrMemPool(mempool_t *bk, const char *name LLM_EXTRA_ARG_DEF) :
        MemPoolConfig{ name LLM_EXTRA_ARG_FWD },
        memres(bk),
        pmrpool(pmr::pool_options{}, &memres)
    {

    }

    ~PmrMemPool() override
    {

    }

    void *Alloc(size_t size, size_t align) override
    {
        return pmrpool.allocate(size, align);
    }

    void Free(void *ptr) override
    {
        pmrpool.deallocate(ptr, 0, 0);
    }

    void Empty() override
    {
        pmrpool.release();
    }

private:
    MemPoolRes memres;
    T pmrpool;
};

class CrtMemPool : public mempool_t
{
public:
    CrtMemPool()
    {

    }

    void *Alloc(size_t size, size_t align) override
    {
        return ::operator new(size, (std::align_val_t)align);
    }

    void Free(void *ptr) override
    {
        ::operator delete(ptr);
    }

    void Empty() override
    {

    }
} g_CrtMemPool;

typedef struct alignas(std::max_align_t) memheader_s {
    mempool_t *pool;
    size_t size;
#ifdef XASH_MEMDBG
    const char *filename;
    int fileline;
    uint		sentinel1;	// should always be MEMHEADER_SENTINEL1
#endif
} memheader_t;

#ifdef XASH_MEMDBG
#define MEMDBG_ALLOC_FUNC_BEGIN try
#define MEMDBG_ALLOC_FUNC_END catch (...) \
{ \
    Sys_Error( "Mem_AllocPool: out of memory (allocpool at %s:%i)\n", filename, fileline ); \
    return nullptr; \
}
#else
#define MEMDBG_ALLOC_FUNC_BEGIN
#define MEMDBG_ALLOC_FUNC_END
#endif

mempool_t *LLM_FUNCTION_MANGLE(Mem_AllocPool)( const char *name LLM_EXTRA_ARG_DEF )
MEMDBG_ALLOC_FUNC_BEGIN
{
#ifdef XASH_TBBMALLOC
    return new TBBMemPool( name LLM_EXTRA_ARG_FWD );
#else
    return new PmrMemPool<pmr::synchronized_pool_resource>(&g_CrtMemPool, name LLM_EXTRA_ARG_FWD);
#endif
}
MEMDBG_ALLOC_FUNC_END

mempool_t *LLM_FUNCTION_MANGLE(Mem_AllocSubPool)( mempool_t *backend, const char *name LLM_EXTRA_ARG_DEF )
MEMDBG_ALLOC_FUNC_BEGIN
{
    return new PmrMemPool<pmr::synchronized_pool_resource>(backend, name LLM_EXTRA_ARG_FWD);
}
MEMDBG_ALLOC_FUNC_END

void LLM_FUNCTION_MANGLE(Mem_FreePool)( mempool_t **poolptr LLM_EXTRA_ARG_DEF )
{
    delete std::exchange((*poolptr), nullptr);
}

void LLM_FUNCTION_MANGLE(Mem_EmptyPool)( mempool_t *pool LLM_EXTRA_ARG_DEF )
{
    pool->Empty();
}

void *LLM_FUNCTION_MANGLE(Mem_Realloc)( mempool_t *poolptr, void *data, size_t size LLM_EXTRA_ARG_DEF )
MEMDBG_ALLOC_FUNC_BEGIN
{
    if(!data)
        return LLM_FUNCTION_MANGLE(Mem_Alloc)(poolptr, size LLM_EXTRA_ARG_FWD);
    auto mem = (memheader_t *)data - 1;
    auto old_size = mem->size;
    if(size <= old_size)
        return data;
    auto new_data = LLM_FUNCTION_MANGLE(Mem_Alloc)( poolptr, size LLM_EXTRA_ARG_FWD );
    memcpy(new_data, data, old_size);
    LLM_FUNCTION_MANGLE(Mem_Free)(data LLM_EXTRA_ARG_FWD);
    return new_data;
}
MEMDBG_ALLOC_FUNC_END

void *LLM_FUNCTION_MANGLE(Mem_Alloc)( mempool_t *poolptr, size_t size LLM_EXTRA_ARG_DEF )
MEMDBG_ALLOC_FUNC_BEGIN
{
#ifdef XASH_MEMDBG
    auto alloc_size = sizeof(memheader_t) + size + sizeof(std::max_align_t);
#else
    auto alloc_size = sizeof(memheader_t) + size;
#endif
    memheader_t *mem = (memheader_t *)poolptr->Alloc(alloc_size, alignof(std::max_align_t));
    mem->pool = poolptr;
    mem->size = size;
#ifdef XASH_MEMDBG
    mem->filename = filename;
    mem->fileline = fileline;
    mem->sentinel1 = MEMHEADER_SENTINEL1;

    *((byte *)mem + sizeof(memheader_t) + mem->size) = MEMHEADER_SENTINEL2;
#endif
    // TODO : why should we need this ?
    memset(mem + 1, 0, size);

    return mem + 1;
}
MEMDBG_ALLOC_FUNC_END

#ifdef XASH_MEMDBG
static const char *Mem_CheckFilename( const char *filename )
{
    static const char	*dummy = "<corrupted>\0";
    const char	*out = filename;
    int		i;

    if( !out ) return dummy;
    for( i = 0; i < 128; i++, out++ )
        if( *out == '\0' ) break; // valid name
    if( i == 128 ) return dummy;
    return filename;
}
#endif

void LLM_FUNCTION_MANGLE(Mem_Free)( void *data LLM_EXTRA_ARG_DEF )
{
    if(!data)
        return;
    auto mem = (memheader_t *)data - 1;
#ifdef XASH_MEMDBG
    if( mem->sentinel1 != MEMHEADER_SENTINEL1 )
    {
        mem->filename = Mem_CheckFilename( mem->filename ); // make sure what we don't crash var_args
        Sys_Error( "Mem_Free: trashed header sentinel 1 (alloc at %s:%i, free at %s:%i)\n", mem->filename, mem->fileline, filename, fileline );
    }

    if( *((byte *)mem + sizeof( memheader_t ) + mem->size ) != MEMHEADER_SENTINEL2 )
    {
        mem->filename = Mem_CheckFilename( mem->filename ); // make sure what we don't crash var_args
        Sys_Error( "Mem_Free: trashed header sentinel 2 (alloc at %s:%i, free at %s:%i)\n", mem->filename, mem->fileline, filename, fileline );
    }
#endif
    mem->pool->Free(mem);
}
