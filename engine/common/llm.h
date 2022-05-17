#pragma once

//
// zone.c
//
#ifdef XASH_MEMDBG

#define Mem_Alloc( pool, size ) _Mem_Alloc( pool, size, __FILE__, __LINE__ )
#define Mem_Realloc( pool, ptr, size ) _Mem_Realloc( pool, ptr, size, __FILE__, __LINE__ )
#define Mem_Free( mem ) _Mem_Free( mem, __FILE__, __LINE__ )
#define Mem_AllocPool( name ) _Mem_AllocPool( name, __FILE__, __LINE__ )
#define Mem_AllocSubPool( backend, name ) _Mem_AllocSubPool( backend, name, __FILE__, __LINE__ )
#define Mem_FreePool( pool ) _Mem_FreePool( pool, __FILE__, __LINE__ )
#define Mem_EmptyPool( pool ) _Mem_EmptyPool( pool, __FILE__, __LINE__ )

#define LLM_FUNCTION_MANGLE(f) _##f
#define LLM_EXTRA_ARG_DEF , const char *filename, int fileline
#define LLM_EXTRA_ARG_FWD , filename, fileline
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