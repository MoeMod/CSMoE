// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

static CODEGEN_ATTR void mublock16_UtoD(char unsigned * RADRESTRICT pixels, int32_t stride, char unsigned * RADRESTRICT mask0, char unsigned * RADRESTRICT mask1, char unsigned * RADRESTRICT mask2, char unsigned * RADRESTRICT mask3)
{
  {
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(pixels);
  int32_t v1 = stride;
  int16x8_t v10_0 = vreinterpretq_s16_u32(vcombine_u32(vld1_u32((uint32_t const *)(v0 + 0*v1)), vld1_u32((uint32_t const *)(v0 + 4*v1))));
  int16x8_t v10_2 = vreinterpretq_s16_u32(vcombine_u32(vld1_u32((uint32_t const *)(v0 + 2*v1)), vld1_u32((uint32_t const *)(v0 + 6*v1))));
  int16x8_t v11_0 = vreinterpretq_s16_s32(vtrnq_s32(vreinterpretq_s32_s16(v10_0), vreinterpretq_s32_s16(v10_2)).val[0]);
  int16x8_t v10_1 = vreinterpretq_s16_u32(vcombine_u32(vld1_u32((uint32_t const *)(v0 + 1*v1)), vld1_u32((uint32_t const *)(v0 + 5*v1))));
  int16x8_t v10_3 = vreinterpretq_s16_u32(vcombine_u32(vld1_u32((uint32_t const *)(v0 + 3*v1)), vld1_u32((uint32_t const *)(v0 + 7*v1))));
  int16x8_t v11_1 = vreinterpretq_s16_s32(vtrnq_s32(vreinterpretq_s32_s16(v10_1), vreinterpretq_s32_s16(v10_3)).val[0]);
  int16x8_t v12_0 = vtrnq_s16(v11_0, v11_1).val[0];
  int16x8_t v11_2 = vreinterpretq_s16_s32(vtrnq_s32(vreinterpretq_s32_s16(v10_0), vreinterpretq_s32_s16(v10_2)).val[1]);
  int16x8_t v11_3 = vreinterpretq_s16_s32(vtrnq_s32(vreinterpretq_s32_s16(v10_1), vreinterpretq_s32_s16(v10_3)).val[1]);
  int16x8_t v12_2 = vtrnq_s16(v11_2, v11_3).val[0];
  int16x8_t v12_1 = vtrnq_s16(v11_0, v11_1).val[1];
  int16x8_t v17 = vsubq_s16(v12_2, v12_1);
  int16x8_t v20 = vaddq_s16(v17, vdupq_n_s16(4));
  int16x8_t v21 = (vshrq_n_s16((v20), 3));
  char unsigned * RADRESTRICT v2 = (char unsigned * RADRESTRICT)(mask0);
  int16x8_t v6 = vld1q_s16((int16_t * RADRESTRICT)(v2 + 0));
  int16x8_t v22 = vandq_s16(v21, v6);
  int16x8_t v23 = vaddq_s16(v12_0, v22);
  int16x8_t v18 = vaddq_s16(v17, vdupq_n_s16(2));
  int16x8_t v19 = (vshrq_n_s16((v18), 2));
  char unsigned * RADRESTRICT v3 = (char unsigned * RADRESTRICT)(mask1);
  int16x8_t v7 = vld1q_s16((int16_t * RADRESTRICT)(v3 + 0));
  int16x8_t v24 = vandq_s16(v19, v7);
  int16x8_t v25 = vaddq_s16(v12_1, v24);
  char unsigned * RADRESTRICT v4 = (char unsigned * RADRESTRICT)(mask2);
  int16x8_t v8 = vld1q_s16((int16_t * RADRESTRICT)(v4 + 0));
  int16x8_t v26 = vandq_s16(v19, v8);
  int16x8_t v27 = vsubq_s16(v12_2, v26);
  int16x8_t v12_3 = vtrnq_s16(v11_2, v11_3).val[1];
  char unsigned * RADRESTRICT v5 = (char unsigned * RADRESTRICT)(mask3);
  int16x8_t v9 = vld1q_s16((int16_t * RADRESTRICT)(v5 + 0));
  int16x8_t v28 = vandq_s16(v21, v9);
  int16x8_t v29 = vsubq_s16(v12_3, v28);
  char unsigned * RADRESTRICT v33_row1 = v0 + v1;
  char unsigned * RADRESTRICT v33_row2 = v33_row1 + v1;
  char unsigned * RADRESTRICT v33_row3 = v33_row2 + v1;
  char unsigned * RADRESTRICT v33_row4 = v33_row3 + v1;
  char unsigned * RADRESTRICT v33_row5 = v33_row4 + v1;
  char unsigned * RADRESTRICT v33_row6 = v33_row5 + v1;
  char unsigned * RADRESTRICT v33_row7 = v33_row6 + v1;
  int16x8x2_t v33_tr0 = vtrnq_s16(v23, v25);
  int16x8x2_t v33_tr1 = vtrnq_s16(v27, v29);
  uint32x4_t v33_w0 = vreinterpretq_u32_s16(v33_tr0.val[0]);
  uint32x4_t v33_w1 = vreinterpretq_u32_s16(v33_tr0.val[1]);
  uint32x4_t v33_w2 = vreinterpretq_u32_s16(v33_tr1.val[0]);
  uint32x4_t v33_w3 = vreinterpretq_u32_s16(v33_tr1.val[1]);
  vst1q_lane_u32((uint32_t *)(v0 + 0), v33_w0, 0);
  vst1q_lane_u32((uint32_t *)(v0 + 4), v33_w2, 0);
  vst1q_lane_u32((uint32_t *)(v33_row1 + 0), v33_w1, 0);
  vst1q_lane_u32((uint32_t *)(v33_row1 + 4), v33_w3, 0);
  vst1q_lane_u32((uint32_t *)(v33_row2 + 0), v33_w0, 1);
  vst1q_lane_u32((uint32_t *)(v33_row2 + 4), v33_w2, 1);
  vst1q_lane_u32((uint32_t *)(v33_row3 + 0), v33_w1, 1);
  vst1q_lane_u32((uint32_t *)(v33_row3 + 4), v33_w3, 1);
  vst1q_lane_u32((uint32_t *)(v33_row4 + 0), v33_w0, 2);
  vst1q_lane_u32((uint32_t *)(v33_row4 + 4), v33_w2, 2);
  vst1q_lane_u32((uint32_t *)(v33_row5 + 0), v33_w1, 2);
  vst1q_lane_u32((uint32_t *)(v33_row5 + 4), v33_w3, 2);
  vst1q_lane_u32((uint32_t *)(v33_row6 + 0), v33_w0, 3);
  vst1q_lane_u32((uint32_t *)(v33_row6 + 4), v33_w2, 3);
  vst1q_lane_u32((uint32_t *)(v33_row7 + 0), v33_w1, 3);
  vst1q_lane_u32((uint32_t *)(v33_row7 + 4), v33_w3, 3);
  }
}

