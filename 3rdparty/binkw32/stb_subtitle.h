// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

typedef struct STB_SRT_SUB
{
  unsigned int count;    // count of subtitles
  unsigned int char_set; // 0=ascii, 1=ascii with high chars?, 2=utf8
  signed int   base;     // offset to base ptr (used internally)
  unsigned int last_ms;  // final subtitle end time
} STB_SRT_SUB;

typedef enum stb_srt_error
{
  stb_srt_no_error = 0,
  stb_srt_utf16_not_supported = 1,  // utf16 not supported (utf 8 or ascii only)
  stb_srt_bad_size = 2,             // srt image is too small or too big (1 MB supported by default)
  stb_srt_parse_error = 3,          // couldn't parse (we do a really loose parse, so this usually means it's not an str at all)
  stb_srt_compact_error = 4,        // internal error where the memory shuffling we do failed (send us the srt if this triggers)
} stb_srt_error;

  
// if successful, returns a structure that you can query for a subtitle quicky (no memory is allocaed, but the image memory buffer is modified)
//   err is optional and returns what we went wrong
STB_SRT_SUB * stb_srt_parse_image( void * image, void * image_end, stb_srt_error * err );


// stb_srt_find_subtitle will find one or more subtitle strings based on time.
//   there can be more than one subtitle at each time index - if you just want the 
//   first one, pass zero for last. Other, you query them all like this:
//
//      unsigned int index = 0;  // init with zero
//      while ( ( str = stb_srt_find_subtitle( sub, ms, 0, 0, &index ) ) != 0 )
//         do_something( str );

char const * stb_srt_find_subtitle( STB_SRT_SUB * sub, unsigned int milliseconds, unsigned int * iterate, unsigned int * out_low_ms, unsigned int * out_high_ms );


// stb_srt_get_subtitle retrieves each subtitle in order (0 to sub->count)
char const * stb_srt_get_subtitle( STB_SRT_SUB * sub, unsigned int index, unsigned int * out_low_ms, unsigned int * out_high_ms );


#ifdef STB_SUB_IMPLEMENTATION

#define STB_U32 unsigned int
#if defined(_MSC_VER)
  #define STB_U64 unsigned __int64
#else
  #define STB_U64 unsigned long long
#endif

#include <stdlib.h>
#ifndef alloca
#if defined(_MSC_VER)
  #include <malloc.h>
#elif defined(__GNUC__)
  #include <alloca.h>
#endif
#endif

static STB_U32 stb_sub_get_count_dbl_lfs( char * in, char * end, int * flagsp )
{
  STB_U32 c;
  unsigned char flags = 0;

  // stop one char early
  if ( end > in ) --end;

  c = 0;
  for(;;)
  {
    STB_U32 lfs;
    unsigned char t;

    // first scan for a linefeed in tight loop
    for(;;)
    {
      if ( in >= end )
        goto done;
      if ( in[0] == 10 )
        break;
      flags |= ( in[0] >> 6 ) & 2;
      t = ( ( in[0] & 0xc0 ) == 0xc0 ); t &= ( ( in[1] & 0xc0 ) == 0x80 ); flags |= t;
      ++in;
    }

    lfs = 1;
    ++in;
    while( in < end )
    {
      if ( in[0] == 10 )
        ++lfs;
      else if ( ( in[0] != 13 ) && ( in[0] != 32 ) ) // ingore crs and spaces
      {
        flags |= ( in[0] >> 6 ) & 2;
        t = ( ( in[0] & 0xc0 ) == 0xc0 ); t &= ( ( in[1] & 0xc0 ) == 0x80 ); flags |= t;
        break;
      }
      ++in;
    }

    if ( lfs >= 2 )
      ++c;
  }
  
 done:
  *flagsp = flags;
  return c + 1;  // add one in case the last title isn't terminated
}

