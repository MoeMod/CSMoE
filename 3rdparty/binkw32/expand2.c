// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#ifdef INC_BINK2

#ifndef NTELEMETRY
  // define when you want to block detailed stats
  //#define NTELEMETRY  
#endif

#include "binkproj.h"

#include "binktm.h" // @cdep ignore

#define SEGMENT_NAME "expand2"
#include "binksegment.h"

#if defined(_MSC_VER)
#pragma warning(disable: 4101)
#pragma warning(disable: 4189)
#endif

#include "expand2.h"
#include "binkcomm.h"
#include "varbits.h"
#include "rrstring.h"
#include "binkmarkers.h"
#include "cpu.h"
#include "binkalloca.h"

#define CODEGEN_ATTR
#define COEFF_ALIGN 16
#define RAD_ALIGN_SIMD(type, name) RAD_ALIGN(type, name, 16)

#ifndef NO_BINK20
  #define BINK20_SUPPORT
#endif

#if defined( __RADARM__ )

#include <arm_neon.h>

#include "neon_new_dc_blur.inl"
#include "neon_mublock_vertical_masked.inl"
#include "neon_mublock_UandD_nogen.inl"
#include "neon_fancypants_utilities.inl"
#include "neon_mublock16.inl"
#include "neon_bilinear_8x8.inl"
#ifndef CUSTOM_MURATORI32_16x16
  #include "neon_muratori32_16x16.inl"
#endif
#ifndef CUSTOM_INT_IDCT
  #include "neon_int_idct.inl"
#endif

#define DONT_PREFETCH_DEST // default to not prefetching dest (usually worse than doing nothing - test!)
#ifdef _MSC_VER
#define PrefetchForRead(ptr) __prefetch((ptr))
#define PrefetchForOverwrite(ptr) __prefetch((ptr))
#else
#define PrefetchForRead(ptr) __builtin_prefetch((ptr), 0)
#define PrefetchForOverwrite(ptr) __builtin_prefetch((ptr), 1)
#endif

#elif defined( __RADX86__ ) 

#ifdef _MSC_VER
#include <intrin.h>
#endif
#include <emmintrin.h>

#ifdef RAD_USES_SSSE3
#include <tmmintrin.h>
#endif

// NOTE(fabiang): Tag the 8-bit mublocks as noinline. This is mostly for the benefit of
// Clang, which gets overzealous with inlining into deblock() otherwise.
#undef CODEGEN_ATTR
#define CODEGEN_ATTR RADNOINLINE
  #include "sse_new_dc_blur.inl"
  #include "sse_mublock_vertical_masked.inl"
  #include "sse_mublock_UandD_nogen.inl"
  #include "sse_fancypants_utilities.inl"
#undef CODEGEN_ATTR
#define CODEGEN_ATTR
#include "sse_mublock16.inl"
#include "sse_int_idct.inl"
#include "sse_bilinear_8x8.inl"

#ifdef RAD_USES_SSSE3
#undef CODEGEN_ATTR
#define CODEGEN_ATTR RAD_USES_SSSE3
#include "ssse3_ssse3_muratori32_16x16.inl"
#undef CODEGEN_ATTR
#define CODEGEN_ATTR
#endif                 

#if !defined(RAD_GUARANTEED_SSE3) || !defined(RAD_USES_SSSE3)
  #include "sse_sse2_muratori32_16x16.inl"
  
  #if !defined(RAD_USES_SSSE3)              
    #define muratori32_16x16 sse2_muratori32_16x16
  #else
    static sse2_muratori32_16x16_jump * muratori32_16x16[4];
  #endif
#else
  #define muratori32_16x16 ssse3_muratori32_16x16
#endif

// NOTE(fabiang): This combination was found to be a Win on both regular
// PCs and consoles. Prefetching motion data helps, prefetching the destination
// buffer just makes things worse.
#define DONT_PREFETCH_DEST
#define PrefetchForRead(ptr) _mm_prefetch((char*)(ptr), _MM_HINT_T0 )
//#define PrefetchForOverwrite(ptr) _mm_prefetch((char*)(ptr), _MM_HINT_T0 )

#elif defined( __RADEMSCRIPTEN__ )

#define ClampToVBT_8x_U8(x) ClampIntToU8(x)
#define ClampToVBT_16x_U8(x) ClampIntToU8(x)

// align DCT coeffs at 32-byte granularity so we can clear them with dcbz
#undef COEFF_ALIGN
#undef RAD_ALIGN_SIMD
#define COEFF_ALIGN 32

#define RAD_ALIGN_SIMD(type, name) type name

static RADINLINE U8 ClampIntToU8(int x)
{
  if( (unsigned) x < 256 )
    return x;
  else
    return ( x < 0 ) ? 0 : 255;
}

#include "ansic_new_dc_blur_nogen.inl"
#include "ansic_mublock_nogen.inl"
#include "ansic_fancypants_utilities_nogen.inl"
#include "ansic_mublock16_nogen.inl"
#include "ansic_int_idct_nogen.inl"
#include "ansic_bilinear_8x8_nogen.inl"
#include "ansic_muratori32_16x16_nogen.inl"

#elif defined( __RAD_NDA_PLATFORM__ )

// @cdep pre $AddPlatformInclude
#include RR_PLATFORM_PATH_STR( __RAD_NDA_PLATFORM__, _expand2.h )

#else

#error "Undefined platform!"

#endif

//#define NO_EDGE_BLUR
//#define NO_FULL_BLUR                 

#define Y_NO_BOTTOM_16        0x1  // used to tell bink not to do the bottom 16
#define Y_MID_TOP_T1          0x2   // MID_TOP on thread 1
#define Y_MID_TOP_T2          0x4  // MID_TOP on thread 2
#define Y_MID_TOP             0x8
#define Y_MID_TOP_T2_NEXT    0x10 // one after MID_TOP on thread 2
#define X_LEFT               0x20
#define X_RIGHT              0x40
#define Y_TOP                0x80
#define Y_BOTTOM            0x100
#define X_2ND_BLOCK         0x200                
#define X_LAST              0x400   // used on the final call to deblock (after X_RIGHT)

// defined in expand.h, but or-ed into the flags var (listed here to remember positions)
#define PH_SLICE_MASK      0x1800 // 0x800=3, 0x1000=4, 0x1800=8
#define PH_BINK22          0x2000
#define PH_UANDD_OFF       BINK_DEBLOCK_UANDD_OFF
#define PH_LTOR_OFF        BINK_DEBLOCK_LTOR_OFF                 

#define IS_KEY             0x10000  // is a key frame
#define PH_BINK23EDGEMOFIX 0x20000  // this uses the new edge clamping

// these defines are only used in deblock!!!!
#define NO_UANDD_MASKS 0x03fc0000
#define NO_UANDD_LL3   0x00040000 // NO U and D on the 3rd 8x8 col of the left of the left 32x32
#define NO_UANDD_L0    0x00080000 // NO U and D on the 1st 8x8 col of the left 32x32
#define NO_UANDD_L1    0x00100000 // NO U and D on the 2nd 8x8 col of the left 32x32
#define NO_UANDD_L2    0x00200000 // NO U and D on the 3rd 8x8 col of the left 32x32
#define NO_UANDD_L3    0x00400000 // NO U and D on the 4th 8x8 col of the left 32x32
#define NO_UANDD_C0    0x00800000 // NO U and D on the 1st 8x8 col of the current 32x32
#define NO_UANDD_C1    0x01000000 // NO U and D on the 2nd 8x8 col of the current 32x32
#define NO_UANDD_C2    0x02000000 // NO U and D on the 3rd 8x8 col of the current 32x32

#define NO_LTOR_MASKS  0xfc000000
#define NO_LTOR_A1     0x04000000 // NO L to R on the last 8x8 row of the above 32x32
#define NO_LTOR_A2     0x08000000 // NO L to R on the last 8x8 row of the above 32x32
#define NO_LTOR_A3     0x10000000 // NO L to R on the last 8x8 row of the above 32x32
#define NO_LTOR_C0     0x20000000 // NO L to R on the 1st 8x8 row of the current 32x32
#define NO_LTOR_C1     0x40000000 // NO L to R on the 2nd 8x8 row of the current 32x32
#define NO_LTOR_C2     0x80000000 // NO L to R on the 3rd 8x8 row of the current 32x32

// DC block type flags
#define DC_INTER    0x0000
#define DC_INTRA    0x8000

#define ENCODE_DC_FLAGS( dc, deblock_strength, type ) ( (U16) ( ( (dc) & 0x1fff ) | ( (deblock_strength) << 13 ) | (type) ) )

typedef struct BLOCK_DCT
{
  // each U64 is 4 U16s
  U64 ydcts[4]; 
  U64 cdcts[2]; // cr then cr
  U64 adcts[4];
  U64 hdcts[4];
} BLOCK_DCT;

// for GPU
typedef struct PLANEPTRS {
  U16 * RADRESTRICT Dc;
  U8 * RADRESTRICT AcStart;
  U8 * RADRESTRICT AcCur;
  U32 * RADRESTRICT AcOffs;
} PLANEPTRS;


#if defined(__RADX86__) || !defined(NO_BINK20)
  static RAD_ALIGN_SIMD( S32, zerosd[4] ) ={0,0,0,0};
#endif

#ifdef BINK20_SUPPORT
  static RAD_ALIGN_SIMD( S32, intraflagd[4] ) ={0x04040404,0x04040404,0x04040404,0x04040404};
  #define zeros (*((simd4i*)zerosd))
  #define intraflag (*((simd4i*)intraflagd))
  #define interflag zeros
#endif

//#include "dwrite.inl"

// Generated by tools/quant_tables.py, fixbits=10
RRSTRIPPABLEPUB RAD_ALIGN( BINK2_SCALETAB const, bink_intra_luma_scaleT[ 4 ], RR_CACHE_LINE_SIZE ) =
{
  {
     1024, 1432, 1506, 1181, 1843, 2025, 5271, 8592,
     1313, 1669, 1630, 1672, 2625, 3442, 8023,12794,
     1076, 1755, 1808, 1950, 3980, 4875, 8813,11909,
     1350, 1868, 2127, 2016, 4725, 4450, 7712, 9637,
     2458, 3103, 4303, 4303, 6963, 6835,11079,13365,
     3375, 5704, 5052, 6049, 9198, 7232,10725, 9834,
     5486, 7521, 7797, 7091,11079,10016,13559,12912,
     7279, 7649, 7020, 6097, 9189, 9047,12661,13768,
  },
  {
     1218, 1703, 1791, 1405, 2192, 2408, 6268,10218,
     1561, 1985, 1938, 1988, 3122, 4093, 9541,15215,
     1279, 2087, 2150, 2319, 4733, 5798,10481,14162,
     1606, 2222, 2530, 2398, 5619, 5292, 9171,11460,
     2923, 3690, 5117, 5118, 8281, 8128,13176,15894,
     4014, 6783, 6008, 7194,10938, 8600,12755,11694,
     6524, 8944, 9272, 8433,13176,11911,16125,15354,
     8657, 9096, 8348, 7250,10927,10759,15056,16373,
  },
  {
     1448, 2025, 2130, 1671, 2607, 2864, 7454,12151,
     1856, 2360, 2305, 2364, 3713, 4867,11346,18094,
     1521, 2482, 2557, 2758, 5628, 6894,12464,16841,
     1909, 2642, 3008, 2852, 6683, 6293,10906,13629,
     3476, 4388, 6085, 6086, 9847, 9666,15668,18901,
     4773, 8066, 7145, 8555,13007,10227,15168,13907,
     7758,10637,11026,10028,15668,14165,19175,18259,
    10294,10817, 9927, 8622,12995,12794,17905,19470,
  },
  {
     1722, 2408, 2533, 1987, 3100, 3406, 8864,14450,
     2208, 2807, 2741, 2811, 4415, 5788,13493,21517,
     1809, 2951, 3041, 3280, 6693, 8199,14822,20028,
     2271, 3142, 3578, 3391, 7947, 7484,12969,16207,
     4133, 5218, 7236, 7238,11711,11495,18633,22478,
     5677, 9592, 8497,10174,15469,12162,18038,16538,
     9226,12649,13112,11926,18633,16845,22804,21715,
    12242,12864,11806,10254,15454,15215,21293,23155,
  },
};

RRSTRIPPABLEPUB RAD_ALIGN( BINK2_SCALETAB const, bink_intra_chroma_scaleT[ 4 ], RR_CACHE_LINE_SIZE ) =
{
  {
     1024, 1193, 1434, 2203, 5632, 4641, 5916, 6563,
     1193, 1622, 1811, 3606, 6563, 5408, 6894, 7649,
     1434, 1811, 3515, 4875, 5916, 4875, 6215, 6894,
     2203, 3606, 4875, 3824, 4641, 3824, 4875, 5408,
     5632, 6563, 5916, 4641, 5632, 4641, 5916, 6563,
     4641, 5408, 4875, 3824, 4641, 3824, 4875, 5408,
     5916, 6894, 6215, 4875, 5916, 4875, 6215, 6894,
     6563, 7649, 6894, 5408, 6563, 5408, 6894, 7649,
  },
  {
     1218, 1419, 1706, 2620, 6698, 5519, 7035, 7805,
     1419, 1929, 2153, 4288, 7805, 6432, 8199, 9096,
     1706, 2153, 4180, 5798, 7035, 5798, 7390, 8199,
     2620, 4288, 5798, 4548, 5519, 4548, 5798, 6432,
     6698, 7805, 7035, 5519, 6698, 5519, 7035, 7805,
     5519, 6432, 5798, 4548, 5519, 4548, 5798, 6432,
     7035, 8199, 7390, 5798, 7035, 5798, 7390, 8199,
     7805, 9096, 8199, 6432, 7805, 6432, 8199, 9096,
  },
  {
     1448, 1688, 2028, 3116, 7965, 6563, 8367, 9282,
     1688, 2294, 2561, 5099, 9282, 7649, 9750,10817,
     2028, 2561, 4971, 6894, 8367, 6894, 8789, 9750,
     3116, 5099, 6894, 5408, 6563, 5408, 6894, 7649,
     7965, 9282, 8367, 6563, 7965, 6563, 8367, 9282,
     6563, 7649, 6894, 5408, 6563, 5408, 6894, 7649,
     8367, 9750, 8789, 6894, 8367, 6894, 8789, 9750,
     9282,10817, 9750, 7649, 9282, 7649, 9750,10817,
  },
  {
     1722, 2007, 2412, 3706, 9472, 7805, 9950,11038,
     2007, 2729, 3045, 6064,11038, 9096,11595,12864,
     2412, 3045, 5912, 8199, 9950, 8199,10452,11595,
     3706, 6064, 8199, 6432, 7805, 6432, 8199, 9096,
     9472,11038, 9950, 7805, 9472, 7805, 9950,11038,
     7805, 9096, 8199, 6432, 7805, 6432, 8199, 9096,
     9950,11595,10452, 8199, 9950, 8199,10452,11595,
    11038,12864,11595, 9096,11038, 9096,11595,12864,
  },
};

RRSTRIPPABLEPUB RAD_ALIGN( BINK2_SCALETAB const, bink_inter_scaleT[ 4 ], RR_CACHE_LINE_SIZE ) =
{
  {
     1024, 1193, 1076,  844, 1052,  914, 1225, 1492,
     1193, 1391, 1254,  983, 1227, 1065, 1463, 1816,
     1076, 1254, 1161,  936, 1195, 1034, 1444, 1741,
      844,  983,  936,  811, 1055,  927, 1305, 1584,
     1052, 1227, 1195, 1055, 1451, 1336, 1912, 2354,
      914, 1065, 1034,  927, 1336, 1313, 1945, 2486,
     1225, 1463, 1444, 1305, 1912, 1945, 3044, 4039,
     1492, 1816, 1741, 1584, 2354, 2486, 4039, 5679,
  },
  {
     1218, 1419, 1279, 1003, 1252, 1087, 1457, 1774,
     1419, 1654, 1491, 1169, 1459, 1267, 1739, 2159,
     1279, 1491, 1381, 1113, 1421, 1230, 1717, 2070,
     1003, 1169, 1113,  965, 1254, 1103, 1552, 1884,
     1252, 1459, 1421, 1254, 1725, 1589, 2274, 2799,
     1087, 1267, 1230, 1103, 1589, 1562, 2313, 2956,
     1457, 1739, 1717, 1552, 2274, 2313, 3620, 4803,
     1774, 2159, 2070, 1884, 2799, 2956, 4803, 6753,
  },
  {
     1448, 1688, 1521, 1193, 1488, 1293, 1732, 2110,
     1688, 1967, 1773, 1391, 1735, 1507, 2068, 2568,
     1521, 1773, 1642, 1323, 1690, 1462, 2042, 2462,
     1193, 1391, 1323, 1147, 1492, 1311, 1845, 2241,
     1488, 1735, 1690, 1492, 2052, 1889, 2704, 3328,
     1293, 1507, 1462, 1311, 1889, 1857, 2751, 3515,
     1732, 2068, 2042, 1845, 2704, 2751, 4306, 5712,
     2110, 2568, 2462, 2241, 3328, 3515, 5712, 8031,
  },
  {
     1722, 2007, 1809, 1419, 1770, 1537, 2060, 2509,
     2007, 2339, 2108, 1654, 2063, 1792, 2460, 3054,
     1809, 2108, 1953, 1574, 2010, 1739, 2428, 2928,
     1419, 1654, 1574, 1364, 1774, 1559, 2195, 2664,
     1770, 2063, 2010, 1774, 2440, 2247, 3216, 3958,
     1537, 1792, 1739, 1559, 2247, 2209, 3271, 4181,
     2060, 2460, 2428, 2195, 3216, 3271, 5120, 6793,
     2509, 3054, 2928, 2664, 3958, 4181, 6793, 9550,
  },
};


RRSTRIPPABLEPUB U8 const bink_zigzag_scan_order[64] =
{
  000,
  001, 010,
  020, 011, 002,
  003, 012, 021, 030,
  040, 031, 022, 013, 004,
  005, 014, 023, 032, 041, 050,
  060, 051, 042, 033, 024, 015, 006,
  007, 016, 025, 034, 043, 052, 061, 070,
  071, 062, 053, 044, 035, 026, 017,
  027, 036, 045, 054, 063, 072,
  073, 064, 055, 046, 037,
  047, 056, 065, 074,
  075, 066, 057,
  067, 076,
  077
};

// align this table so it's in a single cache line
RRSTRIPPABLE RAD_ALIGN( U8, const bink_zigzag_scan_order_transposed[64], 64 ) =
{
  000,
  010, 001,
  002, 011, 020,
  030, 021, 012, 003,
  004, 013, 022, 031, 040,
  050, 041, 032, 023, 014, 005,
  006, 015, 024, 033, 042, 051, 060,
  070, 061, 052, 043, 034, 025, 016, 007,
  017, 026, 035, 044, 053, 062, 071,
  072, 063, 054, 045, 036, 027,
  037, 046, 055, 064, 073,
  074, 065, 056, 047,
  057, 066, 075,
  076, 067,
  077
};

#define BINK22_SPARSE_LAST_COEFF  9 // if scan order index larger than this, don't use sparse IDCT. (9=sparse is 4x4)

RRSTRIPPABLEPUB S32 const bink2_fix10_dc_dequant[ 37 ] = 
{
#define T(x) (S32) ( (x) * 1024.0f + 0.5f ),
  QUANT_FACTORS
#undef T
};


RRSTRIPPABLEPUB RAD_ALIGN( U16 const, bink_hmask[8*16], RR_CACHE_LINE_SIZE ) =
{
   0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
   0x0000,0xffff,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
   0xffff,0xffff,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
   0xffff,0xffff,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

   0x0000,0x0000,0xffff,0x0000,0x0000,0x0000,0x0000,0x0000,
   0x0000,0xffff,0xffff,0x0000,0x0000,0x0000,0x0000,0x0000,
   0xffff,0xffff,0xffff,0x0000,0x0000,0x0000,0x0000,0x0000,
   0xffff,0xffff,0xffff,0x0000,0x0000,0x0000,0x0000,0x0000,

   0x0000,0x0000,0xffff,0xffff,0x0000,0x0000,0x0000,0x0000,
   0x0000,0xffff,0xffff,0xffff,0x0000,0x0000,0x0000,0x0000,
   0xffff,0xffff,0xffff,0xffff,0x0000,0x0000,0x0000,0x0000,
   0xffff,0xffff,0xffff,0xffff,0x0000,0x0000,0x0000,0x0000,

   0x0000,0x0000,0xffff,0xffff,0x0000,0x0000,0x0000,0x0000,
   0x0000,0xffff,0xffff,0xffff,0x0000,0x0000,0x0000,0x0000,
   0xffff,0xffff,0xffff,0xffff,0x0000,0x0000,0x0000,0x0000,
   0xffff,0xffff,0xffff,0xffff,0x0000,0x0000,0x0000,0x0000,
};

// mublock16_near mask: strength >= 1  (index with offset 8)
// mublock15_far mask: strength >= 2   (index with offset 0)
RRSTRIPPABLE RAD_ALIGN_SIMD( U16 const, bink_mublock16_masks[8*5] ) =
{
  0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
  0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
  0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,
  0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,
  0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,
};

#define move16x16(d,s,p) move_u8_16x16(d,p,s,p)
#define move16x8(d,s,p)  move_u8_16x8(d,p,s,p)

#define set_u8_top8_and_mo_comp( dest, mot, dcs, dw, mw )       set_u8_top8_with_mo_comp( (U8*)(dest), (U8*)(mot), (U8*)(dcs), dw, mw )
#define set_u8_and_mo_comp( dest, mot, dcs, dw, mw )            set_u8_with_mo_comp( (U8*)(dest), ((U8*)(dest))+(dw)*8, (U8*)(mot), (U8*)(dcs), dw, dw, mw ) 
#define set_u8_and_mo_comp_tb( top, bot, mot, dcs, tw, bw, mw ) set_u8_with_mo_comp( (U8*)(top), (U8*)(bot), (U8*)(mot), (U8*)(dcs), tw, bw, mw ) 

#ifndef NTELEMETRY

//#define TMDETAILED
#ifdef TMDETAILED
typedef struct TMTIMESPLANE
{
  TmU64 idct;
  TmU64 decodeDCs;
  TmU64 decodeACs;
  TmU64 deswizzle;
  TmU64 deswizzle_mot;
  TmU64 mosample;
  TmU64 copy;
  TmU64 clear;
  TmU64 commit;
  TmU64 mdct_deblock;
  TmU64 dct_average;
} TMTIMESPLANE;

typedef struct TMTIMES
{
  TmU64 decodemos;
  TmU64 deblock;
  TmU64 fullblur;
  TmU64 uanddblur;
  TmU64 ltorblur;

  TMTIMESPLANE luma;
  TMTIMESPLANE chroma;
  TMTIMESPLANE alpha;
  TMTIMESPLANE hdr;
} TMTIMES;

#define OPT_PROFILE TMTIMES * tm,
#define OPT_PROFILE_PLANE TMTIMESPLANE * tm,
#define OPT_PROFILE_PASS(val) val, 

static void emitsubzones( TMTIMESPLANE * t, TmU64 * start, char const * desc )
{
  tmEmitAccumulationZone( tm_mask, TMZF_PLOT_TIME_EXPERIMENTAL, start, 0, t->clear, "%s clear", desc );
  tmEmitAccumulationZone( tm_mask, TMZF_PLOT_TIME_EXPERIMENTAL, start, 0, t->idct, "%s iDCT", desc );
  tmEmitAccumulationZone( tm_mask, TMZF_PLOT_TIME_EXPERIMENTAL, start, 0, t->decodeDCs, "%s DC decode", desc );
  tmEmitAccumulationZone( tm_mask, TMZF_PLOT_TIME_EXPERIMENTAL, start, 0, t->decodeACs, "%s AC decode", desc );
  tmEmitAccumulationZone( tm_mask, TMZF_PLOT_TIME_EXPERIMENTAL, start, 0, t->deswizzle, "%s deswizzle", desc );
  tmEmitAccumulationZone( tm_mask, TMZF_PLOT_TIME_EXPERIMENTAL, start, 0, t->deswizzle_mot, "%s deswizzle with motion", desc );
  tmEmitAccumulationZone( tm_mask, TMZF_PLOT_TIME_EXPERIMENTAL, start, 0, t->mosample, "%s motion sample", desc );
  tmEmitAccumulationZone( tm_mask, TMZF_PLOT_TIME_EXPERIMENTAL, start, 0, t->copy, "%s copy", desc );
  tmEmitAccumulationZone( tm_mask, TMZF_PLOT_TIME_EXPERIMENTAL, start, 0, t->commit, "%s commit", desc );
  tmEmitAccumulationZone( tm_mask, TMZF_PLOT_TIME_EXPERIMENTAL, start, 0, t->mdct_deblock, "%s mdct deblock", desc );
  tmEmitAccumulationZone( tm_mask, TMZF_PLOT_TIME_EXPERIMENTAL, start, 0, t->dct_average, "%s dct_average", desc );
}

static void emitzones( TMTIMES * t, TmU64 * start, S32 do_alpha, S32 do_hdr )
{
  tmEmitAccumulationZone( tm_mask, TMZF_PLOT_TIME_EXPERIMENTAL, start, 0, t->decodemos, "Mo-vec decode" );
  tmEmitAccumulationZone( tm_mask, TMZF_PLOT_TIME_EXPERIMENTAL, start, 0, t->deblock-t->fullblur-t->uanddblur-t->ltorblur, "Deblock" );
  tmEmitAccumulationZone( tm_mask, TMZF_PLOT_TIME_EXPERIMENTAL, start, 0, t->fullblur, "Full Blur" );
  tmEmitAccumulationZone( tm_mask, TMZF_PLOT_TIME_EXPERIMENTAL, start, 0, t->uanddblur, "UandD Blur" );
  tmEmitAccumulationZone( tm_mask, TMZF_PLOT_TIME_EXPERIMENTAL, start, 0, t->ltorblur, "LtoR Blur" );
  emitsubzones( &t->luma, start, "Luma" );
  emitsubzones( &t->chroma, start, "Chroma" );
  if ( do_alpha ) 
    emitsubzones( &t->alpha, start, "Alpha" );
  if ( do_hdr ) 
    emitsubzones( &t->hdr, start, "HDR" );
}

#define dtmEnterAccumulationZone tmEnterAccumulationZone
#define dtmLeaveAccumulationZone tmLeaveAccumulationZone

#define gtmEnterAccumulationZone(...)
#define gtmEnterAccumulationZoneAndInc(...)
#define gtmLeaveAccumulationZone(...)

#else

typedef struct TMTIMES
{
  U64 dct;
  U64 skip;
  U64 mnr;
  U64 mdct;
  U64 deblock;
  U64 prefetch;
  int dct_count;
  int skip_count;
  int mnr_count;
  int mdct_count;
} TMTIMES;

static void emitzones( TMTIMES * t, U64 * start, S32 do_alpha, S32 do_hdr )
{
  tmEmitAccumulationZone( tm_mask, TMZF_PLOT_TIME_EXPERIMENTAL, start, t->dct_count, t->dct, "DCT" );
  tmEmitAccumulationZone( tm_mask, TMZF_PLOT_TIME_EXPERIMENTAL, start, t->skip_count, t->skip, "Skip" );
  tmEmitAccumulationZone( tm_mask, TMZF_PLOT_TIME_EXPERIMENTAL, start, t->mnr_count, t->mnr, "Mo, No Resid" );
  tmEmitAccumulationZone( tm_mask, TMZF_PLOT_TIME_EXPERIMENTAL, start, t->mdct_count, t->mdct, "MDCT" );
  tmEmitAccumulationZone( tm_mask, TMZF_PLOT_TIME_EXPERIMENTAL, start, 0, t->deblock, "Deblock" );
  tmEmitAccumulationZone( tm_mask, TMZF_PLOT_TIME_EXPERIMENTAL, start, 0, t->prefetch, "Prefetch" );
}

#define OPT_PROFILE 
#define OPT_PROFILE_PLANE 
#define OPT_PROFILE_PASS(val) 

#define dtmEnterAccumulationZone(...)
#define dtmLeaveAccumulationZone(...)

#define gtmEnterAccumulationZone(a,b) tmEnterAccumulationZone(a,b)
#define gtmEnterAccumulationZoneAndInc(a,b) ++(b##_count); tmEnterAccumulationZone(a,b)
#define gtmLeaveAccumulationZone(a,b) tmLeaveAccumulationZone(a,b)

#endif

#else

#define OPT_PROFILE 
#define OPT_PROFILE_PLANE 
#define OPT_PROFILE_PASS(val) 

#define dtmEnterAccumulationZone(...)
#define dtmLeaveAccumulationZone(...)

#define gtmEnterAccumulationZone(...)
#define gtmEnterAccumulationZoneAndInc(...)
#define gtmLeaveAccumulationZone(...)

#endif


void bink2_get_chroma_modata( U8 * RADRESTRICT dest, U8 * RADRESTRICT topsrc, U32 x, U32 y, S32 dw, S32 sw )
{
  U8 * RADRESTRICT src;
  src = topsrc + ( x >> 2 ) + sw * ( y >> 2 );

  {
    S32 filter_mode;
    filter_mode = ( ( x & 3 ) << 2 ) | ( y & 3 );
    bilinear_8x8[ filter_mode ]( src, dest, sw, dw );
  }
}

void bink2_get_luma_modata( U8 * RADRESTRICT dest, U8 * RADRESTRICT topsrc, U32 x, U32 y, S32 dw, S32 sw )
{
  U8 * RADRESTRICT src;
  src = topsrc + ( x >> 1 ) + sw * ( y >> 1 );

  {
    S32 filter_mode;
    filter_mode = ( ( x & 1 ) << 1 ) | ( y & 1 );

    muratori32_16x16[ filter_mode ]( src, dest, sw, dw );
  }
}


static RADINLINE U32 popcount16( U32 x )
{
#ifdef POPCOUNT32_INTRINSIC
  return POPCOUNT32_INTRINSIC( x & 0xffff );
#else
  x -= ( x >> 1 ) & 0x5555; // 2-bit chunks
  x = ( x & 0x3333 ) + ( ( x >> 2 ) & 0x3333 ); // 4-bit chunks
  x = ( x + ( x >> 4 ) ) & 0x0f0f; // 8-bit chunks
  return ( x + ( x >> 8 ) ) & 0xff;
#endif
}

#if defined(__RADX86__)

static RADINLINE U32 find_first_set( U32 n )
{
#ifdef _MSC_VER
  unsigned long b;
  _BitScanForward( (unsigned long*)&b, n );
  return b;
#else
  // this is for GCC/Clang.
  return __builtin_ctz( n );
#endif
}

#define get_unary_len( n ) ( find_first_set( ~n ) + 1 )

#else

// no bit scan forward, but we can assume count leading zeros.
//
// x = ~x (complement), x &= -x (isolate first set bit)
// since -x = ~x + 1, this combines into
// "~x & (x + 1)".
#define find_first_set( n ) ( getbitlevelvar( n & (U32)( -(S32)n ) ) - 1 )
#define get_unary_len( n ) getbitlevelvar( ~n & (n + 1) )

#endif

// Reads unary code:
// 0     -> 1
// 10    -> 2
// 110   -> 3
// 1110  -> 4
// and so forth. "maxones" is the maximum number of consecutive
// ones that may occur in a valid code. After encountering
// "maxones" 1 bits, the following bit is interpreted as if it was
// a 0 bit (and consumed).
#define READ_UNARY( out, lvb, maxones ) { \
  U32 t; \
  VarBitsLocalPeek( t, U32, lvb, maxones ); \
  t = get_unary_len( t ); \
  VarBitsLocalUse( lvb, t ); \
  (out) = t; }

// decoding tables

                                                 //   0,    1,    2,    3,    4,    5,    6,    7,    8,    9,   10,   11,   12,   13,   14,   15,   16,   17,   18,   19,   20,   21,   22,   23,   24,   25,   26,   27,   28,   29,   30,   31,   32,   33,   34,   35,   36,   37,   38,   39,   40,   41,   42,   43,   44,   45,   46,   47,   48,   49,   50,   51,   52,   53,   54,   55,   56,   57,   58,   59,   60,   61,   62,   63,   64,   65,   66,   67,   68,   69,   70,   71,   72,   73,   74,   75,   76,   77,   78,   79,   80,   81,   82,   83,   84,   85,   86,   87,   88,   89,   90,   91,   92,   93,   94,   95,   96,   97,   98,   99,  100,  101,  102,  103,  104,  105,  106,  107,  108,  109,  110,  111,  112,  113,  114,  115,  116,  117,  118,  119,  120,  121,  122,  123,  124,  125,  126,  127,  128,  129,  130,  131,  132,  133,  134,  135,  136,  137,  138,  139,  140,  141,  142,  143,  144,  145,  146,  147,  148,  149,  150,  151,  152,  153,  154,  155,  156,  157,  158,  159,  160,  161,  162,  163,  164,  165,  166,  167,  168,  169,  170,  171,  172,  173,  174,  175,  176,  177,  178,  179,  180,  181,  182,  183,  184,  185,  186,  187,  188,  189,  190,  191,  192,  193,  194,  195,  196,  197,  198,  199,  200,  201,  202,  203,  204,  205,  206,  207,  208,  209,  210,  211,  212,  213,  214,  215,  216,  217,  218,  219,  220,  221,  222,  223,  224,  225,  226,  227,  228,  229,  230,  231,  232,  233,  234,  235,  236,  237,  238,  239,  240,  241,  242,  243,  244,  245,  246,  247,  248,  249,  250,  251,  252,  253,  254,  255,  256,  257,  258,  259,  260,  261,  262,  263,  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,  274,  275,  276,  277,  278,  279,  280,  281,  282,  283,  284,  285,  286,  287,  288,  289,  290,  291,  292,  293,  294,  295,  296,  297,  298,  299,  300,  301,  302,  303,  304,  305,  306,  307,  308,  309,  310,  311,  312,  313,  314,  315,  316,  317,  318,  319,  320,  321,  322,  323,  324,  325,  326,  327,  328,  329,  330,  331,  332,  333,  334,  335,  336,  337,  338,  339,  340,  341,  342,  343,  344,  345,  346,  347,  348,  349,  350,  351,  352,  353,  354,  355,  356,  357,  358,  359,  360,  361,  362,  363,  364,  365,  366,  367,  368,  369,  370,  371,  372,  373,  374,  375,  376,  377,  378,  379,  380,  381,  382,  383,  384,  385,  386,  387,  388,  389,  390,  391,  392,  393,  394,  395,  396,  397,  398,  399,  400,  401,  402,  403,  404,  405,  406,  407,  408,  409,  410,  411,  412,  413,  414,  415,  416,  417,  418,  419,  420,  421,  422,  423,  424,  425,  426,  427,  428,  429,  430,  431,  432,  433,  434,  435,  436,  437,  438,  439,  440,  441,  442,  443,  444,  445,  446,  447,  448,  449,  450,  451,  452,  453,  454,  455,  456,  457,  458,  459,  460,  461,  462,  463,  464,  465,  466,  467,  468,  469,  470,  471,  472,  473,  474,  475,  476,  477,  478,  479,  480,  481,  482,  483,  484,  485,  486,  487,  488,  489,  490,  491,  492,  493,  494,  495,  496,  497,  498,  499,  500,  501,  502,  503,  504,  505,  506,  507,  508,  509,  510,  511
RRSTRIPPABLE U8 const bink_coeff_skips1[ 512 ] = { 0x13, 0x01, 0xC2, 0x01, 0x24, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x57, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x24, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0xD5, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x24, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x39, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x24, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0xD5, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x24, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x67, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x24, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0xD5, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x24, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x49, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x24, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0xD5, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x24, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x57, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x24, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0xD5, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x24, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x79, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x24, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0xD5, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x24, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x67, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x24, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0xD5, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x24, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x88, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x24, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0xD5, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x24, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x57, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x24, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0xD5, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x24, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x99, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x24, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0xD5, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x24, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x67, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x24, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0xD5, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x24, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0xA9, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x24, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0xD5, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x24, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x57, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x24, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0xD5, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x24, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0xB9, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x24, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0xD5, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x24, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x67, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x24, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0xD5, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x24, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x88, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0x24, 0x01, 0xC2, 0x01, 0x13, 0x01, 0xC2, 0x01, 0xD5, 0x01, 0xC2, 0x01 };

                                                 //   0,    1,    2,    3,    4,    5,    6,    7,    8,    9,   10,   11,   12,   13,   14,   15,   16,   17,   18,   19,   20,   21,   22,   23,   24,   25,   26,   27,   28,   29,   30,   31,   32,   33,   34,   35,   36,   37,   38,   39,   40,   41,   42,   43,   44,   45,   46,   47,   48,   49,   50,   51,   52,   53,   54,   55,   56,   57,   58,   59,   60,   61,   62,   63,   64,   65,   66,   67,   68,   69,   70,   71,   72,   73,   74,   75,   76,   77,   78,   79,   80,   81,   82,   83,   84,   85,   86,   87,   88,   89,   90,   91,   92,   93,   94,   95,   96,   97,   98,   99,  100,  101,  102,  103,  104,  105,  106,  107,  108,  109,  110,  111,  112,  113,  114,  115,  116,  117,  118,  119,  120,  121,  122,  123,  124,  125,  126,  127,  128,  129,  130,  131,  132,  133,  134,  135,  136,  137,  138,  139,  140,  141,  142,  143,  144,  145,  146,  147,  148,  149,  150,  151,  152,  153,  154,  155,  156,  157,  158,  159,  160,  161,  162,  163,  164,  165,  166,  167,  168,  169,  170,  171,  172,  173,  174,  175,  176,  177,  178,  179,  180,  181,  182,  183,  184,  185,  186,  187,  188,  189,  190,  191,  192,  193,  194,  195,  196,  197,  198,  199,  200,  201,  202,  203,  204,  205,  206,  207,  208,  209,  210,  211,  212,  213,  214,  215,  216,  217,  218,  219,  220,  221,  222,  223,  224,  225,  226,  227,  228,  229,  230,  231,  232,  233,  234,  235,  236,  237,  238,  239,  240,  241,  242,  243,  244,  245,  246,  247,  248,  249,  250,  251,  252,  253,  254,  255
