// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

static CODEGEN_ATTR void muratori32_16x16_h0_v0(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int32_t sw, int32_t dw)
{
  {
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(src);

  {
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dest);

  {
  int32_t v2 = sw;

  {
  int32_t v3 = dw;

  {
{int32_t v4;
for(v4 = 0; v4 < 16; v4 += 1)
  {
  uint8x16_t v8 = vld1q_u8((uint8_t * RADRESTRICT)(v0 + 0));
  vst1q_u8((uint8_t * RADRESTRICT)(v1 + 0), v8);

  {
  char unsigned * RADRESTRICT v20 = v0 + v2;
  v0 = v20;

  {
  char unsigned * RADRESTRICT v22 = v1 + v3;
  v1 = v22;
  }
  }
  }
}
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void muratori32_16x16_h0_v1(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int32_t sw, int32_t dw)
{
  {
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(src);
  int32_t v2 = sw;
  int32_t v4 = v2 * -2;
  char unsigned * RADRESTRICT v5 = v0 + v4;

  {
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dest);

  {

  {
  int32_t v3 = dw;

  {
{int32_t v6;
for(v6 = 0; v6 < 16; v6 += 1)
  {
  char unsigned * RADRESTRICT v11 = v5 + v2;
  char unsigned * RADRESTRICT v13 = v11 + v2;
  uint8x16_t v14 = vld1q_u8((uint8_t * RADRESTRICT)(v13 + 0));
  char unsigned * RADRESTRICT v15 = v13 + v2;
  uint8x16_t v16 = vld1q_u8((uint8_t * RADRESTRICT)(v15 + 0));
  int16x8_t v24_0 = vreinterpretq_s16_u16(vaddl_u8(vget_low_u8(v14), vget_low_u8(v16)));
  int16x8_t v25_0 = vmulq_n_s16(v24_0, 19);
  int16x8_t v26_0 = (vshrq_n_s16((v25_0), 1));
  uint8x16_t v10 = vld1q_u8((uint8_t * RADRESTRICT)(v5 + 0));
  char unsigned * RADRESTRICT v17 = v15 + v2;
  char unsigned * RADRESTRICT v19 = v17 + v2;
  uint8x16_t v20 = vld1q_u8((uint8_t * RADRESTRICT)(v19 + 0));
  uint8x16_t v21 = vhaddq_u8(v10, v20);
  int16x8_t v27_0 = vreinterpretq_s16_u16(vaddw_u8(vreinterpretq_u16_s16(v26_0), vget_low_u8(v21)));
  uint8x16_t v12 = vld1q_u8((uint8_t * RADRESTRICT)(v11 + 0));
  uint8x16_t v18 = vld1q_u8((uint8_t * RADRESTRICT)(v17 + 0));
  int16x8_t v22_0 = vreinterpretq_s16_u16(vaddl_u8(vget_low_u8(v12), vget_low_u8(v18)));
  int16x8_t v23_0 = vshlq_n_s16((v22_0), 1);
  int16x8_t v28_0 = vsubq_s16(v27_0, v23_0);
  int16x8_t v24_1 = vreinterpretq_s16_u16(vaddl_u8(vget_high_u8(v14), vget_high_u8(v16)));
  int16x8_t v25_1 = vmulq_n_s16(v24_1, 19);
  int16x8_t v26_1 = (vshrq_n_s16((v25_1), 1));
  int16x8_t v27_1 = vreinterpretq_s16_u16(vaddw_u8(vreinterpretq_u16_s16(v26_1), vget_high_u8(v21)));
  int16x8_t v22_1 = vreinterpretq_s16_u16(vaddl_u8(vget_high_u8(v12), vget_high_u8(v18)));
  int16x8_t v23_1 = vshlq_n_s16((v22_1), 1);
  int16x8_t v28_1 = vsubq_s16(v27_1, v23_1);
  uint8x16_t v30 = vcombine_u8(vqrshrun_n_s16(v28_0, 4), vqrshrun_n_s16(v28_1, 4));
  vst1q_u8((uint8_t * RADRESTRICT)(v1 + 0), v30);

  {
  char unsigned * RADRESTRICT v32 = v5 + v2;
  v5 = v32;

  {
  char unsigned * RADRESTRICT v34 = v1 + v3;
  v1 = v34;
  }
  }
  }
}
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void muratori32_16x16_h1_v0(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int32_t sw, int32_t dw)
{
  {
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(src);
  char unsigned * RADRESTRICT v4 = v0 + -2;

  {
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dest);

  {
  int32_t v2 = sw;

  {
  int32_t v3 = dw;

  {
{int32_t v5;
for(v5 = 0; v5 < 16; v5 += 1)
  {
  uint8x16_t v11 = vld1q_u8((uint8_t * RADRESTRICT)(v4 + 2));
  uint8x16_t v12 = vld1q_u8((uint8_t * RADRESTRICT)(v4 + 3));
  int16x8_t v18_0 = vreinterpretq_s16_u16(vaddl_u8(vget_low_u8(v11), vget_low_u8(v12)));
  int16x8_t v19_0 = vmulq_n_s16(v18_0, 19);
  int16x8_t v20_0 = (vshrq_n_s16((v19_0), 1));
  uint8x16_t v9 = vld1q_u8((uint8_t * RADRESTRICT)(v4 + 0));
  uint8x16_t v14 = vld1q_u8((uint8_t * RADRESTRICT)(v4 + 5));
  uint8x16_t v15 = vhaddq_u8(v9, v14);
  int16x8_t v21_0 = vreinterpretq_s16_u16(vaddw_u8(vreinterpretq_u16_s16(v20_0), vget_low_u8(v15)));
  uint8x16_t v10 = vld1q_u8((uint8_t * RADRESTRICT)(v4 + 1));
  uint8x16_t v13 = vld1q_u8((uint8_t * RADRESTRICT)(v4 + 4));
  int16x8_t v16_0 = vreinterpretq_s16_u16(vaddl_u8(vget_low_u8(v10), vget_low_u8(v13)));
  int16x8_t v17_0 = vshlq_n_s16((v16_0), 1);
  int16x8_t v22_0 = vsubq_s16(v21_0, v17_0);
  int16x8_t v23_0 = (vrshrq_n_s16(v22_0, 4));
  int16x8_t v18_1 = vreinterpretq_s16_u16(vaddl_u8(vget_high_u8(v11), vget_high_u8(v12)));
  int16x8_t v19_1 = vmulq_n_s16(v18_1, 19);
  int16x8_t v20_1 = (vshrq_n_s16((v19_1), 1));
  int16x8_t v21_1 = vreinterpretq_s16_u16(vaddw_u8(vreinterpretq_u16_s16(v20_1), vget_high_u8(v15)));
  int16x8_t v16_1 = vreinterpretq_s16_u16(vaddl_u8(vget_high_u8(v10), vget_high_u8(v13)));
  int16x8_t v17_1 = vshlq_n_s16((v16_1), 1);
  int16x8_t v22_1 = vsubq_s16(v21_1, v17_1);
  int16x8_t v23_1 = (vrshrq_n_s16(v22_1, 4));
  uint8x16_t v24 = vcombine_u8(vqmovun_s16(v23_0), vqmovun_s16(v23_1));
  vst1q_u8((uint8_t * RADRESTRICT)(v1 + 0), v24);

  {
  char unsigned * RADRESTRICT v26 = v1 + v3;
  v1 = v26;

  {
  char unsigned * RADRESTRICT v28 = v4 + v2;
  v4 = v28;
  }
  }
  }
}
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void muratori32_16x16_h1_v1(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int32_t sw, int32_t dw)
{
  {
  RAD_ALIGN(int16_t, v6_0[21][8], 16);
  RAD_ALIGN(int16_t, v6_1[21][8], 16);

  {
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(src);
  int32_t v2 = sw;
  int32_t v4 = v2 * -2;
  char unsigned * RADRESTRICT v5 = v0 + v4;
  char unsigned * RADRESTRICT v8 = v5 + -2;

  {
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dest);

  {

  {
  int32_t v3 = dw;

  {
{int32_t v9;
for(v9 = 0; v9 < 21; v9 += 1)
  {
  uint8x16_t v15 = vld1q_u8((uint8_t * RADRESTRICT)(v8 + 2));
  uint8x16_t v16 = vld1q_u8((uint8_t * RADRESTRICT)(v8 + 3));
  int16x8_t v22_0 = vreinterpretq_s16_u16(vaddl_u8(vget_low_u8(v15), vget_low_u8(v16)));
  int16x8_t v23_0 = vmulq_n_s16(v22_0, 19);
  int16x8_t v24_0 = (vshrq_n_s16((v23_0), 1));
  uint8x16_t v13 = vld1q_u8((uint8_t * RADRESTRICT)(v8 + 0));
  uint8x16_t v18 = vld1q_u8((uint8_t * RADRESTRICT)(v8 + 5));
  uint8x16_t v19 = vhaddq_u8(v13, v18);
  int16x8_t v25_0 = vreinterpretq_s16_u16(vaddw_u8(vreinterpretq_u16_s16(v24_0), vget_low_u8(v19)));
  uint8x16_t v14 = vld1q_u8((uint8_t * RADRESTRICT)(v8 + 1));
  uint8x16_t v17 = vld1q_u8((uint8_t * RADRESTRICT)(v8 + 4));
  int16x8_t v20_0 = vreinterpretq_s16_u16(vaddl_u8(vget_low_u8(v14), vget_low_u8(v17)));
  int16x8_t v21_0 = vshlq_n_s16((v20_0), 1);
  int16x8_t v26_0 = vsubq_s16(v25_0, v21_0);
  int16x8_t v27_0 = (vrshrq_n_s16(v26_0, 4));
  int16x8_t v22_1 = vreinterpretq_s16_u16(vaddl_u8(vget_high_u8(v15), vget_high_u8(v16)));
  int16x8_t v23_1 = vmulq_n_s16(v22_1, 19);
  int16x8_t v24_1 = (vshrq_n_s16((v23_1), 1));
  int16x8_t v25_1 = vreinterpretq_s16_u16(vaddw_u8(vreinterpretq_u16_s16(v24_1), vget_high_u8(v19)));
  int16x8_t v20_1 = vreinterpretq_s16_u16(vaddl_u8(vget_high_u8(v14), vget_high_u8(v17)));
  int16x8_t v21_1 = vshlq_n_s16((v20_1), 1);
  int16x8_t v26_1 = vsubq_s16(v25_1, v21_1);
  int16x8_t v27_1 = (vrshrq_n_s16(v26_1, 4));
  vst1q_s16((int16_t * RADRESTRICT)(v6_0[v9] + 0), v27_0);
  vst1q_s16((int16_t * RADRESTRICT)(v6_1[v9] + 0), v27_1);

  {
  char unsigned * RADRESTRICT v29 = v8 + v2;
  v8 = v29;
  }
  }
}

  {
{int32_t v31;
for(v31 = 0; v31 < 16; v31 += 1)
  {
  int32_t v35 = v31 + 0;
  int16x8_t v36_0 = vld1q_s16((int16_t * RADRESTRICT)(v6_0[v35] + 0));
  int32_t v45 = v31 + 5;
  int16x8_t v46_0 = vld1q_s16((int16_t * RADRESTRICT)(v6_0[v45] + 0));
  int16x8_t v47_0 = vhaddq_s16(v36_0, v46_0);
  int32_t v39 = v31 + 2;
  int16x8_t v40_0 = vld1q_s16((int16_t * RADRESTRICT)(v6_0[v39] + 0));
  int32_t v41 = v31 + 3;
  int16x8_t v42_0 = vld1q_s16((int16_t * RADRESTRICT)(v6_0[v41] + 0));
  int16x8_t v50_0 = vaddq_s16(v40_0, v42_0);
  int16x8_t v51_0 = vmulq_n_s16(v50_0, 19);
  int16x8_t v52_0 = (vshrq_n_s16((v51_0), 1));
  int16x8_t v53_0 = vaddq_s16(v47_0, v52_0);
  int32_t v37 = v31 + 1;
  int16x8_t v38_0 = vld1q_s16((int16_t * RADRESTRICT)(v6_0[v37] + 0));
  int32_t v43 = v31 + 4;
  int16x8_t v44_0 = vld1q_s16((int16_t * RADRESTRICT)(v6_0[v43] + 0));
  int16x8_t v48_0 = vaddq_s16(v38_0, v44_0);
  int16x8_t v49_0 = vshlq_n_s16((v48_0), 1);
  int16x8_t v54_0 = vsubq_s16(v53_0, v49_0);
  int16x8_t v36_1 = vld1q_s16((int16_t * RADRESTRICT)(v6_1[v35] + 0));
  int16x8_t v46_1 = vld1q_s16((int16_t * RADRESTRICT)(v6_1[v45] + 0));
  int16x8_t v47_1 = vhaddq_s16(v36_1, v46_1);
  int16x8_t v40_1 = vld1q_s16((int16_t * RADRESTRICT)(v6_1[v39] + 0));
  int16x8_t v42_1 = vld1q_s16((int16_t * RADRESTRICT)(v6_1[v41] + 0));
  int16x8_t v50_1 = vaddq_s16(v40_1, v42_1);
  int16x8_t v51_1 = vmulq_n_s16(v50_1, 19);
  int16x8_t v52_1 = (vshrq_n_s16((v51_1), 1));
  int16x8_t v53_1 = vaddq_s16(v47_1, v52_1);
  int16x8_t v38_1 = vld1q_s16((int16_t * RADRESTRICT)(v6_1[v37] + 0));
  int16x8_t v44_1 = vld1q_s16((int16_t * RADRESTRICT)(v6_1[v43] + 0));
  int16x8_t v48_1 = vaddq_s16(v38_1, v44_1);
  int16x8_t v49_1 = vshlq_n_s16((v48_1), 1);
  int16x8_t v54_1 = vsubq_s16(v53_1, v49_1);
  uint8x16_t v56 = vcombine_u8(vqrshrun_n_s16(v54_0, 4), vqrshrun_n_s16(v54_1, 4));
  vst1q_u8((uint8_t * RADRESTRICT)(v1 + 0), v56);

  {
  char unsigned * RADRESTRICT v58 = v8 + v2;
  v8 = v58;

  {
  char unsigned * RADRESTRICT v60 = v1 + v3;
  v1 = v60;
  }
  }
  }
}
  }
  }
  }
  }
  }
  }
  }
}

typedef void muratori32_16x16_jump(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int32_t sw, int32_t dw);
#if !defined(BINK2_CODEGEN_NO_TABLE)
#ifdef _MSC_VER
_declspec(selectany)
#else
static
#endif
muratori32_16x16_jump *muratori32_16x16[] =
{
  muratori32_16x16_h0_v0,
  muratori32_16x16_h0_v1,
  muratori32_16x16_h1_v0,
  muratori32_16x16_h1_v1,
};
#endif