static int stb_sub_get_num( char ** inp, char * end, STB_U32 * outn, int max, int lfok )
{
  char * in = *inp;
  int neg = 0;

  // only scan for the next number for less than 16 chars
  {
    char * cend = end;
    if ( ( end - in ) > max )
      cend = in + max;

    while( in < cend )
    {
      if ( ( !lfok ) && ( in[0] == 10 ) )
        return 0;
      if ( ( in[0] >= '0' ) && ( in[0] <= '9' ) )
        goto getnum;
      neg = ( in[0] == '-' ) ? 1 : 0;
      ++in;
    }
    return 0;
  }

 getnum:
  {
    STB_U32 n = 0;
  
    while( in < end )
    {
      if ( ( in[0] > '9' ) || ( in[0] < '0' ) )
        break;
      n = ( n * 10 ) + in[0] - '0';
      ++in;
    }

    *inp = in;
    *outn = (neg) ? 0 : n;
    return 1;
  }
}

static int stb_sub_get_time( char ** inp, char * end, STB_U32 * outn, int lfok )
{
  STB_U32 v;
  STB_U32 t;

  if ( !stb_sub_get_num( inp, end, &v, 6, lfok ) )
    return 0;
  t = v * 1000*60*60;

  if ( !stb_sub_get_num( inp, end, &v, 3, 0 ) )
    return 0;
  t += v * 1000*60;

  if ( !stb_sub_get_num( inp, end, &v, 3, 0 ) )
    return 0;
  t += v * 1000;

  if ( !stb_sub_get_num( inp, end, &v, 3, 0 ) )
    return 0;
  t += v;
  
  *outn = t;
  return 1;
}

static void stb_sub_eat_whitespace( char ** inp, char * end )
{
  char * in = *inp;
  // eat whitespace
  while ( ( in < end ) && ( ( (unsigned char) in[0]) <= ' ' ) )
    ++in;
  *inp = in;
}

static void stb_sub_eat_until_dbl_lf( char ** inp, char * end )
{
  char * in = *inp;
  char * st = 0;
  int c = 0;
  while ( in < end )
  {
    if ( ( (unsigned char) in[0] ) <= ' ' )
    {
      if ( st == 0 ) st = in;
    }
    else
    {
      st = 0; c = 0;
    }
    if ( in[0] == 10 )
    {
      if( ++c >= 2 )
      {
        in = st;
        break;
      }
    }
    ++in;
  }
  *inp = in;
}

static void stb_sub_memcpy( void * dest, void const * src, STB_U32 len )
{
  STB_U32 dwords = len >> 2;
  while ( dwords )
  {
    *(STB_U32 volatile * )dest = *(STB_U32 *)src;
    dest = (char*)dest + 4; src = (char*)src + 4; --dwords;
  }
  len &= 3;
  while ( len )
  {
    *(char volatile * )dest = *(char *)src;
    dest = (char*)dest + 1; src = (char*)src + 1; --len;
  }
}

static STB_U32 stb_sub_adv_and_count_lfs( char ** inp, char * end )
{
  STB_U32 n = 0;
  char * in = *inp;

  while( in < end )
  {
    if ( in[0] == 10 )
      ++n;
    else  if ( in[0] != 13 ) // we treat CR as nops, this parses win&lnx line endings
      break;
    ++in;
  }
  *inp = in;

  return n;
}

#define STB_LOW_MS_BITS  28
#define STB_HIGH_MS_BITS 16
#define STB_OFFSET_BITS  20
#define STB_LOW_MS_BITS_START (64-STB_LOW_MS_BITS)
#define STB_HIGH_MS_BITS_START (STB_OFFSET_BITS)
#define STB_LOW_MS_BITS_MASK ((1<<STB_LOW_MS_BITS)-1)
#define STB_HIGH_MS_BITS_MASK ((1<<STB_HIGH_MS_BITS)-1)
#define STB_OFFSET_BITS_MASK  ((1<<STB_OFFSET_BITS)-1)

