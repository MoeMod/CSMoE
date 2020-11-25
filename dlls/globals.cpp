
#include "extdll.h"
#include "util.h"

namespace sv {

/*
* Globals initialization
*/
/*
[[hlsdk::dll_global]] ULONG		g_ulFrameCount;
[[hlsdk::dll_global]] ULONG		g_ulModelIndexEyes;
[[hlsdk::dll_global]] ULONG		g_ulModelIndexPlayer;
*/
[[hlsdk::dll_global]] Vector g_vecAttackDir;
[[hlsdk::dll_global]] int g_iSkillLevel;
[[hlsdk::dll_global]] int gDisplayTitle;
[[hlsdk::dll_global]] BOOL g_fGameOver;
[[hlsdk::dll_global]] const Vector g_vecZero = Vector(0, 0, 0);
[[hlsdk::dll_global]] int g_Language;

[[hlsdk::dll_global]] bool g_bIsCzeroGame = false;
[[hlsdk::dll_global]] bool g_bEnableCSBot = false;

}
