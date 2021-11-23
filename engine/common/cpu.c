#ifdef XASH_CPUINFO
#include <cpuinfo.h>
#endif

#ifdef XASH_SDL
#include <SDL_cpuinfo.h>
#endif

void Cpu_Init(void)
{
#ifdef XASH_CPUINFO
	cpuinfo_initialize();
#endif
}

const char* Cpu_GetName(void)
{
#ifdef XASH_CPUINFO
	return cpuinfo_get_package(0)->name;
#else
    return "Unknown";
#endif
}

int Cpu_GetInstalledRamMegaBytes(void)
{
#ifdef XASH_SDL
	return SDL_GetSystemRAM();
#else
	return 0;
#endif
}