RRSTRIPPABLE U8 const bink_coeff_skips2[ 256 ] = { 0x24, 0x01, 0x45, 0x01, 0x13, 0x01, 0xC3, 0x01, 0x34, 0x01, 0x65, 0x01, 0x13, 0x01, 0xC3, 0x01, 0x24, 0x01, 0x76, 0x01, 0x13, 0x01, 0xC3, 0x01, 0x34, 0x01, 0xD6, 0x01, 0x13, 0x01, 0xC3, 0x01, 0x24, 0x01, 0x45, 0x01, 0x13, 0x01, 0xC3, 0x01, 0x34, 0x01, 0x65, 0x01, 0x13, 0x01, 0xC3, 0x01, 0x24, 0x01, 0x57, 0x01, 0x13, 0x01, 0xC3, 0x01, 0x34, 0x01, 0x87, 0x01, 0x13, 0x01, 0xC3, 0x01, 0x24, 0x01, 0x45, 0x01, 0x13, 0x01, 0xC3, 0x01, 0x34, 0x01, 0x65, 0x01, 0x13, 0x01, 0xC3, 0x01, 0x24, 0x01, 0x76, 0x01, 0x13, 0x01, 0xC3, 0x01, 0x34, 0x01, 0xD6, 0x01, 0x13, 0x01, 0xC3, 0x01, 0x24, 0x01, 0x45, 0x01, 0x13, 0x01, 0xC3, 0x01, 0x34, 0x01, 0x65, 0x01, 0x13, 0x01, 0xC3, 0x01, 0x24, 0x01, 0xB7, 0x01, 0x13, 0x01, 0xC3, 0x01, 0x34, 0x01, 0x98, 0x01, 0x13, 0x01, 0xC3, 0x01, 0x24, 0x01, 0x45, 0x01, 0x13, 0x01, 0xC3, 0x01, 0x34, 0x01, 0x65, 0x01, 0x13, 0x01, 0xC3, 0x01, 0x24, 0x01, 0x76, 0x01, 0x13, 0x01, 0xC3, 0x01, 0x34, 0x01, 0xD6, 0x01, 0x13, 0x01, 0xC3, 0x01, 0x24, 0x01, 0x45, 0x01, 0x13, 0x01, 0xC3, 0x01, 0x34, 0x01, 0x65, 0x01, 0x13, 0x01, 0xC3, 0x01, 0x24, 0x01, 0x57, 0x01, 0x13, 0x01, 0xC3, 0x01, 0x34, 0x01, 0x87, 0x01, 0x13, 0x01, 0xC3, 0x01, 0x24, 0x01, 0x45, 0x01, 0x13, 0x01, 0xC3, 0x01, 0x34, 0x01, 0x65, 0x01, 0x13, 0x01, 0xC3, 0x01, 0x24, 0x01, 0x76, 0x01, 0x13, 0x01, 0xC3, 0x01, 0x34, 0x01, 0xD6, 0x01, 0x13, 0x01, 0xC3, 0x01, 0x24, 0x01, 0x45, 0x01, 0x13, 0x01, 0xC3, 0x01, 0x34, 0x01, 0x65, 0x01, 0x13, 0x01, 0xC3, 0x01, 0x24, 0x01, 0xB7, 0x01, 0x13, 0x01, 0xC3, 0x01, 0x34, 0x01, 0xA8, 0x01, 0x13, 0x01, 0xC3, 0x01 };

RRSTRIPPABLE U8 const bink_coeff_skiplens[ 14 ] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 64, 0 };

RRSTRIPPABLE U8 const bink_coeff_skipzs[ 14 ]   = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7 };

static U32 dec_coeffs( U32 control, S16 * RADRESTRICT dqs, VARBITSEND * vb, U8 const * scan_order, S32 Q, BINK2_SCALETAB const * dequant, S32 const * RADRESTRICT dcs, S32 flags, U16 * RADRESTRICT dcflags, U16 dctype )
{
  U32 i, rects;
  U8 const * skips;
  U32 skip_and;
  U32 last_acs;
  S32 Qshift = Q >> 2;
  U16 const * Qtab = dequant[ Q & 3 ];

  VARBITSLOCAL( lvb );

  // early-out if we have no ACs
  if( ( control & 0xf ) == 0 )
  {
    for( rects = 0 ; rects < 4 ; rects++ )
    {
      dqs[ rects*64 ] = (S16) ( dcs[ rects ] * 8 + 32 );
      dcflags[ rects ] = ENCODE_DC_FLAGS( dcs[ rects ], 3, dctype );
    }

    return 0;
  }

  VarBitsCopyToLocal( lvb, *vb );

  last_acs = 0;

  // which huffman table?
  if ( ( control & 0xffff0000 ) == 0 )
  {
    skips = bink_coeff_skips1;
    skip_and = 511;
  }
  else
  {
    skips = bink_coeff_skips2;
    skip_and = 255;
  }

  for( rects = 0 ; rects < 4 ; rects++ )
  {
    S16 * RADRESTRICT dqr = dqs + 64 * rects;
    S32 skipzeros;
    S32 last_ac = 0;
    U32 has_acs = control & 1;
    U32 nonzero_acs = 0;

    control >>= 1;
    if( has_acs )
    {
      // zero this rect once we know it's got ACs
      {
        simd4i z = simd4i_0;
        simd4i * RADRESTRICT dqz = (simd4i *) dqr;
        dqz[0] = z; dqz[1] = z; dqz[2] = z; dqz[3] = z;
        dqz[4] = z; dqz[5] = z; dqz[6] = z; dqz[7] = z;
      }

      i = 1;
      skipzeros = 0;
      DWRITECODE( U32 hufftab = (magnitudes == mag1 + table_ver) ? 1 : 2; )
      DWRITECODE( S32 allstart = VarBitsLocalPos( lvb,vb ); )

      do
      {
        U32 b, z;
        S32 v, s;
        DWRITECODE( S32 magstart = 0; )

        // skip code
        if ( RAD_LIKELY( ( --skipzeros <= 0 ) ) )
        {
          DWRITECODE( S32 codeval = 0; )
          DWRITECODE( S32 skipstart = VarBitsLocalPos( lvb,vb );  )

          // get skip
          VarBitsLocalPeek( b, U32, lvb, 9 ); 

          v = skips[ b&skip_and ];
          b = v & 0xf;
          v >>= 4;
          DWRITECODE( codeval = v; )
          VarBitsLocalUse( lvb, b );
          skipzeros = bink_coeff_skipzs[ v ];
          v = bink_coeff_skiplens[ v ];

          DWRITE8( "lens_all", -1, codeval, 14 );
          //DWRITE8( "lens_each", i, ( ( ( v==0 ) && ( skipzeros > 0 ) ) ? 63 : v ),64 );
          DWRITE8( "lens_len", -1, VarBitsLocalPos( lvb,vb )-skipstart, 0 );

          DWRITE8( "lens_all_wch", hufftab, codeval, 14 );
          DWRITE8( "lens_len_wch", hufftab, VarBitsLocalPos( lvb,vb )-skipstart, 0 ); 

          if ( RAD_UNLIKELY( v == 11 ) )
          {
            VarBitsLocalGet( v,U32,lvb,6 );
          }

          i+=v;
          if ( RAD_UNLIKELY( i > 63 ) )
          {
            break;
          }
        }

        // value code
        DWRITECODE( magstart = VarBitsLocalPos( lvb,vb ); )

        // just unary!
        READ_UNARY( v, lvb, 11 );

        DWRITE8( "bits_all", -1, v,12 );
        DWRITE8( "bits_each", i, v,12 );
        DWRITE8( "bits_len", -1, VarBitsLocalPos( lvb,vb )-magstart,0 ); 
        //DWRITE8( "bits_all_wch", hufftab, v,12 );
        //DWRITE8( "bits_len_wch", hufftab, VarBitsLocalPos( lvb,vb )-magstart,0 ); 

        if ( RAD_UNLIKELY( v >= 4 ) )
        {
          b = v-3;  
          VarBitsLocalGet( v,U32,lvb,b );
          v += ( 1<<b ) + 2;
        }

        DWRITECODE( { U32 t = v; if ( t > 255 ) t = 255; DWRITE8( "val_all", -1, t,256 ); } );
        //DWRITECODE( { U32 t = v; if ( t > 255 ) t = 255; DWRITE8( "val_each", i, t,256 ); } );
        DWRITECODE( { U32 t = VarBitsLocalPos( lvb,vb )-magstart; DWRITE8( "val_len", -1, t, 0 ); } ); // we don't count the sign when measuring the entropy here

        s = VarBitsLocalGet1( lvb, b ); // read sign bit

        // if (s) v = -v;
        s = -s;
        v = ( v ^ s ) - s;

        z = scan_order[ i ];
        dqr[ z ] = (S16) ( ( ( ( v * Qtab[ z ] ) << Qshift ) + 64 ) >> 7 );

        last_ac = i;
        nonzero_acs++;

        ++i;
      } while ( RAD_LIKELY( i < 64 ) );

      DWRITECODE( { U32 t = radabs(dqr[scan_order[ 1 ]]); if ( t ) t = 1; DWRITE8( "val_1z", (flags&IS_KEY)?0:1, t, 2 ); } );
      DWRITE8( "all_len", -1, VarBitsLocalPos( lvb,vb )-allstart,0 );

    }

    last_acs = (last_acs >> 8) | (last_ac << 24);

    // process DC
    {
      U32 strength;

#if !defined(__RADARM__) || defined(__RADBIGENDIAN__)
      dqr[ 0 ] = (S16) ( dcs[ rects ] * 8 + 32 );
#else
      {
        // compute using NEON, because on ARM, having a scalar store
        // access the same cache line as a NEON store stalls.
        int16x4_t val = vld1_dup_s16( (int16_t *)&dcs[rects] ); // NOTE loads low half (i.e. only works in little-endian mode!)
        val = vshl_n_s16( val, 3 );
        val = vadd_s16( val, vdup_n_s16( 32 ) );
        vst1_lane_s16( dqr, val, 0 );
      }
#endif
      strength = ( nonzero_acs < 8 ) + ( nonzero_acs < 4 ) + ( nonzero_acs < 1 );
      dcflags[ rects ] = ENCODE_DC_FLAGS( dcs[ rects ], strength, dctype );
    }
        
/*{ U32 v;
        VarBitsLocalGet( v,U32,lvb,8 );
        if ( v != 197 ) RR_BREAK();
}*/
  }

  VarBitsCopyFromLocal( *vb, lvb);

  return last_acs;
}

#ifdef SUPPORT_BINKGPU

#if _MSC_VER >= 1700 // VS2012 or later
#define COEFFRESTRICT // for some reason, newer VC++s think the stores to out_gpu / dc_coeffs are dead when the ptrs are marked restrict
#else
#define COEFFRESTRICT RADRESTRICT
#endif

#if defined(__RADX86__) && !defined(__RADX64__)
  // 32-bit VC makes a call to _allshl here if we don't do this
  static U64 bink_64bit_shift[64]={0x1,0x2,0x4,0x8,0x10,0x20,0x40,0x80,0x100,0x200,0x400,0x800,0x1000,0x2000,0x4000,0x8000,0x10000,0x20000,0x40000,0x80000,0x100000,0x200000,0x400000,0x800000,0x1000000,0x2000000,0x4000000,0x8000000,0x10000000,0x20000000,0x40000000,0x80000000,0x100000000ull,0x200000000ull,0x400000000ull,0x800000000ull,0x1000000000ull,0x2000000000ull,0x4000000000ull,0x8000000000ull,0x10000000000ull,0x20000000000ull,0x40000000000ull,0x80000000000ull,0x100000000000ull,0x200000000000ull,0x400000000000ull,0x800000000000ull,0x1000000000000ull,0x2000000000000ull,0x4000000000000ull,0x8000000000000ull,0x10000000000000ull,0x20000000000000ull,0x40000000000000ull,0x80000000000000ull,0x100000000000000ull,0x200000000000000ull,0x400000000000000ull,0x800000000000000ull,0x1000000000000000ull,0x2000000000000000ull,0x4000000000000000ull,0x8000000000000000ull};
#endif

;static void gpu_dec_coeffs( PLANEPTRS * RADRESTRICT plane, U32 block_id, U32 control, U32 num_blocks, VARBITSEND * vb, U8 const * scan_order, S32 Q, BINK2_SCALETAB const * dequant, S32 const * RADRESTRICT dcs, U16 dctype )
{
  // scratch space for 16 blocks with 64-bit AC nonzero mask and 64 ACs,
  // plus a 32-byte header. (NB our current header is only 4 bytes)
  U8 scratch[ 32 + 16 * ( sizeof( U64 ) + 64 * sizeof( U16 ) ) ];
  U16 * COEFFRESTRICT dcflags = (U16 *)scratch; // initially, point output ptrs to scratch area
  U8 * COEFFRESTRICT out_gpu = scratch;
  U32 i;
  U32 ac_blocks;
  U8 const * skips;
  U32 skip_and;
  S32 Qshift = Q >> 2;
  U32 * ac_mask;
  U16 const * Qtab = dequant[ Q & 3 ];
  VARBITSLOCAL( lvb );

  // if we need to store this plane, set up dest pointers
  // otherwise, we just write to "scratch". 
  if ( plane->Dc )
  {
    dcflags = plane->Dc + block_id * num_blocks;
    out_gpu = plane->AcCur;

    // save start AC coeff offset for this block
    plane->AcOffs[ block_id ] = (U32) (UINTa) ( out_gpu - plane->AcStart ) / 4;
  }

  // initial DC flags (assuming blocks don't have ACs)
  for( i = 0 ; i < num_blocks ; i++ )
    dcflags[ i ] = ENCODE_DC_FLAGS( dcs[ i ], 3, dctype );

  // mask of blocks that have ACs
  ac_blocks = control & ( ( 1u << num_blocks ) - 1 );

  // store mask
  *(U32 *)out_gpu = ac_blocks; // top 16 bits are available!
  out_gpu += sizeof(U32);

  // reserve area for AC masks (we're gonna write these soon)
  ac_mask = (U32 *)out_gpu;
  out_gpu += popcount16( ac_blocks ) * sizeof(U64);

  // which huffman table?
  if ( ( control & 0xffff0000 ) == 0 )
  {
    skips = bink_coeff_skips1;
    skip_and = 511;
  }
  else
  {
    skips = bink_coeff_skips2;
    skip_and = 255;
  }

  VarBitsCopyToLocal( lvb, *vb );

  // process blocks that have AC coeffs
  while( ac_blocks )
  {
    U64 cur_acs = 0;
    U32 nonzero_acs = 0;
    S32 skipzeros = 0;

    i = 1;
    do
    {
      U32 b;
      S32 v, s;

      // skip code
      if ( RAD_LIKELY( ( --skipzeros <= 0 ) ) )
      {
        // get skip
        VarBitsLocalPeek( b, U32, lvb, 9 );
        v = skips[ b&skip_and ];
        b = v & 0xf;
        v >>= 4;
        VarBitsLocalUse( lvb, b );
        skipzeros = bink_coeff_skipzs[ v ];
        v = bink_coeff_skiplens[ v ];

        if ( RAD_UNLIKELY( v == 11 ) )
        {
          VarBitsLocalGet( v,U32,lvb,6 );
        }

        i += v;
        if ( RAD_UNLIKELY( i > 63 ) )
          break;
      }

      // value code
      READ_UNARY( v, lvb, 11 );
      if ( RAD_UNLIKELY( v >= 4 ) )
      {
        b = v-3;
        VarBitsLocalGet( v,U32,lvb,b );
        v += ( 1<<b ) + 2;
      }

      s = VarBitsLocalGet1( lvb, b ); // read sign bit
      s = -s;
      v = ( v ^ s ) - s; // apply sign flip when required

      // dequantize coeff and write it out
      s = scan_order[ i ];
      *(S16 *)out_gpu = (S16) ( ( ( ( v * Qtab[ s ] ) << Qshift ) + 64 ) >> 7 );
      out_gpu += sizeof(S16);

      // mark this AC as used in our mask
      #if defined(__RADX86__) && !defined(__RADX64__)
      {
        cur_acs |= bink_64bit_shift[i];
      }
      #else
      cur_acs |= 1ull << i;
      #endif
      ++nonzero_acs;
      ++i;
    } while ( RAD_LIKELY( i < 64 ) );

    // update DC/flags for this block
    {
      U32 ac_block_id = find_first_set( ac_blocks );
      U32 strength = ( nonzero_acs < 8 ) + ( nonzero_acs < 4 ) + ( nonzero_acs < 1 );
      dcflags[ ac_block_id ] = ENCODE_DC_FLAGS( dcs[ ac_block_id ], strength, dctype );
    }

    // store ac_mask as two words, low then high.
    *ac_mask++ = (U32)cur_acs;
    *ac_mask++ = (U32)(cur_acs >> 32);
    ac_blocks &= ac_blocks - 1;
  }

  if ( plane->Dc )
  {
    // align output ptr to 32-bit multiple and save it.
    UINTa out_gpu_addr = (UINTa)out_gpu;
    out_gpu_addr = ( out_gpu_addr + 3 ) & ~((UINTa)3);
    plane->AcCur = (U8*)out_gpu_addr;
  }

  VarBitsCopyFromLocal( *vb, lvb );
}

#undef COEFFRESTRICT

#endif

RRSTRIPPABLE RAD_ALIGN_SIMD( U8 const, UandD_zero15times8[16] ) = {0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8,8*8,9*8,10*8,11*8,12*8,13*8,14*8,0*8};

RRSTRIPPABLE RAD_ALIGN_SIMD( U8 const, LtoR_zero15[16] ) =        {0  ,1  ,2  ,3  ,4  ,5  ,6  ,7  ,8  ,9  ,10  ,11  ,12  ,13  ,14  ,0  };

RRSTRIPPABLE RAD_ALIGN_SIMD( U8 const, LtoR_zero15shl4[16] ) =    {0<<4  ,1<<4  ,2<<4  ,3<<4  ,4<<4  ,5<<4  ,6<<4  ,7<<4  ,8<<4  ,9<<4  ,10<<4  ,11<<4  ,12<<4  ,13<<4  ,14<<4  ,0<<4  };

static void mublock_LtoR( U32 code, U8 * RADRESTRICT ptr, U32 pitch )
{
  register S32 c1, c2;

  if ( code )
  {
    c1 = ( code & 15 ) << 3;
    c2 = ( code >> 4 ) << 3;

    mublock_vertical_masked_2( (U8*) ( ptr ), pitch, (U8*) &bink_hmask[ c1 ], (U8*) &bink_hmask[ c2 ] );
  }
}

static RADFORCEINLINE void UandD_blur( U32 code, U8 * addr, U32 pitch )
{
  if( code )
    mublock_UandD( &bink_hmask[ code ], addr, pitch );
}

static void int_idctf8x8( S16 * out, S16 const * in, S32 last_acs, S32 opitch )
{
  S32 i;

  UINTa ofs = 0;
  UINTa out_advance = 16; // l->r step: move right by 8 pixels (=16 bytes)
  UINTa out_advance_xor = (opitch*8 - 16) ^ out_advance; // t->b step: move down by 8 rows.

  for( i = 0 ; i < 4 ; i++ )
  {
    S32 last_ac = last_acs & 0xff;
    if( last_ac == 0 )
      int_idct_fill( (U8 *)out + ofs, opitch, (U8*) in );
    else if( last_ac <= BINK22_SPARSE_LAST_COEFF )
      int_idct_sparse( (U8 *)out + ofs, opitch, (U8 *)in );
    else
      int_idct_default( (U8 *)out + ofs, opitch, (U8 *)in );

    ofs += out_advance;
    out_advance ^= out_advance_xor;
    in += 64;
    last_acs >>= 8;
  }
}

#ifdef __RADX86__

static void insert_dcs( S16 * out, S32 const * dcs )
{
  S32 i;

  // write DCs into coeff array
  for( i = 0 ; i < 4 ; i++ )
    out[ i * 64 ] = (S16) ( dcs[ i ] * 8 + 32 );
}

RRSTRIPPABLE RAD_ALIGN_SIMD(U16,bink_thres[8])={32<<3,32<<3,32<<3,32<<3,32<<3,32<<3,32<<3,32<<3};

#if 0
// used to get an idct without deswizzling or blurring
void bink2_idct( S16 * out, S16 const * in )
{
  int i;
  for( i = 0 ; i < 4 ; i++ )
  {
    int_idct_default( (U8*)(out + (i&1)*8 + (i&2)*16*(8>>1)), 16*2, (U8*)( in + i*64 ) );
  }
}
#endif

S32 bink2_idct_and_deswizzle( U8 * out, S16 * acs, S32 const * dcs, U32 ow, U32 last_acs, U8 * no_blur_out, U32 bw )
{
  RAD_ALIGN_SIMD( S16, work[ 16*16 ] );

  if ( last_acs )
  {
    insert_dcs( acs, dcs );
    int_idctf8x8( work, acs, last_acs, 16*2 );

    pack16x8_to_u8( out+0*ow, ow, (U8 *) (work+0*16), 16*sizeof(S16) );
    pack16x8_to_u8( out+8*ow, ow, (U8 *) (work+8*16), 16*sizeof(S16) );
  }
  //else
  {
    S16 top[4];
    S16 bot[4];
    U8 topcode=3,botcode=3;
    U32 nzc = 0;

    top[1]=(S16) dcs[ 0 ];
    top[2]=(S16) dcs[ 1 ];
    bot[1]=(S16) dcs[ 2 ];
    bot[2]=(S16) dcs[ 3 ];
    if ( last_acs & 0x000000ff ) 
    {
      top[1]=8192;
      topcode&=2;
      ++nzc;
    }
    if ( last_acs & 0x0000ff00 )
    {
      top[2]=8192;
      topcode&=1;
      ++nzc;
    }
    if ( last_acs & 0x00ff0000 )
    {
      bot[1]=8192;
      botcode&=2;
      ++nzc;
    }
    if ( last_acs & 0xff000000 )
    {
      bot[2]=8192;
      botcode&=1;
      ++nzc;
    }

    top[0]=top[1];
    top[3]=top[2];
    bot[0]=bot[1];
    bot[3]=bot[2];

    if ( nzc < 3 ) // this means that two or more rects have no ACs, so they will be full blurred
    {
      // and when we have a blur situation, we also record what it would have looked like with no blur
      move_u8_16x16( no_blur_out, bw, out, ow );
      {
        new_dc_blur[topcode]( (U8*)top,(U8*)top,(U8*)bot,(U8*)zerosd,no_blur_out,bw);
        new_dc_blur[botcode]( (U8*)top,(U8*)bot,(U8*)bot,(U8*)zerosd,no_blur_out+(8*bw),bw);
      }
    }

    {
      new_dc_blur[topcode]( (U8*)top,(U8*)top,(U8*)bot,(U8*)bink_thres,out,ow);
      new_dc_blur[botcode]( (U8*)top,(U8*)bot,(U8*)bot,(U8*)bink_thres,out+(8*ow),ow);
    }

    return( ( nzc < 3 ) ? 1 : 0 );
  }
}

void bink2_idct_and_mo_deswizzle( U8 * out, S16 * acs, U8 const * mo, S32 const * dcs, U32 ow, U32 mw, U32 last_acs )
{
  RAD_ALIGN_SIMD( S16, work[ 16*16 ] );

  insert_dcs( acs, dcs );
  int_idctf8x8( work, acs, last_acs, 16*2 );

  pack16x8_to_u8_and_mocomp( out+0*ow, ow, (U8 *) (work+0*16), 16*sizeof(S16), (U8 *) (mo+0*mw), mw );
  pack16x8_to_u8_and_mocomp( out+8*ow, ow, (U8 *) (work+8*16), 16*sizeof(S16), (U8 *) (mo+8*mw), mw );
}

#endif


#if defined( __RADX86__ )

// x86 gradient clamp via SSE2
// SSE2 has PMINSW / PMAXSW but no 32-bit equivalent (that was added later);
// this is fine (our DCs fit in 16 bits anyway), but it means we compute
// everything in 16 bits and then need to sign-extend to 32 bits later.

#define NGRADTYPE __m128i
#define NGRADLOAD(val) _mm_cvtsi32_si128( val )

#define NGRADSETRANGE( minv, maxv ) \
  __m128i minval = NGRADLOAD( minv & 0xffff ); \
  __m128i maxval = NGRADLOAD( maxv & 0xffff )

#define NGRADCLAMPout( memout,mempos,memadd, dout,nv,wv,nwv ) \
  dout = _mm_sub_epi16( _mm_add_epi16( nv, wv ), nwv ); \
  dout = _mm_max_epi16( dout, _mm_min_epi16( _mm_min_epi16( nv, wv ), nwv ) ); \
  dout = _mm_min_epi16( dout, _mm_max_epi16( _mm_max_epi16( nv, wv ), nwv ) ); \
  dout = _mm_add_epi16( dout, NGRADLOAD( memadd[mempos] ) ); \
  dout = _mm_min_epi16( _mm_max_epi16( dout, minval ), maxval ); \
  memout[mempos] = (S16) _mm_cvtsi128_si32( dout )

#define NDELTAout( memout,mempos,memadd, dout,pred ) \
  dout = _mm_add_epi16( pred, NGRADLOAD( memadd[mempos] ) ); \
  dout = _mm_min_epi16( _mm_max_epi16( dout, minval ), maxval ); \
  memout[mempos] = (S16) _mm_cvtsi128_si32( dout )

#elif defined( __RADARM__ )

#define NGRADTYPE int32x2_t
#define NGRADLOAD(val) vld1_dup_s32( &(val) )

#define NGRADSETRANGE( minv, maxv ) \
  NGRADTYPE minval = vdup_n_s32( minv ); \
  NGRADTYPE maxval = vdup_n_s32( maxv )

#define NGRADCLAMPout( memout,mempos,memadd, dout,nv,wv,nwv ) \
  dout = vsub_s32( vadd_s32( nv, wv ), nwv ); \
  dout = vmax_s32( dout, vmin_s32( vmin_s32( nv, wv ), nwv ) ); \
  dout = vmin_s32( dout, vmax_s32( vmax_s32( nv, wv ), nwv ) ); \
  dout = vadd_s32( dout, NGRADLOAD( memadd[mempos] ) ); \
  dout = vmin_s32( vmax_s32( dout, minval ), maxval ); \
  vst1_lane_s32( &memout[mempos], dout, 0 )

#define NDELTAout( memout,mempos,memadd, dout,pred ) \
  dout = vadd_s32( pred, NGRADLOAD( memadd[mempos] ) ); \
  dout = vmin_s32( vmax_s32( dout, minval ), maxval ); \
  vst1_lane_s32( &memout[mempos], dout, 0 )

#elif !defined(NGRADTYPE)

// generic gradient clamp impl (C)

#define NGRADTYPE S32
#define NGRADLOAD(val) val

#define NGRADSETRANGE( minv, maxv ) \
  S32 minval = minv; \
  S32 maxval = maxv

#define NGRADCLAMPout( memout,mempos,memadd, dout,nv,wv,nwv ) \
  dout = (nv) + (wv) - (nwv); \
  dout = RR_MAX( dout, RR_MIN( RR_MIN( nv, wv ), nwv ) ); \
  dout = RR_MIN( dout, RR_MAX( RR_MAX( nv, wv ), nwv ) ); \
  dout += memadd[mempos]; \
  dout = RR_MIN( RR_MAX( dout, minval ), maxval ); \
  memout[mempos] = dout

#define NDELTAout( memout,mempos,memadd, dout,pred ) \
  dout = pred + memadd[mempos]; \
  dout = RR_MIN( RR_MAX( dout, minval ), maxval ); \
  memout[mempos] = dout

#endif

static S32 bit_decode_dcs16( S32 * RADRESTRICT ldcs, S32 Q, VARBITSEND * vb )
{
  U32 i, b;
  S32 has_residuals = 0;
  simd4f dq = simd4f_0;
  DWRITECODE( S32 st; )
  VARBITSLOCAL( lvb );

  VarBitsCopyToLocal( lvb, *vb );
  DWRITECODE( st = VarBitsLocalPos(lvb,vb); )

  Q = RR_MAX( Q, BINK22_MIN_DC_Q );
  ((simd4f*)ldcs)[0]=dq;
  ((simd4f*)ldcs)[1]=dq;
  ((simd4f*)ldcs)[2]=dq;
  ((simd4f*)ldcs)[3]=dq;

  if( VarBitsLocalGet1( lvb, b ) )
  {
    S32 dequant_dc = bink2_fix10_dc_dequant[ Q ];
    has_residuals = 1;

    for( i = 0 ; i < 16 ; i++ )
    {
      S32 v;

      // read unary
      READ_UNARY( v, lvb, 11 );
      v--;

      // read suffix/sign
      if( v )
      {
        S32 s;

        if( RAD_UNLIKELY( v >= 4 ) )
        {
          b = v-3;
          VarBitsLocalGet( v,U32,lvb,b );
          v += (1 << b) + 2;
        }

        v = ( v * dequant_dc + 512 ) >> 10;

        s = VarBitsLocalGet1( lvb, b );
        s = -s;
        v = ( v ^ s ) - s;

        ldcs[ i ] = v;
      }
    }
  }

  VarBitsCopyFromLocal( *vb, lvb);

  DWRITECODE(
  {
    int i,j;
    j=VarBitsLocalPos(lvb,vb);
    for(i=0;i<16;i++)
    {
      DWRITEDCs( ldcs[i] );
      if ( ldcs[i] )
        --j;
    }
    DWRITE8( "dcs-len", -1, j-st, 0 );
  });

  return has_residuals;
}

static const S32 bink_grad0 = 0, bink_grad1024 = 1024;

static void decode_dcs16( S32 * RADRESTRICT dcs, S32 const * prevydcs, S32 Q, VARBITSEND * vb, U32 flags, S32 minv, S32 maxv )
{
  RAD_ALIGN_SIMD( S32, ldcs[ 16 ] );
  
  // prevydcs == 0, means we predict from zero

 if ( !bit_decode_dcs16( ldcs, Q, vb ) )
  {
    // for MDCT blocks, this just means that all DCs are zero;
    // no need to do the prediction dance.
    if( ( prevydcs == 0 ) && ( (flags & (X_LEFT|Y_TOP|Y_MID_TOP)) == (X_LEFT|Y_TOP|Y_MID_TOP) ) )
    {
      simd4f dq = simd4f_0;
      ((simd4f*)dcs)[0]=dq;
      ((simd4f*)dcs)[1]=dq;
      ((simd4f*)dcs)[2]=dq;
      ((simd4f*)dcs)[3]=dq;
      return;
    }
  }
 
  if ( flags & X_LEFT )
  {
    NGRADTYPE t0,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15;
    NGRADSETRANGE( minv, maxv );

    if ( flags & (Y_TOP|Y_MID_TOP) )
    {
      t0 = ( prevydcs == 0 ) ? NGRADLOAD(bink_grad0) : NGRADLOAD(bink_grad1024);

      NDELTAout    ( dcs, 0, ldcs, t0, t0 );
      NDELTAout    ( dcs, 1, ldcs, t1, t0 );
      NGRADCLAMPout( dcs, 2, ldcs, t4, t0, t0, t1 );    // todo, use all SIMD regs!!
      NGRADCLAMPout( dcs, 3, ldcs, t5, t1, t4, t0 );

      NGRADCLAMPout( dcs, 4, ldcs, t2, t1, t1, t5 );
      NDELTAout    ( dcs, 5, ldcs, t3, t2 );
    }
    else
    {
      // x_left, not top
      NGRADCLAMPout( dcs, 0, ldcs, t0, NGRADLOAD(prevydcs[4]), NGRADLOAD(prevydcs[4]), NGRADLOAD(prevydcs[5]) );
      NGRADCLAMPout( dcs, 1, ldcs, t1, NGRADLOAD(prevydcs[5]), t0, NGRADLOAD(prevydcs[4]) );
      NGRADCLAMPout( dcs, 2, ldcs, t4, t0, t0, t1 );
      NGRADCLAMPout( dcs, 3, ldcs, t5, t1, t4, t0 );

      NGRADCLAMPout( dcs, 4, ldcs, t2, NGRADLOAD(prevydcs[6]), t1, NGRADLOAD(prevydcs[5]) );
      NGRADCLAMPout( dcs, 5, ldcs, t3, NGRADLOAD(prevydcs[7]), t2, NGRADLOAD(prevydcs[6]) );
    }

    NGRADCLAMPout( dcs, 6, ldcs, t6, t2, t5, t1 );
    NGRADCLAMPout( dcs, 7, ldcs, t7, t3, t6, t2 );

    NGRADCLAMPout( dcs, 8, ldcs, t8, t4, t4, t5 );
    NGRADCLAMPout( dcs, 9, ldcs, t9, t5, t8, t4 );
    NGRADCLAMPout( dcs, 10, ldcs, t12, t8, t8, t9 );
    NGRADCLAMPout( dcs, 11, ldcs, t13, t9, t12, t8 );

    NGRADCLAMPout( dcs, 12, ldcs, t10, t6, t9, t5 );
    NGRADCLAMPout( dcs, 13, ldcs, t11, t7, t10, t6 );
    NGRADCLAMPout( dcs, 14, ldcs, t14, t10, t13, t9 );
    NGRADCLAMPout( dcs, 15, ldcs, t15, t11, t14, t10 );
  }
  else
  {
    NGRADTYPE t0,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,tmp;
    NGRADSETRANGE( minv, maxv );
    tmp = NGRADLOAD(dcs[7]);

    if ( flags & (Y_TOP|Y_MID_TOP) )
    {
      // not left, but top
      NGRADCLAMPout( dcs, 0, ldcs, t0, NGRADLOAD(dcs[5]), NGRADLOAD(dcs[5]), tmp );
      NDELTAout    ( dcs, 1, ldcs, t1, t0 );
      NGRADCLAMPout( dcs, 2, ldcs, t4, t0, tmp, NGRADLOAD(dcs[5]) );
      NGRADCLAMPout( dcs, 3, ldcs, t5, t1, t4, t0 );

      NGRADCLAMPout( dcs, 4, ldcs, t2, t1, t1, t5 );
      NDELTAout    ( dcs, 5, ldcs, t3, t2 );
    }
    else
    {
      // not left, not top
      NGRADCLAMPout( dcs, 0, ldcs, t0, NGRADLOAD(prevydcs[4]), NGRADLOAD(dcs[5]), NGRADLOAD(prevydcs[3]) );
      NGRADCLAMPout( dcs, 1, ldcs, t1, NGRADLOAD(prevydcs[5]), t0, NGRADLOAD(prevydcs[4]) );
      NGRADCLAMPout( dcs, 2, ldcs, t4, t0, tmp, NGRADLOAD(dcs[5]) );
      NGRADCLAMPout( dcs, 3, ldcs, t5, t1, t4, t0 );

      NGRADCLAMPout( dcs, 4, ldcs, t2, NGRADLOAD(prevydcs[6]), t1, NGRADLOAD(prevydcs[5]) );
      NGRADCLAMPout( dcs, 5, ldcs, t3, NGRADLOAD(prevydcs[7]), t2, NGRADLOAD(prevydcs[6]) );
    }

    NGRADCLAMPout( dcs, 6, ldcs, t6, t2, t5, t1 );
    NGRADCLAMPout( dcs, 7, ldcs, t7, t3, t6, t2 );

    NGRADCLAMPout( dcs, 8, ldcs, t8, t4, NGRADLOAD(dcs[13]), tmp );
    NGRADCLAMPout( dcs, 9, ldcs, t9, t5, t8, t4 );
    NGRADCLAMPout( dcs, 10, ldcs, t12, t8, NGRADLOAD(dcs[15]), NGRADLOAD(dcs[13]) );
    NGRADCLAMPout( dcs, 11, ldcs, t13, t9, t12, t8 );

    NGRADCLAMPout( dcs, 12, ldcs, t10, t6, t9, t5 );
    NGRADCLAMPout( dcs, 13, ldcs, t11, t7, t10, t6 );
    NGRADCLAMPout( dcs, 14, ldcs, t14, t10, t13, t9 );
    NGRADCLAMPout( dcs, 15, ldcs, t15, t11, t14, t10 );
  }
}