static int stb_sub_parse( char ** inp, STB_U64 * info, STB_U32 index, char ** strp, STB_U32 * len, char * end, int * must_sort, STB_U32 * last_ms )
{
  char * str;
  STB_U32 low, high, duration;
  int retries = 0;

 retry:
  // eat the first number (the index of the subtitle - these are completely unreliable and random)
  {
    STB_U32 n;
    
    if ( !stb_sub_get_num( inp, end, &n, 8, 1 ) )
    {
      reparse:
      if ( ( ++retries ) <= 8 )  // give it 8 restarts and then give up
      {
        if ( stb_sub_adv_and_count_lfs( inp, end ) == 0 )
          stb_sub_eat_until_dbl_lf( inp, end );
        goto retry;
      }
      return 0; // parse error
    }
  }

  if ( !stb_sub_get_time( inp, end, &low, 1 ) )
    goto reparse;
  if ( !stb_sub_get_time( inp, end, &high, 0 ) )
    goto reparse;

  // find the description (with special care for empty descriptions)
  {
    STB_U32 count;

    count = stb_sub_adv_and_count_lfs( inp, end );
    if ( count >= 2 )
    {
      // there is an immediate double line feed, so this subtitle might be empty (weird, but they happen)
      
      // if we're not a number, we might still have a description, so go try to find it
      if ( ( (*inp)[0] < '0' ) || ( (*inp)[0] > '9' ) )
        goto desc;

      --(*inp); // back up one char to hold a null termainator
      str = *inp;
    }
    else
    {
     desc: 
      // we found a description, so scan until a dbl lf
      str = *inp;
      stb_sub_eat_until_dbl_lf( inp, end );
    }
  }

  // null term the string and set the output pos and len
  {
    STB_U32 l = (STB_U32)( *inp - str );

    if ( *inp == end ) // if our string butts up to the end of the buffer, we need to move it to the left a character, so we can fit in a null terminator
    {
      stb_sub_memcpy( str - 1, str, l );
      --str;
      --(*inp);
    }

    // null termainate the string and advance
    *(*inp)++ = 0;

    *strp = str;
    *len = l + 1;
  }

  stb_sub_eat_whitespace( inp, end );

  // set the info for this sub    
  if ( low > STB_LOW_MS_BITS_MASK )
    low = STB_LOW_MS_BITS_MASK;

  duration = 0;
  if ( high > low ) 
  {
    duration = high - low;
    if ( duration > STB_HIGH_MS_BITS_MASK )  // max 65 sec duration
      duration = STB_HIGH_MS_BITS_MASK;
    high = low + duration;
    if ( high > *last_ms )
      *last_ms = high;
  }

  {
    STB_U64 s = ( ( (STB_U64)low ) << STB_LOW_MS_BITS_START ) | ( ( (STB_U64)duration ) << STB_HIGH_MS_BITS_START );
    if ( ( index ) && ( s < info[ index - 1 ] ) )
      *must_sort = 1;
    info[ index ] = s;
  }

  return 1;
}

// insertion sort for small bits
static void stb_sub_insertion_sort( STB_U64 * low, STB_U64 * high )
{
  STB_U64 * i = low + 1;
  do {
    STB_U64 x = *i;
    STB_U64 * j = i - 1;
    while( j >= low )
    {
      STB_U64 y = *j;
      if ( y <= x )
        break;
      j[1] = y;
      --j;
    }
    j[1] = x;
    ++i;
  } while( i <= high );
}

