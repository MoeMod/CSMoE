
#pragma once

#ifdef __cplusplus
extern "C" {
#endif
	
void WinRT_FullscreenMode_Install(boolean fullscreen);
void WinRT_BackButton_Install();
void WinRT_SaveVideoMode(int w, int h);

#ifdef __cplusplus
}
#endif