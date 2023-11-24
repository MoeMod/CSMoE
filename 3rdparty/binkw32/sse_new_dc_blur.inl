// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

static CODEGEN_ATTR void new_dc_blur_r0_l0(char unsigned * RADRESTRICT dcs0, char unsigned * RADRESTRICT dcs1, char unsigned * RADRESTRICT dcs2, char unsigned * RADRESTRICT t, char unsigned * RADRESTRICT dest, int dw)
{
}

static CODEGEN_ATTR void new_dc_blur_r0_l1(char unsigned * RADRESTRICT dcs0, char unsigned * RADRESTRICT dcs1, char unsigned * RADRESTRICT dcs2, char unsigned * RADRESTRICT t, char unsigned * RADRESTRICT dest, int dw)
{
  {
  char unsigned * RADRESTRICT v16 = (char unsigned * RADRESTRICT)(dest);
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(dcs0);
  __m128i v4 = _mm_loadu_si128((__m128i *)(v0 + 0));
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dcs1);
  __m128i v5 = _mm_loadu_si128((__m128i *)(v1 + 0));
  char unsigned * RADRESTRICT v2 = (char unsigned * RADRESTRICT)(dcs2);
  __m128i v6 = _mm_loadu_si128((__m128i *)(v2 + 0));
  char unsigned * RADRESTRICT v3 = (char unsigned * RADRESTRICT)(t);
  __m128i v7 = _mm_load_si128((__m128i *)(v3 + 0));
  __m128i X = v4;
  __m128i Y = v5;
  __m128i Z = v6;
  __m128i T = v7;
  __m128i sY = _mm_srli_si128(Y, 2);
  __m128i C = _mm_shuffle_epi32(sY, _MM_SHUFFLE(0, 0, 0, 0));
  __m128i mXZ = _mm_unpacklo_epi32(_mm_srli_si128(X, 2), _mm_srli_si128(Z, 2));
  __m128i P = _mm_unpacklo_epi64(Y, mXZ);
  __m128i S = _mm_unpacklo_epi64(Z, X);
  __m128i Mcp = _mm_srli_epi16(_mm_cmpgt_epi16(T, _mm_max_epi16(_mm_sub_epi16(C,P),_mm_sub_epi16(P,C))),3);
  __m128i Mcs = _mm_srli_epi16(_mm_cmpgt_epi16(T, _mm_max_epi16(_mm_sub_epi16(C,S),_mm_sub_epi16(S,C))),3);
  __m128i v8_0 = _mm_srli_epi16(_mm_slli_epi16(C, 3), 3);
  __m128i v8_1 = _mm_or_si128(_mm_andnot_si128(Mcp, v8_0), _mm_and_si128(Mcp, P));
  __m128i R = _mm_shuffle_epi32(v8_1, _MM_SHUFFLE(1, 0, 1, 0));
  __m128i v8_2 = _mm_or_si128(_mm_andnot_si128(Mcs, R), _mm_and_si128(Mcs, S));
  __m128 Cf = _mm_castsi128_ps(v8_0);
  __m128 Pf = _mm_castsi128_ps(v8_1);
  __m128 Sf = _mm_castsi128_ps(v8_2);
  __m128i v12_0 = _mm_castps_si128(_mm_shuffle_ps(Sf, Pf, _MM_SHUFFLE(0, 0, 0, 2)));
  __m128i v12_1 = _mm_castps_si128(_mm_shuffle_ps(Sf, Pf, _MM_SHUFFLE(1, 1, 1, 3)));
  __m128i v12_2 = _mm_castps_si128(_mm_shuffle_ps(Pf, Cf, _MM_SHUFFLE(0, 0, 3, 2)));
  __m128i v18 = _mm_sub_epi16(v12_0, v12_2);
  __m128i v20 = _mm_add_epi16(v12_2, _mm_set1_epi16(4));
  __m128i v21 = _mm_add_epi16(v20, _mm_mulhi_epi16(v18, _mm_set1_epi16(26624)));
  __m128i v19 = _mm_sub_epi16(v12_1, v12_2);
  __m128i v32 = _mm_add_epi16(v20, _mm_mulhi_epi16(v19, _mm_set1_epi16(7168)));
  __m128i v31 = _mm_srai_epi16(v18, 4);
  __m128i v33 = _mm_add_epi16(v32, v31);
  __m128i v48_0 = _mm_unpacklo_epi16(v21, v33);
  __m128i v26 = _mm_add_epi16(v20, _mm_mulhi_epi16(v18, _mm_set1_epi16(12288)));
  __m128i v25 = _mm_srai_epi16(v19, 5);
  __m128i v27 = _mm_add_epi16(v26, v25);
  __m128i v38 = _mm_add_epi16(v20, _mm_mulhi_epi16(v19, _mm_set1_epi16(18432)));
  __m128i v37 = _mm_srai_epi16(v18, 7);
  __m128i v39 = _mm_add_epi16(v38, v37);
  __m128i v48_4 = _mm_unpacklo_epi16(v27, v39);
  __m128i v49_0 = _mm_unpacklo_epi16(v48_0, v48_4);
  __m128i v23 = _mm_add_epi16(v20, _mm_mulhi_epi16(v18, _mm_set1_epi16(18432)));
  __m128i v22 = _mm_srai_epi16(v19, 7);
  __m128i v24 = _mm_add_epi16(v23, v22);
  __m128i v35 = _mm_add_epi16(v20, _mm_mulhi_epi16(v19, _mm_set1_epi16(12288)));
  __m128i v34 = _mm_srai_epi16(v18, 5);
  __m128i v36 = _mm_add_epi16(v35, v34);
  __m128i v48_2 = _mm_unpacklo_epi16(v24, v36);
  __m128i v29 = _mm_add_epi16(v20, _mm_mulhi_epi16(v18, _mm_set1_epi16(7168)));
  __m128i v28 = _mm_srai_epi16(v19, 4);
  __m128i v30 = _mm_add_epi16(v29, v28);
  __m128i v40 = _mm_add_epi16(v20, _mm_mulhi_epi16(v19, _mm_set1_epi16(26624)));
  __m128i v48_6 = _mm_unpacklo_epi16(v30, v40);
  __m128i v49_4 = _mm_unpacklo_epi16(v48_2, v48_6);
  __m128i v50_0 = _mm_unpacklo_epi16(v49_0, v49_4);
  __m128i v48_1 = _mm_unpackhi_epi16(v21, v33);
  __m128i v48_5 = _mm_unpackhi_epi16(v27, v39);
  __m128i v49_2 = _mm_unpacklo_epi16(v48_1, v48_5);
  __m128i v48_3 = _mm_unpackhi_epi16(v24, v36);
  __m128i v48_7 = _mm_unpackhi_epi16(v30, v40);
  __m128i v49_6 = _mm_unpacklo_epi16(v48_3, v48_7);
  __m128i v50_4 = _mm_unpacklo_epi16(v49_2, v49_6);
  __m128i v54 = _mm_sub_epi16(v50_0, v50_4);
  __m128i v56 = _mm_add_epi16(v50_4, _mm_mulhi_epi16(v54, _mm_set1_epi16(26624)));
  __m128i v101 = _mm_srai_epi16(v56, 3);
  __m128i v103 = _mm_packus_epi16(v101, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v16 + 0), v103);

  {
  int v17 = dw;
  char unsigned * RADRESTRICT v105 = v16 + v17;
  __m128i v58 = _mm_add_epi16(v50_4, _mm_mulhi_epi16(v54, _mm_set1_epi16(18432)));
  __m128i v49_1 = _mm_unpackhi_epi16(v48_0, v48_4);
  __m128i v49_5 = _mm_unpackhi_epi16(v48_2, v48_6);
  __m128i v50_2 = _mm_unpacklo_epi16(v49_1, v49_5);
  __m128i v55 = _mm_sub_epi16(v50_2, v50_4);
  __m128i v57 = _mm_srai_epi16(v55, 7);
  __m128i v59 = _mm_add_epi16(v58, v57);
  __m128i v106 = _mm_srai_epi16(v59, 3);
  __m128i v108 = _mm_packus_epi16(v106, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v105 + 0), v108);

  {
  char unsigned * RADRESTRICT v110 = v105 + v17;
  __m128i v61 = _mm_add_epi16(v50_4, _mm_mulhi_epi16(v54, _mm_set1_epi16(12288)));
  __m128i v60 = _mm_srai_epi16(v55, 5);
  __m128i v62 = _mm_add_epi16(v61, v60);
  __m128i v111 = _mm_srai_epi16(v62, 3);
  __m128i v113 = _mm_packus_epi16(v111, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v110 + 0), v113);

  {
  char unsigned * RADRESTRICT v115 = v110 + v17;
  __m128i v64 = _mm_add_epi16(v50_4, _mm_mulhi_epi16(v54, _mm_set1_epi16(7168)));
  __m128i v63 = _mm_srai_epi16(v55, 4);
  __m128i v65 = _mm_add_epi16(v64, v63);
  __m128i v116 = _mm_srai_epi16(v65, 3);
  __m128i v118 = _mm_packus_epi16(v116, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v115 + 0), v118);

  {
  char unsigned * RADRESTRICT v120 = v115 + v17;
  __m128i v67 = _mm_add_epi16(v50_4, _mm_mulhi_epi16(v55, _mm_set1_epi16(7168)));
  __m128i v66 = _mm_srai_epi16(v54, 4);
  __m128i v68 = _mm_add_epi16(v67, v66);
  __m128i v121 = _mm_srai_epi16(v68, 3);
  __m128i v123 = _mm_packus_epi16(v121, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v120 + 0), v123);

  {
  char unsigned * RADRESTRICT v125 = v120 + v17;
  __m128i v70 = _mm_add_epi16(v50_4, _mm_mulhi_epi16(v55, _mm_set1_epi16(12288)));
  __m128i v69 = _mm_srai_epi16(v54, 5);
  __m128i v71 = _mm_add_epi16(v70, v69);
  __m128i v126 = _mm_srai_epi16(v71, 3);
  __m128i v128 = _mm_packus_epi16(v126, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v125 + 0), v128);

  {
  char unsigned * RADRESTRICT v130 = v125 + v17;
  __m128i v73 = _mm_add_epi16(v50_4, _mm_mulhi_epi16(v55, _mm_set1_epi16(18432)));
  __m128i v72 = _mm_srai_epi16(v54, 7);
  __m128i v74 = _mm_add_epi16(v73, v72);
  __m128i v131 = _mm_srai_epi16(v74, 3);
  __m128i v133 = _mm_packus_epi16(v131, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v130 + 0), v133);

  {
  char unsigned * RADRESTRICT v135 = v130 + v17;
  __m128i v75 = _mm_add_epi16(v50_4, _mm_mulhi_epi16(v55, _mm_set1_epi16(26624)));
  __m128i v136 = _mm_srai_epi16(v75, 3);
  __m128i v138 = _mm_packus_epi16(v136, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v135 + 0), v138);
  }
  }
  }
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void new_dc_blur_r1_l0(char unsigned * RADRESTRICT dcs0, char unsigned * RADRESTRICT dcs1, char unsigned * RADRESTRICT dcs2, char unsigned * RADRESTRICT t, char unsigned * RADRESTRICT dest, int dw)
{
  {
  char unsigned * RADRESTRICT v16 = (char unsigned * RADRESTRICT)(dest);
  char unsigned * RADRESTRICT v18 = v16 + 8;
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(dcs0);
  __m128i v4 = _mm_loadu_si128((__m128i *)(v0 + 0));
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dcs1);
  __m128i v5 = _mm_loadu_si128((__m128i *)(v1 + 0));
  char unsigned * RADRESTRICT v2 = (char unsigned * RADRESTRICT)(dcs2);
  __m128i v6 = _mm_loadu_si128((__m128i *)(v2 + 0));
  char unsigned * RADRESTRICT v3 = (char unsigned * RADRESTRICT)(t);
  __m128i v7 = _mm_load_si128((__m128i *)(v3 + 0));
  __m128i X = v4;
  __m128i Y = v5;
  __m128i Z = v6;
  __m128i T = v7;
  __m128i sY = _mm_srli_si128(Y, 2);
  __m128i C = _mm_shuffle_epi32(sY, _MM_SHUFFLE(0, 0, 0, 0));
  __m128i mXZ = _mm_unpacklo_epi32(_mm_srli_si128(X, 2), _mm_srli_si128(Z, 2));
  __m128i P = _mm_unpacklo_epi64(Y, mXZ);
  __m128i S = _mm_unpacklo_epi64(Z, X);
  __m128i Mcp = _mm_srli_epi16(_mm_cmpgt_epi16(T, _mm_max_epi16(_mm_sub_epi16(C,P),_mm_sub_epi16(P,C))),3);
  __m128i Mcs = _mm_srli_epi16(_mm_cmpgt_epi16(T, _mm_max_epi16(_mm_sub_epi16(C,S),_mm_sub_epi16(S,C))),3);
  __m128i v8_0 = _mm_srli_epi16(_mm_slli_epi16(C, 3), 3);
  __m128i v8_1 = _mm_or_si128(_mm_andnot_si128(Mcp, v8_0), _mm_and_si128(Mcp, P));
  __m128i R = _mm_shuffle_epi32(v8_1, _MM_SHUFFLE(1, 0, 1, 0));
  __m128i v8_2 = _mm_or_si128(_mm_andnot_si128(Mcs, R), _mm_and_si128(Mcs, S));
  __m128 Cf = _mm_castsi128_ps(v8_0);
  __m128 Pf = _mm_castsi128_ps(v8_1);
  __m128 Sf = _mm_castsi128_ps(v8_2);
  __m128i v12_0 = _mm_castps_si128(_mm_shuffle_ps(Sf, Pf, _MM_SHUFFLE(0, 0, 0, 2)));
  __m128i v12_1 = _mm_castps_si128(_mm_shuffle_ps(Sf, Pf, _MM_SHUFFLE(1, 1, 1, 3)));
  __m128i v12_2 = _mm_castps_si128(_mm_shuffle_ps(Pf, Cf, _MM_SHUFFLE(0, 0, 3, 2)));
  __m128i v19 = _mm_sub_epi16(v12_0, v12_2);
  __m128i v21 = _mm_add_epi16(v12_2, _mm_set1_epi16(4));
  __m128i v22 = _mm_add_epi16(v21, _mm_mulhi_epi16(v19, _mm_set1_epi16(26624)));
  __m128i v20 = _mm_sub_epi16(v12_1, v12_2);
  __m128i v33 = _mm_add_epi16(v21, _mm_mulhi_epi16(v20, _mm_set1_epi16(7168)));
  __m128i v32 = _mm_srai_epi16(v19, 4);
  __m128i v34 = _mm_add_epi16(v33, v32);
  __m128i v49_0 = _mm_unpacklo_epi16(v22, v34);
  __m128i v27 = _mm_add_epi16(v21, _mm_mulhi_epi16(v19, _mm_set1_epi16(12288)));
  __m128i v26 = _mm_srai_epi16(v20, 5);
  __m128i v28 = _mm_add_epi16(v27, v26);
  __m128i v39 = _mm_add_epi16(v21, _mm_mulhi_epi16(v20, _mm_set1_epi16(18432)));
  __m128i v38 = _mm_srai_epi16(v19, 7);
  __m128i v40 = _mm_add_epi16(v39, v38);
  __m128i v49_4 = _mm_unpacklo_epi16(v28, v40);
  __m128i v50_0 = _mm_unpacklo_epi16(v49_0, v49_4);
  __m128i v24 = _mm_add_epi16(v21, _mm_mulhi_epi16(v19, _mm_set1_epi16(18432)));
  __m128i v23 = _mm_srai_epi16(v20, 7);
  __m128i v25 = _mm_add_epi16(v24, v23);
  __m128i v36 = _mm_add_epi16(v21, _mm_mulhi_epi16(v20, _mm_set1_epi16(12288)));
  __m128i v35 = _mm_srai_epi16(v19, 5);
  __m128i v37 = _mm_add_epi16(v36, v35);
  __m128i v49_2 = _mm_unpacklo_epi16(v25, v37);
  __m128i v30 = _mm_add_epi16(v21, _mm_mulhi_epi16(v19, _mm_set1_epi16(7168)));
  __m128i v29 = _mm_srai_epi16(v20, 4);
  __m128i v31 = _mm_add_epi16(v30, v29);
  __m128i v41 = _mm_add_epi16(v21, _mm_mulhi_epi16(v20, _mm_set1_epi16(26624)));
  __m128i v49_6 = _mm_unpacklo_epi16(v31, v41);
  __m128i v50_4 = _mm_unpacklo_epi16(v49_2, v49_6);
  __m128i v51_1 = _mm_unpackhi_epi16(v50_0, v50_4);
  __m128i v49_1 = _mm_unpackhi_epi16(v22, v34);
  __m128i v49_5 = _mm_unpackhi_epi16(v28, v40);
  __m128i v50_2 = _mm_unpacklo_epi16(v49_1, v49_5);
  __m128i v49_3 = _mm_unpackhi_epi16(v25, v37);
  __m128i v49_7 = _mm_unpackhi_epi16(v31, v41);
  __m128i v50_6 = _mm_unpacklo_epi16(v49_3, v49_7);
  __m128i v51_5 = _mm_unpackhi_epi16(v50_2, v50_6);
  __m128i v80 = _mm_sub_epi16(v51_1, v51_5);
  __m128i v82 = _mm_add_epi16(v51_5, _mm_mulhi_epi16(v80, _mm_set1_epi16(26624)));
  __m128i v103 = _mm_srai_epi16(v82, 3);
  __m128i v104 = _mm_packus_epi16(v103, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v18 + 0), v104);

  {
  int v17 = dw;
  char unsigned * RADRESTRICT v106 = v18 + v17;
  __m128i v84 = _mm_add_epi16(v51_5, _mm_mulhi_epi16(v80, _mm_set1_epi16(18432)));
  __m128i v50_1 = _mm_unpackhi_epi16(v49_0, v49_4);
  __m128i v50_5 = _mm_unpackhi_epi16(v49_2, v49_6);
  __m128i v51_3 = _mm_unpackhi_epi16(v50_1, v50_5);
  __m128i v81 = _mm_sub_epi16(v51_3, v51_5);
  __m128i v83 = _mm_srai_epi16(v81, 7);
  __m128i v85 = _mm_add_epi16(v84, v83);
  __m128i v108 = _mm_srai_epi16(v85, 3);
  __m128i v109 = _mm_packus_epi16(v108, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v106 + 0), v109);

  {
  char unsigned * RADRESTRICT v111 = v106 + v17;
  __m128i v87 = _mm_add_epi16(v51_5, _mm_mulhi_epi16(v80, _mm_set1_epi16(12288)));
  __m128i v86 = _mm_srai_epi16(v81, 5);
  __m128i v88 = _mm_add_epi16(v87, v86);
  __m128i v113 = _mm_srai_epi16(v88, 3);
  __m128i v114 = _mm_packus_epi16(v113, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v111 + 0), v114);

  {
  char unsigned * RADRESTRICT v116 = v111 + v17;
  __m128i v90 = _mm_add_epi16(v51_5, _mm_mulhi_epi16(v80, _mm_set1_epi16(7168)));
  __m128i v89 = _mm_srai_epi16(v81, 4);
  __m128i v91 = _mm_add_epi16(v90, v89);
  __m128i v118 = _mm_srai_epi16(v91, 3);
  __m128i v119 = _mm_packus_epi16(v118, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v116 + 0), v119);

  {
  char unsigned * RADRESTRICT v121 = v116 + v17;
  __m128i v93 = _mm_add_epi16(v51_5, _mm_mulhi_epi16(v81, _mm_set1_epi16(7168)));
  __m128i v92 = _mm_srai_epi16(v80, 4);
  __m128i v94 = _mm_add_epi16(v93, v92);
  __m128i v123 = _mm_srai_epi16(v94, 3);
  __m128i v124 = _mm_packus_epi16(v123, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v121 + 0), v124);

  {
  char unsigned * RADRESTRICT v126 = v121 + v17;
  __m128i v96 = _mm_add_epi16(v51_5, _mm_mulhi_epi16(v81, _mm_set1_epi16(12288)));
  __m128i v95 = _mm_srai_epi16(v80, 5);
  __m128i v97 = _mm_add_epi16(v96, v95);
  __m128i v128 = _mm_srai_epi16(v97, 3);
  __m128i v129 = _mm_packus_epi16(v128, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v126 + 0), v129);

  {
  char unsigned * RADRESTRICT v131 = v126 + v17;
  __m128i v99 = _mm_add_epi16(v51_5, _mm_mulhi_epi16(v81, _mm_set1_epi16(18432)));
  __m128i v98 = _mm_srai_epi16(v80, 7);
  __m128i v100 = _mm_add_epi16(v99, v98);
  __m128i v133 = _mm_srai_epi16(v100, 3);
  __m128i v134 = _mm_packus_epi16(v133, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v131 + 0), v134);

  {
  char unsigned * RADRESTRICT v136 = v131 + v17;
  __m128i v101 = _mm_add_epi16(v51_5, _mm_mulhi_epi16(v81, _mm_set1_epi16(26624)));
  __m128i v138 = _mm_srai_epi16(v101, 3);
  __m128i v139 = _mm_packus_epi16(v138, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v136 + 0), v139);
  }
  }
  }
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void new_dc_blur_r1_l1(char unsigned * RADRESTRICT dcs0, char unsigned * RADRESTRICT dcs1, char unsigned * RADRESTRICT dcs2, char unsigned * RADRESTRICT t, char unsigned * RADRESTRICT dest, int dw)
{
  {
  char unsigned * RADRESTRICT v16 = (char unsigned * RADRESTRICT)(dest);
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(dcs0);
  __m128i v4 = _mm_loadu_si128((__m128i *)(v0 + 0));
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dcs1);
  __m128i v5 = _mm_loadu_si128((__m128i *)(v1 + 0));
  char unsigned * RADRESTRICT v2 = (char unsigned * RADRESTRICT)(dcs2);
  __m128i v6 = _mm_loadu_si128((__m128i *)(v2 + 0));
  char unsigned * RADRESTRICT v3 = (char unsigned * RADRESTRICT)(t);
  __m128i v7 = _mm_load_si128((__m128i *)(v3 + 0));
  __m128i X = v4;
  __m128i Y = v5;
  __m128i Z = v6;
  __m128i T = v7;
  __m128i sY = _mm_srli_si128(Y, 2);
  __m128i C = _mm_shuffle_epi32(sY, _MM_SHUFFLE(0, 0, 0, 0));
  __m128i mXZ = _mm_unpacklo_epi32(_mm_srli_si128(X, 2), _mm_srli_si128(Z, 2));
  __m128i P = _mm_unpacklo_epi64(Y, mXZ);
  __m128i S = _mm_unpacklo_epi64(Z, X);
  __m128i Mcp = _mm_srli_epi16(_mm_cmpgt_epi16(T, _mm_max_epi16(_mm_sub_epi16(C,P),_mm_sub_epi16(P,C))),3);
  __m128i Mcs = _mm_srli_epi16(_mm_cmpgt_epi16(T, _mm_max_epi16(_mm_sub_epi16(C,S),_mm_sub_epi16(S,C))),3);
  __m128i v8_0 = _mm_srli_epi16(_mm_slli_epi16(C, 3), 3);
  __m128i v8_1 = _mm_or_si128(_mm_andnot_si128(Mcp, v8_0), _mm_and_si128(Mcp, P));
  __m128i R = _mm_shuffle_epi32(v8_1, _MM_SHUFFLE(1, 0, 1, 0));
  __m128i v8_2 = _mm_or_si128(_mm_andnot_si128(Mcs, R), _mm_and_si128(Mcs, S));
  __m128 Cf = _mm_castsi128_ps(v8_0);
  __m128 Pf = _mm_castsi128_ps(v8_1);
  __m128 Sf = _mm_castsi128_ps(v8_2);
  __m128i v12_0 = _mm_castps_si128(_mm_shuffle_ps(Sf, Pf, _MM_SHUFFLE(0, 0, 0, 2)));
  __m128i v12_1 = _mm_castps_si128(_mm_shuffle_ps(Sf, Pf, _MM_SHUFFLE(1, 1, 1, 3)));
  __m128i v12_2 = _mm_castps_si128(_mm_shuffle_ps(Pf, Cf, _MM_SHUFFLE(0, 0, 3, 2)));
  __m128i v18 = _mm_sub_epi16(v12_0, v12_2);
  __m128i v20 = _mm_add_epi16(v12_2, _mm_set1_epi16(4));
  __m128i v21 = _mm_add_epi16(v20, _mm_mulhi_epi16(v18, _mm_set1_epi16(26624)));
  __m128i v19 = _mm_sub_epi16(v12_1, v12_2);
  __m128i v32 = _mm_add_epi16(v20, _mm_mulhi_epi16(v19, _mm_set1_epi16(7168)));
  __m128i v31 = _mm_srai_epi16(v18, 4);
  __m128i v33 = _mm_add_epi16(v32, v31);
  __m128i v48_0 = _mm_unpacklo_epi16(v21, v33);
  __m128i v26 = _mm_add_epi16(v20, _mm_mulhi_epi16(v18, _mm_set1_epi16(12288)));
  __m128i v25 = _mm_srai_epi16(v19, 5);
  __m128i v27 = _mm_add_epi16(v26, v25);
  __m128i v38 = _mm_add_epi16(v20, _mm_mulhi_epi16(v19, _mm_set1_epi16(18432)));
  __m128i v37 = _mm_srai_epi16(v18, 7);
  __m128i v39 = _mm_add_epi16(v38, v37);
  __m128i v48_4 = _mm_unpacklo_epi16(v27, v39);
  __m128i v49_0 = _mm_unpacklo_epi16(v48_0, v48_4);
  __m128i v23 = _mm_add_epi16(v20, _mm_mulhi_epi16(v18, _mm_set1_epi16(18432)));
  __m128i v22 = _mm_srai_epi16(v19, 7);
  __m128i v24 = _mm_add_epi16(v23, v22);
  __m128i v35 = _mm_add_epi16(v20, _mm_mulhi_epi16(v19, _mm_set1_epi16(12288)));
  __m128i v34 = _mm_srai_epi16(v18, 5);
  __m128i v36 = _mm_add_epi16(v35, v34);
  __m128i v48_2 = _mm_unpacklo_epi16(v24, v36);
  __m128i v29 = _mm_add_epi16(v20, _mm_mulhi_epi16(v18, _mm_set1_epi16(7168)));
  __m128i v28 = _mm_srai_epi16(v19, 4);
  __m128i v30 = _mm_add_epi16(v29, v28);
  __m128i v40 = _mm_add_epi16(v20, _mm_mulhi_epi16(v19, _mm_set1_epi16(26624)));
  __m128i v48_6 = _mm_unpacklo_epi16(v30, v40);
  __m128i v49_4 = _mm_unpacklo_epi16(v48_2, v48_6);
  __m128i v50_0 = _mm_unpacklo_epi16(v49_0, v49_4);
  __m128i v48_1 = _mm_unpackhi_epi16(v21, v33);
  __m128i v48_5 = _mm_unpackhi_epi16(v27, v39);
  __m128i v49_2 = _mm_unpacklo_epi16(v48_1, v48_5);
  __m128i v48_3 = _mm_unpackhi_epi16(v24, v36);
  __m128i v48_7 = _mm_unpackhi_epi16(v30, v40);
  __m128i v49_6 = _mm_unpacklo_epi16(v48_3, v48_7);
  __m128i v50_4 = _mm_unpacklo_epi16(v49_2, v49_6);
  __m128i v54 = _mm_sub_epi16(v50_0, v50_4);
  __m128i v56 = _mm_add_epi16(v50_4, _mm_mulhi_epi16(v54, _mm_set1_epi16(26624)));
  __m128i v101 = _mm_srai_epi16(v56, 3);
  __m128i v50_1 = _mm_unpackhi_epi16(v49_0, v49_4);
  __m128i v50_5 = _mm_unpackhi_epi16(v49_2, v49_6);
  __m128i v79 = _mm_sub_epi16(v50_1, v50_5);
  __m128i v81 = _mm_add_epi16(v50_5, _mm_mulhi_epi16(v79, _mm_set1_epi16(26624)));
  __m128i v102 = _mm_srai_epi16(v81, 3);
  __m128i v104 = _mm_packus_epi16(v101, v102);
  _mm_store_si128((__m128i * RADRESTRICT)(v16 + 0), v104);

  {
  int v17 = dw;
  char unsigned * RADRESTRICT v106 = v16 + v17;
  __m128i v58 = _mm_add_epi16(v50_4, _mm_mulhi_epi16(v54, _mm_set1_epi16(18432)));
  __m128i v49_1 = _mm_unpackhi_epi16(v48_0, v48_4);
  __m128i v49_5 = _mm_unpackhi_epi16(v48_2, v48_6);
  __m128i v50_2 = _mm_unpacklo_epi16(v49_1, v49_5);
  __m128i v55 = _mm_sub_epi16(v50_2, v50_4);
  __m128i v57 = _mm_srai_epi16(v55, 7);
  __m128i v59 = _mm_add_epi16(v58, v57);
  __m128i v107 = _mm_srai_epi16(v59, 3);
  __m128i v83 = _mm_add_epi16(v50_5, _mm_mulhi_epi16(v79, _mm_set1_epi16(18432)));
  __m128i v50_3 = _mm_unpackhi_epi16(v49_1, v49_5);
  __m128i v80 = _mm_sub_epi16(v50_3, v50_5);
  __m128i v82 = _mm_srai_epi16(v80, 7);
  __m128i v84 = _mm_add_epi16(v83, v82);
  __m128i v108 = _mm_srai_epi16(v84, 3);
  __m128i v110 = _mm_packus_epi16(v107, v108);
  _mm_store_si128((__m128i * RADRESTRICT)(v106 + 0), v110);

  {
  char unsigned * RADRESTRICT v112 = v106 + v17;
  __m128i v61 = _mm_add_epi16(v50_4, _mm_mulhi_epi16(v54, _mm_set1_epi16(12288)));
  __m128i v60 = _mm_srai_epi16(v55, 5);
  __m128i v62 = _mm_add_epi16(v61, v60);
  __m128i v113 = _mm_srai_epi16(v62, 3);
  __m128i v86 = _mm_add_epi16(v50_5, _mm_mulhi_epi16(v79, _mm_set1_epi16(12288)));
  __m128i v85 = _mm_srai_epi16(v80, 5);
  __m128i v87 = _mm_add_epi16(v86, v85);
  __m128i v114 = _mm_srai_epi16(v87, 3);
  __m128i v116 = _mm_packus_epi16(v113, v114);
  _mm_store_si128((__m128i * RADRESTRICT)(v112 + 0), v116);

  {
  char unsigned * RADRESTRICT v118 = v112 + v17;
  __m128i v64 = _mm_add_epi16(v50_4, _mm_mulhi_epi16(v54, _mm_set1_epi16(7168)));
  __m128i v63 = _mm_srai_epi16(v55, 4);
  __m128i v65 = _mm_add_epi16(v64, v63);
  __m128i v119 = _mm_srai_epi16(v65, 3);
  __m128i v89 = _mm_add_epi16(v50_5, _mm_mulhi_epi16(v79, _mm_set1_epi16(7168)));
  __m128i v88 = _mm_srai_epi16(v80, 4);
  __m128i v90 = _mm_add_epi16(v89, v88);
  __m128i v120 = _mm_srai_epi16(v90, 3);
  __m128i v122 = _mm_packus_epi16(v119, v120);
  _mm_store_si128((__m128i * RADRESTRICT)(v118 + 0), v122);

  {
  char unsigned * RADRESTRICT v124 = v118 + v17;
  __m128i v67 = _mm_add_epi16(v50_4, _mm_mulhi_epi16(v55, _mm_set1_epi16(7168)));
  __m128i v66 = _mm_srai_epi16(v54, 4);
  __m128i v68 = _mm_add_epi16(v67, v66);
  __m128i v125 = _mm_srai_epi16(v68, 3);
  __m128i v92 = _mm_add_epi16(v50_5, _mm_mulhi_epi16(v80, _mm_set1_epi16(7168)));
  __m128i v91 = _mm_srai_epi16(v79, 4);
  __m128i v93 = _mm_add_epi16(v92, v91);
  __m128i v126 = _mm_srai_epi16(v93, 3);
  __m128i v128 = _mm_packus_epi16(v125, v126);
  _mm_store_si128((__m128i * RADRESTRICT)(v124 + 0), v128);

  {
  char unsigned * RADRESTRICT v130 = v124 + v17;
  __m128i v70 = _mm_add_epi16(v50_4, _mm_mulhi_epi16(v55, _mm_set1_epi16(12288)));
  __m128i v69 = _mm_srai_epi16(v54, 5);
  __m128i v71 = _mm_add_epi16(v70, v69);
  __m128i v131 = _mm_srai_epi16(v71, 3);
  __m128i v95 = _mm_add_epi16(v50_5, _mm_mulhi_epi16(v80, _mm_set1_epi16(12288)));
  __m128i v94 = _mm_srai_epi16(v79, 5);
  __m128i v96 = _mm_add_epi16(v95, v94);
  __m128i v132 = _mm_srai_epi16(v96, 3);
  __m128i v134 = _mm_packus_epi16(v131, v132);
  _mm_store_si128((__m128i * RADRESTRICT)(v130 + 0), v134);

  {
  char unsigned * RADRESTRICT v136 = v130 + v17;
  __m128i v73 = _mm_add_epi16(v50_4, _mm_mulhi_epi16(v55, _mm_set1_epi16(18432)));
  __m128i v72 = _mm_srai_epi16(v54, 7);
  __m128i v74 = _mm_add_epi16(v73, v72);
  __m128i v137 = _mm_srai_epi16(v74, 3);
  __m128i v98 = _mm_add_epi16(v50_5, _mm_mulhi_epi16(v80, _mm_set1_epi16(18432)));
  __m128i v97 = _mm_srai_epi16(v79, 7);
  __m128i v99 = _mm_add_epi16(v98, v97);
  __m128i v138 = _mm_srai_epi16(v99, 3);
  __m128i v140 = _mm_packus_epi16(v137, v138);
  _mm_store_si128((__m128i * RADRESTRICT)(v136 + 0), v140);

  {
  char unsigned * RADRESTRICT v142 = v136 + v17;
  __m128i v75 = _mm_add_epi16(v50_4, _mm_mulhi_epi16(v55, _mm_set1_epi16(26624)));
  __m128i v143 = _mm_srai_epi16(v75, 3);
  __m128i v100 = _mm_add_epi16(v50_5, _mm_mulhi_epi16(v80, _mm_set1_epi16(26624)));
  __m128i v144 = _mm_srai_epi16(v100, 3);
  __m128i v146 = _mm_packus_epi16(v143, v144);
  _mm_store_si128((__m128i * RADRESTRICT)(v142 + 0), v146);
  }
  }
  }
  }
  }
  }
  }
  }
}

typedef void new_dc_blur_jump(char unsigned * RADRESTRICT dcs0, char unsigned * RADRESTRICT dcs1, char unsigned * RADRESTRICT dcs2, char unsigned * RADRESTRICT t, char unsigned * RADRESTRICT dest, int dw);
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

