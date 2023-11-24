// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#define simd4f float32x4_t
#define simd4i int32x4_t
#define simd4f_0 (vdupq_n_f32(0))
#define simd4i_0 (vdupq_n_s32(0))

static CODEGEN_ATTR void move_u8_16x16(char unsigned * RADRESTRICT d, int32_t dw, char unsigned * RADRESTRICT s, int32_t sw)
{
  {
  {
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(d);
  char unsigned * RADRESTRICT v2 = (char unsigned * RADRESTRICT)(s);
  uint8x16_t v8 = vld1q_u8((uint8_t * RADRESTRICT)(v2 + 0));
  vst1q_u8((uint8_t * RADRESTRICT)(v0 + 0), v8);

  {
  int32_t v1 = dw;
  char unsigned * RADRESTRICT v11 = v0 + v1;
  int32_t v3 = sw;
  char unsigned * RADRESTRICT v10 = v2 + v3;
  uint8x16_t v12 = vld1q_u8((uint8_t * RADRESTRICT)(v10 + 0));
  vst1q_u8((uint8_t * RADRESTRICT)(v11 + 0), v12);

  {
  char unsigned * RADRESTRICT v15 = v11 + v1;
  char unsigned * RADRESTRICT v14 = v10 + v3;
  uint8x16_t v16 = vld1q_u8((uint8_t * RADRESTRICT)(v14 + 0));
  vst1q_u8((uint8_t * RADRESTRICT)(v15 + 0), v16);

  {
  char unsigned * RADRESTRICT v19 = v15 + v1;
  char unsigned * RADRESTRICT v18 = v14 + v3;
  uint8x16_t v20 = vld1q_u8((uint8_t * RADRESTRICT)(v18 + 0));
  vst1q_u8((uint8_t * RADRESTRICT)(v19 + 0), v20);

  {
  char unsigned * RADRESTRICT v23 = v19 + v1;
  char unsigned * RADRESTRICT v22 = v18 + v3;
  uint8x16_t v24 = vld1q_u8((uint8_t * RADRESTRICT)(v22 + 0));
  vst1q_u8((uint8_t * RADRESTRICT)(v23 + 0), v24);

  {
  char unsigned * RADRESTRICT v27 = v23 + v1;
  char unsigned * RADRESTRICT v26 = v22 + v3;
  uint8x16_t v28 = vld1q_u8((uint8_t * RADRESTRICT)(v26 + 0));
  vst1q_u8((uint8_t * RADRESTRICT)(v27 + 0), v28);

  {
  char unsigned * RADRESTRICT v31 = v27 + v1;
  char unsigned * RADRESTRICT v30 = v26 + v3;
  uint8x16_t v32 = vld1q_u8((uint8_t * RADRESTRICT)(v30 + 0));
  vst1q_u8((uint8_t * RADRESTRICT)(v31 + 0), v32);

  {
  char unsigned * RADRESTRICT v35 = v31 + v1;
  char unsigned * RADRESTRICT v34 = v30 + v3;
  uint8x16_t v36 = vld1q_u8((uint8_t * RADRESTRICT)(v34 + 0));
  vst1q_u8((uint8_t * RADRESTRICT)(v35 + 0), v36);

  {
  char unsigned * RADRESTRICT v39 = v35 + v1;
  char unsigned * RADRESTRICT v38 = v34 + v3;
  uint8x16_t v40 = vld1q_u8((uint8_t * RADRESTRICT)(v38 + 0));
  vst1q_u8((uint8_t * RADRESTRICT)(v39 + 0), v40);

  {
  char unsigned * RADRESTRICT v43 = v39 + v1;
  char unsigned * RADRESTRICT v42 = v38 + v3;
  uint8x16_t v44 = vld1q_u8((uint8_t * RADRESTRICT)(v42 + 0));
  vst1q_u8((uint8_t * RADRESTRICT)(v43 + 0), v44);

  {
  char unsigned * RADRESTRICT v47 = v43 + v1;
  char unsigned * RADRESTRICT v46 = v42 + v3;
  uint8x16_t v48 = vld1q_u8((uint8_t * RADRESTRICT)(v46 + 0));
  vst1q_u8((uint8_t * RADRESTRICT)(v47 + 0), v48);

  {
  char unsigned * RADRESTRICT v51 = v47 + v1;
  char unsigned * RADRESTRICT v50 = v46 + v3;
  uint8x16_t v52 = vld1q_u8((uint8_t * RADRESTRICT)(v50 + 0));
  vst1q_u8((uint8_t * RADRESTRICT)(v51 + 0), v52);

  {
  char unsigned * RADRESTRICT v55 = v51 + v1;
  char unsigned * RADRESTRICT v54 = v50 + v3;
  uint8x16_t v56 = vld1q_u8((uint8_t * RADRESTRICT)(v54 + 0));
  vst1q_u8((uint8_t * RADRESTRICT)(v55 + 0), v56);

  {
  char unsigned * RADRESTRICT v59 = v55 + v1;
  char unsigned * RADRESTRICT v58 = v54 + v3;
  uint8x16_t v60 = vld1q_u8((uint8_t * RADRESTRICT)(v58 + 0));
  vst1q_u8((uint8_t * RADRESTRICT)(v59 + 0), v60);

  {
  char unsigned * RADRESTRICT v63 = v59 + v1;
  char unsigned * RADRESTRICT v62 = v58 + v3;
  uint8x16_t v64 = vld1q_u8((uint8_t * RADRESTRICT)(v62 + 0));
  vst1q_u8((uint8_t * RADRESTRICT)(v63 + 0), v64);

  {
  char unsigned * RADRESTRICT v67 = v63 + v1;
  char unsigned * RADRESTRICT v66 = v62 + v3;
  uint8x16_t v68 = vld1q_u8((uint8_t * RADRESTRICT)(v66 + 0));
  vst1q_u8((uint8_t * RADRESTRICT)(v67 + 0), v68);
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
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void move_u8_16x8(char unsigned * RADRESTRICT d, int32_t dw, char unsigned * RADRESTRICT s, int32_t sw)
{
  {
  {
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(d);
  char unsigned * RADRESTRICT v2 = (char unsigned * RADRESTRICT)(s);
  uint8x16_t v8 = vld1q_u8((uint8_t * RADRESTRICT)(v2 + 0));
  vst1q_u8((uint8_t * RADRESTRICT)(v0 + 0), v8);

  {
  int32_t v1 = dw;
  char unsigned * RADRESTRICT v11 = v0 + v1;
  int32_t v3 = sw;
  char unsigned * RADRESTRICT v10 = v2 + v3;
  uint8x16_t v12 = vld1q_u8((uint8_t * RADRESTRICT)(v10 + 0));
  vst1q_u8((uint8_t * RADRESTRICT)(v11 + 0), v12);

  {
  char unsigned * RADRESTRICT v15 = v11 + v1;
  char unsigned * RADRESTRICT v14 = v10 + v3;
  uint8x16_t v16 = vld1q_u8((uint8_t * RADRESTRICT)(v14 + 0));
  vst1q_u8((uint8_t * RADRESTRICT)(v15 + 0), v16);

  {
  char unsigned * RADRESTRICT v19 = v15 + v1;
  char unsigned * RADRESTRICT v18 = v14 + v3;
  uint8x16_t v20 = vld1q_u8((uint8_t * RADRESTRICT)(v18 + 0));
  vst1q_u8((uint8_t * RADRESTRICT)(v19 + 0), v20);

  {
  char unsigned * RADRESTRICT v23 = v19 + v1;
  char unsigned * RADRESTRICT v22 = v18 + v3;
  uint8x16_t v24 = vld1q_u8((uint8_t * RADRESTRICT)(v22 + 0));
  vst1q_u8((uint8_t * RADRESTRICT)(v23 + 0), v24);

  {
  char unsigned * RADRESTRICT v27 = v23 + v1;
  char unsigned * RADRESTRICT v26 = v22 + v3;
  uint8x16_t v28 = vld1q_u8((uint8_t * RADRESTRICT)(v26 + 0));
  vst1q_u8((uint8_t * RADRESTRICT)(v27 + 0), v28);

  {
  char unsigned * RADRESTRICT v31 = v27 + v1;
  char unsigned * RADRESTRICT v30 = v26 + v3;
  uint8x16_t v32 = vld1q_u8((uint8_t * RADRESTRICT)(v30 + 0));
  vst1q_u8((uint8_t * RADRESTRICT)(v31 + 0), v32);

  {
  char unsigned * RADRESTRICT v35 = v31 + v1;
  char unsigned * RADRESTRICT v34 = v30 + v3;
  uint8x16_t v36 = vld1q_u8((uint8_t * RADRESTRICT)(v34 + 0));
  vst1q_u8((uint8_t * RADRESTRICT)(v35 + 0), v36);
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

#define dcs_unpackx8(input, output) \
{\
  { \
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(output); \
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(input); \
  int16x8_t v2 = vld1q_s16((int16_t * RADRESTRICT)(v0 + 0)); \
  int16x8_t v3 = vshlq_n_s16((v2), 4); \
  int32x4_t v4_0 = (vshll_n_s16(vget_low_s16(v3), 16)); \
  int32x4_t v6 = (vshrq_n_s32((v4_0), 20)); \
  vst1q_s32((int32_t * RADRESTRICT)(v1 + 0), v6); \
 \
  { \
  int32x4_t v4_1 = (vshll_n_s16(vget_high_s16(v3), 16)); \
  int32x4_t v9 = (vshrq_n_s32((v4_1), 20)); \
  vst1q_s32((int32_t * RADRESTRICT)(v1 + 16), v9); \
  } \
  } \
}

#define dcs_unpackx4(input, output) \
{\
  { \
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(output); \
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(input); \
  int16x8_t v2 = vld1q_s16((int16_t * RADRESTRICT)(v0 + 0)); \
  int16x8_t v3 = vshlq_n_s16((v2), 4); \
  int32x4_t v4_0 = (vshll_n_s16(vget_low_s16(v3), 16)); \
  int32x4_t v6 = (vshrq_n_s32((v4_0), 20)); \
  vst1q_s32((int32_t * RADRESTRICT)(v1 + 0), v6); \
  } \
}

static CODEGEN_ATTR void calc_average8_u8(char unsigned * RADRESTRICT out, char unsigned * RADRESTRICT ptr, int32_t w)
{
  {
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(ptr);
  uint8x8_t v3 = vld1_u8((uint8_t * RADRESTRICT)(v1 + 0));
  int32_t v2 = w;
  char unsigned * RADRESTRICT v4 = v1 + v2;
  uint8x8_t v5 = vld1_u8((uint8_t * RADRESTRICT)(v4 + 0));
  uint8x8_t v23 = vrhadd_u8(v3, v5);
  char unsigned * RADRESTRICT v6 = v4 + v2;
  uint8x8_t v7 = vld1_u8((uint8_t * RADRESTRICT)(v6 + 0));
  char unsigned * RADRESTRICT v8 = v6 + v2;
  uint8x8_t v9 = vld1_u8((uint8_t * RADRESTRICT)(v8 + 0));
  uint8x8_t v22 = vrhadd_u8(v7, v9);
  uint8x8_t v24 = vrhadd_u8(v23, v22);
  char unsigned * RADRESTRICT v10 = v8 + v2;
  uint8x8_t v11 = vld1_u8((uint8_t * RADRESTRICT)(v10 + 0));
  char unsigned * RADRESTRICT v12 = v10 + v2;
  uint8x8_t v13 = vld1_u8((uint8_t * RADRESTRICT)(v12 + 0));
  uint8x8_t v20 = vrhadd_u8(v11, v13);
  char unsigned * RADRESTRICT v14 = v12 + v2;
  uint8x8_t v15 = vld1_u8((uint8_t * RADRESTRICT)(v14 + 0));
  char unsigned * RADRESTRICT v16 = v14 + v2;
  uint8x8_t v17 = vld1_u8((uint8_t * RADRESTRICT)(v16 + 0));
  uint8x8_t v19 = vrhadd_u8(v15, v17);
  uint8x8_t v21 = vrhadd_u8(v20, v19);
  uint8x8_t v25 = vrhadd_u8(v24, v21);
  int16x4_t F0 = vreinterpret_s16_u16(vpaddl_u8(v25));
  int32x2_t F1 = vpaddl_s16(F0);
  int64x1_t F2 = vpaddl_s32(F1);
  int32x4_t v26 = vreinterpretq_s32_s64(vcombine_s64(F2, F2));
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(out);
  vst1q_lane_s32((int32_t * RADRESTRICT)(v0 + 4*0), v26, 0);
  }
}

static CODEGEN_ATTR void calc_average2x8_u8(char unsigned * RADRESTRICT out, char unsigned * RADRESTRICT ptr, int32_t w)
{
  {
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(out);
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(ptr);
  uint8x16_t v3 = vld1q_u8((uint8_t * RADRESTRICT)(v1 + 0));
  int32_t v2 = w;
  char unsigned * RADRESTRICT v5 = v1 + v2;
  uint8x16_t v6 = vld1q_u8((uint8_t * RADRESTRICT)(v5 + 0));
  uint8x16_t v31 = vrhaddq_u8(v3, v6);
  char unsigned * RADRESTRICT v8 = v5 + v2;
  uint8x16_t v9 = vld1q_u8((uint8_t * RADRESTRICT)(v8 + 0));
  char unsigned * RADRESTRICT v11 = v8 + v2;
  uint8x16_t v12 = vld1q_u8((uint8_t * RADRESTRICT)(v11 + 0));
  uint8x16_t v30 = vrhaddq_u8(v9, v12);
  uint8x16_t v32 = vrhaddq_u8(v31, v30);
  char unsigned * RADRESTRICT v14 = v11 + v2;
  uint8x16_t v15 = vld1q_u8((uint8_t * RADRESTRICT)(v14 + 0));
  char unsigned * RADRESTRICT v17 = v14 + v2;
  uint8x16_t v18 = vld1q_u8((uint8_t * RADRESTRICT)(v17 + 0));
  uint8x16_t v28 = vrhaddq_u8(v15, v18);
  char unsigned * RADRESTRICT v20 = v17 + v2;
  uint8x16_t v21 = vld1q_u8((uint8_t * RADRESTRICT)(v20 + 0));
  char unsigned * RADRESTRICT v23 = v20 + v2;
  uint8x16_t v24 = vld1q_u8((uint8_t * RADRESTRICT)(v23 + 0));
  uint8x16_t v27 = vrhaddq_u8(v21, v24);
  uint8x16_t v29 = vrhaddq_u8(v28, v27);
  uint8x16_t v33 = vrhaddq_u8(v32, v29);
  int16x8_t F0 = vreinterpretq_s16_u16(vpaddlq_u8(v33));
  int32x4_t F1 = vpaddlq_s16(F0);
  int16x8_t F2 = vreinterpretq_s16_u16(vpaddlq_u8(v33));
  int32x4_t F3 = vpaddlq_s16(F2);
  int16x8_t F4 = vcombine_s16(vmovn_s32(F1), vmovn_s32(F3));
  int32x4_t v41 = vpaddlq_s16(F4);
  vst1_s32((int32_t * RADRESTRICT)(v0 + 0), vget_low_s32(v41));
  }
}

static CODEGEN_ATTR void calc_average4x8_u8(char unsigned * RADRESTRICT out, char unsigned * RADRESTRICT ptr, int32_t w)
{
  {
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(out);
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(ptr);
  uint8x16_t v3 = vld1q_u8((uint8_t * RADRESTRICT)(v1 + 0));
  int32_t v2 = w;
  char unsigned * RADRESTRICT v5 = v1 + v2;
  uint8x16_t v6 = vld1q_u8((uint8_t * RADRESTRICT)(v5 + 0));
  uint8x16_t v31 = vrhaddq_u8(v3, v6);
  char unsigned * RADRESTRICT v8 = v5 + v2;
  uint8x16_t v9 = vld1q_u8((uint8_t * RADRESTRICT)(v8 + 0));
  char unsigned * RADRESTRICT v11 = v8 + v2;
  uint8x16_t v12 = vld1q_u8((uint8_t * RADRESTRICT)(v11 + 0));
  uint8x16_t v30 = vrhaddq_u8(v9, v12);
  uint8x16_t v32 = vrhaddq_u8(v31, v30);
  char unsigned * RADRESTRICT v14 = v11 + v2;
  uint8x16_t v15 = vld1q_u8((uint8_t * RADRESTRICT)(v14 + 0));
  char unsigned * RADRESTRICT v17 = v14 + v2;
  uint8x16_t v18 = vld1q_u8((uint8_t * RADRESTRICT)(v17 + 0));
  uint8x16_t v28 = vrhaddq_u8(v15, v18);
  char unsigned * RADRESTRICT v20 = v17 + v2;
  uint8x16_t v21 = vld1q_u8((uint8_t * RADRESTRICT)(v20 + 0));
  char unsigned * RADRESTRICT v23 = v20 + v2;
  uint8x16_t v24 = vld1q_u8((uint8_t * RADRESTRICT)(v23 + 0));
  uint8x16_t v27 = vrhaddq_u8(v21, v24);
  uint8x16_t v29 = vrhaddq_u8(v28, v27);
  uint8x16_t v33 = vrhaddq_u8(v32, v29);
  uint8x16_t v4 = vld1q_u8((uint8_t * RADRESTRICT)(v1 + 16));
  uint8x16_t v7 = vld1q_u8((uint8_t * RADRESTRICT)(v5 + 16));
  uint8x16_t v38 = vrhaddq_u8(v4, v7);
  uint8x16_t v10 = vld1q_u8((uint8_t * RADRESTRICT)(v8 + 16));
  uint8x16_t v13 = vld1q_u8((uint8_t * RADRESTRICT)(v11 + 16));
  uint8x16_t v37 = vrhaddq_u8(v10, v13);
  uint8x16_t v39 = vrhaddq_u8(v38, v37);
  uint8x16_t v16 = vld1q_u8((uint8_t * RADRESTRICT)(v14 + 16));
  uint8x16_t v19 = vld1q_u8((uint8_t * RADRESTRICT)(v17 + 16));
  uint8x16_t v35 = vrhaddq_u8(v16, v19);
  uint8x16_t v22 = vld1q_u8((uint8_t * RADRESTRICT)(v20 + 16));
  uint8x16_t v25 = vld1q_u8((uint8_t * RADRESTRICT)(v23 + 16));
  uint8x16_t v34 = vrhaddq_u8(v22, v25);
  uint8x16_t v36 = vrhaddq_u8(v35, v34);
  uint8x16_t v40 = vrhaddq_u8(v39, v36);
  int16x8_t F0 = vreinterpretq_s16_u16(vpaddlq_u8(v33));
  int32x4_t F1 = vpaddlq_s16(F0);
  int16x8_t F2 = vreinterpretq_s16_u16(vpaddlq_u8(v40));
  int32x4_t F3 = vpaddlq_s16(F2);
  int16x8_t F4 = vcombine_s16(vmovn_s32(F1), vmovn_s32(F3));
  int32x4_t v41 = vpaddlq_s16(F4);
  vst1q_s32((int32_t * RADRESTRICT)(v0 + 0), v41);
  }
}

static CODEGEN_ATTR void pack16x8_to_u8(char unsigned * RADRESTRICT dst, int32_t dst_stride, char unsigned * RADRESTRICT src, int32_t src_stride)
{
  {
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(dst);

  {
  char unsigned * RADRESTRICT v2 = (char unsigned * RADRESTRICT)(src);

  {
{int32_t v4;
for(v4 = 0; v4 < 8; v4 += 1)
  {
  int16x8_t v8_0 = vld1q_s16((int16_t * RADRESTRICT)(v2 + 0));
  int16x8_t v8_1 = vld1q_s16((int16_t * RADRESTRICT)(v2 + 16));
  uint8x16_t v9 = vcombine_u8(vqmovun_s16(v8_0), vqmovun_s16(v8_1));
  vst1q_u8((uint8_t * RADRESTRICT)(v0 + 0), v9);

  {
  int32_t v3 = src_stride;
  char unsigned * RADRESTRICT v11 = v2 + v3;
  v2 = v11;

  {
  int32_t v1 = dst_stride;
  char unsigned * RADRESTRICT v13 = v0 + v1;
  v0 = v13;
  }
  }
  }
}
  }
  }
  }
}

static CODEGEN_ATTR void pack16x8_to_u8_and_mocomp(char unsigned * RADRESTRICT dst, int32_t dst_stride, char unsigned * RADRESTRICT src, int32_t src_stride, char unsigned * RADRESTRICT motion, int32_t motion_stride)
{
  {
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(dst);

  {
  char unsigned * RADRESTRICT v2 = (char unsigned * RADRESTRICT)(src);

  {
  char unsigned * RADRESTRICT v4 = (char unsigned * RADRESTRICT)(motion);

  {
{int32_t v6;
for(v6 = 0; v6 < 8; v6 += 1)
  {
  int16x8_t v10_0 = vld1q_s16((int16_t * RADRESTRICT)(v2 + 0));
  uint8x16_t v11 = vld1q_u8((uint8_t * RADRESTRICT)(v4 + 0));
  int16x8_t v12_0 = vreinterpretq_s16_u16(vaddw_u8(vreinterpretq_u16_s16(v10_0), vget_low_u8(v11)));
  int16x8_t v10_1 = vld1q_s16((int16_t * RADRESTRICT)(v2 + 16));
  int16x8_t v12_1 = vreinterpretq_s16_u16(vaddw_u8(vreinterpretq_u16_s16(v10_1), vget_high_u8(v11)));
  uint8x16_t v13 = vcombine_u8(vqmovun_s16(v12_0), vqmovun_s16(v12_1));
  vst1q_u8((uint8_t * RADRESTRICT)(v0 + 0), v13);

  {
  int32_t v3 = src_stride;
  char unsigned * RADRESTRICT v15 = v2 + v3;
  v2 = v15;

  {
  int32_t v1 = dst_stride;
  char unsigned * RADRESTRICT v17 = v0 + v1;
  v0 = v17;

  {
  int32_t v5 = motion_stride;
  char unsigned * RADRESTRICT v19 = v4 + v5;
  v4 = v19;
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

