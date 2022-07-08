//========= Copyright ?1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: Includes all the headers/declarations necessary to access the
//			engine interface
//
// $NoKeywords: $
//=============================================================================

#ifndef ENGINEINTERFACE_H
#define ENGINEINTERFACE_H

#ifdef _WIN32
#pragma once
#endif

// these stupid set of includes are required to use the cdll_int interface
#include <assert.h>
#include "port.h"
#include "xash3d_types.h"

#include "vgui_api.h"
#include "cdll_int.h"
#include "cvardef.h"

// engine interface singleton accessor
extern cl_enginefunc_t *engine;
extern class IGameUIFuncs *gameuifuncs;

#endif // ENGINEINTERFACE_H
