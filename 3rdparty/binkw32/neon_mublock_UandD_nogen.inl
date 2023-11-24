// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#ifdef __SNC__
__asm__ void bink2_mublock_UandD( U16 const * RADRESTRICT mask, U8 * RADRESTRICT ptr, U32 pitch )
{
  sub       r3,r1,#2
  vld1.32   {d0[]},[r3],r2
  vld1.32   {d1[]},[r3],r2
  vld1.32   {d2[]},[r3],r2
  vld1.32   {d3[]},[r3],r2
  vld1.32   {d0[1]},[r3],r2
  vld1.32   {d1[1]},[r3],r2
  vld1.32   {d2[1]},[r3],r2
  vld1.32   {d3[1]},[r3]

  vtrn.8    d0,d1
  vtrn.8    d2,d3
  vtrn.16   q0,q1

  vld1.16   {d4},[r0] // masks

  vsubl.u8  q9,d2,d1      // q9=diff
  vmovl.u8  q12,d0
  vmovl.u8  q13,d1
  vrshr.s16 q10,q9,#2     // q10=delta_near
  vrshr.s16 q11,q9,#3     // q11=delta_far
  vmovl.u8  q14,d2
  vmovl.u8  q15,d3
  vmls.i16  q12,q11,d4[0]
  vmls.i16  q13,q10,d4[1]
  vmla.i16  q14,q10,d4[2]
  vmla.i16  q15,q11,d4[3]

  vqmovun.s16 d0,q12
  vqmovun.s16 d1,q13
  vqmovun.s16 d2,q14
  vqmovun.s16 d3,q15

  sub       r3,r1,#2
  vtrn.16   q0,q1

  vtrn.8    d0,d1
  vtrn.8    d2,d3

  vst1.32   {d0[0]},[r3],r2
  vst1.32   {d1[0]},[r3],r2
  vst1.32   {d2[0]},[r3],r2
  vst1.32   {d3[0]},[r3],r2
  vst1.32   {d0[1]},[r3],r2
  vst1.32   {d1[1]},[r3],r2
  vst1.32   {d2[1]},[r3],r2
  vst1.32   {d3[1]},[r3]

  bx        lr
}

#define mublock_UandD bink2_mublock_UandD