static S32 bit_decode_dcs4( S32 * RADRESTRICT ldcs, S32 Q, VARBITSEND * vb )
{
  U32 i, b;
  S32 has_residuals = 0;
  VARBITSLOCAL( lvb );

  VarBitsCopyToLocal( lvb, *vb );
  Q = RR_MAX( Q, BINK22_MIN_DC_Q );

  ((simd4f* RADRESTRICT)ldcs)[0]=simd4f_0;

  if( VarBitsLocalGet1( lvb, b ) )
  {
    S32 dequant_dc = bink2_fix10_dc_dequant[ Q ];
    has_residuals = 1;

    for( i = 0 ; i < 4 ; i++ )
    {
      S32 v;

      // read unary
      READ_UNARY( v, lvb, 11 );
      v--;

      // read suffix/sign
      if( v )
      {
        S32 s;

        if( RAD_UNLIKELY( v >= 4 ) )
        {
          b = v-3;
          VarBitsLocalGet( v,U32,lvb,b );
          v += (1 << b) + 2;
        }

        v = ( v * dequant_dc + 512 ) >> 10;

        s = VarBitsLocalGet1( lvb, b );
        s = -s;
        v = ( v ^ s ) - s;

        ldcs[ i ] = v;
      }
    }
  }

  VarBitsCopyFromLocal( *vb, lvb);

  DWRITECODE(
  {
    for(i=0;i<4;i++)
    {
      DWRITEDCs( ldcs[i] );
    }
  });

  return has_residuals;
}

static void decode_dcs4( S32 * RADRESTRICT dcs, S32 const * prevydcs, S32 Q, VARBITSEND * vb, U32 flags, S32 minv, S32 maxv )
{
  RAD_ALIGN_SIMD( S32, ldcs[ 4 ] );

  bit_decode_dcs4( ldcs, Q, vb );

  if ( flags & X_LEFT )
  {
    if ( flags & (Y_TOP|Y_MID_TOP) )
    {
      NGRADTYPE t0,t1,t2,t3;

      NGRADSETRANGE( minv, maxv );

      t0 = ( prevydcs == 0 ) ? NGRADLOAD(bink_grad0) : NGRADLOAD(bink_grad1024);

      NDELTAout    ( dcs, 0, ldcs, t0, t0 );
      NDELTAout    ( dcs, 1, ldcs, t1, t0 );
      NGRADCLAMPout( dcs, 2, ldcs, t2, t0, t0, t1 );
      NGRADCLAMPout( dcs, 3, ldcs, t3, t1, t2, t0 );
    }
    else
    {
      NGRADTYPE t0,t1,t2,t3;

      NGRADSETRANGE( minv, maxv );

      // x_left, not top

      NGRADCLAMPout( dcs, 0, ldcs, t0, NGRADLOAD(prevydcs[2]), NGRADLOAD(prevydcs[2]), NGRADLOAD(prevydcs[3]) );
      NGRADCLAMPout( dcs, 1, ldcs, t1, NGRADLOAD(prevydcs[3]), t0, NGRADLOAD(prevydcs[2]) );
      NGRADCLAMPout( dcs, 2, ldcs, t2, t0, t0, t1 );
      NGRADCLAMPout( dcs, 3, ldcs, t3, t1, t2, t0 );
    }
  }
  else
  {
    if ( flags & (Y_TOP|Y_MID_TOP) )
    {
      NGRADTYPE t0,t1,t2,t3, tmp;

      NGRADSETRANGE( minv, maxv );

      tmp = NGRADLOAD(dcs[1]);

      // not left, but top

      NGRADCLAMPout( dcs, 0, ldcs, t0, tmp, tmp, NGRADLOAD(dcs[3]) );
      NDELTAout    ( dcs, 1, ldcs, t1, t0 );
      NGRADCLAMPout( dcs, 2, ldcs, t2, t0, NGRADLOAD(dcs[3]), tmp );
      NGRADCLAMPout( dcs, 3, ldcs, t3, t1, t2, t0 );
    }
    else
    {
      NGRADTYPE t0,t1,t2,t3,tmp;

      NGRADSETRANGE( minv, maxv );

      tmp = NGRADLOAD(dcs[1]);

      // not left, not top

      NGRADCLAMPout( dcs, 0, ldcs, t0, NGRADLOAD(prevydcs[2]), tmp, NGRADLOAD(prevydcs[1]) );
      NGRADCLAMPout( dcs, 1, ldcs, t1, NGRADLOAD(prevydcs[3]), t0, NGRADLOAD(prevydcs[2]) );
      NGRADCLAMPout( dcs, 2, ldcs, t2, t0, NGRADLOAD(dcs[3]), tmp );
      NGRADCLAMPout( dcs, 3, ldcs, t3, t1, t2, t0 );
    }
  }
}

static S32 med3( S32 a, S32 b, S32 c )
{
#if defined(__RADINORDER__)
  // Use branch-free version on the in-order PPCs.
  S32 x = a, y = b;
  S32 t;

  t = ( ( x - y ) >> 31 ) & ( x - y ); // x-y if x<y, 0 otherwise.
  x -= t;
  y += t;
  // now x = max(a,b), y = min(a,b)
  
  t = ( ( y - c ) >> 31 ) & ( y - c ); // y-c if y<c, 0 otherwise.
  y -= t;

  t = ( ( x - y ) >> 31 ) & ( x - y ); // x-y if x<y, 0 otherwise.
  y += t;

  return y;
#else
  S32 x = a, y = b;

  if( y > x )
  {
    x = b;
    y = a;
  }
  // now x = max(a,b), y = min(a,b)

  if( c > y )
    y = c;

  return ( y > x ) ? x : y;
#endif
}


RRSTRIPPABLE U8 const bink_fasttab[ 128 ] = { 0x01, 0x01, 0xfb, 0x01, 0xf5, 0x01, 0x0b, 0x01, 0xe7, 0x01, 0xfb, 0x01, 0x15, 0x01, 0x0b, 0x01, 0x01, 0x01, 0xfb, 0x01, 0xed, 0x01, 0x0b, 0x01, 0x27, 0x01, 0xfb, 0x01, 0x1d, 0x01, 0x0b, 0x01, 0x01, 0x01, 0xfb, 0x01, 0xf5, 0x01, 0x0b, 0x01, 0xdf, 0x01, 0xfb, 0x01, 0x15, 0x01, 0x0b, 0x01, 0x01, 0x01, 0xfb, 0x01, 0xed, 0x01, 0x0b, 0x01, 0x2f, 0x01, 0xfb, 0x01, 0x1d, 0x01, 0x0b, 0x01, 0x01, 0x01, 0xfb, 0x01, 0xf5, 0x01, 0x0b, 0x01, 0xd7, 0x01, 0xfb, 0x01, 0x15, 0x01, 0x0b, 0x01, 0x01, 0x01, 0xfb, 0x01, 0xed, 0x01, 0x0b, 0x01, 0x37, 0x01, 0xfb, 0x01, 0x1d, 0x01, 0x0b, 0x01, 0x01, 0x01, 0xfb, 0x01, 0xf5, 0x01, 0x0b, 0x01, 0xcf, 0x01, 0xfb, 0x01, 0x15, 0x01, 0x0b, 0x01, 0x01, 0x01, 0xfb, 0x01, 0xed, 0x01, 0x0b, 0x01, 0x3f, 0x01, 0xfb, 0x01, 0x1d, 0x01, 0x0b, 0x01 };

static void decode_mo_coords( VARBITSEND * vb, S32 * RADRESTRICT cur, S32 const * RADRESTRICT prev, U32 flags )
{
  VARBITSLOCAL( lvb );
  S32 t, count = 4;

  VarBitsCopyToLocal( lvb, *vb );

  // 32x32 mo block?
  if( VarBitsLocalGet1(lvb, t) )
    count = 1;
  
  for( t = 0 ; t < 2 ; t++ )
  {
    S32 a,b,c,d,v;
    S32 i, m, move[ 4 ];
    U32 len;

    for( i = 0 ; i < count ; i++ )
    {
      VarBitsLocalPeek(v,U32,lvb,16);
      if ( RAD_LIKELY( v&15 ) ) // code <=7 bits - common case
      {
        m = bink_fasttab[v & 127];
        len = m & 7;
        VarBitsLocalUse(lvb, len);
        m = (S8)m >> 3;
      }
      else switch ( (v>>4)&31 )
      {
      #define HandleTail(prefix_bits, suffix_bits) \
        VarBitsLocalGet(m,U32,lvb,((prefix_bits)+(suffix_bits))); \
        m = ((U32)m >> (prefix_bits)) + (1 << (suffix_bits)) - 1; \
        m = (m >> 1) ^ -(m & 1) /* dezigzag */

        // 1xxxx
      case 0x01: case 0x03: case 0x05: case 0x07: case 0x09: case 0x0b: case 0x0d: case 0x0f:
      case 0x11: case 0x13: case 0x15: case 0x17: case 0x19: case 0x1b: case 0x1d: case 0x1f:
        HandleTail(5, 4); break;

        // 01xxx
      case 0x02: case 0x06: case 0x0a: case 0x0e: case 0x12: case 0x16: case 0x1a: case 0x1e:
        HandleTail(6, 5); break;

        // 001xx
      case 0x04: case 0x0c: case 0x14: case 0x1c:
        HandleTail(7, 6); break;

        // 0001x
      case 0x08: case 0x18:
        HandleTail(8, 7); break;

        // 00001
      case 0x10:
        HandleTail(9, 8); break;

        // 00000
      case 0x00:
        if ( RAD_LIKELY( v != 0 ) )
        {
          U32 v_lsb = v & -v; // exactly 1 bit set in here
          U32 prefix_len = getbitlevelvar( v_lsb );
          HandleTail(prefix_len, prefix_len-1); 
        }
        else // full 16 zero bits
        {
          //HandleTail(16, 16); // can't do this since VarBitsGet of 32 bits is unsupported.
          VarBitsLocalUse(lvb,16); // use up prefix (16 zero bits)
          VarBitsLocalGet(m,U32,lvb,16); // get suffix bits
          m += (1 << 16) - 1; // bias
          m = (m >> 1) ^ -(m & 1); // dezigzag
        }
        break;

      default:
        RADUNREACHABLE;

      #undef HandleTail
      }

      move[i] = m;
    }

    // todo stats?

    {
      #define PREDMED3( o, l, W, N, NW ) coords[o]=(l+=med3(W, N, NW))

      #define Ws (cur-8)
      #define coords (cur)
      #define Ns  (prev)
      #define NWs (prev-8)

      if( count == 1 )
      {
        // one MV for the whole 32x32 block
        a = move[0];
        if( flags & ( Y_TOP|Y_MID_TOP ) )
        {
          if( flags & X_LEFT )
            coords[0] = a;
          else
          {
            PREDMED3( 0, a, Ws[0], Ws[2], Ws[6] );
          }
        }
        else if( flags & X_LEFT )
        {
          PREDMED3( 0, a, Ns[0], Ns[4], Ns[6] );
        }
        else
        {
          PREDMED3( 0, a, Ws[2], Ns[4], NWs[6] );
        }

        coords[2] = a;
        coords[4] = a;
        coords[6] = a;
      }
      else
      {
        a = move[0];
        b = move[1];
        c = move[2];
        d = move[3];

        if ( flags & ( Y_TOP|Y_MID_TOP ) )
        {
          if ( flags & X_LEFT )
          {
            coords[ 0 ] = a;
            coords[ 2 ] = b += a;
            coords[ 4 ] = c += a;
            PREDMED3( 6, d, a, b, c );
          }
          else
          {
            PREDMED3( 0, a, Ws[0], Ws[2], Ws[6] );
            PREDMED3( 4, c, a, Ws[2], Ws[6] );
            PREDMED3( 2, b, a, c, Ws[2] );
            PREDMED3( 6, d, a, b, c );
          }
        }
        else if ( flags & X_LEFT )
        {
          PREDMED3( 0, a, Ns[0], Ns[4], Ns[6] );
          PREDMED3( 2, b, a, Ns[4], Ns[6] );
          PREDMED3( 4, c, a, b, Ns[4] );
          PREDMED3( 6, d, a, b, c );
        }
        else
        {
          PREDMED3( 0, a, Ws[2], Ns[4], NWs[6] );
          PREDMED3( 2, b, a, Ns[4], Ns[6] );
          PREDMED3( 4, c, a, Ws[2], Ws[6] );
          PREDMED3( 6, d, a, b, c );
        }
      }

      #undef Ws
      #undef coords
      #undef Ns
      #undef NWs
      #undef PREDMED3
    }

    ++cur;
    ++prev;
  }
  VarBitsCopyFromLocal( *vb, lvb);
}

static U32 dec_4_controls( VARBITSEND * vb, U32 prev, U32 flags )
{
  U32 control;
  U32 tmp, c;
  U32 pop = popcount16( prev );
  U32 flip = 0;

  if( pop >= 8 )
  {
    flip = 0xffff;
    pop = 16 - pop;
  }

  if ( VarBitsGet1( *vb, tmp ) )
  {
    control = 0;
  }
  else
  {
    U32 r, t;
    
    c = 0;
    if( pop <= 3 )
    {
      for( r = 0 ; r < 4 ; r++ )
      {
        c >>= 4;
        if ( !( VarBitsGet1( *vb, tmp ) ))
        {
          VarBitsGet(t,U32,*vb,4);
          c |= t << 12;
        }
      }
    }
    else
    {
      VarBitsGet(c,U32,*vb,16);
    }

    control = c;
  }

  control ^= flip;

  if ( ( ( flags & BINKVER25 ) == 0 ) || ( ( control & 0xffff ) != 0 ) )
    if ( VarBitsGet1( *vb, tmp ) )
      control |= ( control << 16 );

  return control;
}

static U32 dec_1_control( VARBITSEND * vb, U32 prev )
{
  // popcount( x ) >= 2 ? 0xf : 0
  static const U8 predict[ 16 ] =
  {
    /*          00   01   10   11 */
    /* 00xx */ 0x0, 0x0, 0x0, 0xf,
    /* 01xx */ 0x0, 0xf, 0xf, 0xf,
    /* 10xx */ 0x0, 0xf, 0xf, 0xf,
    /* 11xx */ 0xf, 0xf, 0xf, 0xf
  };

  U32 control;
  U32 pred = ( prev & 0xf0000 ) | predict[ prev & 0xf ];
  U32 tmp;

  if ( VarBitsGet1( *vb, tmp ) )
  {
    control = pred;
  }
  else
  {
    VarBitsGet( control,U32,*vb, 4 );
    if ( VarBitsGet1( *vb, tmp ) )
      control |= ( control << 16 );
  }

  return control;
}

static S32 decode_q( VARBITSEND *vb, U32 flags, S32 predQ DWRITECODE(,char*slen,char*sdiff,char*sq) )
{
  S32 Q;

  VarBitsPeek( Q, U32, *vb, 9 );  
  if ( ( Q & 7 ) == 0 )
  {
    if ( ( Q & 15 ) == 0 )
    {
      VarBitsUse( *vb, 9 );
      Q = (Q>>4)+5;
      DWRITE8( slen, -1, 9, 0 );
    }
    else
    {
      VarBitsUse( *vb, 5 );
      Q = ( ( Q >> 4 ) & 1 ) + 3;
      DWRITE8( slen, -1, 5, 0 );
    }
  }
  else
  {
    static U8 const Qs[ 8 ] = { 8, 0, 1, 0,  2, 0, 1, 0 }; //8 never picked
    U32 b;
    Q = Qs[ Q&7 ];
    b = Q+1;
    VarBitsUse( *vb, b );
    DWRITE8( slen, -1, b, 0 );
  }

  if ( Q )
  {
    U32 tmp;
    if ( VarBitsGet1( *vb, tmp ) )
      Q = -Q;
  }
  Q += predQ;
  DWRITE8( sdiff, -1, abs(Q-predQ), 32 );
  DWRITE8( sq, -1, Q, 32 );

  return Q;
}

static void dec_dct_luma( OPT_PROFILE_PLANE  S32 * RADRESTRICT dcs, S32 * RADRESTRICT ydcs, U8 * RADRESTRICT y, U32 w, VARBITSEND * vb, U16 * RADRESTRICT a, U32 flags, S32 Q, U32 * RADRESTRICT cnp )
{
  U32 control = 0;
  S32 rects;

  cnp[0] = control = dec_4_controls( vb, cnp[0], flags );

  dtmEnterAccumulationZone( tm_mask, tm->decodeDCs );
  decode_dcs16( dcs, ydcs, Q, vb, flags, MININTRA, MAXINTRA );
  dtmLeaveAccumulationZone( tm_mask, tm->decodeDCs );

  for( rects = 0 ; rects < 4 ; rects++ )
  {
    RAD_ALIGN( S16, coeffs[ 64*4 ], COEFF_ALIGN );
    U32 last_acs;
  
    U32 ofs = ( ( rects & 1 ) << 4 );  // 0 or 16
    if ( rects & 2 ) 
      ofs += ( w * 16 );

    // we do two 16x16s to avoid LHS
    dtmEnterAccumulationZone( tm_mask, tm->decodeACs );
    last_acs = dec_coeffs( control, coeffs, vb, bink_zigzag_scan_order_transposed, Q, bink_intra_luma_scaleT, dcs + 4*rects, flags, a + 4*rects, DC_INTRA );
    control >>= 4;
    dtmLeaveAccumulationZone( tm_mask, tm->decodeACs );

    if ( y ) 
    {
      if ( ( ( ( flags & Y_MID_TOP_T2 ) == 0 ) && ( ( rects & 2 ) == 0 ) ) ||
           ( ( ( flags & (Y_MID_TOP_T1|Y_NO_BOTTOM_16) ) == 0 ) && ( ( rects & 2 ) ) ) )
      {
#ifndef NO_FULL_BLUR
        if ( last_acs != 0 )
#endif
        {
          RAD_ALIGN_SIMD( S16, out[16*16] );

          dtmEnterAccumulationZone( tm_mask, tm->idct );
          int_idctf8x8( out, coeffs, last_acs, 16*2 );
          dtmLeaveAccumulationZone( tm_mask, tm->idct );

          dtmEnterAccumulationZone( tm_mask, tm->deswizzle );
          pack16x8_to_u8( y+ofs+0*w, w, (U8 *) (out+0*16), 16*sizeof(S16) );
          pack16x8_to_u8( y+ofs+8*w, w, (U8 *) (out+8*16), 16*sizeof(S16) );
          dtmLeaveAccumulationZone( tm_mask, tm->deswizzle );
        }
      }
    }
  }
}

static void dec_dct_chroma( OPT_PROFILE_PLANE  S32 * RADRESTRICT dcs, S32 * RADRESTRICT ydcs, U8 * RADRESTRICT c, U32 w, VARBITSEND * vb, U16 * RADRESTRICT a, U32 flags, S32 Q, U32 * RADRESTRICT cnp )
{
  RAD_ALIGN( S16, coeffs[ 64*4 ], COEFF_ALIGN );
  U32 last_acs;
  U32 control;

  cnp[ 0 ] = control = dec_1_control( vb, cnp[ 0 ] );

  dtmEnterAccumulationZone( tm_mask, tm->decodeDCs );
  decode_dcs4( dcs, ydcs, Q, vb, flags, MININTRA, MAXINTRA );
  dtmLeaveAccumulationZone( tm_mask, tm->decodeDCs );

  dtmEnterAccumulationZone( tm_mask, tm->decodeACs );
  last_acs = dec_coeffs( control, coeffs, vb, bink_zigzag_scan_order_transposed, Q, bink_intra_chroma_scaleT, dcs, flags, a, DC_INTRA );
  dtmLeaveAccumulationZone( tm_mask, tm->decodeACs );

#ifndef NO_FULL_BLUR
  if ( last_acs != 0 )
#endif
  {
    RAD_ALIGN_SIMD( S16, out[16*16] );

    dtmEnterAccumulationZone( tm_mask, tm->idct );
    int_idctf8x8( out, coeffs, last_acs, 16*2 );
    dtmLeaveAccumulationZone( tm_mask, tm->idct );

    dtmEnterAccumulationZone( tm_mask, tm->deswizzle );
    if ( flags & (Y_MID_TOP_T1|Y_NO_BOTTOM_16) )  
    {
      pack16x8_to_u8( c+0*w, w, (U8 *) (out+0*16), 16*sizeof(S16) );
    }
    else if ( flags & Y_MID_TOP_T2 )
    {
      pack16x8_to_u8( c+8*w, w, (U8 *) (out+8*16), 16*sizeof(S16) );
    }
    else
    {
      pack16x8_to_u8( c+0*w, w, (U8 *) (out+0*16), 16*sizeof(S16) );
      pack16x8_to_u8( c+8*w, w, (U8 *) (out+8*16), 16*sizeof(S16) );
    }
    dtmLeaveAccumulationZone( tm_mask, tm->deswizzle );
  }
}

static RADINLINE void UandD_s16_blur( S16 * col, U32 dcfL, U32 dcfR, U32 pitch )
{
  // NOTE: this code assumes that both the INTRA bit (bit 15) and bit 12 of the dc flags are 0.
  U32 str0 = dcfL >> 13;
  U32 str1 = dcfR >> 13;

  // if either both strengths are 0, or both strengths are 3 and the absolute
  // difference of DCs abs(dcL - dcR) <= 32...
  if ( ( dcfL | dcfR ) < ( 1 << 13 ) ||
       ( ( dcfL & dcfR ) >= ( 3 << 13 ) && ( dcfL ^ 0x1000 ) - ( dcfR ^ 0x1000 ) + 32 < 65 ) )
    return;

  mublock16_UtoD( (U8*) col, pitch,
                  (U8*) &bink_mublock16_masks[ str0*8 + 0 ], (U8*) &bink_mublock16_masks[ str0*8 + 8 ],
                  (U8*) &bink_mublock16_masks[ str1*8 + 8 ], (U8*) &bink_mublock16_masks[ str1*8 + 0 ] );
}

static RADINLINE void LtoR_s16_blur( S16 * row, U32 dcfT, U32 dcfB, U32 pitch )
{
  // NOTE: this code assumes that both the INTRA bit (bit 15) and bit 12 of the dc flags are 0.
  U32 str0 = dcfT >> 13;
  U32 str1 = dcfB >> 13;

  // if either both strengths are 0, or both strengths are 3 and the absolute
  // difference of DCs abs(dcT - dcB) <= 32...
  if ( ( dcfT | dcfB ) < ( 1 << 13 ) ||
       ( ( dcfT & dcfB ) >= ( 3 << 13 ) && ( dcfT ^ 0x1000 ) - ( dcfB ^ 0x1000 ) + 32 < 65 ) )
    return;

  mublock16_LtoR( (U8*) row, pitch,
                  (U8*) &bink_mublock16_masks[ str0*8 + 0 ], (U8*) &bink_mublock16_masks[ str0*8 + 8 ],
                  (U8*) &bink_mublock16_masks[ str1*8 + 8 ], (U8*) &bink_mublock16_masks[ str1*8 + 0 ] );
}


// edge blurring dec_mdct
static void dec_mdct_luma( OPT_PROFILE_PLANE  U8 * RADRESTRICT oy, U8 * RADRESTRICT yp, U32 dw, U32 pw, VARBITSEND * vb, U16 * RADRESTRICT a, U32 flags, S32 const * RADRESTRICT coords, S32 Q, U32 * RADRESTRICT cnp, U8 * RADRESTRICT right_edge )
{
  S32 rects;
  RAD_ALIGN_SIMD( S32, dcs[16] );
  RAD_ALIGN_SIMD( U8, l[ 16 * 16 ] );
  RAD_ALIGN_SIMD( S16, out[ 32 * 32 ] );
  U16 dcflags[ 16 ];
  U32 control;

  cnp[0] = control = dec_4_controls( vb, cnp[0], flags );

  dtmEnterAccumulationZone( tm_mask, tm->decodeDCs );
  decode_dcs16( dcs, 0, Q, vb, X_LEFT|Y_TOP|Y_MID_TOP|BINKVER22, (flags&BINKVER25)?MININTER25:OLDMININTER, (flags&BINKVER25)?MAXINTER25:OLDMAXINTER );  
  dtmLeaveAccumulationZone( tm_mask, tm->decodeDCs );

  // decode the residual plane into out
  for( rects = 0 ; rects < 4 ; rects++ )
  {
    RAD_ALIGN( S16, coeffs[ 64*4 ], COEFF_ALIGN );
    U32 last_acs;
    
    dtmEnterAccumulationZone( tm_mask, tm->decodeACs );
    last_acs = dec_coeffs( control, coeffs, vb, bink_zigzag_scan_order_transposed, Q, bink_inter_scaleT, dcs + 4*rects, flags, dcflags + 4*rects, DC_INTER );
    control >>= 4;
    dtmLeaveAccumulationZone( tm_mask, tm->decodeACs );

    if ( yp )
    {
      U32 oofs;
      oofs = ( ( rects & 1 ) << 4 ) + ( ( rects & 2 ) << (3+5) );  // 0, 16, 16*32 or 16*32+16

      dtmEnterAccumulationZone( tm_mask, tm->idct );
      int_idctf8x8( (out+oofs), coeffs, last_acs, 32*2 );
      dtmLeaveAccumulationZone( tm_mask, tm->idct );

      // do the up and down edges immediately
      dtmEnterAccumulationZone( tm_mask, tm->mdct_deblock );
      if ( RAD_LIKELY( ( flags & (BINK_DEBLOCK_UANDD_OFF|BINKVER25) ) == 0 ) )
      {
        if( rects & 1 )
        {
          UandD_s16_blur( out-2+oofs,      dcflags[4*rects-3], dcflags[4*rects+0], 32*2 );
          UandD_s16_blur( out-2+oofs+8*32, dcflags[4*rects-1], dcflags[4*rects+2], 32*2 );
        }
       UandD_s16_blur( out+6+oofs,      dcflags[4*rects+0], dcflags[4*rects+1], 32*2 );
       UandD_s16_blur( out+6+oofs+8*32, dcflags[4*rects+2], dcflags[4*rects+3], 32*2 );
      }
      dtmLeaveAccumulationZone( tm_mask, tm->mdct_deblock );
    }
  }
  
  if ( yp )
  {
    // left and right edge blurs within each 32x32
    dtmEnterAccumulationZone( tm_mask, tm->mdct_deblock );
    if ( RAD_LIKELY( ( flags & (BINK_DEBLOCK_LTOR_OFF|BINKVER25) ) == 0 ) )
    {
      S32 col;
      for( col = 0 ; col < 4 ; col++ )
      {
        S32 ofs = col << 3;
        U32 swizx = ( col & 1 ) | ( ( col & 2 ) << 1 );

        LtoR_s16_blur( out+ofs+ 6*32, dcflags[swizx+ 0], dcflags[swizx+ 2], 32*2 );
        LtoR_s16_blur( out+ofs+14*32, dcflags[swizx+ 2], dcflags[swizx+ 8], 32*2 );
        LtoR_s16_blur( out+ofs+22*32, dcflags[swizx+ 8], dcflags[swizx+10], 32*2 );
      }
    }
    dtmLeaveAccumulationZone( tm_mask, tm->mdct_deblock );

    // ok, now add move comp data to out, and then copy it into the output plane
    for( rects = 0 ; rects < 4 ; rects++ )
    {
      U8 * RADRESTRICT to;
      U32 tw;
      S32 ofs;

      ofs = ( ( rects & 1 ) << 4 );  // 0 or 16
      if ( rects & 2 ) 
        ofs += ( dw * 16 );
     
      to = oy + ofs;
      tw = dw;

      if ( RAD_UNLIKELY( flags & (Y_MID_TOP_T1|Y_NO_BOTTOM_16|Y_MID_TOP_T2 ) ) )
      {
        if ( flags & Y_MID_TOP_T2 )
        {
          if ( RAD_UNLIKELY( rects == 0 ) )
            continue;
          if ( RAD_UNLIKELY(rects == 1 ) )
          {
            to = right_edge;
            tw = 16;
          }
        }
        else
        {
          if ( RAD_UNLIKELY( rects == 2 ) )
            continue;
          if ( RAD_UNLIKELY( rects == 3 ) )
          {
            to = right_edge;
            tw = 16;
          }
        }
      }

      {
        U32 oofs;

        oofs = ( ( rects & 1 ) << 4 ) + ( ( rects & 2 ) << (3+5) );  // 0, 16, 16*32 or 16*32+16

        dtmEnterAccumulationZone( tm_mask, tm->mosample );
        bink2_get_luma_modata( l, yp, coords[rects*2], coords[rects*2+1], 16, pw );
        dtmLeaveAccumulationZone( tm_mask, tm->mosample );

        dtmEnterAccumulationZone( tm_mask, tm->deswizzle_mot );
        pack16x8_to_u8_and_mocomp( to+0*tw, tw, (U8 *) (out+0*32+oofs), 32*sizeof(S16), (U8 *) (l+0*16), 16 );
        pack16x8_to_u8_and_mocomp( to+8*tw, tw, (U8 *) (out+8*32+oofs), 32*sizeof(S16), (U8 *) (l+8*16), 16 );
        dtmLeaveAccumulationZone( tm_mask, tm->deswizzle_mot );
      }
    }
  }
}

static void dec_mdct_chroma( OPT_PROFILE_PLANE  U8 * RADRESTRICT c, U8 * RADRESTRICT oc, U32 dw, U32 pw, VARBITSEND * vb, U16 * RADRESTRICT a, S32 const * RADRESTRICT coords, U32 flags, S32 Q, U32 * RADRESTRICT cnp, U8 * RADRESTRICT right_edge )
{
  RAD_ALIGN_SIMD( S32, dcs[ 4 ] );
  RAD_ALIGN( S16, coeffs[ 64*4 ], COEFF_ALIGN );
  RAD_ALIGN_SIMD( U8, lc[ 16 * 16 ] );
  U16 dcflags[ 4 ];
  U32 control;
  U32 last_acs;

  cnp[0] = control = dec_1_control( vb, cnp[0] );

  dtmEnterAccumulationZone( tm_mask, tm->decodeDCs );
  decode_dcs4( dcs, 0, Q, vb, X_LEFT|Y_TOP|Y_MID_TOP|BINKVER22, (flags&BINKVER25)?MININTER25:OLDMININTER, (flags&BINKVER25)?MAXINTER25:OLDMAXINTER );  
  dtmLeaveAccumulationZone( tm_mask, tm->decodeDCs );

  dtmEnterAccumulationZone( tm_mask, tm->decodeACs );
  last_acs = dec_coeffs( control, coeffs, vb, bink_zigzag_scan_order_transposed, Q, bink_inter_scaleT, dcs, flags, dcflags, DC_INTER );
  dtmLeaveAccumulationZone( tm_mask, tm->decodeACs );

  dtmEnterAccumulationZone( tm_mask, tm->mosample );
  bink2_get_chroma_modata( lc,           oc, coords[0], coords[1], 16, pw );
  bink2_get_chroma_modata( lc+8,         oc, coords[2], coords[3], 16, pw );
  bink2_get_chroma_modata( lc+16*8,      oc, coords[4], coords[5], 16, pw );
  bink2_get_chroma_modata( lc+16*8+8,    oc, coords[6], coords[7], 16, pw );
  dtmLeaveAccumulationZone( tm_mask, tm->mosample );

  {
    RAD_ALIGN_SIMD( S16, out[16*16] );

    dtmEnterAccumulationZone( tm_mask, tm->idct );
    int_idctf8x8( out, coeffs, last_acs, 16*2 );
    dtmLeaveAccumulationZone( tm_mask, tm->idct );

    // do blurs
  
    // up and down edge blurs within each 32x32
    dtmEnterAccumulationZone( tm_mask, tm->mdct_deblock );
    if ( RAD_LIKELY( ( flags & (BINK_DEBLOCK_UANDD_OFF|BINKVER25) ) == 0 ) )
    {
      UandD_s16_blur( out+6,      dcflags[0], dcflags[1], 16*2 );
      UandD_s16_blur( out+6+8*16, dcflags[2], dcflags[3], 16*2 );
    }

    // left and right edge blurs within each 32x32
    if ( RAD_LIKELY( ( flags & (BINK_DEBLOCK_LTOR_OFF|BINKVER25) ) == 0 ) )
    {
      LtoR_s16_blur( out+6*16,   dcflags[0], dcflags[2], 16*2 );
      LtoR_s16_blur( out+6*16+8, dcflags[1], dcflags[3], 16*2 );
    }
    dtmLeaveAccumulationZone( tm_mask, tm->mdct_deblock );

    dtmEnterAccumulationZone( tm_mask, tm->deswizzle_mot );
    {
      U8 * RADRESTRICT dest = c;
      U32 dp = dw;

      if ( RAD_UNLIKELY( flags & Y_MID_TOP_T2 ) )
      {
        dest = right_edge;
        dp = 16;
      }
      pack16x8_to_u8_and_mocomp( dest, dp, (U8 *) (out+0*16), 16*sizeof(S16), (U8 *) (lc+0*16), 16 );


      dest = c+8*dw;
      dp = dw;

      if ( RAD_UNLIKELY( flags & (Y_MID_TOP_T1|Y_NO_BOTTOM_16) ) )
      {
        dest = right_edge;
        dp = 16;
      }

      pack16x8_to_u8_and_mocomp( dest, dp, (U8 *) (out+8*16), 16*sizeof(S16), (U8 *) (lc+8*16), 16 );
    }
    dtmLeaveAccumulationZone( tm_mask, tm->deswizzle_mot );
  }
}

#ifdef BINK20_SUPPORT
// Bink 2.0 backwards compat code here:
#include "expand2_bink20.inl"
#endif

// NOTE(fg): Prefetch / memory commit functions. Fully unrolling / inlining these
// wastes lots of I$ space for no good reason.

#ifdef MemoryCommit

static void MemoryCommitMultiple_x2( U8 * RADRESTRICT ptr, UINTa pitch, U32 count )
{
  do
  {
    MemoryCommit( ptr );
    MemoryCommit( ptr + pitch );
    ptr += 2*pitch;
  } while (--count);
}

#endif

#ifdef PrefetchForRead

static void PrefetchForRead_x5( U8 const * RADRESTRICT ptr, UINTa pitch )
{
  PrefetchForRead( ptr ); PrefetchForRead( ptr + pitch ); ptr += 2*pitch;
  PrefetchForRead( ptr ); PrefetchForRead( ptr + pitch ); ptr += 2*pitch;
  PrefetchForRead( ptr );
}

static RADNOINLINE void PrefetchForRead_x8( U8 const * RADRESTRICT ptr, UINTa pitch )
{
  PrefetchForRead( ptr ); PrefetchForRead( ptr + pitch ); ptr += 2*pitch;
  PrefetchForRead( ptr ); PrefetchForRead( ptr + pitch ); ptr += 2*pitch;
  PrefetchForRead( ptr ); PrefetchForRead( ptr + pitch ); ptr += 2*pitch;
  PrefetchForRead( ptr ); PrefetchForRead( ptr + pitch ); ptr += 2*pitch;
}

static RADNOINLINE void PrefetchForRead_x10( U8 const * RADRESTRICT ptr, UINTa pitch )
{
  PrefetchForRead( ptr ); PrefetchForRead( ptr + pitch ); ptr += 2*pitch;
  PrefetchForRead( ptr ); PrefetchForRead( ptr + pitch ); ptr += 2*pitch;
  PrefetchForRead( ptr ); PrefetchForRead( ptr + pitch ); ptr += 2*pitch;
  PrefetchForRead( ptr ); PrefetchForRead( ptr + pitch ); ptr += 2*pitch;
  PrefetchForRead( ptr ); PrefetchForRead( ptr + pitch ); ptr += 2*pitch;
}

#endif

#ifdef PrefetchForOverwrite

static RADNOINLINE U8 * PrefetchForOverwrite_x8( U8 * RADRESTRICT ptr, UINTa pitch )
{
  PrefetchForOverwrite( ptr ); PrefetchForOverwrite( ptr + pitch ); ptr += 2*pitch;
  PrefetchForOverwrite( ptr ); PrefetchForOverwrite( ptr + pitch ); ptr += 2*pitch;
  PrefetchForOverwrite( ptr ); PrefetchForOverwrite( ptr + pitch ); ptr += 2*pitch;
  PrefetchForOverwrite( ptr ); PrefetchForOverwrite( ptr + pitch ); ptr += 2*pitch;

  return ptr;
}

#endif

#define different_cache_lines( ptra, ptrb ) ( ( (UINTa)(ptra) ^ (UINTa)(ptrb) ) >= RR_CACHE_LINE_SIZE )

