#pragma once

//
// zone.c
//
#ifdef XASH_MEMDBG

#define Mem_Alloc( pool, size ) LLM_FUNCTION_MANGLE(Mem_Alloc)( pool, size LLM_EXTRA_ARG_GEN )
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
#endif

void *LLM_FUNCTION_MANGLE(Mem_Realloc)( mempool_t *poolptr, void *memptr, size_t size LLM_EXTRA_ARG_DEF );
void *LLM_FUNCTION_MANGLE(Mem_Alloc)( mempool_t *poolptr, size_t size LLM_EXTRA_ARG_DEF );
mempool_t *LLM_FUNCTION_MANGLE(Mem_AllocPool)( const char *name LLM_EXTRA_ARG_DEF );
mempool_t *LLM_FUNCTION_MANGLE(Mem_AllocSubPool)( mempool_t *backend, const char *name LLM_EXTRA_ARG_DEF );
void LLM_FUNCTION_MANGLE(Mem_FreePool)( mempool_t **poolptr LLM_EXTRA_ARG_DEF );
void LLM_FUNCTION_MANGLE(Mem_EmptyPool)( mempool_t *poolptr LLM_EXTRA_ARG_DEF );
void LLM_FUNCTION_MANGLE(Mem_Free)( void *data LLM_EXTRA_ARG_DEF );