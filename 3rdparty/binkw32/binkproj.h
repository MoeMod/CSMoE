// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#ifndef BINKPROJH
#define BINKPROJH

#ifndef RAD_NO_LOWERCASE_TYPES
#define RAD_NO_LOWERCASE_TYPES
#endif

#include "egttypes.h"

#define radassert rrassert
#define radassertfail() rrassert(0)

#define BreakPoint() RR_BREAK()

#ifndef __RADTIMERH__
#include "radtimer.h"
#endif

#ifndef __BINKH__
#include "bink.h"
#endif

#if !defined(__RADNT__) && !defined(__RADCONSOLE__)
  #define NTELEMETRY
#endif

#define GENABOUT \
               "Epic Games Tools LLC\n"                              \
               "550 Kirkland Way - Suite 406\n"                      \
               "Kirkland, WA  98033\n"                               \
               "425-893-4300\n"                                      \
               "FAX: 425-609-2463\n\n"                               \
               "Web: www.radgametools.com\n"                         \
               "E-mail: sales3@radgametools.com\n\n"                 \
               "By Jeff Roberts\n\n"                                 \
               "This software cannot be used for commercial purposes\n" \
               "of any kind without first obtaining a license\n"      \
               "from RAD Game Tools.\n\n" 

#define BINKABOUT \
               "Bink Video Technology - Version " BinkVersion "\n\n" \
               GENABOUT

#define SMACKABOUT \
               "Smacker Video Technology - Version " SMACKVERSION "\n\n" \
               GENABOUT

#endif