// valid window: top-left corners (in half-pel coordinates)
static U32 const bink2_motion_old_tl[ 2 ] = {      0,       4 }; // 0=fullpel, 1=halfpel
static U32 const bink2_motion_new_tl[ 2 ] = {      0,       5 }; // 0=fullpel, 1=halfpel
// valid window: width/height subtract
static U32 const bink2_motion_wh[ 2 ]     = { 16*2+0, 16*2+10 }; // 0=fullpel, 1=halfpel

// NB this is broken. Need to put in a version bit for this!
static U32 RADFORCEINLINE bink2_clamp_mo_component( U32 x, U32 max_x )
{
  // if the motion coordinates are out of bounds, clamp them.
  // NB this is using unsigned overflow.
  U32 tl = bink2_motion_old_tl[ x&1 ];
  U32 effective_wh = max_x - bink2_motion_wh[ x&1 ];
 
  if ( RAD_UNLIKELY( ( x - tl ) > effective_wh ) )
    x = ( x < tl ) ? tl : effective_wh;

  return x;
}

static U32 RADFORCEINLINE bink2_clamp_mo_component_new( U32 x, U32 max_x )
{
  // if the motion coordinates are out of bounds, clamp them.
  // NB this is using unsigned overflow.
  U32 tl = bink2_motion_new_tl[ x&1 ];
  U32 effective_wh = max_x - bink2_motion_wh[ x&1 ];
  if ( RAD_UNLIKELY( ( x - tl ) > effective_wh ) )
    x = tl + ( ( (S32)x < (S32)tl ) ? 0 : effective_wh );

  return x;
}

U32 bink2_clamp_mo_component_old_comp( U32 x, U32 max_x )
{
  return  bink2_clamp_mo_component( x, max_x );
}

U32 bink2_clamp_mo_component_new_comp( U32 x, U32 max_x )
{
  return  bink2_clamp_mo_component_new( x, max_x );
}


static void prepare_mos( S32 * RADRESTRICT coords, U8 * RADRESTRICT yp, U8 * RADRESTRICT crp, U8 * RADRESTRICT cbp, U32 basex, U32 basey, U32 lpitch, U32 cpitch, S32 const * RADRESTRICT rel_coords, U32 flags, U32 w2, U32 h2 )
{
  S32 b;

  basex += basex;
  basey += basey;

  for (b=0;b<8;b+=2)
  {
#ifdef PrefetchForRead
    U8 const * RADRESTRICT ptr;
    U8 const * RADRESTRICT ptrh;
#endif
    U32 x = basex + ((b&2) * 16) + rel_coords[b];
    U32 y = basey + ((b&4) *  8) + rel_coords[b+1];

    if ( RAD_LIKELY( flags & BINKVER23EDGEMOFIX ) )
    {
      x = bink2_clamp_mo_component_new( x, w2 );
      y = bink2_clamp_mo_component_new( y, h2 );
    }
    else
    {
      x = bink2_clamp_mo_component( x, w2 );
      y = bink2_clamp_mo_component( y, h2 );
    }

    coords[b] = x;
    coords[b+1] = y;

#ifdef PrefetchForRead
    // the x&1 stuff here accounts for the fact that luma half-pel reads
    // start 2 samples "early" and read another 16 bytes (or 8 on SSE,
    // but that doesn't go through this path).

    ptr = yp + ( ( x >> 1 ) - ( x & 1 ) * 2 ) + lpitch * ( ( y >> 1 ) - ( y & 1 ) * 2 );
    PrefetchForRead_x8( ptr, lpitch );
    PrefetchForRead_x8( ptr+8*lpitch, lpitch );
    if( y & 1 )
      PrefetchForRead_x5( ptr+16*lpitch, lpitch );

    ptrh = ptr + 15 + ( x & 1 ) * 16;
    if ( different_cache_lines( ptr, ptrh ) )
    {
      PrefetchForRead_x8( ptrh, lpitch );
      PrefetchForRead_x8( ptrh+8*lpitch, lpitch );
      if( y & 1 )
        PrefetchForRead_x5( ptrh+16*lpitch, lpitch );
    }

    ptr = crp + ( x >> 2 ) + cpitch * ( y >> 2 );
    PrefetchForRead_x8( ptr, cpitch );
    if ( different_cache_lines( ptr, ptr + 7 ) )
      PrefetchForRead_x8( ptr + 7, cpitch );

    ptr = cbp + ( x >> 2 ) + cpitch * ( y >> 2 );
    PrefetchForRead_x8( ptr, cpitch );
    if ( different_cache_lines( ptr, ptr + 7 ) )
      PrefetchForRead_x8( ptr + 7, cpitch );
#endif
  }
}

#define GETBLUR( dest, dcs ) { dest = ((((U32)(dcs)[0])+(1<<13))>>16)|(((((U32)(dcs)[1])+(1<<13))>>15)&2); }

#define full_blur( table, ibdc0, ibdc1, ibdc2, p, pitch ) \
  blur_fns[ table ]( (U8*)(ibdc0), (U8*)(ibdc1), (U8*)(ibdc2), (U8*)threshold, (p), pitch )


#define get_strength1( a, i ) (((a)[i] >> 13) & 3)
#define get_strength2( a, i ) (((a)[i] >> 11) & (3<<2))

#define get_LtoR_blur( top, bot ) \
            ( LtoR_zero15[ get_strength1(top,0) | (get_strength2(bot,0)) ] | \
             ( LtoR_zero15shl4[get_strength1(top,1) | (get_strength2(bot,1)) ] ) )

// called for LtoR edges, where top and bot are different block types (same currently)
#define get_LtoR_diff_blur( top, bot ) \
            get_LtoR_blur( top, bot )

#define DIFFERENT_MO_COORDS_BLUR_LTOR (2|(2<<2)|(2<<4)|(2<<6))
#define DIFFERENT_MO_COORDS_BLUR1_LTOR (2|(2<<2))


#define get_UandD_blur( left, right ) \
            UandD_zero15times8[get_strength1(left,0)|get_strength2(right,0)]

// called for UandD edges, where Left and Right are different block types (same currently)
#define get_UandD_diff_blur( left, right ) \
            get_UandD_blur( left, right )

#define DIFFERENT_MO_COORDS_BLUR1_UANDD ((2|(2<<2))<<3)

// within one of the coord in the x and y?
#define different_coords( p1, p2 ) ( ( ((U32)(1+((p1)[0])-((p2)[0]))) > 2 ) || ( ((U32)(1+((p1)[1])-((p2)[1]))) > 2 ) )

static void do_LtoR_Xm16( U8 * RADRESTRICT yt, U8 * RADRESTRICT cr, U8 * RADRESTRICT cb, U32 * locals, S32 lpitch, S32 cpitch )
{
  U32 m0, mcr, mcb;
  S32 lpitch8 = lpitch * 8;

  m0 = locals[0];
  mcr = locals[1];
  mcb = locals[2];
  if ( m0 )
  {
    mublock_LtoR( m0 & 255, yt - lpitch8 - 16, lpitch );
    mublock_LtoR( ( m0 >> 8 ) & 255, yt - 16, lpitch );
    mublock_LtoR( ( m0 >> 16 ) & 255, yt + lpitch8 - 16, lpitch );
    mublock_LtoR( m0 >> 24, yt + lpitch8 + lpitch8 - 16, lpitch );
  }

  if ( mcr )
  {
    mublock_LtoR( mcr >> 24, yt + lpitch8 + lpitch8 + lpitch8 - 16, lpitch ); // only on final lines

    mublock_LtoR( mcr & 255, cr - cpitch*8 - 16, cpitch );
    mublock_LtoR( ( mcr >> 8 ) & 255, cr - 16, cpitch );
    mublock_LtoR( ( mcr >> 16 ) & 255, cr + cpitch*8 - 16, cpitch );
  }

  if ( mcb )
  {
    mublock_LtoR( mcb & 255, cb - cpitch*8 - 16, cpitch );
    mublock_LtoR( ( mcb >> 8 ) & 255, cb - 16, cpitch );
    mublock_LtoR( ( mcb >> 16 ) & 255, cb + cpitch*8 - 16, cpitch );
  }
}

typedef struct LOCAL_ROW_DCS
{
  //  [ W3 ] [ P0 ] [ P1 ] [ P2 ] [ P3 ] [ C0 ] [ C1 ] [ C2 ]
  //  [ W3 ] [ P0 ] [ P1 ] [ P2 ] [ P3 ] [ C0 ] [ C1 ] [ C2 ]
  //  [ W3 ] [ P0 ] [ P1 ] [ P2 ] [ P3 ] [ C0 ] [ C1 ] [ C2 ]
  //  [ W3 ] [ P0 ] [ P1 ] [ P2 ] [ P3 ] [ C0 ] [ C1 ] [ C2 ]
  //  [ C3 ] [____] [ C3 ] [____] [ C3 ] [____] [ C3 ] [____] 
  U16 ylocals[ 40 ];
  U16 alocals[ 40 ];
  U16 hlocals[ 40 ];

  //  [WWW3] [ W0 ] [ W1 ] [ P0 ]
  //  [WWW3] [ W0 ] [ W1 ] [ P0 ]
  //  [ P1 ] [____] [ P1 ] [____]
  U16 crlocals[ 16 ]; // only use 12
  U16 cblocals[ 16 ]; // only use 12

  U32 next_LtoRs[4];  // only use 3
} LOCAL_ROW_DCS;

