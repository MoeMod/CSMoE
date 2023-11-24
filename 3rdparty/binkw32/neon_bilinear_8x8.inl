// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

static CODEGEN_ATTR void bilinear_8x8_h0_v0(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int32_t sw, int32_t dw)
{
  {
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dest);
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(src);
  uint8x8_t v4 = vld1_u8((uint8_t * RADRESTRICT)(v0 + 0));
  vst1_u8((uint8_t * RADRESTRICT)(v1 + 0), v4);

  {
  int32_t v3 = dw;
  char unsigned * RADRESTRICT v57 = v1 + v3;
  int32_t v2 = sw;
  char unsigned * RADRESTRICT v7 = v0 + v2;
  uint8x8_t v8 = vld1_u8((uint8_t * RADRESTRICT)(v7 + 0));
  vst1_u8((uint8_t * RADRESTRICT)(v57 + 0), v8);

  {
  char unsigned * RADRESTRICT v59 = v57 + v3;
  char unsigned * RADRESTRICT v11 = v7 + v2;
  uint8x8_t v12 = vld1_u8((uint8_t * RADRESTRICT)(v11 + 0));
  vst1_u8((uint8_t * RADRESTRICT)(v59 + 0), v12);

  {
  char unsigned * RADRESTRICT v61 = v59 + v3;
  char unsigned * RADRESTRICT v15 = v11 + v2;
  uint8x8_t v16 = vld1_u8((uint8_t * RADRESTRICT)(v15 + 0));
  vst1_u8((uint8_t * RADRESTRICT)(v61 + 0), v16);

  {
  char unsigned * RADRESTRICT v63 = v61 + v3;
  char unsigned * RADRESTRICT v19 = v15 + v2;
  uint8x8_t v20 = vld1_u8((uint8_t * RADRESTRICT)(v19 + 0));
  vst1_u8((uint8_t * RADRESTRICT)(v63 + 0), v20);

  {
  char unsigned * RADRESTRICT v65 = v63 + v3;
  char unsigned * RADRESTRICT v23 = v19 + v2;
  uint8x8_t v24 = vld1_u8((uint8_t * RADRESTRICT)(v23 + 0));
  vst1_u8((uint8_t * RADRESTRICT)(v65 + 0), v24);

  {
  char unsigned * RADRESTRICT v67 = v65 + v3;
  char unsigned * RADRESTRICT v27 = v23 + v2;
  uint8x8_t v28 = vld1_u8((uint8_t * RADRESTRICT)(v27 + 0));
  vst1_u8((uint8_t * RADRESTRICT)(v67 + 0), v28);

  {
  char unsigned * RADRESTRICT v69 = v67 + v3;
  char unsigned * RADRESTRICT v31 = v27 + v2;
  uint8x8_t v32 = vld1_u8((uint8_t * RADRESTRICT)(v31 + 0));
  vst1_u8((uint8_t * RADRESTRICT)(v69 + 0), v32);
  }
  }
  }
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void bilinear_8x8_h0_v25(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int32_t sw, int32_t dw)
{
  {
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dest);
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(src);
  uint8x8_t v4 = vld1_u8((uint8_t * RADRESTRICT)(v0 + 0));
  int16x8_t v6 = vreinterpretq_s16_u16(vshll_n_u8(v4, 4));
  int32_t v2 = sw;
  char unsigned * RADRESTRICT v7 = v0 + v2;
  uint8x8_t v8 = vld1_u8((uint8_t * RADRESTRICT)(v7 + 0));
  int16x8_t v10 = vreinterpretq_s16_u16(vshll_n_u8(v8, 4));
  int16x8_t v40 = vsubq_s16(v10, v6);
  int16x8_t v41 = vsraq_n_s16(v6, v40, 2);
  uint8x8_t v43 = vqrshrun_n_s16(v41, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v1 + 0), v43);

  {
  int32_t v3 = dw;
  char unsigned * RADRESTRICT v73 = v1 + v3;
  char unsigned * RADRESTRICT v11 = v7 + v2;
  uint8x8_t v12 = vld1_u8((uint8_t * RADRESTRICT)(v11 + 0));
  int16x8_t v14 = vreinterpretq_s16_u16(vshll_n_u8(v12, 4));
  int16x8_t v44 = vsubq_s16(v14, v10);
  int16x8_t v45 = vsraq_n_s16(v10, v44, 2);
  uint8x8_t v47 = vqrshrun_n_s16(v45, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v73 + 0), v47);

  {
  char unsigned * RADRESTRICT v75 = v73 + v3;
  char unsigned * RADRESTRICT v15 = v11 + v2;
  uint8x8_t v16 = vld1_u8((uint8_t * RADRESTRICT)(v15 + 0));
  int16x8_t v18 = vreinterpretq_s16_u16(vshll_n_u8(v16, 4));
  int16x8_t v48 = vsubq_s16(v18, v14);
  int16x8_t v49 = vsraq_n_s16(v14, v48, 2);
  uint8x8_t v51 = vqrshrun_n_s16(v49, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v75 + 0), v51);

  {
  char unsigned * RADRESTRICT v77 = v75 + v3;
  char unsigned * RADRESTRICT v19 = v15 + v2;
  uint8x8_t v20 = vld1_u8((uint8_t * RADRESTRICT)(v19 + 0));
  int16x8_t v22 = vreinterpretq_s16_u16(vshll_n_u8(v20, 4));
  int16x8_t v52 = vsubq_s16(v22, v18);
  int16x8_t v53 = vsraq_n_s16(v18, v52, 2);
  uint8x8_t v55 = vqrshrun_n_s16(v53, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v77 + 0), v55);

  {
  char unsigned * RADRESTRICT v79 = v77 + v3;
  char unsigned * RADRESTRICT v23 = v19 + v2;
  uint8x8_t v24 = vld1_u8((uint8_t * RADRESTRICT)(v23 + 0));
  int16x8_t v26 = vreinterpretq_s16_u16(vshll_n_u8(v24, 4));
  int16x8_t v56 = vsubq_s16(v26, v22);
  int16x8_t v57 = vsraq_n_s16(v22, v56, 2);
  uint8x8_t v59 = vqrshrun_n_s16(v57, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v79 + 0), v59);

  {
  char unsigned * RADRESTRICT v81 = v79 + v3;
  char unsigned * RADRESTRICT v27 = v23 + v2;
  uint8x8_t v28 = vld1_u8((uint8_t * RADRESTRICT)(v27 + 0));
  int16x8_t v30 = vreinterpretq_s16_u16(vshll_n_u8(v28, 4));
  int16x8_t v60 = vsubq_s16(v30, v26);
  int16x8_t v61 = vsraq_n_s16(v26, v60, 2);
  uint8x8_t v63 = vqrshrun_n_s16(v61, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v81 + 0), v63);

  {
  char unsigned * RADRESTRICT v83 = v81 + v3;
  char unsigned * RADRESTRICT v31 = v27 + v2;
  uint8x8_t v32 = vld1_u8((uint8_t * RADRESTRICT)(v31 + 0));
  int16x8_t v34 = vreinterpretq_s16_u16(vshll_n_u8(v32, 4));
  int16x8_t v64 = vsubq_s16(v34, v30);
  int16x8_t v65 = vsraq_n_s16(v30, v64, 2);
  uint8x8_t v67 = vqrshrun_n_s16(v65, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v83 + 0), v67);

  {
  char unsigned * RADRESTRICT v85 = v83 + v3;
  char unsigned * RADRESTRICT v35 = v31 + v2;
  uint8x8_t v36 = vld1_u8((uint8_t * RADRESTRICT)(v35 + 0));
  int16x8_t v38 = vreinterpretq_s16_u16(vshll_n_u8(v36, 4));
  int16x8_t v68 = vsubq_s16(v38, v34);
  int16x8_t v69 = vsraq_n_s16(v34, v68, 2);
  uint8x8_t v71 = vqrshrun_n_s16(v69, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v85 + 0), v71);
  }
  }
  }
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void bilinear_8x8_h0_v50(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int32_t sw, int32_t dw)
{
  {
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dest);
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(src);
  uint8x8_t v4 = vld1_u8((uint8_t * RADRESTRICT)(v0 + 0));
  int32_t v2 = sw;
  char unsigned * RADRESTRICT v7 = v0 + v2;
  uint8x8_t v8 = vld1_u8((uint8_t * RADRESTRICT)(v7 + 0));
  uint8x8_t v65 = vrhadd_u8(v4, v8);
  vst1_u8((uint8_t * RADRESTRICT)(v1 + 0), v65);

  {
  int32_t v3 = dw;
  char unsigned * RADRESTRICT v66 = v1 + v3;
  char unsigned * RADRESTRICT v11 = v7 + v2;
  uint8x8_t v12 = vld1_u8((uint8_t * RADRESTRICT)(v11 + 0));
  uint8x8_t v68 = vrhadd_u8(v8, v12);
  vst1_u8((uint8_t * RADRESTRICT)(v66 + 0), v68);

  {
  char unsigned * RADRESTRICT v69 = v66 + v3;
  char unsigned * RADRESTRICT v15 = v11 + v2;
  uint8x8_t v16 = vld1_u8((uint8_t * RADRESTRICT)(v15 + 0));
  uint8x8_t v71 = vrhadd_u8(v12, v16);
  vst1_u8((uint8_t * RADRESTRICT)(v69 + 0), v71);

  {
  char unsigned * RADRESTRICT v72 = v69 + v3;
  char unsigned * RADRESTRICT v19 = v15 + v2;
  uint8x8_t v20 = vld1_u8((uint8_t * RADRESTRICT)(v19 + 0));
  uint8x8_t v74 = vrhadd_u8(v16, v20);
  vst1_u8((uint8_t * RADRESTRICT)(v72 + 0), v74);

  {
  char unsigned * RADRESTRICT v75 = v72 + v3;
  char unsigned * RADRESTRICT v23 = v19 + v2;
  uint8x8_t v24 = vld1_u8((uint8_t * RADRESTRICT)(v23 + 0));
  uint8x8_t v77 = vrhadd_u8(v20, v24);
  vst1_u8((uint8_t * RADRESTRICT)(v75 + 0), v77);

  {
  char unsigned * RADRESTRICT v78 = v75 + v3;
  char unsigned * RADRESTRICT v27 = v23 + v2;
  uint8x8_t v28 = vld1_u8((uint8_t * RADRESTRICT)(v27 + 0));
  uint8x8_t v80 = vrhadd_u8(v24, v28);
  vst1_u8((uint8_t * RADRESTRICT)(v78 + 0), v80);

  {
  char unsigned * RADRESTRICT v81 = v78 + v3;
  char unsigned * RADRESTRICT v31 = v27 + v2;
  uint8x8_t v32 = vld1_u8((uint8_t * RADRESTRICT)(v31 + 0));
  uint8x8_t v83 = vrhadd_u8(v28, v32);
  vst1_u8((uint8_t * RADRESTRICT)(v81 + 0), v83);

  {
  char unsigned * RADRESTRICT v84 = v81 + v3;
  char unsigned * RADRESTRICT v35 = v31 + v2;
  uint8x8_t v36 = vld1_u8((uint8_t * RADRESTRICT)(v35 + 0));
  uint8x8_t v86 = vrhadd_u8(v32, v36);
  vst1_u8((uint8_t * RADRESTRICT)(v84 + 0), v86);
  }
  }
  }
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void bilinear_8x8_h0_v75(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int32_t sw, int32_t dw)
{
  {
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dest);
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(src);
  int32_t v2 = sw;
  char unsigned * RADRESTRICT v7 = v0 + v2;
  uint8x8_t v8 = vld1_u8((uint8_t * RADRESTRICT)(v7 + 0));
  int16x8_t v10 = vreinterpretq_s16_u16(vshll_n_u8(v8, 4));
  uint8x8_t v4 = vld1_u8((uint8_t * RADRESTRICT)(v0 + 0));
  int16x8_t v6 = vreinterpretq_s16_u16(vshll_n_u8(v4, 4));
  int16x8_t v40 = vsubq_s16(v6, v10);
  int16x8_t v41 = vsraq_n_s16(v10, v40, 2);
  uint8x8_t v43 = vqrshrun_n_s16(v41, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v1 + 0), v43);

  {
  int32_t v3 = dw;
  char unsigned * RADRESTRICT v73 = v1 + v3;
  char unsigned * RADRESTRICT v11 = v7 + v2;
  uint8x8_t v12 = vld1_u8((uint8_t * RADRESTRICT)(v11 + 0));
  int16x8_t v14 = vreinterpretq_s16_u16(vshll_n_u8(v12, 4));
  int16x8_t v44 = vsubq_s16(v10, v14);
  int16x8_t v45 = vsraq_n_s16(v14, v44, 2);
  uint8x8_t v47 = vqrshrun_n_s16(v45, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v73 + 0), v47);

  {
  char unsigned * RADRESTRICT v75 = v73 + v3;
  char unsigned * RADRESTRICT v15 = v11 + v2;
  uint8x8_t v16 = vld1_u8((uint8_t * RADRESTRICT)(v15 + 0));
  int16x8_t v18 = vreinterpretq_s16_u16(vshll_n_u8(v16, 4));
  int16x8_t v48 = vsubq_s16(v14, v18);
  int16x8_t v49 = vsraq_n_s16(v18, v48, 2);
  uint8x8_t v51 = vqrshrun_n_s16(v49, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v75 + 0), v51);

  {
  char unsigned * RADRESTRICT v77 = v75 + v3;
  char unsigned * RADRESTRICT v19 = v15 + v2;
  uint8x8_t v20 = vld1_u8((uint8_t * RADRESTRICT)(v19 + 0));
  int16x8_t v22 = vreinterpretq_s16_u16(vshll_n_u8(v20, 4));
  int16x8_t v52 = vsubq_s16(v18, v22);
  int16x8_t v53 = vsraq_n_s16(v22, v52, 2);
  uint8x8_t v55 = vqrshrun_n_s16(v53, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v77 + 0), v55);

  {
  char unsigned * RADRESTRICT v79 = v77 + v3;
  char unsigned * RADRESTRICT v23 = v19 + v2;
  uint8x8_t v24 = vld1_u8((uint8_t * RADRESTRICT)(v23 + 0));
  int16x8_t v26 = vreinterpretq_s16_u16(vshll_n_u8(v24, 4));
  int16x8_t v56 = vsubq_s16(v22, v26);
  int16x8_t v57 = vsraq_n_s16(v26, v56, 2);
  uint8x8_t v59 = vqrshrun_n_s16(v57, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v79 + 0), v59);

  {
  char unsigned * RADRESTRICT v81 = v79 + v3;
  char unsigned * RADRESTRICT v27 = v23 + v2;
  uint8x8_t v28 = vld1_u8((uint8_t * RADRESTRICT)(v27 + 0));
  int16x8_t v30 = vreinterpretq_s16_u16(vshll_n_u8(v28, 4));
  int16x8_t v60 = vsubq_s16(v26, v30);
  int16x8_t v61 = vsraq_n_s16(v30, v60, 2);
  uint8x8_t v63 = vqrshrun_n_s16(v61, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v81 + 0), v63);

  {
  char unsigned * RADRESTRICT v83 = v81 + v3;
  char unsigned * RADRESTRICT v31 = v27 + v2;
  uint8x8_t v32 = vld1_u8((uint8_t * RADRESTRICT)(v31 + 0));
  int16x8_t v34 = vreinterpretq_s16_u16(vshll_n_u8(v32, 4));
  int16x8_t v64 = vsubq_s16(v30, v34);
  int16x8_t v65 = vsraq_n_s16(v34, v64, 2);
  uint8x8_t v67 = vqrshrun_n_s16(v65, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v83 + 0), v67);

  {
  char unsigned * RADRESTRICT v85 = v83 + v3;
  char unsigned * RADRESTRICT v35 = v31 + v2;
  uint8x8_t v36 = vld1_u8((uint8_t * RADRESTRICT)(v35 + 0));
  int16x8_t v38 = vreinterpretq_s16_u16(vshll_n_u8(v36, 4));
  int16x8_t v68 = vsubq_s16(v34, v38);
  int16x8_t v69 = vsraq_n_s16(v38, v68, 2);
  uint8x8_t v71 = vqrshrun_n_s16(v69, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v85 + 0), v71);
  }
  }
  }
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void bilinear_8x8_h25_v0(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int32_t sw, int32_t dw)
{
  {
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dest);
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(src);
  uint8x8_t v4 = vld1_u8((uint8_t * RADRESTRICT)(v0 + 0));
  int16x8_t v6 = vreinterpretq_s16_u16(vshll_n_u8(v4, 4));
  uint8x8_t v7 = vld1_u8((uint8_t * RADRESTRICT)(v0 + 1));
  int16x8_t v9 = vreinterpretq_s16_u16(vshll_n_u8(v7, 4));
  int16x8_t v10 = vsubq_s16(v9, v6);
  int16x8_t v11 = vsraq_n_s16(v6, v10, 2);
  uint8x8_t v86 = vqrshrun_n_s16(v11, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v1 + 0), v86);

  {
  int32_t v3 = dw;
  char unsigned * RADRESTRICT v102 = v1 + v3;
  int32_t v2 = sw;
  char unsigned * RADRESTRICT v12 = v0 + v2;
  uint8x8_t v13 = vld1_u8((uint8_t * RADRESTRICT)(v12 + 0));
  int16x8_t v15 = vreinterpretq_s16_u16(vshll_n_u8(v13, 4));
  uint8x8_t v16 = vld1_u8((uint8_t * RADRESTRICT)(v12 + 1));
  int16x8_t v18 = vreinterpretq_s16_u16(vshll_n_u8(v16, 4));
  int16x8_t v19 = vsubq_s16(v18, v15);
  int16x8_t v20 = vsraq_n_s16(v15, v19, 2);
  uint8x8_t v88 = vqrshrun_n_s16(v20, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v102 + 0), v88);

  {
  char unsigned * RADRESTRICT v104 = v102 + v3;
  char unsigned * RADRESTRICT v21 = v12 + v2;
  uint8x8_t v22 = vld1_u8((uint8_t * RADRESTRICT)(v21 + 0));
  int16x8_t v24 = vreinterpretq_s16_u16(vshll_n_u8(v22, 4));
  uint8x8_t v25 = vld1_u8((uint8_t * RADRESTRICT)(v21 + 1));
  int16x8_t v27 = vreinterpretq_s16_u16(vshll_n_u8(v25, 4));
  int16x8_t v28 = vsubq_s16(v27, v24);
  int16x8_t v29 = vsraq_n_s16(v24, v28, 2);
  uint8x8_t v90 = vqrshrun_n_s16(v29, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v104 + 0), v90);

  {
  char unsigned * RADRESTRICT v106 = v104 + v3;
  char unsigned * RADRESTRICT v30 = v21 + v2;
  uint8x8_t v31 = vld1_u8((uint8_t * RADRESTRICT)(v30 + 0));
  int16x8_t v33 = vreinterpretq_s16_u16(vshll_n_u8(v31, 4));
  uint8x8_t v34 = vld1_u8((uint8_t * RADRESTRICT)(v30 + 1));
  int16x8_t v36 = vreinterpretq_s16_u16(vshll_n_u8(v34, 4));
  int16x8_t v37 = vsubq_s16(v36, v33);
  int16x8_t v38 = vsraq_n_s16(v33, v37, 2);
  uint8x8_t v92 = vqrshrun_n_s16(v38, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v106 + 0), v92);

  {
  char unsigned * RADRESTRICT v108 = v106 + v3;
  char unsigned * RADRESTRICT v39 = v30 + v2;
  uint8x8_t v40 = vld1_u8((uint8_t * RADRESTRICT)(v39 + 0));
  int16x8_t v42 = vreinterpretq_s16_u16(vshll_n_u8(v40, 4));
  uint8x8_t v43 = vld1_u8((uint8_t * RADRESTRICT)(v39 + 1));
  int16x8_t v45 = vreinterpretq_s16_u16(vshll_n_u8(v43, 4));
  int16x8_t v46 = vsubq_s16(v45, v42);
  int16x8_t v47 = vsraq_n_s16(v42, v46, 2);
  uint8x8_t v94 = vqrshrun_n_s16(v47, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v108 + 0), v94);

  {
  char unsigned * RADRESTRICT v110 = v108 + v3;
  char unsigned * RADRESTRICT v48 = v39 + v2;
  uint8x8_t v49 = vld1_u8((uint8_t * RADRESTRICT)(v48 + 0));
  int16x8_t v51 = vreinterpretq_s16_u16(vshll_n_u8(v49, 4));
  uint8x8_t v52 = vld1_u8((uint8_t * RADRESTRICT)(v48 + 1));
  int16x8_t v54 = vreinterpretq_s16_u16(vshll_n_u8(v52, 4));
  int16x8_t v55 = vsubq_s16(v54, v51);
  int16x8_t v56 = vsraq_n_s16(v51, v55, 2);
  uint8x8_t v96 = vqrshrun_n_s16(v56, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v110 + 0), v96);

  {
  char unsigned * RADRESTRICT v112 = v110 + v3;
  char unsigned * RADRESTRICT v57 = v48 + v2;
  uint8x8_t v58 = vld1_u8((uint8_t * RADRESTRICT)(v57 + 0));
  int16x8_t v60 = vreinterpretq_s16_u16(vshll_n_u8(v58, 4));
  uint8x8_t v61 = vld1_u8((uint8_t * RADRESTRICT)(v57 + 1));
  int16x8_t v63 = vreinterpretq_s16_u16(vshll_n_u8(v61, 4));
  int16x8_t v64 = vsubq_s16(v63, v60);
  int16x8_t v65 = vsraq_n_s16(v60, v64, 2);
  uint8x8_t v98 = vqrshrun_n_s16(v65, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v112 + 0), v98);

  {
  char unsigned * RADRESTRICT v114 = v112 + v3;
  char unsigned * RADRESTRICT v66 = v57 + v2;
  uint8x8_t v67 = vld1_u8((uint8_t * RADRESTRICT)(v66 + 0));
  int16x8_t v69 = vreinterpretq_s16_u16(vshll_n_u8(v67, 4));
  uint8x8_t v70 = vld1_u8((uint8_t * RADRESTRICT)(v66 + 1));
  int16x8_t v72 = vreinterpretq_s16_u16(vshll_n_u8(v70, 4));
  int16x8_t v73 = vsubq_s16(v72, v69);
  int16x8_t v74 = vsraq_n_s16(v69, v73, 2);
  uint8x8_t v100 = vqrshrun_n_s16(v74, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v114 + 0), v100);
  }
  }
  }
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void bilinear_8x8_h25_v25(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int32_t sw, int32_t dw)
{
  {
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dest);
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(src);
  uint8x8_t v4 = vld1_u8((uint8_t * RADRESTRICT)(v0 + 0));
  int16x8_t v6 = vreinterpretq_s16_u16(vshll_n_u8(v4, 4));
  uint8x8_t v7 = vld1_u8((uint8_t * RADRESTRICT)(v0 + 1));
  int16x8_t v9 = vreinterpretq_s16_u16(vshll_n_u8(v7, 4));
  int16x8_t v10 = vsubq_s16(v9, v6);
  int16x8_t v11 = vsraq_n_s16(v6, v10, 2);
  int32_t v2 = sw;
  char unsigned * RADRESTRICT v12 = v0 + v2;
  uint8x8_t v13 = vld1_u8((uint8_t * RADRESTRICT)(v12 + 0));
  int16x8_t v15 = vreinterpretq_s16_u16(vshll_n_u8(v13, 4));
  uint8x8_t v16 = vld1_u8((uint8_t * RADRESTRICT)(v12 + 1));
  int16x8_t v18 = vreinterpretq_s16_u16(vshll_n_u8(v16, 4));
  int16x8_t v19 = vsubq_s16(v18, v15);
  int16x8_t v20 = vsraq_n_s16(v15, v19, 2);
  int16x8_t v85 = vsubq_s16(v20, v11);
  int16x8_t v86 = vsraq_n_s16(v11, v85, 2);
  uint8x8_t v88 = vqrshrun_n_s16(v86, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v1 + 0), v88);

  {
  int32_t v3 = dw;
  char unsigned * RADRESTRICT v118 = v1 + v3;
  char unsigned * RADRESTRICT v21 = v12 + v2;
  uint8x8_t v22 = vld1_u8((uint8_t * RADRESTRICT)(v21 + 0));
  int16x8_t v24 = vreinterpretq_s16_u16(vshll_n_u8(v22, 4));
  uint8x8_t v25 = vld1_u8((uint8_t * RADRESTRICT)(v21 + 1));
  int16x8_t v27 = vreinterpretq_s16_u16(vshll_n_u8(v25, 4));
  int16x8_t v28 = vsubq_s16(v27, v24);
  int16x8_t v29 = vsraq_n_s16(v24, v28, 2);
  int16x8_t v89 = vsubq_s16(v29, v20);
  int16x8_t v90 = vsraq_n_s16(v20, v89, 2);
  uint8x8_t v92 = vqrshrun_n_s16(v90, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v118 + 0), v92);

  {
  char unsigned * RADRESTRICT v120 = v118 + v3;
  char unsigned * RADRESTRICT v30 = v21 + v2;
  uint8x8_t v31 = vld1_u8((uint8_t * RADRESTRICT)(v30 + 0));
  int16x8_t v33 = vreinterpretq_s16_u16(vshll_n_u8(v31, 4));
  uint8x8_t v34 = vld1_u8((uint8_t * RADRESTRICT)(v30 + 1));
  int16x8_t v36 = vreinterpretq_s16_u16(vshll_n_u8(v34, 4));
  int16x8_t v37 = vsubq_s16(v36, v33);
  int16x8_t v38 = vsraq_n_s16(v33, v37, 2);
  int16x8_t v93 = vsubq_s16(v38, v29);
  int16x8_t v94 = vsraq_n_s16(v29, v93, 2);
  uint8x8_t v96 = vqrshrun_n_s16(v94, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v120 + 0), v96);

  {
  char unsigned * RADRESTRICT v122 = v120 + v3;
  char unsigned * RADRESTRICT v39 = v30 + v2;
  uint8x8_t v40 = vld1_u8((uint8_t * RADRESTRICT)(v39 + 0));
  int16x8_t v42 = vreinterpretq_s16_u16(vshll_n_u8(v40, 4));
  uint8x8_t v43 = vld1_u8((uint8_t * RADRESTRICT)(v39 + 1));
  int16x8_t v45 = vreinterpretq_s16_u16(vshll_n_u8(v43, 4));
  int16x8_t v46 = vsubq_s16(v45, v42);
  int16x8_t v47 = vsraq_n_s16(v42, v46, 2);
  int16x8_t v97 = vsubq_s16(v47, v38);
  int16x8_t v98 = vsraq_n_s16(v38, v97, 2);
  uint8x8_t v100 = vqrshrun_n_s16(v98, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v122 + 0), v100);

  {
  char unsigned * RADRESTRICT v124 = v122 + v3;
  char unsigned * RADRESTRICT v48 = v39 + v2;
  uint8x8_t v49 = vld1_u8((uint8_t * RADRESTRICT)(v48 + 0));
  int16x8_t v51 = vreinterpretq_s16_u16(vshll_n_u8(v49, 4));
  uint8x8_t v52 = vld1_u8((uint8_t * RADRESTRICT)(v48 + 1));
  int16x8_t v54 = vreinterpretq_s16_u16(vshll_n_u8(v52, 4));
  int16x8_t v55 = vsubq_s16(v54, v51);
  int16x8_t v56 = vsraq_n_s16(v51, v55, 2);
  int16x8_t v101 = vsubq_s16(v56, v47);
  int16x8_t v102 = vsraq_n_s16(v47, v101, 2);
  uint8x8_t v104 = vqrshrun_n_s16(v102, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v124 + 0), v104);

  {
  char unsigned * RADRESTRICT v126 = v124 + v3;
  char unsigned * RADRESTRICT v57 = v48 + v2;
  uint8x8_t v58 = vld1_u8((uint8_t * RADRESTRICT)(v57 + 0));
  int16x8_t v60 = vreinterpretq_s16_u16(vshll_n_u8(v58, 4));
  uint8x8_t v61 = vld1_u8((uint8_t * RADRESTRICT)(v57 + 1));
  int16x8_t v63 = vreinterpretq_s16_u16(vshll_n_u8(v61, 4));
  int16x8_t v64 = vsubq_s16(v63, v60);
  int16x8_t v65 = vsraq_n_s16(v60, v64, 2);
  int16x8_t v105 = vsubq_s16(v65, v56);
  int16x8_t v106 = vsraq_n_s16(v56, v105, 2);
  uint8x8_t v108 = vqrshrun_n_s16(v106, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v126 + 0), v108);

  {
  char unsigned * RADRESTRICT v128 = v126 + v3;
  char unsigned * RADRESTRICT v66 = v57 + v2;
  uint8x8_t v67 = vld1_u8((uint8_t * RADRESTRICT)(v66 + 0));
  int16x8_t v69 = vreinterpretq_s16_u16(vshll_n_u8(v67, 4));
  uint8x8_t v70 = vld1_u8((uint8_t * RADRESTRICT)(v66 + 1));
  int16x8_t v72 = vreinterpretq_s16_u16(vshll_n_u8(v70, 4));
  int16x8_t v73 = vsubq_s16(v72, v69);
  int16x8_t v74 = vsraq_n_s16(v69, v73, 2);
  int16x8_t v109 = vsubq_s16(v74, v65);
  int16x8_t v110 = vsraq_n_s16(v65, v109, 2);
  uint8x8_t v112 = vqrshrun_n_s16(v110, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v128 + 0), v112);

  {
  char unsigned * RADRESTRICT v130 = v128 + v3;
  char unsigned * RADRESTRICT v75 = v66 + v2;
  uint8x8_t v76 = vld1_u8((uint8_t * RADRESTRICT)(v75 + 0));
  int16x8_t v78 = vreinterpretq_s16_u16(vshll_n_u8(v76, 4));
  uint8x8_t v79 = vld1_u8((uint8_t * RADRESTRICT)(v75 + 1));
  int16x8_t v81 = vreinterpretq_s16_u16(vshll_n_u8(v79, 4));
  int16x8_t v82 = vsubq_s16(v81, v78);
  int16x8_t v83 = vsraq_n_s16(v78, v82, 2);
  int16x8_t v113 = vsubq_s16(v83, v74);
  int16x8_t v114 = vsraq_n_s16(v74, v113, 2);
  uint8x8_t v116 = vqrshrun_n_s16(v114, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v130 + 0), v116);
  }
  }
  }
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void bilinear_8x8_h25_v50(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int32_t sw, int32_t dw)
{
  {
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dest);
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(src);
  uint8x8_t v4 = vld1_u8((uint8_t * RADRESTRICT)(v0 + 0));
  int16x8_t v6 = vreinterpretq_s16_u16(vshll_n_u8(v4, 4));
  uint8x8_t v7 = vld1_u8((uint8_t * RADRESTRICT)(v0 + 1));
  int16x8_t v9 = vreinterpretq_s16_u16(vshll_n_u8(v7, 4));
  int16x8_t v10 = vsubq_s16(v9, v6);
  int16x8_t v11 = vsraq_n_s16(v6, v10, 2);
  int32_t v2 = sw;
  char unsigned * RADRESTRICT v12 = v0 + v2;
  uint8x8_t v13 = vld1_u8((uint8_t * RADRESTRICT)(v12 + 0));
  int16x8_t v15 = vreinterpretq_s16_u16(vshll_n_u8(v13, 4));
  uint8x8_t v16 = vld1_u8((uint8_t * RADRESTRICT)(v12 + 1));
  int16x8_t v18 = vreinterpretq_s16_u16(vshll_n_u8(v16, 4));
  int16x8_t v19 = vsubq_s16(v18, v15);
  int16x8_t v20 = vsraq_n_s16(v15, v19, 2);
  int16x8_t v85 = vrhaddq_s16(v11, v20);
  uint8x8_t v87 = vqrshrun_n_s16(v85, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v1 + 0), v87);

  {
  int32_t v3 = dw;
  char unsigned * RADRESTRICT v110 = v1 + v3;
  char unsigned * RADRESTRICT v21 = v12 + v2;
  uint8x8_t v22 = vld1_u8((uint8_t * RADRESTRICT)(v21 + 0));
  int16x8_t v24 = vreinterpretq_s16_u16(vshll_n_u8(v22, 4));
  uint8x8_t v25 = vld1_u8((uint8_t * RADRESTRICT)(v21 + 1));
  int16x8_t v27 = vreinterpretq_s16_u16(vshll_n_u8(v25, 4));
  int16x8_t v28 = vsubq_s16(v27, v24);
  int16x8_t v29 = vsraq_n_s16(v24, v28, 2);
  int16x8_t v88 = vrhaddq_s16(v20, v29);
  uint8x8_t v90 = vqrshrun_n_s16(v88, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v110 + 0), v90);

  {
  char unsigned * RADRESTRICT v112 = v110 + v3;
  char unsigned * RADRESTRICT v30 = v21 + v2;
  uint8x8_t v31 = vld1_u8((uint8_t * RADRESTRICT)(v30 + 0));
  int16x8_t v33 = vreinterpretq_s16_u16(vshll_n_u8(v31, 4));
  uint8x8_t v34 = vld1_u8((uint8_t * RADRESTRICT)(v30 + 1));
  int16x8_t v36 = vreinterpretq_s16_u16(vshll_n_u8(v34, 4));
  int16x8_t v37 = vsubq_s16(v36, v33);
  int16x8_t v38 = vsraq_n_s16(v33, v37, 2);
  int16x8_t v91 = vrhaddq_s16(v29, v38);
  uint8x8_t v93 = vqrshrun_n_s16(v91, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v112 + 0), v93);

  {
  char unsigned * RADRESTRICT v114 = v112 + v3;
  char unsigned * RADRESTRICT v39 = v30 + v2;
  uint8x8_t v40 = vld1_u8((uint8_t * RADRESTRICT)(v39 + 0));
  int16x8_t v42 = vreinterpretq_s16_u16(vshll_n_u8(v40, 4));
  uint8x8_t v43 = vld1_u8((uint8_t * RADRESTRICT)(v39 + 1));
  int16x8_t v45 = vreinterpretq_s16_u16(vshll_n_u8(v43, 4));
  int16x8_t v46 = vsubq_s16(v45, v42);
  int16x8_t v47 = vsraq_n_s16(v42, v46, 2);
  int16x8_t v94 = vrhaddq_s16(v38, v47);
  uint8x8_t v96 = vqrshrun_n_s16(v94, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v114 + 0), v96);

  {
  char unsigned * RADRESTRICT v116 = v114 + v3;
  char unsigned * RADRESTRICT v48 = v39 + v2;
  uint8x8_t v49 = vld1_u8((uint8_t * RADRESTRICT)(v48 + 0));
  int16x8_t v51 = vreinterpretq_s16_u16(vshll_n_u8(v49, 4));
  uint8x8_t v52 = vld1_u8((uint8_t * RADRESTRICT)(v48 + 1));
  int16x8_t v54 = vreinterpretq_s16_u16(vshll_n_u8(v52, 4));
  int16x8_t v55 = vsubq_s16(v54, v51);
  int16x8_t v56 = vsraq_n_s16(v51, v55, 2);
  int16x8_t v97 = vrhaddq_s16(v47, v56);
  uint8x8_t v99 = vqrshrun_n_s16(v97, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v116 + 0), v99);

  {
  char unsigned * RADRESTRICT v118 = v116 + v3;
  char unsigned * RADRESTRICT v57 = v48 + v2;
  uint8x8_t v58 = vld1_u8((uint8_t * RADRESTRICT)(v57 + 0));
  int16x8_t v60 = vreinterpretq_s16_u16(vshll_n_u8(v58, 4));
  uint8x8_t v61 = vld1_u8((uint8_t * RADRESTRICT)(v57 + 1));
  int16x8_t v63 = vreinterpretq_s16_u16(vshll_n_u8(v61, 4));
  int16x8_t v64 = vsubq_s16(v63, v60);
  int16x8_t v65 = vsraq_n_s16(v60, v64, 2);
  int16x8_t v100 = vrhaddq_s16(v56, v65);
  uint8x8_t v102 = vqrshrun_n_s16(v100, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v118 + 0), v102);

  {
  char unsigned * RADRESTRICT v120 = v118 + v3;
  char unsigned * RADRESTRICT v66 = v57 + v2;
  uint8x8_t v67 = vld1_u8((uint8_t * RADRESTRICT)(v66 + 0));
  int16x8_t v69 = vreinterpretq_s16_u16(vshll_n_u8(v67, 4));
  uint8x8_t v70 = vld1_u8((uint8_t * RADRESTRICT)(v66 + 1));
  int16x8_t v72 = vreinterpretq_s16_u16(vshll_n_u8(v70, 4));
  int16x8_t v73 = vsubq_s16(v72, v69);
  int16x8_t v74 = vsraq_n_s16(v69, v73, 2);
  int16x8_t v103 = vrhaddq_s16(v65, v74);
  uint8x8_t v105 = vqrshrun_n_s16(v103, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v120 + 0), v105);

  {
  char unsigned * RADRESTRICT v122 = v120 + v3;
  char unsigned * RADRESTRICT v75 = v66 + v2;
  uint8x8_t v76 = vld1_u8((uint8_t * RADRESTRICT)(v75 + 0));
  int16x8_t v78 = vreinterpretq_s16_u16(vshll_n_u8(v76, 4));
  uint8x8_t v79 = vld1_u8((uint8_t * RADRESTRICT)(v75 + 1));
  int16x8_t v81 = vreinterpretq_s16_u16(vshll_n_u8(v79, 4));
  int16x8_t v82 = vsubq_s16(v81, v78);
  int16x8_t v83 = vsraq_n_s16(v78, v82, 2);
  int16x8_t v106 = vrhaddq_s16(v74, v83);
  uint8x8_t v108 = vqrshrun_n_s16(v106, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v122 + 0), v108);
  }
  }
  }
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void bilinear_8x8_h25_v75(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int32_t sw, int32_t dw)
{
  {
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dest);
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(src);
  int32_t v2 = sw;
  char unsigned * RADRESTRICT v12 = v0 + v2;
  uint8x8_t v13 = vld1_u8((uint8_t * RADRESTRICT)(v12 + 0));
  int16x8_t v15 = vreinterpretq_s16_u16(vshll_n_u8(v13, 4));
  uint8x8_t v16 = vld1_u8((uint8_t * RADRESTRICT)(v12 + 1));
  int16x8_t v18 = vreinterpretq_s16_u16(vshll_n_u8(v16, 4));
  int16x8_t v19 = vsubq_s16(v18, v15);
  int16x8_t v20 = vsraq_n_s16(v15, v19, 2);
  uint8x8_t v4 = vld1_u8((uint8_t * RADRESTRICT)(v0 + 0));
  int16x8_t v6 = vreinterpretq_s16_u16(vshll_n_u8(v4, 4));
  uint8x8_t v7 = vld1_u8((uint8_t * RADRESTRICT)(v0 + 1));
  int16x8_t v9 = vreinterpretq_s16_u16(vshll_n_u8(v7, 4));
  int16x8_t v10 = vsubq_s16(v9, v6);
  int16x8_t v11 = vsraq_n_s16(v6, v10, 2);
  int16x8_t v85 = vsubq_s16(v11, v20);
  int16x8_t v86 = vsraq_n_s16(v20, v85, 2);
  uint8x8_t v88 = vqrshrun_n_s16(v86, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v1 + 0), v88);

  {
  int32_t v3 = dw;
  char unsigned * RADRESTRICT v118 = v1 + v3;
  char unsigned * RADRESTRICT v21 = v12 + v2;
  uint8x8_t v22 = vld1_u8((uint8_t * RADRESTRICT)(v21 + 0));
  int16x8_t v24 = vreinterpretq_s16_u16(vshll_n_u8(v22, 4));
  uint8x8_t v25 = vld1_u8((uint8_t * RADRESTRICT)(v21 + 1));
  int16x8_t v27 = vreinterpretq_s16_u16(vshll_n_u8(v25, 4));
  int16x8_t v28 = vsubq_s16(v27, v24);
  int16x8_t v29 = vsraq_n_s16(v24, v28, 2);
  int16x8_t v89 = vsubq_s16(v20, v29);
  int16x8_t v90 = vsraq_n_s16(v29, v89, 2);
  uint8x8_t v92 = vqrshrun_n_s16(v90, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v118 + 0), v92);

  {
  char unsigned * RADRESTRICT v120 = v118 + v3;
  char unsigned * RADRESTRICT v30 = v21 + v2;
  uint8x8_t v31 = vld1_u8((uint8_t * RADRESTRICT)(v30 + 0));
  int16x8_t v33 = vreinterpretq_s16_u16(vshll_n_u8(v31, 4));
  uint8x8_t v34 = vld1_u8((uint8_t * RADRESTRICT)(v30 + 1));
  int16x8_t v36 = vreinterpretq_s16_u16(vshll_n_u8(v34, 4));
  int16x8_t v37 = vsubq_s16(v36, v33);
  int16x8_t v38 = vsraq_n_s16(v33, v37, 2);
  int16x8_t v93 = vsubq_s16(v29, v38);
  int16x8_t v94 = vsraq_n_s16(v38, v93, 2);
  uint8x8_t v96 = vqrshrun_n_s16(v94, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v120 + 0), v96);

  {
  char unsigned * RADRESTRICT v122 = v120 + v3;
  char unsigned * RADRESTRICT v39 = v30 + v2;
  uint8x8_t v40 = vld1_u8((uint8_t * RADRESTRICT)(v39 + 0));
  int16x8_t v42 = vreinterpretq_s16_u16(vshll_n_u8(v40, 4));
  uint8x8_t v43 = vld1_u8((uint8_t * RADRESTRICT)(v39 + 1));
  int16x8_t v45 = vreinterpretq_s16_u16(vshll_n_u8(v43, 4));
  int16x8_t v46 = vsubq_s16(v45, v42);
  int16x8_t v47 = vsraq_n_s16(v42, v46, 2);
  int16x8_t v97 = vsubq_s16(v38, v47);
  int16x8_t v98 = vsraq_n_s16(v47, v97, 2);
  uint8x8_t v100 = vqrshrun_n_s16(v98, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v122 + 0), v100);

  {
  char unsigned * RADRESTRICT v124 = v122 + v3;
  char unsigned * RADRESTRICT v48 = v39 + v2;
  uint8x8_t v49 = vld1_u8((uint8_t * RADRESTRICT)(v48 + 0));
  int16x8_t v51 = vreinterpretq_s16_u16(vshll_n_u8(v49, 4));
  uint8x8_t v52 = vld1_u8((uint8_t * RADRESTRICT)(v48 + 1));
  int16x8_t v54 = vreinterpretq_s16_u16(vshll_n_u8(v52, 4));
  int16x8_t v55 = vsubq_s16(v54, v51);
  int16x8_t v56 = vsraq_n_s16(v51, v55, 2);
  int16x8_t v101 = vsubq_s16(v47, v56);
  int16x8_t v102 = vsraq_n_s16(v56, v101, 2);
  uint8x8_t v104 = vqrshrun_n_s16(v102, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v124 + 0), v104);

  {
  char unsigned * RADRESTRICT v126 = v124 + v3;
  char unsigned * RADRESTRICT v57 = v48 + v2;
  uint8x8_t v58 = vld1_u8((uint8_t * RADRESTRICT)(v57 + 0));
  int16x8_t v60 = vreinterpretq_s16_u16(vshll_n_u8(v58, 4));
  uint8x8_t v61 = vld1_u8((uint8_t * RADRESTRICT)(v57 + 1));
  int16x8_t v63 = vreinterpretq_s16_u16(vshll_n_u8(v61, 4));
  int16x8_t v64 = vsubq_s16(v63, v60);
  int16x8_t v65 = vsraq_n_s16(v60, v64, 2);
  int16x8_t v105 = vsubq_s16(v56, v65);
  int16x8_t v106 = vsraq_n_s16(v65, v105, 2);
  uint8x8_t v108 = vqrshrun_n_s16(v106, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v126 + 0), v108);

  {
  char unsigned * RADRESTRICT v128 = v126 + v3;
  char unsigned * RADRESTRICT v66 = v57 + v2;
  uint8x8_t v67 = vld1_u8((uint8_t * RADRESTRICT)(v66 + 0));
  int16x8_t v69 = vreinterpretq_s16_u16(vshll_n_u8(v67, 4));
  uint8x8_t v70 = vld1_u8((uint8_t * RADRESTRICT)(v66 + 1));
  int16x8_t v72 = vreinterpretq_s16_u16(vshll_n_u8(v70, 4));
  int16x8_t v73 = vsubq_s16(v72, v69);
  int16x8_t v74 = vsraq_n_s16(v69, v73, 2);
  int16x8_t v109 = vsubq_s16(v65, v74);
  int16x8_t v110 = vsraq_n_s16(v74, v109, 2);
  uint8x8_t v112 = vqrshrun_n_s16(v110, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v128 + 0), v112);

  {
  char unsigned * RADRESTRICT v130 = v128 + v3;
  char unsigned * RADRESTRICT v75 = v66 + v2;
  uint8x8_t v76 = vld1_u8((uint8_t * RADRESTRICT)(v75 + 0));
  int16x8_t v78 = vreinterpretq_s16_u16(vshll_n_u8(v76, 4));
  uint8x8_t v79 = vld1_u8((uint8_t * RADRESTRICT)(v75 + 1));
  int16x8_t v81 = vreinterpretq_s16_u16(vshll_n_u8(v79, 4));
  int16x8_t v82 = vsubq_s16(v81, v78);
  int16x8_t v83 = vsraq_n_s16(v78, v82, 2);
  int16x8_t v113 = vsubq_s16(v74, v83);
  int16x8_t v114 = vsraq_n_s16(v83, v113, 2);
  uint8x8_t v116 = vqrshrun_n_s16(v114, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v130 + 0), v116);
  }
  }
  }
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void bilinear_8x8_h50_v0(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int32_t sw, int32_t dw)
{
  {
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dest);
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(src);
  uint8x8_t v4 = vld1_u8((uint8_t * RADRESTRICT)(v0 + 0));
  uint8x8_t v7 = vld1_u8((uint8_t * RADRESTRICT)(v0 + 1));
  uint8x8_t v93 = vrhadd_u8(v4, v7);
  vst1_u8((uint8_t * RADRESTRICT)(v1 + 0), v93);

  {
  int32_t v3 = dw;
  char unsigned * RADRESTRICT v94 = v1 + v3;
  int32_t v2 = sw;
  char unsigned * RADRESTRICT v11 = v0 + v2;
  uint8x8_t v12 = vld1_u8((uint8_t * RADRESTRICT)(v11 + 0));
  uint8x8_t v15 = vld1_u8((uint8_t * RADRESTRICT)(v11 + 1));
  uint8x8_t v96 = vrhadd_u8(v12, v15);
  vst1_u8((uint8_t * RADRESTRICT)(v94 + 0), v96);

  {
  char unsigned * RADRESTRICT v97 = v94 + v3;
  char unsigned * RADRESTRICT v19 = v11 + v2;
  uint8x8_t v20 = vld1_u8((uint8_t * RADRESTRICT)(v19 + 0));
  uint8x8_t v23 = vld1_u8((uint8_t * RADRESTRICT)(v19 + 1));
  uint8x8_t v99 = vrhadd_u8(v20, v23);
  vst1_u8((uint8_t * RADRESTRICT)(v97 + 0), v99);

  {
  char unsigned * RADRESTRICT v100 = v97 + v3;
  char unsigned * RADRESTRICT v27 = v19 + v2;
  uint8x8_t v28 = vld1_u8((uint8_t * RADRESTRICT)(v27 + 0));
  uint8x8_t v31 = vld1_u8((uint8_t * RADRESTRICT)(v27 + 1));
  uint8x8_t v102 = vrhadd_u8(v28, v31);
  vst1_u8((uint8_t * RADRESTRICT)(v100 + 0), v102);

  {
  char unsigned * RADRESTRICT v103 = v100 + v3;
  char unsigned * RADRESTRICT v35 = v27 + v2;
  uint8x8_t v36 = vld1_u8((uint8_t * RADRESTRICT)(v35 + 0));
  uint8x8_t v39 = vld1_u8((uint8_t * RADRESTRICT)(v35 + 1));
  uint8x8_t v105 = vrhadd_u8(v36, v39);
  vst1_u8((uint8_t * RADRESTRICT)(v103 + 0), v105);

  {
  char unsigned * RADRESTRICT v106 = v103 + v3;
  char unsigned * RADRESTRICT v43 = v35 + v2;
  uint8x8_t v44 = vld1_u8((uint8_t * RADRESTRICT)(v43 + 0));
  uint8x8_t v47 = vld1_u8((uint8_t * RADRESTRICT)(v43 + 1));
  uint8x8_t v108 = vrhadd_u8(v44, v47);
  vst1_u8((uint8_t * RADRESTRICT)(v106 + 0), v108);

  {
  char unsigned * RADRESTRICT v109 = v106 + v3;
  char unsigned * RADRESTRICT v51 = v43 + v2;
  uint8x8_t v52 = vld1_u8((uint8_t * RADRESTRICT)(v51 + 0));
  uint8x8_t v55 = vld1_u8((uint8_t * RADRESTRICT)(v51 + 1));
  uint8x8_t v111 = vrhadd_u8(v52, v55);
  vst1_u8((uint8_t * RADRESTRICT)(v109 + 0), v111);

  {
  char unsigned * RADRESTRICT v112 = v109 + v3;
  char unsigned * RADRESTRICT v59 = v51 + v2;
  uint8x8_t v60 = vld1_u8((uint8_t * RADRESTRICT)(v59 + 0));
  uint8x8_t v63 = vld1_u8((uint8_t * RADRESTRICT)(v59 + 1));
  uint8x8_t v114 = vrhadd_u8(v60, v63);
  vst1_u8((uint8_t * RADRESTRICT)(v112 + 0), v114);
  }
  }
  }
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void bilinear_8x8_h50_v25(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int32_t sw, int32_t dw)
{
  {
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dest);
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(src);
  uint8x8_t v4 = vld1_u8((uint8_t * RADRESTRICT)(v0 + 0));
  int16x8_t v6 = vreinterpretq_s16_u16(vshll_n_u8(v4, 4));
  uint8x8_t v7 = vld1_u8((uint8_t * RADRESTRICT)(v0 + 1));
  int16x8_t v9 = vreinterpretq_s16_u16(vshll_n_u8(v7, 4));
  int16x8_t v10 = vrhaddq_s16(v6, v9);
  int32_t v2 = sw;
  char unsigned * RADRESTRICT v11 = v0 + v2;
  uint8x8_t v12 = vld1_u8((uint8_t * RADRESTRICT)(v11 + 0));
  int16x8_t v14 = vreinterpretq_s16_u16(vshll_n_u8(v12, 4));
  uint8x8_t v15 = vld1_u8((uint8_t * RADRESTRICT)(v11 + 1));
  int16x8_t v17 = vreinterpretq_s16_u16(vshll_n_u8(v15, 4));
  int16x8_t v18 = vrhaddq_s16(v14, v17);
  int16x8_t v76 = vsubq_s16(v18, v10);
  int16x8_t v77 = vsraq_n_s16(v10, v76, 2);
  uint8x8_t v79 = vqrshrun_n_s16(v77, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v1 + 0), v79);

  {
  int32_t v3 = dw;
  char unsigned * RADRESTRICT v109 = v1 + v3;
  char unsigned * RADRESTRICT v19 = v11 + v2;
  uint8x8_t v20 = vld1_u8((uint8_t * RADRESTRICT)(v19 + 0));
  int16x8_t v22 = vreinterpretq_s16_u16(vshll_n_u8(v20, 4));
  uint8x8_t v23 = vld1_u8((uint8_t * RADRESTRICT)(v19 + 1));
  int16x8_t v25 = vreinterpretq_s16_u16(vshll_n_u8(v23, 4));
  int16x8_t v26 = vrhaddq_s16(v22, v25);
  int16x8_t v80 = vsubq_s16(v26, v18);
  int16x8_t v81 = vsraq_n_s16(v18, v80, 2);
  uint8x8_t v83 = vqrshrun_n_s16(v81, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v109 + 0), v83);

  {
  char unsigned * RADRESTRICT v111 = v109 + v3;
  char unsigned * RADRESTRICT v27 = v19 + v2;
  uint8x8_t v28 = vld1_u8((uint8_t * RADRESTRICT)(v27 + 0));
  int16x8_t v30 = vreinterpretq_s16_u16(vshll_n_u8(v28, 4));
  uint8x8_t v31 = vld1_u8((uint8_t * RADRESTRICT)(v27 + 1));
  int16x8_t v33 = vreinterpretq_s16_u16(vshll_n_u8(v31, 4));
  int16x8_t v34 = vrhaddq_s16(v30, v33);
  int16x8_t v84 = vsubq_s16(v34, v26);
  int16x8_t v85 = vsraq_n_s16(v26, v84, 2);
  uint8x8_t v87 = vqrshrun_n_s16(v85, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v111 + 0), v87);

  {
  char unsigned * RADRESTRICT v113 = v111 + v3;
  char unsigned * RADRESTRICT v35 = v27 + v2;
  uint8x8_t v36 = vld1_u8((uint8_t * RADRESTRICT)(v35 + 0));
  int16x8_t v38 = vreinterpretq_s16_u16(vshll_n_u8(v36, 4));
  uint8x8_t v39 = vld1_u8((uint8_t * RADRESTRICT)(v35 + 1));
  int16x8_t v41 = vreinterpretq_s16_u16(vshll_n_u8(v39, 4));
  int16x8_t v42 = vrhaddq_s16(v38, v41);
  int16x8_t v88 = vsubq_s16(v42, v34);
  int16x8_t v89 = vsraq_n_s16(v34, v88, 2);
  uint8x8_t v91 = vqrshrun_n_s16(v89, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v113 + 0), v91);

  {
  char unsigned * RADRESTRICT v115 = v113 + v3;
  char unsigned * RADRESTRICT v43 = v35 + v2;
  uint8x8_t v44 = vld1_u8((uint8_t * RADRESTRICT)(v43 + 0));
  int16x8_t v46 = vreinterpretq_s16_u16(vshll_n_u8(v44, 4));
  uint8x8_t v47 = vld1_u8((uint8_t * RADRESTRICT)(v43 + 1));
  int16x8_t v49 = vreinterpretq_s16_u16(vshll_n_u8(v47, 4));
  int16x8_t v50 = vrhaddq_s16(v46, v49);
  int16x8_t v92 = vsubq_s16(v50, v42);
  int16x8_t v93 = vsraq_n_s16(v42, v92, 2);
  uint8x8_t v95 = vqrshrun_n_s16(v93, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v115 + 0), v95);

  {
  char unsigned * RADRESTRICT v117 = v115 + v3;
  char unsigned * RADRESTRICT v51 = v43 + v2;
  uint8x8_t v52 = vld1_u8((uint8_t * RADRESTRICT)(v51 + 0));
  int16x8_t v54 = vreinterpretq_s16_u16(vshll_n_u8(v52, 4));
  uint8x8_t v55 = vld1_u8((uint8_t * RADRESTRICT)(v51 + 1));
  int16x8_t v57 = vreinterpretq_s16_u16(vshll_n_u8(v55, 4));
  int16x8_t v58 = vrhaddq_s16(v54, v57);
  int16x8_t v96 = vsubq_s16(v58, v50);
  int16x8_t v97 = vsraq_n_s16(v50, v96, 2);
  uint8x8_t v99 = vqrshrun_n_s16(v97, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v117 + 0), v99);

  {
  char unsigned * RADRESTRICT v119 = v117 + v3;
  char unsigned * RADRESTRICT v59 = v51 + v2;
  uint8x8_t v60 = vld1_u8((uint8_t * RADRESTRICT)(v59 + 0));
  int16x8_t v62 = vreinterpretq_s16_u16(vshll_n_u8(v60, 4));
  uint8x8_t v63 = vld1_u8((uint8_t * RADRESTRICT)(v59 + 1));
  int16x8_t v65 = vreinterpretq_s16_u16(vshll_n_u8(v63, 4));
  int16x8_t v66 = vrhaddq_s16(v62, v65);
  int16x8_t v100 = vsubq_s16(v66, v58);
  int16x8_t v101 = vsraq_n_s16(v58, v100, 2);
  uint8x8_t v103 = vqrshrun_n_s16(v101, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v119 + 0), v103);

  {
  char unsigned * RADRESTRICT v121 = v119 + v3;
  char unsigned * RADRESTRICT v67 = v59 + v2;
  uint8x8_t v68 = vld1_u8((uint8_t * RADRESTRICT)(v67 + 0));
  int16x8_t v70 = vreinterpretq_s16_u16(vshll_n_u8(v68, 4));
  uint8x8_t v71 = vld1_u8((uint8_t * RADRESTRICT)(v67 + 1));
  int16x8_t v73 = vreinterpretq_s16_u16(vshll_n_u8(v71, 4));
  int16x8_t v74 = vrhaddq_s16(v70, v73);
  int16x8_t v104 = vsubq_s16(v74, v66);
  int16x8_t v105 = vsraq_n_s16(v66, v104, 2);
  uint8x8_t v107 = vqrshrun_n_s16(v105, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v121 + 0), v107);
  }
  }
  }
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void bilinear_8x8_h50_v50(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int32_t sw, int32_t dw)
{
  {
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dest);
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(src);
  uint8x8_t v4 = vld1_u8((uint8_t * RADRESTRICT)(v0 + 0));
  int16x8_t v6 = vreinterpretq_s16_u16(vshll_n_u8(v4, 4));
  uint8x8_t v7 = vld1_u8((uint8_t * RADRESTRICT)(v0 + 1));
  int16x8_t v9 = vreinterpretq_s16_u16(vshll_n_u8(v7, 4));
  int16x8_t v10 = vrhaddq_s16(v6, v9);
  int32_t v2 = sw;
  char unsigned * RADRESTRICT v11 = v0 + v2;
  uint8x8_t v12 = vld1_u8((uint8_t * RADRESTRICT)(v11 + 0));
  int16x8_t v14 = vreinterpretq_s16_u16(vshll_n_u8(v12, 4));
  uint8x8_t v15 = vld1_u8((uint8_t * RADRESTRICT)(v11 + 1));
  int16x8_t v17 = vreinterpretq_s16_u16(vshll_n_u8(v15, 4));
  int16x8_t v18 = vrhaddq_s16(v14, v17);
  int16x8_t v76 = vrhaddq_s16(v10, v18);
  uint8x8_t v78 = vqrshrun_n_s16(v76, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v1 + 0), v78);

  {
  int32_t v3 = dw;
  char unsigned * RADRESTRICT v101 = v1 + v3;
  char unsigned * RADRESTRICT v19 = v11 + v2;
  uint8x8_t v20 = vld1_u8((uint8_t * RADRESTRICT)(v19 + 0));
  int16x8_t v22 = vreinterpretq_s16_u16(vshll_n_u8(v20, 4));
  uint8x8_t v23 = vld1_u8((uint8_t * RADRESTRICT)(v19 + 1));
  int16x8_t v25 = vreinterpretq_s16_u16(vshll_n_u8(v23, 4));
  int16x8_t v26 = vrhaddq_s16(v22, v25);
  int16x8_t v79 = vrhaddq_s16(v18, v26);
  uint8x8_t v81 = vqrshrun_n_s16(v79, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v101 + 0), v81);

  {
  char unsigned * RADRESTRICT v103 = v101 + v3;
  char unsigned * RADRESTRICT v27 = v19 + v2;
  uint8x8_t v28 = vld1_u8((uint8_t * RADRESTRICT)(v27 + 0));
  int16x8_t v30 = vreinterpretq_s16_u16(vshll_n_u8(v28, 4));
  uint8x8_t v31 = vld1_u8((uint8_t * RADRESTRICT)(v27 + 1));
  int16x8_t v33 = vreinterpretq_s16_u16(vshll_n_u8(v31, 4));
  int16x8_t v34 = vrhaddq_s16(v30, v33);
  int16x8_t v82 = vrhaddq_s16(v26, v34);
  uint8x8_t v84 = vqrshrun_n_s16(v82, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v103 + 0), v84);

  {
  char unsigned * RADRESTRICT v105 = v103 + v3;
  char unsigned * RADRESTRICT v35 = v27 + v2;
  uint8x8_t v36 = vld1_u8((uint8_t * RADRESTRICT)(v35 + 0));
  int16x8_t v38 = vreinterpretq_s16_u16(vshll_n_u8(v36, 4));
  uint8x8_t v39 = vld1_u8((uint8_t * RADRESTRICT)(v35 + 1));
  int16x8_t v41 = vreinterpretq_s16_u16(vshll_n_u8(v39, 4));
  int16x8_t v42 = vrhaddq_s16(v38, v41);
  int16x8_t v85 = vrhaddq_s16(v34, v42);
  uint8x8_t v87 = vqrshrun_n_s16(v85, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v105 + 0), v87);

  {
  char unsigned * RADRESTRICT v107 = v105 + v3;
  char unsigned * RADRESTRICT v43 = v35 + v2;
  uint8x8_t v44 = vld1_u8((uint8_t * RADRESTRICT)(v43 + 0));
  int16x8_t v46 = vreinterpretq_s16_u16(vshll_n_u8(v44, 4));
  uint8x8_t v47 = vld1_u8((uint8_t * RADRESTRICT)(v43 + 1));
  int16x8_t v49 = vreinterpretq_s16_u16(vshll_n_u8(v47, 4));
  int16x8_t v50 = vrhaddq_s16(v46, v49);
  int16x8_t v88 = vrhaddq_s16(v42, v50);
  uint8x8_t v90 = vqrshrun_n_s16(v88, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v107 + 0), v90);

  {
  char unsigned * RADRESTRICT v109 = v107 + v3;
  char unsigned * RADRESTRICT v51 = v43 + v2;
  uint8x8_t v52 = vld1_u8((uint8_t * RADRESTRICT)(v51 + 0));
  int16x8_t v54 = vreinterpretq_s16_u16(vshll_n_u8(v52, 4));
  uint8x8_t v55 = vld1_u8((uint8_t * RADRESTRICT)(v51 + 1));
  int16x8_t v57 = vreinterpretq_s16_u16(vshll_n_u8(v55, 4));
  int16x8_t v58 = vrhaddq_s16(v54, v57);
  int16x8_t v91 = vrhaddq_s16(v50, v58);
  uint8x8_t v93 = vqrshrun_n_s16(v91, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v109 + 0), v93);

  {
  char unsigned * RADRESTRICT v111 = v109 + v3;
  char unsigned * RADRESTRICT v59 = v51 + v2;
  uint8x8_t v60 = vld1_u8((uint8_t * RADRESTRICT)(v59 + 0));
  int16x8_t v62 = vreinterpretq_s16_u16(vshll_n_u8(v60, 4));
  uint8x8_t v63 = vld1_u8((uint8_t * RADRESTRICT)(v59 + 1));
  int16x8_t v65 = vreinterpretq_s16_u16(vshll_n_u8(v63, 4));
  int16x8_t v66 = vrhaddq_s16(v62, v65);
  int16x8_t v94 = vrhaddq_s16(v58, v66);
  uint8x8_t v96 = vqrshrun_n_s16(v94, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v111 + 0), v96);

  {
  char unsigned * RADRESTRICT v113 = v111 + v3;
  char unsigned * RADRESTRICT v67 = v59 + v2;
  uint8x8_t v68 = vld1_u8((uint8_t * RADRESTRICT)(v67 + 0));
  int16x8_t v70 = vreinterpretq_s16_u16(vshll_n_u8(v68, 4));
  uint8x8_t v71 = vld1_u8((uint8_t * RADRESTRICT)(v67 + 1));
  int16x8_t v73 = vreinterpretq_s16_u16(vshll_n_u8(v71, 4));
  int16x8_t v74 = vrhaddq_s16(v70, v73);
  int16x8_t v97 = vrhaddq_s16(v66, v74);
  uint8x8_t v99 = vqrshrun_n_s16(v97, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v113 + 0), v99);
  }
  }
  }
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void bilinear_8x8_h50_v75(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int32_t sw, int32_t dw)
{
  {
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dest);
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(src);
  int32_t v2 = sw;
  char unsigned * RADRESTRICT v11 = v0 + v2;
  uint8x8_t v12 = vld1_u8((uint8_t * RADRESTRICT)(v11 + 0));
  int16x8_t v14 = vreinterpretq_s16_u16(vshll_n_u8(v12, 4));
  uint8x8_t v15 = vld1_u8((uint8_t * RADRESTRICT)(v11 + 1));
  int16x8_t v17 = vreinterpretq_s16_u16(vshll_n_u8(v15, 4));
  int16x8_t v18 = vrhaddq_s16(v14, v17);
  uint8x8_t v4 = vld1_u8((uint8_t * RADRESTRICT)(v0 + 0));
  int16x8_t v6 = vreinterpretq_s16_u16(vshll_n_u8(v4, 4));
  uint8x8_t v7 = vld1_u8((uint8_t * RADRESTRICT)(v0 + 1));
  int16x8_t v9 = vreinterpretq_s16_u16(vshll_n_u8(v7, 4));
  int16x8_t v10 = vrhaddq_s16(v6, v9);
  int16x8_t v76 = vsubq_s16(v10, v18);
  int16x8_t v77 = vsraq_n_s16(v18, v76, 2);
  uint8x8_t v79 = vqrshrun_n_s16(v77, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v1 + 0), v79);

  {
  int32_t v3 = dw;
  char unsigned * RADRESTRICT v109 = v1 + v3;
  char unsigned * RADRESTRICT v19 = v11 + v2;
  uint8x8_t v20 = vld1_u8((uint8_t * RADRESTRICT)(v19 + 0));
  int16x8_t v22 = vreinterpretq_s16_u16(vshll_n_u8(v20, 4));
  uint8x8_t v23 = vld1_u8((uint8_t * RADRESTRICT)(v19 + 1));
  int16x8_t v25 = vreinterpretq_s16_u16(vshll_n_u8(v23, 4));
  int16x8_t v26 = vrhaddq_s16(v22, v25);
  int16x8_t v80 = vsubq_s16(v18, v26);
  int16x8_t v81 = vsraq_n_s16(v26, v80, 2);
  uint8x8_t v83 = vqrshrun_n_s16(v81, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v109 + 0), v83);

  {
  char unsigned * RADRESTRICT v111 = v109 + v3;
  char unsigned * RADRESTRICT v27 = v19 + v2;
  uint8x8_t v28 = vld1_u8((uint8_t * RADRESTRICT)(v27 + 0));
  int16x8_t v30 = vreinterpretq_s16_u16(vshll_n_u8(v28, 4));
  uint8x8_t v31 = vld1_u8((uint8_t * RADRESTRICT)(v27 + 1));
  int16x8_t v33 = vreinterpretq_s16_u16(vshll_n_u8(v31, 4));
  int16x8_t v34 = vrhaddq_s16(v30, v33);
  int16x8_t v84 = vsubq_s16(v26, v34);
  int16x8_t v85 = vsraq_n_s16(v34, v84, 2);
  uint8x8_t v87 = vqrshrun_n_s16(v85, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v111 + 0), v87);

  {
  char unsigned * RADRESTRICT v113 = v111 + v3;
  char unsigned * RADRESTRICT v35 = v27 + v2;
  uint8x8_t v36 = vld1_u8((uint8_t * RADRESTRICT)(v35 + 0));
  int16x8_t v38 = vreinterpretq_s16_u16(vshll_n_u8(v36, 4));
  uint8x8_t v39 = vld1_u8((uint8_t * RADRESTRICT)(v35 + 1));
  int16x8_t v41 = vreinterpretq_s16_u16(vshll_n_u8(v39, 4));
  int16x8_t v42 = vrhaddq_s16(v38, v41);
  int16x8_t v88 = vsubq_s16(v34, v42);
  int16x8_t v89 = vsraq_n_s16(v42, v88, 2);
  uint8x8_t v91 = vqrshrun_n_s16(v89, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v113 + 0), v91);

  {
  char unsigned * RADRESTRICT v115 = v113 + v3;
  char unsigned * RADRESTRICT v43 = v35 + v2;
  uint8x8_t v44 = vld1_u8((uint8_t * RADRESTRICT)(v43 + 0));
  int16x8_t v46 = vreinterpretq_s16_u16(vshll_n_u8(v44, 4));
  uint8x8_t v47 = vld1_u8((uint8_t * RADRESTRICT)(v43 + 1));
  int16x8_t v49 = vreinterpretq_s16_u16(vshll_n_u8(v47, 4));
  int16x8_t v50 = vrhaddq_s16(v46, v49);
  int16x8_t v92 = vsubq_s16(v42, v50);
  int16x8_t v93 = vsraq_n_s16(v50, v92, 2);
  uint8x8_t v95 = vqrshrun_n_s16(v93, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v115 + 0), v95);

  {
  char unsigned * RADRESTRICT v117 = v115 + v3;
  char unsigned * RADRESTRICT v51 = v43 + v2;
  uint8x8_t v52 = vld1_u8((uint8_t * RADRESTRICT)(v51 + 0));
  int16x8_t v54 = vreinterpretq_s16_u16(vshll_n_u8(v52, 4));
  uint8x8_t v55 = vld1_u8((uint8_t * RADRESTRICT)(v51 + 1));
  int16x8_t v57 = vreinterpretq_s16_u16(vshll_n_u8(v55, 4));
  int16x8_t v58 = vrhaddq_s16(v54, v57);
  int16x8_t v96 = vsubq_s16(v50, v58);
  int16x8_t v97 = vsraq_n_s16(v58, v96, 2);
  uint8x8_t v99 = vqrshrun_n_s16(v97, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v117 + 0), v99);

  {
  char unsigned * RADRESTRICT v119 = v117 + v3;
  char unsigned * RADRESTRICT v59 = v51 + v2;
  uint8x8_t v60 = vld1_u8((uint8_t * RADRESTRICT)(v59 + 0));
  int16x8_t v62 = vreinterpretq_s16_u16(vshll_n_u8(v60, 4));
  uint8x8_t v63 = vld1_u8((uint8_t * RADRESTRICT)(v59 + 1));
  int16x8_t v65 = vreinterpretq_s16_u16(vshll_n_u8(v63, 4));
  int16x8_t v66 = vrhaddq_s16(v62, v65);
  int16x8_t v100 = vsubq_s16(v58, v66);
  int16x8_t v101 = vsraq_n_s16(v66, v100, 2);
  uint8x8_t v103 = vqrshrun_n_s16(v101, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v119 + 0), v103);

  {
  char unsigned * RADRESTRICT v121 = v119 + v3;
  char unsigned * RADRESTRICT v67 = v59 + v2;
  uint8x8_t v68 = vld1_u8((uint8_t * RADRESTRICT)(v67 + 0));
  int16x8_t v70 = vreinterpretq_s16_u16(vshll_n_u8(v68, 4));
  uint8x8_t v71 = vld1_u8((uint8_t * RADRESTRICT)(v67 + 1));
  int16x8_t v73 = vreinterpretq_s16_u16(vshll_n_u8(v71, 4));
  int16x8_t v74 = vrhaddq_s16(v70, v73);
  int16x8_t v104 = vsubq_s16(v66, v74);
  int16x8_t v105 = vsraq_n_s16(v74, v104, 2);
  uint8x8_t v107 = vqrshrun_n_s16(v105, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v121 + 0), v107);
  }
  }
  }
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void bilinear_8x8_h75_v0(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int32_t sw, int32_t dw)
{
  {
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dest);
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(src);
  uint8x8_t v7 = vld1_u8((uint8_t * RADRESTRICT)(v0 + 1));
  int16x8_t v9 = vreinterpretq_s16_u16(vshll_n_u8(v7, 4));
  uint8x8_t v4 = vld1_u8((uint8_t * RADRESTRICT)(v0 + 0));
  int16x8_t v6 = vreinterpretq_s16_u16(vshll_n_u8(v4, 4));
  int16x8_t v10 = vsubq_s16(v6, v9);
  int16x8_t v11 = vsraq_n_s16(v9, v10, 2);
  uint8x8_t v86 = vqrshrun_n_s16(v11, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v1 + 0), v86);

  {
  int32_t v3 = dw;
  char unsigned * RADRESTRICT v102 = v1 + v3;
  int32_t v2 = sw;
  char unsigned * RADRESTRICT v12 = v0 + v2;
  uint8x8_t v16 = vld1_u8((uint8_t * RADRESTRICT)(v12 + 1));
  int16x8_t v18 = vreinterpretq_s16_u16(vshll_n_u8(v16, 4));
  uint8x8_t v13 = vld1_u8((uint8_t * RADRESTRICT)(v12 + 0));
  int16x8_t v15 = vreinterpretq_s16_u16(vshll_n_u8(v13, 4));
  int16x8_t v19 = vsubq_s16(v15, v18);
  int16x8_t v20 = vsraq_n_s16(v18, v19, 2);
  uint8x8_t v88 = vqrshrun_n_s16(v20, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v102 + 0), v88);

  {
  char unsigned * RADRESTRICT v104 = v102 + v3;
  char unsigned * RADRESTRICT v21 = v12 + v2;
  uint8x8_t v25 = vld1_u8((uint8_t * RADRESTRICT)(v21 + 1));
  int16x8_t v27 = vreinterpretq_s16_u16(vshll_n_u8(v25, 4));
  uint8x8_t v22 = vld1_u8((uint8_t * RADRESTRICT)(v21 + 0));
  int16x8_t v24 = vreinterpretq_s16_u16(vshll_n_u8(v22, 4));
  int16x8_t v28 = vsubq_s16(v24, v27);
  int16x8_t v29 = vsraq_n_s16(v27, v28, 2);
  uint8x8_t v90 = vqrshrun_n_s16(v29, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v104 + 0), v90);

  {
  char unsigned * RADRESTRICT v106 = v104 + v3;
  char unsigned * RADRESTRICT v30 = v21 + v2;
  uint8x8_t v34 = vld1_u8((uint8_t * RADRESTRICT)(v30 + 1));
  int16x8_t v36 = vreinterpretq_s16_u16(vshll_n_u8(v34, 4));
  uint8x8_t v31 = vld1_u8((uint8_t * RADRESTRICT)(v30 + 0));
  int16x8_t v33 = vreinterpretq_s16_u16(vshll_n_u8(v31, 4));
  int16x8_t v37 = vsubq_s16(v33, v36);
  int16x8_t v38 = vsraq_n_s16(v36, v37, 2);
  uint8x8_t v92 = vqrshrun_n_s16(v38, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v106 + 0), v92);

  {
  char unsigned * RADRESTRICT v108 = v106 + v3;
  char unsigned * RADRESTRICT v39 = v30 + v2;
  uint8x8_t v43 = vld1_u8((uint8_t * RADRESTRICT)(v39 + 1));
  int16x8_t v45 = vreinterpretq_s16_u16(vshll_n_u8(v43, 4));
  uint8x8_t v40 = vld1_u8((uint8_t * RADRESTRICT)(v39 + 0));
  int16x8_t v42 = vreinterpretq_s16_u16(vshll_n_u8(v40, 4));
  int16x8_t v46 = vsubq_s16(v42, v45);
  int16x8_t v47 = vsraq_n_s16(v45, v46, 2);
  uint8x8_t v94 = vqrshrun_n_s16(v47, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v108 + 0), v94);

  {
  char unsigned * RADRESTRICT v110 = v108 + v3;
  char unsigned * RADRESTRICT v48 = v39 + v2;
  uint8x8_t v52 = vld1_u8((uint8_t * RADRESTRICT)(v48 + 1));
  int16x8_t v54 = vreinterpretq_s16_u16(vshll_n_u8(v52, 4));
  uint8x8_t v49 = vld1_u8((uint8_t * RADRESTRICT)(v48 + 0));
  int16x8_t v51 = vreinterpretq_s16_u16(vshll_n_u8(v49, 4));
  int16x8_t v55 = vsubq_s16(v51, v54);
  int16x8_t v56 = vsraq_n_s16(v54, v55, 2);
  uint8x8_t v96 = vqrshrun_n_s16(v56, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v110 + 0), v96);

  {
  char unsigned * RADRESTRICT v112 = v110 + v3;
  char unsigned * RADRESTRICT v57 = v48 + v2;
  uint8x8_t v61 = vld1_u8((uint8_t * RADRESTRICT)(v57 + 1));
  int16x8_t v63 = vreinterpretq_s16_u16(vshll_n_u8(v61, 4));
  uint8x8_t v58 = vld1_u8((uint8_t * RADRESTRICT)(v57 + 0));
  int16x8_t v60 = vreinterpretq_s16_u16(vshll_n_u8(v58, 4));
  int16x8_t v64 = vsubq_s16(v60, v63);
  int16x8_t v65 = vsraq_n_s16(v63, v64, 2);
  uint8x8_t v98 = vqrshrun_n_s16(v65, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v112 + 0), v98);

  {
  char unsigned * RADRESTRICT v114 = v112 + v3;
  char unsigned * RADRESTRICT v66 = v57 + v2;
  uint8x8_t v70 = vld1_u8((uint8_t * RADRESTRICT)(v66 + 1));
  int16x8_t v72 = vreinterpretq_s16_u16(vshll_n_u8(v70, 4));
  uint8x8_t v67 = vld1_u8((uint8_t * RADRESTRICT)(v66 + 0));
  int16x8_t v69 = vreinterpretq_s16_u16(vshll_n_u8(v67, 4));
  int16x8_t v73 = vsubq_s16(v69, v72);
  int16x8_t v74 = vsraq_n_s16(v72, v73, 2);
  uint8x8_t v100 = vqrshrun_n_s16(v74, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v114 + 0), v100);
  }
  }
  }
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void bilinear_8x8_h75_v25(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int32_t sw, int32_t dw)
{
  {
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dest);
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(src);
  uint8x8_t v7 = vld1_u8((uint8_t * RADRESTRICT)(v0 + 1));
  int16x8_t v9 = vreinterpretq_s16_u16(vshll_n_u8(v7, 4));
  uint8x8_t v4 = vld1_u8((uint8_t * RADRESTRICT)(v0 + 0));
  int16x8_t v6 = vreinterpretq_s16_u16(vshll_n_u8(v4, 4));
  int16x8_t v10 = vsubq_s16(v6, v9);
  int16x8_t v11 = vsraq_n_s16(v9, v10, 2);
  int32_t v2 = sw;
  char unsigned * RADRESTRICT v12 = v0 + v2;
  uint8x8_t v16 = vld1_u8((uint8_t * RADRESTRICT)(v12 + 1));
  int16x8_t v18 = vreinterpretq_s16_u16(vshll_n_u8(v16, 4));
  uint8x8_t v13 = vld1_u8((uint8_t * RADRESTRICT)(v12 + 0));
  int16x8_t v15 = vreinterpretq_s16_u16(vshll_n_u8(v13, 4));
  int16x8_t v19 = vsubq_s16(v15, v18);
  int16x8_t v20 = vsraq_n_s16(v18, v19, 2);
  int16x8_t v85 = vsubq_s16(v20, v11);
  int16x8_t v86 = vsraq_n_s16(v11, v85, 2);
  uint8x8_t v88 = vqrshrun_n_s16(v86, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v1 + 0), v88);

  {
  int32_t v3 = dw;
  char unsigned * RADRESTRICT v118 = v1 + v3;
  char unsigned * RADRESTRICT v21 = v12 + v2;
  uint8x8_t v25 = vld1_u8((uint8_t * RADRESTRICT)(v21 + 1));
  int16x8_t v27 = vreinterpretq_s16_u16(vshll_n_u8(v25, 4));
  uint8x8_t v22 = vld1_u8((uint8_t * RADRESTRICT)(v21 + 0));
  int16x8_t v24 = vreinterpretq_s16_u16(vshll_n_u8(v22, 4));
  int16x8_t v28 = vsubq_s16(v24, v27);
  int16x8_t v29 = vsraq_n_s16(v27, v28, 2);
  int16x8_t v89 = vsubq_s16(v29, v20);
  int16x8_t v90 = vsraq_n_s16(v20, v89, 2);
  uint8x8_t v92 = vqrshrun_n_s16(v90, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v118 + 0), v92);

  {
  char unsigned * RADRESTRICT v120 = v118 + v3;
  char unsigned * RADRESTRICT v30 = v21 + v2;
  uint8x8_t v34 = vld1_u8((uint8_t * RADRESTRICT)(v30 + 1));
  int16x8_t v36 = vreinterpretq_s16_u16(vshll_n_u8(v34, 4));
  uint8x8_t v31 = vld1_u8((uint8_t * RADRESTRICT)(v30 + 0));
  int16x8_t v33 = vreinterpretq_s16_u16(vshll_n_u8(v31, 4));
  int16x8_t v37 = vsubq_s16(v33, v36);
  int16x8_t v38 = vsraq_n_s16(v36, v37, 2);
  int16x8_t v93 = vsubq_s16(v38, v29);
  int16x8_t v94 = vsraq_n_s16(v29, v93, 2);
  uint8x8_t v96 = vqrshrun_n_s16(v94, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v120 + 0), v96);

  {
  char unsigned * RADRESTRICT v122 = v120 + v3;
  char unsigned * RADRESTRICT v39 = v30 + v2;
  uint8x8_t v43 = vld1_u8((uint8_t * RADRESTRICT)(v39 + 1));
  int16x8_t v45 = vreinterpretq_s16_u16(vshll_n_u8(v43, 4));
  uint8x8_t v40 = vld1_u8((uint8_t * RADRESTRICT)(v39 + 0));
  int16x8_t v42 = vreinterpretq_s16_u16(vshll_n_u8(v40, 4));
  int16x8_t v46 = vsubq_s16(v42, v45);
  int16x8_t v47 = vsraq_n_s16(v45, v46, 2);
  int16x8_t v97 = vsubq_s16(v47, v38);
  int16x8_t v98 = vsraq_n_s16(v38, v97, 2);
  uint8x8_t v100 = vqrshrun_n_s16(v98, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v122 + 0), v100);

  {
  char unsigned * RADRESTRICT v124 = v122 + v3;
  char unsigned * RADRESTRICT v48 = v39 + v2;
  uint8x8_t v52 = vld1_u8((uint8_t * RADRESTRICT)(v48 + 1));
  int16x8_t v54 = vreinterpretq_s16_u16(vshll_n_u8(v52, 4));
  uint8x8_t v49 = vld1_u8((uint8_t * RADRESTRICT)(v48 + 0));
  int16x8_t v51 = vreinterpretq_s16_u16(vshll_n_u8(v49, 4));
  int16x8_t v55 = vsubq_s16(v51, v54);
  int16x8_t v56 = vsraq_n_s16(v54, v55, 2);
  int16x8_t v101 = vsubq_s16(v56, v47);
  int16x8_t v102 = vsraq_n_s16(v47, v101, 2);
  uint8x8_t v104 = vqrshrun_n_s16(v102, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v124 + 0), v104);

  {
  char unsigned * RADRESTRICT v126 = v124 + v3;
  char unsigned * RADRESTRICT v57 = v48 + v2;
  uint8x8_t v61 = vld1_u8((uint8_t * RADRESTRICT)(v57 + 1));
  int16x8_t v63 = vreinterpretq_s16_u16(vshll_n_u8(v61, 4));
  uint8x8_t v58 = vld1_u8((uint8_t * RADRESTRICT)(v57 + 0));
  int16x8_t v60 = vreinterpretq_s16_u16(vshll_n_u8(v58, 4));
  int16x8_t v64 = vsubq_s16(v60, v63);
  int16x8_t v65 = vsraq_n_s16(v63, v64, 2);
  int16x8_t v105 = vsubq_s16(v65, v56);
  int16x8_t v106 = vsraq_n_s16(v56, v105, 2);
  uint8x8_t v108 = vqrshrun_n_s16(v106, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v126 + 0), v108);

  {
  char unsigned * RADRESTRICT v128 = v126 + v3;
  char unsigned * RADRESTRICT v66 = v57 + v2;
  uint8x8_t v70 = vld1_u8((uint8_t * RADRESTRICT)(v66 + 1));
  int16x8_t v72 = vreinterpretq_s16_u16(vshll_n_u8(v70, 4));
  uint8x8_t v67 = vld1_u8((uint8_t * RADRESTRICT)(v66 + 0));
  int16x8_t v69 = vreinterpretq_s16_u16(vshll_n_u8(v67, 4));
  int16x8_t v73 = vsubq_s16(v69, v72);
  int16x8_t v74 = vsraq_n_s16(v72, v73, 2);
  int16x8_t v109 = vsubq_s16(v74, v65);
  int16x8_t v110 = vsraq_n_s16(v65, v109, 2);
  uint8x8_t v112 = vqrshrun_n_s16(v110, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v128 + 0), v112);

  {
  char unsigned * RADRESTRICT v130 = v128 + v3;
  char unsigned * RADRESTRICT v75 = v66 + v2;
  uint8x8_t v79 = vld1_u8((uint8_t * RADRESTRICT)(v75 + 1));
  int16x8_t v81 = vreinterpretq_s16_u16(vshll_n_u8(v79, 4));
  uint8x8_t v76 = vld1_u8((uint8_t * RADRESTRICT)(v75 + 0));
  int16x8_t v78 = vreinterpretq_s16_u16(vshll_n_u8(v76, 4));
  int16x8_t v82 = vsubq_s16(v78, v81);
  int16x8_t v83 = vsraq_n_s16(v81, v82, 2);
  int16x8_t v113 = vsubq_s16(v83, v74);
  int16x8_t v114 = vsraq_n_s16(v74, v113, 2);
  uint8x8_t v116 = vqrshrun_n_s16(v114, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v130 + 0), v116);
  }
  }
  }
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void bilinear_8x8_h75_v50(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int32_t sw, int32_t dw)
{
  {
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dest);
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(src);
  uint8x8_t v7 = vld1_u8((uint8_t * RADRESTRICT)(v0 + 1));
  int16x8_t v9 = vreinterpretq_s16_u16(vshll_n_u8(v7, 4));
  uint8x8_t v4 = vld1_u8((uint8_t * RADRESTRICT)(v0 + 0));
  int16x8_t v6 = vreinterpretq_s16_u16(vshll_n_u8(v4, 4));
  int16x8_t v10 = vsubq_s16(v6, v9);
  int16x8_t v11 = vsraq_n_s16(v9, v10, 2);
  int32_t v2 = sw;
  char unsigned * RADRESTRICT v12 = v0 + v2;
  uint8x8_t v16 = vld1_u8((uint8_t * RADRESTRICT)(v12 + 1));
  int16x8_t v18 = vreinterpretq_s16_u16(vshll_n_u8(v16, 4));
  uint8x8_t v13 = vld1_u8((uint8_t * RADRESTRICT)(v12 + 0));
  int16x8_t v15 = vreinterpretq_s16_u16(vshll_n_u8(v13, 4));
  int16x8_t v19 = vsubq_s16(v15, v18);
  int16x8_t v20 = vsraq_n_s16(v18, v19, 2);
  int16x8_t v85 = vrhaddq_s16(v11, v20);
  uint8x8_t v87 = vqrshrun_n_s16(v85, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v1 + 0), v87);

  {
  int32_t v3 = dw;
  char unsigned * RADRESTRICT v110 = v1 + v3;
  char unsigned * RADRESTRICT v21 = v12 + v2;
  uint8x8_t v25 = vld1_u8((uint8_t * RADRESTRICT)(v21 + 1));
  int16x8_t v27 = vreinterpretq_s16_u16(vshll_n_u8(v25, 4));
  uint8x8_t v22 = vld1_u8((uint8_t * RADRESTRICT)(v21 + 0));
  int16x8_t v24 = vreinterpretq_s16_u16(vshll_n_u8(v22, 4));
  int16x8_t v28 = vsubq_s16(v24, v27);
  int16x8_t v29 = vsraq_n_s16(v27, v28, 2);
  int16x8_t v88 = vrhaddq_s16(v20, v29);
  uint8x8_t v90 = vqrshrun_n_s16(v88, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v110 + 0), v90);

  {
  char unsigned * RADRESTRICT v112 = v110 + v3;
  char unsigned * RADRESTRICT v30 = v21 + v2;
  uint8x8_t v34 = vld1_u8((uint8_t * RADRESTRICT)(v30 + 1));
  int16x8_t v36 = vreinterpretq_s16_u16(vshll_n_u8(v34, 4));
  uint8x8_t v31 = vld1_u8((uint8_t * RADRESTRICT)(v30 + 0));
  int16x8_t v33 = vreinterpretq_s16_u16(vshll_n_u8(v31, 4));
  int16x8_t v37 = vsubq_s16(v33, v36);
  int16x8_t v38 = vsraq_n_s16(v36, v37, 2);
  int16x8_t v91 = vrhaddq_s16(v29, v38);
  uint8x8_t v93 = vqrshrun_n_s16(v91, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v112 + 0), v93);

  {
  char unsigned * RADRESTRICT v114 = v112 + v3;
  char unsigned * RADRESTRICT v39 = v30 + v2;
  uint8x8_t v43 = vld1_u8((uint8_t * RADRESTRICT)(v39 + 1));
  int16x8_t v45 = vreinterpretq_s16_u16(vshll_n_u8(v43, 4));
  uint8x8_t v40 = vld1_u8((uint8_t * RADRESTRICT)(v39 + 0));
  int16x8_t v42 = vreinterpretq_s16_u16(vshll_n_u8(v40, 4));
  int16x8_t v46 = vsubq_s16(v42, v45);
  int16x8_t v47 = vsraq_n_s16(v45, v46, 2);
  int16x8_t v94 = vrhaddq_s16(v38, v47);
  uint8x8_t v96 = vqrshrun_n_s16(v94, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v114 + 0), v96);

  {
  char unsigned * RADRESTRICT v116 = v114 + v3;
  char unsigned * RADRESTRICT v48 = v39 + v2;
  uint8x8_t v52 = vld1_u8((uint8_t * RADRESTRICT)(v48 + 1));
  int16x8_t v54 = vreinterpretq_s16_u16(vshll_n_u8(v52, 4));
  uint8x8_t v49 = vld1_u8((uint8_t * RADRESTRICT)(v48 + 0));
  int16x8_t v51 = vreinterpretq_s16_u16(vshll_n_u8(v49, 4));
  int16x8_t v55 = vsubq_s16(v51, v54);
  int16x8_t v56 = vsraq_n_s16(v54, v55, 2);
  int16x8_t v97 = vrhaddq_s16(v47, v56);
  uint8x8_t v99 = vqrshrun_n_s16(v97, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v116 + 0), v99);

  {
  char unsigned * RADRESTRICT v118 = v116 + v3;
  char unsigned * RADRESTRICT v57 = v48 + v2;
  uint8x8_t v61 = vld1_u8((uint8_t * RADRESTRICT)(v57 + 1));
  int16x8_t v63 = vreinterpretq_s16_u16(vshll_n_u8(v61, 4));
  uint8x8_t v58 = vld1_u8((uint8_t * RADRESTRICT)(v57 + 0));
  int16x8_t v60 = vreinterpretq_s16_u16(vshll_n_u8(v58, 4));
  int16x8_t v64 = vsubq_s16(v60, v63);
  int16x8_t v65 = vsraq_n_s16(v63, v64, 2);
  int16x8_t v100 = vrhaddq_s16(v56, v65);
  uint8x8_t v102 = vqrshrun_n_s16(v100, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v118 + 0), v102);

  {
  char unsigned * RADRESTRICT v120 = v118 + v3;
  char unsigned * RADRESTRICT v66 = v57 + v2;
  uint8x8_t v70 = vld1_u8((uint8_t * RADRESTRICT)(v66 + 1));
  int16x8_t v72 = vreinterpretq_s16_u16(vshll_n_u8(v70, 4));
  uint8x8_t v67 = vld1_u8((uint8_t * RADRESTRICT)(v66 + 0));
  int16x8_t v69 = vreinterpretq_s16_u16(vshll_n_u8(v67, 4));
  int16x8_t v73 = vsubq_s16(v69, v72);
  int16x8_t v74 = vsraq_n_s16(v72, v73, 2);
  int16x8_t v103 = vrhaddq_s16(v65, v74);
  uint8x8_t v105 = vqrshrun_n_s16(v103, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v120 + 0), v105);

  {
  char unsigned * RADRESTRICT v122 = v120 + v3;
  char unsigned * RADRESTRICT v75 = v66 + v2;
  uint8x8_t v79 = vld1_u8((uint8_t * RADRESTRICT)(v75 + 1));
  int16x8_t v81 = vreinterpretq_s16_u16(vshll_n_u8(v79, 4));
  uint8x8_t v76 = vld1_u8((uint8_t * RADRESTRICT)(v75 + 0));
  int16x8_t v78 = vreinterpretq_s16_u16(vshll_n_u8(v76, 4));
  int16x8_t v82 = vsubq_s16(v78, v81);
  int16x8_t v83 = vsraq_n_s16(v81, v82, 2);
  int16x8_t v106 = vrhaddq_s16(v74, v83);
  uint8x8_t v108 = vqrshrun_n_s16(v106, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v122 + 0), v108);
  }
  }
  }
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void bilinear_8x8_h75_v75(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int32_t sw, int32_t dw)
{
  {
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dest);
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(src);
  int32_t v2 = sw;
  char unsigned * RADRESTRICT v12 = v0 + v2;
  uint8x8_t v16 = vld1_u8((uint8_t * RADRESTRICT)(v12 + 1));
  int16x8_t v18 = vreinterpretq_s16_u16(vshll_n_u8(v16, 4));
  uint8x8_t v13 = vld1_u8((uint8_t * RADRESTRICT)(v12 + 0));
  int16x8_t v15 = vreinterpretq_s16_u16(vshll_n_u8(v13, 4));
  int16x8_t v19 = vsubq_s16(v15, v18);
  int16x8_t v20 = vsraq_n_s16(v18, v19, 2);
  uint8x8_t v7 = vld1_u8((uint8_t * RADRESTRICT)(v0 + 1));
  int16x8_t v9 = vreinterpretq_s16_u16(vshll_n_u8(v7, 4));
  uint8x8_t v4 = vld1_u8((uint8_t * RADRESTRICT)(v0 + 0));
  int16x8_t v6 = vreinterpretq_s16_u16(vshll_n_u8(v4, 4));
  int16x8_t v10 = vsubq_s16(v6, v9);
  int16x8_t v11 = vsraq_n_s16(v9, v10, 2);
  int16x8_t v85 = vsubq_s16(v11, v20);
  int16x8_t v86 = vsraq_n_s16(v20, v85, 2);
  uint8x8_t v88 = vqrshrun_n_s16(v86, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v1 + 0), v88);

  {
  int32_t v3 = dw;
  char unsigned * RADRESTRICT v118 = v1 + v3;
  char unsigned * RADRESTRICT v21 = v12 + v2;
  uint8x8_t v25 = vld1_u8((uint8_t * RADRESTRICT)(v21 + 1));
  int16x8_t v27 = vreinterpretq_s16_u16(vshll_n_u8(v25, 4));
  uint8x8_t v22 = vld1_u8((uint8_t * RADRESTRICT)(v21 + 0));
  int16x8_t v24 = vreinterpretq_s16_u16(vshll_n_u8(v22, 4));
  int16x8_t v28 = vsubq_s16(v24, v27);
  int16x8_t v29 = vsraq_n_s16(v27, v28, 2);
  int16x8_t v89 = vsubq_s16(v20, v29);
  int16x8_t v90 = vsraq_n_s16(v29, v89, 2);
  uint8x8_t v92 = vqrshrun_n_s16(v90, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v118 + 0), v92);

  {
  char unsigned * RADRESTRICT v120 = v118 + v3;
  char unsigned * RADRESTRICT v30 = v21 + v2;
  uint8x8_t v34 = vld1_u8((uint8_t * RADRESTRICT)(v30 + 1));
  int16x8_t v36 = vreinterpretq_s16_u16(vshll_n_u8(v34, 4));
  uint8x8_t v31 = vld1_u8((uint8_t * RADRESTRICT)(v30 + 0));
  int16x8_t v33 = vreinterpretq_s16_u16(vshll_n_u8(v31, 4));
  int16x8_t v37 = vsubq_s16(v33, v36);
  int16x8_t v38 = vsraq_n_s16(v36, v37, 2);
  int16x8_t v93 = vsubq_s16(v29, v38);
  int16x8_t v94 = vsraq_n_s16(v38, v93, 2);
  uint8x8_t v96 = vqrshrun_n_s16(v94, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v120 + 0), v96);

  {
  char unsigned * RADRESTRICT v122 = v120 + v3;
  char unsigned * RADRESTRICT v39 = v30 + v2;
  uint8x8_t v43 = vld1_u8((uint8_t * RADRESTRICT)(v39 + 1));
  int16x8_t v45 = vreinterpretq_s16_u16(vshll_n_u8(v43, 4));
  uint8x8_t v40 = vld1_u8((uint8_t * RADRESTRICT)(v39 + 0));
  int16x8_t v42 = vreinterpretq_s16_u16(vshll_n_u8(v40, 4));
  int16x8_t v46 = vsubq_s16(v42, v45);
  int16x8_t v47 = vsraq_n_s16(v45, v46, 2);
  int16x8_t v97 = vsubq_s16(v38, v47);
  int16x8_t v98 = vsraq_n_s16(v47, v97, 2);
  uint8x8_t v100 = vqrshrun_n_s16(v98, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v122 + 0), v100);

  {
  char unsigned * RADRESTRICT v124 = v122 + v3;
  char unsigned * RADRESTRICT v48 = v39 + v2;
  uint8x8_t v52 = vld1_u8((uint8_t * RADRESTRICT)(v48 + 1));
  int16x8_t v54 = vreinterpretq_s16_u16(vshll_n_u8(v52, 4));
  uint8x8_t v49 = vld1_u8((uint8_t * RADRESTRICT)(v48 + 0));
  int16x8_t v51 = vreinterpretq_s16_u16(vshll_n_u8(v49, 4));
  int16x8_t v55 = vsubq_s16(v51, v54);
  int16x8_t v56 = vsraq_n_s16(v54, v55, 2);
  int16x8_t v101 = vsubq_s16(v47, v56);
  int16x8_t v102 = vsraq_n_s16(v56, v101, 2);
  uint8x8_t v104 = vqrshrun_n_s16(v102, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v124 + 0), v104);

  {
  char unsigned * RADRESTRICT v126 = v124 + v3;
  char unsigned * RADRESTRICT v57 = v48 + v2;
  uint8x8_t v61 = vld1_u8((uint8_t * RADRESTRICT)(v57 + 1));
  int16x8_t v63 = vreinterpretq_s16_u16(vshll_n_u8(v61, 4));
  uint8x8_t v58 = vld1_u8((uint8_t * RADRESTRICT)(v57 + 0));
  int16x8_t v60 = vreinterpretq_s16_u16(vshll_n_u8(v58, 4));
  int16x8_t v64 = vsubq_s16(v60, v63);
  int16x8_t v65 = vsraq_n_s16(v63, v64, 2);
  int16x8_t v105 = vsubq_s16(v56, v65);
  int16x8_t v106 = vsraq_n_s16(v65, v105, 2);
  uint8x8_t v108 = vqrshrun_n_s16(v106, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v126 + 0), v108);

  {
  char unsigned * RADRESTRICT v128 = v126 + v3;
  char unsigned * RADRESTRICT v66 = v57 + v2;
  uint8x8_t v70 = vld1_u8((uint8_t * RADRESTRICT)(v66 + 1));
  int16x8_t v72 = vreinterpretq_s16_u16(vshll_n_u8(v70, 4));
  uint8x8_t v67 = vld1_u8((uint8_t * RADRESTRICT)(v66 + 0));
  int16x8_t v69 = vreinterpretq_s16_u16(vshll_n_u8(v67, 4));
  int16x8_t v73 = vsubq_s16(v69, v72);
  int16x8_t v74 = vsraq_n_s16(v72, v73, 2);
  int16x8_t v109 = vsubq_s16(v65, v74);
  int16x8_t v110 = vsraq_n_s16(v74, v109, 2);
  uint8x8_t v112 = vqrshrun_n_s16(v110, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v128 + 0), v112);

  {
  char unsigned * RADRESTRICT v130 = v128 + v3;
  char unsigned * RADRESTRICT v75 = v66 + v2;
  uint8x8_t v79 = vld1_u8((uint8_t * RADRESTRICT)(v75 + 1));
  int16x8_t v81 = vreinterpretq_s16_u16(vshll_n_u8(v79, 4));
  uint8x8_t v76 = vld1_u8((uint8_t * RADRESTRICT)(v75 + 0));
  int16x8_t v78 = vreinterpretq_s16_u16(vshll_n_u8(v76, 4));
  int16x8_t v82 = vsubq_s16(v78, v81);
  int16x8_t v83 = vsraq_n_s16(v81, v82, 2);
  int16x8_t v113 = vsubq_s16(v74, v83);
  int16x8_t v114 = vsraq_n_s16(v83, v113, 2);
  uint8x8_t v116 = vqrshrun_n_s16(v114, 4);
  vst1_u8((uint8_t * RADRESTRICT)(v130 + 0), v116);
  }
  }
  }
  }
  }
  }
  }
  }
}

typedef void bilinear_8x8_jump(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int32_t sw, int32_t dw);
#if !defined(BINK2_CODEGEN_NO_TABLE)
#ifdef _MSC_VER
_declspec(selectany)
#else
static
#endif
bilinear_8x8_jump *bilinear_8x8[] =
{
  bilinear_8x8_h0_v0,
  bilinear_8x8_h0_v25,
  bilinear_8x8_h0_v50,
  bilinear_8x8_h0_v75,
  bilinear_8x8_h25_v0,
  bilinear_8x8_h25_v25,
  bilinear_8x8_h25_v50,
  bilinear_8x8_h25_v75,
  bilinear_8x8_h50_v0,
  bilinear_8x8_h50_v25,
  bilinear_8x8_h50_v50,
  bilinear_8x8_h50_v75,
  bilinear_8x8_h75_v0,
  bilinear_8x8_h75_v25,
  bilinear_8x8_h75_v50,
  bilinear_8x8_h75_v75,
};
#endif

