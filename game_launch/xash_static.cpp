#include <SDL.h>
#include <vector>
#if _WIN32
#include <Windows.h>
#endif

typedef void(*pfnChangeGame)(const char *progname);
typedef int(*pfnInit)(int argc, char **argv, const char *progname, int bChangeGame, pfnChangeGame func);

extern "C" int Host_Main(int szArgc, char** szArgv, const char* szGameDir, int chg, void* callback);

#ifdef _WIN32
int __stdcall WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmdLine, int nShow)
{
	int argc;
	LPWSTR* lpArgv = CommandLineToArgvW(GetCommandLineW(), &argc);
	char** argv = (char**)malloc(argc * sizeof(char*));

	for (int i = 0; i < argc; ++i)
	{
		int size = wcslen(lpArgv[i]) + 1;
		argv[i] = (char*)malloc(size);
		wcstombs(argv[i], lpArgv[i], size);
	}

	LocalFree(lpArgv);
#else
int main(int argc, char **argv)
{
#endif
	std::vector<char*> av{ "-game", "cstrike", "-console", "-developer" };
	std::copy_n(argv, argc, std::back_inserter(av));
	
	Host_Main(av.size(), av.data(), "cstrike", 0, NULL);

	return 0;
}