static RADNOINLINE void deblock( OPT_PROFILE LOCAL_ROW_DCS * RADRESTRICT local_row, U16 * RADRESTRICT ylinearDCs, U16 * RADRESTRICT alinearDCs, U16 * RADRESTRICT hlinearDCs, U16 * RADRESTRICT clinearDCs, BLOCK_DCT * c, U32 flags, U8 * RADRESTRICT yt, U8 * RADRESTRICT at, U8 * RADRESTRICT ht, U8 * RADRESTRICT cr, U8 * RADRESTRICT cb, U32 ydcpitch, U32 cdcpitch, U32 lpitch, U32 cpitch, S32 * RADRESTRICT ccoords, S32 * RADRESTRICT pcoords, U16 const * threshold, void * seamp )
{
  register U32 m0, m1, mcr, mcb;
  U32 lpitch8;
  U16 * RADRESTRICT hlinearDCsnext;
  U16 * RADRESTRICT alinearDCsnext;
  U16 * RADRESTRICT ylinearDCsnext;
  U16 * RADRESTRICT crlinearDCs;
  U16 * RADRESTRICT crlinearDCsnext;
  U16 * RADRESTRICT cblinearDCs;
  U16 * RADRESTRICT cblinearDCsnext;
  U16 * RADRESTRICT hlocal;
  U16 * RADRESTRICT alocal;
  U16 * RADRESTRICT ylocal;
  U16 * RADRESTRICT crlocal;
  U16 * RADRESTRICT cblocal;
  U32 seam_ofs = 0;
  U8 * RADRESTRICT seam = 0;
  new_dc_blur_jump * * blur_fns = new_dc_blur;

  dtmEnterAccumulationZone( tm_mask, tm->deblock );

  if ( ( seamp ) && ( RAD_UNLIKELY( flags & ( Y_MID_TOP_T2_NEXT | Y_MID_TOP_T2 ) ) ) )
  {
    seam_ofs = ((U32*)seamp)[0]; // high bit is frame type
    seam = ((U8*)seamp) + 4 + seam_ofs;
  }

  hlinearDCsnext = (U16*)((UINTa)hlinearDCs+ydcpitch);
  alinearDCsnext = (U16*)((UINTa)alinearDCs+ydcpitch);
  ylinearDCsnext = (U16*)((UINTa)ylinearDCs+ydcpitch);

  hlocal = local_row->hlocals;
  alocal = local_row->alocals;
  ylocal = local_row->ylocals;
  crlocal = local_row->crlocals;
  cblocal = local_row->cblocals;

  crlinearDCs = (U16*)((UINTa)clinearDCs+0);
  crlinearDCsnext = (U16*)((UINTa)crlinearDCs+cdcpitch);
  cblinearDCs = (U16*)((UINTa)crlinearDCsnext+cdcpitch);
  cblinearDCsnext = (U16*)((UINTa)cblinearDCs+cdcpitch);

  lpitch8 = lpitch * 8;

  yt -= 32;
  if ( RAD_UNLIKELY( at != 0 ) ) at -= 32;
  if ( RAD_UNLIKELY( ht != 0 ) ) ht -= 32;
  cr -= 16;
  cb -= 16;

#ifdef BINK20_SUPPORT
  if ( RAD_UNLIKELY( ( flags & BINKVER22 ) == 0 ) )
  {
    blur_fns = dc_blur5_s16;
  }
#endif

  if ( RAD_UNLIKELY( flags & X_LEFT ) )  // x==0
  {
    // clear all the previous blurs
    ((simd4i*)local_row->next_LtoRs)[0] = simd4i_0;

    if ( at )
    {
      // clear the rows
      ((simd4i*)alocal)[0] = simd4i_0;
      ((simd4i*)alocal)[1] = simd4i_0;
      ((simd4i*)alocal)[2] = simd4i_0;
      ((simd4i*)alocal)[3] = simd4i_0;
   
      alocal[5+0]=((U16* RADRESTRICT)c->adcts)[0];
      alocal[5+1]=((U16* RADRESTRICT)c->adcts)[1];
      alocal[5+2]=((U16* RADRESTRICT)c->adcts)[4];
      
      alocal[8+5+0]=((U16* RADRESTRICT)c->adcts)[2];
      alocal[8+5+1]=((U16* RADRESTRICT)c->adcts)[3];
      alocal[8+5+2]=((U16* RADRESTRICT)c->adcts)[6];
      
      alocal[16+5+0]=((U16* RADRESTRICT)c->adcts)[8];
      alocal[16+5+1]=((U16* RADRESTRICT)c->adcts)[9];
      alocal[16+5+2]=((U16* RADRESTRICT)c->adcts)[12];
      
      alocal[24+5+0]=((U16*)c->adcts)[10];
      alocal[24+5+1]=((U16*)c->adcts)[11];
      alocal[24+5+2]=((U16*)c->adcts)[14];
      
      alocal[32+0]= ((U16*)c->adcts)[5];
      alocal[32+2]= ((U16*)c->adcts)[7];
      alocal[32+4]= ((U16*)c->adcts)[13];
      alocal[32+6]= ((U16*)c->adcts)[15];
    }

    if ( ht )
    {
      // clear the rows
      ((simd4i*)hlocal)[0] = simd4i_0;
      ((simd4i*)hlocal)[1] = simd4i_0;
      ((simd4i*)hlocal)[2] = simd4i_0;
      ((simd4i*)hlocal)[3] = simd4i_0;
   
      hlocal[5+0]=((U16* RADRESTRICT)c->hdcts)[0];
      hlocal[5+1]=((U16* RADRESTRICT)c->hdcts)[1];
      hlocal[5+2]=((U16* RADRESTRICT)c->hdcts)[4];
      
      hlocal[8+5+0]=((U16* RADRESTRICT)c->hdcts)[2];
      hlocal[8+5+1]=((U16* RADRESTRICT)c->hdcts)[3];
      hlocal[8+5+2]=((U16* RADRESTRICT)c->hdcts)[6];
      
      hlocal[16+5+0]=((U16* RADRESTRICT)c->hdcts)[8];
      hlocal[16+5+1]=((U16* RADRESTRICT)c->hdcts)[9];
      hlocal[16+5+2]=((U16* RADRESTRICT)c->hdcts)[12];
      
      hlocal[24+5+0]=((U16*)c->hdcts)[10];
      hlocal[24+5+1]=((U16*)c->hdcts)[11];
      hlocal[24+5+2]=((U16*)c->hdcts)[14];
      
      hlocal[32+0]= ((U16*)c->hdcts)[5];
      hlocal[32+2]= ((U16*)c->hdcts)[7];
      hlocal[32+4]= ((U16*)c->hdcts)[13];
      hlocal[32+6]= ((U16*)c->hdcts)[15];
    }

    // clear the rows
    ((simd4i*)ylocal)[0] = simd4i_0;
    ((simd4i*)ylocal)[1] = simd4i_0;
    ((simd4i*)ylocal)[2] = simd4i_0;
    ((simd4i*)ylocal)[3] = simd4i_0;
 
    ylocal[5+0]=((U16* RADRESTRICT)c->ydcts)[0];
    ylocal[5+1]=((U16* RADRESTRICT)c->ydcts)[1];
    ylocal[5+2]=((U16* RADRESTRICT)c->ydcts)[4];
    
    ylocal[8+5+0]=((U16* RADRESTRICT)c->ydcts)[2];
    ylocal[8+5+1]=((U16* RADRESTRICT)c->ydcts)[3];
    ylocal[8+5+2]=((U16* RADRESTRICT)c->ydcts)[6];
    
    ylocal[16+5+0]=((U16* RADRESTRICT)c->ydcts)[8];
    ylocal[16+5+1]=((U16* RADRESTRICT)c->ydcts)[9];
    ylocal[16+5+2]=((U16* RADRESTRICT)c->ydcts)[12];
    
    ylocal[24+5+0]=((U16* RADRESTRICT)c->ydcts)[10];
    ylocal[24+5+1]=((U16* RADRESTRICT)c->ydcts)[11];
    ylocal[24+5+2]=((U16* RADRESTRICT)c->ydcts)[14];
    
    ylocal[32+0]= ((U16* RADRESTRICT)c->ydcts)[5];
    ylocal[32+2]= ((U16* RADRESTRICT)c->ydcts)[7];
    ylocal[32+4]= ((U16* RADRESTRICT)c->ydcts)[13];
    ylocal[32+6]= ((U16* RADRESTRICT)c->ydcts)[15];

    ((simd4i*)crlocal)[0] = simd4i_0;
    ((simd4i*)crlocal)[1] = simd4i_0;

    crlocal[3] = ((U16* RADRESTRICT)c->cdcts)[0];
    crlocal[4+3] = ((U16* RADRESTRICT)c->cdcts)[2];

    crlocal[8+0] = ((U16* RADRESTRICT)c->cdcts)[1];
    crlocal[8+2] = ((U16* RADRESTRICT)c->cdcts)[3];

    ((simd4i*)cblocal)[0] = simd4i_0;
    ((simd4i*)cblocal)[1] = simd4i_0;

    cblocal[3] = ((U16* RADRESTRICT)c->cdcts)[4+0];
    cblocal[4+3] = ((U16* RADRESTRICT)c->cdcts)[4+2];

    cblocal[8+0] = ((U16* RADRESTRICT)c->cdcts)[4+1];
    cblocal[8+2] = ((U16* RADRESTRICT)c->cdcts)[4+3];
  }
  else
  {
    // turns:

    //  [WWW3] [ W0 ] [ W1 ] [ W2 ] [ W3 ] [ P0 ] [ P1 ] [ P2 ]
    //  [WWW3] [ W0 ] [ W1 ] [ W2 ] [ W3 ] [ P0 ] [ P1 ] [ P2 ]
    //  [WWW3] [ W0 ] [ W1 ] [ W2 ] [ W3 ] [ P0 ] [ P1 ] [ P2 ]
    //  [WWW3] [ W0 ] [ W1 ] [ W2 ] [ W3 ] [ P0 ] [ P1 ] [ P2 ]
    //  [ P3 ] [____] [ P3 ] [____] [ P3 ] [____] [ P3 ] [____] 

    // into:

    //  [ W3 ] [ P0 ] [ P1 ] [ P2 ] [ P3 ] [ C0 ] [ C1 ] [ C2 ]
    //  [ W3 ] [ P0 ] [ P1 ] [ P2 ] [ P3 ] [ C0 ] [ C1 ] [ C2 ]
    //  [ W3 ] [ P0 ] [ P1 ] [ P2 ] [ P3 ] [ C0 ] [ C1 ] [ C2 ]
    //  [ W3 ] [ P0 ] [ P1 ] [ P2 ] [ P3 ] [ C0 ] [ C1 ] [ C2 ]
    //  [ C3 ] [____] [ C3 ] [____] [ C3 ] [____] [ C3 ] [____] 

    if ( RAD_UNLIKELY( ( alocal[0]|alocal[1]|alocal[5]|((U16*)c->adcts)[0] ) & 0x8000 ) )
    {
      if ( RAD_UNLIKELY( at != 0 ) )
      {
        ((U64* RADRESTRICT)(alocal))[0] = ((U64* RADRESTRICT)(alocal))[1];
        ((U64* RADRESTRICT)(alocal))[2] = ((U64* RADRESTRICT)(alocal))[3];
        ((U64* RADRESTRICT)(alocal))[4] = ((U64* RADRESTRICT)(alocal))[5];
        ((U64* RADRESTRICT)(alocal))[6] = ((U64* RADRESTRICT)(alocal))[7];

        alocal[4]=alocal[32+0];
        alocal[5+0]=((U16*)c->adcts)[0];
        alocal[5+1]=((U16*)c->adcts)[1];
        alocal[5+2]=((U16*)c->adcts)[4];
        
        alocal[8+4]=alocal[32+2];
        alocal[8+5+0]=((U16* RADRESTRICT)c->adcts)[2];
        alocal[8+5+1]=((U16* RADRESTRICT)c->adcts)[3];
        alocal[8+5+2]=((U16* RADRESTRICT)c->adcts)[6];
        
        alocal[16+4]=alocal[32+4];
        alocal[16+5+0]=((U16* RADRESTRICT)c->adcts)[8];
        alocal[16+5+1]=((U16* RADRESTRICT)c->adcts)[9];
        alocal[16+5+2]=((U16* RADRESTRICT)c->adcts)[12];
        
        alocal[24+4]=alocal[32+6];
        alocal[24+5+0]=((U16* RADRESTRICT)c->adcts)[10];
        alocal[24+5+1]=((U16* RADRESTRICT)c->adcts)[11];
        alocal[24+5+2]=((U16* RADRESTRICT)c->adcts)[14];
        
        alocal[32+0]= ((U16* RADRESTRICT)c->adcts)[5];
        alocal[32+2]= ((U16* RADRESTRICT)c->adcts)[7];
        alocal[32+4]= ((U16* RADRESTRICT)c->adcts)[13];
        alocal[32+6]= ((U16* RADRESTRICT)c->adcts)[15];
      }
    }

    if ( RAD_UNLIKELY( ( hlocal[0]|hlocal[1]|hlocal[5]|((U16*)c->hdcts)[0] ) & 0x8000 ) )
    {
      if ( RAD_UNLIKELY( ht != 0 ) )
      {
        ((U64* RADRESTRICT)(hlocal))[0] = ((U64* RADRESTRICT)(hlocal))[1];
        ((U64* RADRESTRICT)(hlocal))[2] = ((U64* RADRESTRICT)(hlocal))[3];
        ((U64* RADRESTRICT)(hlocal))[4] = ((U64* RADRESTRICT)(hlocal))[5];
        ((U64* RADRESTRICT)(hlocal))[6] = ((U64* RADRESTRICT)(hlocal))[7];

        hlocal[4]=hlocal[32+0];
        hlocal[5+0]=((U16*)c->hdcts)[0];
        hlocal[5+1]=((U16*)c->hdcts)[1];
        hlocal[5+2]=((U16*)c->hdcts)[4];
        
        hlocal[8+4]=hlocal[32+2];
        hlocal[8+5+0]=((U16* RADRESTRICT)c->hdcts)[2];
        hlocal[8+5+1]=((U16* RADRESTRICT)c->hdcts)[3];
        hlocal[8+5+2]=((U16* RADRESTRICT)c->hdcts)[6];
        
        hlocal[16+4]=hlocal[32+4];
        hlocal[16+5+0]=((U16* RADRESTRICT)c->hdcts)[8];
        hlocal[16+5+1]=((U16* RADRESTRICT)c->hdcts)[9];
        hlocal[16+5+2]=((U16* RADRESTRICT)c->hdcts)[12];
        
        hlocal[24+4]=hlocal[32+6];
        hlocal[24+5+0]=((U16* RADRESTRICT)c->hdcts)[10];
        hlocal[24+5+1]=((U16* RADRESTRICT)c->hdcts)[11];
        hlocal[24+5+2]=((U16* RADRESTRICT)c->hdcts)[14];
        
        hlocal[32+0]= ((U16* RADRESTRICT)c->hdcts)[5];
        hlocal[32+2]= ((U16* RADRESTRICT)c->hdcts)[7];
        hlocal[32+4]= ((U16* RADRESTRICT)c->hdcts)[13];
        hlocal[32+6]= ((U16* RADRESTRICT)c->hdcts)[15];
      }
    }

    if ( RAD_UNLIKELY( ( ylocal[0]|ylocal[1]|ylocal[5]|((U16*)c->ydcts)[0] ) & 0x8000 ) )
    {
      ((U64* RADRESTRICT)(ylocal))[0] = ((U64* RADRESTRICT)(ylocal))[1];
      ((U64* RADRESTRICT)(ylocal))[2] = ((U64* RADRESTRICT)(ylocal))[3];
      ((U64* RADRESTRICT)(ylocal))[4] = ((U64* RADRESTRICT)(ylocal))[5];
      ((U64* RADRESTRICT)(ylocal))[6] = ((U64* RADRESTRICT)(ylocal))[7];

      ylocal[4]=ylocal[32+0];
      ylocal[5+0]=((U16* RADRESTRICT)c->ydcts)[0];
      ylocal[5+1]=((U16* RADRESTRICT)c->ydcts)[1];
      ylocal[5+2]=((U16* RADRESTRICT)c->ydcts)[4];
      
      ylocal[8+4]=ylocal[32+2];
      ylocal[8+5+0]=((U16* RADRESTRICT)c->ydcts)[2];
      ylocal[8+5+1]=((U16* RADRESTRICT)c->ydcts)[3];
      ylocal[8+5+2]=((U16* RADRESTRICT)c->ydcts)[6];
      
      ylocal[16+4]=ylocal[32+4];
      ylocal[16+5+0]=((U16* RADRESTRICT)c->ydcts)[8];
      ylocal[16+5+1]=((U16* RADRESTRICT)c->ydcts)[9];
      ylocal[16+5+2]=((U16* RADRESTRICT)c->ydcts)[12];
      
      ylocal[24+4]=ylocal[32+6];
      ylocal[24+5+0]=((U16* RADRESTRICT)c->ydcts)[10];
      ylocal[24+5+1]=((U16* RADRESTRICT)c->ydcts)[11];
      ylocal[24+5+2]=((U16* RADRESTRICT)c->ydcts)[14];
      
      ylocal[32+0]= ((U16* RADRESTRICT)c->ydcts)[5];
      ylocal[32+2]= ((U16* RADRESTRICT)c->ydcts)[7];
      ylocal[32+4]= ((U16* RADRESTRICT)c->ydcts)[13];
      ylocal[32+6]= ((U16* RADRESTRICT)c->ydcts)[15];

      // turns:

      //  [WWW3] [ W0 ] [ W1 ] [ P0 ]
      //  [WWW3] [ W0 ] [ W1 ] [ P0 ]
      //  [ P1 ] [____] [ P1 ] [____]

      // into:

      //  [ W1 ] [ P0 ] [ P1 ] [ C0 ]
      //  [ W1 ] [ P0 ] [ P1 ] [ C0 ]
      //  [ C1 ] [____] [ C1 ] [____]

      ((U32* RADRESTRICT)(crlocal))[0] = ((U32* RADRESTRICT)(crlocal))[1];
      ((U32* RADRESTRICT)(crlocal))[2] = ((U32* RADRESTRICT)(crlocal))[3];

      crlocal[2] = crlocal[8+0];
      crlocal[4+2] = crlocal[8+2];

      crlocal[3] = ((U16* RADRESTRICT)c->cdcts)[0];
      crlocal[4+3] = ((U16* RADRESTRICT)c->cdcts)[2];

      crlocal[8+0] = ((U16* RADRESTRICT)c->cdcts)[1];
      crlocal[8+2] = ((U16* RADRESTRICT)c->cdcts)[3];

      ((U32* RADRESTRICT)(cblocal))[0] = ((U32* RADRESTRICT)(cblocal))[1];
      ((U32* RADRESTRICT)(cblocal))[2] = ((U32* RADRESTRICT)(cblocal))[3];

      cblocal[2] = cblocal[8+0];
      cblocal[4+2] = cblocal[8+2];

      cblocal[3] = ((U16* RADRESTRICT)c->cdcts)[4+0];
      cblocal[4+3] = ((U16* RADRESTRICT)c->cdcts)[4+2];

      cblocal[8+0] = ((U16* RADRESTRICT)c->cdcts)[4+1];
      cblocal[8+2] = ((U16* RADRESTRICT)c->cdcts)[4+3];
    }
  
   again: 

    // ======================================================================================

#ifndef NO_FULL_BLUR
    // do all of alpha right now (no order blurring on alpha)
    if ( RAD_UNLIKELY( at != 0 ) )
    {
      dtmEnterAccumulationZone( tm_mask, tm->fullblur );
      if ( RAD_UNLIKELY( ( alinearDCsnext[-4+1] & 0x8000 ) ) ) // ( N is DCT )
      {
        if ( RAD_LIKELY( ( flags & ( Y_MID_TOP_T2 | Y_TOP ) ) == 0 ) )
        {
          GETBLUR( m0, alinearDCsnext-4+1 );
          GETBLUR( m1, alinearDCsnext-2+1 );
          if ( m0 ) full_blur( m0, alinearDCs-4, alinearDCsnext-4, alocal,     at - lpitch8, lpitch );
          if ( m1 ) full_blur( m1, alinearDCs-2, alinearDCsnext-2, alocal + 2, at - lpitch8 + 16, lpitch );
        }
      }

      if ( RAD_UNLIKELY( alocal[1] & 0x8000 ) ) //( Current is DCT )
      {
        if ( RAD_LIKELY( ( flags & Y_MID_TOP_T2 ) == 0 ) )
        {
          if ( RAD_UNLIKELY( flags & Y_TOP ) )
          {
            GETBLUR( m0, alocal+1 );
            GETBLUR( m1, alocal+3 );
            if ( m0 ) full_blur( m0, alocal, alocal, alocal + 8,             at, lpitch );
            if ( m1 ) full_blur( m1, alocal + 2, alocal + 2, alocal + 8 + 2, at + 16, lpitch );
          }
          else
          {
            GETBLUR( m0, alocal+1 );
            GETBLUR( m1, alocal+3 );
            if ( m0 ) full_blur( m0, alinearDCsnext-4, alocal, alocal + 8,         at, lpitch );
            if ( m1 ) full_blur( m1, alinearDCsnext-2, alocal + 2, alocal + 8 + 2, at + 16, lpitch );
          }

          GETBLUR( m0, alocal+1+8 );
          GETBLUR( m1, alocal+3+8 );

          if ( m0 ) full_blur( m0, alocal, alocal + 8, alocal + 16,             at + lpitch8, lpitch );
          if ( m1 ) full_blur( m1, alocal + 2, alocal + 8 + 2, alocal + 16 + 2, at + lpitch8 + 16, lpitch );
        }

        if ( RAD_LIKELY( ( flags & (Y_MID_TOP_T1|Y_NO_BOTTOM_16) ) == 0 ) )
        {
          GETBLUR( m0, alocal+1+16 );
          GETBLUR( m1, alocal+3+16 );
          if ( m0 ) full_blur( m0, alocal + 8, alocal + 16, alocal + 24,             at+ lpitch8+ lpitch8, lpitch );
          if ( m1 ) full_blur( m1, alocal + 8 + 2, alocal + 16 + 2, alocal + 24 + 2, at+ lpitch8+ lpitch8 + 16, lpitch );

          if ( RAD_UNLIKELY( flags & Y_BOTTOM ) )
          {
            GETBLUR( m0, alocal+1+24 );
            GETBLUR( m1, alocal+3+24 );
            if ( m0 ) full_blur( m0, alocal + 16, alocal + 24, alocal + 24,             at + lpitch8 + lpitch8 + lpitch8, lpitch );
            if ( m1 ) full_blur( m1, alocal + 16 + 2, alocal + 24 + 2, alocal + 24 + 2, at + lpitch8 + lpitch8 + lpitch8 + 16, lpitch );
          }
        }
      }
      dtmLeaveAccumulationZone( tm_mask, tm->fullblur );
    }

    // do all of hdr right now (no order blurring on hdr)
    if ( RAD_UNLIKELY( ht != 0 ) )
    {
      dtmEnterAccumulationZone( tm_mask, tm->fullblur );
      if ( RAD_UNLIKELY( ( hlinearDCsnext[-4+1] & 0x8000 ) ) ) // ( N is DCT )
      {
        if ( RAD_LIKELY( ( flags & ( Y_MID_TOP_T2 | Y_TOP ) ) == 0 ) )
        {
          GETBLUR( m0, hlinearDCsnext-4+1 );
          GETBLUR( m1, hlinearDCsnext-2+1 );
          if ( m0 ) full_blur( m0, hlinearDCs-4, hlinearDCsnext-4, hlocal,     ht - lpitch8, lpitch );
          if ( m1 ) full_blur( m1, hlinearDCs-2, hlinearDCsnext-2, hlocal + 2, ht - lpitch8 + 16, lpitch );
        }
      }

      if ( RAD_UNLIKELY( hlocal[1] & 0x8000 ) ) //( Current is DCT )
      {
        if ( RAD_LIKELY( ( flags & Y_MID_TOP_T2 ) == 0 ) )
        {
          if ( RAD_UNLIKELY( flags & Y_TOP ) )
          {
            GETBLUR( m0, hlocal+1 );
            GETBLUR( m1, hlocal+3 );
            if ( m0 ) full_blur( m0, hlocal, hlocal, hlocal + 8,             ht, lpitch );
            if ( m1 ) full_blur( m1, hlocal + 2, hlocal + 2, hlocal + 8 + 2, ht + 16, lpitch );
          }
          else
          {
            GETBLUR( m0, hlocal+1 );
            GETBLUR( m1, hlocal+3 );
            if ( m0 ) full_blur( m0, hlinearDCsnext-4, hlocal, hlocal + 8,         ht, lpitch );
            if ( m1 ) full_blur( m1, hlinearDCsnext-2, hlocal + 2, hlocal + 8 + 2, ht + 16, lpitch );
          }

          GETBLUR( m0, hlocal+1+8 );
          GETBLUR( m1, hlocal+3+8 );

          if ( m0 ) full_blur( m0, hlocal, hlocal + 8, hlocal + 16,             ht + lpitch8, lpitch );
          if ( m1 ) full_blur( m1, hlocal + 2, hlocal + 8 + 2, hlocal + 16 + 2, ht + lpitch8 + 16, lpitch );
        }

        if ( RAD_LIKELY( ( flags & (Y_MID_TOP_T1|Y_NO_BOTTOM_16) ) == 0 ) )
        {
          GETBLUR( m0, hlocal+1+16 );
          GETBLUR( m1, hlocal+3+16 );
          if ( m0 ) full_blur( m0, hlocal + 8, hlocal + 16, hlocal + 24,             ht+ lpitch8+ lpitch8, lpitch );
          if ( m1 ) full_blur( m1, hlocal + 8 + 2, hlocal + 16 + 2, hlocal + 24 + 2, ht+ lpitch8+ lpitch8 + 16, lpitch );

          if ( RAD_UNLIKELY( flags & Y_BOTTOM ) )
          {
            GETBLUR( m0, hlocal+1+24 );
            GETBLUR( m1, hlocal+3+24 );
            if ( m0 ) full_blur( m0, hlocal + 16, hlocal + 24, hlocal + 24,             ht + lpitch8 + lpitch8 + lpitch8, lpitch );
            if ( m1 ) full_blur( m1, hlocal + 16 + 2, hlocal + 24 + 2, hlocal + 24 + 2, ht + lpitch8 + lpitch8 + lpitch8 + 16, lpitch );
          }
        }
      }
      dtmLeaveAccumulationZone( tm_mask, tm->fullblur );
    }
#endif

    if ( RAD_UNLIKELY( flags & Y_TOP ) )
    {
      dtmEnterAccumulationZone( tm_mask, tm->fullblur );

#ifndef NO_FULL_BLUR
      GETBLUR( m0, ylocal+1 );
      GETBLUR( m1, ylocal+3 );
      GETBLUR( mcr, crlocal+1 );
      GETBLUR( mcb, cblocal+1 );

      if ( m0 ) full_blur( m0, ylocal, ylocal, ylocal + 8,         yt, lpitch );
      if ( m1 ) full_blur( m1, ylocal+2, ylocal + 2, ylocal + 8 + 2, yt + 16, lpitch );
      if ( mcr ) full_blur( mcr, crlocal, crlocal, crlocal + 4, cr, cpitch );
      if ( mcb ) full_blur( mcb, cblocal, cblocal, cblocal + 4, cb, cpitch );
#endif
      if ( ylocal[1] & 0x8000 ) //( Current is DCT )
      {
        // we leave the fullblur tm acc open
        goto cont_dct;
      }
      else
      {
        dtmLeaveAccumulationZone( tm_mask, tm->fullblur );
        goto cont_mdct;
      }
    }
    else
    {
      if ( RAD_LIKELY( ( flags & Y_MID_TOP_T2 ) == 0 ) ) 
      {
        // we're doing the last row of the block above us here - all of the fulls and UandDs
        if ( RAD_UNLIKELY( ( ylinearDCsnext[-4+1] & 0x8000 ) ) ) // ( N is DCT )
        {
          dtmEnterAccumulationZone( tm_mask, tm->fullblur );
#ifndef NO_FULL_BLUR
          GETBLUR( m0, ylinearDCsnext-4+1 );
          GETBLUR( m1, ylinearDCsnext-2+1 );
          GETBLUR( mcr, crlinearDCsnext-2+1 );
          GETBLUR( mcb, cblinearDCsnext-2+1 );

          // X0,8,16,24 Y=-8
          if ( m0 ) full_blur( m0, ylinearDCs-4, ylinearDCsnext-4, ylocal,     yt - lpitch8, lpitch );
          if ( m1 ) full_blur( m1, ylinearDCs-2, ylinearDCsnext-2, ylocal + 2, yt - lpitch8 + 16, lpitch );
          if ( mcr ) full_blur( mcr, crlinearDCs-2, crlinearDCsnext - 2, crlocal, cr - cpitch * 8, cpitch );
          if ( mcb ) full_blur( mcb, cblinearDCs-2, cblinearDCsnext - 2, cblocal, cb - cpitch * 8, cpitch );
#endif
          dtmLeaveAccumulationZone( tm_mask, tm->fullblur );
        
          if ( ( flags & BINK_DEBLOCK_UANDD_OFF ) == 0 ) 
          {
            dtmEnterAccumulationZone( tm_mask, tm->uanddblur );
            // if we aren't on the left, do X0,Y-8
            if ( RAD_LIKELY( ( ( flags & ( X_2ND_BLOCK | NO_UANDD_LL3 ) ) == 0 ) ) )
            {
              UandD_blur( get_UandD_blur( ylinearDCsnext-4, ylinearDCsnext-4+1 ), yt - lpitch8, lpitch );
              UandD_blur( get_UandD_blur( crlinearDCsnext-2, crlinearDCsnext-2+1 ), cr - cpitch*8, cpitch );
              UandD_blur( get_UandD_blur( cblinearDCsnext-2, cblinearDCsnext-2+1 ), cb - cpitch*8, cpitch );
            }

            if ( ( flags & NO_UANDD_L0 ) == 0 )
              UandD_blur( get_UandD_blur( ylinearDCsnext-4+1, ylinearDCsnext-4+2 ), yt - lpitch8 + 8, lpitch );
            
            if ( ( flags & NO_UANDD_L1 ) == 0 )
            {
              UandD_blur( get_UandD_blur( ylinearDCsnext-4+2, ylinearDCsnext-4+3 ), yt - lpitch8 + 16, lpitch );
              UandD_blur( get_UandD_blur( crlinearDCsnext-2+1, crlinearDCsnext-2+2 ), cr - cpitch*8+8, cpitch );
              UandD_blur( get_UandD_blur( cblinearDCsnext-2+1, cblinearDCsnext-2+2 ), cb - cpitch*8+8, cpitch );
            }

            if ( ( flags & NO_UANDD_L2 ) == 0 )
              UandD_blur( get_UandD_blur( ylinearDCsnext-4+3, ylinearDCsnext-4+4 ), yt - lpitch8 + 24, lpitch );

            dtmLeaveAccumulationZone( tm_mask, tm->uanddblur );
          }
        }
        else
        {
          if ( RAD_LIKELY( ( flags & BINK_DEBLOCK_UANDD_OFF ) == 0 ) ) 
          {
            dtmEnterAccumulationZone( tm_mask, tm->uanddblur );
            // if we aren't on the left, do X0,Y-8
            if ( RAD_LIKELY( ( ( flags & ( X_2ND_BLOCK | NO_UANDD_LL3 ) ) == 0 ) ) )
            {
              if ( RAD_UNLIKELY( ( ylinearDCsnext[-4] & 0x8000 ) ) ) // ( NW is DCT )
              {
                // N is MDCT, NW is DCT, blur2lines - todo - can this be constant?
                UandD_blur( get_UandD_blur( ylinearDCsnext-4, ylinearDCsnext-4+1 ), yt - lpitch8, lpitch );
                UandD_blur( get_UandD_blur( crlinearDCsnext-2, crlinearDCsnext-2+1 ), cr - cpitch*8, cpitch );
                UandD_blur( get_UandD_blur( cblinearDCsnext-2, cblinearDCsnext-2+1 ), cb - cpitch*8, cpitch );
              }
              else
              {
                // N is MDCT, NW is MDCT, compare mo-vecs
                if ( different_coords( &pcoords[-8-8+6], &pcoords[-8+4] ) )
                {
                  UandD_blur( DIFFERENT_MO_COORDS_BLUR1_UANDD, yt - lpitch8, lpitch );
                  UandD_blur( DIFFERENT_MO_COORDS_BLUR1_UANDD, cr - cpitch*8, cpitch );
                  UandD_blur( DIFFERENT_MO_COORDS_BLUR1_UANDD, cb - cpitch*8, cpitch );
                }
              }
            }

            if ( ( flags & NO_UANDD_L1 ) == 0 )
            {
              if ( different_coords( &pcoords[-8+4], &pcoords[-8+6] ) )
              {
                UandD_blur( DIFFERENT_MO_COORDS_BLUR1_UANDD, yt - lpitch8 + 16, lpitch );
                UandD_blur( DIFFERENT_MO_COORDS_BLUR1_UANDD, cr - cpitch*8+8, cpitch );
                UandD_blur( DIFFERENT_MO_COORDS_BLUR1_UANDD, cb - cpitch*8+8, cpitch );
              }
            }

            dtmLeaveAccumulationZone( tm_mask, tm->uanddblur );
          }
        }
      }
    }
    // at this point, we've done X0,8,16,24,Y-8 full blurs, and X=0,8,16,24,Y=8 UandD

    if ( RAD_UNLIKELY( ylocal[1] & 0x8000 ) ) //( Current is DCT )
    {
      // now we do the remaining full blurs X=0,8,16,24 Y=0,8,16
      if ( RAD_LIKELY( ( flags & Y_MID_TOP_T2 ) == 0 ) )
      {
        dtmEnterAccumulationZone( tm_mask, tm->fullblur );
#ifndef NO_FULL_BLUR
        GETBLUR( m0, ylocal+1 );
        GETBLUR( m1, ylocal+3 );
        GETBLUR( mcr, crlocal+1 );
        GETBLUR( mcb, cblocal+1 );

        if ( m0 ) full_blur( m0, ylinearDCsnext-4, ylocal, ylocal + 8,         yt, lpitch );
        if ( m1 ) full_blur( m1, ylinearDCsnext-2, ylocal + 2, ylocal + 8 + 2, yt + 16, lpitch );
        if ( mcr ) full_blur( mcr, crlinearDCsnext-2, crlocal, crlocal + 4, cr, cpitch );
        if ( mcb ) full_blur( mcb, cblinearDCsnext-2, cblocal, cblocal + 4, cb, cpitch );
#endif

       cont_dct: 
       
#ifndef NO_FULL_BLUR
        GETBLUR( m0, ylocal+1+8 );
        GETBLUR( m1, ylocal+3+8 );

        if ( m0 ) full_blur( m0, ylocal, ylocal + 8, ylocal + 16,             yt + lpitch8, lpitch );
        if ( m1 ) full_blur( m1, ylocal + 2, ylocal + 8 + 2, ylocal + 16 + 2, yt + lpitch8 + 16, lpitch );
#endif
        dtmLeaveAccumulationZone( tm_mask, tm->fullblur );

        if ( RAD_LIKELY( ( flags & BINK_DEBLOCK_UANDD_OFF ) == 0 ) )
        {
          dtmEnterAccumulationZone( tm_mask, tm->uanddblur );
          // now let's do the left side UandB blurs X=0, y=0,8,16
          if ( RAD_LIKELY( ( flags & ( X_2ND_BLOCK | NO_UANDD_LL3 ) ) == 0 ) )
          {
            UandD_blur( get_UandD_blur( ylocal+0, ylocal+1 ), yt, lpitch );
            UandD_blur( get_UandD_blur( crlocal+0, crlocal+1 ), cr, cpitch );
            UandD_blur( get_UandD_blur( cblocal+0, cblocal+1 ), cb, cpitch );
            UandD_blur( get_UandD_blur( ylocal+8+0, ylocal+8+1 ), yt + lpitch8, lpitch );
          }

          // this block does X=1,2,3, Y=0 (the top 8x8 row)
          //  and then X=1,2,3, Y=1 (the rest of our 2nd row)
          //  and then X=1,2,3, Y=2 (the rest of our 3rd row)
          //  and then, if we are at the end of the screen, X=1,2,3, Y=3 (the rest of our 3rd line)
          //            if we are not at the end of the screen, then we do this last line when
          //            we end up doing the 32x32 directly below us

          if ( ( flags & NO_UANDD_L0 ) == 0 )
          {
            UandD_blur( get_UandD_blur( ylocal+1, ylocal+2 ), yt + 8, lpitch );
            UandD_blur( get_UandD_blur( ylocal+8+1, ylocal+8+2 ), yt + lpitch8 + 8, lpitch );
          }

          if ( ( flags & NO_UANDD_L1 ) == 0 )
          {
            UandD_blur( get_UandD_blur( ylocal+2, ylocal+3 ), yt + 16, lpitch );
            UandD_blur( get_UandD_blur( ylocal+8+2, ylocal+8+3 ), yt + lpitch8  + 16, lpitch );
            UandD_blur( get_UandD_blur( crlocal+1, crlocal+2 ), cr+8, cpitch );
            UandD_blur( get_UandD_blur( cblocal+1, cblocal+2 ), cb+8, cpitch );
          }

          if ( ( flags & NO_UANDD_L2 ) == 0 )
          {
            UandD_blur( get_UandD_blur( ylocal+3, ylocal+4 ), yt + 24, lpitch );
            UandD_blur( get_UandD_blur( ylocal+8+3, ylocal+8+4 ), yt + lpitch8  + 24, lpitch );
          }

          dtmLeaveAccumulationZone( tm_mask, tm->uanddblur );
        }
      }

      if ( RAD_LIKELY( ( flags & (Y_MID_TOP_T1|Y_NO_BOTTOM_16) ) == 0 ) )
      {
        dtmEnterAccumulationZone( tm_mask, tm->fullblur );

#ifndef NO_FULL_BLUR
        GETBLUR( m0, ylocal+1+16 );
        GETBLUR( m1, ylocal+3+16 );

        if ( m0 ) full_blur( m0, ylocal + 8, ylocal + 16, ylocal + 24,             yt+ lpitch8+ lpitch8, lpitch );
        if ( m1 ) full_blur( m1, ylocal + 8 + 2, ylocal + 16 + 2, ylocal + 24 + 2, yt+ lpitch8+ lpitch8 + 16, lpitch );
#endif
        dtmLeaveAccumulationZone( tm_mask, tm->fullblur );

        if ( RAD_LIKELY( ( flags & BINK_DEBLOCK_UANDD_OFF ) == 0 ) )
        {
          dtmEnterAccumulationZone( tm_mask, tm->uanddblur );
          if ( RAD_LIKELY( ( flags & ( X_2ND_BLOCK | NO_UANDD_LL3 ) ) == 0 ) )
            UandD_blur( get_UandD_blur( ylocal+16+0, ylocal+16+1 ), yt + lpitch8 + lpitch8, lpitch );

          if ( ( flags & NO_UANDD_L0 ) == 0 )
            UandD_blur( get_UandD_blur( ylocal+16+1, ylocal+16+2 ), yt + lpitch8  + lpitch8 + 8, lpitch );

          if ( ( flags & NO_UANDD_L1 ) == 0 )
            UandD_blur( get_UandD_blur( ylocal+16+2, ylocal+16+3 ), yt + lpitch8  + lpitch8 + 16, lpitch );

          if ( ( flags & NO_UANDD_L2 ) == 0 )
            UandD_blur( get_UandD_blur( ylocal+16+3, ylocal+16+4 ), yt + lpitch8  + lpitch8 + 24, lpitch );
          dtmLeaveAccumulationZone( tm_mask, tm->uanddblur );
        }

        if ( RAD_UNLIKELY( flags & Y_BOTTOM ) )
        {
          dtmEnterAccumulationZone( tm_mask, tm->fullblur );
#ifndef NO_FULL_BLUR
          GETBLUR( m0, ylocal+1+24 );
          GETBLUR( m1, ylocal+3+24 );
          GETBLUR( mcr, crlocal+1+4 );
          GETBLUR( mcb, cblocal+1+4 );

          if ( m0 ) full_blur( m0, ylocal + 16, ylocal + 24, ylocal + 24,             yt + lpitch8 + lpitch8 + lpitch8, lpitch );
          if ( m1 ) full_blur( m1, ylocal + 16 + 2, ylocal + 24 + 2, ylocal + 24 + 2, yt + lpitch8 + lpitch8 + lpitch8 + 16, lpitch );
          if ( mcr ) full_blur( mcr, crlocal, crlocal+4, crlocal+4, cr + cpitch*8, cpitch );
          if ( mcb ) full_blur( mcb, cblocal, cblocal+4, cblocal+4, cb + cpitch*8, cpitch );
#endif
          dtmLeaveAccumulationZone( tm_mask, tm->fullblur );
     
          if ( RAD_LIKELY( ( flags & BINK_DEBLOCK_UANDD_OFF ) == 0 ) )
          {
            dtmEnterAccumulationZone( tm_mask, tm->uanddblur );
            if ( RAD_LIKELY( ( ( flags & ( X_2ND_BLOCK | NO_UANDD_LL3 ) ) == 0 ) ) ) 
            {
              UandD_blur( get_UandD_blur( ylocal+24+0, ylocal+24+1 ), yt + lpitch8 + lpitch8 +lpitch8, lpitch );
              UandD_blur( get_UandD_blur( crlocal+4, crlocal+4+1 ), cr+cpitch*8, cpitch );
              UandD_blur( get_UandD_blur( cblocal+4, cblocal+4+1 ), cb+cpitch*8, cpitch );
            }
            if ( ( flags & NO_UANDD_L0 ) == 0 )
              UandD_blur( get_UandD_blur( ylocal+24+1, ylocal+24+2 ), yt + lpitch8 + lpitch8 + lpitch8 + 8, lpitch );

            if ( ( flags & NO_UANDD_L1 ) == 0 )
            {
              UandD_blur( get_UandD_blur( ylocal+24+2, ylocal+24+3 ), yt + lpitch8 + lpitch8 + lpitch8 + 16, lpitch );
              UandD_blur( get_UandD_blur( crlocal+4+1, crlocal+4+2 ), cr + cpitch*8 + 8, cpitch );
              UandD_blur( get_UandD_blur( cblocal+4+1, cblocal+4+2 ), cb + cpitch*8 + 8, cpitch );
            }

            if ( ( flags & NO_UANDD_L2 ) == 0 )
              UandD_blur( get_UandD_blur( ylocal+24+3, ylocal+24+4 ), yt + lpitch8 + lpitch8 + lpitch8 + 24, lpitch );

            dtmLeaveAccumulationZone( tm_mask, tm->uanddblur );
          }
        }
      }

      // now start interleaving the LtoRs with the UandDs
      if ( RAD_LIKELY( ( flags & BINK_DEBLOCK_LTOR_OFF ) == 0 ) )
      {
        dtmEnterAccumulationZone( tm_mask, tm->ltorblur );
        yt = yt - lpitch - lpitch;
        cr = cr - cpitch - cpitch;
        cb = cb - cpitch - cpitch;
        
        m0 = mcr = mcb = 0;

        // do left LtoR W -16, that is X-16, Y-8, Y=0, Y=8, Y=16 
        do_LtoR_Xm16( yt, cr, cb, local_row->next_LtoRs, lpitch, cpitch );

        if ( RAD_LIKELY( ( flags & ( Y_TOP | Y_MID_TOP_T2 ) ) == 0 ) )
        {
          if ( RAD_UNLIKELY( ( ylinearDCsnext[-4+1] & 0x8000 ) ) ) // ( N is DCT )
          {
            // calc N X=0,8, Y=-8, store X=16,24, Y=-8
            if ( ( flags & NO_LTOR_A2 ) == 0 )
            {
              mublock_LtoR( get_LtoR_blur( ylinearDCs-4+1, ylinearDCsnext-4+1 ), yt - lpitch8, lpitch );
              m0 = get_LtoR_blur( ylinearDCs-4+3, ylinearDCsnext-4+3 );
            }

            if ( ( flags & NO_LTOR_A1 ) == 0 )
            {
              mcr = get_LtoR_blur( crlinearDCs-2+1, crlinearDCsnext-2+1 );
              mcb = get_LtoR_blur( cblinearDCs-2+1, cblinearDCsnext-2+1 );
            }

            // N and C are DCT
            if ( ( flags & NO_LTOR_A3 ) == 0 )
            {
              mublock_LtoR( get_LtoR_blur( ylinearDCsnext-4+1, ylocal+1 ), yt, lpitch );
              m0 |= (get_LtoR_blur( ylinearDCsnext-4+3, ylocal+3 ))<<8;
            }
          }
          else
          {
            // N is MDCT and C is DCT - todo, can this be constant?
            if ( ( flags & NO_LTOR_A3 ) == 0 )
            {
              mublock_LtoR( get_LtoR_blur( ylinearDCsnext-4+1, ylocal+1 ), yt, lpitch );
              m0 = (get_LtoR_blur( ylinearDCsnext-4+3, ylocal+3 ))<<8;
            }

            if ( ( flags & NO_LTOR_A1 ) == 0 )
            {
              if ( different_coords( &pcoords[-8+4], &pcoords[-8+0] ) )
              {
                mcr = (DIFFERENT_MO_COORDS_BLUR1_LTOR); 
                mcb = (DIFFERENT_MO_COORDS_BLUR1_LTOR);
              }
              if ( different_coords( &pcoords[-8+6], &pcoords[-8+2] ) )
              {
                mcr |= (DIFFERENT_MO_COORDS_BLUR1_LTOR<<4);
                mcb |= (DIFFERENT_MO_COORDS_BLUR1_LTOR<<4);
              }
        
            }
          }

          if ( RAD_UNLIKELY( ( flags & ( Y_MID_TOP_T2_NEXT | NO_LTOR_A1 ) ) == Y_MID_TOP_T2_NEXT ) ) //one after the mid point (don't do the over the line)
          {
            // store for later deblocking (after both threads are done)
            *seam++ = (U8) mcr;
            *seam++ = (U8) mcb;
            mcr = 0;
            mcb = 0;
          }

          if ( ( flags & NO_LTOR_A3 ) == 0 )
          {
            mcr |= ( get_LtoR_blur( crlinearDCsnext-2+1, crlocal+1 ) ) << 8;
            mcb |= ( get_LtoR_blur( cblinearDCsnext-2+1, cblocal+1 ) ) << 8;
          }
        }  

        if ( RAD_LIKELY( ( flags & ( Y_MID_TOP_T2 | NO_LTOR_C0 ) ) == 0 ) )
        {
          mublock_LtoR( get_LtoR_blur( ylocal+1, ylocal+8+1 ), yt + lpitch8, lpitch );
          m0 |= ( get_LtoR_blur( ylocal+3, ylocal+8+3 ) ) << 16;
        }

        if ( RAD_LIKELY( ( flags & (Y_MID_TOP_T1|Y_NO_BOTTOM_16) ) == 0 ) )
        {
          if ( RAD_UNLIKELY( ( flags & NO_LTOR_C1 ) == 0 ) )
          {
            if ( RAD_UNLIKELY( ( flags & Y_MID_TOP_T2 ) == 0 ) )
            {
              mublock_LtoR( get_LtoR_blur( ylocal+8+1, ylocal+16+1 ), yt+ lpitch8+ lpitch8, lpitch );
              m0 |= ( get_LtoR_blur( ylocal+8+3, ylocal+16+3 ) ) << 24;
            }
            else
            {
              // store for later deblocking (after both threads are done)
              *seam++ = get_LtoR_blur( ylocal+8+1, ylocal+16+1 );
              *seam++ = get_LtoR_blur( ylocal+8+3, ylocal+16+3 );
            }
          }

          if ( RAD_UNLIKELY( flags & Y_BOTTOM ) )
          {
            if ( RAD_UNLIKELY( ( flags & NO_LTOR_C2 ) == 0 ) )
            {
              mublock_LtoR( get_LtoR_blur( ylocal+16+1, ylocal+24+1 ), yt + lpitch8+ lpitch8+ lpitch8, lpitch );
              mcr |= ( get_LtoR_blur( ylocal+16+3, ylocal+24+3 ) ) << 24; //stash this one in the mcr
            }

            if ( RAD_UNLIKELY( ( flags & NO_LTOR_C1 ) == 0 ) )
            {
              mcr |= ( get_LtoR_blur( crlocal+1, crlocal+4+1 ) ) << 16;
              mcb |= ( get_LtoR_blur( cblocal+1, cblocal+4+1 ) ) << 16;
            }
          }
        }

        local_row->next_LtoRs[0] = m0;
        local_row->next_LtoRs[1] = mcr;  
        local_row->next_LtoRs[2] = mcb;  

        yt = yt + lpitch + lpitch;
        cr = cr + cpitch + cpitch;
        cb = cb + cpitch + cpitch;
        dtmLeaveAccumulationZone( tm_mask, tm->ltorblur );
      }
    }
    else
    {
     cont_mdct: 

      if ( RAD_LIKELY( ( flags & BINK_DEBLOCK_UANDD_OFF ) == 0 ) )
      {
        dtmEnterAccumulationZone( tm_mask, tm->uanddblur );
        if ( RAD_UNLIKELY( ylocal[0] & 0x8000 ) ) // ( W is DCT ) 
        {
          // same as above (when C was DCT)
          // now let's UandD blur X=0 Y=0,8, 16
          if ( RAD_LIKELY( ( flags & Y_MID_TOP_T2 ) == 0 ) )
          { 
            if ( RAD_LIKELY( ( flags & ( X_2ND_BLOCK | NO_UANDD_LL3 ) ) == 0 ) )
            {
              UandD_blur( get_UandD_blur( ylocal+0, ylocal+1 ), yt, lpitch );
              UandD_blur( get_UandD_blur( crlocal+0, crlocal+1 ), cr, cpitch );
              UandD_blur( get_UandD_blur( cblocal+0, cblocal+1 ), cb, cpitch );
              UandD_blur( get_UandD_blur( ylocal+8+0, ylocal+8+1 ), yt + lpitch8, lpitch );
            }

            // do current top (duplicated below)
            if ( ( flags & NO_UANDD_L1 ) == 0 )
            {
              if ( different_coords( &ccoords[-8+0], &ccoords[-8+2] ) )
              {
                UandD_blur( DIFFERENT_MO_COORDS_BLUR1_UANDD, yt + 16, lpitch );
                UandD_blur( DIFFERENT_MO_COORDS_BLUR1_UANDD, yt + 16 + lpitch8, lpitch );

                UandD_blur( DIFFERENT_MO_COORDS_BLUR1_UANDD, cr+8, cpitch );
                UandD_blur( DIFFERENT_MO_COORDS_BLUR1_UANDD, cb+8, cpitch );
              }
            }

          }
          if ( RAD_LIKELY( ( flags & (Y_MID_TOP_T1|Y_NO_BOTTOM_16) ) == 0 ) )
          {
            if ( RAD_LIKELY( ( flags & ( X_2ND_BLOCK | NO_UANDD_LL3 ) ) == 0 ) )
            {
              UandD_blur( get_UandD_blur( ylocal+16+0, ylocal+16+1 ), yt + lpitch8 + lpitch8, lpitch );
              if ( RAD_UNLIKELY( flags & Y_BOTTOM ) )
              {
                UandD_blur( get_UandD_blur( ylocal+24+0, ylocal+24+1 ), yt + lpitch8 + lpitch8 +lpitch8, lpitch );
                UandD_blur( get_UandD_diff_blur( crlocal+4, crlocal+4+1 ), cr + cpitch*8, cpitch );
                UandD_blur( get_UandD_diff_blur( cblocal+4, cblocal+4+1 ), cb + cpitch*8, cpitch );
              }
            }
           
            // go to current block (no need to duplicate, because we jump right there
            goto do_bottom_of_current;
          }
        }
        else
        {
          if ( RAD_LIKELY( ( flags & Y_MID_TOP_T2 ) == 0 ) )
          { 
            if ( RAD_LIKELY( ( flags & ( X_2ND_BLOCK | NO_UANDD_LL3 ) ) == 0 ) )
            { 
              if ( different_coords( &ccoords[-8-8+2], &ccoords[-8+0] ) )
              {
                UandD_blur( DIFFERENT_MO_COORDS_BLUR1_UANDD, yt, lpitch );
                UandD_blur( DIFFERENT_MO_COORDS_BLUR1_UANDD, yt + lpitch8, lpitch );
                UandD_blur( DIFFERENT_MO_COORDS_BLUR1_UANDD, cr, cpitch );
                UandD_blur( DIFFERENT_MO_COORDS_BLUR1_UANDD, cb, cpitch );
              }
            }

            // do current top (duplicated above)
            if ( ( flags & NO_UANDD_L1 ) == 0 )
            {
              if ( different_coords( &ccoords[-8+0], &ccoords[-8+2] ) )
              {
                UandD_blur( DIFFERENT_MO_COORDS_BLUR1_UANDD, yt + 16, lpitch );
                UandD_blur( DIFFERENT_MO_COORDS_BLUR1_UANDD, yt + 16 + lpitch8, lpitch );

                UandD_blur( DIFFERENT_MO_COORDS_BLUR1_UANDD, cr+8, cpitch );
                UandD_blur( DIFFERENT_MO_COORDS_BLUR1_UANDD, cb+8, cpitch );
              }
            }
          }

          if ( RAD_LIKELY( ( flags & (Y_MID_TOP_T1|Y_NO_BOTTOM_16) ) == 0 ) )
          {
            if ( RAD_LIKELY( ( flags & ( X_2ND_BLOCK | NO_UANDD_LL3 ) ) == 0 ) )
            { 
              if ( different_coords( &ccoords[-8-8+6], &ccoords[-8+4] ) )
              {
                UandD_blur( DIFFERENT_MO_COORDS_BLUR1_UANDD, yt + lpitch8 + lpitch8, lpitch );
                if ( RAD_UNLIKELY( flags & Y_BOTTOM ) )
                {
                  UandD_blur( DIFFERENT_MO_COORDS_BLUR1_UANDD, yt + lpitch8 + lpitch8 + lpitch8, lpitch );
                  UandD_blur( DIFFERENT_MO_COORDS_BLUR1_UANDD, cr+cpitch*8, cpitch );
                  UandD_blur( DIFFERENT_MO_COORDS_BLUR1_UANDD, cb+cpitch*8, cpitch );
                }
              }
            }

           do_bottom_of_current:
            if ( ( flags & NO_UANDD_L1 ) == 0 )
            {
              if ( different_coords( &ccoords[-8+4], &ccoords[-8+6] ) )
              {
                UandD_blur( DIFFERENT_MO_COORDS_BLUR1_UANDD, yt + 16 + lpitch8 + lpitch8, lpitch );
                if ( RAD_UNLIKELY( flags & Y_BOTTOM ) )
                {
                  UandD_blur( DIFFERENT_MO_COORDS_BLUR1_UANDD, yt + 16 + lpitch8 + lpitch8 + lpitch8, lpitch );
                  UandD_blur( DIFFERENT_MO_COORDS_BLUR1_UANDD, cr + cpitch*8 + 8, cpitch );
                  UandD_blur( DIFFERENT_MO_COORDS_BLUR1_UANDD, cb + cpitch*8 + 8, cpitch );
                }
              }
            }
          }
        }
        dtmLeaveAccumulationZone( tm_mask, tm->uanddblur );
      }

      if ( RAD_LIKELY( ( flags & BINK_DEBLOCK_LTOR_OFF ) == 0 ) )
      {
        dtmEnterAccumulationZone( tm_mask, tm->ltorblur );
        yt = yt - lpitch - lpitch;
        cr = cr - cpitch - cpitch;
        cb = cb - cpitch - cpitch;

        m0 = mcr = mcb = 0;

        // do left LtoR previous -16, that is X-16, Y-8, Y=0, Y=8, Y=16 
        do_LtoR_Xm16( yt, cr, cb, local_row->next_LtoRs, lpitch, cpitch );

        if ( RAD_LIKELY( ( flags & ( Y_TOP | Y_MID_TOP_T2 ) ) == 0 ) )
        {
          if ( RAD_UNLIKELY( ( ylinearDCsnext[-4+1] & 0x8000 ) ) ) // ( N is DCT )
          {
            if ( ( flags & NO_LTOR_A2 ) == 0 )
            {
              mublock_LtoR( get_LtoR_blur( ylinearDCs-4+1, ylinearDCsnext-4+1 ), yt - lpitch8, lpitch );
              m0 = get_LtoR_blur( ylinearDCs-4+3, ylinearDCsnext-4+3 );
            }

            if ( ( flags & NO_LTOR_A1 ) == 0 )
            {
              mcr = get_LtoR_blur( crlinearDCs-2+1, crlinearDCsnext-2+1 );
              mcb = get_LtoR_blur( cblinearDCs-2+1, cblinearDCsnext-2+1 );
            }

            // N is DCT, and C is MDCT - todo, constant?
            if ( ( flags & NO_LTOR_A3 ) == 0 )
            {
              mublock_LtoR( get_LtoR_blur( ylinearDCsnext-4+1, ylocal+1 ), yt, lpitch );
              m0 |= (get_LtoR_blur( ylinearDCsnext-4+3, ylocal+3 ))<<8;

              mcr |= ( get_LtoR_blur( crlinearDCsnext-2+1, crlocal+1 ) ) << 8;
              mcb |= ( get_LtoR_blur( cblinearDCsnext-2+1, cblocal+1 ) ) << 8;
            }
          }
          else
          {
            if ( ( flags & NO_LTOR_A3 ) == 0 )
            {
              if ( different_coords( &ccoords[-8+0], &pcoords[-8+4] ) )
              {
                mublock_LtoR( DIFFERENT_MO_COORDS_BLUR_LTOR, yt, lpitch );
                mcr |= (DIFFERENT_MO_COORDS_BLUR1_LTOR<<8);
                mcb |= (DIFFERENT_MO_COORDS_BLUR1_LTOR<<8);
              }
             
              if ( different_coords( &ccoords[-8+2], &pcoords[-8+6] ) )
              {
                m0 |= (DIFFERENT_MO_COORDS_BLUR_LTOR<<8);
                mcr |= (DIFFERENT_MO_COORDS_BLUR1_LTOR<<(4+8));
                mcb |= (DIFFERENT_MO_COORDS_BLUR1_LTOR<<(4+8));
              }
            }

            if ( ( flags & NO_LTOR_A1 ) == 0 )
            {
              if ( different_coords( &pcoords[-8+4], &pcoords[-8+0] ) )
              {
                mcr |= (DIFFERENT_MO_COORDS_BLUR1_LTOR);
                mcb |= (DIFFERENT_MO_COORDS_BLUR1_LTOR);
              }
              if ( different_coords( &pcoords[-8+6], &pcoords[-8+2] ) )
              {
                mcr |= (DIFFERENT_MO_COORDS_BLUR1_LTOR<<4);
                mcb |= (DIFFERENT_MO_COORDS_BLUR1_LTOR<<4);
              }
          
            }
          }

          if ( RAD_UNLIKELY( ( flags & ( Y_MID_TOP_T2_NEXT | NO_LTOR_A1 ) ) == Y_MID_TOP_T2_NEXT ) ) //one after the mid point (don't do the over the line)
          {
            // store for later deblocking (after both threads are done)
            *seam++ = (U8) mcr;
            *seam++ = (U8) mcb;
            mcr &= 0xff00;
            mcb &= 0xff00;
          }

        }

        if ( RAD_LIKELY( ( flags & (Y_MID_TOP_T1|Y_NO_BOTTOM_16) ) == 0 ) )
        {
          if ( RAD_UNLIKELY( ( flags & NO_LTOR_C1 ) == 0 ) )
          {
            if ( RAD_LIKELY( ( flags & Y_MID_TOP_T2 ) == 0 ) )
            {
              if ( different_coords( &ccoords[-8+0], &ccoords[-8+4] ) )
                mublock_LtoR( DIFFERENT_MO_COORDS_BLUR_LTOR, yt + lpitch8 + lpitch8, lpitch );

              if ( different_coords( &ccoords[-8+2], &ccoords[-8+6] ) )
                m0 |= (DIFFERENT_MO_COORDS_BLUR_LTOR<<24);
            }
            else
            {
              // store for later deblocking (after both threads are done)
              if ( different_coords( &ccoords[-8+0], &ccoords[-8+4] ) )
                *seam++ = DIFFERENT_MO_COORDS_BLUR_LTOR;
              else
                *seam++ = 0;
              if ( different_coords( &ccoords[-8+2], &ccoords[-8+6] ) )
                *seam++ = DIFFERENT_MO_COORDS_BLUR_LTOR;
              else
                *seam++ = 0;
            }
          }

          if ( RAD_UNLIKELY( ( flags & ( Y_BOTTOM | NO_LTOR_C1 ) ) == Y_BOTTOM ) )
          {
            if ( different_coords( &ccoords[-8+4], &ccoords[-8+0] ) )
            {
              mcr |= (DIFFERENT_MO_COORDS_BLUR1_LTOR<<16);
              mcb |= (DIFFERENT_MO_COORDS_BLUR1_LTOR<<16);
            }
            if ( different_coords( &ccoords[-8+6], &ccoords[-8+2] ) )
            {
              mcr |= (DIFFERENT_MO_COORDS_BLUR1_LTOR<<(4+16));
              mcb |= (DIFFERENT_MO_COORDS_BLUR1_LTOR<<(4+16));
            }
          }
        }

        local_row->next_LtoRs[0] = m0;
        local_row->next_LtoRs[1] = mcr;  
        local_row->next_LtoRs[2] = mcb;  

        yt = yt + lpitch + lpitch;
        cr = cr + cpitch + cpitch;
        cb = cb + cpitch + cpitch;
        dtmLeaveAccumulationZone( tm_mask, tm->ltorblur );
      }
    }


    // ======================================================================================
    // end of blurring - rotate all our data

    if ( RAD_UNLIKELY( at != 0 ) )
    {
      *((U64* RADRESTRICT)(alinearDCs-4))     = *((U64* RADRESTRICT)(alocal+16));
      *((U64* RADRESTRICT)(alinearDCsnext-4)) = *((U64* RADRESTRICT)(alocal+24));
    }

    if ( RAD_UNLIKELY( ht != 0 ) )
    {
      *((U64* RADRESTRICT)(hlinearDCs-4))     = *((U64* RADRESTRICT)(hlocal+16));
      *((U64* RADRESTRICT)(hlinearDCsnext-4)) = *((U64* RADRESTRICT)(hlocal+24));
    }

    *((U64* RADRESTRICT)(ylinearDCs-4))     = *((U64* RADRESTRICT)(ylocal+16));
    *((U64* RADRESTRICT)(ylinearDCsnext-4)) = *((U64* RADRESTRICT)(ylocal+24));

    *((U32* RADRESTRICT)(crlinearDCs-2))     = *((U32* RADRESTRICT)(crlocal+0));
    *((U32* RADRESTRICT)(crlinearDCsnext-2)) = *((U32* RADRESTRICT)(crlocal+4));

    *((U32* RADRESTRICT)(cblinearDCs-2))     = *((U32* RADRESTRICT)(cblocal+0));
    *((U32* RADRESTRICT)(cblinearDCsnext-2)) = *((U32* RADRESTRICT)(cblocal+4));
  }
  
  // ======================================================================================  
  // finish up this block

  if ( flags & X_RIGHT ) //(after right edge)
  {
    if ( flags & X_LAST )
    {
      if ( at )
      {
        alinearDCs[0]      = alocal[20];
        alinearDCsnext[0]  = alocal[28];
      }

      if ( ht )
      {
        hlinearDCs[0]      = hlocal[20];
        hlinearDCsnext[0]  = hlocal[28];
      }

      ylinearDCs[0]      = ylocal[20];
      ylinearDCsnext[0]  = ylocal[28];
      crlinearDCs[0]     = crlocal[2];
      crlinearDCsnext[0] = crlocal[2+4];
      cblinearDCs[0]     = cblocal[2];
      cblinearDCsnext[0] = cblocal[2+4];

      if ( ( flags & BINK_DEBLOCK_LTOR_OFF ) == 0 ) 
      {
        m0 = local_row->next_LtoRs[0];
        mcr = local_row->next_LtoRs[1];
        mcb = local_row->next_LtoRs[2];

        yt = yt - lpitch - lpitch; // back up two lines
        if ( m0 )
        {
          mublock_LtoR( m0 & 255, yt - lpitch8 + 16, lpitch );
          mublock_LtoR( ( m0 >> 8 ) & 255, yt + 16, lpitch );
          mublock_LtoR( ( m0 >> 16 ) & 255, yt + lpitch8 + 16, lpitch );
          mublock_LtoR( m0 >> 24, yt + lpitch8 + lpitch8 + 16, lpitch );
        }
    
        if ( mcr )
        {
          mublock_LtoR( mcr >> 24, yt + lpitch8 + lpitch8 + lpitch8 + 16, lpitch ); // only on final lines

          cr = cr - cpitch - cpitch; // back up two lines
          mublock_LtoR( mcr & 255, cr - cpitch*8, cpitch );
          mublock_LtoR( ( mcr >> 8 ) & 255, cr, cpitch );
          mublock_LtoR( ( mcr >> 16 ) & 255, cr + cpitch*8, cpitch );
        }

        if ( mcb )
        {
          cb = cb - cpitch - cpitch; // back up two lines
          mublock_LtoR( mcb & 255, cb - cpitch*8, cpitch );
          mublock_LtoR( ( mcb >> 8 ) & 255, cb, cpitch );
          mublock_LtoR( ( mcb >> 16 ) & 255, cb + cpitch*8, cpitch );
        }
      }
    }
    else
    {
      flags |= X_LAST;
      yt = yt + 32;
      cr = cr + 16;
      cb = cb + 16;
      ylinearDCs += 4;
      ylinearDCsnext += 4;
      crlinearDCs += 2;
      crlinearDCsnext += 2;
      cblinearDCs += 2;
      cblinearDCsnext += 2;
      ccoords += 8;
      pcoords += 8;
      if ( flags & X_LEFT ) flags |= X_2ND_BLOCK; // if a single 32 wide movie, mark the left edge

      // rotate the column flags
      flags = (flags & ~NO_UANDD_MASKS)|((flags&(NO_UANDD_L3|NO_UANDD_C0|NO_UANDD_C1|NO_UANDD_C2))>>4);

      if ( at )
      {
        at = at + 32;
        alinearDCs += 4;
        alinearDCsnext += 4;
        ((U64* RADRESTRICT)(alocal))[0] = ((U64* RADRESTRICT)(alocal))[1];
        ((U64* RADRESTRICT)(alocal))[2] = ((U64* RADRESTRICT)(alocal))[3];
        ((U64* RADRESTRICT)(alocal))[4] = ((U64* RADRESTRICT)(alocal))[5];
        ((U64* RADRESTRICT)(alocal))[6] = ((U64* RADRESTRICT)(alocal))[7];

        // subtlety: the odd address in alocal are zero, so this terminates 
        //   the right edge in addition to copying the final DC
        ((U32* RADRESTRICT)(alocal))[2]    = ((U32* RADRESTRICT)&alocal[32+0])[0];
        ((U32* RADRESTRICT)(alocal))[2+4]  = ((U32* RADRESTRICT)&alocal[32+2])[0];
        ((U32* RADRESTRICT)(alocal))[2+8]  = ((U32* RADRESTRICT)&alocal[32+4])[0];
        ((U32* RADRESTRICT)(alocal))[2+12] = ((U32* RADRESTRICT)&alocal[32+6])[0];
      }

      if ( ht )
      {
        ht = ht + 32;
        hlinearDCs += 4;
        hlinearDCsnext += 4;
        ((U64* RADRESTRICT)(hlocal))[0] = ((U64* RADRESTRICT)(hlocal))[1];
        ((U64* RADRESTRICT)(hlocal))[2] = ((U64* RADRESTRICT)(hlocal))[3];
        ((U64* RADRESTRICT)(hlocal))[4] = ((U64* RADRESTRICT)(hlocal))[5];
        ((U64* RADRESTRICT)(hlocal))[6] = ((U64* RADRESTRICT)(hlocal))[7];

        // subtlety: the odd address in hlocal are zero, so this terminates 
        //   the right edge in addition to copying the final DC
        ((U32* RADRESTRICT)(hlocal))[2]    = ((U32* RADRESTRICT)&hlocal[32+0])[0];
        ((U32* RADRESTRICT)(hlocal))[2+4]  = ((U32* RADRESTRICT)&hlocal[32+2])[0];
        ((U32* RADRESTRICT)(hlocal))[2+8]  = ((U32* RADRESTRICT)&hlocal[32+4])[0];
        ((U32* RADRESTRICT)(hlocal))[2+12] = ((U32* RADRESTRICT)&hlocal[32+6])[0];
      }

      ((U64* RADRESTRICT)(ylocal))[0] = ((U64* RADRESTRICT)(ylocal))[1];
      ((U64* RADRESTRICT)(ylocal))[2] = ((U64* RADRESTRICT)(ylocal))[3];
      ((U64* RADRESTRICT)(ylocal))[4] = ((U64* RADRESTRICT)(ylocal))[5];
      ((U64* RADRESTRICT)(ylocal))[6] = ((U64* RADRESTRICT)(ylocal))[7];

      // subtlety: the odd address in ylocal are zero, so this terminates 
      //   the right edge in addition to copying the final DC
      ((U32* RADRESTRICT)(ylocal))[2]    = ((U32* RADRESTRICT)&ylocal[32+0])[0];
      ((U32* RADRESTRICT)(ylocal))[2+4]  = ((U32* RADRESTRICT)&ylocal[32+2])[0];
      ((U32* RADRESTRICT)(ylocal))[2+8]  = ((U32* RADRESTRICT)&ylocal[32+4])[0];
      ((U32* RADRESTRICT)(ylocal))[2+12] = ((U32* RADRESTRICT)&ylocal[32+6])[0];

      ((U32* RADRESTRICT)(crlocal))[0] = ((U32* RADRESTRICT)(crlocal))[1];
      ((U32* RADRESTRICT)(crlocal))[2] = ((U32* RADRESTRICT)(crlocal))[3];
       
      ((U32* RADRESTRICT)(crlocal))[1]   = ((U32* RADRESTRICT)&crlocal[8+0])[0];
      ((U32* RADRESTRICT)(crlocal))[3]   = ((U32* RADRESTRICT)&crlocal[8+2])[0];
  
      ((U32* RADRESTRICT)(cblocal))[0] = ((U32* RADRESTRICT)(cblocal))[1];
      ((U32* RADRESTRICT)(cblocal))[2] = ((U32* RADRESTRICT)(cblocal))[3];
       
      ((U32* RADRESTRICT)(cblocal))[1]   = ((U32* RADRESTRICT)&cblocal[8+0])[0];
      ((U32* RADRESTRICT)(cblocal))[3]   = ((U32* RADRESTRICT)&cblocal[8+2])[0];

      goto again;
    }
  }

  if ( ( seamp ) && ( RAD_UNLIKELY( flags & ( Y_MID_TOP_T2_NEXT | Y_MID_TOP_T2 ) ) ) )
  {
    U32 new_ofs = (U32) ( ( (UINTa)seam - (UINTa)seamp ) - 4 );
    if ( new_ofs != seam_ofs )
      ((U32*)seamp)[0] = new_ofs;
  }
  dtmLeaveAccumulationZone( tm_mask, tm->deblock );
}