#else
static CODEGEN_ATTR void mublock_UandD( U16 const * RADRESTRICT mask, U8 * RADRESTRICT ptr, U32 pitch )
{
#if defined(__GNUC__) || defined(__clang__)
  typedef U32 U32un __attribute__((aligned(1))); // unaligned U32
#else
  typedef U32 U32un;
#endif

  uint8x16_t r0, r1;
  int16x4_t masks;

  // load rows and transpose
  {
    uint8x8_t p0,p1,p2,p3;
    uint8x8x2_t p01t, p23t;
    uint16x8_t q0,q1;
    U8 const * RADRESTRICT inp = ptr - 2;

    p0 = vreinterpret_u8_u32(vld1_dup_u32((U32un const *) inp)); inp += pitch;
    p1 = vreinterpret_u8_u32(vld1_dup_u32((U32un const *) inp)); inp += pitch;
    p2 = vreinterpret_u8_u32(vld1_dup_u32((U32un const *) inp)); inp += pitch;
    p3 = vreinterpret_u8_u32(vld1_dup_u32((U32un const *) inp)); inp += pitch;
    p0 = vreinterpret_u8_u32(vld1_lane_u32((U32un const *) inp, vreinterpret_u32_u8(p0), 1)); inp += pitch; // p0=(a0b0c0d0a4b4c4d4)
    p1 = vreinterpret_u8_u32(vld1_lane_u32((U32un const *) inp, vreinterpret_u32_u8(p1), 1)); inp += pitch; // p1=(a1b1c1d1a5b5c5d5)
    p2 = vreinterpret_u8_u32(vld1_lane_u32((U32un const *) inp, vreinterpret_u32_u8(p2), 1)); inp += pitch; // p2=(a2b2c2d2a6b6c6d6)
    p3 = vreinterpret_u8_u32(vld1_lane_u32((U32un const *) inp, vreinterpret_u32_u8(p3), 1)); inp += pitch; // p3=(a3b3c3d3a7b7c7d7)

    p01t = vtrn_u8(p0, p1);
    p23t = vtrn_u8(p2, p3);

    p0 = p01t.val[0]; // a0a1c0c1a4a5c4c5
    p1 = p01t.val[1]; // b0b1d0d1b4b5d4d5
    p2 = p23t.val[0]; // a2a3c2c3a6a7c6c7
    p3 = p23t.val[1]; // b2b3d2d3b6b7d6d7

    q0 = vreinterpretq_u16_u8(vcombine_u8(p0, p1));      // a0a1c0c1a4a5c4c5|b0b1d0d1b4b5d4d5
    q1 = vreinterpretq_u16_u8(vcombine_u8(p2, p3));      // a2a3c2c3a6a7c6c7|b2b3d2d3b6b7d6d7
    r0 = vreinterpretq_u8_u16(vtrnq_u16(q0, q1).val[0]); // a0a1a2a3a4a5a6a7|b0b1b2b3b4b5b6b7
    r1 = vreinterpretq_u8_u16(vtrnq_u16(q0, q1).val[1]); // c0c1c2c3c4c5c6c7|d0d1d2d3d4d5d6d7
  }

  // load masks
  masks = vld1_s16((S16 const *) mask);

  // actual deblock
  {
    int16x8_t m2, m1, p0, p1;
    int16x8_t diff, delta_near, delta_far;

    m2 = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(r0)));  // a0..a7
    m1 = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(r0))); // b0..b7
    p0 = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(r1)));  // c0..c7
    p1 = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(r1))); // d0..d7

    diff = vreinterpretq_s16_u16(vsubl_u8(vget_low_u8(r1), vget_high_u8(r0))); // p0-m1
    delta_near = vrshrq_n_s16(diff, 2);
    delta_far = vrshrq_n_s16(diff, 3);

    // each lane of "masks" is either 0 or 0xffff (-1).
    // so even though we add to m2/m1 and subtract from p0/p1, we use
    // "multiply-subtract" for the former and "multiply-add" for the latter.
    m2 = vmlsq_lane_s16(m2, delta_far,  masks, 0);
    m1 = vmlsq_lane_s16(m1, delta_near, masks, 1);
    p0 = vmlaq_lane_s16(p0, delta_near, masks, 2);
    p1 = vmlaq_lane_s16(p1, delta_far,  masks, 3);

    r0 = vcombine_u8(vqmovun_s16(m2), vqmovun_s16(m1)); // a0..a7|b0..b7
    r1 = vcombine_u8(vqmovun_s16(p0), vqmovun_s16(p1)); // c0..c7|d0..d7
  }

  // transpose back and store
  {
    U8 * RADRESTRICT outp = ptr - 2;
    uint32x2_t p0,p1,p2,p3;
    uint8x16_t q0,q1;

    q0 = vreinterpretq_u8_u16(vtrnq_u16(vreinterpretq_u16_u8(r0), vreinterpretq_u16_u8(r1)).val[0]); // a0a1c0c1a4a5c4c5|b0b1d0d1b4b5d4d5
    q1 = vreinterpretq_u8_u16(vtrnq_u16(vreinterpretq_u16_u8(r0), vreinterpretq_u16_u8(r1)).val[1]); // a2a3c2c3a6a7c6c7|b2b3d2d3b6b7d6d7

    p0 = vreinterpret_u32_u8(vtrn_u8(vget_low_u8(q0), vget_high_u8(q0)).val[0]); // p0=(a0b0c0d0a4b4c4d4)
    p1 = vreinterpret_u32_u8(vtrn_u8(vget_low_u8(q0), vget_high_u8(q0)).val[1]); // p1=(a1b1c1d1a5b5c5d5)
    p2 = vreinterpret_u32_u8(vtrn_u8(vget_low_u8(q1), vget_high_u8(q1)).val[0]); // p2=(a2b2c2d2a6b6c6d6)
    p3 = vreinterpret_u32_u8(vtrn_u8(vget_low_u8(q1), vget_high_u8(q1)).val[1]); // p3=(a3b3c3d3a7b7c7d7)

    vst1_lane_u32((U32un *)outp, p0, 0); outp += pitch;
    vst1_lane_u32((U32un *)outp, p1, 0); outp += pitch;
    vst1_lane_u32((U32un *)outp, p2, 0); outp += pitch;
    vst1_lane_u32((U32un *)outp, p3, 0); outp += pitch;
    vst1_lane_u32((U32un *)outp, p0, 1); outp += pitch;
    vst1_lane_u32((U32un *)outp, p1, 1); outp += pitch;
    vst1_lane_u32((U32un *)outp, p2, 1); outp += pitch;
    vst1_lane_u32((U32un *)outp, p3, 1); outp += pitch;
  }
}
#endif
