
#pragma once

#ifdef __cplusplus
extern "C" {
#endif
	
void WinRT_FullscreenMode_Install(int fullscreen);
void WinRT_BackButton_Install();
void WinRT_SaveVideoMode(int w, int h);
float WinRT_GetDisplayDPI();
char* WinRT_GetUserName();
void WinRT_ShellExecute(const char* path);
void WinRT_OpenGameFolderWithExplorer();

void WinRT_SetClipboardData(const char* buffer, size_t size);
void WinRT_GetClipboardData(char* buffer, size_t size);

// related to window, not scaled
void WinRT_ImeSetInputScreenPos(int x, int y);
void WinRT_ImeCreateContext();
void WinRT_ImeEnableTextInput(int enable, int force);

#ifdef __cplusplus
}
#endif