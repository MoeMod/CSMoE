// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#include "egttypes.h"

RADDEFSTART

RADDEFFUNC S32  RADLINK binkdefopen (UINTa * user_data, const char * fn, U64 * optional_out_file_size);
RADDEFFUNC U64  RADLINK binkdefread (UINTa * user_data, void * dest, U64 bytes);
RADDEFFUNC void RADLINK binkdefseek (UINTa * user_data, U64 pos);
RADDEFFUNC void RADLINK binkdefclose(UINTa * user_data);

RADDEFEND

// @cdep pre $TakePlatformCPP

