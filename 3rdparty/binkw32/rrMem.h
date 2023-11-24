// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

// Copyright 2008-2013 RAD Game Tools

#ifndef __RADRR_MEMH__
#define __RADRR_MEMH__

#include "rrCore.h"

RADDEFSTART

//===================================================================================
// alloca / stack array stuff

// alloca config
// - define RAD_ALLOCA to the alloca function
// - leave undefined if no alloca implementation
#ifdef __RADWIN__
  #include <malloc.h>
  #define RAD_ALLOCA_BASE _alloca
#elif defined(__RADPS4__) || defined(__RADPS5__)
  #include <stdlib.h>
  #define RAD_ALLOCA_BASE alloca
#else
  #include <alloca.h>
  #define RAD_ALLOCA_BASE alloca
#endif

#define RAD_ALLOCA RAD_ALLOCA_BASE

RADDEFEND


#endif // __RADRR_MEMH__
