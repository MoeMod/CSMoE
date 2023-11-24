// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

static CODEGEN_ATTR void mublock_UandD( U16 const * mask, U8 * RADRESTRICT ptr, U32 pitch )
{
  __m128i r0, r1;
  __m128i masks;

  // load rows
  {
    __m128i p0,p1,p2,p3;
    U8 const * RADRESTRICT inp = ptr - 2;

    p0 = _mm_cvtsi32_si128(*(U32*)inp); inp += pitch; // (x0):x=[abcd]
    p1 = _mm_cvtsi32_si128(*(U32*)inp); inp += pitch; // (x1):x=[abcd]
    p2 = _mm_cvtsi32_si128(*(U32*)inp); inp += pitch; // (x2):x=[abcd]
    p3 = _mm_cvtsi32_si128(*(U32*)inp); inp += pitch; // (x3):x=[abcd]
    p0 = _mm_unpacklo_epi8(p0, _mm_cvtsi32_si128(*(U32*)inp)); inp += pitch; // (x0x4):x=[abcd]
    p1 = _mm_unpacklo_epi8(p1, _mm_cvtsi32_si128(*(U32*)inp)); inp += pitch; // (x1x5):x=[abcd]
    p2 = _mm_unpacklo_epi8(p2, _mm_cvtsi32_si128(*(U32*)inp)); inp += pitch; // (x2x6):x=[abcd]
    p3 = _mm_unpacklo_epi8(p3, _mm_cvtsi32_si128(*(U32*)inp)); inp += pitch; // (x3x7):x=[abcd]

    p0 = _mm_unpacklo_epi8(p0, p2); // (x0x2x4x6):x=[abcd]
    p1 = _mm_unpacklo_epi8(p1, p3); // (x1x3x5x7):x=[abcd]

    r0 = _mm_unpacklo_epi8(p0, p1); // (x0..x7):x=[ab]
    r1 = _mm_unpackhi_epi8(p0, p1); // (x0..x7):x=[cd]
  }

  // load masks
  masks = _mm_load_si128((__m128i const *) mask);
  masks = _mm_unpacklo_epi16(masks, masks); // expand to dwords which are cheaper to shuffle

  // actual deblock
  {
    __m128i m2, m1, p0, p1, zero;
    __m128i diff, delta_near, delta_far;

    zero = _mm_setzero_si128();
    m2 = _mm_unpacklo_epi8(r0, zero); // a0..a7
    m1 = _mm_unpackhi_epi8(r0, zero); // b0..b7
    p0 = _mm_unpacklo_epi8(r1, zero); // c0..c7
    p1 = _mm_unpackhi_epi8(r1, zero); // d0..d7

    diff = _mm_sub_epi16(p0, m1);
    delta_near = _mm_srai_epi16(_mm_add_epi16(diff, _mm_set1_epi16(2)), 2);
    delta_far = _mm_srai_epi16(_mm_add_epi16(diff, _mm_set1_epi16(4)), 3);

    m2 = _mm_add_epi16(m2, _mm_and_si128(_mm_shuffle_epi32(masks, 0x00), delta_far));
    m1 = _mm_add_epi16(m1, _mm_and_si128(_mm_shuffle_epi32(masks, 0x55), delta_near));
    p0 = _mm_sub_epi16(p0, _mm_and_si128(_mm_shuffle_epi32(masks, 0xaa), delta_near));
    p1 = _mm_sub_epi16(p1, _mm_and_si128(_mm_shuffle_epi32(masks, 0xff), delta_far));

    r0 = _mm_packus_epi16(m2, m1); // (x0..x7):x=[ab]
    r1 = _mm_packus_epi16(p0, p1); // (x0..x7):x=[cd]
  }

  // interleave back to 4-pixel groups
  {
    __m128i t0, t1;

    t0 = _mm_unpacklo_epi8(r0, r1); // (aXcX):X=[0..7]
    t1 = _mm_unpackhi_epi8(r0, r1); // (bXdX):X=[0..7]

    r0 = _mm_unpacklo_epi8(t0, t1); // (aXbXcXdX):X=[0..3]
    r1 = _mm_unpackhi_epi8(t0, t1); // (aXbXcXdX):X=[4..7]
  }

  // store
  {
    U8 * RADRESTRICT outp = ptr - 2;

    *(U32 *) outp = _mm_cvtsi128_si32(r0); outp += pitch;
    *(U32 *) outp = _mm_cvtsi128_si32(_mm_shuffle_epi32(r0, 0x55)); outp += pitch;
    *(U32 *) outp = _mm_cvtsi128_si32(_mm_shuffle_epi32(r0, 0xaa)); outp += pitch;
    *(U32 *) outp = _mm_cvtsi128_si32(_mm_shuffle_epi32(r0, 0xff)); outp += pitch;
    *(U32 *) outp = _mm_cvtsi128_si32(r1); outp += pitch;
    *(U32 *) outp = _mm_cvtsi128_si32(_mm_shuffle_epi32(r1, 0x55)); outp += pitch;
    *(U32 *) outp = _mm_cvtsi128_si32(_mm_shuffle_epi32(r1, 0xaa)); outp += pitch;
    *(U32 *) outp = _mm_cvtsi128_si32(_mm_shuffle_epi32(r1, 0xff)); outp += pitch;
  }
}


