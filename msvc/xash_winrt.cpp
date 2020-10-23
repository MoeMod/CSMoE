#include <SDL.h>
#include <Windows.h>
#include <vector>
#include <iterator>

typedef void(*pfnChangeGame)(const char *progname);
typedef int(*pfnInit)(int argc, char **argv, const char *progname, int bChangeGame, pfnChangeGame func);

extern "C" int Host_Main(int szArgc, const char** szArgv, const char* szGameDir, int chg, void* callback);

int main(int argc, char **argv)
{
	std::vector<const char*> av{ "-game", "csmoe", "-console", "-developer" };
	std::copy_n(argv, argc, std::back_inserter(av));
	
	Host_Main(av.size(), av.data(), "csmoe", 0, NULL);

	return 0;
}