// encode bits (offset is how many dummy 0 bits to encode first)
static void rle_bit_decode( void * outp, U32 offset, VARBITSEND * inp, S32 cnt )
{
  U32 tmp;
  U8 * out;
  U32 bits;
  U32 bitcount;

  out = (U8*) outp;
  bits = 0;
  bitcount = offset;

  if ( VarBitsGet1( *inp, tmp ) )
  {
    VARBITSLOCAL( in );

    U32 c = cnt;
    S32 last_type = 0;
    U32 last_bit = 0;
    
    VarBitsCopyToLocal( in, *inp );

    while ( c )
    {
      if ( VarBitsLocalGet1( in, tmp ) )
      {
        //rle
        U32 rle_len, run;

        rle_len = RLE_LEN;

        if ( c < 4 )
          rle_len = 2;
        else if ( c < 8 )
          rle_len = 4;
        else if ( c < 16 )
          rle_len = 4;

        run = rle_len + 1;

        // send the rle bit
        if ( last_type == EARLY_RLE ) 
        {
          last_bit ^= 1;
        }
        else
        {
          ++run;
          last_bit = VarBitsLocalGet1( in, tmp );
        }

        if ( c < run )
          run = c;

        c -= run;

        if ( c )
        {
          VarBitsLocalGet( tmp,U32,in,rle_len );
  
          run += tmp;

          last_type = EARLY_RLE;
          c -= tmp;
          if ( tmp == ( ( 1U << rle_len ) - 1 ) )
            last_type = MAX_RLE;
        }

        // replicate last_bit into tmp
        tmp = (((S32)( last_bit << 31 ))>>31)&255;

        // copy run bits of last_bit in bytes first
        while( run > 8 )
        {
          bits |= (tmp<<bitcount);
          *out++ = (U8) bits;
          bits >>= 8;
          run -= 8;
        }
        // now do the remnant
        if ( run )
        {
          bits |= ((tmp&((1<<run)-1))<<bitcount);
          bitcount += run;
          if ( bitcount >= 8 )
          {
            *out++ = (U8) bits;
            bits >>= 8;
            bitcount -= 8;
          }
        }

      }
      else
      {
        // literal run
        if ( last_type == EARLY_RLE )
        {
          last_bit ^= 1;
        }
        else
        {
          last_bit = VarBitsLocalGet1( in, tmp );
        }

        // last_bit is set to the first bit of the literal run, but that
        //   doesn't matter, since we only use it as the output of an early rle

        bits |= (last_bit<<bitcount);
        bitcount++;
        --c;

        last_type = LITERAL;

        if ( c < ( LIT_LEN -1 ) )
        {
          VarBitsLocalGet( tmp,U32,in,c );
          // put c bits
          bits |= (tmp<<bitcount);
          bitcount += c;
          
          c = 0;
        }
        else
        {
          VarBitsLocalGet( tmp,U32,in,(LIT_LEN-1) );
          //put LIT_LEN -1
          bits |= (tmp<<bitcount);
          bitcount += (LIT_LEN-1);
          c -= (LIT_LEN-1);
        }

        if ( bitcount >= 8 )
        {
          *out++ = (U8) bits;
          bits >>= 8;
          bitcount -= 8;
        }
      }
    }
    VarBitsCopyFromLocal( *inp, in );
    if (bitcount)
      out[0]=(U8)bits; // flush left over byte
  }
  else
  {
    while( cnt > 8 )
    {
      VarBitsGet( tmp,U32,*inp,8 );
      bits |= (tmp<<bitcount);
      bitcount += 8;
      if ( bitcount >= 8 )
      {
        *out++ = (U8) bits;
        bits >>= 8;
        bitcount -= 8;
      }
      cnt -= 8;
    }
    if ( cnt )
    {
      VarBitsGet( tmp,U32,*inp,((U32)cnt) );
      bits |= (tmp<<bitcount);
      bitcount += 8;
      *out = (U8) bits;
    }
  }
}

static U32 dec_block_type( VARBITSEND * vb, U8 * last_type )
{
  U32 type;
  VARBITSLOCAL( lvb );

  VarBitsCopyToLocal( lvb, *vb );
  VarBitsLocalPeek( type, U32, lvb, 3 );
  if ( RAD_LIKELY( type & 1 ) )
  {
    VarBitsLocalUse( lvb, 1 );
    type = last_type[0];
  }
  else if ( RAD_LIKELY( type & 2 ) )
  {
    type = last_type[1];
    last_type[1]=last_type[0];
    last_type[0]=(U8)type;
    VarBitsLocalUse( lvb, 2 );
  }
  else if ( RAD_LIKELY( type == 0 ) )
  {
    type = last_type[2];
    last_type[2]=last_type[1];
    last_type[1]=(U8)type;
    VarBitsLocalUse( lvb, 3 );
  }
  else
  {
    type = last_type[3];
    last_type[3]=last_type[2];
    last_type[2]=(U8)type;
    VarBitsLocalUse( lvb, 3 );
  }
  VarBitsCopyFromLocal( *vb, lvb );
  return type;
}

void bink2_average_4_8x8( S32 *out, U8 const * RADRESTRICT px, U32 pitch )
{
  calc_average4x8_u8( (U8*)out, (U8*)px, pitch );
}

void bink2_average_8x8( S32 *out, U8 const * RADRESTRICT px, U32 pitch )
{
  calc_average8_u8( (U8*)out, (U8*)px, pitch );
}

void bink2_average_2_8x8( S32 * RADRESTRICT out, U8 const * RADRESTRICT ptr, U32 pitch )
{
  calc_average2x8_u8( (U8*)out, (U8*)ptr, pitch );
}


typedef struct CALCPREVDCS
{
  S32 y[8];
  S32 a[8];
  S32 h[8];
  S32 cr[4];
  S32 cb[4];
} CALCPREVDCS;

static void calcprevdcs( CALCPREVDCS * RADRESTRICT out, 
                           U16 const * RADRESTRICT ydcs, U16 const * RADRESTRICT crdcs, U16 const * RADRESTRICT cbdcs, U16 const * RADRESTRICT adcs, U16 const * RADRESTRICT hdcs, 
                           U8 const * RADRESTRICT ypixels, U8 const * RADRESTRICT crpixels, U8 const * RADRESTRICT cbpixels, U8 const * RADRESTRICT apixels, U8 const * RADRESTRICT hpixels, 
                           U32 lpitch, U32 cpitch, U32 flags )
{
  // first, unpack everything assuming both blocks above us are intra blocks.
  dcs_unpackx8( ydcs-3, out->y ); // Y
  dcs_unpackx4( crdcs-1, out->cr ); // Cr
  dcs_unpackx4( cbdcs-1, out->cb ); // Cb
  dcs_unpackx8( adcs-3, out->a ); // A
  dcs_unpackx8( hdcs-3, out->h ); // HDR

  // if NW block is not intra, need to average to get DCs for NW corner
  if ( ( ydcs[0]&0x8000 ) == 0 && ( flags & X_LEFT ) == 0 )
  {
    bink2_average_8x8( out->y+3, ypixels - 8 * lpitch - 8, lpitch ); // Y
    bink2_average_8x8( out->cr+1, crpixels - 8 * cpitch - 8, cpitch ); // Cr
    bink2_average_8x8( out->cb+1, cbpixels - 8 * cpitch - 8, cpitch ); // Cb
    if ( apixels )
      bink2_average_8x8( out->a+3, apixels - 8 * lpitch - 8, lpitch );
    if ( hpixels )
      bink2_average_8x8( out->h+3, hpixels - 8 * lpitch - 8, lpitch );
  }

  // and if N block is not intra, need to average across whole N edge
  if ( ( ydcs[1]&0x8000 ) == 0 )
  {
    bink2_average_4_8x8( out->y+4, ypixels - 8 * lpitch, lpitch ); // Y
    bink2_average_2_8x8( out->cr+2, crpixels - 8 * cpitch, cpitch ); // Cr
    bink2_average_2_8x8( out->cb+2, cbpixels - 8 * cpitch, cpitch ); // Cb
    if ( apixels )
      bink2_average_4_8x8( out->a+4, apixels - 8 * lpitch, lpitch ); 
    if ( hpixels )
      bink2_average_4_8x8( out->h+4, hpixels - 8 * lpitch, lpitch ); 
  }
}

static void predict_down_coords( U32 flags, S32 * RADRESTRICT cur_coords, S32 const * RADRESTRICT prev_coords )
{
  register S32 cs0,cs1,cs2,cs3,cs4,cs5;
  #define PREDMED3( o, W, N, NW ) o=med3( W, N, NW )

  if ( flags & ( Y_TOP | Y_MID_TOP ) )  // this is y==0, or y==h/2 (for the seam)
  {
    if ( flags & X_LEFT )
    {
      cur_coords[0]=0; cur_coords[1]=0; cur_coords[2]=0; cur_coords[3]=0;
      cur_coords[4]=0; cur_coords[5]=0; cur_coords[6]=0; cur_coords[7]=0;
    }
    else
    {
      S32 * RADRESTRICT Cs = cur_coords;
      S32 const * RADRESTRICT Ws = cur_coords-8;
      PREDMED3( cs0, Ws[0], Ws[2], Ws[6] );
      Cs[0]=cs0;
      PREDMED3( cs1, Ws[1], Ws[3], Ws[7] );
      Cs[1]=cs1;
      PREDMED3( cs4, cs0, Ws[2], Ws[6] );
      Cs[4]=cs4;
      PREDMED3( cs5, cs1, Ws[3], Ws[7] );
      Cs[5]=cs5;
      PREDMED3( cs2, cs0, cs4, Ws[2] );
      Cs[2]=cs2;
      PREDMED3( cs3, cs1, cs5, Ws[3] );
      Cs[3]=cs3;
      PREDMED3( Cs[6], cs0, cs2, cs4 );
      PREDMED3( Cs[7], cs1, cs3, cs5 );
    }
  }
  else if ( flags & X_LEFT )
  {
    S32 * RADRESTRICT Cs = cur_coords;
    S32 const * RADRESTRICT Ns = prev_coords;
    PREDMED3( cs0, Ns[0], Ns[4], Ns[6] );
    Cs[0]=cs0;
    PREDMED3( cs1, Ns[1], Ns[5], Ns[7] );
    Cs[1]=cs1;
    PREDMED3( cs2, cs0, Ns[4], Ns[6] );
    Cs[2]=cs2;
    PREDMED3( cs3, cs1, Ns[5], Ns[7] );
    Cs[3]=cs3;
    PREDMED3( cs4, cs0, cs2, Ns[4] );
    Cs[4]=cs4;
    PREDMED3( cs5, cs1, cs3, Ns[5] );
    Cs[5]=cs5;
    PREDMED3( Cs[6], cs0, cs2, cs4 );
    PREDMED3( Cs[7], cs1, cs3, cs5 );

  }
  else
  {
    S32 * RADRESTRICT Cs = cur_coords;
    S32 const * RADRESTRICT Ns = prev_coords;
    S32 const * RADRESTRICT Ws = cur_coords-8;
    S32 const * RADRESTRICT NWs = prev_coords-8;
    PREDMED3( cs0, Ws[2], Ns[4], NWs[6] );
    Cs[0]=cs0;
    PREDMED3( cs1, Ws[3], Ns[5], NWs[7] );
    Cs[1]=cs1;
    PREDMED3( cs2, cs0, Ns[6], Ns[4] );
    Cs[2]=cs2;
    PREDMED3( cs3, cs1, Ns[7], Ns[5] );
    Cs[3]=cs3;
#ifdef BINK20_SUPPORT
    if ( RAD_UNLIKELY( ( flags & BINKVER22 ) == 0 ) )
    {
      PREDMED3( cs4, cs0, cs2, Ns[4] );
      PREDMED3( cs5, cs1, cs3, Ns[5] );
    }
    else
#endif
    {
      PREDMED3( cs4, cs0, Ws[2], Ws[6] );
      PREDMED3( cs5, cs1, Ws[3], Ws[7] );
    }
    Cs[4]=cs4;
    Cs[5]=cs5;
    PREDMED3( Cs[6], cs0, cs2, cs4 );
    PREDMED3( Cs[7], cs1, cs3, cs5 );
  }
  #undef PREDMED3
}

static void layslice( U32 start, U32 height, U32 slices, U32 * harray )
{
  if ( slices == 1 )
    harray[0] = height;
  else
  {
    U32 hd = ( ( ( ( height - start ) + ( ( slices * 32 ) - 1 ) ) / slices ) / 32 ) * 32;
    hd += start;
    harray[0] = hd;
    layslice( hd, height, slices - 1, harray + 1 );
  }
}

RRSTRIPPABLEPUB U8 slice_mask_to_count[4]={2,3,4,8};

// first U32 = count of slices, 2nd is the seam pitch, 3rd on is the slice end Y
void setup_slices( U32 marker, U32 slice_table_index, U32 width, U32 height, BINKSLICES * s )
{
  s->slice_pitch = 4+((width+31)/32)*4;
  if ( RAD_UNLIKELY( ( marker==(U32)BINKMARKERV2) ) )  // bink 2.0
  {
    U32 mh = ( (height+32) >> (5+1) ) << 5;  //old way
    s->slice_count = 2;
    s->slices[0] = mh;
    if ( mh != height )
      s->slices[1] = height;  
  }
  else
  {
    height = (height+31)&~31;
    if ( RAD_UNLIKELY( ( marker==(U32)BINKMARKERV22) ) ) // bink 2.2
    {
      if ( RAD_LIKELY( height >= 128 ) )
      {
        s->slice_count = 2;
        s->slices[0] = ( ( ( height ) >> (5+1) ) << 5 );  
        s->slices[1] = height;
      }
      else
      {
        s->slice_count = 1;
        s->slices[0] = height;
      }
    }
    else if ( RAD_LIKELY( is_binkv23_or_later( marker ) ) )
    {
      if ( RAD_LIKELY( height >= 128 ) )
      {
        s->slice_count = SliceMaskToCount( slice_table_index );
        layslice( 0, height, s->slice_count, s->slices );
      }
      else
      {
        s->slice_count = 1;
        s->slices[0] = height;
      }
    }
    else  // bink 1
    {
      s->slice_count = 2;
      s->slice_pitch = 0;
    }
  }
}


static void fill_alpha16( U8 * a, U32 width, U32 pitch, simd4i v128 )
{
  S32 i;

  for( i = 0 ; i < 16 ; i++ )
  {
    S32 w;
    w = width / 32;
    while(w)
    {
      ((simd4i* RADRESTRICT)a)[0]=v128;
      ((simd4i* RADRESTRICT)a)[1]=v128;
      a += 32;
      --w;
    }
    a = a + (pitch-width);
  }
}


static RADINLINE U32 debug_update_frame_flags( U32 frameflags )
{
#ifdef NO_EDGE_BLUR
  frameflags|=BINK_DEBLOCK_UANDD_OFF|BINK_DEBLOCK_LTOR_OFF;//manually force blurring off
#endif
  //frameflags&=~(BINK_DEBLOCK_UANDD_OFF|LTOR_OFF);//manually force blurring on

  return frameflags;
}

