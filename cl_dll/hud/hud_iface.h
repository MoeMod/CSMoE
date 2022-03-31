//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
#pragma once
#if !defined( HUD_IFACEH )
#define HUD_IFACEH

#include "exportdef.h"

#include "wrect.h"
#include "../engine/cdll_int.h"
namespace cl {
    typedef int (*pfnUserMsgHook)(const char *pszName, int iSize, void *pbuf);
    extern cl_enginefunc_t gEngfuncs;
}

#endif
