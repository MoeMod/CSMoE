
#pragma once

#ifdef __cplusplus
extern "C" {
#endif
	
void WinRT_FullscreenMode_Install(IInspectable* hwnd, BOOL fullscreen);
void WinRT_BackButton_Install();

#ifdef __cplusplus
}
#endif