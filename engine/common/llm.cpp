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
#include <boost/align.hpp>

#include <bit>

#include "tier0/platform.h"

#ifdef XASH_TBBMALLOC
#include <tbb/scalable_allocator.h>
#endif

#ifdef __APPLE__
#include <malloc/malloc.h>
#include <mach/mach.h>

#endif

// MoeMod : there is no std::pmr on ios, so we are using the boost one
namespace pmr = boost::container::pmr;

#define MEMHEADER_SENTINEL1	0xDEADF00D
#define MEMHEADER_SENTINEL2	0xDF

class ILowLevelMalloc
{
public:
    virtual ~ILowLevelMalloc() = default;
    virtual void *Alloc(size_t size) = 0;
    virtual void *CAlloc(size_t n, size_t size) = 0;
    virtual void *AlignedAlloc(size_t size, size_t align =  alignof(std::max_align_t)) = 0;
    virtual void Free(void *ptr) = 0;
};

class IMemPool : public ILowLevelMalloc
{
public:
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

    void *Alloc(size_t size) override
    {
        return rml::pool_malloc(tbbpool, size);
    }

    void* CAlloc(size_t n, size_t size) override
    {
        void *res = rml::pool_malloc(tbbpool, n * size);
        memset(res, 0, n * size);
        return res;
    }

    void *AlignedAlloc(size_t size, size_t align) override
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

class MallocRes : public pmr::memory_resource
{
public:
    MallocRes(ILowLevelMalloc *bk) : backendpool(bk) {}

    void* do_allocate(std::size_t size, std::size_t align) override
    {
        return backendpool->AlignedAlloc(size, align);
    }
    void do_deallocate(void* ptr, std::size_t bytes, std::size_t alignment) override
    {
        return backendpool->Free(ptr);
    }
    bool do_is_equal(const memory_resource& other) const noexcept override
    {
        return this == &other;
    }

    ILowLevelMalloc *backendpool;
};

template<class T = pmr::unsynchronized_pool_resource>
class PmrMemPool : public mempool_t, public MemPoolConfig
{
public:
    PmrMemPool(ILowLevelMalloc *bk, const char *name LLM_EXTRA_ARG_DEF) :
        MemPoolConfig{ name LLM_EXTRA_ARG_FWD },
        memres(bk),
        pmrpool(pmr::pool_options{}, &memres)
    {

    }

    ~PmrMemPool() override
    {

    }

    void *Alloc(size_t size) override
    {
        return pmrpool.allocate(size);
    }

    void *CAlloc(size_t n, size_t size) override
    {
        auto res = pmrpool.allocate(n * size);
        memset(res, 0, n * size);
        return res;
    }

    void *AlignedAlloc(size_t size, size_t align) override
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
    MallocRes memres;
    T pmrpool;
};

#ifdef __APPLE__
class AppleMemPool : public mempool_t, public MemPoolConfig
{
public:
    AppleMemPool(const char *name LLM_EXTRA_ARG_DEF) :
            MemPoolConfig{ name LLM_EXTRA_ARG_FWD },
            zone(nullptr)
    {
        zone = malloc_create_zone(0, 0);
        malloc_set_zone_name(zone, name);
    }

    ~AppleMemPool() override
    {
        malloc_destroy_zone(zone);
    }

    void *Alloc(size_t size) override
    {
        return malloc_zone_malloc(zone, size);
    }

    void *CAlloc(size_t n, size_t size) override
    {
        return malloc_zone_calloc(zone, n, size);
    }

    void *AlignedAlloc(size_t size, size_t align) override
    {
        return malloc_zone_memalign(zone, align, size);
    }

    void Free(void *ptr) override
    {
        malloc_zone_free(zone, ptr);
    }

    void Empty() override
    {
        // should be MT safe
        malloc_destroy_zone(zone.exchange(malloc_create_zone(0, 0)));
    }

private:
    std::atomic<malloc_zone_t *> zone;
};
#endif

class CrtMalloc : public ILowLevelMalloc
{
public:

    void *Alloc(size_t size) override
    {
#ifndef __APPLE__
        return ::operator new(size);
#else
        return malloc(size);
#endif
    }

    void *CAlloc(size_t n, size_t size) override
    {
#ifndef __APPLE__
        return ::operator new(n * size);
#else
        return calloc(n, size);
#endif
    }

    void *AlignedAlloc(size_t size, size_t align) override
    {
#ifndef __APPLE__
        return ::operator new(size, (std::align_val_t)align);
#else
        return aligned_alloc(align, size);
#endif
    }