// qsort a list of U64s (this is about 400 bytes of code)
static void stb_sub_sort_U64( STB_U64 * low, STB_U64 * high ) 
{
  STB_U64 * stack[(STB_OFFSET_BITS-4)*2]; // rough log2 of max subtitles in our address range
  int s = 0;
  for(;;)
  {
    for(;;)
    {
      // put on each size of pivot
      STB_U64 piv = *(STB_U64*) ( (char*)low + ( ( ( (char*)high - (char*)low ) >> 1 ) & ~(sizeof(STB_U64)-1) ) ); // same as floor(l+h)/2 but with ptrs
      STB_U64 l, h, * i = low - 1, * j = high + 1;
      for(;;)
      {
        do { l = *(++i); } while( l < piv );
        do { h = *(--j); } while( h > piv );
        if ( i >= j )
          break;
        *i = h; *j = l;
      }

      // handle right side - do we have 2 or more?
      if ( ( j + 1 ) < high )
      {
        // flip to insert on small blocks - optimization
        if ( ( high - j ) < 32 )
          stb_sub_insertion_sort( j+1, high );
        else
        {
          // put the right side on the stack
          stack[s] = j + 1; stack[s+1] = high; s += 2;
        }
      }

      // if less than 2 on the left, go get something range off stack (otherwise continue below)
      if ( low >= j )
        break;

      // check if we only have 2 on the left and handle specially - optimization
      if ( ( j - low + 1 ) < 32 )
      {
        stb_sub_insertion_sort( low, j );
        break;
      }

      // assume we will loop and do the left partition
      high = j;

      // however, before we loop around, we want to make sure that we loop on the smaller of
      //   the left partition and whatever is on the top of the stack (usually the right partition)
      //   we do this to keep the stack space to log(n) - otherwise we will overflow with some bufs
      if ( s )
      {
        if ( ( stack[s-1] - stack[s-2] ) < ( j - low ) )
        {
          STB_U64 * t = stack[s-1]; stack[s-1] = high; high = t;
          t = stack[s-2]; stack[s-2] = low; low = t;
        }
      }
    }

    if ( s == 0 )
      return;
    s -= 2; low = stack[s]; high = stack[s+1];
  }
}

#define STB_SWITCH_TO_LINEAR_SEARCH 32

// search a binary sorted list for the start of the value containing the first value larger than or equal to val
static STB_U32 stb_sub_find_U64( STB_U64 * info, STB_U32 count, STB_U64 val )
{
  STB_U32 l, r;
  
  l = 0;
  r = count;
  while ( l < r )
  {
    STB_U32 m;
    
    if ( ( r - l ) <= STB_SWITCH_TO_LINEAR_SEARCH )
      break;

    m = ( ( l + r ) / 2 );
    if ( info[ m ] < val )
      l = m + 1;
    else
      r = m;
  }

  return l;
}

