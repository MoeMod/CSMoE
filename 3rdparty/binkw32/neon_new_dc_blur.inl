// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

static CODEGEN_ATTR void new_dc_blur_r0_l0(char unsigned * RADRESTRICT dcs0, char unsigned * RADRESTRICT dcs1, char unsigned * RADRESTRICT dcs2, char unsigned * RADRESTRICT t, char unsigned * RADRESTRICT dest, int32_t dw)
{
}

static CODEGEN_ATTR void new_dc_blur_r0_l1(char unsigned * RADRESTRICT dcs0, char unsigned * RADRESTRICT dcs1, char unsigned * RADRESTRICT dcs2, char unsigned * RADRESTRICT t, char unsigned * RADRESTRICT dest, int32_t dw)
{
  {
  char unsigned * RADRESTRICT v16 = (char unsigned * RADRESTRICT)(dest);
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(dcs0);
  int16x8_t v4 = vld1q_s16((int16_t * RADRESTRICT)(v0 + 0));
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dcs1);
  int16x8_t v5 = vld1q_s16((int16_t * RADRESTRICT)(v1 + 0));
  char unsigned * RADRESTRICT v2 = (char unsigned * RADRESTRICT)(dcs2);
  int16x8_t v6 = vld1q_s16((int16_t * RADRESTRICT)(v2 + 0));
  char unsigned * RADRESTRICT v3 = (char unsigned * RADRESTRICT)(t);
  int16x8_t v7 = vld1q_s16((int16_t * RADRESTRICT)(v3 + 0));
  int16x8_t X = v4;
  int16x8_t Y = v5;
  int16x8_t Z = v6;
  int16x8_t T = v7;
  int16x8_t sY = vextq_s16(Y, Y, 1);
  int16x8_t C = vreinterpretq_s16_s32(vdupq_lane_s32(vreinterpret_s32_s16(vget_low_s16(sY)), 0));
  int32x2x2_t mXZ = vzip_s32(vget_low_s32(vreinterpretq_s32_s16(vextq_s16(X, X, 1))), vget_low_s32(vreinterpretq_s32_s16(vextq_s16(Z, Z, 1))));
  int16x8_t P = vcombine_s16(vget_low_s16(Y), vreinterpret_s16_s32(mXZ.val[0]));
  int16x8_t S = vcombine_s16(vget_low_s16(Z), vget_low_s16(X));
  uint16x8_t Mcp = vshrq_n_u16(vcgtq_s16(T, vabdq_s16(C, P)), 3);
  uint16x8_t Mcs = vshrq_n_u16(vcgtq_s16(T, vabdq_s16(C, S)), 3);
  int16x8_t v8_0 = vshrq_n_s16(vshlq_n_s16(C, 3), 3);
  int16x8_t v8_1 = vbslq_s16(Mcp, P, v8_0);
  int16x8_t R = vcombine_s16(vget_low_s16(v8_1), vget_low_s16(v8_1));
  int16x8_t v8_2 = vbslq_s16(Mcs, S, R);
  int32x2_t Slo = vreinterpret_s32_s16(vget_low_s16(v8_2));
  int32x2_t Shi = vreinterpret_s32_s16(vget_high_s16(v8_2));
  int32x4_t Sswap = vcombine_s32(Shi, Slo);
  int32x4_t P32 = vreinterpretq_s32_s16(v8_1);
  int32x4x2_t SPmix = vuzpq_s32(Sswap, P32);
  int16x8_t v12_0 = vreinterpretq_s16_s32(SPmix.val[0]);
  int16x8_t v12_1 = vreinterpretq_s16_s32(SPmix.val[1]);
  int16x8_t v12_2 = vcombine_s16(vget_high_s16(v8_1), vget_low_s16(v8_0));
  int16x8_t v18 = vsubq_s16(v12_0, v12_2);
  int16x8_t v20 = vaddq_s16(v12_2, vdupq_n_s16(4));
  int16x8_t v21 = vaddq_s16(v20, vqdmulhq_s16(v18, vdupq_n_s16(13312)));
  int16x8_t v19 = vsubq_s16(v12_1, v12_2);
  int16x8_t v28 = vaddq_s16(v20, vqdmulhq_s16(v19, vdupq_n_s16(3584)));
  int16x8_t v29 = vsraq_n_s16(v28, v18, 4);
  int16x8_t v42_0 = vcombine_s16(vget_low_s16(v21), vget_low_s16(v29));
  int16x8_t v24 = vaddq_s16(v20, vqdmulhq_s16(v18, vdupq_n_s16(6144)));
  int16x8_t v25 = vsraq_n_s16(v24, v19, 5);
  int16x8_t v32 = vaddq_s16(v20, vqdmulhq_s16(v19, vdupq_n_s16(9216)));
  int16x8_t v33 = vsraq_n_s16(v32, v18, 7);
  int16x8_t v42_2 = vcombine_s16(vget_low_s16(v25), vget_low_s16(v33));
  int16x8_t v43_0 = vreinterpretq_s16_s32(vtrnq_s32(vreinterpretq_s32_s16(v42_0), vreinterpretq_s32_s16(v42_2)).val[0]);
  int16x8_t v22 = vaddq_s16(v20, vqdmulhq_s16(v18, vdupq_n_s16(9216)));
  int16x8_t v23 = vsraq_n_s16(v22, v19, 7);
  int16x8_t v30 = vaddq_s16(v20, vqdmulhq_s16(v19, vdupq_n_s16(6144)));
  int16x8_t v31 = vsraq_n_s16(v30, v18, 5);
  int16x8_t v42_1 = vcombine_s16(vget_low_s16(v23), vget_low_s16(v31));
  int16x8_t v26 = vaddq_s16(v20, vqdmulhq_s16(v18, vdupq_n_s16(3584)));
  int16x8_t v27 = vsraq_n_s16(v26, v19, 4);
  int16x8_t v34 = vaddq_s16(v20, vqdmulhq_s16(v19, vdupq_n_s16(13312)));
  int16x8_t v42_3 = vcombine_s16(vget_low_s16(v27), vget_low_s16(v34));
  int16x8_t v43_1 = vreinterpretq_s16_s32(vtrnq_s32(vreinterpretq_s32_s16(v42_1), vreinterpretq_s32_s16(v42_3)).val[0]);
  int16x8_t v44_0 = vtrnq_s16(v43_0, v43_1).val[0];
  int16x8_t v42_4 = vcombine_s16(vget_high_s16(v21), vget_high_s16(v29));
  int16x8_t v42_6 = vcombine_s16(vget_high_s16(v25), vget_high_s16(v33));
  int16x8_t v43_4 = vreinterpretq_s16_s32(vtrnq_s32(vreinterpretq_s32_s16(v42_4), vreinterpretq_s32_s16(v42_6)).val[0]);
  int16x8_t v42_5 = vcombine_s16(vget_high_s16(v23), vget_high_s16(v31));
  int16x8_t v42_7 = vcombine_s16(vget_high_s16(v27), vget_high_s16(v34));
  int16x8_t v43_5 = vreinterpretq_s16_s32(vtrnq_s32(vreinterpretq_s32_s16(v42_5), vreinterpretq_s32_s16(v42_7)).val[0]);
  int16x8_t v44_4 = vtrnq_s16(v43_4, v43_5).val[0];
  int16x8_t v48 = vsubq_s16(v44_0, v44_4);
  int16x8_t v50 = vaddq_s16(v44_4, vqdmulhq_s16(v48, vdupq_n_s16(13312)));
  uint8x8_t v83 = vqshrun_n_s16(v50, 3);
  vst1_u8((uint8_t * RADRESTRICT)(v16 + 0), v83);

  {
  int32_t v17 = dw;
  char unsigned * RADRESTRICT v85 = v16 + v17;
  int16x8_t v51 = vaddq_s16(v44_4, vqdmulhq_s16(v48, vdupq_n_s16(9216)));
  int16x8_t v43_2 = vreinterpretq_s16_s32(vtrnq_s32(vreinterpretq_s32_s16(v42_0), vreinterpretq_s32_s16(v42_2)).val[1]);
  int16x8_t v43_3 = vreinterpretq_s16_s32(vtrnq_s32(vreinterpretq_s32_s16(v42_1), vreinterpretq_s32_s16(v42_3)).val[1]);
  int16x8_t v44_2 = vtrnq_s16(v43_2, v43_3).val[0];
  int16x8_t v49 = vsubq_s16(v44_2, v44_4);
  int16x8_t v52 = vsraq_n_s16(v51, v49, 7);
  uint8x8_t v86 = vqshrun_n_s16(v52, 3);
  vst1_u8((uint8_t * RADRESTRICT)(v85 + 0), v86);

  {
  char unsigned * RADRESTRICT v88 = v85 + v17;
  int16x8_t v53 = vaddq_s16(v44_4, vqdmulhq_s16(v48, vdupq_n_s16(6144)));
  int16x8_t v54 = vsraq_n_s16(v53, v49, 5);
  uint8x8_t v89 = vqshrun_n_s16(v54, 3);
  vst1_u8((uint8_t * RADRESTRICT)(v88 + 0), v89);

  {
  char unsigned * RADRESTRICT v91 = v88 + v17;
  int16x8_t v55 = vaddq_s16(v44_4, vqdmulhq_s16(v48, vdupq_n_s16(3584)));
  int16x8_t v56 = vsraq_n_s16(v55, v49, 4);
  uint8x8_t v92 = vqshrun_n_s16(v56, 3);
  vst1_u8((uint8_t * RADRESTRICT)(v91 + 0), v92);

  {
  char unsigned * RADRESTRICT v94 = v91 + v17;
  int16x8_t v57 = vaddq_s16(v44_4, vqdmulhq_s16(v49, vdupq_n_s16(3584)));
  int16x8_t v58 = vsraq_n_s16(v57, v48, 4);
  uint8x8_t v95 = vqshrun_n_s16(v58, 3);
  vst1_u8((uint8_t * RADRESTRICT)(v94 + 0), v95);

  {
  char unsigned * RADRESTRICT v97 = v94 + v17;
  int16x8_t v59 = vaddq_s16(v44_4, vqdmulhq_s16(v49, vdupq_n_s16(6144)));
  int16x8_t v60 = vsraq_n_s16(v59, v48, 5);
  uint8x8_t v98 = vqshrun_n_s16(v60, 3);
  vst1_u8((uint8_t * RADRESTRICT)(v97 + 0), v98);

  {
  char unsigned * RADRESTRICT v100 = v97 + v17;
  int16x8_t v61 = vaddq_s16(v44_4, vqdmulhq_s16(v49, vdupq_n_s16(9216)));
  int16x8_t v62 = vsraq_n_s16(v61, v48, 7);
  uint8x8_t v101 = vqshrun_n_s16(v62, 3);
  vst1_u8((uint8_t * RADRESTRICT)(v100 + 0), v101);

  {
  char unsigned * RADRESTRICT v103 = v100 + v17;
  int16x8_t v63 = vaddq_s16(v44_4, vqdmulhq_s16(v49, vdupq_n_s16(13312)));
  uint8x8_t v104 = vqshrun_n_s16(v63, 3);
  vst1_u8((uint8_t * RADRESTRICT)(v103 + 0), v104);
  }
  }
  }
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void new_dc_blur_r1_l0(char unsigned * RADRESTRICT dcs0, char unsigned * RADRESTRICT dcs1, char unsigned * RADRESTRICT dcs2, char unsigned * RADRESTRICT t, char unsigned * RADRESTRICT dest, int32_t dw)
{
  {
  char unsigned * RADRESTRICT v16 = (char unsigned * RADRESTRICT)(dest);
  char unsigned * RADRESTRICT v18 = v16 + 8;
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(dcs0);
  int16x8_t v4 = vld1q_s16((int16_t * RADRESTRICT)(v0 + 0));
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dcs1);
  int16x8_t v5 = vld1q_s16((int16_t * RADRESTRICT)(v1 + 0));
  char unsigned * RADRESTRICT v2 = (char unsigned * RADRESTRICT)(dcs2);
  int16x8_t v6 = vld1q_s16((int16_t * RADRESTRICT)(v2 + 0));
  char unsigned * RADRESTRICT v3 = (char unsigned * RADRESTRICT)(t);
  int16x8_t v7 = vld1q_s16((int16_t * RADRESTRICT)(v3 + 0));
  int16x8_t X = v4;
  int16x8_t Y = v5;
  int16x8_t Z = v6;
  int16x8_t T = v7;
  int16x8_t sY = vextq_s16(Y, Y, 1);
  int16x8_t C = vreinterpretq_s16_s32(vdupq_lane_s32(vreinterpret_s32_s16(vget_low_s16(sY)), 0));
  int32x2x2_t mXZ = vzip_s32(vget_low_s32(vreinterpretq_s32_s16(vextq_s16(X, X, 1))), vget_low_s32(vreinterpretq_s32_s16(vextq_s16(Z, Z, 1))));
  int16x8_t P = vcombine_s16(vget_low_s16(Y), vreinterpret_s16_s32(mXZ.val[0]));
  int16x8_t S = vcombine_s16(vget_low_s16(Z), vget_low_s16(X));
  uint16x8_t Mcp = vshrq_n_u16(vcgtq_s16(T, vabdq_s16(C, P)), 3);
  uint16x8_t Mcs = vshrq_n_u16(vcgtq_s16(T, vabdq_s16(C, S)), 3);
  int16x8_t v8_0 = vshrq_n_s16(vshlq_n_s16(C, 3), 3);
  int16x8_t v8_1 = vbslq_s16(Mcp, P, v8_0);
  int16x8_t R = vcombine_s16(vget_low_s16(v8_1), vget_low_s16(v8_1));
  int16x8_t v8_2 = vbslq_s16(Mcs, S, R);
  int32x2_t Slo = vreinterpret_s32_s16(vget_low_s16(v8_2));
  int32x2_t Shi = vreinterpret_s32_s16(vget_high_s16(v8_2));
  int32x4_t Sswap = vcombine_s32(Shi, Slo);
  int32x4_t P32 = vreinterpretq_s32_s16(v8_1);
  int32x4x2_t SPmix = vuzpq_s32(Sswap, P32);
  int16x8_t v12_0 = vreinterpretq_s16_s32(SPmix.val[0]);
  int16x8_t v12_1 = vreinterpretq_s16_s32(SPmix.val[1]);
  int16x8_t v12_2 = vcombine_s16(vget_high_s16(v8_1), vget_low_s16(v8_0));
  int16x8_t v19 = vsubq_s16(v12_0, v12_2);
  int16x8_t v21 = vaddq_s16(v12_2, vdupq_n_s16(4));
  int16x8_t v22 = vaddq_s16(v21, vqdmulhq_s16(v19, vdupq_n_s16(13312)));
  int16x8_t v20 = vsubq_s16(v12_1, v12_2);
  int16x8_t v29 = vaddq_s16(v21, vqdmulhq_s16(v20, vdupq_n_s16(3584)));
  int16x8_t v30 = vsraq_n_s16(v29, v19, 4);
  int16x8_t v43_0 = vcombine_s16(vget_low_s16(v22), vget_low_s16(v30));
  int16x8_t v25 = vaddq_s16(v21, vqdmulhq_s16(v19, vdupq_n_s16(6144)));
  int16x8_t v26 = vsraq_n_s16(v25, v20, 5);
  int16x8_t v33 = vaddq_s16(v21, vqdmulhq_s16(v20, vdupq_n_s16(9216)));
  int16x8_t v34 = vsraq_n_s16(v33, v19, 7);
  int16x8_t v43_2 = vcombine_s16(vget_low_s16(v26), vget_low_s16(v34));
  int16x8_t v44_0 = vreinterpretq_s16_s32(vtrnq_s32(vreinterpretq_s32_s16(v43_0), vreinterpretq_s32_s16(v43_2)).val[0]);
  int16x8_t v23 = vaddq_s16(v21, vqdmulhq_s16(v19, vdupq_n_s16(9216)));
  int16x8_t v24 = vsraq_n_s16(v23, v20, 7);
  int16x8_t v31 = vaddq_s16(v21, vqdmulhq_s16(v20, vdupq_n_s16(6144)));
  int16x8_t v32 = vsraq_n_s16(v31, v19, 5);
  int16x8_t v43_1 = vcombine_s16(vget_low_s16(v24), vget_low_s16(v32));
  int16x8_t v27 = vaddq_s16(v21, vqdmulhq_s16(v19, vdupq_n_s16(3584)));
  int16x8_t v28 = vsraq_n_s16(v27, v20, 4);
  int16x8_t v35 = vaddq_s16(v21, vqdmulhq_s16(v20, vdupq_n_s16(13312)));
  int16x8_t v43_3 = vcombine_s16(vget_low_s16(v28), vget_low_s16(v35));
  int16x8_t v44_1 = vreinterpretq_s16_s32(vtrnq_s32(vreinterpretq_s32_s16(v43_1), vreinterpretq_s32_s16(v43_3)).val[0]);
  int16x8_t v45_1 = vtrnq_s16(v44_0, v44_1).val[1];
  int16x8_t v43_4 = vcombine_s16(vget_high_s16(v22), vget_high_s16(v30));
  int16x8_t v43_6 = vcombine_s16(vget_high_s16(v26), vget_high_s16(v34));
  int16x8_t v44_4 = vreinterpretq_s16_s32(vtrnq_s32(vreinterpretq_s32_s16(v43_4), vreinterpretq_s32_s16(v43_6)).val[0]);
  int16x8_t v43_5 = vcombine_s16(vget_high_s16(v24), vget_high_s16(v32));
  int16x8_t v43_7 = vcombine_s16(vget_high_s16(v28), vget_high_s16(v35));
  int16x8_t v44_5 = vreinterpretq_s16_s32(vtrnq_s32(vreinterpretq_s32_s16(v43_5), vreinterpretq_s32_s16(v43_7)).val[0]);
  int16x8_t v45_5 = vtrnq_s16(v44_4, v44_5).val[1];
  int16x8_t v68 = vsubq_s16(v45_1, v45_5);
  int16x8_t v70 = vaddq_s16(v45_5, vqdmulhq_s16(v68, vdupq_n_s16(13312)));
  uint8x8_t v84 = vqshrun_n_s16(v70, 3);
  vst1_u8((uint8_t * RADRESTRICT)(v18 + 0), v84);

  {
  int32_t v17 = dw;
  char unsigned * RADRESTRICT v86 = v18 + v17;
  int16x8_t v71 = vaddq_s16(v45_5, vqdmulhq_s16(v68, vdupq_n_s16(9216)));
  int16x8_t v44_2 = vreinterpretq_s16_s32(vtrnq_s32(vreinterpretq_s32_s16(v43_0), vreinterpretq_s32_s16(v43_2)).val[1]);
  int16x8_t v44_3 = vreinterpretq_s16_s32(vtrnq_s32(vreinterpretq_s32_s16(v43_1), vreinterpretq_s32_s16(v43_3)).val[1]);
  int16x8_t v45_3 = vtrnq_s16(v44_2, v44_3).val[1];
  int16x8_t v69 = vsubq_s16(v45_3, v45_5);
  int16x8_t v72 = vsraq_n_s16(v71, v69, 7);
  uint8x8_t v87 = vqshrun_n_s16(v72, 3);
  vst1_u8((uint8_t * RADRESTRICT)(v86 + 0), v87);

  {
  char unsigned * RADRESTRICT v89 = v86 + v17;
  int16x8_t v73 = vaddq_s16(v45_5, vqdmulhq_s16(v68, vdupq_n_s16(6144)));
  int16x8_t v74 = vsraq_n_s16(v73, v69, 5);
  uint8x8_t v90 = vqshrun_n_s16(v74, 3);
  vst1_u8((uint8_t * RADRESTRICT)(v89 + 0), v90);

  {
  char unsigned * RADRESTRICT v92 = v89 + v17;
  int16x8_t v75 = vaddq_s16(v45_5, vqdmulhq_s16(v68, vdupq_n_s16(3584)));
  int16x8_t v76 = vsraq_n_s16(v75, v69, 4);
  uint8x8_t v93 = vqshrun_n_s16(v76, 3);
  vst1_u8((uint8_t * RADRESTRICT)(v92 + 0), v93);

  {
  char unsigned * RADRESTRICT v95 = v92 + v17;
  int16x8_t v77 = vaddq_s16(v45_5, vqdmulhq_s16(v69, vdupq_n_s16(3584)));
  int16x8_t v78 = vsraq_n_s16(v77, v68, 4);
  uint8x8_t v96 = vqshrun_n_s16(v78, 3);
  vst1_u8((uint8_t * RADRESTRICT)(v95 + 0), v96);

  {
  char unsigned * RADRESTRICT v98 = v95 + v17;
  int16x8_t v79 = vaddq_s16(v45_5, vqdmulhq_s16(v69, vdupq_n_s16(6144)));
  int16x8_t v80 = vsraq_n_s16(v79, v68, 5);
  uint8x8_t v99 = vqshrun_n_s16(v80, 3);
  vst1_u8((uint8_t * RADRESTRICT)(v98 + 0), v99);

  {
  char unsigned * RADRESTRICT v101 = v98 + v17;
  int16x8_t v81 = vaddq_s16(v45_5, vqdmulhq_s16(v69, vdupq_n_s16(9216)));
  int16x8_t v82 = vsraq_n_s16(v81, v68, 7);
  uint8x8_t v102 = vqshrun_n_s16(v82, 3);
  vst1_u8((uint8_t * RADRESTRICT)(v101 + 0), v102);

  {
  char unsigned * RADRESTRICT v104 = v101 + v17;
  int16x8_t v83 = vaddq_s16(v45_5, vqdmulhq_s16(v69, vdupq_n_s16(13312)));
  uint8x8_t v105 = vqshrun_n_s16(v83, 3);
  vst1_u8((uint8_t * RADRESTRICT)(v104 + 0), v105);
  }
  }
  }
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void new_dc_blur_r1_l1(char unsigned * RADRESTRICT dcs0, char unsigned * RADRESTRICT dcs1, char unsigned * RADRESTRICT dcs2, char unsigned * RADRESTRICT t, char unsigned * RADRESTRICT dest, int32_t dw)
{
  {
  char unsigned * RADRESTRICT v16 = (char unsigned * RADRESTRICT)(dest);
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(dcs0);
  int16x8_t v4 = vld1q_s16((int16_t * RADRESTRICT)(v0 + 0));
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dcs1);
  int16x8_t v5 = vld1q_s16((int16_t * RADRESTRICT)(v1 + 0));
  char unsigned * RADRESTRICT v2 = (char unsigned * RADRESTRICT)(dcs2);
  int16x8_t v6 = vld1q_s16((int16_t * RADRESTRICT)(v2 + 0));
  char unsigned * RADRESTRICT v3 = (char unsigned * RADRESTRICT)(t);
  int16x8_t v7 = vld1q_s16((int16_t * RADRESTRICT)(v3 + 0));
  int16x8_t X = v4;
  int16x8_t Y = v5;
  int16x8_t Z = v6;
  int16x8_t T = v7;
  int16x8_t sY = vextq_s16(Y, Y, 1);
  int16x8_t C = vreinterpretq_s16_s32(vdupq_lane_s32(vreinterpret_s32_s16(vget_low_s16(sY)), 0));
  int32x2x2_t mXZ = vzip_s32(vget_low_s32(vreinterpretq_s32_s16(vextq_s16(X, X, 1))), vget_low_s32(vreinterpretq_s32_s16(vextq_s16(Z, Z, 1))));
  int16x8_t P = vcombine_s16(vget_low_s16(Y), vreinterpret_s16_s32(mXZ.val[0]));
  int16x8_t S = vcombine_s16(vget_low_s16(Z), vget_low_s16(X));
  uint16x8_t Mcp = vshrq_n_u16(vcgtq_s16(T, vabdq_s16(C, P)), 3);
  uint16x8_t Mcs = vshrq_n_u16(vcgtq_s16(T, vabdq_s16(C, S)), 3);
  int16x8_t v8_0 = vshrq_n_s16(vshlq_n_s16(C, 3), 3);
  int16x8_t v8_1 = vbslq_s16(Mcp, P, v8_0);
  int16x8_t R = vcombine_s16(vget_low_s16(v8_1), vget_low_s16(v8_1));
  int16x8_t v8_2 = vbslq_s16(Mcs, S, R);
  int32x2_t Slo = vreinterpret_s32_s16(vget_low_s16(v8_2));
  int32x2_t Shi = vreinterpret_s32_s16(vget_high_s16(v8_2));
  int32x4_t Sswap = vcombine_s32(Shi, Slo);
  int32x4_t P32 = vreinterpretq_s32_s16(v8_1);
  int32x4x2_t SPmix = vuzpq_s32(Sswap, P32);
  int16x8_t v12_0 = vreinterpretq_s16_s32(SPmix.val[0]);
  int16x8_t v12_1 = vreinterpretq_s16_s32(SPmix.val[1]);
  int16x8_t v12_2 = vcombine_s16(vget_high_s16(v8_1), vget_low_s16(v8_0));
  int16x8_t v18 = vsubq_s16(v12_0, v12_2);
  int16x8_t v20 = vaddq_s16(v12_2, vdupq_n_s16(4));
  int16x8_t v21 = vaddq_s16(v20, vqdmulhq_s16(v18, vdupq_n_s16(13312)));
  int16x8_t v19 = vsubq_s16(v12_1, v12_2);
  int16x8_t v28 = vaddq_s16(v20, vqdmulhq_s16(v19, vdupq_n_s16(3584)));
  int16x8_t v29 = vsraq_n_s16(v28, v18, 4);
  int16x8_t v42_0 = vcombine_s16(vget_low_s16(v21), vget_low_s16(v29));
  int16x8_t v24 = vaddq_s16(v20, vqdmulhq_s16(v18, vdupq_n_s16(6144)));
  int16x8_t v25 = vsraq_n_s16(v24, v19, 5);
  int16x8_t v32 = vaddq_s16(v20, vqdmulhq_s16(v19, vdupq_n_s16(9216)));
  int16x8_t v33 = vsraq_n_s16(v32, v18, 7);
  int16x8_t v42_2 = vcombine_s16(vget_low_s16(v25), vget_low_s16(v33));
  int16x8_t v43_0 = vreinterpretq_s16_s32(vtrnq_s32(vreinterpretq_s32_s16(v42_0), vreinterpretq_s32_s16(v42_2)).val[0]);
  int16x8_t v22 = vaddq_s16(v20, vqdmulhq_s16(v18, vdupq_n_s16(9216)));
  int16x8_t v23 = vsraq_n_s16(v22, v19, 7);
  int16x8_t v30 = vaddq_s16(v20, vqdmulhq_s16(v19, vdupq_n_s16(6144)));
  int16x8_t v31 = vsraq_n_s16(v30, v18, 5);
  int16x8_t v42_1 = vcombine_s16(vget_low_s16(v23), vget_low_s16(v31));
  int16x8_t v26 = vaddq_s16(v20, vqdmulhq_s16(v18, vdupq_n_s16(3584)));
  int16x8_t v27 = vsraq_n_s16(v26, v19, 4);
  int16x8_t v34 = vaddq_s16(v20, vqdmulhq_s16(v19, vdupq_n_s16(13312)));
  int16x8_t v42_3 = vcombine_s16(vget_low_s16(v27), vget_low_s16(v34));
  int16x8_t v43_1 = vreinterpretq_s16_s32(vtrnq_s32(vreinterpretq_s32_s16(v42_1), vreinterpretq_s32_s16(v42_3)).val[0]);
  int16x8_t v44_0 = vtrnq_s16(v43_0, v43_1).val[0];
  int16x8_t v42_4 = vcombine_s16(vget_high_s16(v21), vget_high_s16(v29));
  int16x8_t v42_6 = vcombine_s16(vget_high_s16(v25), vget_high_s16(v33));
  int16x8_t v43_4 = vreinterpretq_s16_s32(vtrnq_s32(vreinterpretq_s32_s16(v42_4), vreinterpretq_s32_s16(v42_6)).val[0]);
  int16x8_t v42_5 = vcombine_s16(vget_high_s16(v23), vget_high_s16(v31));
  int16x8_t v42_7 = vcombine_s16(vget_high_s16(v27), vget_high_s16(v34));
  int16x8_t v43_5 = vreinterpretq_s16_s32(vtrnq_s32(vreinterpretq_s32_s16(v42_5), vreinterpretq_s32_s16(v42_7)).val[0]);
  int16x8_t v44_4 = vtrnq_s16(v43_4, v43_5).val[0];
  int16x8_t v48 = vsubq_s16(v44_0, v44_4);
  int16x8_t v50 = vaddq_s16(v44_4, vqdmulhq_s16(v48, vdupq_n_s16(13312)));
  int16x8_t v44_1 = vtrnq_s16(v43_0, v43_1).val[1];
  int16x8_t v44_5 = vtrnq_s16(v43_4, v43_5).val[1];
  int16x8_t v67 = vsubq_s16(v44_1, v44_5);
  int16x8_t v69 = vaddq_s16(v44_5, vqdmulhq_s16(v67, vdupq_n_s16(13312)));
  uint8x16_t v84 = vcombine_u8(vqshrun_n_s16(v50, 3), vqshrun_n_s16(v69, 3));
  vst1q_u8((uint8_t * RADRESTRICT)(v16 + 0), v84);

  {
  int32_t v17 = dw;
  char unsigned * RADRESTRICT v86 = v16 + v17;
  int16x8_t v51 = vaddq_s16(v44_4, vqdmulhq_s16(v48, vdupq_n_s16(9216)));
  int16x8_t v43_2 = vreinterpretq_s16_s32(vtrnq_s32(vreinterpretq_s32_s16(v42_0), vreinterpretq_s32_s16(v42_2)).val[1]);
  int16x8_t v43_3 = vreinterpretq_s16_s32(vtrnq_s32(vreinterpretq_s32_s16(v42_1), vreinterpretq_s32_s16(v42_3)).val[1]);
  int16x8_t v44_2 = vtrnq_s16(v43_2, v43_3).val[0];
  int16x8_t v49 = vsubq_s16(v44_2, v44_4);
  int16x8_t v52 = vsraq_n_s16(v51, v49, 7);
  int16x8_t v70 = vaddq_s16(v44_5, vqdmulhq_s16(v67, vdupq_n_s16(9216)));
  int16x8_t v44_3 = vtrnq_s16(v43_2, v43_3).val[1];
  int16x8_t v68 = vsubq_s16(v44_3, v44_5);
  int16x8_t v71 = vsraq_n_s16(v70, v68, 7);
  uint8x16_t v88 = vcombine_u8(vqshrun_n_s16(v52, 3), vqshrun_n_s16(v71, 3));
  vst1q_u8((uint8_t * RADRESTRICT)(v86 + 0), v88);

  {
  char unsigned * RADRESTRICT v90 = v86 + v17;
  int16x8_t v53 = vaddq_s16(v44_4, vqdmulhq_s16(v48, vdupq_n_s16(6144)));
  int16x8_t v54 = vsraq_n_s16(v53, v49, 5);
  int16x8_t v72 = vaddq_s16(v44_5, vqdmulhq_s16(v67, vdupq_n_s16(6144)));
  int16x8_t v73 = vsraq_n_s16(v72, v68, 5);
  uint8x16_t v92 = vcombine_u8(vqshrun_n_s16(v54, 3), vqshrun_n_s16(v73, 3));
  vst1q_u8((uint8_t * RADRESTRICT)(v90 + 0), v92);

  {
  char unsigned * RADRESTRICT v94 = v90 + v17;
  int16x8_t v55 = vaddq_s16(v44_4, vqdmulhq_s16(v48, vdupq_n_s16(3584)));
  int16x8_t v56 = vsraq_n_s16(v55, v49, 4);
  int16x8_t v74 = vaddq_s16(v44_5, vqdmulhq_s16(v67, vdupq_n_s16(3584)));
  int16x8_t v75 = vsraq_n_s16(v74, v68, 4);
  uint8x16_t v96 = vcombine_u8(vqshrun_n_s16(v56, 3), vqshrun_n_s16(v75, 3));
  vst1q_u8((uint8_t * RADRESTRICT)(v94 + 0), v96);

  {
  char unsigned * RADRESTRICT v98 = v94 + v17;
  int16x8_t v57 = vaddq_s16(v44_4, vqdmulhq_s16(v49, vdupq_n_s16(3584)));
  int16x8_t v58 = vsraq_n_s16(v57, v48, 4);
  int16x8_t v76 = vaddq_s16(v44_5, vqdmulhq_s16(v68, vdupq_n_s16(3584)));
  int16x8_t v77 = vsraq_n_s16(v76, v67, 4);
  uint8x16_t v100 = vcombine_u8(vqshrun_n_s16(v58, 3), vqshrun_n_s16(v77, 3));
  vst1q_u8((uint8_t * RADRESTRICT)(v98 + 0), v100);

  {
  char unsigned * RADRESTRICT v102 = v98 + v17;
  int16x8_t v59 = vaddq_s16(v44_4, vqdmulhq_s16(v49, vdupq_n_s16(6144)));
  int16x8_t v60 = vsraq_n_s16(v59, v48, 5);
  int16x8_t v78 = vaddq_s16(v44_5, vqdmulhq_s16(v68, vdupq_n_s16(6144)));
  int16x8_t v79 = vsraq_n_s16(v78, v67, 5);
  uint8x16_t v104 = vcombine_u8(vqshrun_n_s16(v60, 3), vqshrun_n_s16(v79, 3));
  vst1q_u8((uint8_t * RADRESTRICT)(v102 + 0), v104);

  {
  char unsigned * RADRESTRICT v106 = v102 + v17;
  int16x8_t v61 = vaddq_s16(v44_4, vqdmulhq_s16(v49, vdupq_n_s16(9216)));
  int16x8_t v62 = vsraq_n_s16(v61, v48, 7);
  int16x8_t v80 = vaddq_s16(v44_5, vqdmulhq_s16(v68, vdupq_n_s16(9216)));
  int16x8_t v81 = vsraq_n_s16(v80, v67, 7);
  uint8x16_t v108 = vcombine_u8(vqshrun_n_s16(v62, 3), vqshrun_n_s16(v81, 3));
  vst1q_u8((uint8_t * RADRESTRICT)(v106 + 0), v108);

  {
  char unsigned * RADRESTRICT v110 = v106 + v17;
  int16x8_t v63 = vaddq_s16(v44_4, vqdmulhq_s16(v49, vdupq_n_s16(13312)));
  int16x8_t v82 = vaddq_s16(v44_5, vqdmulhq_s16(v68, vdupq_n_s16(13312)));
  uint8x16_t v112 = vcombine_u8(vqshrun_n_s16(v63, 3), vqshrun_n_s16(v82, 3));
  vst1q_u8((uint8_t * RADRESTRICT)(v110 + 0), v112);
  }
  }
  }
  }
  }
  }
  }
  }
}

typedef void new_dc_blur_jump(char unsigned * RADRESTRICT dcs0, char unsigned * RADRESTRICT dcs1, char unsigned * RADRESTRICT dcs2, char unsigned * RADRESTRICT t, char unsigned * RADRESTRICT dest, int32_t dw);
#if !defined(BINK2_CODEGEN_NO_TABLE)
#ifdef _MSC_VER
_declspec(selectany)
#else
static
#endif
new_dc_blur_jump *new_dc_blur[] =
{
  new_dc_blur_r0_l0,
  new_dc_blur_r0_l1,
  new_dc_blur_r1_l0,
  new_dc_blur_r1_l1,
};
#endif