static CODEGEN_ATTR void mublock16_LtoR(char unsigned * RADRESTRICT pixels, int32_t stride, char unsigned * RADRESTRICT mask0, char unsigned * RADRESTRICT mask1, char unsigned * RADRESTRICT mask2, char unsigned * RADRESTRICT mask3)
{
  {
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(pixels);
  int16x8_t v10 = vld1q_s16((int16_t * RADRESTRICT)(v0 + 0));
  int32_t v1 = stride;
  char unsigned * RADRESTRICT v11 = v0 + v1;
  char unsigned * RADRESTRICT v13 = v11 + v1;
  int16x8_t v14 = vld1q_s16((int16_t * RADRESTRICT)(v13 + 0));
  int16x8_t v12 = vld1q_s16((int16_t * RADRESTRICT)(v11 + 0));
  int16x8_t v18 = vsubq_s16(v14, v12);
  int16x8_t v21 = vaddq_s16(v18, vdupq_n_s16(4));
  int16x8_t v22 = (vshrq_n_s16((v21), 3));
  char unsigned * RADRESTRICT v2 = (char unsigned * RADRESTRICT)(mask0);
  int16x8_t v6 = vld1q_s16((int16_t * RADRESTRICT)(v2 + 0));
  int16x8_t v23 = vandq_s16(v22, v6);
  int16x8_t v24 = vaddq_s16(v10, v23);
  vst1q_s16((int16_t * RADRESTRICT)(v0 + 0), v24);

  {
  int16x8_t v19 = vaddq_s16(v18, vdupq_n_s16(2));
  int16x8_t v20 = (vshrq_n_s16((v19), 2));
  char unsigned * RADRESTRICT v3 = (char unsigned * RADRESTRICT)(mask1);
  int16x8_t v7 = vld1q_s16((int16_t * RADRESTRICT)(v3 + 0));
  int16x8_t v25 = vandq_s16(v20, v7);
  int16x8_t v26 = vaddq_s16(v12, v25);
  vst1q_s16((int16_t * RADRESTRICT)(v11 + 0), v26);

  {
  char unsigned * RADRESTRICT v4 = (char unsigned * RADRESTRICT)(mask2);
  int16x8_t v8 = vld1q_s16((int16_t * RADRESTRICT)(v4 + 0));
  int16x8_t v27 = vandq_s16(v20, v8);
  int16x8_t v28 = vsubq_s16(v14, v27);
  vst1q_s16((int16_t * RADRESTRICT)(v13 + 0), v28);

  {
  char unsigned * RADRESTRICT v15 = v13 + v1;
  int16x8_t v16 = vld1q_s16((int16_t * RADRESTRICT)(v15 + 0));
  char unsigned * RADRESTRICT v5 = (char unsigned * RADRESTRICT)(mask3);
  int16x8_t v9 = vld1q_s16((int16_t * RADRESTRICT)(v5 + 0));
  int16x8_t v29 = vandq_s16(v22, v9);
  int16x8_t v30 = vsubq_s16(v16, v29);
  vst1q_s16((int16_t * RADRESTRICT)(v15 + 0), v30);
  }
  }
  }
  }
}

