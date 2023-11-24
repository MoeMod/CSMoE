// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

// this ugliness is for cross platform alloca
#include <stdlib.h>
#ifndef alloca
#if defined(_MSC_VER)
  #include <malloc.h>
#elif defined(__GNUC__)
  #include <alloca.h>
#endif
#endif
#define radalloca(amt) ((void*)((((UINTa)alloca((amt)+15))+15)&~15))
