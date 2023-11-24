// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

static CODEGEN_ATTR void mublock_vertical_masked_2(char unsigned * RADRESTRICT px, int32_t sw, char unsigned * RADRESTRICT mask_l, char unsigned * RADRESTRICT mask_r)
{
  {
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(px);
  uint8x16_t v17 = vld1q_u8((uint8_t * RADRESTRICT)(v0 + 0));
  int16x8_t v21_0 = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(v17)));
  int32_t v1 = sw;
  char unsigned * RADRESTRICT v2 = v0 + v1;
  char unsigned * RADRESTRICT v3 = v2 + v1;
  uint8x16_t v19 = vld1q_u8((uint8_t * RADRESTRICT)(v3 + 0));
  int16x8_t v23_0 = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(v19)));
  uint8x16_t v18 = vld1q_u8((uint8_t * RADRESTRICT)(v2 + 0));
  int16x8_t v22_0 = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(v18)));
  int16x8_t v33 = vsubq_s16(v23_0, v22_0);
  int16x8_t v36 = vshlq_n_s16((v33), 1);
  int16x8_t v39 = (vrshrq_n_s16(v36, 4));
  char unsigned * RADRESTRICT v5 = (char unsigned * RADRESTRICT)(mask_l);
  int16x8_t v7 = vld1q_s16((int16_t * RADRESTRICT)(v5 + 0));
  int16x8_t v9 = vdupq_lane_s16(vget_low_s16(v7), 0);
  int16x8_t v40 = vandq_s16(v39, v9);
  int16x8_t v44 = vaddq_s16(v21_0, v40);
  int16x8_t v21_1 = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(v17)));
  int16x8_t v23_1 = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(v19)));
  int16x8_t v22_1 = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(v18)));
  int16x8_t v48 = vsubq_s16(v23_1, v22_1);
  int16x8_t v51 = vshlq_n_s16((v48), 1);
  int16x8_t v54 = (vrshrq_n_s16(v51, 4));
  char unsigned * RADRESTRICT v6 = (char unsigned * RADRESTRICT)(mask_r);
  int16x8_t v8 = vld1q_s16((int16_t * RADRESTRICT)(v6 + 0));
  int16x8_t v13 = vdupq_lane_s16(vget_low_s16(v8), 0);
  int16x8_t v55 = vandq_s16(v54, v13);
  int16x8_t v59 = vaddq_s16(v21_1, v55);
  uint8x16_t v64 = vcombine_u8(vqmovun_s16(v44), vqmovun_s16(v59));
  vst1q_u8((uint8_t * RADRESTRICT)(v0 + 0), v64);

  {
  int16x8_t v35 = vshlq_n_s16((v33), 2);
  int16x8_t v38 = (vrshrq_n_s16(v35, 4));
  int16x8_t v10 = vdupq_lane_s16(vget_low_s16(v7), 1);
  int16x8_t v41 = vandq_s16(v38, v10);
  int16x8_t v45 = vaddq_s16(v22_0, v41);
  int16x8_t v50 = vshlq_n_s16((v48), 2);
  int16x8_t v53 = (vrshrq_n_s16(v50, 4));
  int16x8_t v14 = vdupq_lane_s16(vget_low_s16(v8), 1);
  int16x8_t v56 = vandq_s16(v53, v14);
  int16x8_t v60 = vaddq_s16(v22_1, v56);
  uint8x16_t v66 = vcombine_u8(vqmovun_s16(v45), vqmovun_s16(v60));
  vst1q_u8((uint8_t * RADRESTRICT)(v2 + 0), v66);

  {
  int16x8_t v11 = vdupq_lane_s16(vget_low_s16(v7), 2);
  int16x8_t v42 = vandq_s16(v38, v11);
  int16x8_t v46 = vsubq_s16(v23_0, v42);
  int16x8_t v15 = vdupq_lane_s16(vget_low_s16(v8), 2);
  int16x8_t v57 = vandq_s16(v53, v15);
  int16x8_t v61 = vsubq_s16(v23_1, v57);
  uint8x16_t v68 = vcombine_u8(vqmovun_s16(v46), vqmovun_s16(v61));
  vst1q_u8((uint8_t * RADRESTRICT)(v3 + 0), v68);

  {
  char unsigned * RADRESTRICT v4 = v3 + v1;
  uint8x16_t v20 = vld1q_u8((uint8_t * RADRESTRICT)(v4 + 0));
  int16x8_t v24_0 = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(v20)));
  int16x8_t v12 = vdupq_lane_s16(vget_low_s16(v7), 3);
  int16x8_t v43 = vandq_s16(v39, v12);
  int16x8_t v47 = vsubq_s16(v24_0, v43);
  int16x8_t v24_1 = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(v20)));
  int16x8_t v16 = vdupq_lane_s16(vget_low_s16(v8), 3);
  int16x8_t v58 = vandq_s16(v54, v16);
  int16x8_t v62 = vsubq_s16(v24_1, v58);
  uint8x16_t v70 = vcombine_u8(vqmovun_s16(v47), vqmovun_s16(v62));
  vst1q_u8((uint8_t * RADRESTRICT)(v4 + 0), v70);
  }
  }
  }
  }
}

