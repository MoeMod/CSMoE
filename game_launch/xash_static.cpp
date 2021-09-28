#ifdef XASH_SDL
#include <SDL.h>
#endif
#include <vector>
#include <algorithm>
#include <iterator>
#if _WIN32
#include <Windows.h>
#endif

typedef void(*pfnChangeGame)(const char *progname);
typedef int(*pfnInit)(int argc, char **argv, const char *progname, int bChangeGame, pfnChangeGame func);

extern "C" int Host_Main(int szArgc, const char** szArgv, const char* szGameDir, int chg, void* callback);

#ifdef _WIN32
int __stdcall WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmdLine, int nShow)
{
	WinExec("cd /d %~dp0", SW_NORMAL);
	WinExec("SimpleUpdater.exe /startupdate /cv \"2.3.7.0\" /url \"https://tzres.blob.core.chinacloudapi.cn/csmoe/win32/{0}\" /infofile \"update_c.xml\" /autokill /forceupdate /hideCheckUI", SW_NORMAL);
	
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
	std::vector<const char*> av{ "-game", "csmoe", "-console", "-developer" };
	std::copy_n(argv, argc, std::back_inserter(av));
	
	Host_Main(av.size(), av.data(), "csmoe", 0, NULL);

	return 0;
}