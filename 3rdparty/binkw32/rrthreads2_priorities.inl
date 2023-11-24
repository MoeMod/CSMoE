// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

// functions to convert from and to platform and system priroities

// at minimum, you must define PLATFORM_HIGHEST_PRIORITY, PLATFORM_LOWEST_PRIORITY
//   and a function or macro called "PLATFORM_CURRENT_THREAD_PRIOITY".
//   these defines can be constants or global variables or functions. The highest
//   and lowest defines can be numerically larger or smaller than each other.

// optional defines are (if you don't define, we blend from highest and lowest):
//   PLATFORM_NORMAL_PRIORITY (usually directly in between highest&lowest)
//   PLATFORM_LOW_PRIORITY (higher than lowest, lower than normal)
//   PLATFORM_HIGH_PRIORITY (higher than normal, lower than highest)

#ifndef PLATFORM_HIGHEST_PRIORITY
#error you need to define PLATFORM_HIGHEST_PRIORITY!!
#endif

#ifndef PLATFORM_LOWEST_PRIORITY
#error you need to define PLATFORM_LOWEST_PRIORITY!!
#endif

#ifndef PLATFORM_GET_CURRENT_THREAD_PRIORITY
#error you need to define PLATFORM_GET_CURRENT_THREAD_PRIORITY!!
#endif

#ifndef PLATFORM_NORMAL_PRIORITY
#define PLATFORM_NORMAL_PRIORITY (((PLATFORM_HIGHEST_PRIORITY)+(PLATFORM_LOWEST_PRIORITY)+1)/2)
#endif

#ifndef PLATFORM_LOW_PRIORITY
#define PLATFORM_LOW_PRIORITY (((PLATFORM_HIGHEST_PRIORITY)+((PLATFORM_LOWEST_PRIORITY)*3)+1)/4)
#endif

#ifndef PLATFORM_HIGH_PRIORITY
#define PLATFORM_HIGH_PRIORITY (((PLATFORM_LOWEST_PRIORITY)+((PLATFORM_HIGHEST_PRIORITY)*3)+1)/4)
#endif

// helper function to see if platform prioity is in range
static int is_good_platform_priority( int platform_pri )
{
  if ( PLATFORM_HIGHEST_PRIORITY > PLATFORM_LOWEST_PRIORITY )
    return ( ( platform_pri <= PLATFORM_HIGHEST_PRIORITY ) && ( platform_pri >= PLATFORM_LOWEST_PRIORITY ) ) ? 1 : 0;
  else
    return ( ( platform_pri <= PLATFORM_LOWEST_PRIORITY ) && ( platform_pri >= PLATFORM_HIGHEST_PRIORITY ) ) ? 1 : 0;
}

// given a platform priority, turn it into a RAD one
//   when porting, you just need to make sure the platforms native priority range
//   is adjusted to not conflict with the special RAD ones, if the values are less
//   than 0xf0000000 (signed or not), then you can just shift up, otherwise,
//   you might have to bias first
static rrThreadPriority2 platform_to_rad_pri( int val )
{
  if ( val == PLATFORM_NORMAL_PRIORITY )
    return rrThreadNormal;
  else if ( val == PLATFORM_LOWEST_PRIORITY )
    return rrThreadLowest;
  else if ( val == PLATFORM_HIGHEST_PRIORITY )
    return rrThreadHighest;
  else if ( val == PLATFORM_LOW_PRIORITY )
    return rrThreadLow;
  else if ( val == PLATFORM_HIGH_PRIORITY )
    return rrThreadHigh;
  else
  {
    // if this is an inbounds priority, just encode it
    if ( is_good_platform_priority( val ) )
    {
      // encode platform priority as shifted up value
      return (rrThreadPriority2)(SINTa)((val<<4)|0xf);
    }
    else
    {
      // otherwise mark as "don't set flag"
      return 0;
    }
  }
}