static U32 doplanes( BINKFRAMEBUFFERS * b, void * out, S32 alpha, S32 hdr, S32 key, void * out_end, BINKSLICES * slices, U32 startslice, U32 numslices, void * seam )
{
  U32 lwidth, h, cwidth, cp8, wd32, lpitch, cpitch;
  U32 x, y;
  S32 do_alpha;
  U8 * yt, * cr, * cb, * at, *ht;
  U8 * oyp, * ocrp, * ocbp, * oap, * ohp;
  U8 * ytp, * crp, * cbp, *atp, *htp;
  S32 * row_coords0, * row_coords1;
  S32 * prev_coords, * cur_coords;
  U8 * row_Qs0, * row_Qs1;
  U8 * prev_Qs, * cur_Qs;
  U16 * ydcs, * ydc;
  U16 * adcs, * adc;
  U16 * hdcs, * hdc;
  U16 * cdcs, * cdc;
  U32 ydcpitch, cdcpitch;
  RAD_ALIGN_SIMD( LOCAL_ROW_DCS, dcs_locals );
  RAD_ALIGN_SIMD( BLOCK_DCT, curdct );
  S32 yadj, cadj;
  RAD_ALIGN_SIMD( simd4i, av128 );
  RAD_ALIGN_SIMD( U16, threshold[8] );
  RAD_ALIGN_SIMD( CALCPREVDCS, prevdcs );
  VARBITSEND vb; 
  S32 ret = 0;
  U32 frameflags = 0;
  U32 yflags = 0;
  U32 motion_w2, motion_h2;
  U8 * row_masks;
  U8 * col_masks;

  #ifndef NTELEMETRY
    U64 start;
    TMTIMES t;
    static char const * const thrstr[4]={0,"Top Half","Bottom Half","Both Halves"};
    tmEnter( tm_mask, TMZF_NONE, "Bink 2 Expand %d-%d", (startslice==0)?0:slices->slices[startslice-1],slices->slices[startslice+numslices-1] );
    start = tmGetAccumulationStart( tm_mask );
    rrmemset( &t, 0, sizeof( t ) );
  #endif

  lwidth = b->YABufferWidth;
  cwidth = b->cRcBBufferWidth; cwidth=cwidth;

  motion_w2 = b->YABufferWidth * 2;
  motion_h2 = b->YABufferHeight * 2;

  frameflags = ((U32*)out)[0];

  vb.end = out_end;

  wd32 = lwidth / 32;

  lpitch=b->Frames[0].YPlane.BufferPitch;
  cpitch=b->Frames[0].cRPlane.BufferPitch;
  cp8 = cpitch * 8;

  yadj = lpitch * 32 - ( wd32 * 32 );
  cadj = cpitch * 16 - ( wd32 * 16 );

  {
    S32 oldf = b->FrameNum;
    S32 newf = oldf + 1;
    if ( newf >= b->TotalFrames )
      newf = 0;

    yt = (U8*) b->Frames[newf].YPlane.Buffer;
    cr = (U8*) b->Frames[newf].cRPlane.Buffer;
    cb = (U8*) b->Frames[newf].cBPlane.Buffer;
    at = (U8*) b->Frames[newf].APlane.Buffer;
    ht = (U8*) b->Frames[newf].HPlane.Buffer;

    oyp = (U8*) b->Frames[oldf].YPlane.Buffer;
    ocrp = (U8*) b->Frames[oldf].cRPlane.Buffer;
    ocbp = (U8*) b->Frames[oldf].cBPlane.Buffer;
    oap = (U8*) b->Frames[oldf].APlane.Buffer;
    ohp = (U8*) b->Frames[oldf].HPlane.Buffer;
    ytp = oyp;
    crp = ocrp;
    cbp = ocbp;
    atp = oap;
    htp = ohp;

    do_alpha=(at)?1:0;
    if ( alpha == 0 ) do_alpha = 0; //alpha is presence of alpha in bit stream, do_alpha means do it
  }

  av128 = simd4i_0;
  if ( frameflags & BINKCONSTANTA )
  {
    alpha = do_alpha = 0;  // if it's constant alpha, it's not in the bitstream
    x = frameflags >> 24;
    x |= ( x << 8 );
    x |= ( x << 16 );
    ((U32*)&av128)[0]=((U32*)&av128)[1]=((U32*)&av128)[2]=((U32*)&av128)[3]=x;
    if ( at == 0 )
      frameflags &= ~BINKCONSTANTA;
  }


  {
    x = wd32 * 8 * sizeof( S32 );
    row_coords0 = (S32*) radalloca( 2 * x );
    row_coords1 = (S32*)( ( (U8*)row_coords0 ) + x );
    prev_coords = row_coords1;
    cur_coords = row_coords0;
  }

  if( RAD_LIKELY( frameflags & BINKVER22 ) )
  {
    x = wd32 * 2 * sizeof( U8 ); // 2 values per 32x32: DCT and MDCT Q
    row_Qs0 = (U8*) radalloca( 2 * x ); // 2 rows
    row_Qs1 = row_Qs0 + x;
    prev_Qs = row_Qs1;
    cur_Qs = row_Qs0;
  }
  else
  {
    row_Qs0 = row_Qs1 = NULL;
    prev_Qs = cur_Qs = NULL;
#ifndef BINK20_SUPPORT
    goto ret;
#endif
  }

  row_masks = radalloca( ( (b->YABufferHeight/8)+7)/8 );
  col_masks = radalloca( ( (b->YABufferWidth/8)+7)/8 );

  ydcpitch = ( wd32 * 4 * sizeof(U16) + sizeof(U16) + sizeof(U16) + 15 ) & ~15;
  cdcpitch = ( wd32 * 2 * sizeof(U16) + sizeof(U16) + sizeof(U16) + 15 ) & ~15;
  hdcs = (U16*)( ( ( (UINTa) radalloca( ydcpitch * 6 + cdcpitch * 4 + 15 ) ) + 15 ) & ~15 );
  adcs = (U16*) ( ( (UINTa) hdcs ) + ydcpitch + ydcpitch );
  ydcs = (U16*) ( ( (UINTa) adcs ) + ydcpitch + ydcpitch );
  cdcs = (U16*) ( ( (UINTa) ydcs ) + ydcpitch + ydcpitch );
  ydcs[ wd32 * 4 + 1 ] = 0;
  adcs[ wd32 * 4 + 1 ] = 0;
  hdcs[ wd32 * 4 + 1 ] = 0;
  ((U16*)(((UINTa)ydcs)+ydcpitch))[ wd32 * 4 + 1 ] = 0;
  ((U16*)(((UINTa)adcs)+ydcpitch))[ wd32 * 4 + 1 ] = 0;
  ((U16*)(((UINTa)hdcs)+ydcpitch))[ wd32 * 4 + 1 ] = 0;
  cdcs[ wd32 * 2 + 1 ] = 0;
  ((U16*)(((UINTa)cdcs)+cdcpitch))[ wd32 * 2 + 1 ] = 0;
  ((U16*)(((UINTa)cdcs)+2*cdcpitch))[ wd32 * 2 + 1 ] = 0;
  ((U16*)(((UINTa)cdcs)+3*cdcpitch))[ wd32 * 2 + 1 ] = 0;


  // zero everything
  ((simd4i* RADRESTRICT)(dcs_locals.ylocals))[0]=simd4i_0;
  ((simd4i* RADRESTRICT)(dcs_locals.ylocals))[1]=simd4i_0;
  ((simd4i* RADRESTRICT)(dcs_locals.ylocals))[2]=simd4i_0;
  ((simd4i* RADRESTRICT)(dcs_locals.ylocals))[3]=simd4i_0;
  ((simd4i* RADRESTRICT)(dcs_locals.ylocals))[4]=simd4i_0;
  ((simd4i* RADRESTRICT)(dcs_locals.alocals))[0]=simd4i_0;
  ((simd4i* RADRESTRICT)(dcs_locals.alocals))[1]=simd4i_0;
  ((simd4i* RADRESTRICT)(dcs_locals.alocals))[2]=simd4i_0;
  ((simd4i* RADRESTRICT)(dcs_locals.alocals))[3]=simd4i_0;
  ((simd4i* RADRESTRICT)(dcs_locals.alocals))[4]=simd4i_0;
  ((simd4i* RADRESTRICT)(dcs_locals.hlocals))[0]=simd4i_0;
  ((simd4i* RADRESTRICT)(dcs_locals.hlocals))[1]=simd4i_0;
  ((simd4i* RADRESTRICT)(dcs_locals.hlocals))[2]=simd4i_0;
  ((simd4i* RADRESTRICT)(dcs_locals.hlocals))[3]=simd4i_0;
  ((simd4i* RADRESTRICT)(dcs_locals.hlocals))[4]=simd4i_0;
  ((simd4i* RADRESTRICT)(dcs_locals.crlocals))[0]=simd4i_0;
  ((simd4i* RADRESTRICT)(dcs_locals.crlocals))[1]=simd4i_0;
  ((simd4i* RADRESTRICT)(dcs_locals.cblocals))[0]=simd4i_0;
  ((simd4i* RADRESTRICT)(dcs_locals.cblocals))[1]=simd4i_0;
  ((simd4i* RADRESTRICT)(dcs_locals.next_LtoRs))[0]=simd4i_0;

  y = h = 0; // just for killing warnings
  if ( startslice == 0 )
  {
    y = 0;
    h = slices->slices[ 0 ]; // get first end slice 
  
    seam = (U8*) seam - slices->slice_pitch; 

    yflags = 0;
  }

  VarBitsOpen( vb, (((U8*)out)+((slices->slice_count<=2)?8:(4*slices->slice_count))) );

  if ( frameflags & BINKVER23BLURBITS )
  {
    if ( ( frameflags & BINK_DEBLOCK_LTOR_OFF ) == 0 )
      rle_bit_decode( row_masks, 1, &vb, (b->YABufferHeight/8)-1 );
    else
      rrmemset( row_masks, 0, ( (b->YABufferHeight/8)+7)/8 );

    if ( ( frameflags & BINK_DEBLOCK_UANDD_OFF ) == 0 )
      rle_bit_decode( col_masks, 1, &vb, (b->YABufferWidth/8)-1 );
    else
      rrmemset( col_masks, 0, ( (b->YABufferWidth/8)+7)/8 );
  }
  else
  {
    rrmemset( row_masks, 0, ( (b->YABufferHeight/8)+7)/8 );
    rrmemset( col_masks, 0, ( (b->YABufferWidth/8)+7)/8 );
  }

  // this needs to happen after the rle_bit_decode for the block flags
  frameflags = debug_update_frame_flags( frameflags );

  if ( startslice != 0 )
  {
    y = slices->slices[ startslice-1 ];    // get start slice row
    h = slices->slices[ startslice ];  // get stop slice end
    
    seam = (U8*) seam + (slices->slice_pitch*(startslice-1)); 

    x = ( lpitch * y );
    yt += x;
    ytp += x;
    at += x;
    atp += x;
    ht += x;
    htp += x;
 
    x = ( cpitch * (y>>1) );
    cr += x;
    cb += x;
    crp += x;
    cbp += x;

    yflags = Y_MID_TOP_T2; // defaults to this

    x = ((U32*)out)[startslice];
    VarBitsOpen( vb, (((U8*)out)+x) );
  }

//{ U32 v; VarBitsLocalGet( v,U32,vb,8 ); if (v!=247) RR_BREAK();}


  threshold[7]=threshold[6]=threshold[5]=threshold[4]=threshold[3]=threshold[2]=threshold[1]=threshold[0]=(U16)((frameflags&255)<<3);

  for(;;)
  {
    if ( RAD_UNLIKELY( y == 0 ) )
    {
      yflags = Y_TOP;
    }
    else
    {
      yflags |= Y_MID_TOP;  // or in because we could be inheriting Y_MID_TOP_T1 or Y_MID_TOP_T2

      // set up the previous block's ltor flags
      yflags |= (((row_masks[(y-32)>>6]>>(((y-32)>>(5-2))&4))&0xf)*NO_LTOR_A3);
    }

    yflags |= (frameflags & (BINK_DEBLOCK_LTOR_OFF|BINK_DEBLOCK_UANDD_OFF|BINKVER22|BINKVER23EDGEMOFIX));

    if (key) yflags|=IS_KEY;

    while( y < h )
    {
      RAD_ALIGN_SIMD( S32, luma_dcs[ 16 ] );
      RAD_ALIGN_SIMD( S32, chroma_dcs[ 8 ] );
      RAD_ALIGN_SIMD( S32, alpha_dcs[ 16 ] );
      RAD_ALIGN_SIMD( S32, hdr_dcs[ 16 ] );

      U32 v=0;
      U32 yCn, aCn, hCn, crCn, cbCn;
      U32 yCnm, aCnm, hCnm, crCnm, cbCnm;
      U8 bink20_Qs[8]; // DCT/MDCT * 4 planes
      U8 last_type[4];
      #ifdef MemoryCommit
      U32 lCommitX = 0;
      U32 cCommitX = 0;
      U32 lCommitSub = 0, cCommitSub = 0;
      U32 lCommitLines, cCommitLines;
      #endif
      U32 flags = 0;

      RAD_ALIGN_SIMD( U8, right_inter_edges[16*16+16*16+16*8+16*8+16*16] );  // y 16x16, a 16x16, cr 16x8, cb 16x8, h 16x16
      
      ydc = ydcs;
      adc = adcs;
      hdc = hdcs;
      cdc = cdcs;
      
      x = 0;
      yCn = aCn = hCn = yCnm = aCnm = hCnm = INITIAL_CONTROL_4_PREDICT;
      crCn = cbCn = crCnm = cbCnm = INITIAL_CONTROL_1_PREDICT;

      bink20_Qs[0] = bink20_Qs[1] = bink20_Qs[2] = bink20_Qs[3] = 8; // DCT
      bink20_Qs[4] = bink20_Qs[5] = bink20_Qs[6] = bink20_Qs[7] = 8; // MDCT

      yflags = (yflags & ~NO_LTOR_MASKS)|((yflags&(NO_LTOR_C1|NO_LTOR_C2))>>4);
      yflags |= (((row_masks[y>>6]>>((y>>(5-2))&4))&0xf)*NO_LTOR_A3);

      if ( RAD_UNLIKELY( ( y + 32 ) >= b->YABufferHeight ) )
      {
        yflags |= Y_BOTTOM;
        if ( ( y + 32 ) > b->YABufferHeight )
        {
          yflags |= Y_NO_BOTTOM_16;
        }
      }

      rrassert( ( ( yflags & Y_BOTTOM ) == 0 ) || ( ( yflags & ( Y_MID_TOP_T1 | Y_MID_TOP_T2 ) ) == 0 ) );

      last_type[0] = BLK_MNR; last_type[1] = BLK_MDCT; last_type[2] = BLK_SKIP; last_type[3] = BLK_DCT;

    #ifdef MemoryCommit
      lCommitLines = 22/2;
      cCommitLines = 6/2;

      if( RAD_LIKELY( ( yflags & ( Y_TOP|Y_MID_TOP_T2 ) ) == 0 ) )
      {
        lCommitSub = 10;
        lCommitLines += 10/2;
        cCommitSub = 10;
        cCommitLines += 10/2;
      }

      if ( RAD_UNLIKELY( ( y + 32 ) >= h ) )
      {
        lCommitLines += 10/2;
        cCommitLines += 10/2;

        if ( b->YABufferHeight < h )
        {
          U32 stoplines = ( ( y - lCommitSub+ ( lCommitLines * 2 ) ) - b->YABufferHeight ) / 2;
          lCommitLines -= stoplines;
          stoplines = ( ( (y/2) - cCommitSub + ( cCommitLines * 2 ) ) - (b->YABufferHeight/2) ) / 2;
          cCommitLines -= stoplines;
        }
      }

      // back in pitch
      lCommitSub *= lpitch;
      cCommitSub *= cpitch;


    #endif

      // Initial Q prediction for this row
      if( RAD_LIKELY( frameflags & BINKVER22 ) )
      {
        if ( RAD_UNLIKELY( yflags & (Y_TOP|Y_MID_TOP) ) )
        {
          cur_Qs[0] = INITIAL_Q_PREDICT;
          cur_Qs[1] = INITIAL_Q_PREDICT;
        }
        else
        {
          // predict from top
          cur_Qs[0] = prev_Qs[0];
          cur_Qs[1] = prev_Qs[1];
        }
      }

      if ( ( frameflags & BINKCONSTANTA ) && ( at ) )
      {
        if ( RAD_LIKELY( ( yflags & Y_MID_TOP_T2 ) == 0 ) )
          fill_alpha16( at, lwidth, lpitch, av128 );
        if ( RAD_LIKELY( ( yflags & (Y_MID_TOP_T1|Y_NO_BOTTOM_16) ) == 0 ) )
          fill_alpha16( at+lpitch*16, lwidth, lpitch, av128 );
      }
 
      for(;;)
      {
//         { U32 v; VarBitsGet( v,U32,vb,8 );
//         if (v != 243 ) RR_BREAK(); }        

        flags = ( flags & NO_UANDD_MASKS ) | yflags;
        if ( RAD_UNLIKELY( x == 0 ) ) flags |= X_LEFT;
        if ( RAD_UNLIKELY( ( x + 32 ) ) >= lwidth ) flags |= X_RIGHT;
        if ( RAD_UNLIKELY( x == 32 ) ) flags |= X_2ND_BLOCK;

        // rotate the column flags
        flags = (flags & ~NO_UANDD_MASKS)|((flags&(NO_UANDD_L3|NO_UANDD_C0|NO_UANDD_C1|NO_UANDD_C2))>>4);
        flags |= (((col_masks[x>>6]>>((x>>(5-2))&4))&0xf)*NO_UANDD_L3);

        gtmEnterAccumulationZone( tm_mask, t.prefetch );

        #ifdef MemoryCommit
        if ( ( x - lCommitX ) >= ( RR_CACHE_LINE_SIZE + RR_CACHE_LINE_SIZE ) )
        {
          dtmEnterAccumulationZone( tm_mask, t.luma.commit );
          MemoryCommitMultiple_x2( yt - ( x - lCommitX ) - lCommitSub, lpitch, lCommitLines );
          dtmLeaveAccumulationZone( tm_mask, t.luma.commit );

          if ( do_alpha )
          {
            dtmEnterAccumulationZone( tm_mask, t.alpha.commit );
            MemoryCommitMultiple_x2( at - ( x - lCommitX ) - lCommitSub, lpitch, lCommitLines );
            dtmLeaveAccumulationZone( tm_mask, t.alpha.commit );
          }

          if ( hdr )
          {
            dtmEnterAccumulationZone( tm_mask, t.hdr.commit );
            MemoryCommitMultiple_x2( ht - ( x - lCommitX ) - lCommitSub, lpitch, lCommitLines );
            dtmLeaveAccumulationZone( tm_mask, t.hdr.commit );
          }

          lCommitX += RR_CACHE_LINE_SIZE;
        }

        if ( ( (x/2) - cCommitX ) >= ( RR_CACHE_LINE_SIZE + RR_CACHE_LINE_SIZE ) )
        {
          dtmEnterAccumulationZone( tm_mask, t.chroma.commit );
          MemoryCommitMultiple_x2( cr - ( ( x/2) - cCommitX ) - cCommitSub, cpitch, cCommitLines );
          MemoryCommitMultiple_x2( cb - ( ( x/2) - cCommitX ) - cCommitSub, cpitch, cCommitLines );
          cCommitX += RR_CACHE_LINE_SIZE;
          dtmLeaveAccumulationZone( tm_mask, t.chroma.commit );
        }
        #endif

        #if !defined(DONT_PREFETCH_DEST) && (defined(PrefetchForRead) || defined(PrefetchForOverwrite))

        if ( ( ( (UINTa) yt ) & (RR_CACHE_LINE_SIZE-1) ) == 0 )
        {
          if ( ( lwidth - x ) >= RR_CACHE_LINE_SIZE )
          {
            dtmEnterAccumulationZone( tm_mask, t.luma.clear );
            #ifdef PrefetchForRead
            if ( ( flags & ( Y_TOP|Y_MID_TOP_T2 ) ) == 0 )
              PrefetchForRead_x10( yt - 10*lpitch, lpitch );
            #endif
            #ifdef PrefetchForOverwrite
            if ( ( flags & Y_MID_TOP_T2 ) == 0 )
            {
              U8 * ptr = yt;
              ptr = PrefetchForOverwrite_x8( ptr, lpitch );
              ptr = PrefetchForOverwrite_x8( ptr, lpitch );
            }
            if ( ( flags & (Y_MID_TOP_T1|Y_NO_BOTTOM_16) ) == 0 )
            {
              U8 * ptr = yt + 16*lpitch;
              ptr = PrefetchForOverwrite_x8( ptr, lpitch );
              ptr = PrefetchForOverwrite_x8( ptr, lpitch );
            }
            #endif
            dtmLeaveAccumulationZone( tm_mask, t.luma.clear );
          }
        }

        if ( do_alpha )
        {
          if ( ( ( (UINTa) at ) & (RR_CACHE_LINE_SIZE-1) ) == 0 )
          {
            if ( ( lwidth - x ) >= RR_CACHE_LINE_SIZE )
            {
              dtmEnterAccumulationZone( tm_mask, t.alpha.clear );
              #ifdef PrefetchForRead
              if ( ( flags & ( Y_TOP|Y_MID_TOP_T2 ) ) == 0 )
                PrefetchForRead_x10( at - 10*lpitch, lpitch );
              #endif
              #ifdef PrefetchForOverwrite
              if ( ( flags & Y_MID_TOP_T2 ) == 0 )
              {
                U8 * ptr = at;
                ptr = PrefetchForOverwrite_x8( ptr, lpitch );
                ptr = PrefetchForOverwrite_x8( ptr, lpitch );
              }
              if ( ( flags & (Y_MID_TOP_T1|Y_NO_BOTTOM_16) ) == 0 )
              {
                U8 * ptr = at + 16*lpitch;
                ptr = PrefetchForOverwrite_x8( ptr, lpitch );
                ptr = PrefetchForOverwrite_x8( ptr, lpitch );
              }
              #endif
              dtmLeaveAccumulationZone( tm_mask, t.alpha.clear );
            }
          }
        }
    
        if ( hdr )
        {
          if ( ( ( (UINTa) ht ) & (RR_CACHE_LINE_SIZE-1) ) == 0 )
          {
            if ( ( lwidth - x ) >= RR_CACHE_LINE_SIZE )
            {
              dtmEnterAccumulationZone( tm_mask, t.hdr.clear );
              #ifdef PrefetchForRead
              if ( ( flags & ( Y_TOP|Y_MID_TOP_T2 ) ) == 0 )
                PrefetchForRead_x10( ht - 10*lpitch, lpitch );
              #endif
              #ifdef PrefetchForOverwrite
              if ( ( flags & Y_MID_TOP_T2 ) == 0 )
              {
                U8 * ptr = ht;
                ptr = PrefetchForOverwrite_x8( ptr, lpitch );
                ptr = PrefetchForOverwrite_x8( ptr, lpitch );
              }
              if ( ( flags & (Y_MID_TOP_T1|Y_NO_BOTTOM_16) ) == 0 )
              {
                U8 * ptr = ht + 16*lpitch;
                ptr = PrefetchForOverwrite_x8( ptr, lpitch );
                ptr = PrefetchForOverwrite_x8( ptr, lpitch );
              }
              #endif
              dtmLeaveAccumulationZone( tm_mask, t.hdr.clear );
            }
          }
        }
    
        if ( ( cwidth - (x>>1) ) >= RR_CACHE_LINE_SIZE )
        {
          if ( ( ( (UINTa) cr ) & (RR_CACHE_LINE_SIZE-1) ) == 0 )
          {
            dtmEnterAccumulationZone( tm_mask, t.chroma.clear );
            #ifdef PrefetchForRead
            if ( ( flags & ( Y_TOP|Y_MID_TOP_T2 ) ) == 0 )
              PrefetchForRead_x10( cr - 10*cpitch, cpitch );
            #endif
            #ifdef PrefetchForOverwrite
            if ( ( flags & Y_MID_TOP_T2 ) == 0 )
              PrefetchForOverwrite_x8( cr, cpitch );
            if ( ( flags & (Y_MID_TOP_T1|Y_NO_BOTTOM_16) ) == 0 )
              PrefetchForOverwrite_x8( cr+8*cpitch, cpitch );
            #endif
            dtmLeaveAccumulationZone( tm_mask, t.chroma.clear );
          } 
          if ( ( ( (UINTa) cb ) & (RR_CACHE_LINE_SIZE-1) ) == 0 )
          {
            dtmEnterAccumulationZone( tm_mask, t.chroma.clear );
            #ifdef PrefetchForRead
            if ( ( flags & ( Y_TOP|Y_MID_TOP_T2 ) ) == 0 )
              PrefetchForRead_x10( cb - 10*cpitch, cpitch );
            #endif
            #ifdef PrefetchForOverwrite
            if ( ( flags & Y_MID_TOP_T2 ) == 0 )
              PrefetchForOverwrite_x8( cb, cpitch );
            if ( ( flags & (Y_MID_TOP_T1|Y_NO_BOTTOM_16) ) == 0 )
              PrefetchForOverwrite_x8( cb+8*cpitch, cpitch );
            #endif
            dtmLeaveAccumulationZone( tm_mask, t.chroma.clear );
          } 
        }
        #endif
        gtmLeaveAccumulationZone( tm_mask, t.prefetch );

        if ( key ) 
        {
          goto dct;          
        }
        else
        {
          if ( RAD_LIKELY( ( flags & BINKVER22 ) ) )
            v = dec_block_type( &vb, last_type );
          else
          {
            VarBitsGet( v,U32,vb,2 );
          }
        }

        switch ( v )
        {
          case BLK_DCT:
          dct:
            gtmEnterAccumulationZoneAndInc( tm_mask, t.dct );

            // get the above dcs to predict from
            if ( RAD_LIKELY( ( flags & (Y_TOP|Y_MID_TOP) ) == 0 ) )
            {
              calcprevdcs( &prevdcs, (U16*)(((U8*)ydc)+ydcpitch), 
                                     (U16*)(((U8*)cdc)+cdcpitch), (U16*)(((U8*)cdc)+(cdcpitch*3)), 
                                     (U16*)(((U8*)adc)+ydcpitch), (U16*)(((U8*)hdc)+ydcpitch),
                                     yt, cr, cb, (do_alpha)?at:0, (hdr)?ht:0, lpitch, cpitch, flags );
            }
            
            if ( RAD_LIKELY( ( ( x ) && ( ( dcs_locals.ylocals[5]&0x8000 ) == 0 ) ) ) )
            {
              U32 tlpitch = lpitch;
              U32 tcpitch = cpitch;
              U32 blpitch = lpitch;
              U32 bcpitch = cpitch;
              U8 * tysrc = yt - 8;
              U8 * tasrc = at - 8;
              U8 * thsrc = ht - 8;
              U8 * tcrsrc = cr - 8;
              U8 * tcbsrc = cb - 8;
              U8 * bysrc = yt - 8 + 16*lpitch;
              U8 * basrc = at - 8 + 16*lpitch;
              U8 * bhsrc = ht - 8 + 16*lpitch;
              U8 * bcrsrc = cr - 8+ 8*cpitch;
              U8 * bcbsrc = cb - 8+ 8*cpitch;

              if ( RAD_UNLIKELY( ( flags & Y_MID_TOP_T2 ) != 0 ) )
              {
                tlpitch = 16;
                tcpitch = 16;
                tysrc = right_inter_edges+8;
                tasrc = right_inter_edges+16*16+8;
                tcrsrc = right_inter_edges+16*16*2+8;
                tcbsrc = right_inter_edges+16*16*2+16*8+8;
                thsrc = right_inter_edges+16*16*2+16*8+16*8+8;
              }
              else if ( RAD_UNLIKELY( ( flags & (Y_MID_TOP_T1|Y_NO_BOTTOM_16) ) != 0 ) )
              {
                blpitch = 16;
                bcpitch = 16;
                bysrc = right_inter_edges+8;
                basrc = right_inter_edges+16*16+8;
                bcrsrc = right_inter_edges+16*16*2+8;
                bcbsrc = right_inter_edges+16*16*2+16*8+8;
                bhsrc = right_inter_edges+16*16*2+16*8+16*8+8;
              }

              bink2_average_8x8( luma_dcs+5,   tysrc+0*tlpitch, tlpitch );
              bink2_average_8x8( luma_dcs+7,   tysrc+8*tlpitch, tlpitch );
              bink2_average_8x8( chroma_dcs+1, tcrsrc, tcpitch );
              bink2_average_8x8( chroma_dcs+5, tcbsrc, tcpitch );

              if ( RAD_UNLIKELY( do_alpha ) )
              {
                bink2_average_8x8( alpha_dcs+5, tasrc+0*tlpitch, tlpitch );
                bink2_average_8x8( alpha_dcs+7, tasrc+8*tlpitch, tlpitch );
              }

              if ( RAD_UNLIKELY( hdr ) )
              {
                bink2_average_8x8( hdr_dcs+5, thsrc+0*tlpitch, tlpitch );
                bink2_average_8x8( hdr_dcs+7, thsrc+8*tlpitch, tlpitch );
              }

              bink2_average_8x8( luma_dcs+13, bysrc+0*blpitch, blpitch );
              bink2_average_8x8( luma_dcs+15, bysrc+8*blpitch, blpitch );
              bink2_average_8x8( chroma_dcs+3, bcrsrc, bcpitch );
              bink2_average_8x8( chroma_dcs+7, bcbsrc, bcpitch );

              if ( RAD_UNLIKELY( do_alpha ) )
              {
                bink2_average_8x8( alpha_dcs+13, basrc+0*blpitch, blpitch );
                bink2_average_8x8( alpha_dcs+15, basrc+8*blpitch, blpitch );
              }

              if ( RAD_UNLIKELY( hdr ) )
              {
                bink2_average_8x8( hdr_dcs+13, bhsrc+0*blpitch, blpitch );
                bink2_average_8x8( hdr_dcs+15, bhsrc+8*blpitch, blpitch );
              }
            }

#ifdef BINK20_SUPPORT
            if ( RAD_UNLIKELY( ( flags & BINKVER22 ) == 0 ) )
            {
              RAD_ALIGN_SIMD( U32, acs[4] );

              bink20_dec_dct_luma( OPT_PROFILE_PASS(&t.luma)  luma_dcs, prevdcs.y, yt, lpitch, &vb, (U16*)curdct.ydcts, flags, bink20_Qs + 0, &yCn );
              bink20_dec_dct_chroma( OPT_PROFILE_PASS(&t.chroma)  chroma_dcs,  prevdcs.cr, cr, cpitch, &vb, acs, flags, bink20_Qs + 1, &crCn );
              bink20_dec_dct_chroma( OPT_PROFILE_PASS(&t.chroma)  chroma_dcs+4,prevdcs.cb, cb, cpitch, &vb, acs+1, flags, bink20_Qs + 2, &cbCn );

              dcs8_pack( (U8*)curdct.cdcts, (U8*)chroma_dcs, (U8*)&intraflag, (U8*) acs );

              if( RAD_UNLIKELY( alpha == 1 ) )
                bink20_dec_dct_luma( OPT_PROFILE_PASS(&t.alpha)  alpha_dcs, prevdcs.a, (do_alpha)?at:0, lpitch, &vb, (U16*)curdct.adcts, flags, bink20_Qs + 3, &aCn );
            }
            else
#endif
            {
              U8 Q;
              // crazy ugly - the dct functions need the Bink2.5 flag, but not the deblock mask flags
              //    we are out of flags, so we just or out the deblock mask and add the bink2.5 flag instead
              U32 dct_flags = (flags&~(NO_UANDD_MASKS|NO_LTOR_MASKS)) | ( frameflags & BINKVER25 );

              cur_Qs[ 0 ] = Q = (U8) decode_q( &vb, flags, cur_Qs[0] DWRITECODE(,"q-len","q-dct-diff","q-dct" ) );

              dec_dct_luma( OPT_PROFILE_PASS(&t.luma)  luma_dcs, prevdcs.y, yt, lpitch, &vb, (U16*)curdct.ydcts, dct_flags, Q, &yCn );
              dec_dct_chroma( OPT_PROFILE_PASS(&t.chroma)  chroma_dcs,  prevdcs.cr, cr, cpitch, &vb, (U16*)&curdct.cdcts[0], flags, Q, &crCn );
              dec_dct_chroma( OPT_PROFILE_PASS(&t.chroma)  chroma_dcs+4,prevdcs.cb, cb, cpitch, &vb, (U16*)&curdct.cdcts[1], flags, Q, &cbCn );

              if( RAD_UNLIKELY( alpha ) )
                dec_dct_luma( OPT_PROFILE_PASS(&t.alpha)  alpha_dcs, prevdcs.a, (do_alpha)?at:0, lpitch, &vb, (U16*)curdct.adcts, dct_flags, Q, &aCn );
              if( RAD_UNLIKELY( hdr ) )
                dec_dct_luma( OPT_PROFILE_PASS(&t.hdr)  hdr_dcs, prevdcs.h, (hdr)?ht:0, lpitch, &vb, (U16*)curdct.hdcts, dct_flags, Q, &hCn );
            }

            if ( RAD_LIKELY( !key ) )
            {
              predict_down_coords( flags, cur_coords, prev_coords );
            }

            gtmLeaveAccumulationZone( tm_mask, t.dct );
            break;

          case BLK_SKIP:
            gtmEnterAccumulationZoneAndInc( tm_mask, t.skip );

            if ( RAD_LIKELY( ( flags & Y_MID_TOP_T2 ) == 0 ) )
            {
              dtmEnterAccumulationZone( tm_mask, t.luma.copy );
              move16x16( yt,              ytp,              lpitch );
              move16x16( yt+16,           ytp+16,           lpitch );
              dtmLeaveAccumulationZone( tm_mask, t.luma.copy );

              dtmEnterAccumulationZone( tm_mask, t.chroma.copy );
              move16x8( cr, crp, cpitch );
              move16x8( cb, cbp, cpitch );
              dtmLeaveAccumulationZone( tm_mask, t.chroma.copy );
            }
            else
            {
              dtmEnterAccumulationZone( tm_mask, t.luma.copy );
              move_u8_16x16( right_inter_edges, 16,  ytp+16, lpitch );
              dtmLeaveAccumulationZone( tm_mask, t.luma.copy );

              dtmEnterAccumulationZone( tm_mask, t.chroma.copy );
              move_u8_16x8( right_inter_edges+16*16*2, 16, crp, cpitch );
              move_u8_16x8( right_inter_edges+16*16*2+16*8, 16, cbp, cpitch );
              dtmLeaveAccumulationZone( tm_mask, t.chroma.copy );
            }
            if ( RAD_LIKELY( ( flags & (Y_MID_TOP_T1|Y_NO_BOTTOM_16) ) == 0 ) )
            {
              dtmEnterAccumulationZone( tm_mask, t.luma.copy );
              move16x16( yt+lpitch*16,    ytp+lpitch*16,    lpitch );
              move16x16( yt+lpitch*16+16, ytp+lpitch*16+16, lpitch );
              dtmLeaveAccumulationZone( tm_mask, t.luma.copy );
    
              dtmEnterAccumulationZone( tm_mask, t.chroma.copy );
              move16x8( cr+cp8, crp+cp8, cpitch );
              move16x8( cb+cp8, cbp+cp8, cpitch );
              dtmLeaveAccumulationZone( tm_mask, t.chroma.copy );
            }
            else 
            {
              if ( RAD_LIKELY( ( flags & Y_NO_BOTTOM_16 ) == 0 ) )
              {
                dtmEnterAccumulationZone( tm_mask, t.luma.copy );
                move_u8_16x16( right_inter_edges, 16,  ytp+lpitch*16+16, lpitch );
                dtmLeaveAccumulationZone( tm_mask, t.luma.copy );

                dtmEnterAccumulationZone( tm_mask, t.chroma.copy );
                move_u8_16x8( right_inter_edges+16*16*2, 16, crp+cp8, cpitch );
                move_u8_16x8( right_inter_edges+16*16*2+16*8, 16, cbp+cp8, cpitch );
                dtmLeaveAccumulationZone( tm_mask, t.chroma.copy );
              }
              else
              {
                dtmEnterAccumulationZone( tm_mask, t.luma.copy );
                move_u8_16x16( right_inter_edges, 16,  ytp+16, lpitch );
                dtmLeaveAccumulationZone( tm_mask, t.luma.copy );

                dtmEnterAccumulationZone( tm_mask, t.chroma.copy );
                move_u8_16x8( right_inter_edges+16*16*2, 16, crp, cpitch );
                move_u8_16x8( right_inter_edges+16*16*2+16*8, 16, cbp, cpitch );
                dtmLeaveAccumulationZone( tm_mask, t.chroma.copy );
              }
            }
            
            if ( RAD_UNLIKELY( do_alpha ) )
            {
              dtmEnterAccumulationZone( tm_mask, t.alpha.copy );
              if ( RAD_LIKELY( ( flags & Y_MID_TOP_T2 ) == 0 ) )
              {
                move16x16( at,              atp,              lpitch );
                move16x16( at+16,           atp+16,           lpitch );
              }
              else
              {
                move_u8_16x16( right_inter_edges+16*16, 16,  atp+16, lpitch );
              }
              if ( RAD_LIKELY( ( flags & (Y_MID_TOP_T1|Y_NO_BOTTOM_16) ) == 0 ) )
              {
                move16x16( at+lpitch*16,    atp+lpitch*16,    lpitch );
                move16x16( at+lpitch*16+16, atp+lpitch*16+16, lpitch );
              }
              else
              {
                if ( RAD_LIKELY( ( flags & Y_NO_BOTTOM_16 ) == 0 ) )
                {
                  move_u8_16x16( right_inter_edges+16*16, 16,  atp+lpitch*16+16, lpitch );
                }
                else
                {
                  move_u8_16x16( right_inter_edges+16*16, 16,  atp+16, lpitch );
                }
              }
              curdct.adcts[0]=curdct.adcts[1]=curdct.adcts[2]=curdct.adcts[3]=0;
              dtmLeaveAccumulationZone( tm_mask, t.alpha.copy );
            }

            if ( RAD_UNLIKELY( hdr ) )
            {
              dtmEnterAccumulationZone( tm_mask, t.hdr.copy );
              if ( RAD_LIKELY( ( flags & Y_MID_TOP_T2 ) == 0 ) )
              {
                move16x16( ht,              htp,              lpitch );
                move16x16( ht+16,           htp+16,           lpitch );
              }
              else
              {
                move_u8_16x16( right_inter_edges+16*16*2+16*8+16*8, 16,  htp+16, lpitch );
              }
              if ( RAD_LIKELY( ( flags & (Y_MID_TOP_T1|Y_NO_BOTTOM_16) ) == 0 ) )
              {
                move16x16( ht+lpitch*16,    htp+lpitch*16,    lpitch );
                move16x16( ht+lpitch*16+16, htp+lpitch*16+16, lpitch );
              }
              else
              {
                if ( RAD_LIKELY( ( flags & Y_NO_BOTTOM_16 ) == 0 ) )
                {
                  move_u8_16x16( right_inter_edges+16*16*2+16*8+16*8, 16,  htp+lpitch*16+16, lpitch );
                }
                else
                {
                  move_u8_16x16( right_inter_edges+16*16*2+16*8+16*8, 16,  htp+16, lpitch );
                }
              }
              curdct.hdcts[0]=curdct.hdcts[1]=curdct.hdcts[2]=curdct.hdcts[3]=0;
              dtmLeaveAccumulationZone( tm_mask, t.hdr.copy );
            }

            curdct.ydcts[0]=curdct.ydcts[1]=curdct.ydcts[2]=curdct.ydcts[3]=0;
            curdct.cdcts[0]=curdct.cdcts[1]=0;
    
            cur_coords[0]=0; cur_coords[1]=0; cur_coords[2]=0; cur_coords[3]=0;
            cur_coords[4]=0; cur_coords[5]=0; cur_coords[6]=0; cur_coords[7]=0;

            gtmLeaveAccumulationZone( tm_mask, t.skip );
            break;

          case BLK_MNR:
          {
            S32 coords[ 8 ];

            gtmEnterAccumulationZoneAndInc( tm_mask, t.mnr );

            dtmEnterAccumulationZone( tm_mask, t.decodemos );
            if ( RAD_LIKELY( ( flags & BINKVER22 ) ) )
              decode_mo_coords( &vb, cur_coords, prev_coords, flags );
#ifdef BINK20_SUPPORT
            else
              bink20_decode_mo_coords( &vb, cur_coords, prev_coords, flags );
#endif
            
            prepare_mos( coords, oyp, ocrp, ocbp, x, y, lpitch, cpitch, cur_coords, flags, motion_w2, motion_h2 );
            dtmLeaveAccumulationZone( tm_mask, t.decodemos );

            if ( RAD_LIKELY( ( flags & Y_MID_TOP_T2 ) == 0 ) )
            {
              dtmEnterAccumulationZone( tm_mask, t.luma.mosample );
              bink2_get_luma_modata( yt,              oyp, coords[0], coords[1], lpitch, lpitch );
              bink2_get_luma_modata( yt+16,           oyp, coords[2], coords[3], lpitch, lpitch );
              dtmLeaveAccumulationZone( tm_mask, t.luma.mosample );

              dtmEnterAccumulationZone( tm_mask, t.chroma.mosample );
              bink2_get_chroma_modata( cr,         ocrp, coords[0], coords[1], cpitch, cpitch );
              bink2_get_chroma_modata( cr + 8,     ocrp, coords[2], coords[3], cpitch, cpitch );
              bink2_get_chroma_modata( cb,         ocbp, coords[0], coords[1], cpitch, cpitch );
              bink2_get_chroma_modata( cb + 8,     ocbp, coords[2], coords[3], cpitch, cpitch );
              dtmLeaveAccumulationZone( tm_mask, t.chroma.mosample );
            }
            else
            {
              dtmEnterAccumulationZone( tm_mask, t.luma.mosample );
              bink2_get_luma_modata( right_inter_edges, oyp, coords[2], coords[3], 16, lpitch );
              dtmLeaveAccumulationZone( tm_mask, t.luma.mosample );

              dtmEnterAccumulationZone( tm_mask, t.chroma.mosample );
              bink2_get_chroma_modata( right_inter_edges+16*16*2 + 8,      ocrp, coords[2], coords[3], 16, cpitch );
              bink2_get_chroma_modata( right_inter_edges+16*16*2+16*8 + 8, ocbp, coords[2], coords[3], 16, cpitch );
              dtmLeaveAccumulationZone( tm_mask, t.chroma.mosample );
            }
            if ( RAD_LIKELY( ( flags & (Y_MID_TOP_T1|Y_NO_BOTTOM_16) ) == 0 ) )
            {
              dtmEnterAccumulationZone( tm_mask, t.luma.mosample );
              bink2_get_luma_modata( yt+lpitch*16,    oyp, coords[4], coords[5], lpitch, lpitch );
              bink2_get_luma_modata( yt+lpitch*16+16, oyp, coords[6], coords[7], lpitch, lpitch );
              dtmLeaveAccumulationZone( tm_mask, t.luma.mosample );

              dtmEnterAccumulationZone( tm_mask, t.chroma.mosample );
              bink2_get_chroma_modata( cr+cp8,     ocrp, coords[4], coords[5], cpitch, cpitch );
              bink2_get_chroma_modata( cr + 8+cp8, ocrp, coords[6], coords[7], cpitch, cpitch );
              bink2_get_chroma_modata( cb+cp8,     ocbp, coords[4], coords[5], cpitch, cpitch );
              bink2_get_chroma_modata( cb + 8+cp8, ocbp, coords[6], coords[7], cpitch, cpitch );
              dtmLeaveAccumulationZone( tm_mask, t.chroma.mosample );
            }
            else 
            {
              dtmEnterAccumulationZone( tm_mask, t.luma.mosample );
              bink2_get_luma_modata( right_inter_edges, oyp, coords[6], coords[7], 16, lpitch );
              dtmLeaveAccumulationZone( tm_mask, t.luma.mosample );

              dtmEnterAccumulationZone( tm_mask, t.chroma.mosample );
              bink2_get_chroma_modata( right_inter_edges+16*16*2 + 8,      ocrp, coords[6], coords[7], 16, cpitch );
              bink2_get_chroma_modata( right_inter_edges+16*16*2+16*8 + 8, ocbp, coords[6], coords[7], 16, cpitch );
              dtmLeaveAccumulationZone( tm_mask, t.chroma.mosample );
            }
    
            if ( RAD_UNLIKELY( do_alpha ) )
            {
              dtmEnterAccumulationZone( tm_mask, t.alpha.mosample );
              if ( RAD_LIKELY( ( flags & Y_MID_TOP_T2 ) == 0 ) )
              {
                bink2_get_luma_modata( at,              oap, coords[0], coords[1], lpitch, lpitch );
                bink2_get_luma_modata( at+16,           oap, coords[2], coords[3], lpitch, lpitch );
              }
              else
              {
                bink2_get_luma_modata( right_inter_edges+16*16, oap, coords[2], coords[3], 16, lpitch );
              }
              if ( RAD_LIKELY( ( flags & (Y_MID_TOP_T1|Y_NO_BOTTOM_16) ) == 0 ) )
              {
                bink2_get_luma_modata( at+lpitch*16,    oap, coords[4], coords[5], lpitch, lpitch );
                bink2_get_luma_modata( at+lpitch*16+16, oap, coords[6], coords[7], lpitch, lpitch );
              }
              else
              {
                bink2_get_luma_modata( right_inter_edges+16*16, oap, coords[6], coords[7], 16, lpitch );
              }
              curdct.adcts[0]=curdct.adcts[1]=curdct.adcts[2]=curdct.adcts[3]=0;
              dtmLeaveAccumulationZone( tm_mask, t.alpha.mosample );
            }

            if ( RAD_UNLIKELY( hdr ) )
            {
              dtmEnterAccumulationZone( tm_mask, t.hdr.mosample );
              if ( RAD_LIKELY( ( flags & Y_MID_TOP_T2 ) == 0 ) )
              {
                bink2_get_luma_modata( ht,              ohp, coords[0], coords[1], lpitch, lpitch );
                bink2_get_luma_modata( ht+16,           ohp, coords[2], coords[3], lpitch, lpitch );
              }
              else
              {
                bink2_get_luma_modata( right_inter_edges+16*16*2+16*8+16*8, ohp, coords[2], coords[3], 16, lpitch );
              }
              if ( RAD_LIKELY( ( flags & (Y_MID_TOP_T1|Y_NO_BOTTOM_16) ) == 0 ) )
              {
                bink2_get_luma_modata( ht+lpitch*16,    ohp, coords[4], coords[5], lpitch, lpitch );
                bink2_get_luma_modata( ht+lpitch*16+16, ohp, coords[6], coords[7], lpitch, lpitch );
              }
              else
              {
                bink2_get_luma_modata( right_inter_edges+16*16*2+16*8+16*8, ohp, coords[6], coords[7], 16, lpitch );
              }
              curdct.hdcts[0]=curdct.hdcts[1]=curdct.hdcts[2]=curdct.hdcts[3]=0;
              dtmLeaveAccumulationZone( tm_mask, t.hdr.mosample );
            }
          }

          curdct.ydcts[0]=curdct.ydcts[1]=curdct.ydcts[2]=curdct.ydcts[3]=0;
          curdct.cdcts[0]=curdct.cdcts[1]=0;
          gtmLeaveAccumulationZone( tm_mask, t.mnr );
          break;
          
          case BLK_MDCT:
          {  
            S32 coords[ 8 ];
            U32 temp;

            gtmEnterAccumulationZoneAndInc( tm_mask, t.mdct );

            dtmEnterAccumulationZone( tm_mask, t.decodemos );
#ifdef BINK20_SUPPORT
            if ( RAD_UNLIKELY( ( flags & BINKVER22 ) == 0 ) )
              bink20_decode_mo_coords( &vb, cur_coords, prev_coords, flags );
            else
#endif
              decode_mo_coords( &vb, cur_coords, prev_coords, flags );

            prepare_mos( coords, oyp, ocrp, ocbp, x, y, lpitch, cpitch, cur_coords, flags, motion_w2, motion_h2 );
            dtmLeaveAccumulationZone( tm_mask, t.decodemos );

#ifdef BINK20_SUPPORT
            if( RAD_UNLIKELY( ( flags & BINKVER22 ) == 0 ) )
            {
              bink20_dec_mdct_luma( OPT_PROFILE_PASS(&t.luma) yt, oyp, lpitch, lpitch, &vb, (U16*)curdct.ydcts, flags, coords, bink20_Qs + 4, &yCnm, right_inter_edges );
              bink20_dec_mdct_chroma( OPT_PROFILE_PASS(&t.chroma) cr, ocrp, cpitch, cpitch,&vb, (U16*)curdct.cdcts, coords, flags, bink20_Qs + 5, &crCnm, right_inter_edges+16*16*2 );
              bink20_dec_mdct_chroma( OPT_PROFILE_PASS(&t.chroma) cb, ocbp, cpitch, cpitch,&vb, (U16*)(curdct.cdcts+1), coords, flags, bink20_Qs + 6, &cbCnm, right_inter_edges+16*16*2+16*8 );

              if ( RAD_UNLIKELY( alpha == 1 ) )
              {
                bink20_dec_mdct_luma( OPT_PROFILE_PASS(&t.alpha) at, oap, lpitch, lpitch, &vb, (U16*)curdct.adcts, flags, coords, bink20_Qs + 7, &aCnm, right_inter_edges+16*16 );
              }
            }
            else
#endif
            {
              U8 Q;
              // crazy ugly - the mdct functions need the Bink2.5 flag, but not the deblock mask flags
              //    we are out of flags, so we just or out the deblock mask and add the bink2.5 flag instead
              U32 mdct_flags = (flags&~(NO_UANDD_MASKS|NO_LTOR_MASKS)) | ( ((U32*)out)[0] & BINKVER25 );

              cur_Qs[ 1 ] = Q = (U8) decode_q( &vb, flags, cur_Qs[1] DWRITECODE(,"q-len-m","q-dct-diff-m","q-dct-m" ) );

              dec_mdct_luma( OPT_PROFILE_PASS(&t.luma) yt, oyp, lpitch, lpitch, &vb, (U16*)curdct.ydcts, mdct_flags, coords, Q, &yCnm, right_inter_edges );

              if( VarBitsGet1( vb, temp ) )
              {
                dec_mdct_chroma( OPT_PROFILE_PASS(&t.chroma) cr, ocrp, cpitch, cpitch,&vb, (U16*)curdct.cdcts, coords, mdct_flags, Q, &crCnm, right_inter_edges+16*16*2 );
                dec_mdct_chroma( OPT_PROFILE_PASS(&t.chroma) cb, ocbp, cpitch, cpitch,&vb, (U16*)(curdct.cdcts+1), coords, mdct_flags, Q, &cbCnm, right_inter_edges+16*16*2+16*8 );
              }
              else
              {
                crCnm = cbCnm = 0;
                curdct.cdcts[0]=curdct.cdcts[1]=0x6000;

                dtmEnterAccumulationZone( tm_mask, t.chroma.mosample );
                if ( RAD_LIKELY( ( flags & Y_MID_TOP_T2 ) == 0 ) )
                {
                  bink2_get_chroma_modata( cr,         ocrp, coords[0], coords[1], cpitch, cpitch );
                  bink2_get_chroma_modata( cr + 8,     ocrp, coords[2], coords[3], cpitch, cpitch );
                  bink2_get_chroma_modata( cb,         ocbp, coords[0], coords[1], cpitch, cpitch );
                  bink2_get_chroma_modata( cb + 8,     ocbp, coords[2], coords[3], cpitch, cpitch );
                }
                else
                {
                  bink2_get_chroma_modata( right_inter_edges+16*16*2 + 8,      ocrp, coords[2], coords[3], 16, cpitch );
                  bink2_get_chroma_modata( right_inter_edges+16*16*2+16*8 + 8, ocbp, coords[2], coords[3], 16, cpitch );
                }
                if ( RAD_LIKELY( ( flags & (Y_MID_TOP_T1|Y_NO_BOTTOM_16) ) == 0 ) )
                {
                  bink2_get_chroma_modata( cr+cp8,     ocrp, coords[4], coords[5], cpitch, cpitch );
                  bink2_get_chroma_modata( cr + 8+cp8, ocrp, coords[6], coords[7], cpitch, cpitch );
                  bink2_get_chroma_modata( cb+cp8,     ocbp, coords[4], coords[5], cpitch, cpitch );
                  bink2_get_chroma_modata( cb + 8+cp8, ocbp, coords[6], coords[7], cpitch, cpitch );
                }
                else 
                {
                  bink2_get_chroma_modata( right_inter_edges+16*16*2 + 8,      ocrp, coords[6], coords[7], 16, cpitch );
                  bink2_get_chroma_modata( right_inter_edges+16*16*2+16*8 + 8, ocbp, coords[6], coords[7], 16, cpitch );
                }
                dtmLeaveAccumulationZone( tm_mask, t.chroma.mosample );
              }

              if ( RAD_UNLIKELY( alpha ) )
              {
                dec_mdct_luma( OPT_PROFILE_PASS(&t.alpha)  at, oap, lpitch, lpitch, &vb, (U16*)curdct.adcts, mdct_flags, coords, Q, &aCnm, right_inter_edges+16*16 );
                curdct.adcts[0]=curdct.adcts[1]=curdct.adcts[2]=curdct.adcts[3]=0;
              }

              if ( RAD_UNLIKELY( hdr ) )
              {
                dec_mdct_luma( OPT_PROFILE_PASS(&t.hdr)  ht, ohp, lpitch, lpitch, &vb, (U16*)curdct.hdcts, mdct_flags, coords, Q, &hCnm, right_inter_edges+16*16*2+16*8+16*8 );
                curdct.hdcts[0]=curdct.hdcts[1]=curdct.hdcts[2]=curdct.hdcts[3]=0;
              }
            }
          }
          curdct.ydcts[0]=curdct.ydcts[1]=curdct.ydcts[2]=curdct.ydcts[3]=0;
          curdct.cdcts[0]=curdct.cdcts[1]=0;

          gtmLeaveAccumulationZone( tm_mask, t.mdct );
          break;
        }

        // lotta action here
        gtmEnterAccumulationZone( tm_mask, t.deblock );
        deblock( OPT_PROFILE_PASS( &t ) &dcs_locals, ydc, adc, hdc, cdc, &curdct, flags, yt, (do_alpha)?at:0, (hdr)?ht:0, cr, cb, ydcpitch, cdcpitch, lpitch, cpitch, cur_coords, prev_coords, threshold, seam ); 
        gtmLeaveAccumulationZone( tm_mask, t.deblock );        

        yt += 32;
        cr += 16;
        cb += 16;
        at += 32;
        ht += 32;

        ytp += 32;
        crp += 16;
        cbp += 16;
        atp += 32;
        htp += 32;

        cur_coords += 8;
        prev_coords += 8;
        cur_Qs += 2;
        prev_Qs += 2;
        x += 32;
        if ( x >= lwidth )
          break;

        // predict Qs
        if ( RAD_LIKELY( ( flags & BINKVER22 ) ) )
        {
          if( RAD_UNLIKELY( flags & (Y_TOP|Y_MID_TOP) ) )
          {
            // predict from left
            cur_Qs[0] = cur_Qs[0-2];
            cur_Qs[1] = cur_Qs[1-2];
          }
          else
          {
            // median predict
            U8 dctQ = (U8)med3( prev_Qs[0-2], prev_Qs[0], cur_Qs[0-2] );
            U8 mdctQ = (U8)med3( prev_Qs[1-2], prev_Qs[1], cur_Qs[1-2] );
            cur_Qs[0] = dctQ;
            cur_Qs[1] = mdctQ;
          }
        }
        
        ydc += 4;
        adc += 4;
        hdc += 4;
        cdc += 2;
      }

      if ( cur_coords == row_coords1 )
      {
        prev_coords = row_coords0;
        //cur_coords = row_coords1;//already set to this because of wrapping
        prev_Qs = row_Qs0;
        //cur_Qs = row_Qs1; // already set to this because of wrapping
      }
      else
      {
        prev_coords = row_coords1;
        cur_coords = row_coords0;
        prev_Qs = row_Qs1;
        cur_Qs = row_Qs0;
      }

      y += 32;

      gtmEnterAccumulationZone( tm_mask, t.prefetch );

      #ifdef MemoryCommit
      lCommitX=0;
      {
        U8 * ptr;

        dtmEnterAccumulationZone( tm_mask, t.luma.commit );
        for( ptr = yt - ( x - lCommitX ) ; ptr < yt ; ptr += RR_CACHE_LINE_SIZE )
          MemoryCommitMultiple_x2( ptr - lCommitSub, lpitch, lCommitLines );
        dtmLeaveAccumulationZone( tm_mask, t.luma.commit );

        if ( RAD_UNLIKELY( do_alpha ) )
        {
          dtmEnterAccumulationZone( tm_mask, t.alpha.commit );
          for( ptr = at - ( x - lCommitX ) ; ptr < at ; ptr += RR_CACHE_LINE_SIZE )
            MemoryCommitMultiple_x2( ptr - lCommitSub, lpitch, lCommitLines );
          dtmLeaveAccumulationZone( tm_mask, t.alpha.commit );
        }
        if ( RAD_UNLIKELY( hdr ) )
        {
          dtmEnterAccumulationZone( tm_mask, t.hdr.commit );
          for( ptr = ht - ( x - lCommitX ) ; ptr < ht ; ptr += RR_CACHE_LINE_SIZE )
            MemoryCommitMultiple_x2( ptr - lCommitSub, lpitch, lCommitLines );
          dtmLeaveAccumulationZone( tm_mask, t.hdr.commit );
        }
      }

      dtmEnterAccumulationZone( tm_mask, t.chroma.commit );
      {
        U8 * crp, * cbp;

        crp = cr - ( (x/2) - cCommitX );
        cbp = cb - ( (x/2) - cCommitX );
        while ( crp < cr )
        {
          MemoryCommitMultiple_x2( crp - cCommitSub, cpitch, cCommitLines );
          MemoryCommitMultiple_x2( cbp - cCommitSub, cpitch, cCommitLines );

          crp += RR_CACHE_LINE_SIZE;
          cbp += RR_CACHE_LINE_SIZE;
        }
      }
      dtmLeaveAccumulationZone( tm_mask, t.chroma.commit );
     
      #endif

      gtmLeaveAccumulationZone( tm_mask, t.prefetch );

      yt += yadj;
      cr += cadj;
      cb += cadj;
      at += yadj;
      ht += yadj;

      ytp += yadj;
      crp += cadj;
      cbp += cadj;
      atp += yadj;
      htp += yadj;

      // update yflags:
      // clear all the various row flags
      // if this row was Y_MID_TOP_T2, next one will be Y_MID_TOP_T2_NEXT.
      yflags = ( yflags & ~(Y_TOP|Y_MID_TOP|Y_MID_TOP_T1|Y_NO_BOTTOM_16|Y_MID_TOP_T2|Y_MID_TOP_T2_NEXT|Y_BOTTOM) ) | ( ( yflags & Y_MID_TOP_T2 ) ? Y_MID_TOP_T2_NEXT : 0 );
    }

    yflags = 0; // clear yflags before next slice

    ++startslice;
    h = slices->slices[ startslice ];  // get stop slice end
    seam = (U8*) seam + slices->slice_pitch;  // advance to the next seam

    if ( RAD_LIKELY( numslices <= 1 ) )
    {
      // is the whole frame done? if so exit
      if ( ( numslices == 0 ) || ( y >= ( (b->YABufferHeight+16)&~31 ) ) )
        break;

      // if we fall through to here, we are now doing the top 16 pixels of the next slice
      h = y + 32;
      yflags = Y_MID_TOP_T1;
    }

    --numslices;

    // align the varbits to the next seam
    if ( RAD_LIKELY( vb.bitlen&31) )
    {
      vb.bitlen=0;  // align varbits
      vb.bits=0;

    }
//{ U32 v; VarBitsLocalGet( v,U32,vb,8 ); if (v!=247) RR_BREAK(); }
  }

  ret = 1;

// done:

  #ifndef NTELEMETRY
    if ( RAD_UNLIKELY( tmRunning() ) )
    {
      emitzones( &t, &start, do_alpha, hdr );
    }
  #endif

  #ifndef BINK20_SUPPORT
  ret:
#endif  
  #ifndef NTELEMETRY
  tmLeave( tm_mask );
  #endif
  return ret;
}

