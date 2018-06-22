#include "maintypes.h"

#include "extdll.h"
#include "util.h"

/*
* Globals initialization
*/
/*
DLL_GLOBAL ULONG		g_ulFrameCount;
DLL_GLOBAL ULONG		g_ulModelIndexEyes;
DLL_GLOBAL ULONG		g_ulModelIndexPlayer;
*/
DLL_GLOBAL Vector g_vecAttackDir;
DLL_GLOBAL int g_iSkillLevel;
DLL_GLOBAL int gDisplayTitle;
DLL_GLOBAL BOOL g_fGameOver;
DLL_GLOBAL const Vector g_vecZero = Vector(0, 0, 0);
DLL_GLOBAL int g_Language;

DLL_GLOBAL bool g_bIsCzeroGame = false;
DLL_GLOBAL bool g_bEnableCSBot = false;
