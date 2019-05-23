#ifndef DLL_SWITCH_H
#define DLL_SWITCH_H

#ifdef __cplusplus
extern "C" {
#endif

#define RTLD_LAZY 0x0001
#define RTLD_NOW  0x0002

typedef struct dllexport_s
{
	const char *name;
	void *func;
} dllexport_t;

#if 0
typedef struct Dl_info_s
{
	void *dli_fhandle;
	const char *dli_sname;
	const void *dli_saddr;
} Dl_info;
#endif

int dll_register( const char *name, dllexport_t *exports );

#ifdef __cplusplus
}
#endif

#endif