    void Free(void *ptr) override
    {
#ifndef __APPLE__
        ::operator delete(ptr);
#else
        free(ptr);
#endif
    }

} g_CrtMalloc;

typedef struct alignas(std::max_align_t) memheader_s {
    mempool_t *pool;
    uint size;
    uint header_size;
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
#elif defined(__APPLE__)
    return new AppleMemPool( name LLM_EXTRA_ARG_FWD );
#else
    return new PmrMemPool<pmr::synchronized_pool_resource>(&g_CrtMalloc, name LLM_EXTRA_ARG_FWD);
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
        return LLM_FUNCTION_MANGLE(Mem_ZeroAlloc)(poolptr, size LLM_EXTRA_ARG_FWD);
    auto mem = (memheader_t *)data - 1;
    auto old_size = mem->size;
    if(size <= old_size)
        return data;
    auto new_data = LLM_FUNCTION_MANGLE(Mem_ZeroAlloc)( poolptr, size LLM_EXTRA_ARG_FWD );
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
    memheader_t *mem = (memheader_t *)poolptr->Alloc(alloc_size);
    mem->pool = poolptr;
    mem->size = size;
    mem->header_size = sizeof(memheader_t);
#ifdef XASH_MEMDBG
    mem->filename = filename;
    mem->fileline = fileline;
    mem->sentinel1 = MEMHEADER_SENTINEL1;

    *((byte *)mem + sizeof(memheader_t) + mem->size) = MEMHEADER_SENTINEL2;
    memset(mem + 1, 0xCD, size);
#endif

    return mem + 1;
}
MEMDBG_ALLOC_FUNC_END

void *LLM_FUNCTION_MANGLE(Mem_ZeroAlloc)( mempool_t *poolptr, size_t size LLM_EXTRA_ARG_DEF )
MEMDBG_ALLOC_FUNC_BEGIN
{
#ifdef XASH_MEMDBG
    auto alloc_size = sizeof(memheader_t) + size + sizeof(std::max_align_t);
#else
    auto alloc_size = sizeof(memheader_t) + size;
#endif
    memheader_t *mem = (memheader_t *)poolptr->CAlloc(1, alloc_size);
    mem->pool = poolptr;
    mem->size = size;
    mem->header_size = sizeof(memheader_t);
#ifdef XASH_MEMDBG
    mem->filename = filename;
    mem->fileline = fileline;
    mem->sentinel1 = MEMHEADER_SENTINEL1;

    *((byte *)mem + sizeof(memheader_t) + mem->size) = MEMHEADER_SENTINEL2;
#endif

    return mem + 1;
}
MEMDBG_ALLOC_FUNC_END

void *LLM_FUNCTION_MANGLE(Mem_AlignedAlloc)( mempool_t *poolptr, size_t size, size_t align LLM_EXTRA_ARG_DEF )
MEMDBG_ALLOC_FUNC_BEGIN
{
    align = boost::alignment::align_up(align, alignof(std::max_align_t));
    auto header_size = boost::alignment::align_up(sizeof(memheader_t), align);
#ifdef XASH_MEMDBG
    auto alloc_size = header_size + size + align + sizeof(std::max_align_t);
#else
    auto alloc_size = header_size + size + align;
#endif
    void *mem = poolptr->AlignedAlloc(alloc_size, align);
    memset(mem, 0, alloc_size);
    void *data = (byte *)mem + header_size;
    data = boost::alignment::align(align, size, data, alloc_size);
    header_size = (byte *)data - (byte *)mem;
    auto header = (memheader_t *)((byte *)data - sizeof(memheader_t));
    header->pool = poolptr;
    header->size = size;
    header->header_size = header_size;
#ifdef XASH_MEMDBG
    header->filename = filename;
    header->fileline = fileline;
    header->sentinel1 = MEMHEADER_SENTINEL1;

    *((byte *)mem + header_size + size) = MEMHEADER_SENTINEL2;
#endif

    return data;
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
    auto header = (memheader_t *)data - 1;
#ifdef XASH_MEMDBG
    if( header->sentinel1 != MEMHEADER_SENTINEL1 )
    {
        header->filename = Mem_CheckFilename( header->filename ); // make sure what we don't crash var_args
        DebuggerBreak();
        Sys_Error( "Mem_Free: trashed header sentinel 1 (alloc at %s:%i, free at %s:%i)\n", header->filename, header->fileline, filename, fileline );
    }

    if( *((byte *)data + header->size ) != MEMHEADER_SENTINEL2 )
    {
        header->filename = Mem_CheckFilename( header->filename ); // make sure what we don't crash var_args
        DebuggerBreak();
        Sys_Error( "Mem_Free: trashed header sentinel 2 (alloc at %s:%i, free at %s:%i)\n", header->filename, header->fileline, filename, fileline );
    }
    header->sentinel1 = 0xCDCDCDCD;
    *((byte*)data + header->size) = 0xCD;
#endif
    auto mem = (byte *)data - header->header_size;
    if(header->pool)
        header->pool->Free(mem);
    else
        free(mem);
}

void *Mem_VirtualCopy(void *dest, const void *src, size_t size)
{
#ifdef __APPLE__
    if(size >= 16384)
    {
        // Apple has COW memcpy
        vm_copy(mach_task_self(), (vm_address_t)src, (vm_size_t)size, (vm_address_t)dest);
        return dest;
    }
#endif
    return memcpy(dest, src, size);
}
#ifdef XASH_OPERATOR_NEW_DELETE
void *operator new(std::size_t size, mempool_t *poolptr)
{
    return Mem_Alloc(poolptr, size);
}

void *operator new[](std::size_t size, mempool_t *poolptr)
{
    return operator new(size, poolptr);
}

void operator delete(void *ptr, mempool_t *poolptr)
{
    return operator delete(ptr);
}

void operator delete[](void *ptr, mempool_t *poolptr)
{
    return operator delete(ptr);
}

void* operator new(std::size_t count)
{
    return Mem_Alloc(nullptr, count);
}

void operator delete(void* ptr) noexcept
{
    return Mem_Free(ptr);
}
#endif