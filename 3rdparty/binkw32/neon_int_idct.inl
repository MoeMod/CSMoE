// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

static CODEGEN_ATTR void int_idct_default(char unsigned * RADRESTRICT out_ptr, int32_t out_stride, char unsigned * RADRESTRICT in_ptr)
{
  {
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(out_ptr);
  char unsigned * RADRESTRICT v2 = (char unsigned * RADRESTRICT)(in_ptr);
  int16x8_t v3 = vld1q_s16((int16_t * RADRESTRICT)(v2 + 0));
  int16x8_t v7 = vld1q_s16((int16_t * RADRESTRICT)(v2 + 64));
  int16x8_t v37 = vaddq_s16(v3, v7);
  int16x8_t v9 = vld1q_s16((int16_t * RADRESTRICT)(v2 + 96));
  int16x8_t v41 = (vshrq_n_s16((v9), 1));
  int16x8_t v5 = vld1q_s16((int16_t * RADRESTRICT)(v2 + 32));
  int16x8_t v39 = (vshrq_n_s16((v5), 2));
  int16x8_t v40 = vaddq_s16(v5, v39);
  int16x8_t v42 = vaddq_s16(v41, v40);
  int16x8_t v47 = vaddq_s16(v37, v42);
  int16x8_t v6 = vld1q_s16((int16_t * RADRESTRICT)(v2 + 48));
  int16x8_t v8 = vld1q_s16((int16_t * RADRESTRICT)(v2 + 80));
  int16x8_t v12 = vsubq_s16(v6, v8);
  int16x8_t v10 = vld1q_s16((int16_t * RADRESTRICT)(v2 + 112));
  int16x8_t v16 = vaddq_s16(v12, v10);
  int16x8_t v18 = (vshrq_n_s16((v16), 2));
  int16x8_t v4 = vld1q_s16((int16_t * RADRESTRICT)(v2 + 16));
  int16x8_t v11 = vaddq_s16(v6, v8);
  int16x8_t v13 = vaddq_s16(v4, v11);
  int16x8_t v17 = (vshrq_n_s16((v13), 2));
  int16x8_t v20 = vaddq_s16(v13, v17);
  int16x8_t v19 = (vshrq_n_s16((v13), 4));
  int16x8_t v21 = vsubq_s16(v20, v19);
  int16x8_t v22 = vaddq_s16(v18, v21);
  int16x8_t v51 = vaddq_s16(v47, v22);
  int16x8_t v50 = vsubq_s16(v37, v42);
  int16x8_t v24 = vaddq_s16(v16, v18);
  int16x8_t v23 = (vshrq_n_s16((v16), 4));
  int16x8_t v25 = vsubq_s16(v24, v23);
  int16x8_t v26 = vsubq_s16(v17, v25);
  int16x8_t v55 = vsubq_s16(v50, v26);
  int16x8_t v66_0 = vcombine_s16(vget_low_s16(v51), vget_low_s16(v55));
  int16x8_t v38 = vsubq_s16(v3, v7);
  int16x8_t v45 = (vshrq_n_s16((v5), 1));
  int16x8_t v43 = (vshrq_n_s16((v9), 2));
  int16x8_t v44 = vaddq_s16(v9, v43);
  int16x8_t v46 = vsubq_s16(v45, v44);
  int16x8_t v49 = vsubq_s16(v38, v46);
  int16x8_t v14 = vsubq_s16(v4, v11);
  int16x8_t v33 = (vshrq_n_s16((v14), 2));
  int16x8_t v34 = vsubq_s16(v14, v33);
  int16x8_t v32 = (vshrq_n_s16((v14), 4));
  int16x8_t v35 = vsubq_s16(v34, v32);
  int16x8_t v15 = vsubq_s16(v12, v10);
  int16x8_t v36 = vsubq_s16(v35, v15);
  int16x8_t v53 = vaddq_s16(v49, v36);
  int16x8_t v48 = vaddq_s16(v38, v46);
  int16x8_t v28 = (vshrq_n_s16((v15), 2));
  int16x8_t v29 = vsubq_s16(v15, v28);
  int16x8_t v27 = (vshrq_n_s16((v15), 4));
  int16x8_t v30 = vsubq_s16(v29, v27);
  int16x8_t v31 = vaddq_s16(v30, v14);
  int16x8_t v57 = vsubq_s16(v48, v31);
  int16x8_t v66_2 = vcombine_s16(vget_low_s16(v53), vget_low_s16(v57));
  int16x8_t v67_0 = vreinterpretq_s16_s32(vtrnq_s32(vreinterpretq_s32_s16(v66_0), vreinterpretq_s32_s16(v66_2)).val[0]);
  int16x8_t v52 = vaddq_s16(v48, v31);
  int16x8_t v56 = vsubq_s16(v49, v36);
  int16x8_t v66_1 = vcombine_s16(vget_low_s16(v52), vget_low_s16(v56));
  int16x8_t v54 = vaddq_s16(v50, v26);
  int16x8_t v58 = vsubq_s16(v47, v22);
  int16x8_t v66_3 = vcombine_s16(vget_low_s16(v54), vget_low_s16(v58));
  int16x8_t v67_1 = vreinterpretq_s16_s32(vtrnq_s32(vreinterpretq_s32_s16(v66_1), vreinterpretq_s32_s16(v66_3)).val[0]);
  int16x8_t v68_0 = vtrnq_s16(v67_0, v67_1).val[0];
  int16x8_t v66_4 = vcombine_s16(vget_high_s16(v51), vget_high_s16(v55));
  int16x8_t v66_6 = vcombine_s16(vget_high_s16(v53), vget_high_s16(v57));
  int16x8_t v67_4 = vreinterpretq_s16_s32(vtrnq_s32(vreinterpretq_s32_s16(v66_4), vreinterpretq_s32_s16(v66_6)).val[0]);
  int16x8_t v66_5 = vcombine_s16(vget_high_s16(v52), vget_high_s16(v56));
  int16x8_t v66_7 = vcombine_s16(vget_high_s16(v54), vget_high_s16(v58));
  int16x8_t v67_5 = vreinterpretq_s16_s32(vtrnq_s32(vreinterpretq_s32_s16(v66_5), vreinterpretq_s32_s16(v66_7)).val[0]);
  int16x8_t v68_4 = vtrnq_s16(v67_4, v67_5).val[0];
  int16x8_t v103 = vaddq_s16(v68_0, v68_4);
  int16x8_t v67_6 = vreinterpretq_s16_s32(vtrnq_s32(vreinterpretq_s32_s16(v66_4), vreinterpretq_s32_s16(v66_6)).val[1]);
  int16x8_t v67_7 = vreinterpretq_s16_s32(vtrnq_s32(vreinterpretq_s32_s16(v66_5), vreinterpretq_s32_s16(v66_7)).val[1]);
  int16x8_t v68_6 = vtrnq_s16(v67_6, v67_7).val[0];
  int16x8_t v107 = (vshrq_n_s16((v68_6), 1));
  int16x8_t v67_2 = vreinterpretq_s16_s32(vtrnq_s32(vreinterpretq_s32_s16(v66_0), vreinterpretq_s32_s16(v66_2)).val[1]);
  int16x8_t v67_3 = vreinterpretq_s16_s32(vtrnq_s32(vreinterpretq_s32_s16(v66_1), vreinterpretq_s32_s16(v66_3)).val[1]);
  int16x8_t v68_2 = vtrnq_s16(v67_2, v67_3).val[0];
  int16x8_t v105 = (vshrq_n_s16((v68_2), 2));
  int16x8_t v106 = vaddq_s16(v68_2, v105);
  int16x8_t v108 = vaddq_s16(v107, v106);
  int16x8_t v113 = vaddq_s16(v103, v108);
  int16x8_t v68_3 = vtrnq_s16(v67_2, v67_3).val[1];
  int16x8_t v68_5 = vtrnq_s16(v67_4, v67_5).val[1];
  int16x8_t v78 = vsubq_s16(v68_3, v68_5);
  int16x8_t v68_7 = vtrnq_s16(v67_6, v67_7).val[1];
  int16x8_t v82 = vaddq_s16(v78, v68_7);
  int16x8_t v84 = (vshrq_n_s16((v82), 2));
  int16x8_t v68_1 = vtrnq_s16(v67_0, v67_1).val[1];
  int16x8_t v77 = vaddq_s16(v68_3, v68_5);
  int16x8_t v79 = vaddq_s16(v68_1, v77);
  int16x8_t v83 = (vshrq_n_s16((v79), 2));
  int16x8_t v86 = vaddq_s16(v79, v83);
  int16x8_t v85 = (vshrq_n_s16((v79), 4));
  int16x8_t v87 = vsubq_s16(v86, v85);
  int16x8_t v88 = vaddq_s16(v84, v87);
  int16x8_t v117 = vaddq_s16(v113, v88);
  int16x8_t v125 = (vshrq_n_s16((v117), 6));
  vst1q_s16((int16_t * RADRESTRICT)(v0 + 0), v125);

  {
  int32_t v1 = out_stride;
  char unsigned * RADRESTRICT v127 = v0 + v1;
  int16x8_t v104 = vsubq_s16(v68_0, v68_4);
  int16x8_t v111 = (vshrq_n_s16((v68_2), 1));
  int16x8_t v109 = (vshrq_n_s16((v68_6), 2));
  int16x8_t v110 = vaddq_s16(v68_6, v109);
  int16x8_t v112 = vsubq_s16(v111, v110);
  int16x8_t v114 = vaddq_s16(v104, v112);
  int16x8_t v81 = vsubq_s16(v78, v68_7);
  int16x8_t v94 = (vshrq_n_s16((v81), 2));
  int16x8_t v95 = vsubq_s16(v81, v94);
  int16x8_t v93 = (vshrq_n_s16((v81), 4));
  int16x8_t v96 = vsubq_s16(v95, v93);
  int16x8_t v80 = vsubq_s16(v68_1, v77);
  int16x8_t v97 = vaddq_s16(v96, v80);
  int16x8_t v118 = vaddq_s16(v114, v97);
  int16x8_t v128 = (vshrq_n_s16((v118), 6));
  vst1q_s16((int16_t * RADRESTRICT)(v127 + 0), v128);

  {
  char unsigned * RADRESTRICT v130 = v127 + v1;
  int16x8_t v115 = vsubq_s16(v104, v112);
  int16x8_t v99 = (vshrq_n_s16((v80), 2));
  int16x8_t v100 = vsubq_s16(v80, v99);
  int16x8_t v98 = (vshrq_n_s16((v80), 4));
  int16x8_t v101 = vsubq_s16(v100, v98);
  int16x8_t v102 = vsubq_s16(v101, v81);
  int16x8_t v119 = vaddq_s16(v115, v102);
  int16x8_t v131 = (vshrq_n_s16((v119), 6));
  vst1q_s16((int16_t * RADRESTRICT)(v130 + 0), v131);

  {
  char unsigned * RADRESTRICT v133 = v130 + v1;
  int16x8_t v116 = vsubq_s16(v103, v108);
  int16x8_t v90 = vaddq_s16(v82, v84);
  int16x8_t v89 = (vshrq_n_s16((v82), 4));
  int16x8_t v91 = vsubq_s16(v90, v89);
  int16x8_t v92 = vsubq_s16(v83, v91);
  int16x8_t v120 = vaddq_s16(v116, v92);
  int16x8_t v134 = (vshrq_n_s16((v120), 6));
  vst1q_s16((int16_t * RADRESTRICT)(v133 + 0), v134);

  {
  char unsigned * RADRESTRICT v136 = v133 + v1;
  int16x8_t v121 = vsubq_s16(v116, v92);
  int16x8_t v137 = (vshrq_n_s16((v121), 6));
  vst1q_s16((int16_t * RADRESTRICT)(v136 + 0), v137);

  {
  char unsigned * RADRESTRICT v139 = v136 + v1;
  int16x8_t v122 = vsubq_s16(v115, v102);
  int16x8_t v140 = (vshrq_n_s16((v122), 6));
  vst1q_s16((int16_t * RADRESTRICT)(v139 + 0), v140);

  {
  char unsigned * RADRESTRICT v142 = v139 + v1;
  int16x8_t v123 = vsubq_s16(v114, v97);
  int16x8_t v143 = (vshrq_n_s16((v123), 6));
  vst1q_s16((int16_t * RADRESTRICT)(v142 + 0), v143);

  {
  char unsigned * RADRESTRICT v145 = v142 + v1;
  int16x8_t v124 = vsubq_s16(v113, v88);
  int16x8_t v146 = (vshrq_n_s16((v124), 6));
  vst1q_s16((int16_t * RADRESTRICT)(v145 + 0), v146);
  }
  }
  }
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void int_idct_sparse(char unsigned * RADRESTRICT out_ptr, int32_t out_stride, char unsigned * RADRESTRICT in_ptr)
{
  {
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(out_ptr);
  char unsigned * RADRESTRICT v2 = (char unsigned * RADRESTRICT)(in_ptr);
  int16x8_t v3 = vld1q_s16((int16_t * RADRESTRICT)(v2 + 0));
  int16x8_t v5 = vld1q_s16((int16_t * RADRESTRICT)(v2 + 32));
  int16x8_t v26 = (vshrq_n_s16((v5), 2));
  int16x8_t v27 = vaddq_s16(v5, v26);
  int16x8_t v29 = vaddq_s16(v3, v27);
  int16x8_t v6 = vld1q_s16((int16_t * RADRESTRICT)(v2 + 48));
  int16x8_t v10 = (vshrq_n_s16((v6), 2));
  int16x8_t v4 = vld1q_s16((int16_t * RADRESTRICT)(v2 + 16));
  int16x8_t v7 = vaddq_s16(v4, v6);
  int16x8_t v9 = (vshrq_n_s16((v7), 2));
  int16x8_t v14 = vaddq_s16(v7, v9);
  int16x8_t v13 = (vshrq_n_s16((v7), 4));
  int16x8_t v15 = vsubq_s16(v14, v13);
  int16x8_t v16 = vaddq_s16(v10, v15);
  int16x8_t v33 = vaddq_s16(v29, v16);
  int16x8_t v32 = vsubq_s16(v3, v27);
  int16x8_t v11 = (vshrq_n_s16((v6), 4));
  int16x8_t v12 = vsubq_s16(v6, v11);
  int16x8_t v17 = vaddq_s16(v12, v10);
  int16x8_t v18 = vsubq_s16(v9, v17);
  int16x8_t v37 = vsubq_s16(v32, v18);
  int16x8_t v48_0 = vcombine_s16(vget_low_s16(v33), vget_low_s16(v37));
  int16x8_t v28 = (vshrq_n_s16((v5), 1));
  int16x8_t v31 = vsubq_s16(v3, v28);
  int16x8_t v8 = vsubq_s16(v4, v6);
  int16x8_t v22 = (vshrq_n_s16((v8), 2));
  int16x8_t v23 = vsubq_s16(v8, v22);
  int16x8_t v21 = (vshrq_n_s16((v8), 4));
  int16x8_t v24 = vsubq_s16(v23, v21);
  int16x8_t v25 = vsubq_s16(v24, v6);
  int16x8_t v35 = vaddq_s16(v31, v25);
  int16x8_t v30 = vaddq_s16(v3, v28);
  int16x8_t v19 = vsubq_s16(v12, v10);
  int16x8_t v20 = vaddq_s16(v19, v8);
  int16x8_t v39 = vsubq_s16(v30, v20);
  int16x8_t v48_2 = vcombine_s16(vget_low_s16(v35), vget_low_s16(v39));
  int16x8_t v49_0 = vreinterpretq_s16_s32(vtrnq_s32(vreinterpretq_s32_s16(v48_0), vreinterpretq_s32_s16(v48_2)).val[0]);
  int16x8_t v34 = vaddq_s16(v30, v20);
  int16x8_t v38 = vsubq_s16(v31, v25);
  int16x8_t v48_1 = vcombine_s16(vget_low_s16(v34), vget_low_s16(v38));
  int16x8_t v36 = vaddq_s16(v32, v18);
  int16x8_t v40 = vsubq_s16(v29, v16);
  int16x8_t v48_3 = vcombine_s16(vget_low_s16(v36), vget_low_s16(v40));
  int16x8_t v49_1 = vreinterpretq_s16_s32(vtrnq_s32(vreinterpretq_s32_s16(v48_1), vreinterpretq_s32_s16(v48_3)).val[0]);
  int16x8_t v50_0 = vtrnq_s16(v49_0, v49_1).val[0];
  int16x8_t v49_2 = vreinterpretq_s16_s32(vtrnq_s32(vreinterpretq_s32_s16(v48_0), vreinterpretq_s32_s16(v48_2)).val[1]);
  int16x8_t v49_3 = vreinterpretq_s16_s32(vtrnq_s32(vreinterpretq_s32_s16(v48_1), vreinterpretq_s32_s16(v48_3)).val[1]);
  int16x8_t v50_2 = vtrnq_s16(v49_2, v49_3).val[0];
  int16x8_t v78 = (vshrq_n_s16((v50_2), 2));
  int16x8_t v79 = vaddq_s16(v50_2, v78);
  int16x8_t v81 = vaddq_s16(v50_0, v79);
  int16x8_t v50_3 = vtrnq_s16(v49_2, v49_3).val[1];
  int16x8_t v62 = (vshrq_n_s16((v50_3), 2));
  int16x8_t v50_1 = vtrnq_s16(v49_0, v49_1).val[1];
  int16x8_t v59 = vaddq_s16(v50_1, v50_3);
  int16x8_t v61 = (vshrq_n_s16((v59), 2));
  int16x8_t v66 = vaddq_s16(v59, v61);
  int16x8_t v65 = (vshrq_n_s16((v59), 4));
  int16x8_t v67 = vsubq_s16(v66, v65);
  int16x8_t v68 = vaddq_s16(v62, v67);
  int16x8_t v85 = vaddq_s16(v81, v68);
  int16x8_t v93 = (vshrq_n_s16((v85), 6));
  vst1q_s16((int16_t * RADRESTRICT)(v0 + 0), v93);

  {
  int32_t v1 = out_stride;
  char unsigned * RADRESTRICT v95 = v0 + v1;
  int16x8_t v80 = (vshrq_n_s16((v50_2), 1));
  int16x8_t v82 = vaddq_s16(v50_0, v80);
  int16x8_t v63 = (vshrq_n_s16((v50_3), 4));
  int16x8_t v64 = vsubq_s16(v50_3, v63);
  int16x8_t v71 = vsubq_s16(v64, v62);
  int16x8_t v60 = vsubq_s16(v50_1, v50_3);
  int16x8_t v72 = vaddq_s16(v71, v60);
  int16x8_t v86 = vaddq_s16(v82, v72);
  int16x8_t v96 = (vshrq_n_s16((v86), 6));
  vst1q_s16((int16_t * RADRESTRICT)(v95 + 0), v96);

  {
  char unsigned * RADRESTRICT v98 = v95 + v1;
  int16x8_t v83 = vsubq_s16(v50_0, v80);
  int16x8_t v74 = (vshrq_n_s16((v60), 2));
  int16x8_t v75 = vsubq_s16(v60, v74);
  int16x8_t v73 = (vshrq_n_s16((v60), 4));
  int16x8_t v76 = vsubq_s16(v75, v73);
  int16x8_t v77 = vsubq_s16(v76, v50_3);
  int16x8_t v87 = vaddq_s16(v83, v77);
  int16x8_t v99 = (vshrq_n_s16((v87), 6));
  vst1q_s16((int16_t * RADRESTRICT)(v98 + 0), v99);

  {
  char unsigned * RADRESTRICT v101 = v98 + v1;
  int16x8_t v84 = vsubq_s16(v50_0, v79);
  int16x8_t v69 = vaddq_s16(v64, v62);
  int16x8_t v70 = vsubq_s16(v61, v69);
  int16x8_t v88 = vaddq_s16(v84, v70);
  int16x8_t v102 = (vshrq_n_s16((v88), 6));
  vst1q_s16((int16_t * RADRESTRICT)(v101 + 0), v102);

  {
  char unsigned * RADRESTRICT v104 = v101 + v1;
  int16x8_t v89 = vsubq_s16(v84, v70);
  int16x8_t v105 = (vshrq_n_s16((v89), 6));
  vst1q_s16((int16_t * RADRESTRICT)(v104 + 0), v105);

  {
  char unsigned * RADRESTRICT v107 = v104 + v1;
  int16x8_t v90 = vsubq_s16(v83, v77);
  int16x8_t v108 = (vshrq_n_s16((v90), 6));
  vst1q_s16((int16_t * RADRESTRICT)(v107 + 0), v108);

  {
  char unsigned * RADRESTRICT v110 = v107 + v1;
  int16x8_t v91 = vsubq_s16(v82, v72);
  int16x8_t v111 = (vshrq_n_s16((v91), 6));
  vst1q_s16((int16_t * RADRESTRICT)(v110 + 0), v111);

  {
  char unsigned * RADRESTRICT v113 = v110 + v1;
  int16x8_t v92 = vsubq_s16(v81, v68);
  int16x8_t v114 = (vshrq_n_s16((v92), 6));
  vst1q_s16((int16_t * RADRESTRICT)(v113 + 0), v114);
  }
  }
  }
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void int_idct_fill(char unsigned * RADRESTRICT out_ptr, int32_t out_stride, char unsigned * RADRESTRICT in_ptr)
{
  {
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(out_ptr);
  char unsigned * RADRESTRICT v2 = (char unsigned * RADRESTRICT)(in_ptr);
  int16x8_t v3 = vld1q_s16((int16_t * RADRESTRICT)(v2 + 0));
  int16x8_t v4 = vdupq_lane_s16(vget_low_s16(v3), 0);
  int16x8_t v5 = (vshrq_n_s16((v4), 6));
  vst1q_s16((int16_t * RADRESTRICT)(v0 + 0), v5);

  {
  int32_t v1 = out_stride;
  char unsigned * RADRESTRICT v7 = v0 + v1;
  vst1q_s16((int16_t * RADRESTRICT)(v7 + 0), v5);

  {
  char unsigned * RADRESTRICT v9 = v7 + v1;
  vst1q_s16((int16_t * RADRESTRICT)(v9 + 0), v5);

  {
  char unsigned * RADRESTRICT v11 = v9 + v1;
  vst1q_s16((int16_t * RADRESTRICT)(v11 + 0), v5);

  {
  char unsigned * RADRESTRICT v13 = v11 + v1;
  vst1q_s16((int16_t * RADRESTRICT)(v13 + 0), v5);

  {
  char unsigned * RADRESTRICT v15 = v13 + v1;
  vst1q_s16((int16_t * RADRESTRICT)(v15 + 0), v5);

  {
  char unsigned * RADRESTRICT v17 = v15 + v1;
  vst1q_s16((int16_t * RADRESTRICT)(v17 + 0), v5);

  {
  char unsigned * RADRESTRICT v19 = v17 + v1;
  vst1q_s16((int16_t * RADRESTRICT)(v19 + 0), v5);
  }
  }
  }
  }
  }
  }
  }
  }
}

