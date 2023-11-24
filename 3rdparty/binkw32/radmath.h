// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#ifndef __RADMATHH__
#define __RADMATHH__

#include <math.h>

RADDEFSTART

#define mult64anddiv( a, b, c )  ( (U32) ( ( ( (U64) a ) * ( (U64) b ) ) / ( (U64) c ) ) )
#define mult64andshift( a, b, c )  ( (U32) ( ( ( (U64) a ) * ( (U64) b ) ) >> ( (U64) c ) ) )
#define radabs abs

#if defined( _MSC_VER )

  #if defined(__RADX86__) 
    #pragma intrinsic(abs, log, fabs, sqrt, fmod, sin, cos, tan, asin, acos, atan, atan2, exp)
  #endif

  #define radcos( val ) cos( (float)(val) )
  #define radsin( val ) sin( (float)(val) )
  #define radatan( val ) atan( (float)(val) )
  #define radatan2( val1, val2 ) atan2( (float)(val1), (float)(val2) )
  #define radpow( val1, val2 ) pow( (float)(val1), (float)(val2) )
  #define radfsqrt( val ) sqrt( (float)(val) )
  #define radlog( val ) log( (float)(val) )
  #define radlog10( val ) log10( (float)(val) )
  #define radexp( val ) exp( (float)(val) )
  #define radfabs( val ) fabs( (float)(val) )
  #define radfloor( val ) floor( (float)(val) )
  #define radceil( val ) ceil( (float)(val) )

  #if defined(__RAD64__) || (__RADARM__)
  
    #define LN2 0.693147181F

    static F64 __inline radlog2( F64 X )
    {
      return( radlog( X ) / LN2 );
    }

  #else
  
    static F64 __inline radlog2( F64 X )
    {
      F64 Result;
      __asm
      {
        fld1
        fld X
        fyl2x
        fstp Result
      }
      return( Result );
    }
  
  #endif

#else

  // generally no non-windows math stuff...
    
  //#define radcos( val ) cosf( val )
  //#define radsin( val ) sinf( val )
  //#define radatan( val ) atanf( val )
  //#define radatan2( val1, val2 ) atanf2( val1, val2 )
  #define radpow( val1, val2 ) powf( val1, val2 )
  //#define radfsqrt( val ) sqrtf( val )
  //#define radlog( val ) log( val )
  //#define radlog10( val ) log10f( val )
  //#define radexp( val ) expf( val )
  //#define radfabs( val ) fabsf( val )
  //#define radfloor( val ) floorf( val )
  //#define radceil( val ) ceilf( val )

#endif

RADDEFEND

#endif