// convert the other direction
static int rad_to_platform_pri( rrThreadPriority2 val )
{
  if ( val == rrThreadNormal )
    return PLATFORM_NORMAL_PRIORITY;
  else if ( val == rrThreadLowest )
    return PLATFORM_LOWEST_PRIORITY;
  else if ( val == rrThreadHighest )
    return PLATFORM_HIGHEST_PRIORITY;
  else if ( val == rrThreadLow )
    return PLATFORM_LOW_PRIORITY;
  else if ( val == rrThreadHigh )
    return PLATFORM_HIGH_PRIORITY;
  else if ( val == rrThreadSameAsCalling )
    return PLATFORM_GET_CURRENT_THREAD_PRIORITY;
  else if ( ( ( (int)(SINTa) val ) & 0xf ) == 0xf ) 
  {
    // decode platform priority as shifted down value
    return (int) ( ( (SINTa) val ) >> 4 );
  }

  // unrecognized rad pri, return out of bounds priority (which causes us not to set it)
  return ( PLATFORM_HIGHEST_PRIORITY > PLATFORM_LOWEST_PRIORITY ) ? ( PLATFORM_HIGHEST_PRIORITY + 1 ) : ( PLATFORM_HIGHEST_PRIORITY - 1 );
}

static S32 add_to_platform_priority( int platform_pri, S32 upordown ) // upordown: pos = higher, neg = lower
{
  int np;
  if ( PLATFORM_HIGHEST_PRIORITY > PLATFORM_LOWEST_PRIORITY )
  {
    np = platform_pri + upordown;
    if ( ( ( np < platform_pri ) && ( upordown > 0 ) ) || ( np > PLATFORM_HIGHEST_PRIORITY ) )
      np = PLATFORM_HIGHEST_PRIORITY;
    if ( ( ( np > platform_pri ) && ( upordown < 0 ) ) || ( np < PLATFORM_LOWEST_PRIORITY ) )
      np = PLATFORM_LOWEST_PRIORITY;
  }
  else
  {
    np = platform_pri - upordown;
    if ( ( ( np > platform_pri ) && ( upordown > 0 ) ) || ( np < PLATFORM_HIGHEST_PRIORITY ) )
      np = PLATFORM_HIGHEST_PRIORITY;
    if ( ( ( np < platform_pri ) && ( upordown < 0 ) ) || ( np > PLATFORM_LOWEST_PRIORITY ) )
      np = PLATFORM_LOWEST_PRIORITY;
  }
  return np;
}

RADDEFFUNC rrThreadPriority2 RADLINK rrThreadPriorityAdd( rrThreadPriority2 v, S32 upordown )
{
  S32 pp,np;
  pp = rad_to_platform_pri( v );
  // if the passed in rrthreadpriority is invalid, just return unchanged
  if ( !is_good_platform_priority( pp ) )
    return v;
  np = add_to_platform_priority( pp, upordown );
  return platform_to_rad_pri( np );
}

RADDEFFUNC rrThreadPriority2 RADLINK rrThreadNativePriorityHelper( SINTa val )
{
  return platform_to_rad_pri( (int)val );
}

// decodes rrThreadCoreIndex into flags and core numbers...
static int rad_to_platform_core( rrThreadCoreIndex index, U32 * core_num, S32 * hard_affin )
{
  *core_num = (U32)-1;
  *hard_affin = 0;

  // if zero, don't assign
  if ( index == 0 )
    return 0;
  
  *hard_affin = ( ( (UINTa) index ) & 2) ? 1 : 0;

  if ( ( ( (UINTa) index ) & 0xfff0 ) == 0xfff0 )
  {
    // by prec
    *core_num = rrGetCPUCoreByPrecedence( ( (U32) (UINTa) index ) >> 16 );
  }
  else if ( ( ( (UINTa) index ) & 0xf0 ) == 0xf0 )
  {
    // direct core index
    *core_num = ( ( (U32) (UINTa) index ) >> 8 ) & 0xff;
  }
  else
  {
    // weird core index, assert in debug
    RR_ASSERT(0);
    return 0;
  }

  return 1;
}

static int is_good_core_num( U32 core_num )
{
  return ( core_num != (U32)-1 ) ? 1 : 0;
}

static char * limit_thread_name( char * short_name, int max, char const * name )
{
  short_name[0] = 0;
  if ( name )
  {
    int len = 0;
    for (;;)
    {
      short_name[len] = name[len];
      if (name[len] == 0)
        break;
      ++len;
      if ( len == max ) 
      {
        short_name[max-1] = 0;
        break;
      }
    }
    return short_name;
  }
  return 0;
}