static STB_SRT_SUB * stb_srt_parse( void * image, void * image_end, stb_srt_error * err, int * must_sort )
{
  STB_U32 i, num, compact;
  STB_U32 bytes_needed, last_ms;
  char * out, * in, * end, * base;
  STB_U64 * info;
  int utf8, csflags;

  in = (char*)image;
  base = (char*)image;
  out = (char*)image;
  end = (char*)image_end;

  last_ms = 0;

  if ( ( in >= end ) || ( ( end - in ) >= STB_OFFSET_BITS_MASK ) || ( ( end - in ) < ( sizeof(STB_SRT_SUB) + sizeof(STB_U64) ) ) )  // srts must be smaller than 1 MB (for our packing)
  {
    if ( err ) *err = stb_srt_bad_size;
    return 0;
  }

  // BOM dection for utf16 (and fail)
  if ( ( ( ((unsigned char)in[0]) == 0xff ) && ( ((unsigned char)in[1]) == 0xfe ) ) || ( ( ((unsigned char)in[0]) == 0xfe ) && ( ((unsigned char)in[1]) == 0xff ) ) )
  {
    if ( err ) *err = stb_srt_utf16_not_supported;
    return 0;
  }

  // BOM dection for utf8
  utf8 = 0;
  if ( ( ( ((unsigned char)in[0]) == 0xef ) && ( ((unsigned char)in[1]) == 0xbb ) && ( ((unsigned char)in[2]) == 0xbf ) ) )
  {
    utf8 = 1;
    in += 3;
  }

  // get a conversative count of subtitles by counting dbl linefees
  num = stb_sub_get_count_dbl_lfs( in, end, &csflags );
  if ( num == 0 )
  {
   parse_error: 
    if ( err ) *err = stb_srt_parse_error;
    return 0;
  }

  bytes_needed = sizeof(STB_U64) * num;  // a U64 per subtitle
  bytes_needed += sizeof(STB_SRT_SUB);

  // figure out how many subtitles we need to compact
  {
    STB_U32 bytes_saved;
    bytes_saved = 32 - sizeof(STB_U64); // converting a minimal subtitle info (32 bytes) to binary saves this many bytes
    
    // now we can figure out how much to compact
    compact = ( (bytes_needed+7)+((bytes_saved+1)/2) ) / bytes_saved; // plus 7 for possible alignment
  }

  // if we have to compact, do those first
  if ( compact )
  {
    info = alloca( sizeof(STB_U64) * compact );
    for( i = 0 ; i < compact ; i++ )
    {
      STB_U32 len;
      char * str;

      if ( in >= end )  // parsed past end of file?
      {
        bytes_needed -= ( sizeof(STB_U64) * (num-i) );
        compact = num = i;
        break;
      }

      if ( !stb_sub_parse( &in, info, i, &str, &len, end, must_sort, &last_ms ) )
        goto parse_error;

      // has something gone very wrong in the compacting?
      if ( ( out + len ) > in )
        goto compact_error;

      stb_sub_memcpy( out, str, len );
      // string ptr offset
      info[ i ] |= ( out - base );
      out += len;

      // have we done enough and scan stop early?
      if ( (STB_U32) ( in - out ) >= ( bytes_needed + 7 ) )
      {
        compact = i + 1;
        break;
      }
    }

    {
      // assume the base is aligned
      STB_U32 align = ( (STB_U32) ( out - ((char*)0) ) ) & 7;
      if ( ( align ) && ( num > 1 ) ) // only align if we have more than 1 sub (with 1, we need every byte)
        out += ( 8 - align );
    }

    // have we not compacted enough somehow? Something gone wrong, error!
    if ( (STB_U32) ( in - out ) < bytes_needed )
    {
     compact_error: 
      if ( err ) *err = stb_srt_compact_error;
      return 0;
    }

    // move over the binary subtitle info into final position
    stb_sub_memcpy( out + sizeof(STB_SRT_SUB), info, sizeof(STB_U64) * compact );
  } 

  // we have room one way or the other now (leave room for header)
  info = (STB_U64 *) ( out + sizeof(STB_SRT_SUB) );

  // do the remaining frames (that aren't compacted)
  for( i = compact ; i < num ; i++ )
  {
    STB_U32 len;
    char * str;
    
    if ( in >= end )  // parsed past end of file?
      break;

    if ( !stb_sub_parse( &in, info, i, &str, &len, end, must_sort, &last_ms ) )
      goto parse_error;

    // string ptr offset
    info[ i ] |= ( str - base );
  }
  
  {
    STB_SRT_SUB * ret = ( (STB_SRT_SUB*)info ) - 1;
    ret->count = i;
    ret->last_ms = last_ms;
    ret->base = (signed int)( (char*)image - (char*)ret );
    if ( ( utf8 ) || ( csflags & 1 ) )
      ret->char_set = 2;
    else
      ret->char_set = ( csflags & 2 ) ? 1 : 0;
    return ret;
  }
}

STB_SRT_SUB * stb_srt_parse_image( void * image, void * image_end, stb_srt_error * err )
{
  int must_sort = 0;
  STB_SRT_SUB * ret = stb_srt_parse( image, image_end, err, &must_sort );
  STB_U64 * info = (STB_U64*) ( ret + 1 );

  if ( ret == 0 )
    return 0;

  // if any were out of order, sort them (sort here so that the alloca in stb_srt_parse is freed before the stack mem in sort)
  if ( must_sort )
    stb_sub_sort_U64( info, info + ret->count - 1 );

  return ret;
} 

