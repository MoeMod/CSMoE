#include <SDL.h>
#include <Windows.h>
#include <vector>
#include <Inspectable.h>

typedef void(*pfnChangeGame)(const char *progname);
typedef int(*pfnInit)(int argc, char **argv, const char *progname, int bChangeGame, pfnChangeGame func);

extern "C" int Host_Main(int szArgc, char** szArgv, const char* szGameDir, int chg, void* callback);

int main(int argc, char **argv)
{
	std::vector<char*> av{ "-game", "csmoe", "-console", "-developer" };
	std::copy_n(argv, argc, std::back_inserter(av));
	
	Host_Main(av.size(), av.data(), "csmoe", 0, NULL);

	return 0;
}

extern "C" int __declspec(dllexport) WinRT_InitWithXaml(Windows::UI::Xaml::Controls::SwapChainBackgroundPanel ^ hwnd)
{
	return SDL_WinRTRunApp(SDL_main, reinterpret_cast<void *>(hwnd));
}