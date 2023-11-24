// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#include "binkproj.h"

RADDEFSTART

S32 is_bink( U32 marker );

S32 is_binkv1( U32 marker );

S32 is_binkv11_or_later_but_not_v2( U32 marker );

S32 is_binkv1_flipped_RB_chroma( U32 marker );

S32 is_binkv1_old_format( U32 marker );

S32 is_binkv23_or_later( U32 marker );

S32 is_binkv23d_or_later( U32 marker );

S32 is_binkv22_or_later( U32 marker );

S32 is_binkv2_or_later( U32 marker );

RADDEFEND
