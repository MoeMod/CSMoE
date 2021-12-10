#include "DeviceListener.h"
#include "SurfaceDial.h"


extern "C" void SurfaceDial_Install(HWND hwnd)
{
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
	// Create Surface Dial Interface.
	DeviceListener* listener = new DeviceListener(console);
	listener->Init(hwnd);
}