#ifdef SUPPORT_BINKGPU

#define PLANE_Y     BINKGPUPLANE_Y 
#define PLANE_CR    BINKGPUPLANE_CR
#define PLANE_CB    BINKGPUPLANE_CB
#define PLANE_A     BINKGPUPLANE_A
#define PLANE_H     BINKGPUPLANE_H

static void gpu_movec( BINKGPUMOVEC * mv_out, S32 const * rel_coords )
{
  U32 i;
  for ( i = 0 ; i < 4 ; i++ )
  {
    mv_out[i].rel_x = (S16)rel_coords[ i*2 + 0 ];
    mv_out[i].rel_y = (S16)rel_coords[ i*2 + 1 ];
  }
}

static void gpu_dec_dct_luma( PLANEPTRS * RADRESTRICT plane, U32 block_id, VARBITSEND * vb, U32 frameflags, S32 Q, U32 * RADRESTRICT cnp )
{
  RAD_ALIGN( S32, dctemp[ 16 ], COEFF_ALIGN );
  U32 control;

  cnp[0] = control = dec_4_controls( vb, cnp[0], frameflags );
  bit_decode_dcs16( dctemp, Q, vb );
  gpu_dec_coeffs( plane, block_id, control, 16, vb, bink_zigzag_scan_order_transposed, Q, bink_intra_luma_scaleT, dctemp, DC_INTRA );
}

static void gpu_dec_dct_chroma( PLANEPTRS * RADRESTRICT plane, U32 block_id, VARBITSEND * vb, S32 Q, U32 * RADRESTRICT cnp )
{
  RAD_ALIGN( S32, dctemp[ 4 ], COEFF_ALIGN );
  U32 control;

  cnp[0] = control = dec_1_control( vb, cnp[0] );
  bit_decode_dcs4( dctemp, Q, vb );
  gpu_dec_coeffs( plane, block_id, control, 4, vb, bink_zigzag_scan_order_transposed, Q, bink_intra_chroma_scaleT, dctemp, DC_INTRA );
}

static void gpu_dec_mdct_luma( PLANEPTRS * RADRESTRICT plane, U32 block_id, VARBITSEND * vb, U32 frameflags, S32 Q, U32 * RADRESTRICT cnp )
{
  RAD_ALIGN( S32, dctemp[ 16 ], COEFF_ALIGN );
  U32 control;

  cnp[0] = control = dec_4_controls( vb, cnp[0], frameflags );
  decode_dcs16( dctemp, 0, Q, vb, X_LEFT|Y_TOP|Y_MID_TOP|BINKVER22, (frameflags&BINKVER25)?MININTER25:OLDMININTER, (frameflags&BINKVER25)?MAXINTER25:OLDMAXINTER );
  gpu_dec_coeffs( plane, block_id, control, 16, vb, bink_zigzag_scan_order_transposed, Q, bink_inter_scaleT, dctemp, DC_INTER );
}

static void gpu_dec_mdct_chroma( PLANEPTRS * RADRESTRICT plane, U32 block_id, VARBITSEND * vb, U32 frameflags, S32 Q, U32 * RADRESTRICT cnp )
{
  RAD_ALIGN( S32, dctemp[ 4 ], COEFF_ALIGN );
  U32 control;

  cnp[0] = control = dec_1_control( vb, cnp[0] );
  decode_dcs4( dctemp, 0, Q, vb, X_LEFT|Y_TOP|Y_MID_TOP|BINKVER22, (frameflags&BINKVER25)?MININTER25:OLDMININTER, (frameflags&BINKVER25)?MAXINTER25:OLDMAXINTER );
  gpu_dec_coeffs( plane, block_id, control, 4, vb, bink_zigzag_scan_order_transposed, Q, bink_inter_scaleT, dctemp, DC_INTER );
}

static U32 doplanes_gpu( BINKGPUDATABUFFERS * RADRESTRICT gpu, void * out, S32 alpha, S32 hdr, S32 key, void * out_end, BINKSLICES * slices, U32 startslice, U32 numslices )
{
  U32 frameflags = 0;
  S32 * row_coords0, * row_coords1;
  S32 * prev_coords, * cur_coords;
  U8 * row_Qs0, * row_Qs1;
  U8 * prev_Qs, * cur_Qs;
  U32 xb, y, starty, endy;
  U32 wd32, hd32, num_blocks32;
  U32 i;
  U32 ret = 0;
  VARBITSEND vb;

  frameflags = ((U32*)out)[0];
  vb.end = out_end;
  wd32 = gpu->BufferWidth / 32;
  hd32 = ( gpu->BufferHeight + 31 ) / 32;
  num_blocks32 = wd32 * hd32;
  
  if ( RAD_UNLIKELY( gpu->Motion == 0 ) )
    goto ret;

  if ( RAD_UNLIKELY( ( frameflags & BINKVER22 ) == 0 ) )
    goto ret;

  if ( frameflags & BINKCONSTANTA )
  {
    alpha = 0;  // if it's constant alpha, we don't decode it at all
  }

  // alloc row buffers for Q and coords
  {
    i = wd32 * 8 * sizeof( S32 );
    row_coords0 = (S32*) radalloca( 2 * i );
    row_coords1 = (S32*)( ( (U8*)row_coords0 ) + i );
    prev_coords = row_coords1;
    cur_coords = row_coords0;

    i = wd32 * 2 * sizeof( U8 ); // 2 values per 32x32: DCT and MDCT Q
    row_Qs0 = (U8*) radalloca( 2 * i ); // 2 rows
    row_Qs1 = row_Qs0 + i;
    prev_Qs = row_Qs1;
    cur_Qs = row_Qs0;
  }

  starty = endy = 0;
  if ( startslice == 0 )
  {
    U32 motion_w2, motion_h2;

    starty = 0;
    endy = slices->slices[ 0 ];

    // first slice also handles everything shared between all slices
    gpu->DcThreshold = ( frameflags & 255 ) << 3;
    gpu->Flags = 0;
    if ( frameflags & BINKVER23EDGEMOFIX )
      gpu->Flags |= BINKGPUNEWCLAMP;

    motion_w2 = gpu->BufferWidth * 2;
    motion_h2 = gpu->BufferHeight * 2;
    if ( gpu->Flags & BINKGPUNEWCLAMP )
    {
      // 0=fullpel, 1=halfpel
      for ( i = 0 ; i < 2 ; i++ )
      {
        S32 tl = bink2_motion_new_tl[ i ];
        S32 wh = bink2_motion_wh[ i ];
        gpu->MotionXClamp[ i*2 + 0 ] = tl; // left
        gpu->MotionXClamp[ i*2 + 1 ] = motion_w2 - wh + tl; // right
        gpu->MotionYClamp[ i*2 + 0 ] = tl; // top
        gpu->MotionYClamp[ i*2 + 1 ] = motion_h2 - wh + tl; // bottom
      }
    }
    else
    {
      // 0=fullpel, 1=halfpel
      for ( i = 0 ; i < 2 ; i++ )
      {
        S32 tl = bink2_motion_old_tl[ i ];
        S32 wh = bink2_motion_wh[ i ];
        gpu->MotionXClamp[ i*2 + 0 ] = tl; // left
        gpu->MotionXClamp[ i*2 + 1 ] = motion_w2 - wh; // width
        gpu->MotionYClamp[ i*2 + 0 ] = tl; // top
        gpu->MotionYClamp[ i*2 + 1 ] = motion_h2 - wh; // height
      }
    }

    // deblock bits
    VarBitsOpen( vb, (((U8*)out + 4*RR_MAX(slices->slice_count,2))) );
    if ( ( frameflags & BINK_DEBLOCK_LTOR_OFF ) == 0 )
    {
      if ( ( frameflags & BINKVER23BLURBITS ) != 0 )
        rle_bit_decode( gpu->DeblockLrFlag, 0, &vb, gpu->BufferHeight/8 - 1 );
      else
        rrmemset( gpu->DeblockLrFlag, 0, ( (gpu->BufferHeight / 8) + 7) / 8 );
    }

    if ( ( frameflags & BINK_DEBLOCK_UANDD_OFF ) == 0 )
    {
      if ( ( frameflags & BINKVER23BLURBITS ) != 0 )
        rle_bit_decode( gpu->DeblockUdFlag, 0, &vb, gpu->BufferWidth/8 - 1 );
      else
        rrmemset( gpu->DeblockUdFlag, 0, ( (gpu->BufferWidth / 8) + 7) / 8 );
    }

    // this needs to happen after the rle_bit_decode for the block flags
    frameflags = debug_update_frame_flags( frameflags );

    // now set GPU deblock flags
    if ( ( frameflags & BINK_DEBLOCK_LTOR_OFF ) == 0 )
      gpu->Flags |= BINKGPUDEBLOCKLR;
    if ( ( frameflags & BINK_DEBLOCK_UANDD_OFF ) == 0 )
      gpu->Flags |= BINKGPUDEBLOCKUD;

    // remember if this is a 2.5 bink frame
    if ( frameflags & BINKVER25 )
      gpu->Flags |= BINKGPUBINK25;

    // remember constant alpha if set
    if ( frameflags & BINKCONSTANTA )
    {
      gpu->Flags |= BINKGPUCONSTANTA;
      gpu->FillAlphaValue = frameflags>>24;
    }

    // slice info
    rrmemset( gpu->TopFlag, 0, ( hd32 + 7 ) / 8 );
    gpu->TopFlag[ 0 ] = 1; // y=0 is always a slice start
    for ( i = 0 ; i < slices->slice_count ; i++ )
    {
      U32 slice_block_y = slices->slices[ i ] / 32;
      gpu->TopFlag[ slice_block_y / 8 ] |= (U8) ( 1 << ( slice_block_y % 8 ) );
    }
  }
  else
  {
    starty = slices->slices[ startslice-1 ];
    endy = slices->slices[ startslice ];
    i = ((U32*)out)[startslice];
    VarBitsOpen( vb, ((U8*)out + i) );
  }

  // loop over slices
  do
  {
    PLANEPTRS planes[ BINKMAXPLANES ];
    U32 block_id = ( starty / 32 ) * wd32;
    U32 end_block_id = ( ( endy + 31 ) / 32 ) * wd32;
    U32 blocks_in_slice = end_block_id - block_id;

    // init planes for slice
    for ( i = 0 ; i < BINKMAXPLANES ; i++ )
    {
      PLANEPTRS * plane = &planes[ i ];
      U32 num8x8s = ( i == PLANE_CR || i == PLANE_CB ) ? 4 : 16;
      U32 max_ac_bytes_per_mb = sizeof( U32 ) + // "which blocks have ACs" flag
        num8x8s * (
                    sizeof( U64 ) +     // AC nonzero mask
                    64 * sizeof( S16 )  // AC coeffs
                  );

      plane->Dc = gpu->Dc[ i ];
      if ( !plane->Dc )
        continue;

      plane->AcStart = (U8 *)gpu->Ac[ i ];
      plane->AcCur = plane->AcStart + num_blocks32 * sizeof( U32 ) /* offs table */ + block_id * max_ac_bytes_per_mb /* ac coeffs */;
      plane->AcOffs = (U32 *)plane->AcStart;

      if ( startslice == 0 ) // slice 0 "owns" offset table
        gpu->AcRanges[ i ][ startslice ].Offset = 0;
      else
        gpu->AcRanges[ i ][ startslice ].Offset = (U32) (UINTa) ( plane->AcCur - plane->AcStart );

      // clear GPU DCs, AC offsets for this slice
      rrmemset( plane->Dc + block_id * num8x8s, 0, blocks_in_slice * num8x8s * sizeof( U16 ) );
      rrmemset( plane->AcOffs + block_id, 0, blocks_in_slice * sizeof( U32 ) );
    }

    // clear motion vectors for this slice
    rrmemset( gpu->Motion + block_id * 4, 0, blocks_in_slice * 4 * sizeof( BINKGPUMOVEC ) );

    // loop over block rows
    for ( y = starty ; y < endy ; y += 32 )
    {
      U32 yCn, aCn, hCn, crCn, cbCn;
      U32 yCnm, aCnm, hCnm, crCnm, cbCnm;
      U32 yflags;
      U8 last_type[4];

      // initial control predictions
      yCn = aCn = hCn = yCnm = aCnm = hCnm = INITIAL_CONTROL_4_PREDICT;
      crCn = cbCn = crCnm = cbCnm = INITIAL_CONTROL_1_PREDICT;

      last_type[0] = BLK_MNR; last_type[1] = BLK_MDCT; last_type[2] = BLK_SKIP; last_type[3] = BLK_DCT;
      yflags = BINKVER22;

      // Initial Q prediction for this row
      if ( RAD_UNLIKELY( y == starty ) )
      {
        yflags |= Y_MID_TOP; // only need this for motion predict, it's OK if it's not very precise.
        cur_Qs[0] = INITIAL_Q_PREDICT;
        cur_Qs[1] = INITIAL_Q_PREDICT;
      }
      else
      {
        // predict from top
        cur_Qs[0] = prev_Qs[0];
        cur_Qs[1] = prev_Qs[1];
      }

      xb = 0;
      for (;; )
      {
        U32 flags = yflags;
        U32 type = BLK_DCT;

        if ( xb == 0 ) flags |= X_LEFT;

        if ( key )
          goto dct;

        type = dec_block_type( &vb, last_type );
        switch ( type )
        {
        case BLK_DCT:
        dct:
          {
            U8 Q;
            cur_Qs[ xb*2 + 0 ] = Q = (U8) decode_q( &vb, flags, cur_Qs[ xb*2 + 0] DWRITECODE(,"q-len","q-dct-diff","q-dct" ) );

            gpu_dec_dct_luma  ( &planes[PLANE_Y],  block_id, &vb, frameflags, Q, &yCn );
            gpu_dec_dct_chroma( &planes[PLANE_CR], block_id, &vb, Q, &crCn );
            gpu_dec_dct_chroma( &planes[PLANE_CB], block_id, &vb, Q, &cbCn );
            if ( RAD_UNLIKELY( alpha != 0 ) )
              gpu_dec_dct_luma( &planes[PLANE_A],  block_id, &vb, frameflags, Q, &aCn );
            if ( RAD_UNLIKELY( hdr != 0 ) )
              gpu_dec_dct_luma( &planes[PLANE_H],  block_id, &vb, frameflags, Q, &hCn );
          }
          if ( RAD_LIKELY( !key ) )
            predict_down_coords( flags, cur_coords + xb*8, prev_coords + xb*8 );
          break;

        case BLK_SKIP:
          {
            S32 * coords = cur_coords + xb*8;
            coords[0]=0; coords[1]=0; coords[2]=0; coords[3]=0;
            coords[4]=0; coords[5]=0; coords[6]=0; coords[7]=0;
          }
          break;

        case BLK_MNR:
          decode_mo_coords( &vb, cur_coords + xb*8, prev_coords + xb*8, flags );
          gpu_movec( gpu->Motion + block_id*4, cur_coords + xb*8 );
          break;

        case BLK_MDCT:
          decode_mo_coords( &vb, cur_coords + xb*8, prev_coords + xb*8, flags );
          gpu_movec( gpu->Motion + block_id*4, cur_coords + xb*8 );
          {
            U8 Q;
            cur_Qs[ xb*2 + 1 ] = Q = (U8) decode_q( &vb, flags, cur_Qs[ xb*2 + 1 ] DWRITECODE(,"q-len-m","q-dct-diff-m","q-dct-m" ) );

            gpu_dec_mdct_luma( &planes[PLANE_Y], block_id, &vb, frameflags, Q, &yCnm );
            if ( VarBitsGet1( vb, i ) )
            {
              gpu_dec_mdct_chroma( &planes[PLANE_CR], block_id, &vb, frameflags, Q, &crCnm );
              gpu_dec_mdct_chroma( &planes[PLANE_CB], block_id, &vb, frameflags, Q, &cbCnm );
            }
            else
            {
              crCnm = cbCnm = 0;
            }
            if ( RAD_UNLIKELY( alpha != 0 ) )
              gpu_dec_mdct_luma( &planes[PLANE_A], block_id, &vb, frameflags, Q, &aCnm );
            if ( RAD_UNLIKELY( hdr != 0 ) )
              gpu_dec_mdct_luma( &planes[PLANE_H], block_id, &vb, frameflags, Q, &hCnm );
          }
          break;
        }

        ++block_id;
        ++xb;
        if ( xb >= wd32 )
          break;

        // predict Qs
        {
          U32 xb2 = xb*2;
          if( RAD_UNLIKELY( flags & (Y_TOP|Y_MID_TOP) ) )
          {
            // predict from left
            cur_Qs[xb2 + 0] = cur_Qs[ xb2-2 + 0 ];
            cur_Qs[xb2 + 1] = cur_Qs[ xb2-2 + 1 ];
          }
          else
          {
            // median predict
            U8 dctQ = (U8)med3( prev_Qs[xb2-2+0], prev_Qs[xb2+0], cur_Qs[xb2-2+0] );
            U8 mdctQ = (U8)med3( prev_Qs[xb2-2+1], prev_Qs[xb2+1], cur_Qs[xb2-2+1] );
            cur_Qs[xb2 + 0] = dctQ;
            cur_Qs[xb2 + 1] = mdctQ;
          }
        }
      }

      // toggle set of coords we use for prediction
      // NB unlike regular decoder, we don't modify the pointers during main loop,
      // so the logic comes out a bit differently!
      if ( cur_coords == row_coords0 )
      {
        cur_coords = row_coords1;
        prev_coords = row_coords0;
        cur_Qs = row_Qs1;
        prev_Qs = row_Qs0;
      }
      else
      {
        cur_coords = row_coords0;
        prev_coords = row_coords1;
        cur_Qs = row_Qs0;
        prev_Qs = row_Qs1;
      }
    }

    // write AC range for this slice to GPU buffer
    for ( i = 0 ; i < BINKMAXPLANES ; i++ )
    {
      PLANEPTRS * plane = &planes[ i ];
      U32 end_offs;

      if ( !gpu->Dc[ i ] )
        continue;

      end_offs = (U32) (UINTa) ( plane->AcCur - plane->AcStart );
      gpu->AcRanges[ i ][ startslice ].Size = end_offs - gpu->AcRanges[ i ][ startslice ].Offset;
    }

    // next slice
    ++startslice;
    starty = endy;
    endy = slices->slices[ startslice ];
    if ( RAD_LIKELY( vb.bitlen&31 ) )
    {
      vb.bitlen = 0; // align varbits
      vb.bits = 0;
    }
  } while ( --numslices );

  gpu->HasDataBeenDecoded = 1;
  ret = 1;

ret:
  return ret;
}

#endif // SUPPORT_BINKGPU

RADDEFFUNC void RADLINK ExpandBink2GlobalInit( void )
{
#if defined(__RADX86__) && !defined(RAD_GUARANTEED_SSE3) && defined(RAD_USES_SSSE3)
  // NOTE(fg): this code is *not* run under a lock - you need to write
  // it so it's safe to run it concurrently on multiple cores at the
  // same time.
  //
  // This boils down to "write every value only once".
  S32 i;

  // muratori32_16x16
  {
    sse2_muratori32_16x16_jump * * src = sse2_muratori32_16x16;
    if( CPU_can_use( CPU_SSSE3 ) )
      src = ssse3_muratori32_16x16;

    for( i = 0 ; i < 4 ; i++ )
      muratori32_16x16[ i ] = src[ i ];
  }
#endif
}

RADDEFFUNC void RADLINK ExpandBink2SplitFinish( BINKFRAMEBUFFERS * b, BINKSLICES * slices, S32 alpha, S32 hdr, S32 key, void * seamp )
{
  U32 h = (b->YABufferHeight+16)&~31;
  U32 s;
  U32 * sl = slices->slices;

  // if we aren't decompressing an alpha plane, don't blur it
  if ( b->Frames[0].APlane.Buffer == 0 )
    alpha = 0;

  for( s = 0 ; s < ( slices->slice_count - 1 ) ; s++ )
  {
    U32 width, pitch, wd32;
    S32 oldf = b->FrameNum;
    S32 newf = oldf + 1;
    if ( newf >= b->TotalFrames )
      newf = 0;

    width = b->YABufferWidth;
    wd32 = width / 32;
    pitch=b->Frames[0].YPlane.BufferPitch;

    if ( ( seamp ) && ( ((U32*)seamp)[ 0 ] ) )
    {
      U32 i;
      U8 * RADRESTRICT p;
      U8 * RADRESTRICT p2;
      U8 * RADRESTRICT to_prefetch;
      U8 * RADRESTRICT to_prefetch2;
      #ifdef MemoryCommit
      U8 * RADRESTRICT to_commit;
      U8 * RADRESTRICT to_commit2;
      #endif
      U8 * RADRESTRICT seam = (U8*)seamp;

      rrassert( ((U32*)seamp)[0] == (slices->slice_pitch-4) );

      ((U32*)seamp)[0]=0;

      seam += 4; // skip the count

      p = (U8*) b->Frames[newf].YPlane.Buffer;

      h = sl[0];

      // deblock and send back out to memory Y 
      p += ( pitch * ( h + 14 ) );
      to_prefetch = (U8*) ( ( (UINTa) ( p + RR_CACHE_LINE_SIZE + RR_CACHE_LINE_SIZE - 1 ) ) & ~(RR_CACHE_LINE_SIZE-1) );

      #ifdef MemoryCommit
        to_commit = (U8*) ( ( (UINTa) p ) & ~(RR_CACHE_LINE_SIZE-1) );
      #endif

      for( i = 0 ; i < wd32 ; i++ )
      {
        #ifdef PrefetchForRead
        if ( ((U32)( to_prefetch - p )) <= RR_CACHE_LINE_SIZE )
        {
          PrefetchForRead( to_prefetch );
          PrefetchForRead( to_prefetch + pitch );
          PrefetchForRead( to_prefetch + pitch + pitch );
          PrefetchForRead( to_prefetch + pitch + pitch + pitch );
          to_prefetch += RR_CACHE_LINE_SIZE;
        }
        #endif
        mublock_LtoR( *seam++, p, pitch );
        mublock_LtoR( *seam++, p+16, pitch );
        p += 32;
        #ifdef MemoryCommit
          if ( ((U32)( p - to_commit )) >= RR_CACHE_LINE_SIZE )
          {
            MemoryCommit( to_commit );
            MemoryCommit( to_commit + pitch );
            MemoryCommit( to_commit + pitch + pitch );
            MemoryCommit( to_commit + pitch + pitch + pitch );
            to_commit += RR_CACHE_LINE_SIZE;
          }
        #endif
      }

      #ifdef MemoryCommit
        if ( p > to_commit )
        {
          MemoryCommit( to_commit );
          MemoryCommit( to_commit + pitch );
          MemoryCommit( to_commit + pitch + pitch );
          MemoryCommit( to_commit + pitch + pitch + pitch );
        }
      #endif


      // deblock and send back out to memory Y 
      pitch=b->Frames[0].cRPlane.BufferPitch;

      p = (U8*) b->Frames[newf].cRPlane.Buffer;
      p2 = (U8*) b->Frames[newf].cBPlane.Buffer;

      i = ( pitch * ( (h>>1) + 6 ) );
      p += i;
      p2 += i;

      to_prefetch = (U8*) ( ( (UINTa) ( p + RR_CACHE_LINE_SIZE + RR_CACHE_LINE_SIZE - 1 ) ) & ~(RR_CACHE_LINE_SIZE-1) );
      to_prefetch2 = (U8*) ( ( (UINTa) ( p2 + RR_CACHE_LINE_SIZE + RR_CACHE_LINE_SIZE - 1 ) ) & ~(RR_CACHE_LINE_SIZE-1) );

      #ifdef MemoryCommit
        to_commit = (U8*) ( ( (UINTa) p ) & ~(RR_CACHE_LINE_SIZE-1) );
        to_commit2 = (U8*) ( ( (UINTa) p2 ) & ~(RR_CACHE_LINE_SIZE-1) );
      #endif

      for( i = 0 ; i < wd32 ; i++ )
      {
        #ifdef PrefetchForRead
        if ( ((U32)( to_prefetch - p )) <= RR_CACHE_LINE_SIZE )
        {
          PrefetchForRead( to_prefetch );
          PrefetchForRead( to_prefetch + pitch );
          PrefetchForRead( to_prefetch + pitch + pitch );
          PrefetchForRead( to_prefetch + pitch + pitch + pitch );
          to_prefetch += RR_CACHE_LINE_SIZE;
        }
        if ( ((U32)( to_prefetch2 - p2 )) <= RR_CACHE_LINE_SIZE )
        {
          PrefetchForRead( to_prefetch2 );
          PrefetchForRead( to_prefetch2 + pitch );
          PrefetchForRead( to_prefetch2 + pitch + pitch );
          PrefetchForRead( to_prefetch2 + pitch + pitch + pitch );
          to_prefetch2 += RR_CACHE_LINE_SIZE;
        }
        #endif

        mublock_LtoR( *seam++, p, pitch );
        mublock_LtoR( *seam++, p2, pitch );

        p += 16;
        p2 += 16;

        #ifdef MemoryCommit
        if ( ((U32)( p - to_commit )) >= RR_CACHE_LINE_SIZE )
        {
          MemoryCommit( to_commit );
          MemoryCommit( to_commit + pitch );
          MemoryCommit( to_commit + pitch + pitch );
          MemoryCommit( to_commit + pitch + pitch + pitch );
          to_commit += RR_CACHE_LINE_SIZE;
        }
        if ( ((U32)( p2 - to_commit2 )) >= RR_CACHE_LINE_SIZE )
        {
          MemoryCommit( to_commit2 );
          MemoryCommit( to_commit2 + pitch );
          MemoryCommit( to_commit2 + pitch + pitch );
          MemoryCommit( to_commit2 + pitch + pitch + pitch );
          to_commit2 += RR_CACHE_LINE_SIZE;
        }
        #endif
      }

      #ifdef MemoryCommit
        if ( p > to_commit )
        {
          MemoryCommit( to_commit );
          MemoryCommit( to_commit + pitch );
          MemoryCommit( to_commit + pitch + pitch );
          MemoryCommit( to_commit + pitch + pitch + pitch );
        }
        if ( p2 > to_commit )
        {
          MemoryCommit( to_commit2 );
          MemoryCommit( to_commit2 + pitch );
          MemoryCommit( to_commit2 + pitch + pitch );
          MemoryCommit( to_commit2 + pitch + pitch + pitch );
        }
      #endif
    }

    seamp = ((U8*) seamp ) + slices->slice_pitch;
    ++sl;
  }
}

RADDEFFUNC U32 RADLINK ExpandBink2( BINKFRAMEBUFFERS * b, void * out, S32 alpha, S32 hdr, S32 key, void * out_end, BINKSLICES * slices, U32 control, void * seam, BINKGPUDATABUFFERS * gpu )
{ 
  U32 ret;

#if defined(BINK20_SUPPORT) && defined(__RADX86__)
  U32 orig_mxcsr = 0;
  U32 restore_mxcsr = 0;
#endif

  CPU_check(0,0);
  
  #if defined(BINK20_SUPPORT) && defined(__RADX86__)
    if ( ( ((U32*)out)[0] & BINKVER22 ) == 0 )
    {
      orig_mxcsr = _mm_getcsr();
      restore_mxcsr = 1;

      _MM_SET_ROUNDING_MODE(_MM_ROUND_NEAREST);
      _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
      if ( CPU_can_use( CPU_SSE3 ) )
      {
        #define r_MM_SET_DENORMALS_ZERO_MODE(x) (_mm_setcsr((_mm_getcsr() & ~r_MM_DENORMALS_ZERO_MASK) | (x)))
        #define r_MM_DENORMALS_ZERO_ON   (0x0040)
        #define r_MM_DENORMALS_ZERO_MASK   0x0040
        r_MM_SET_DENORMALS_ZERO_MODE(r_MM_DENORMALS_ZERO_ON);
      }
    }
  #endif

  if ( !gpu )
    ret = doplanes( b, out, alpha, hdr, key, out_end, slices, control&15, (control>>4)&15, seam );
  else
  {
  #ifdef SUPPORT_BINKGPU
    ret = doplanes_gpu( gpu, out, alpha, hdr, key, out_end, slices, control&15, (control>>4)&15 );
  #else
    ret = 0;
  #endif
  }
  
  #if defined(BINK20_SUPPORT) && defined(__RADX86__)
    if ( restore_mxcsr )
      _mm_setcsr( orig_mxcsr );
  #endif

  if ( ret == 0 )
    return 0;
 
  return 1;
}

#else
typedef int t;
#endif
