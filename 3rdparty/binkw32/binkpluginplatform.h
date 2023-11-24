// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#include "egttypes.h"

#if defined( BUILDING_FOR_UNREAL_ONLY ) 
  #define PLATFORM_HAS_RHI 1
#endif

#ifdef __RAD_NDA_PLATFORM__

  #define PLATFORM_HAS_NDA 1

  // @cdep pre $AddPlatformInclude
  #include RR_PLATFORM_PATH_STR( __RAD_NDA_PLATFORM__, _binkpluginplatform.h )

#else

  #if defined(__RADWIN__) 
    #if !defined( BUILDING_FOR_UNREAL_ONLY ) && !defined( __RADWINRT__)
      #define PLATFORM_HAS_D3D9 1
    #endif
    #define PLATFORM_HAS_D3D11 1
    #define PLATFORM_HAS_D3D12 1
    #if !defined( __RADWINRT__)
      #define PLATFORM_HAS_GL 1
    #endif
  #endif

  #if defined(__RADLINUX__) || defined(__RADANDROID__) || ( defined(__RADNT__) && defined(__RAD64__) )
    #define PLATFORM_HAS_VULKAN 1
  #endif

  #if defined(__RADLINUX__) || defined(__RADANDROID__) || defined(__RADEMSCRIPTEN__)
    #define PLATFORM_HAS_GL 1
  #endif

  #if ( defined(__RADMAC__) || defined(__RADIPHONE__) ) && !defined(BUILDING_FOR_UNREAL_ONLY)
    #define PLATFORM_HAS_GL 1
  #endif

  #if defined(__RADIPHONE__) || (defined(__RADMAC__) && defined(__RAD64__)) // metal on mac is 64-bit
    #define PLATFORM_HAS_METAL 1
  #endif
#endif
