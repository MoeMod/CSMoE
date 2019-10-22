#include <SDL.h>
#include <Windows.h>
#include <vector>

typedef void(*pfnChangeGame)(const char *progname);
typedef int(*pfnInit)(int argc, char **argv, const char *progname, int bChangeGame, pfnChangeGame func);

extern "C" int Host_Main(int szArgc, char** szArgv, const char* szGameDir, int chg, void* callback);

int main(int argc, char **argv)
{
	std::vector<char*> av(argv, argv + argc);
	char ext[3][32] = { "-game", "csmoe", "-console" };
	std::copy(std::begin(ext), std::end(ext), std::back_inserter(av));
	
	Host_Main(argc, argv, "csmoe", 0, NULL);

	return 0;
}