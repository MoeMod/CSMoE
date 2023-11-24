// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#include "binkmarkers.h"

S32 is_binkv1( U32 marker )
{
  if ( (marker==(U32)BINKMARKER1) || (marker==(U32)BINKMARKER2) || (marker==(U32)BINKMARKER3) || (marker==(U32)BINKMARKER4) || is_binkv11_or_later_but_not_v2(marker) )
    return 1;
  return 0;
}

S32 is_binkv11_or_later_but_not_v2( U32 marker )
{
  if (marker==(U32)BINKMARKER5)
    return 1;
  return 0;
}

S32 is_binkv1_flipped_RB_chroma( U32 marker )
{
  if ((marker==(U32)BINKMARKER1) || (marker==(U32)BINKMARKER2) )
    return 1;
  return 0;
}

S32 is_binkv1_old_format( U32 marker )
{
  if ( is_binkv1_flipped_RB_chroma(marker) || (marker==(U32)BINKMARKER3) )
    return 1;
  return 0;
}

S32 is_binkv23d_or_later( U32 marker )
{
  if ( ( marker == BINKMARKERV23D ) || ( marker == BINKMARKERV25 ) || ( marker == BINKMARKERV25K ) || ( marker == BINKMARKERV27 )  || ( marker == BINKMARKERV28 ) )
    return 1;
  return 0;
}

S32 is_binkv23_or_later( U32 marker )
{
  if (  ( marker == BINKMARKERV23 ) || ( is_binkv23d_or_later(marker) ) )
    return 1;
  return 0;
}

S32 is_binkv22_or_later( U32 marker )
{
  if ( ( marker == BINKMARKERV22 ) || ( is_binkv23_or_later(marker) ) )
    return 1;
  return 0;
}

S32 is_binkv2_or_later( U32 marker )
{
  if ( ( marker==(U32)BINKMARKERV2) || ( is_binkv22_or_later( marker ) ) )
    return 1;
  return 0;  
}

S32 is_bink( U32 marker )
{
  if ( ( is_binkv1( marker ) ) || ( is_binkv2_or_later( marker ) ) )
    return 1;
  return 0;
}