char const * stb_srt_get_subtitle( STB_SRT_SUB * sub, unsigned int index, unsigned int * out_low_ms, unsigned int * out_high_ms )
{
  STB_U64 * info = (STB_U64*) ( sub + 1 );
  STB_U32 low;
  char const * str;
  
  if ( index >= sub->count )
    return 0;
  
  low = (STB_U32) ( info[ index ] >> STB_LOW_MS_BITS_START );
  *out_low_ms = low;
  *out_high_ms = low + ( ( (STB_U32) ( info[ index ] >> STB_HIGH_MS_BITS_START ) ) & STB_HIGH_MS_BITS_MASK );
  
  str = ( (char*)sub ) + sub->base;
  str += ( (STB_U32) info[ index ] ) & STB_OFFSET_BITS_MASK;

  return str;
}

#define STB_SCANBACK_COUNT 4

char const * stb_srt_find_subtitle( STB_SRT_SUB * sub, unsigned int milliseconds, unsigned int * iterate, unsigned int * out_low_ms, unsigned int * out_high_ms )
{
  STB_U64 * info = (STB_U64*) ( sub + 1 );
  STB_U32 f, scan_end, ind;

  // if before first or after last, fail
  if ( ( milliseconds < ( (STB_U32) ( info[0] >> STB_LOW_MS_BITS_START ) ) ) || ( milliseconds > sub->last_ms ) )
    return 0;
  
  // otherwise search for our ms, and then back up a bit
  f = stb_sub_find_U64( info, sub->count, ( (STB_U64) milliseconds ) << STB_LOW_MS_BITS_START );
  // back up to linearly scan four values before (in case of overlapped ranges - we see them 4 away max)
  f = ( f >= STB_SCANBACK_COUNT ) ? ( f - STB_SCANBACK_COUNT ) : 0;

  // setup index countdown (if no index pointer, just always return the first match)
  ind = 0; if ( iterate ) ind = *iterate;

  // setup the end of the scan
  scan_end = f + ( STB_SCANBACK_COUNT + STB_SWITCH_TO_LINEAR_SEARCH + 1 );
  if ( scan_end > sub->count )
    scan_end = sub->count;

  // quick linear scan of the area
  while ( f < scan_end )
  {
    STB_U64 s = info[ f++ ];
    STB_U32 lt = (STB_U32) ( s >> STB_LOW_MS_BITS_START );

    if ( lt <= milliseconds ) 
    {
      STB_U32 ht = lt + ( ( (STB_U32) ( s >> STB_HIGH_MS_BITS_START ) ) & STB_HIGH_MS_BITS_MASK );
      if ( ht >= milliseconds )
      {
        // is this is the right index off the first one? (for iterating overlaps)
        if ( ind-- == 0 )
        {
          char const * str;
          if ( out_low_ms ) *out_low_ms = lt;
          if ( out_high_ms ) *out_high_ms = ht;

          str = ( (char*)sub ) + sub->base;
          str += ( (STB_U32) s ) & STB_OFFSET_BITS_MASK;
          if ( iterate ) ind = ++(*iterate);
          return str;
        }
      }
    }
  } 

  return 0;
}

#undef STB_SCANBACK_COUNT
#undef STB_SWITCH_TO_LINEAR_SEARCH 
#undef STB_U32
#undef STB_U64
#undef STB_LOW_MS_BITS
#undef STB_HIGH_MS_BITS
#undef STB_OFFSET_BITS
#undef STB_HIGH_MS_BITS_START
#undef STB_OFFSET_BITS_START
#undef STB_LOW_MS_BITS_MASK
#undef STB_HIGH_MS_BITS_MASK
#undef STB_OFFSET_BITS_MASK

#endif
