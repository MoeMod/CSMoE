// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#ifndef __RADTIMERH__
  #define __RADTIMERH__

  RADDEFSTART

#ifdef __RADDOS__

  RADDEFSTART
  extern void* RADTimerSetupAddr;
  extern void* RADTimerReadAddr;
  extern void* RADTimerDoneAddr;
  RADDEFEND

  typedef void RADEXPLINK (*RADTimerSetupType)(void);
  typedef U32 RADEXPLINK (*RADTimerReadType)(void);
  typedef void RADEXPLINK (*RADTimerDoneType)(void);

  #define RADTimerSetup() ((RADTimerSetupType)(RADTimerSetupAddr))()
  #define RADTimerRead() ((RADTimerReadType)(RADTimerReadAddr))()
  #define RADTimerDone() ((RADTimerDoneType)(RADTimerDoneAddr))()

#else

  #define RADTimerSetup()
  #define RADTimerDone()

  #if (defined(__RAD16__) || defined(__RADWINEXT__))

    #define RADTimerRead timeGetTime

  #else

    RADEXPFUNC U32 RADEXPLINK RADTimerRead(void);

  #endif

#endif

  RADDEFEND

#endif
