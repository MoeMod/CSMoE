// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#ifndef __EXPAND2H__
#define __EXPAND2H__

#ifndef __BINKH__
#include "bink.h"
#endif

#include "varbits.h"

#define MININTRA 0
#define MAXINTRA 2047
#define MININTER25 -2047
#define MAXINTER25 2047

#define OLDMININTER -1023
#define OLDMAXINTER 1023

#define BLK_DCT   0
#define BLK_SKIP  1
#define BLK_MNR   2  // motion, no residual
#define BLK_MDCT  3

#define INITIAL_CONTROL_4_PREDICT 0
#define INITIAL_CONTROL_1_PREDICT 0
#define INITIAL_Q_PREDICT 16

#define BINK22_MIN_DC_Q 8

#define FULLGRADIANTCLAMPMASK    0xff
#define BINKVER22              0x2000                 
#define BINKVER23BLURBITS     0x10000                 
#define BINKVER23EDGEMOFIX    0x20000                 
#define BINKVER25             0x40000  // turns off interior blur and uses bigger inter-clamp
#define BINKCONSTANTA         0x80000  
#define BINKCONSTANTAVMASK 0xff000000

extern U8 const bink_zigzag_scan_order[64];

// Bink2.2 quant factors are simply pow2(Q / 4.0f)
#define QUANT_FACTORS \
  T(  1.0000f) \
  T(  1.1892f) \
  T(  1.4142f) \
  T(  1.6818f) \
  T(  2.0000f) \
  T(  2.3784f) \
  T(  2.8284f) \
  T(  3.3636f) \
  T(  4.0000f) \
  T(  4.7568f) \
  T(  5.6569f) \
  T(  6.7272f) \
  T(  8.0000f) \
  T(  9.5137f) \
  T( 11.3137f) \
  T( 13.4543f) \
  T( 16.0000f) \
  T( 19.0273f) \
  T( 22.6274f) \
  T( 26.9087f) \
  T( 32.0000f) \
  T( 38.0546f) \
  T( 45.2548f) \
  T( 53.8174f) \
  T( 64.0000f) \
  T( 76.1093f) \
  T( 90.5097f) \
  T(107.6347f) \
  T(128.0000f) \
  T(152.2185f) \
  T(181.0193f) \
  T(215.2695f) \
  T(256.0000f) \
  T(304.4370f) \
  T(362.0387f) \
  T(430.5390f) \
  T(512.0000f)

typedef U16 BINK2_SCALETAB[64];

extern S32 const bink2_fix10_dc_dequant[ 37 ];
extern RAD_ALIGN( BINK2_SCALETAB const, bink_intra_luma_scaleT[ 4 ], RR_CACHE_LINE_SIZE );
extern RAD_ALIGN( BINK2_SCALETAB const, bink_intra_chroma_scaleT[ 4 ], RR_CACHE_LINE_SIZE );
extern RAD_ALIGN( BINK2_SCALETAB const, bink_inter_scaleT[ 4 ], RR_CACHE_LINE_SIZE );

extern U8 slice_mask_to_count[4];
#define SliceMaskToCount(mask) ((U32)slice_mask_to_count[( mask/BINK_SLICES_3 )&3])
void setup_slices( U32 marker, U32 openflags, U32 width, U32 height, BINKSLICES * harray );
U32 get_slice_range( BINKSLICES * slices, U32 per,U32 perdiv ); // in binkread

void bink2_average_4_8x8( S32 *out, U8 const * RADRESTRICT px, U32 pitch );
void bink2_average_8x8( S32 *out, U8 const * RADRESTRICT px, U32 pitch );
void bink2_average_2_8x8( S32 * RADRESTRICT out, U8 const * RADRESTRICT ptr, U32 pitch ); // can be any offset on x86

void bink2_get_chroma_modata( U8 * RADRESTRICT dest, U8 * RADRESTRICT topsrc, U32 x, U32 y, S32 dw, S32 sw );
void bink2_get_luma_modata( U8 * RADRESTRICT dest, U8 * RADRESTRICT src, U32 x, U32 y, S32 dw, S32 sw );
S32 bink2_idct_and_deswizzle( U8 * out, S16 * acs, S32 const * dcs, U32 ow, U32 last_acs, U8 * no_blur_out, U32 bw );
void bink2_idct_and_mo_deswizzle( U8 * out, S16 * acs, U8 const * mo, S32 const * dcs, U32 ow, U32 mw, U32 last_acs );

// full-pel motion vectors ((x&1)==0) get clamped to [0,max_x-16*2].
// half-pel motion vectors have a slightly smaller allowed range because
// the 6-tap filter requires extra samples.
U32 bink2_clamp_mo_component_old_comp( U32 x, U32 max_x );
U32 bink2_clamp_mo_component_new_comp( U32 x, U32 max_x );

RADDEFFUNC void RADLINK ExpandBink2GlobalInit( void );

RADDEFFUNC U32 RADLINK ExpandBink2( BINKFRAMEBUFFERS * frameset, 
                                   void * comp, S32 alpha, S32 hdr, S32 key, void * compinfo,
                                   BINKSLICES * slices, U32 slicecontrol, void * seam, BINKGPUDATABUFFERS * gpu );

RADDEFFUNC void RADLINK ExpandBink2SplitFinish( BINKFRAMEBUFFERS * b, BINKSLICES * slices, S32 alpha, S32 hdr, S32 key, void * seamp );


// for blur bit encoding/decoding
#define RLE_LEN 5  // 
#define LIT_LEN 5  // 1 to (1<<LIT_LEN+1)
#define MAX_RLE 1
#define LITERAL 2
#define EARLY_RLE 3


#if defined(N_BINKPLNC) && defined(INC_BINK2)
#define USE_DWRITE
#define USE_DWRITEDUMP
#endif

#if defined(USE_DWRITE) && defined(__RADWIN__)
void DWRITE8( char const * file_str, int file_num, int value, int max );
#define DWRITECODE(...) __VA_ARGS__
#else
#define DWRITE8(...)
#define DWRITECODE(...) 
#endif

#if defined(USE_DWRITEDUMP) && defined(__RADWIN__)
void DWRITEDUMP( void );
#else
#define DWRITEDUMP(...)
#endif

#endif

