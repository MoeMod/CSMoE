// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

static CODEGEN_ATTR void bilinear_8x8_h0_v0(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int sw, int dw)
{
  {
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dest);
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(src);
  __m128i v4 = _mm_loadl_epi64((__m128i *)(v0 + 0));
  _mm_storel_epi64((__m128i * RADRESTRICT)(v1 + 0), v4);

  {
  int v3 = dw;
  char unsigned * RADRESTRICT v57 = v1 + v3;
  int v2 = sw;
  char unsigned * RADRESTRICT v7 = v0 + v2;
  __m128i v8 = _mm_loadl_epi64((__m128i *)(v7 + 0));
  _mm_storel_epi64((__m128i * RADRESTRICT)(v57 + 0), v8);

  {
  char unsigned * RADRESTRICT v59 = v57 + v3;
  char unsigned * RADRESTRICT v11 = v7 + v2;
  __m128i v12 = _mm_loadl_epi64((__m128i *)(v11 + 0));
  _mm_storel_epi64((__m128i * RADRESTRICT)(v59 + 0), v12);

  {
  char unsigned * RADRESTRICT v61 = v59 + v3;
  char unsigned * RADRESTRICT v15 = v11 + v2;
  __m128i v16 = _mm_loadl_epi64((__m128i *)(v15 + 0));
  _mm_storel_epi64((__m128i * RADRESTRICT)(v61 + 0), v16);

  {
  char unsigned * RADRESTRICT v63 = v61 + v3;
  char unsigned * RADRESTRICT v19 = v15 + v2;
  __m128i v20 = _mm_loadl_epi64((__m128i *)(v19 + 0));
  _mm_storel_epi64((__m128i * RADRESTRICT)(v63 + 0), v20);

  {
  char unsigned * RADRESTRICT v65 = v63 + v3;
  char unsigned * RADRESTRICT v23 = v19 + v2;
  __m128i v24 = _mm_loadl_epi64((__m128i *)(v23 + 0));
  _mm_storel_epi64((__m128i * RADRESTRICT)(v65 + 0), v24);

  {
  char unsigned * RADRESTRICT v67 = v65 + v3;
  char unsigned * RADRESTRICT v27 = v23 + v2;
  __m128i v28 = _mm_loadl_epi64((__m128i *)(v27 + 0));
  _mm_storel_epi64((__m128i * RADRESTRICT)(v67 + 0), v28);

  {
  char unsigned * RADRESTRICT v69 = v67 + v3;
  char unsigned * RADRESTRICT v31 = v27 + v2;
  __m128i v32 = _mm_loadl_epi64((__m128i *)(v31 + 0));
  _mm_storel_epi64((__m128i * RADRESTRICT)(v69 + 0), v32);
  }
  }
  }
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void bilinear_8x8_h0_v25(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int sw, int dw)
{
  {
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dest);
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(src);
  __m128i v4 = _mm_loadl_epi64((__m128i *)(v0 + 0));
  __m128i v5 = _mm_unpacklo_epi8(v4, _mm_setzero_si128());
  __m128i v6 = _mm_slli_epi16(v5, 4);
  int v2 = sw;
  char unsigned * RADRESTRICT v7 = v0 + v2;
  __m128i v8 = _mm_loadl_epi64((__m128i *)(v7 + 0));
  __m128i v9 = _mm_unpacklo_epi8(v8, _mm_setzero_si128());
  __m128i v10 = _mm_slli_epi16(v9, 4);
  __m128i v40 = _mm_sub_epi16(v10, v6);
  __m128i v41 = _mm_srai_epi16(v40, 2);
  __m128i v42 = _mm_add_epi16(v6, v41);
  __m128i v43 = _mm_srai_epi16(_mm_add_epi16(v42, _mm_set1_epi16(8)), 4);
  __m128i v44 = _mm_packus_epi16(v43, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v1 + 0), v44);

  {
  int v3 = dw;
  char unsigned * RADRESTRICT v81 = v1 + v3;
  char unsigned * RADRESTRICT v11 = v7 + v2;
  __m128i v12 = _mm_loadl_epi64((__m128i *)(v11 + 0));
  __m128i v13 = _mm_unpacklo_epi8(v12, _mm_setzero_si128());
  __m128i v14 = _mm_slli_epi16(v13, 4);
  __m128i v45 = _mm_sub_epi16(v14, v10);
  __m128i v46 = _mm_srai_epi16(v45, 2);
  __m128i v47 = _mm_add_epi16(v10, v46);
  __m128i v48 = _mm_srai_epi16(_mm_add_epi16(v47, _mm_set1_epi16(8)), 4);
  __m128i v49 = _mm_packus_epi16(v48, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v81 + 0), v49);

  {
  char unsigned * RADRESTRICT v83 = v81 + v3;
  char unsigned * RADRESTRICT v15 = v11 + v2;
  __m128i v16 = _mm_loadl_epi64((__m128i *)(v15 + 0));
  __m128i v17 = _mm_unpacklo_epi8(v16, _mm_setzero_si128());
  __m128i v18 = _mm_slli_epi16(v17, 4);
  __m128i v50 = _mm_sub_epi16(v18, v14);
  __m128i v51 = _mm_srai_epi16(v50, 2);
  __m128i v52 = _mm_add_epi16(v14, v51);
  __m128i v53 = _mm_srai_epi16(_mm_add_epi16(v52, _mm_set1_epi16(8)), 4);
  __m128i v54 = _mm_packus_epi16(v53, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v83 + 0), v54);

  {
  char unsigned * RADRESTRICT v85 = v83 + v3;
  char unsigned * RADRESTRICT v19 = v15 + v2;
  __m128i v20 = _mm_loadl_epi64((__m128i *)(v19 + 0));
  __m128i v21 = _mm_unpacklo_epi8(v20, _mm_setzero_si128());
  __m128i v22 = _mm_slli_epi16(v21, 4);
  __m128i v55 = _mm_sub_epi16(v22, v18);
  __m128i v56 = _mm_srai_epi16(v55, 2);
  __m128i v57 = _mm_add_epi16(v18, v56);
  __m128i v58 = _mm_srai_epi16(_mm_add_epi16(v57, _mm_set1_epi16(8)), 4);
  __m128i v59 = _mm_packus_epi16(v58, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v85 + 0), v59);

  {
  char unsigned * RADRESTRICT v87 = v85 + v3;
  char unsigned * RADRESTRICT v23 = v19 + v2;
  __m128i v24 = _mm_loadl_epi64((__m128i *)(v23 + 0));
  __m128i v25 = _mm_unpacklo_epi8(v24, _mm_setzero_si128());
  __m128i v26 = _mm_slli_epi16(v25, 4);
  __m128i v60 = _mm_sub_epi16(v26, v22);
  __m128i v61 = _mm_srai_epi16(v60, 2);
  __m128i v62 = _mm_add_epi16(v22, v61);
  __m128i v63 = _mm_srai_epi16(_mm_add_epi16(v62, _mm_set1_epi16(8)), 4);
  __m128i v64 = _mm_packus_epi16(v63, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v87 + 0), v64);

  {
  char unsigned * RADRESTRICT v89 = v87 + v3;
  char unsigned * RADRESTRICT v27 = v23 + v2;
  __m128i v28 = _mm_loadl_epi64((__m128i *)(v27 + 0));
  __m128i v29 = _mm_unpacklo_epi8(v28, _mm_setzero_si128());
  __m128i v30 = _mm_slli_epi16(v29, 4);
  __m128i v65 = _mm_sub_epi16(v30, v26);
  __m128i v66 = _mm_srai_epi16(v65, 2);
  __m128i v67 = _mm_add_epi16(v26, v66);
  __m128i v68 = _mm_srai_epi16(_mm_add_epi16(v67, _mm_set1_epi16(8)), 4);
  __m128i v69 = _mm_packus_epi16(v68, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v89 + 0), v69);

  {
  char unsigned * RADRESTRICT v91 = v89 + v3;
  char unsigned * RADRESTRICT v31 = v27 + v2;
  __m128i v32 = _mm_loadl_epi64((__m128i *)(v31 + 0));
  __m128i v33 = _mm_unpacklo_epi8(v32, _mm_setzero_si128());
  __m128i v34 = _mm_slli_epi16(v33, 4);
  __m128i v70 = _mm_sub_epi16(v34, v30);
  __m128i v71 = _mm_srai_epi16(v70, 2);
  __m128i v72 = _mm_add_epi16(v30, v71);
  __m128i v73 = _mm_srai_epi16(_mm_add_epi16(v72, _mm_set1_epi16(8)), 4);
  __m128i v74 = _mm_packus_epi16(v73, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v91 + 0), v74);

  {
  char unsigned * RADRESTRICT v93 = v91 + v3;
  char unsigned * RADRESTRICT v35 = v31 + v2;
  __m128i v36 = _mm_loadl_epi64((__m128i *)(v35 + 0));
  __m128i v37 = _mm_unpacklo_epi8(v36, _mm_setzero_si128());
  __m128i v38 = _mm_slli_epi16(v37, 4);
  __m128i v75 = _mm_sub_epi16(v38, v34);
  __m128i v76 = _mm_srai_epi16(v75, 2);
  __m128i v77 = _mm_add_epi16(v34, v76);
  __m128i v78 = _mm_srai_epi16(_mm_add_epi16(v77, _mm_set1_epi16(8)), 4);
  __m128i v79 = _mm_packus_epi16(v78, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v93 + 0), v79);
  }
  }
  }
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void bilinear_8x8_h0_v50(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int sw, int dw)
{
  {
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dest);
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(src);
  __m128i v4 = _mm_loadl_epi64((__m128i *)(v0 + 0));
  __m128i v5 = _mm_unpacklo_epi8(v4, _mm_setzero_si128());
  __m128i v6 = _mm_slli_epi16(v5, 4);
  int v2 = sw;
  char unsigned * RADRESTRICT v7 = v0 + v2;
  __m128i v8 = _mm_loadl_epi64((__m128i *)(v7 + 0));
  __m128i v9 = _mm_unpacklo_epi8(v8, _mm_setzero_si128());
  __m128i v10 = _mm_slli_epi16(v9, 4);
  __m128i v40 = _mm_avg_epu16(v6, v10);
  __m128i v41 = _mm_srai_epi16(_mm_add_epi16(v40, _mm_set1_epi16(8)), 4);
  __m128i v42 = _mm_packus_epi16(v41, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v1 + 0), v42);

  {
  int v3 = dw;
  char unsigned * RADRESTRICT v65 = v1 + v3;
  char unsigned * RADRESTRICT v11 = v7 + v2;
  __m128i v12 = _mm_loadl_epi64((__m128i *)(v11 + 0));
  __m128i v13 = _mm_unpacklo_epi8(v12, _mm_setzero_si128());
  __m128i v14 = _mm_slli_epi16(v13, 4);
  __m128i v43 = _mm_avg_epu16(v10, v14);
  __m128i v44 = _mm_srai_epi16(_mm_add_epi16(v43, _mm_set1_epi16(8)), 4);
  __m128i v45 = _mm_packus_epi16(v44, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v65 + 0), v45);

  {
  char unsigned * RADRESTRICT v67 = v65 + v3;
  char unsigned * RADRESTRICT v15 = v11 + v2;
  __m128i v16 = _mm_loadl_epi64((__m128i *)(v15 + 0));
  __m128i v17 = _mm_unpacklo_epi8(v16, _mm_setzero_si128());
  __m128i v18 = _mm_slli_epi16(v17, 4);
  __m128i v46 = _mm_avg_epu16(v14, v18);
  __m128i v47 = _mm_srai_epi16(_mm_add_epi16(v46, _mm_set1_epi16(8)), 4);
  __m128i v48 = _mm_packus_epi16(v47, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v67 + 0), v48);

  {
  char unsigned * RADRESTRICT v69 = v67 + v3;
  char unsigned * RADRESTRICT v19 = v15 + v2;
  __m128i v20 = _mm_loadl_epi64((__m128i *)(v19 + 0));
  __m128i v21 = _mm_unpacklo_epi8(v20, _mm_setzero_si128());
  __m128i v22 = _mm_slli_epi16(v21, 4);
  __m128i v49 = _mm_avg_epu16(v18, v22);
  __m128i v50 = _mm_srai_epi16(_mm_add_epi16(v49, _mm_set1_epi16(8)), 4);
  __m128i v51 = _mm_packus_epi16(v50, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v69 + 0), v51);

  {
  char unsigned * RADRESTRICT v71 = v69 + v3;
  char unsigned * RADRESTRICT v23 = v19 + v2;
  __m128i v24 = _mm_loadl_epi64((__m128i *)(v23 + 0));
  __m128i v25 = _mm_unpacklo_epi8(v24, _mm_setzero_si128());
  __m128i v26 = _mm_slli_epi16(v25, 4);
  __m128i v52 = _mm_avg_epu16(v22, v26);
  __m128i v53 = _mm_srai_epi16(_mm_add_epi16(v52, _mm_set1_epi16(8)), 4);
  __m128i v54 = _mm_packus_epi16(v53, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v71 + 0), v54);

  {
  char unsigned * RADRESTRICT v73 = v71 + v3;
  char unsigned * RADRESTRICT v27 = v23 + v2;
  __m128i v28 = _mm_loadl_epi64((__m128i *)(v27 + 0));
  __m128i v29 = _mm_unpacklo_epi8(v28, _mm_setzero_si128());
  __m128i v30 = _mm_slli_epi16(v29, 4);
  __m128i v55 = _mm_avg_epu16(v26, v30);
  __m128i v56 = _mm_srai_epi16(_mm_add_epi16(v55, _mm_set1_epi16(8)), 4);
  __m128i v57 = _mm_packus_epi16(v56, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v73 + 0), v57);

  {
  char unsigned * RADRESTRICT v75 = v73 + v3;
  char unsigned * RADRESTRICT v31 = v27 + v2;
  __m128i v32 = _mm_loadl_epi64((__m128i *)(v31 + 0));
  __m128i v33 = _mm_unpacklo_epi8(v32, _mm_setzero_si128());
  __m128i v34 = _mm_slli_epi16(v33, 4);
  __m128i v58 = _mm_avg_epu16(v30, v34);
  __m128i v59 = _mm_srai_epi16(_mm_add_epi16(v58, _mm_set1_epi16(8)), 4);
  __m128i v60 = _mm_packus_epi16(v59, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v75 + 0), v60);

  {
  char unsigned * RADRESTRICT v77 = v75 + v3;
  char unsigned * RADRESTRICT v35 = v31 + v2;
  __m128i v36 = _mm_loadl_epi64((__m128i *)(v35 + 0));
  __m128i v37 = _mm_unpacklo_epi8(v36, _mm_setzero_si128());
  __m128i v38 = _mm_slli_epi16(v37, 4);
  __m128i v61 = _mm_avg_epu16(v34, v38);
  __m128i v62 = _mm_srai_epi16(_mm_add_epi16(v61, _mm_set1_epi16(8)), 4);
  __m128i v63 = _mm_packus_epi16(v62, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v77 + 0), v63);
  }
  }
  }
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void bilinear_8x8_h0_v75(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int sw, int dw)
{
  {
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dest);
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(src);
  int v2 = sw;
  char unsigned * RADRESTRICT v7 = v0 + v2;
  __m128i v8 = _mm_loadl_epi64((__m128i *)(v7 + 0));
  __m128i v9 = _mm_unpacklo_epi8(v8, _mm_setzero_si128());
  __m128i v10 = _mm_slli_epi16(v9, 4);
  __m128i v4 = _mm_loadl_epi64((__m128i *)(v0 + 0));
  __m128i v5 = _mm_unpacklo_epi8(v4, _mm_setzero_si128());
  __m128i v6 = _mm_slli_epi16(v5, 4);
  __m128i v40 = _mm_sub_epi16(v6, v10);
  __m128i v41 = _mm_srai_epi16(v40, 2);
  __m128i v42 = _mm_add_epi16(v10, v41);
  __m128i v43 = _mm_srai_epi16(_mm_add_epi16(v42, _mm_set1_epi16(8)), 4);
  __m128i v44 = _mm_packus_epi16(v43, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v1 + 0), v44);

  {
  int v3 = dw;
  char unsigned * RADRESTRICT v81 = v1 + v3;
  char unsigned * RADRESTRICT v11 = v7 + v2;
  __m128i v12 = _mm_loadl_epi64((__m128i *)(v11 + 0));
  __m128i v13 = _mm_unpacklo_epi8(v12, _mm_setzero_si128());
  __m128i v14 = _mm_slli_epi16(v13, 4);
  __m128i v45 = _mm_sub_epi16(v10, v14);
  __m128i v46 = _mm_srai_epi16(v45, 2);
  __m128i v47 = _mm_add_epi16(v14, v46);
  __m128i v48 = _mm_srai_epi16(_mm_add_epi16(v47, _mm_set1_epi16(8)), 4);
  __m128i v49 = _mm_packus_epi16(v48, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v81 + 0), v49);

  {
  char unsigned * RADRESTRICT v83 = v81 + v3;
  char unsigned * RADRESTRICT v15 = v11 + v2;
  __m128i v16 = _mm_loadl_epi64((__m128i *)(v15 + 0));
  __m128i v17 = _mm_unpacklo_epi8(v16, _mm_setzero_si128());
  __m128i v18 = _mm_slli_epi16(v17, 4);
  __m128i v50 = _mm_sub_epi16(v14, v18);
  __m128i v51 = _mm_srai_epi16(v50, 2);
  __m128i v52 = _mm_add_epi16(v18, v51);
  __m128i v53 = _mm_srai_epi16(_mm_add_epi16(v52, _mm_set1_epi16(8)), 4);
  __m128i v54 = _mm_packus_epi16(v53, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v83 + 0), v54);

  {
  char unsigned * RADRESTRICT v85 = v83 + v3;
  char unsigned * RADRESTRICT v19 = v15 + v2;
  __m128i v20 = _mm_loadl_epi64((__m128i *)(v19 + 0));
  __m128i v21 = _mm_unpacklo_epi8(v20, _mm_setzero_si128());
  __m128i v22 = _mm_slli_epi16(v21, 4);
  __m128i v55 = _mm_sub_epi16(v18, v22);
  __m128i v56 = _mm_srai_epi16(v55, 2);
  __m128i v57 = _mm_add_epi16(v22, v56);
  __m128i v58 = _mm_srai_epi16(_mm_add_epi16(v57, _mm_set1_epi16(8)), 4);
  __m128i v59 = _mm_packus_epi16(v58, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v85 + 0), v59);

  {
  char unsigned * RADRESTRICT v87 = v85 + v3;
  char unsigned * RADRESTRICT v23 = v19 + v2;
  __m128i v24 = _mm_loadl_epi64((__m128i *)(v23 + 0));
  __m128i v25 = _mm_unpacklo_epi8(v24, _mm_setzero_si128());
  __m128i v26 = _mm_slli_epi16(v25, 4);
  __m128i v60 = _mm_sub_epi16(v22, v26);
  __m128i v61 = _mm_srai_epi16(v60, 2);
  __m128i v62 = _mm_add_epi16(v26, v61);
  __m128i v63 = _mm_srai_epi16(_mm_add_epi16(v62, _mm_set1_epi16(8)), 4);
  __m128i v64 = _mm_packus_epi16(v63, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v87 + 0), v64);

  {
  char unsigned * RADRESTRICT v89 = v87 + v3;
  char unsigned * RADRESTRICT v27 = v23 + v2;
  __m128i v28 = _mm_loadl_epi64((__m128i *)(v27 + 0));
  __m128i v29 = _mm_unpacklo_epi8(v28, _mm_setzero_si128());
  __m128i v30 = _mm_slli_epi16(v29, 4);
  __m128i v65 = _mm_sub_epi16(v26, v30);
  __m128i v66 = _mm_srai_epi16(v65, 2);
  __m128i v67 = _mm_add_epi16(v30, v66);
  __m128i v68 = _mm_srai_epi16(_mm_add_epi16(v67, _mm_set1_epi16(8)), 4);
  __m128i v69 = _mm_packus_epi16(v68, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v89 + 0), v69);

  {
  char unsigned * RADRESTRICT v91 = v89 + v3;
  char unsigned * RADRESTRICT v31 = v27 + v2;
  __m128i v32 = _mm_loadl_epi64((__m128i *)(v31 + 0));
  __m128i v33 = _mm_unpacklo_epi8(v32, _mm_setzero_si128());
  __m128i v34 = _mm_slli_epi16(v33, 4);
  __m128i v70 = _mm_sub_epi16(v30, v34);
  __m128i v71 = _mm_srai_epi16(v70, 2);
  __m128i v72 = _mm_add_epi16(v34, v71);
  __m128i v73 = _mm_srai_epi16(_mm_add_epi16(v72, _mm_set1_epi16(8)), 4);
  __m128i v74 = _mm_packus_epi16(v73, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v91 + 0), v74);

  {
  char unsigned * RADRESTRICT v93 = v91 + v3;
  char unsigned * RADRESTRICT v35 = v31 + v2;
  __m128i v36 = _mm_loadl_epi64((__m128i *)(v35 + 0));
  __m128i v37 = _mm_unpacklo_epi8(v36, _mm_setzero_si128());
  __m128i v38 = _mm_slli_epi16(v37, 4);
  __m128i v75 = _mm_sub_epi16(v34, v38);
  __m128i v76 = _mm_srai_epi16(v75, 2);
  __m128i v77 = _mm_add_epi16(v38, v76);
  __m128i v78 = _mm_srai_epi16(_mm_add_epi16(v77, _mm_set1_epi16(8)), 4);
  __m128i v79 = _mm_packus_epi16(v78, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v93 + 0), v79);
  }
  }
  }
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void bilinear_8x8_h25_v0(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int sw, int dw)
{
  {
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dest);
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(src);
  __m128i v4 = _mm_loadl_epi64((__m128i *)(v0 + 0));
  __m128i v5 = _mm_unpacklo_epi8(v4, _mm_setzero_si128());
  __m128i v6 = _mm_slli_epi16(v5, 4);
  __m128i v7 = _mm_loadl_epi64((__m128i *)(v0 + 1));
  __m128i v8 = _mm_unpacklo_epi8(v7, _mm_setzero_si128());
  __m128i v9 = _mm_slli_epi16(v8, 4);
  __m128i v10 = _mm_sub_epi16(v9, v6);
  __m128i v11 = _mm_srai_epi16(v10, 2);
  __m128i v12 = _mm_add_epi16(v6, v11);
  __m128i v94 = _mm_srai_epi16(_mm_add_epi16(v12, _mm_set1_epi16(8)), 4);
  __m128i v95 = _mm_packus_epi16(v94, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v1 + 0), v95);

  {
  int v3 = dw;
  char unsigned * RADRESTRICT v111 = v1 + v3;
  int v2 = sw;
  char unsigned * RADRESTRICT v13 = v0 + v2;
  __m128i v14 = _mm_loadl_epi64((__m128i *)(v13 + 0));
  __m128i v15 = _mm_unpacklo_epi8(v14, _mm_setzero_si128());
  __m128i v16 = _mm_slli_epi16(v15, 4);
  __m128i v17 = _mm_loadl_epi64((__m128i *)(v13 + 1));
  __m128i v18 = _mm_unpacklo_epi8(v17, _mm_setzero_si128());
  __m128i v19 = _mm_slli_epi16(v18, 4);
  __m128i v20 = _mm_sub_epi16(v19, v16);
  __m128i v21 = _mm_srai_epi16(v20, 2);
  __m128i v22 = _mm_add_epi16(v16, v21);
  __m128i v96 = _mm_srai_epi16(_mm_add_epi16(v22, _mm_set1_epi16(8)), 4);
  __m128i v97 = _mm_packus_epi16(v96, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v111 + 0), v97);

  {
  char unsigned * RADRESTRICT v113 = v111 + v3;
  char unsigned * RADRESTRICT v23 = v13 + v2;
  __m128i v24 = _mm_loadl_epi64((__m128i *)(v23 + 0));
  __m128i v25 = _mm_unpacklo_epi8(v24, _mm_setzero_si128());
  __m128i v26 = _mm_slli_epi16(v25, 4);
  __m128i v27 = _mm_loadl_epi64((__m128i *)(v23 + 1));
  __m128i v28 = _mm_unpacklo_epi8(v27, _mm_setzero_si128());
  __m128i v29 = _mm_slli_epi16(v28, 4);
  __m128i v30 = _mm_sub_epi16(v29, v26);
  __m128i v31 = _mm_srai_epi16(v30, 2);
  __m128i v32 = _mm_add_epi16(v26, v31);
  __m128i v98 = _mm_srai_epi16(_mm_add_epi16(v32, _mm_set1_epi16(8)), 4);
  __m128i v99 = _mm_packus_epi16(v98, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v113 + 0), v99);

  {
  char unsigned * RADRESTRICT v115 = v113 + v3;
  char unsigned * RADRESTRICT v33 = v23 + v2;
  __m128i v34 = _mm_loadl_epi64((__m128i *)(v33 + 0));
  __m128i v35 = _mm_unpacklo_epi8(v34, _mm_setzero_si128());
  __m128i v36 = _mm_slli_epi16(v35, 4);
  __m128i v37 = _mm_loadl_epi64((__m128i *)(v33 + 1));
  __m128i v38 = _mm_unpacklo_epi8(v37, _mm_setzero_si128());
  __m128i v39 = _mm_slli_epi16(v38, 4);
  __m128i v40 = _mm_sub_epi16(v39, v36);
  __m128i v41 = _mm_srai_epi16(v40, 2);
  __m128i v42 = _mm_add_epi16(v36, v41);
  __m128i v100 = _mm_srai_epi16(_mm_add_epi16(v42, _mm_set1_epi16(8)), 4);
  __m128i v101 = _mm_packus_epi16(v100, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v115 + 0), v101);

  {
  char unsigned * RADRESTRICT v117 = v115 + v3;
  char unsigned * RADRESTRICT v43 = v33 + v2;
  __m128i v44 = _mm_loadl_epi64((__m128i *)(v43 + 0));
  __m128i v45 = _mm_unpacklo_epi8(v44, _mm_setzero_si128());
  __m128i v46 = _mm_slli_epi16(v45, 4);
  __m128i v47 = _mm_loadl_epi64((__m128i *)(v43 + 1));
  __m128i v48 = _mm_unpacklo_epi8(v47, _mm_setzero_si128());
  __m128i v49 = _mm_slli_epi16(v48, 4);
  __m128i v50 = _mm_sub_epi16(v49, v46);
  __m128i v51 = _mm_srai_epi16(v50, 2);
  __m128i v52 = _mm_add_epi16(v46, v51);
  __m128i v102 = _mm_srai_epi16(_mm_add_epi16(v52, _mm_set1_epi16(8)), 4);
  __m128i v103 = _mm_packus_epi16(v102, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v117 + 0), v103);

  {
  char unsigned * RADRESTRICT v119 = v117 + v3;
  char unsigned * RADRESTRICT v53 = v43 + v2;
  __m128i v54 = _mm_loadl_epi64((__m128i *)(v53 + 0));
  __m128i v55 = _mm_unpacklo_epi8(v54, _mm_setzero_si128());
  __m128i v56 = _mm_slli_epi16(v55, 4);
  __m128i v57 = _mm_loadl_epi64((__m128i *)(v53 + 1));
  __m128i v58 = _mm_unpacklo_epi8(v57, _mm_setzero_si128());
  __m128i v59 = _mm_slli_epi16(v58, 4);
  __m128i v60 = _mm_sub_epi16(v59, v56);
  __m128i v61 = _mm_srai_epi16(v60, 2);
  __m128i v62 = _mm_add_epi16(v56, v61);
  __m128i v104 = _mm_srai_epi16(_mm_add_epi16(v62, _mm_set1_epi16(8)), 4);
  __m128i v105 = _mm_packus_epi16(v104, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v119 + 0), v105);

  {
  char unsigned * RADRESTRICT v121 = v119 + v3;
  char unsigned * RADRESTRICT v63 = v53 + v2;
  __m128i v64 = _mm_loadl_epi64((__m128i *)(v63 + 0));
  __m128i v65 = _mm_unpacklo_epi8(v64, _mm_setzero_si128());
  __m128i v66 = _mm_slli_epi16(v65, 4);
  __m128i v67 = _mm_loadl_epi64((__m128i *)(v63 + 1));
  __m128i v68 = _mm_unpacklo_epi8(v67, _mm_setzero_si128());
  __m128i v69 = _mm_slli_epi16(v68, 4);
  __m128i v70 = _mm_sub_epi16(v69, v66);
  __m128i v71 = _mm_srai_epi16(v70, 2);
  __m128i v72 = _mm_add_epi16(v66, v71);
  __m128i v106 = _mm_srai_epi16(_mm_add_epi16(v72, _mm_set1_epi16(8)), 4);
  __m128i v107 = _mm_packus_epi16(v106, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v121 + 0), v107);

  {
  char unsigned * RADRESTRICT v123 = v121 + v3;
  char unsigned * RADRESTRICT v73 = v63 + v2;
  __m128i v74 = _mm_loadl_epi64((__m128i *)(v73 + 0));
  __m128i v75 = _mm_unpacklo_epi8(v74, _mm_setzero_si128());
  __m128i v76 = _mm_slli_epi16(v75, 4);
  __m128i v77 = _mm_loadl_epi64((__m128i *)(v73 + 1));
  __m128i v78 = _mm_unpacklo_epi8(v77, _mm_setzero_si128());
  __m128i v79 = _mm_slli_epi16(v78, 4);
  __m128i v80 = _mm_sub_epi16(v79, v76);
  __m128i v81 = _mm_srai_epi16(v80, 2);
  __m128i v82 = _mm_add_epi16(v76, v81);
  __m128i v108 = _mm_srai_epi16(_mm_add_epi16(v82, _mm_set1_epi16(8)), 4);
  __m128i v109 = _mm_packus_epi16(v108, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v123 + 0), v109);
  }
  }
  }
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void bilinear_8x8_h25_v25(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int sw, int dw)
{
  {
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dest);
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(src);
  __m128i v4 = _mm_loadl_epi64((__m128i *)(v0 + 0));
  __m128i v5 = _mm_unpacklo_epi8(v4, _mm_setzero_si128());
  __m128i v6 = _mm_slli_epi16(v5, 4);
  __m128i v7 = _mm_loadl_epi64((__m128i *)(v0 + 1));
  __m128i v8 = _mm_unpacklo_epi8(v7, _mm_setzero_si128());
  __m128i v9 = _mm_slli_epi16(v8, 4);
  __m128i v10 = _mm_sub_epi16(v9, v6);
  __m128i v11 = _mm_srai_epi16(v10, 2);
  __m128i v12 = _mm_add_epi16(v6, v11);
  int v2 = sw;
  char unsigned * RADRESTRICT v13 = v0 + v2;
  __m128i v14 = _mm_loadl_epi64((__m128i *)(v13 + 0));
  __m128i v15 = _mm_unpacklo_epi8(v14, _mm_setzero_si128());
  __m128i v16 = _mm_slli_epi16(v15, 4);
  __m128i v17 = _mm_loadl_epi64((__m128i *)(v13 + 1));
  __m128i v18 = _mm_unpacklo_epi8(v17, _mm_setzero_si128());
  __m128i v19 = _mm_slli_epi16(v18, 4);
  __m128i v20 = _mm_sub_epi16(v19, v16);
  __m128i v21 = _mm_srai_epi16(v20, 2);
  __m128i v22 = _mm_add_epi16(v16, v21);
  __m128i v94 = _mm_sub_epi16(v22, v12);
  __m128i v95 = _mm_srai_epi16(v94, 2);
  __m128i v96 = _mm_add_epi16(v12, v95);
  __m128i v97 = _mm_srai_epi16(_mm_add_epi16(v96, _mm_set1_epi16(8)), 4);
  __m128i v98 = _mm_packus_epi16(v97, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v1 + 0), v98);

  {
  int v3 = dw;
  char unsigned * RADRESTRICT v135 = v1 + v3;
  char unsigned * RADRESTRICT v23 = v13 + v2;
  __m128i v24 = _mm_loadl_epi64((__m128i *)(v23 + 0));
  __m128i v25 = _mm_unpacklo_epi8(v24, _mm_setzero_si128());
  __m128i v26 = _mm_slli_epi16(v25, 4);
  __m128i v27 = _mm_loadl_epi64((__m128i *)(v23 + 1));
  __m128i v28 = _mm_unpacklo_epi8(v27, _mm_setzero_si128());
  __m128i v29 = _mm_slli_epi16(v28, 4);
  __m128i v30 = _mm_sub_epi16(v29, v26);
  __m128i v31 = _mm_srai_epi16(v30, 2);
  __m128i v32 = _mm_add_epi16(v26, v31);
  __m128i v99 = _mm_sub_epi16(v32, v22);
  __m128i v100 = _mm_srai_epi16(v99, 2);
  __m128i v101 = _mm_add_epi16(v22, v100);
  __m128i v102 = _mm_srai_epi16(_mm_add_epi16(v101, _mm_set1_epi16(8)), 4);
  __m128i v103 = _mm_packus_epi16(v102, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v135 + 0), v103);

  {
  char unsigned * RADRESTRICT v137 = v135 + v3;
  char unsigned * RADRESTRICT v33 = v23 + v2;
  __m128i v34 = _mm_loadl_epi64((__m128i *)(v33 + 0));
  __m128i v35 = _mm_unpacklo_epi8(v34, _mm_setzero_si128());
  __m128i v36 = _mm_slli_epi16(v35, 4);
  __m128i v37 = _mm_loadl_epi64((__m128i *)(v33 + 1));
  __m128i v38 = _mm_unpacklo_epi8(v37, _mm_setzero_si128());
  __m128i v39 = _mm_slli_epi16(v38, 4);
  __m128i v40 = _mm_sub_epi16(v39, v36);
  __m128i v41 = _mm_srai_epi16(v40, 2);
  __m128i v42 = _mm_add_epi16(v36, v41);
  __m128i v104 = _mm_sub_epi16(v42, v32);
  __m128i v105 = _mm_srai_epi16(v104, 2);
  __m128i v106 = _mm_add_epi16(v32, v105);
  __m128i v107 = _mm_srai_epi16(_mm_add_epi16(v106, _mm_set1_epi16(8)), 4);
  __m128i v108 = _mm_packus_epi16(v107, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v137 + 0), v108);

  {
  char unsigned * RADRESTRICT v139 = v137 + v3;
  char unsigned * RADRESTRICT v43 = v33 + v2;
  __m128i v44 = _mm_loadl_epi64((__m128i *)(v43 + 0));
  __m128i v45 = _mm_unpacklo_epi8(v44, _mm_setzero_si128());
  __m128i v46 = _mm_slli_epi16(v45, 4);
  __m128i v47 = _mm_loadl_epi64((__m128i *)(v43 + 1));
  __m128i v48 = _mm_unpacklo_epi8(v47, _mm_setzero_si128());
  __m128i v49 = _mm_slli_epi16(v48, 4);
  __m128i v50 = _mm_sub_epi16(v49, v46);
  __m128i v51 = _mm_srai_epi16(v50, 2);
  __m128i v52 = _mm_add_epi16(v46, v51);
  __m128i v109 = _mm_sub_epi16(v52, v42);
  __m128i v110 = _mm_srai_epi16(v109, 2);
  __m128i v111 = _mm_add_epi16(v42, v110);
  __m128i v112 = _mm_srai_epi16(_mm_add_epi16(v111, _mm_set1_epi16(8)), 4);
  __m128i v113 = _mm_packus_epi16(v112, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v139 + 0), v113);

  {
  char unsigned * RADRESTRICT v141 = v139 + v3;
  char unsigned * RADRESTRICT v53 = v43 + v2;
  __m128i v54 = _mm_loadl_epi64((__m128i *)(v53 + 0));
  __m128i v55 = _mm_unpacklo_epi8(v54, _mm_setzero_si128());
  __m128i v56 = _mm_slli_epi16(v55, 4);
  __m128i v57 = _mm_loadl_epi64((__m128i *)(v53 + 1));
  __m128i v58 = _mm_unpacklo_epi8(v57, _mm_setzero_si128());
  __m128i v59 = _mm_slli_epi16(v58, 4);
  __m128i v60 = _mm_sub_epi16(v59, v56);
  __m128i v61 = _mm_srai_epi16(v60, 2);
  __m128i v62 = _mm_add_epi16(v56, v61);
  __m128i v114 = _mm_sub_epi16(v62, v52);
  __m128i v115 = _mm_srai_epi16(v114, 2);
  __m128i v116 = _mm_add_epi16(v52, v115);
  __m128i v117 = _mm_srai_epi16(_mm_add_epi16(v116, _mm_set1_epi16(8)), 4);
  __m128i v118 = _mm_packus_epi16(v117, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v141 + 0), v118);

  {
  char unsigned * RADRESTRICT v143 = v141 + v3;
  char unsigned * RADRESTRICT v63 = v53 + v2;
  __m128i v64 = _mm_loadl_epi64((__m128i *)(v63 + 0));
  __m128i v65 = _mm_unpacklo_epi8(v64, _mm_setzero_si128());
  __m128i v66 = _mm_slli_epi16(v65, 4);
  __m128i v67 = _mm_loadl_epi64((__m128i *)(v63 + 1));
  __m128i v68 = _mm_unpacklo_epi8(v67, _mm_setzero_si128());
  __m128i v69 = _mm_slli_epi16(v68, 4);
  __m128i v70 = _mm_sub_epi16(v69, v66);
  __m128i v71 = _mm_srai_epi16(v70, 2);
  __m128i v72 = _mm_add_epi16(v66, v71);
  __m128i v119 = _mm_sub_epi16(v72, v62);
  __m128i v120 = _mm_srai_epi16(v119, 2);
  __m128i v121 = _mm_add_epi16(v62, v120);
  __m128i v122 = _mm_srai_epi16(_mm_add_epi16(v121, _mm_set1_epi16(8)), 4);
  __m128i v123 = _mm_packus_epi16(v122, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v143 + 0), v123);

  {
  char unsigned * RADRESTRICT v145 = v143 + v3;
  char unsigned * RADRESTRICT v73 = v63 + v2;
  __m128i v74 = _mm_loadl_epi64((__m128i *)(v73 + 0));
  __m128i v75 = _mm_unpacklo_epi8(v74, _mm_setzero_si128());
  __m128i v76 = _mm_slli_epi16(v75, 4);
  __m128i v77 = _mm_loadl_epi64((__m128i *)(v73 + 1));
  __m128i v78 = _mm_unpacklo_epi8(v77, _mm_setzero_si128());
  __m128i v79 = _mm_slli_epi16(v78, 4);
  __m128i v80 = _mm_sub_epi16(v79, v76);
  __m128i v81 = _mm_srai_epi16(v80, 2);
  __m128i v82 = _mm_add_epi16(v76, v81);
  __m128i v124 = _mm_sub_epi16(v82, v72);
  __m128i v125 = _mm_srai_epi16(v124, 2);
  __m128i v126 = _mm_add_epi16(v72, v125);
  __m128i v127 = _mm_srai_epi16(_mm_add_epi16(v126, _mm_set1_epi16(8)), 4);
  __m128i v128 = _mm_packus_epi16(v127, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v145 + 0), v128);

  {
  char unsigned * RADRESTRICT v147 = v145 + v3;
  char unsigned * RADRESTRICT v83 = v73 + v2;
  __m128i v84 = _mm_loadl_epi64((__m128i *)(v83 + 0));
  __m128i v85 = _mm_unpacklo_epi8(v84, _mm_setzero_si128());
  __m128i v86 = _mm_slli_epi16(v85, 4);
  __m128i v87 = _mm_loadl_epi64((__m128i *)(v83 + 1));
  __m128i v88 = _mm_unpacklo_epi8(v87, _mm_setzero_si128());
  __m128i v89 = _mm_slli_epi16(v88, 4);
  __m128i v90 = _mm_sub_epi16(v89, v86);
  __m128i v91 = _mm_srai_epi16(v90, 2);
  __m128i v92 = _mm_add_epi16(v86, v91);
  __m128i v129 = _mm_sub_epi16(v92, v82);
  __m128i v130 = _mm_srai_epi16(v129, 2);
  __m128i v131 = _mm_add_epi16(v82, v130);
  __m128i v132 = _mm_srai_epi16(_mm_add_epi16(v131, _mm_set1_epi16(8)), 4);
  __m128i v133 = _mm_packus_epi16(v132, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v147 + 0), v133);
  }
  }
  }
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void bilinear_8x8_h25_v50(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int sw, int dw)
{
  {
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dest);
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(src);
  __m128i v4 = _mm_loadl_epi64((__m128i *)(v0 + 0));
  __m128i v5 = _mm_unpacklo_epi8(v4, _mm_setzero_si128());
  __m128i v6 = _mm_slli_epi16(v5, 4);
  __m128i v7 = _mm_loadl_epi64((__m128i *)(v0 + 1));
  __m128i v8 = _mm_unpacklo_epi8(v7, _mm_setzero_si128());
  __m128i v9 = _mm_slli_epi16(v8, 4);
  __m128i v10 = _mm_sub_epi16(v9, v6);
  __m128i v11 = _mm_srai_epi16(v10, 2);
  __m128i v12 = _mm_add_epi16(v6, v11);
  int v2 = sw;
  char unsigned * RADRESTRICT v13 = v0 + v2;
  __m128i v14 = _mm_loadl_epi64((__m128i *)(v13 + 0));
  __m128i v15 = _mm_unpacklo_epi8(v14, _mm_setzero_si128());
  __m128i v16 = _mm_slli_epi16(v15, 4);
  __m128i v17 = _mm_loadl_epi64((__m128i *)(v13 + 1));
  __m128i v18 = _mm_unpacklo_epi8(v17, _mm_setzero_si128());
  __m128i v19 = _mm_slli_epi16(v18, 4);
  __m128i v20 = _mm_sub_epi16(v19, v16);
  __m128i v21 = _mm_srai_epi16(v20, 2);
  __m128i v22 = _mm_add_epi16(v16, v21);
  __m128i v94 = _mm_avg_epu16(v12, v22);
  __m128i v95 = _mm_srai_epi16(_mm_add_epi16(v94, _mm_set1_epi16(8)), 4);
  __m128i v96 = _mm_packus_epi16(v95, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v1 + 0), v96);

  {
  int v3 = dw;
  char unsigned * RADRESTRICT v119 = v1 + v3;
  char unsigned * RADRESTRICT v23 = v13 + v2;
  __m128i v24 = _mm_loadl_epi64((__m128i *)(v23 + 0));
  __m128i v25 = _mm_unpacklo_epi8(v24, _mm_setzero_si128());
  __m128i v26 = _mm_slli_epi16(v25, 4);
  __m128i v27 = _mm_loadl_epi64((__m128i *)(v23 + 1));
  __m128i v28 = _mm_unpacklo_epi8(v27, _mm_setzero_si128());
  __m128i v29 = _mm_slli_epi16(v28, 4);
  __m128i v30 = _mm_sub_epi16(v29, v26);
  __m128i v31 = _mm_srai_epi16(v30, 2);
  __m128i v32 = _mm_add_epi16(v26, v31);
  __m128i v97 = _mm_avg_epu16(v22, v32);
  __m128i v98 = _mm_srai_epi16(_mm_add_epi16(v97, _mm_set1_epi16(8)), 4);
  __m128i v99 = _mm_packus_epi16(v98, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v119 + 0), v99);

  {
  char unsigned * RADRESTRICT v121 = v119 + v3;
  char unsigned * RADRESTRICT v33 = v23 + v2;
  __m128i v34 = _mm_loadl_epi64((__m128i *)(v33 + 0));
  __m128i v35 = _mm_unpacklo_epi8(v34, _mm_setzero_si128());
  __m128i v36 = _mm_slli_epi16(v35, 4);
  __m128i v37 = _mm_loadl_epi64((__m128i *)(v33 + 1));
  __m128i v38 = _mm_unpacklo_epi8(v37, _mm_setzero_si128());
  __m128i v39 = _mm_slli_epi16(v38, 4);
  __m128i v40 = _mm_sub_epi16(v39, v36);
  __m128i v41 = _mm_srai_epi16(v40, 2);
  __m128i v42 = _mm_add_epi16(v36, v41);
  __m128i v100 = _mm_avg_epu16(v32, v42);
  __m128i v101 = _mm_srai_epi16(_mm_add_epi16(v100, _mm_set1_epi16(8)), 4);
  __m128i v102 = _mm_packus_epi16(v101, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v121 + 0), v102);

  {
  char unsigned * RADRESTRICT v123 = v121 + v3;
  char unsigned * RADRESTRICT v43 = v33 + v2;
  __m128i v44 = _mm_loadl_epi64((__m128i *)(v43 + 0));
  __m128i v45 = _mm_unpacklo_epi8(v44, _mm_setzero_si128());
  __m128i v46 = _mm_slli_epi16(v45, 4);
  __m128i v47 = _mm_loadl_epi64((__m128i *)(v43 + 1));
  __m128i v48 = _mm_unpacklo_epi8(v47, _mm_setzero_si128());
  __m128i v49 = _mm_slli_epi16(v48, 4);
  __m128i v50 = _mm_sub_epi16(v49, v46);
  __m128i v51 = _mm_srai_epi16(v50, 2);
  __m128i v52 = _mm_add_epi16(v46, v51);
  __m128i v103 = _mm_avg_epu16(v42, v52);
  __m128i v104 = _mm_srai_epi16(_mm_add_epi16(v103, _mm_set1_epi16(8)), 4);
  __m128i v105 = _mm_packus_epi16(v104, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v123 + 0), v105);

  {
  char unsigned * RADRESTRICT v125 = v123 + v3;
  char unsigned * RADRESTRICT v53 = v43 + v2;
  __m128i v54 = _mm_loadl_epi64((__m128i *)(v53 + 0));
  __m128i v55 = _mm_unpacklo_epi8(v54, _mm_setzero_si128());
  __m128i v56 = _mm_slli_epi16(v55, 4);
  __m128i v57 = _mm_loadl_epi64((__m128i *)(v53 + 1));
  __m128i v58 = _mm_unpacklo_epi8(v57, _mm_setzero_si128());
  __m128i v59 = _mm_slli_epi16(v58, 4);
  __m128i v60 = _mm_sub_epi16(v59, v56);
  __m128i v61 = _mm_srai_epi16(v60, 2);
  __m128i v62 = _mm_add_epi16(v56, v61);
  __m128i v106 = _mm_avg_epu16(v52, v62);
  __m128i v107 = _mm_srai_epi16(_mm_add_epi16(v106, _mm_set1_epi16(8)), 4);
  __m128i v108 = _mm_packus_epi16(v107, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v125 + 0), v108);

  {
  char unsigned * RADRESTRICT v127 = v125 + v3;
  char unsigned * RADRESTRICT v63 = v53 + v2;
  __m128i v64 = _mm_loadl_epi64((__m128i *)(v63 + 0));
  __m128i v65 = _mm_unpacklo_epi8(v64, _mm_setzero_si128());
  __m128i v66 = _mm_slli_epi16(v65, 4);
  __m128i v67 = _mm_loadl_epi64((__m128i *)(v63 + 1));
  __m128i v68 = _mm_unpacklo_epi8(v67, _mm_setzero_si128());
  __m128i v69 = _mm_slli_epi16(v68, 4);
  __m128i v70 = _mm_sub_epi16(v69, v66);
  __m128i v71 = _mm_srai_epi16(v70, 2);
  __m128i v72 = _mm_add_epi16(v66, v71);
  __m128i v109 = _mm_avg_epu16(v62, v72);
  __m128i v110 = _mm_srai_epi16(_mm_add_epi16(v109, _mm_set1_epi16(8)), 4);
  __m128i v111 = _mm_packus_epi16(v110, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v127 + 0), v111);

  {
  char unsigned * RADRESTRICT v129 = v127 + v3;
  char unsigned * RADRESTRICT v73 = v63 + v2;
  __m128i v74 = _mm_loadl_epi64((__m128i *)(v73 + 0));
  __m128i v75 = _mm_unpacklo_epi8(v74, _mm_setzero_si128());
  __m128i v76 = _mm_slli_epi16(v75, 4);
  __m128i v77 = _mm_loadl_epi64((__m128i *)(v73 + 1));
  __m128i v78 = _mm_unpacklo_epi8(v77, _mm_setzero_si128());
  __m128i v79 = _mm_slli_epi16(v78, 4);
  __m128i v80 = _mm_sub_epi16(v79, v76);
  __m128i v81 = _mm_srai_epi16(v80, 2);
  __m128i v82 = _mm_add_epi16(v76, v81);
  __m128i v112 = _mm_avg_epu16(v72, v82);
  __m128i v113 = _mm_srai_epi16(_mm_add_epi16(v112, _mm_set1_epi16(8)), 4);
  __m128i v114 = _mm_packus_epi16(v113, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v129 + 0), v114);

  {
  char unsigned * RADRESTRICT v131 = v129 + v3;
  char unsigned * RADRESTRICT v83 = v73 + v2;
  __m128i v84 = _mm_loadl_epi64((__m128i *)(v83 + 0));
  __m128i v85 = _mm_unpacklo_epi8(v84, _mm_setzero_si128());
  __m128i v86 = _mm_slli_epi16(v85, 4);
  __m128i v87 = _mm_loadl_epi64((__m128i *)(v83 + 1));
  __m128i v88 = _mm_unpacklo_epi8(v87, _mm_setzero_si128());
  __m128i v89 = _mm_slli_epi16(v88, 4);
  __m128i v90 = _mm_sub_epi16(v89, v86);
  __m128i v91 = _mm_srai_epi16(v90, 2);
  __m128i v92 = _mm_add_epi16(v86, v91);
  __m128i v115 = _mm_avg_epu16(v82, v92);
  __m128i v116 = _mm_srai_epi16(_mm_add_epi16(v115, _mm_set1_epi16(8)), 4);
  __m128i v117 = _mm_packus_epi16(v116, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v131 + 0), v117);
  }
  }
  }
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void bilinear_8x8_h25_v75(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int sw, int dw)
{
  {
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dest);
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(src);
  int v2 = sw;
  char unsigned * RADRESTRICT v13 = v0 + v2;
  __m128i v14 = _mm_loadl_epi64((__m128i *)(v13 + 0));
  __m128i v15 = _mm_unpacklo_epi8(v14, _mm_setzero_si128());
  __m128i v16 = _mm_slli_epi16(v15, 4);
  __m128i v17 = _mm_loadl_epi64((__m128i *)(v13 + 1));
  __m128i v18 = _mm_unpacklo_epi8(v17, _mm_setzero_si128());
  __m128i v19 = _mm_slli_epi16(v18, 4);
  __m128i v20 = _mm_sub_epi16(v19, v16);
  __m128i v21 = _mm_srai_epi16(v20, 2);
  __m128i v22 = _mm_add_epi16(v16, v21);
  __m128i v4 = _mm_loadl_epi64((__m128i *)(v0 + 0));
  __m128i v5 = _mm_unpacklo_epi8(v4, _mm_setzero_si128());
  __m128i v6 = _mm_slli_epi16(v5, 4);
  __m128i v7 = _mm_loadl_epi64((__m128i *)(v0 + 1));
  __m128i v8 = _mm_unpacklo_epi8(v7, _mm_setzero_si128());
  __m128i v9 = _mm_slli_epi16(v8, 4);
  __m128i v10 = _mm_sub_epi16(v9, v6);
  __m128i v11 = _mm_srai_epi16(v10, 2);
  __m128i v12 = _mm_add_epi16(v6, v11);
  __m128i v94 = _mm_sub_epi16(v12, v22);
  __m128i v95 = _mm_srai_epi16(v94, 2);
  __m128i v96 = _mm_add_epi16(v22, v95);
  __m128i v97 = _mm_srai_epi16(_mm_add_epi16(v96, _mm_set1_epi16(8)), 4);
  __m128i v98 = _mm_packus_epi16(v97, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v1 + 0), v98);

  {
  int v3 = dw;
  char unsigned * RADRESTRICT v135 = v1 + v3;
  char unsigned * RADRESTRICT v23 = v13 + v2;
  __m128i v24 = _mm_loadl_epi64((__m128i *)(v23 + 0));
  __m128i v25 = _mm_unpacklo_epi8(v24, _mm_setzero_si128());
  __m128i v26 = _mm_slli_epi16(v25, 4);
  __m128i v27 = _mm_loadl_epi64((__m128i *)(v23 + 1));
  __m128i v28 = _mm_unpacklo_epi8(v27, _mm_setzero_si128());
  __m128i v29 = _mm_slli_epi16(v28, 4);
  __m128i v30 = _mm_sub_epi16(v29, v26);
  __m128i v31 = _mm_srai_epi16(v30, 2);
  __m128i v32 = _mm_add_epi16(v26, v31);
  __m128i v99 = _mm_sub_epi16(v22, v32);
  __m128i v100 = _mm_srai_epi16(v99, 2);
  __m128i v101 = _mm_add_epi16(v32, v100);
  __m128i v102 = _mm_srai_epi16(_mm_add_epi16(v101, _mm_set1_epi16(8)), 4);
  __m128i v103 = _mm_packus_epi16(v102, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v135 + 0), v103);

  {
  char unsigned * RADRESTRICT v137 = v135 + v3;
  char unsigned * RADRESTRICT v33 = v23 + v2;
  __m128i v34 = _mm_loadl_epi64((__m128i *)(v33 + 0));
  __m128i v35 = _mm_unpacklo_epi8(v34, _mm_setzero_si128());
  __m128i v36 = _mm_slli_epi16(v35, 4);
  __m128i v37 = _mm_loadl_epi64((__m128i *)(v33 + 1));
  __m128i v38 = _mm_unpacklo_epi8(v37, _mm_setzero_si128());
  __m128i v39 = _mm_slli_epi16(v38, 4);
  __m128i v40 = _mm_sub_epi16(v39, v36);
  __m128i v41 = _mm_srai_epi16(v40, 2);
  __m128i v42 = _mm_add_epi16(v36, v41);
  __m128i v104 = _mm_sub_epi16(v32, v42);
  __m128i v105 = _mm_srai_epi16(v104, 2);
  __m128i v106 = _mm_add_epi16(v42, v105);
  __m128i v107 = _mm_srai_epi16(_mm_add_epi16(v106, _mm_set1_epi16(8)), 4);
  __m128i v108 = _mm_packus_epi16(v107, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v137 + 0), v108);

  {
  char unsigned * RADRESTRICT v139 = v137 + v3;
  char unsigned * RADRESTRICT v43 = v33 + v2;
  __m128i v44 = _mm_loadl_epi64((__m128i *)(v43 + 0));
  __m128i v45 = _mm_unpacklo_epi8(v44, _mm_setzero_si128());
  __m128i v46 = _mm_slli_epi16(v45, 4);
  __m128i v47 = _mm_loadl_epi64((__m128i *)(v43 + 1));
  __m128i v48 = _mm_unpacklo_epi8(v47, _mm_setzero_si128());
  __m128i v49 = _mm_slli_epi16(v48, 4);
  __m128i v50 = _mm_sub_epi16(v49, v46);
  __m128i v51 = _mm_srai_epi16(v50, 2);
  __m128i v52 = _mm_add_epi16(v46, v51);
  __m128i v109 = _mm_sub_epi16(v42, v52);
  __m128i v110 = _mm_srai_epi16(v109, 2);
  __m128i v111 = _mm_add_epi16(v52, v110);
  __m128i v112 = _mm_srai_epi16(_mm_add_epi16(v111, _mm_set1_epi16(8)), 4);
  __m128i v113 = _mm_packus_epi16(v112, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v139 + 0), v113);

  {
  char unsigned * RADRESTRICT v141 = v139 + v3;
  char unsigned * RADRESTRICT v53 = v43 + v2;
  __m128i v54 = _mm_loadl_epi64((__m128i *)(v53 + 0));
  __m128i v55 = _mm_unpacklo_epi8(v54, _mm_setzero_si128());
  __m128i v56 = _mm_slli_epi16(v55, 4);
  __m128i v57 = _mm_loadl_epi64((__m128i *)(v53 + 1));
  __m128i v58 = _mm_unpacklo_epi8(v57, _mm_setzero_si128());
  __m128i v59 = _mm_slli_epi16(v58, 4);
  __m128i v60 = _mm_sub_epi16(v59, v56);
  __m128i v61 = _mm_srai_epi16(v60, 2);
  __m128i v62 = _mm_add_epi16(v56, v61);
  __m128i v114 = _mm_sub_epi16(v52, v62);
  __m128i v115 = _mm_srai_epi16(v114, 2);
  __m128i v116 = _mm_add_epi16(v62, v115);
  __m128i v117 = _mm_srai_epi16(_mm_add_epi16(v116, _mm_set1_epi16(8)), 4);
  __m128i v118 = _mm_packus_epi16(v117, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v141 + 0), v118);

  {
  char unsigned * RADRESTRICT v143 = v141 + v3;
  char unsigned * RADRESTRICT v63 = v53 + v2;
  __m128i v64 = _mm_loadl_epi64((__m128i *)(v63 + 0));
  __m128i v65 = _mm_unpacklo_epi8(v64, _mm_setzero_si128());
  __m128i v66 = _mm_slli_epi16(v65, 4);
  __m128i v67 = _mm_loadl_epi64((__m128i *)(v63 + 1));
  __m128i v68 = _mm_unpacklo_epi8(v67, _mm_setzero_si128());
  __m128i v69 = _mm_slli_epi16(v68, 4);
  __m128i v70 = _mm_sub_epi16(v69, v66);
  __m128i v71 = _mm_srai_epi16(v70, 2);
  __m128i v72 = _mm_add_epi16(v66, v71);
  __m128i v119 = _mm_sub_epi16(v62, v72);
  __m128i v120 = _mm_srai_epi16(v119, 2);
  __m128i v121 = _mm_add_epi16(v72, v120);
  __m128i v122 = _mm_srai_epi16(_mm_add_epi16(v121, _mm_set1_epi16(8)), 4);
  __m128i v123 = _mm_packus_epi16(v122, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v143 + 0), v123);

  {
  char unsigned * RADRESTRICT v145 = v143 + v3;
  char unsigned * RADRESTRICT v73 = v63 + v2;
  __m128i v74 = _mm_loadl_epi64((__m128i *)(v73 + 0));
  __m128i v75 = _mm_unpacklo_epi8(v74, _mm_setzero_si128());
  __m128i v76 = _mm_slli_epi16(v75, 4);
  __m128i v77 = _mm_loadl_epi64((__m128i *)(v73 + 1));
  __m128i v78 = _mm_unpacklo_epi8(v77, _mm_setzero_si128());
  __m128i v79 = _mm_slli_epi16(v78, 4);
  __m128i v80 = _mm_sub_epi16(v79, v76);
  __m128i v81 = _mm_srai_epi16(v80, 2);
  __m128i v82 = _mm_add_epi16(v76, v81);
  __m128i v124 = _mm_sub_epi16(v72, v82);
  __m128i v125 = _mm_srai_epi16(v124, 2);
  __m128i v126 = _mm_add_epi16(v82, v125);
  __m128i v127 = _mm_srai_epi16(_mm_add_epi16(v126, _mm_set1_epi16(8)), 4);
  __m128i v128 = _mm_packus_epi16(v127, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v145 + 0), v128);

  {
  char unsigned * RADRESTRICT v147 = v145 + v3;
  char unsigned * RADRESTRICT v83 = v73 + v2;
  __m128i v84 = _mm_loadl_epi64((__m128i *)(v83 + 0));
  __m128i v85 = _mm_unpacklo_epi8(v84, _mm_setzero_si128());
  __m128i v86 = _mm_slli_epi16(v85, 4);
  __m128i v87 = _mm_loadl_epi64((__m128i *)(v83 + 1));
  __m128i v88 = _mm_unpacklo_epi8(v87, _mm_setzero_si128());
  __m128i v89 = _mm_slli_epi16(v88, 4);
  __m128i v90 = _mm_sub_epi16(v89, v86);
  __m128i v91 = _mm_srai_epi16(v90, 2);
  __m128i v92 = _mm_add_epi16(v86, v91);
  __m128i v129 = _mm_sub_epi16(v82, v92);
  __m128i v130 = _mm_srai_epi16(v129, 2);
  __m128i v131 = _mm_add_epi16(v92, v130);
  __m128i v132 = _mm_srai_epi16(_mm_add_epi16(v131, _mm_set1_epi16(8)), 4);
  __m128i v133 = _mm_packus_epi16(v132, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v147 + 0), v133);
  }
  }
  }
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void bilinear_8x8_h50_v0(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int sw, int dw)
{
  {
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dest);
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(src);
  __m128i v4 = _mm_loadl_epi64((__m128i *)(v0 + 0));
  __m128i v5 = _mm_unpacklo_epi8(v4, _mm_setzero_si128());
  __m128i v6 = _mm_slli_epi16(v5, 4);
  __m128i v7 = _mm_loadl_epi64((__m128i *)(v0 + 1));
  __m128i v8 = _mm_unpacklo_epi8(v7, _mm_setzero_si128());
  __m128i v9 = _mm_slli_epi16(v8, 4);
  __m128i v10 = _mm_avg_epu16(v6, v9);
  __m128i v76 = _mm_srai_epi16(_mm_add_epi16(v10, _mm_set1_epi16(8)), 4);
  __m128i v77 = _mm_packus_epi16(v76, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v1 + 0), v77);

  {
  int v3 = dw;
  char unsigned * RADRESTRICT v93 = v1 + v3;
  int v2 = sw;
  char unsigned * RADRESTRICT v11 = v0 + v2;
  __m128i v12 = _mm_loadl_epi64((__m128i *)(v11 + 0));
  __m128i v13 = _mm_unpacklo_epi8(v12, _mm_setzero_si128());
  __m128i v14 = _mm_slli_epi16(v13, 4);
  __m128i v15 = _mm_loadl_epi64((__m128i *)(v11 + 1));
  __m128i v16 = _mm_unpacklo_epi8(v15, _mm_setzero_si128());
  __m128i v17 = _mm_slli_epi16(v16, 4);
  __m128i v18 = _mm_avg_epu16(v14, v17);
  __m128i v78 = _mm_srai_epi16(_mm_add_epi16(v18, _mm_set1_epi16(8)), 4);
  __m128i v79 = _mm_packus_epi16(v78, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v93 + 0), v79);

  {
  char unsigned * RADRESTRICT v95 = v93 + v3;
  char unsigned * RADRESTRICT v19 = v11 + v2;
  __m128i v20 = _mm_loadl_epi64((__m128i *)(v19 + 0));
  __m128i v21 = _mm_unpacklo_epi8(v20, _mm_setzero_si128());
  __m128i v22 = _mm_slli_epi16(v21, 4);
  __m128i v23 = _mm_loadl_epi64((__m128i *)(v19 + 1));
  __m128i v24 = _mm_unpacklo_epi8(v23, _mm_setzero_si128());
  __m128i v25 = _mm_slli_epi16(v24, 4);
  __m128i v26 = _mm_avg_epu16(v22, v25);
  __m128i v80 = _mm_srai_epi16(_mm_add_epi16(v26, _mm_set1_epi16(8)), 4);
  __m128i v81 = _mm_packus_epi16(v80, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v95 + 0), v81);

  {
  char unsigned * RADRESTRICT v97 = v95 + v3;
  char unsigned * RADRESTRICT v27 = v19 + v2;
  __m128i v28 = _mm_loadl_epi64((__m128i *)(v27 + 0));
  __m128i v29 = _mm_unpacklo_epi8(v28, _mm_setzero_si128());
  __m128i v30 = _mm_slli_epi16(v29, 4);
  __m128i v31 = _mm_loadl_epi64((__m128i *)(v27 + 1));
  __m128i v32 = _mm_unpacklo_epi8(v31, _mm_setzero_si128());
  __m128i v33 = _mm_slli_epi16(v32, 4);
  __m128i v34 = _mm_avg_epu16(v30, v33);
  __m128i v82 = _mm_srai_epi16(_mm_add_epi16(v34, _mm_set1_epi16(8)), 4);
  __m128i v83 = _mm_packus_epi16(v82, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v97 + 0), v83);

  {
  char unsigned * RADRESTRICT v99 = v97 + v3;
  char unsigned * RADRESTRICT v35 = v27 + v2;
  __m128i v36 = _mm_loadl_epi64((__m128i *)(v35 + 0));
  __m128i v37 = _mm_unpacklo_epi8(v36, _mm_setzero_si128());
  __m128i v38 = _mm_slli_epi16(v37, 4);
  __m128i v39 = _mm_loadl_epi64((__m128i *)(v35 + 1));
  __m128i v40 = _mm_unpacklo_epi8(v39, _mm_setzero_si128());
  __m128i v41 = _mm_slli_epi16(v40, 4);
  __m128i v42 = _mm_avg_epu16(v38, v41);
  __m128i v84 = _mm_srai_epi16(_mm_add_epi16(v42, _mm_set1_epi16(8)), 4);
  __m128i v85 = _mm_packus_epi16(v84, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v99 + 0), v85);

  {
  char unsigned * RADRESTRICT v101 = v99 + v3;
  char unsigned * RADRESTRICT v43 = v35 + v2;
  __m128i v44 = _mm_loadl_epi64((__m128i *)(v43 + 0));
  __m128i v45 = _mm_unpacklo_epi8(v44, _mm_setzero_si128());
  __m128i v46 = _mm_slli_epi16(v45, 4);
  __m128i v47 = _mm_loadl_epi64((__m128i *)(v43 + 1));
  __m128i v48 = _mm_unpacklo_epi8(v47, _mm_setzero_si128());
  __m128i v49 = _mm_slli_epi16(v48, 4);
  __m128i v50 = _mm_avg_epu16(v46, v49);
  __m128i v86 = _mm_srai_epi16(_mm_add_epi16(v50, _mm_set1_epi16(8)), 4);
  __m128i v87 = _mm_packus_epi16(v86, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v101 + 0), v87);

  {
  char unsigned * RADRESTRICT v103 = v101 + v3;
  char unsigned * RADRESTRICT v51 = v43 + v2;
  __m128i v52 = _mm_loadl_epi64((__m128i *)(v51 + 0));
  __m128i v53 = _mm_unpacklo_epi8(v52, _mm_setzero_si128());
  __m128i v54 = _mm_slli_epi16(v53, 4);
  __m128i v55 = _mm_loadl_epi64((__m128i *)(v51 + 1));
  __m128i v56 = _mm_unpacklo_epi8(v55, _mm_setzero_si128());
  __m128i v57 = _mm_slli_epi16(v56, 4);
  __m128i v58 = _mm_avg_epu16(v54, v57);
  __m128i v88 = _mm_srai_epi16(_mm_add_epi16(v58, _mm_set1_epi16(8)), 4);
  __m128i v89 = _mm_packus_epi16(v88, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v103 + 0), v89);

  {
  char unsigned * RADRESTRICT v105 = v103 + v3;
  char unsigned * RADRESTRICT v59 = v51 + v2;
  __m128i v60 = _mm_loadl_epi64((__m128i *)(v59 + 0));
  __m128i v61 = _mm_unpacklo_epi8(v60, _mm_setzero_si128());
  __m128i v62 = _mm_slli_epi16(v61, 4);
  __m128i v63 = _mm_loadl_epi64((__m128i *)(v59 + 1));
  __m128i v64 = _mm_unpacklo_epi8(v63, _mm_setzero_si128());
  __m128i v65 = _mm_slli_epi16(v64, 4);
  __m128i v66 = _mm_avg_epu16(v62, v65);
  __m128i v90 = _mm_srai_epi16(_mm_add_epi16(v66, _mm_set1_epi16(8)), 4);
  __m128i v91 = _mm_packus_epi16(v90, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v105 + 0), v91);
  }
  }
  }
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void bilinear_8x8_h50_v25(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int sw, int dw)
{
  {
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dest);
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(src);
  __m128i v4 = _mm_loadl_epi64((__m128i *)(v0 + 0));
  __m128i v5 = _mm_unpacklo_epi8(v4, _mm_setzero_si128());
  __m128i v6 = _mm_slli_epi16(v5, 4);
  __m128i v7 = _mm_loadl_epi64((__m128i *)(v0 + 1));
  __m128i v8 = _mm_unpacklo_epi8(v7, _mm_setzero_si128());
  __m128i v9 = _mm_slli_epi16(v8, 4);
  __m128i v10 = _mm_avg_epu16(v6, v9);
  int v2 = sw;
  char unsigned * RADRESTRICT v11 = v0 + v2;
  __m128i v12 = _mm_loadl_epi64((__m128i *)(v11 + 0));
  __m128i v13 = _mm_unpacklo_epi8(v12, _mm_setzero_si128());
  __m128i v14 = _mm_slli_epi16(v13, 4);
  __m128i v15 = _mm_loadl_epi64((__m128i *)(v11 + 1));
  __m128i v16 = _mm_unpacklo_epi8(v15, _mm_setzero_si128());
  __m128i v17 = _mm_slli_epi16(v16, 4);
  __m128i v18 = _mm_avg_epu16(v14, v17);
  __m128i v76 = _mm_sub_epi16(v18, v10);
  __m128i v77 = _mm_srai_epi16(v76, 2);
  __m128i v78 = _mm_add_epi16(v10, v77);
  __m128i v79 = _mm_srai_epi16(_mm_add_epi16(v78, _mm_set1_epi16(8)), 4);
  __m128i v80 = _mm_packus_epi16(v79, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v1 + 0), v80);

  {
  int v3 = dw;
  char unsigned * RADRESTRICT v117 = v1 + v3;
  char unsigned * RADRESTRICT v19 = v11 + v2;
  __m128i v20 = _mm_loadl_epi64((__m128i *)(v19 + 0));
  __m128i v21 = _mm_unpacklo_epi8(v20, _mm_setzero_si128());
  __m128i v22 = _mm_slli_epi16(v21, 4);
  __m128i v23 = _mm_loadl_epi64((__m128i *)(v19 + 1));
  __m128i v24 = _mm_unpacklo_epi8(v23, _mm_setzero_si128());
  __m128i v25 = _mm_slli_epi16(v24, 4);
  __m128i v26 = _mm_avg_epu16(v22, v25);
  __m128i v81 = _mm_sub_epi16(v26, v18);
  __m128i v82 = _mm_srai_epi16(v81, 2);
  __m128i v83 = _mm_add_epi16(v18, v82);
  __m128i v84 = _mm_srai_epi16(_mm_add_epi16(v83, _mm_set1_epi16(8)), 4);
  __m128i v85 = _mm_packus_epi16(v84, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v117 + 0), v85);

  {
  char unsigned * RADRESTRICT v119 = v117 + v3;
  char unsigned * RADRESTRICT v27 = v19 + v2;
  __m128i v28 = _mm_loadl_epi64((__m128i *)(v27 + 0));
  __m128i v29 = _mm_unpacklo_epi8(v28, _mm_setzero_si128());
  __m128i v30 = _mm_slli_epi16(v29, 4);
  __m128i v31 = _mm_loadl_epi64((__m128i *)(v27 + 1));
  __m128i v32 = _mm_unpacklo_epi8(v31, _mm_setzero_si128());
  __m128i v33 = _mm_slli_epi16(v32, 4);
  __m128i v34 = _mm_avg_epu16(v30, v33);
  __m128i v86 = _mm_sub_epi16(v34, v26);
  __m128i v87 = _mm_srai_epi16(v86, 2);
  __m128i v88 = _mm_add_epi16(v26, v87);
  __m128i v89 = _mm_srai_epi16(_mm_add_epi16(v88, _mm_set1_epi16(8)), 4);
  __m128i v90 = _mm_packus_epi16(v89, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v119 + 0), v90);

  {
  char unsigned * RADRESTRICT v121 = v119 + v3;
  char unsigned * RADRESTRICT v35 = v27 + v2;
  __m128i v36 = _mm_loadl_epi64((__m128i *)(v35 + 0));
  __m128i v37 = _mm_unpacklo_epi8(v36, _mm_setzero_si128());
  __m128i v38 = _mm_slli_epi16(v37, 4);
  __m128i v39 = _mm_loadl_epi64((__m128i *)(v35 + 1));
  __m128i v40 = _mm_unpacklo_epi8(v39, _mm_setzero_si128());
  __m128i v41 = _mm_slli_epi16(v40, 4);
  __m128i v42 = _mm_avg_epu16(v38, v41);
  __m128i v91 = _mm_sub_epi16(v42, v34);
  __m128i v92 = _mm_srai_epi16(v91, 2);
  __m128i v93 = _mm_add_epi16(v34, v92);
  __m128i v94 = _mm_srai_epi16(_mm_add_epi16(v93, _mm_set1_epi16(8)), 4);
  __m128i v95 = _mm_packus_epi16(v94, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v121 + 0), v95);

  {
  char unsigned * RADRESTRICT v123 = v121 + v3;
  char unsigned * RADRESTRICT v43 = v35 + v2;
  __m128i v44 = _mm_loadl_epi64((__m128i *)(v43 + 0));
  __m128i v45 = _mm_unpacklo_epi8(v44, _mm_setzero_si128());
  __m128i v46 = _mm_slli_epi16(v45, 4);
  __m128i v47 = _mm_loadl_epi64((__m128i *)(v43 + 1));
  __m128i v48 = _mm_unpacklo_epi8(v47, _mm_setzero_si128());
  __m128i v49 = _mm_slli_epi16(v48, 4);
  __m128i v50 = _mm_avg_epu16(v46, v49);
  __m128i v96 = _mm_sub_epi16(v50, v42);
  __m128i v97 = _mm_srai_epi16(v96, 2);
  __m128i v98 = _mm_add_epi16(v42, v97);
  __m128i v99 = _mm_srai_epi16(_mm_add_epi16(v98, _mm_set1_epi16(8)), 4);
  __m128i v100 = _mm_packus_epi16(v99, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v123 + 0), v100);

  {
  char unsigned * RADRESTRICT v125 = v123 + v3;
  char unsigned * RADRESTRICT v51 = v43 + v2;
  __m128i v52 = _mm_loadl_epi64((__m128i *)(v51 + 0));
  __m128i v53 = _mm_unpacklo_epi8(v52, _mm_setzero_si128());
  __m128i v54 = _mm_slli_epi16(v53, 4);
  __m128i v55 = _mm_loadl_epi64((__m128i *)(v51 + 1));
  __m128i v56 = _mm_unpacklo_epi8(v55, _mm_setzero_si128());
  __m128i v57 = _mm_slli_epi16(v56, 4);
  __m128i v58 = _mm_avg_epu16(v54, v57);
  __m128i v101 = _mm_sub_epi16(v58, v50);
  __m128i v102 = _mm_srai_epi16(v101, 2);
  __m128i v103 = _mm_add_epi16(v50, v102);
  __m128i v104 = _mm_srai_epi16(_mm_add_epi16(v103, _mm_set1_epi16(8)), 4);
  __m128i v105 = _mm_packus_epi16(v104, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v125 + 0), v105);

  {
  char unsigned * RADRESTRICT v127 = v125 + v3;
  char unsigned * RADRESTRICT v59 = v51 + v2;
  __m128i v60 = _mm_loadl_epi64((__m128i *)(v59 + 0));
  __m128i v61 = _mm_unpacklo_epi8(v60, _mm_setzero_si128());
  __m128i v62 = _mm_slli_epi16(v61, 4);
  __m128i v63 = _mm_loadl_epi64((__m128i *)(v59 + 1));
  __m128i v64 = _mm_unpacklo_epi8(v63, _mm_setzero_si128());
  __m128i v65 = _mm_slli_epi16(v64, 4);
  __m128i v66 = _mm_avg_epu16(v62, v65);
  __m128i v106 = _mm_sub_epi16(v66, v58);
  __m128i v107 = _mm_srai_epi16(v106, 2);
  __m128i v108 = _mm_add_epi16(v58, v107);
  __m128i v109 = _mm_srai_epi16(_mm_add_epi16(v108, _mm_set1_epi16(8)), 4);
  __m128i v110 = _mm_packus_epi16(v109, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v127 + 0), v110);

  {
  char unsigned * RADRESTRICT v129 = v127 + v3;
  char unsigned * RADRESTRICT v67 = v59 + v2;
  __m128i v68 = _mm_loadl_epi64((__m128i *)(v67 + 0));
  __m128i v69 = _mm_unpacklo_epi8(v68, _mm_setzero_si128());
  __m128i v70 = _mm_slli_epi16(v69, 4);
  __m128i v71 = _mm_loadl_epi64((__m128i *)(v67 + 1));
  __m128i v72 = _mm_unpacklo_epi8(v71, _mm_setzero_si128());
  __m128i v73 = _mm_slli_epi16(v72, 4);
  __m128i v74 = _mm_avg_epu16(v70, v73);
  __m128i v111 = _mm_sub_epi16(v74, v66);
  __m128i v112 = _mm_srai_epi16(v111, 2);
  __m128i v113 = _mm_add_epi16(v66, v112);
  __m128i v114 = _mm_srai_epi16(_mm_add_epi16(v113, _mm_set1_epi16(8)), 4);
  __m128i v115 = _mm_packus_epi16(v114, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v129 + 0), v115);
  }
  }
  }
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void bilinear_8x8_h50_v50(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int sw, int dw)
{
  {
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dest);
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(src);
  __m128i v4 = _mm_loadl_epi64((__m128i *)(v0 + 0));
  __m128i v5 = _mm_unpacklo_epi8(v4, _mm_setzero_si128());
  __m128i v6 = _mm_slli_epi16(v5, 4);
  __m128i v7 = _mm_loadl_epi64((__m128i *)(v0 + 1));
  __m128i v8 = _mm_unpacklo_epi8(v7, _mm_setzero_si128());
  __m128i v9 = _mm_slli_epi16(v8, 4);
  __m128i v10 = _mm_avg_epu16(v6, v9);
  int v2 = sw;
  char unsigned * RADRESTRICT v11 = v0 + v2;
  __m128i v12 = _mm_loadl_epi64((__m128i *)(v11 + 0));
  __m128i v13 = _mm_unpacklo_epi8(v12, _mm_setzero_si128());
  __m128i v14 = _mm_slli_epi16(v13, 4);
  __m128i v15 = _mm_loadl_epi64((__m128i *)(v11 + 1));
  __m128i v16 = _mm_unpacklo_epi8(v15, _mm_setzero_si128());
  __m128i v17 = _mm_slli_epi16(v16, 4);
  __m128i v18 = _mm_avg_epu16(v14, v17);
  __m128i v76 = _mm_avg_epu16(v10, v18);
  __m128i v77 = _mm_srai_epi16(_mm_add_epi16(v76, _mm_set1_epi16(8)), 4);
  __m128i v78 = _mm_packus_epi16(v77, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v1 + 0), v78);

  {
  int v3 = dw;
  char unsigned * RADRESTRICT v101 = v1 + v3;
  char unsigned * RADRESTRICT v19 = v11 + v2;
  __m128i v20 = _mm_loadl_epi64((__m128i *)(v19 + 0));
  __m128i v21 = _mm_unpacklo_epi8(v20, _mm_setzero_si128());
  __m128i v22 = _mm_slli_epi16(v21, 4);
  __m128i v23 = _mm_loadl_epi64((__m128i *)(v19 + 1));
  __m128i v24 = _mm_unpacklo_epi8(v23, _mm_setzero_si128());
  __m128i v25 = _mm_slli_epi16(v24, 4);
  __m128i v26 = _mm_avg_epu16(v22, v25);
  __m128i v79 = _mm_avg_epu16(v18, v26);
  __m128i v80 = _mm_srai_epi16(_mm_add_epi16(v79, _mm_set1_epi16(8)), 4);
  __m128i v81 = _mm_packus_epi16(v80, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v101 + 0), v81);

  {
  char unsigned * RADRESTRICT v103 = v101 + v3;
  char unsigned * RADRESTRICT v27 = v19 + v2;
  __m128i v28 = _mm_loadl_epi64((__m128i *)(v27 + 0));
  __m128i v29 = _mm_unpacklo_epi8(v28, _mm_setzero_si128());
  __m128i v30 = _mm_slli_epi16(v29, 4);
  __m128i v31 = _mm_loadl_epi64((__m128i *)(v27 + 1));
  __m128i v32 = _mm_unpacklo_epi8(v31, _mm_setzero_si128());
  __m128i v33 = _mm_slli_epi16(v32, 4);
  __m128i v34 = _mm_avg_epu16(v30, v33);
  __m128i v82 = _mm_avg_epu16(v26, v34);
  __m128i v83 = _mm_srai_epi16(_mm_add_epi16(v82, _mm_set1_epi16(8)), 4);
  __m128i v84 = _mm_packus_epi16(v83, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v103 + 0), v84);

  {
  char unsigned * RADRESTRICT v105 = v103 + v3;
  char unsigned * RADRESTRICT v35 = v27 + v2;
  __m128i v36 = _mm_loadl_epi64((__m128i *)(v35 + 0));
  __m128i v37 = _mm_unpacklo_epi8(v36, _mm_setzero_si128());
  __m128i v38 = _mm_slli_epi16(v37, 4);
  __m128i v39 = _mm_loadl_epi64((__m128i *)(v35 + 1));
  __m128i v40 = _mm_unpacklo_epi8(v39, _mm_setzero_si128());
  __m128i v41 = _mm_slli_epi16(v40, 4);
  __m128i v42 = _mm_avg_epu16(v38, v41);
  __m128i v85 = _mm_avg_epu16(v34, v42);
  __m128i v86 = _mm_srai_epi16(_mm_add_epi16(v85, _mm_set1_epi16(8)), 4);
  __m128i v87 = _mm_packus_epi16(v86, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v105 + 0), v87);

  {
  char unsigned * RADRESTRICT v107 = v105 + v3;
  char unsigned * RADRESTRICT v43 = v35 + v2;
  __m128i v44 = _mm_loadl_epi64((__m128i *)(v43 + 0));
  __m128i v45 = _mm_unpacklo_epi8(v44, _mm_setzero_si128());
  __m128i v46 = _mm_slli_epi16(v45, 4);
  __m128i v47 = _mm_loadl_epi64((__m128i *)(v43 + 1));
  __m128i v48 = _mm_unpacklo_epi8(v47, _mm_setzero_si128());
  __m128i v49 = _mm_slli_epi16(v48, 4);
  __m128i v50 = _mm_avg_epu16(v46, v49);
  __m128i v88 = _mm_avg_epu16(v42, v50);
  __m128i v89 = _mm_srai_epi16(_mm_add_epi16(v88, _mm_set1_epi16(8)), 4);
  __m128i v90 = _mm_packus_epi16(v89, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v107 + 0), v90);

  {
  char unsigned * RADRESTRICT v109 = v107 + v3;
  char unsigned * RADRESTRICT v51 = v43 + v2;
  __m128i v52 = _mm_loadl_epi64((__m128i *)(v51 + 0));
  __m128i v53 = _mm_unpacklo_epi8(v52, _mm_setzero_si128());
  __m128i v54 = _mm_slli_epi16(v53, 4);
  __m128i v55 = _mm_loadl_epi64((__m128i *)(v51 + 1));
  __m128i v56 = _mm_unpacklo_epi8(v55, _mm_setzero_si128());
  __m128i v57 = _mm_slli_epi16(v56, 4);
  __m128i v58 = _mm_avg_epu16(v54, v57);
  __m128i v91 = _mm_avg_epu16(v50, v58);
  __m128i v92 = _mm_srai_epi16(_mm_add_epi16(v91, _mm_set1_epi16(8)), 4);
  __m128i v93 = _mm_packus_epi16(v92, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v109 + 0), v93);

  {
  char unsigned * RADRESTRICT v111 = v109 + v3;
  char unsigned * RADRESTRICT v59 = v51 + v2;
  __m128i v60 = _mm_loadl_epi64((__m128i *)(v59 + 0));
  __m128i v61 = _mm_unpacklo_epi8(v60, _mm_setzero_si128());
  __m128i v62 = _mm_slli_epi16(v61, 4);
  __m128i v63 = _mm_loadl_epi64((__m128i *)(v59 + 1));
  __m128i v64 = _mm_unpacklo_epi8(v63, _mm_setzero_si128());
  __m128i v65 = _mm_slli_epi16(v64, 4);
  __m128i v66 = _mm_avg_epu16(v62, v65);
  __m128i v94 = _mm_avg_epu16(v58, v66);
  __m128i v95 = _mm_srai_epi16(_mm_add_epi16(v94, _mm_set1_epi16(8)), 4);
  __m128i v96 = _mm_packus_epi16(v95, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v111 + 0), v96);

  {
  char unsigned * RADRESTRICT v113 = v111 + v3;
  char unsigned * RADRESTRICT v67 = v59 + v2;
  __m128i v68 = _mm_loadl_epi64((__m128i *)(v67 + 0));
  __m128i v69 = _mm_unpacklo_epi8(v68, _mm_setzero_si128());
  __m128i v70 = _mm_slli_epi16(v69, 4);
  __m128i v71 = _mm_loadl_epi64((__m128i *)(v67 + 1));
  __m128i v72 = _mm_unpacklo_epi8(v71, _mm_setzero_si128());
  __m128i v73 = _mm_slli_epi16(v72, 4);
  __m128i v74 = _mm_avg_epu16(v70, v73);
  __m128i v97 = _mm_avg_epu16(v66, v74);
  __m128i v98 = _mm_srai_epi16(_mm_add_epi16(v97, _mm_set1_epi16(8)), 4);
  __m128i v99 = _mm_packus_epi16(v98, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v113 + 0), v99);
  }
  }
  }
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void bilinear_8x8_h50_v75(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int sw, int dw)
{
  {
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dest);
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(src);
  int v2 = sw;
  char unsigned * RADRESTRICT v11 = v0 + v2;
  __m128i v12 = _mm_loadl_epi64((__m128i *)(v11 + 0));
  __m128i v13 = _mm_unpacklo_epi8(v12, _mm_setzero_si128());
  __m128i v14 = _mm_slli_epi16(v13, 4);
  __m128i v15 = _mm_loadl_epi64((__m128i *)(v11 + 1));
  __m128i v16 = _mm_unpacklo_epi8(v15, _mm_setzero_si128());
  __m128i v17 = _mm_slli_epi16(v16, 4);
  __m128i v18 = _mm_avg_epu16(v14, v17);
  __m128i v4 = _mm_loadl_epi64((__m128i *)(v0 + 0));
  __m128i v5 = _mm_unpacklo_epi8(v4, _mm_setzero_si128());
  __m128i v6 = _mm_slli_epi16(v5, 4);
  __m128i v7 = _mm_loadl_epi64((__m128i *)(v0 + 1));
  __m128i v8 = _mm_unpacklo_epi8(v7, _mm_setzero_si128());
  __m128i v9 = _mm_slli_epi16(v8, 4);
  __m128i v10 = _mm_avg_epu16(v6, v9);
  __m128i v76 = _mm_sub_epi16(v10, v18);
  __m128i v77 = _mm_srai_epi16(v76, 2);
  __m128i v78 = _mm_add_epi16(v18, v77);
  __m128i v79 = _mm_srai_epi16(_mm_add_epi16(v78, _mm_set1_epi16(8)), 4);
  __m128i v80 = _mm_packus_epi16(v79, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v1 + 0), v80);

  {
  int v3 = dw;
  char unsigned * RADRESTRICT v117 = v1 + v3;
  char unsigned * RADRESTRICT v19 = v11 + v2;
  __m128i v20 = _mm_loadl_epi64((__m128i *)(v19 + 0));
  __m128i v21 = _mm_unpacklo_epi8(v20, _mm_setzero_si128());
  __m128i v22 = _mm_slli_epi16(v21, 4);
  __m128i v23 = _mm_loadl_epi64((__m128i *)(v19 + 1));
  __m128i v24 = _mm_unpacklo_epi8(v23, _mm_setzero_si128());
  __m128i v25 = _mm_slli_epi16(v24, 4);
  __m128i v26 = _mm_avg_epu16(v22, v25);
  __m128i v81 = _mm_sub_epi16(v18, v26);
  __m128i v82 = _mm_srai_epi16(v81, 2);
  __m128i v83 = _mm_add_epi16(v26, v82);
  __m128i v84 = _mm_srai_epi16(_mm_add_epi16(v83, _mm_set1_epi16(8)), 4);
  __m128i v85 = _mm_packus_epi16(v84, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v117 + 0), v85);

  {
  char unsigned * RADRESTRICT v119 = v117 + v3;
  char unsigned * RADRESTRICT v27 = v19 + v2;
  __m128i v28 = _mm_loadl_epi64((__m128i *)(v27 + 0));
  __m128i v29 = _mm_unpacklo_epi8(v28, _mm_setzero_si128());
  __m128i v30 = _mm_slli_epi16(v29, 4);
  __m128i v31 = _mm_loadl_epi64((__m128i *)(v27 + 1));
  __m128i v32 = _mm_unpacklo_epi8(v31, _mm_setzero_si128());
  __m128i v33 = _mm_slli_epi16(v32, 4);
  __m128i v34 = _mm_avg_epu16(v30, v33);
  __m128i v86 = _mm_sub_epi16(v26, v34);
  __m128i v87 = _mm_srai_epi16(v86, 2);
  __m128i v88 = _mm_add_epi16(v34, v87);
  __m128i v89 = _mm_srai_epi16(_mm_add_epi16(v88, _mm_set1_epi16(8)), 4);
  __m128i v90 = _mm_packus_epi16(v89, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v119 + 0), v90);

  {
  char unsigned * RADRESTRICT v121 = v119 + v3;
  char unsigned * RADRESTRICT v35 = v27 + v2;
  __m128i v36 = _mm_loadl_epi64((__m128i *)(v35 + 0));
  __m128i v37 = _mm_unpacklo_epi8(v36, _mm_setzero_si128());
  __m128i v38 = _mm_slli_epi16(v37, 4);
  __m128i v39 = _mm_loadl_epi64((__m128i *)(v35 + 1));
  __m128i v40 = _mm_unpacklo_epi8(v39, _mm_setzero_si128());
  __m128i v41 = _mm_slli_epi16(v40, 4);
  __m128i v42 = _mm_avg_epu16(v38, v41);
  __m128i v91 = _mm_sub_epi16(v34, v42);
  __m128i v92 = _mm_srai_epi16(v91, 2);
  __m128i v93 = _mm_add_epi16(v42, v92);
  __m128i v94 = _mm_srai_epi16(_mm_add_epi16(v93, _mm_set1_epi16(8)), 4);
  __m128i v95 = _mm_packus_epi16(v94, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v121 + 0), v95);

  {
  char unsigned * RADRESTRICT v123 = v121 + v3;
  char unsigned * RADRESTRICT v43 = v35 + v2;
  __m128i v44 = _mm_loadl_epi64((__m128i *)(v43 + 0));
  __m128i v45 = _mm_unpacklo_epi8(v44, _mm_setzero_si128());
  __m128i v46 = _mm_slli_epi16(v45, 4);
  __m128i v47 = _mm_loadl_epi64((__m128i *)(v43 + 1));
  __m128i v48 = _mm_unpacklo_epi8(v47, _mm_setzero_si128());
  __m128i v49 = _mm_slli_epi16(v48, 4);
  __m128i v50 = _mm_avg_epu16(v46, v49);
  __m128i v96 = _mm_sub_epi16(v42, v50);
  __m128i v97 = _mm_srai_epi16(v96, 2);
  __m128i v98 = _mm_add_epi16(v50, v97);
  __m128i v99 = _mm_srai_epi16(_mm_add_epi16(v98, _mm_set1_epi16(8)), 4);
  __m128i v100 = _mm_packus_epi16(v99, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v123 + 0), v100);

  {
  char unsigned * RADRESTRICT v125 = v123 + v3;
  char unsigned * RADRESTRICT v51 = v43 + v2;
  __m128i v52 = _mm_loadl_epi64((__m128i *)(v51 + 0));
  __m128i v53 = _mm_unpacklo_epi8(v52, _mm_setzero_si128());
  __m128i v54 = _mm_slli_epi16(v53, 4);
  __m128i v55 = _mm_loadl_epi64((__m128i *)(v51 + 1));
  __m128i v56 = _mm_unpacklo_epi8(v55, _mm_setzero_si128());
  __m128i v57 = _mm_slli_epi16(v56, 4);
  __m128i v58 = _mm_avg_epu16(v54, v57);
  __m128i v101 = _mm_sub_epi16(v50, v58);
  __m128i v102 = _mm_srai_epi16(v101, 2);
  __m128i v103 = _mm_add_epi16(v58, v102);
  __m128i v104 = _mm_srai_epi16(_mm_add_epi16(v103, _mm_set1_epi16(8)), 4);
  __m128i v105 = _mm_packus_epi16(v104, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v125 + 0), v105);

  {
  char unsigned * RADRESTRICT v127 = v125 + v3;
  char unsigned * RADRESTRICT v59 = v51 + v2;
  __m128i v60 = _mm_loadl_epi64((__m128i *)(v59 + 0));
  __m128i v61 = _mm_unpacklo_epi8(v60, _mm_setzero_si128());
  __m128i v62 = _mm_slli_epi16(v61, 4);
  __m128i v63 = _mm_loadl_epi64((__m128i *)(v59 + 1));
  __m128i v64 = _mm_unpacklo_epi8(v63, _mm_setzero_si128());
  __m128i v65 = _mm_slli_epi16(v64, 4);
  __m128i v66 = _mm_avg_epu16(v62, v65);
  __m128i v106 = _mm_sub_epi16(v58, v66);
  __m128i v107 = _mm_srai_epi16(v106, 2);
  __m128i v108 = _mm_add_epi16(v66, v107);
  __m128i v109 = _mm_srai_epi16(_mm_add_epi16(v108, _mm_set1_epi16(8)), 4);
  __m128i v110 = _mm_packus_epi16(v109, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v127 + 0), v110);

  {
  char unsigned * RADRESTRICT v129 = v127 + v3;
  char unsigned * RADRESTRICT v67 = v59 + v2;
  __m128i v68 = _mm_loadl_epi64((__m128i *)(v67 + 0));
  __m128i v69 = _mm_unpacklo_epi8(v68, _mm_setzero_si128());
  __m128i v70 = _mm_slli_epi16(v69, 4);
  __m128i v71 = _mm_loadl_epi64((__m128i *)(v67 + 1));
  __m128i v72 = _mm_unpacklo_epi8(v71, _mm_setzero_si128());
  __m128i v73 = _mm_slli_epi16(v72, 4);
  __m128i v74 = _mm_avg_epu16(v70, v73);
  __m128i v111 = _mm_sub_epi16(v66, v74);
  __m128i v112 = _mm_srai_epi16(v111, 2);
  __m128i v113 = _mm_add_epi16(v74, v112);
  __m128i v114 = _mm_srai_epi16(_mm_add_epi16(v113, _mm_set1_epi16(8)), 4);
  __m128i v115 = _mm_packus_epi16(v114, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v129 + 0), v115);
  }
  }
  }
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void bilinear_8x8_h75_v0(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int sw, int dw)
{
  {
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dest);
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(src);
  __m128i v7 = _mm_loadl_epi64((__m128i *)(v0 + 1));
  __m128i v8 = _mm_unpacklo_epi8(v7, _mm_setzero_si128());
  __m128i v9 = _mm_slli_epi16(v8, 4);
  __m128i v4 = _mm_loadl_epi64((__m128i *)(v0 + 0));
  __m128i v5 = _mm_unpacklo_epi8(v4, _mm_setzero_si128());
  __m128i v6 = _mm_slli_epi16(v5, 4);
  __m128i v10 = _mm_sub_epi16(v6, v9);
  __m128i v11 = _mm_srai_epi16(v10, 2);
  __m128i v12 = _mm_add_epi16(v9, v11);
  __m128i v94 = _mm_srai_epi16(_mm_add_epi16(v12, _mm_set1_epi16(8)), 4);
  __m128i v95 = _mm_packus_epi16(v94, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v1 + 0), v95);

  {
  int v3 = dw;
  char unsigned * RADRESTRICT v111 = v1 + v3;
  int v2 = sw;
  char unsigned * RADRESTRICT v13 = v0 + v2;
  __m128i v17 = _mm_loadl_epi64((__m128i *)(v13 + 1));
  __m128i v18 = _mm_unpacklo_epi8(v17, _mm_setzero_si128());
  __m128i v19 = _mm_slli_epi16(v18, 4);
  __m128i v14 = _mm_loadl_epi64((__m128i *)(v13 + 0));
  __m128i v15 = _mm_unpacklo_epi8(v14, _mm_setzero_si128());
  __m128i v16 = _mm_slli_epi16(v15, 4);
  __m128i v20 = _mm_sub_epi16(v16, v19);
  __m128i v21 = _mm_srai_epi16(v20, 2);
  __m128i v22 = _mm_add_epi16(v19, v21);
  __m128i v96 = _mm_srai_epi16(_mm_add_epi16(v22, _mm_set1_epi16(8)), 4);
  __m128i v97 = _mm_packus_epi16(v96, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v111 + 0), v97);

  {
  char unsigned * RADRESTRICT v113 = v111 + v3;
  char unsigned * RADRESTRICT v23 = v13 + v2;
  __m128i v27 = _mm_loadl_epi64((__m128i *)(v23 + 1));
  __m128i v28 = _mm_unpacklo_epi8(v27, _mm_setzero_si128());
  __m128i v29 = _mm_slli_epi16(v28, 4);
  __m128i v24 = _mm_loadl_epi64((__m128i *)(v23 + 0));
  __m128i v25 = _mm_unpacklo_epi8(v24, _mm_setzero_si128());
  __m128i v26 = _mm_slli_epi16(v25, 4);
  __m128i v30 = _mm_sub_epi16(v26, v29);
  __m128i v31 = _mm_srai_epi16(v30, 2);
  __m128i v32 = _mm_add_epi16(v29, v31);
  __m128i v98 = _mm_srai_epi16(_mm_add_epi16(v32, _mm_set1_epi16(8)), 4);
  __m128i v99 = _mm_packus_epi16(v98, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v113 + 0), v99);

  {
  char unsigned * RADRESTRICT v115 = v113 + v3;
  char unsigned * RADRESTRICT v33 = v23 + v2;
  __m128i v37 = _mm_loadl_epi64((__m128i *)(v33 + 1));
  __m128i v38 = _mm_unpacklo_epi8(v37, _mm_setzero_si128());
  __m128i v39 = _mm_slli_epi16(v38, 4);
  __m128i v34 = _mm_loadl_epi64((__m128i *)(v33 + 0));
  __m128i v35 = _mm_unpacklo_epi8(v34, _mm_setzero_si128());
  __m128i v36 = _mm_slli_epi16(v35, 4);
  __m128i v40 = _mm_sub_epi16(v36, v39);
  __m128i v41 = _mm_srai_epi16(v40, 2);
  __m128i v42 = _mm_add_epi16(v39, v41);
  __m128i v100 = _mm_srai_epi16(_mm_add_epi16(v42, _mm_set1_epi16(8)), 4);
  __m128i v101 = _mm_packus_epi16(v100, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v115 + 0), v101);

  {
  char unsigned * RADRESTRICT v117 = v115 + v3;
  char unsigned * RADRESTRICT v43 = v33 + v2;
  __m128i v47 = _mm_loadl_epi64((__m128i *)(v43 + 1));
  __m128i v48 = _mm_unpacklo_epi8(v47, _mm_setzero_si128());
  __m128i v49 = _mm_slli_epi16(v48, 4);
  __m128i v44 = _mm_loadl_epi64((__m128i *)(v43 + 0));
  __m128i v45 = _mm_unpacklo_epi8(v44, _mm_setzero_si128());
  __m128i v46 = _mm_slli_epi16(v45, 4);
  __m128i v50 = _mm_sub_epi16(v46, v49);
  __m128i v51 = _mm_srai_epi16(v50, 2);
  __m128i v52 = _mm_add_epi16(v49, v51);
  __m128i v102 = _mm_srai_epi16(_mm_add_epi16(v52, _mm_set1_epi16(8)), 4);
  __m128i v103 = _mm_packus_epi16(v102, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v117 + 0), v103);

  {
  char unsigned * RADRESTRICT v119 = v117 + v3;
  char unsigned * RADRESTRICT v53 = v43 + v2;
  __m128i v57 = _mm_loadl_epi64((__m128i *)(v53 + 1));
  __m128i v58 = _mm_unpacklo_epi8(v57, _mm_setzero_si128());
  __m128i v59 = _mm_slli_epi16(v58, 4);
  __m128i v54 = _mm_loadl_epi64((__m128i *)(v53 + 0));
  __m128i v55 = _mm_unpacklo_epi8(v54, _mm_setzero_si128());
  __m128i v56 = _mm_slli_epi16(v55, 4);
  __m128i v60 = _mm_sub_epi16(v56, v59);
  __m128i v61 = _mm_srai_epi16(v60, 2);
  __m128i v62 = _mm_add_epi16(v59, v61);
  __m128i v104 = _mm_srai_epi16(_mm_add_epi16(v62, _mm_set1_epi16(8)), 4);
  __m128i v105 = _mm_packus_epi16(v104, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v119 + 0), v105);

  {
  char unsigned * RADRESTRICT v121 = v119 + v3;
  char unsigned * RADRESTRICT v63 = v53 + v2;
  __m128i v67 = _mm_loadl_epi64((__m128i *)(v63 + 1));
  __m128i v68 = _mm_unpacklo_epi8(v67, _mm_setzero_si128());
  __m128i v69 = _mm_slli_epi16(v68, 4);
  __m128i v64 = _mm_loadl_epi64((__m128i *)(v63 + 0));
  __m128i v65 = _mm_unpacklo_epi8(v64, _mm_setzero_si128());
  __m128i v66 = _mm_slli_epi16(v65, 4);
  __m128i v70 = _mm_sub_epi16(v66, v69);
  __m128i v71 = _mm_srai_epi16(v70, 2);
  __m128i v72 = _mm_add_epi16(v69, v71);
  __m128i v106 = _mm_srai_epi16(_mm_add_epi16(v72, _mm_set1_epi16(8)), 4);
  __m128i v107 = _mm_packus_epi16(v106, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v121 + 0), v107);

  {
  char unsigned * RADRESTRICT v123 = v121 + v3;
  char unsigned * RADRESTRICT v73 = v63 + v2;
  __m128i v77 = _mm_loadl_epi64((__m128i *)(v73 + 1));
  __m128i v78 = _mm_unpacklo_epi8(v77, _mm_setzero_si128());
  __m128i v79 = _mm_slli_epi16(v78, 4);
  __m128i v74 = _mm_loadl_epi64((__m128i *)(v73 + 0));
  __m128i v75 = _mm_unpacklo_epi8(v74, _mm_setzero_si128());
  __m128i v76 = _mm_slli_epi16(v75, 4);
  __m128i v80 = _mm_sub_epi16(v76, v79);
  __m128i v81 = _mm_srai_epi16(v80, 2);
  __m128i v82 = _mm_add_epi16(v79, v81);
  __m128i v108 = _mm_srai_epi16(_mm_add_epi16(v82, _mm_set1_epi16(8)), 4);
  __m128i v109 = _mm_packus_epi16(v108, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v123 + 0), v109);
  }
  }
  }
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void bilinear_8x8_h75_v25(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int sw, int dw)
{
  {
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dest);
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(src);
  __m128i v7 = _mm_loadl_epi64((__m128i *)(v0 + 1));
  __m128i v8 = _mm_unpacklo_epi8(v7, _mm_setzero_si128());
  __m128i v9 = _mm_slli_epi16(v8, 4);
  __m128i v4 = _mm_loadl_epi64((__m128i *)(v0 + 0));
  __m128i v5 = _mm_unpacklo_epi8(v4, _mm_setzero_si128());
  __m128i v6 = _mm_slli_epi16(v5, 4);
  __m128i v10 = _mm_sub_epi16(v6, v9);
  __m128i v11 = _mm_srai_epi16(v10, 2);
  __m128i v12 = _mm_add_epi16(v9, v11);
  int v2 = sw;
  char unsigned * RADRESTRICT v13 = v0 + v2;
  __m128i v17 = _mm_loadl_epi64((__m128i *)(v13 + 1));
  __m128i v18 = _mm_unpacklo_epi8(v17, _mm_setzero_si128());
  __m128i v19 = _mm_slli_epi16(v18, 4);
  __m128i v14 = _mm_loadl_epi64((__m128i *)(v13 + 0));
  __m128i v15 = _mm_unpacklo_epi8(v14, _mm_setzero_si128());
  __m128i v16 = _mm_slli_epi16(v15, 4);
  __m128i v20 = _mm_sub_epi16(v16, v19);
  __m128i v21 = _mm_srai_epi16(v20, 2);
  __m128i v22 = _mm_add_epi16(v19, v21);
  __m128i v94 = _mm_sub_epi16(v22, v12);
  __m128i v95 = _mm_srai_epi16(v94, 2);
  __m128i v96 = _mm_add_epi16(v12, v95);
  __m128i v97 = _mm_srai_epi16(_mm_add_epi16(v96, _mm_set1_epi16(8)), 4);
  __m128i v98 = _mm_packus_epi16(v97, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v1 + 0), v98);

  {
  int v3 = dw;
  char unsigned * RADRESTRICT v135 = v1 + v3;
  char unsigned * RADRESTRICT v23 = v13 + v2;
  __m128i v27 = _mm_loadl_epi64((__m128i *)(v23 + 1));
  __m128i v28 = _mm_unpacklo_epi8(v27, _mm_setzero_si128());
  __m128i v29 = _mm_slli_epi16(v28, 4);
  __m128i v24 = _mm_loadl_epi64((__m128i *)(v23 + 0));
  __m128i v25 = _mm_unpacklo_epi8(v24, _mm_setzero_si128());
  __m128i v26 = _mm_slli_epi16(v25, 4);
  __m128i v30 = _mm_sub_epi16(v26, v29);
  __m128i v31 = _mm_srai_epi16(v30, 2);
  __m128i v32 = _mm_add_epi16(v29, v31);
  __m128i v99 = _mm_sub_epi16(v32, v22);
  __m128i v100 = _mm_srai_epi16(v99, 2);
  __m128i v101 = _mm_add_epi16(v22, v100);
  __m128i v102 = _mm_srai_epi16(_mm_add_epi16(v101, _mm_set1_epi16(8)), 4);
  __m128i v103 = _mm_packus_epi16(v102, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v135 + 0), v103);

  {
  char unsigned * RADRESTRICT v137 = v135 + v3;
  char unsigned * RADRESTRICT v33 = v23 + v2;
  __m128i v37 = _mm_loadl_epi64((__m128i *)(v33 + 1));
  __m128i v38 = _mm_unpacklo_epi8(v37, _mm_setzero_si128());
  __m128i v39 = _mm_slli_epi16(v38, 4);
  __m128i v34 = _mm_loadl_epi64((__m128i *)(v33 + 0));
  __m128i v35 = _mm_unpacklo_epi8(v34, _mm_setzero_si128());
  __m128i v36 = _mm_slli_epi16(v35, 4);
  __m128i v40 = _mm_sub_epi16(v36, v39);
  __m128i v41 = _mm_srai_epi16(v40, 2);
  __m128i v42 = _mm_add_epi16(v39, v41);
  __m128i v104 = _mm_sub_epi16(v42, v32);
  __m128i v105 = _mm_srai_epi16(v104, 2);
  __m128i v106 = _mm_add_epi16(v32, v105);
  __m128i v107 = _mm_srai_epi16(_mm_add_epi16(v106, _mm_set1_epi16(8)), 4);
  __m128i v108 = _mm_packus_epi16(v107, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v137 + 0), v108);

  {
  char unsigned * RADRESTRICT v139 = v137 + v3;
  char unsigned * RADRESTRICT v43 = v33 + v2;
  __m128i v47 = _mm_loadl_epi64((__m128i *)(v43 + 1));
  __m128i v48 = _mm_unpacklo_epi8(v47, _mm_setzero_si128());
  __m128i v49 = _mm_slli_epi16(v48, 4);
  __m128i v44 = _mm_loadl_epi64((__m128i *)(v43 + 0));
  __m128i v45 = _mm_unpacklo_epi8(v44, _mm_setzero_si128());
  __m128i v46 = _mm_slli_epi16(v45, 4);
  __m128i v50 = _mm_sub_epi16(v46, v49);
  __m128i v51 = _mm_srai_epi16(v50, 2);
  __m128i v52 = _mm_add_epi16(v49, v51);
  __m128i v109 = _mm_sub_epi16(v52, v42);
  __m128i v110 = _mm_srai_epi16(v109, 2);
  __m128i v111 = _mm_add_epi16(v42, v110);
  __m128i v112 = _mm_srai_epi16(_mm_add_epi16(v111, _mm_set1_epi16(8)), 4);
  __m128i v113 = _mm_packus_epi16(v112, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v139 + 0), v113);

  {
  char unsigned * RADRESTRICT v141 = v139 + v3;
  char unsigned * RADRESTRICT v53 = v43 + v2;
  __m128i v57 = _mm_loadl_epi64((__m128i *)(v53 + 1));
  __m128i v58 = _mm_unpacklo_epi8(v57, _mm_setzero_si128());
  __m128i v59 = _mm_slli_epi16(v58, 4);
  __m128i v54 = _mm_loadl_epi64((__m128i *)(v53 + 0));
  __m128i v55 = _mm_unpacklo_epi8(v54, _mm_setzero_si128());
  __m128i v56 = _mm_slli_epi16(v55, 4);
  __m128i v60 = _mm_sub_epi16(v56, v59);
  __m128i v61 = _mm_srai_epi16(v60, 2);
  __m128i v62 = _mm_add_epi16(v59, v61);
  __m128i v114 = _mm_sub_epi16(v62, v52);
  __m128i v115 = _mm_srai_epi16(v114, 2);
  __m128i v116 = _mm_add_epi16(v52, v115);
  __m128i v117 = _mm_srai_epi16(_mm_add_epi16(v116, _mm_set1_epi16(8)), 4);
  __m128i v118 = _mm_packus_epi16(v117, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v141 + 0), v118);

  {
  char unsigned * RADRESTRICT v143 = v141 + v3;
  char unsigned * RADRESTRICT v63 = v53 + v2;
  __m128i v67 = _mm_loadl_epi64((__m128i *)(v63 + 1));
  __m128i v68 = _mm_unpacklo_epi8(v67, _mm_setzero_si128());
  __m128i v69 = _mm_slli_epi16(v68, 4);
  __m128i v64 = _mm_loadl_epi64((__m128i *)(v63 + 0));
  __m128i v65 = _mm_unpacklo_epi8(v64, _mm_setzero_si128());
  __m128i v66 = _mm_slli_epi16(v65, 4);
  __m128i v70 = _mm_sub_epi16(v66, v69);
  __m128i v71 = _mm_srai_epi16(v70, 2);
  __m128i v72 = _mm_add_epi16(v69, v71);
  __m128i v119 = _mm_sub_epi16(v72, v62);
  __m128i v120 = _mm_srai_epi16(v119, 2);
  __m128i v121 = _mm_add_epi16(v62, v120);
  __m128i v122 = _mm_srai_epi16(_mm_add_epi16(v121, _mm_set1_epi16(8)), 4);
  __m128i v123 = _mm_packus_epi16(v122, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v143 + 0), v123);

  {
  char unsigned * RADRESTRICT v145 = v143 + v3;
  char unsigned * RADRESTRICT v73 = v63 + v2;
  __m128i v77 = _mm_loadl_epi64((__m128i *)(v73 + 1));
  __m128i v78 = _mm_unpacklo_epi8(v77, _mm_setzero_si128());
  __m128i v79 = _mm_slli_epi16(v78, 4);
  __m128i v74 = _mm_loadl_epi64((__m128i *)(v73 + 0));
  __m128i v75 = _mm_unpacklo_epi8(v74, _mm_setzero_si128());
  __m128i v76 = _mm_slli_epi16(v75, 4);
  __m128i v80 = _mm_sub_epi16(v76, v79);
  __m128i v81 = _mm_srai_epi16(v80, 2);
  __m128i v82 = _mm_add_epi16(v79, v81);
  __m128i v124 = _mm_sub_epi16(v82, v72);
  __m128i v125 = _mm_srai_epi16(v124, 2);
  __m128i v126 = _mm_add_epi16(v72, v125);
  __m128i v127 = _mm_srai_epi16(_mm_add_epi16(v126, _mm_set1_epi16(8)), 4);
  __m128i v128 = _mm_packus_epi16(v127, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v145 + 0), v128);

  {
  char unsigned * RADRESTRICT v147 = v145 + v3;
  char unsigned * RADRESTRICT v83 = v73 + v2;
  __m128i v87 = _mm_loadl_epi64((__m128i *)(v83 + 1));
  __m128i v88 = _mm_unpacklo_epi8(v87, _mm_setzero_si128());
  __m128i v89 = _mm_slli_epi16(v88, 4);
  __m128i v84 = _mm_loadl_epi64((__m128i *)(v83 + 0));
  __m128i v85 = _mm_unpacklo_epi8(v84, _mm_setzero_si128());
  __m128i v86 = _mm_slli_epi16(v85, 4);
  __m128i v90 = _mm_sub_epi16(v86, v89);
  __m128i v91 = _mm_srai_epi16(v90, 2);
  __m128i v92 = _mm_add_epi16(v89, v91);
  __m128i v129 = _mm_sub_epi16(v92, v82);
  __m128i v130 = _mm_srai_epi16(v129, 2);
  __m128i v131 = _mm_add_epi16(v82, v130);
  __m128i v132 = _mm_srai_epi16(_mm_add_epi16(v131, _mm_set1_epi16(8)), 4);
  __m128i v133 = _mm_packus_epi16(v132, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v147 + 0), v133);
  }
  }
  }
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void bilinear_8x8_h75_v50(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int sw, int dw)
{
  {
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dest);
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(src);
  __m128i v7 = _mm_loadl_epi64((__m128i *)(v0 + 1));
  __m128i v8 = _mm_unpacklo_epi8(v7, _mm_setzero_si128());
  __m128i v9 = _mm_slli_epi16(v8, 4);
  __m128i v4 = _mm_loadl_epi64((__m128i *)(v0 + 0));
  __m128i v5 = _mm_unpacklo_epi8(v4, _mm_setzero_si128());
  __m128i v6 = _mm_slli_epi16(v5, 4);
  __m128i v10 = _mm_sub_epi16(v6, v9);
  __m128i v11 = _mm_srai_epi16(v10, 2);
  __m128i v12 = _mm_add_epi16(v9, v11);
  int v2 = sw;
  char unsigned * RADRESTRICT v13 = v0 + v2;
  __m128i v17 = _mm_loadl_epi64((__m128i *)(v13 + 1));
  __m128i v18 = _mm_unpacklo_epi8(v17, _mm_setzero_si128());
  __m128i v19 = _mm_slli_epi16(v18, 4);
  __m128i v14 = _mm_loadl_epi64((__m128i *)(v13 + 0));
  __m128i v15 = _mm_unpacklo_epi8(v14, _mm_setzero_si128());
  __m128i v16 = _mm_slli_epi16(v15, 4);
  __m128i v20 = _mm_sub_epi16(v16, v19);
  __m128i v21 = _mm_srai_epi16(v20, 2);
  __m128i v22 = _mm_add_epi16(v19, v21);
  __m128i v94 = _mm_avg_epu16(v12, v22);
  __m128i v95 = _mm_srai_epi16(_mm_add_epi16(v94, _mm_set1_epi16(8)), 4);
  __m128i v96 = _mm_packus_epi16(v95, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v1 + 0), v96);

  {
  int v3 = dw;
  char unsigned * RADRESTRICT v119 = v1 + v3;
  char unsigned * RADRESTRICT v23 = v13 + v2;
  __m128i v27 = _mm_loadl_epi64((__m128i *)(v23 + 1));
  __m128i v28 = _mm_unpacklo_epi8(v27, _mm_setzero_si128());
  __m128i v29 = _mm_slli_epi16(v28, 4);
  __m128i v24 = _mm_loadl_epi64((__m128i *)(v23 + 0));
  __m128i v25 = _mm_unpacklo_epi8(v24, _mm_setzero_si128());
  __m128i v26 = _mm_slli_epi16(v25, 4);
  __m128i v30 = _mm_sub_epi16(v26, v29);
  __m128i v31 = _mm_srai_epi16(v30, 2);
  __m128i v32 = _mm_add_epi16(v29, v31);
  __m128i v97 = _mm_avg_epu16(v22, v32);
  __m128i v98 = _mm_srai_epi16(_mm_add_epi16(v97, _mm_set1_epi16(8)), 4);
  __m128i v99 = _mm_packus_epi16(v98, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v119 + 0), v99);

  {
  char unsigned * RADRESTRICT v121 = v119 + v3;
  char unsigned * RADRESTRICT v33 = v23 + v2;
  __m128i v37 = _mm_loadl_epi64((__m128i *)(v33 + 1));
  __m128i v38 = _mm_unpacklo_epi8(v37, _mm_setzero_si128());
  __m128i v39 = _mm_slli_epi16(v38, 4);
  __m128i v34 = _mm_loadl_epi64((__m128i *)(v33 + 0));
  __m128i v35 = _mm_unpacklo_epi8(v34, _mm_setzero_si128());
  __m128i v36 = _mm_slli_epi16(v35, 4);
  __m128i v40 = _mm_sub_epi16(v36, v39);
  __m128i v41 = _mm_srai_epi16(v40, 2);
  __m128i v42 = _mm_add_epi16(v39, v41);
  __m128i v100 = _mm_avg_epu16(v32, v42);
  __m128i v101 = _mm_srai_epi16(_mm_add_epi16(v100, _mm_set1_epi16(8)), 4);
  __m128i v102 = _mm_packus_epi16(v101, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v121 + 0), v102);

  {
  char unsigned * RADRESTRICT v123 = v121 + v3;
  char unsigned * RADRESTRICT v43 = v33 + v2;
  __m128i v47 = _mm_loadl_epi64((__m128i *)(v43 + 1));
  __m128i v48 = _mm_unpacklo_epi8(v47, _mm_setzero_si128());
  __m128i v49 = _mm_slli_epi16(v48, 4);
  __m128i v44 = _mm_loadl_epi64((__m128i *)(v43 + 0));
  __m128i v45 = _mm_unpacklo_epi8(v44, _mm_setzero_si128());
  __m128i v46 = _mm_slli_epi16(v45, 4);
  __m128i v50 = _mm_sub_epi16(v46, v49);
  __m128i v51 = _mm_srai_epi16(v50, 2);
  __m128i v52 = _mm_add_epi16(v49, v51);
  __m128i v103 = _mm_avg_epu16(v42, v52);
  __m128i v104 = _mm_srai_epi16(_mm_add_epi16(v103, _mm_set1_epi16(8)), 4);
  __m128i v105 = _mm_packus_epi16(v104, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v123 + 0), v105);

  {
  char unsigned * RADRESTRICT v125 = v123 + v3;
  char unsigned * RADRESTRICT v53 = v43 + v2;
  __m128i v57 = _mm_loadl_epi64((__m128i *)(v53 + 1));
  __m128i v58 = _mm_unpacklo_epi8(v57, _mm_setzero_si128());
  __m128i v59 = _mm_slli_epi16(v58, 4);
  __m128i v54 = _mm_loadl_epi64((__m128i *)(v53 + 0));
  __m128i v55 = _mm_unpacklo_epi8(v54, _mm_setzero_si128());
  __m128i v56 = _mm_slli_epi16(v55, 4);
  __m128i v60 = _mm_sub_epi16(v56, v59);
  __m128i v61 = _mm_srai_epi16(v60, 2);
  __m128i v62 = _mm_add_epi16(v59, v61);
  __m128i v106 = _mm_avg_epu16(v52, v62);
  __m128i v107 = _mm_srai_epi16(_mm_add_epi16(v106, _mm_set1_epi16(8)), 4);
  __m128i v108 = _mm_packus_epi16(v107, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v125 + 0), v108);

  {
  char unsigned * RADRESTRICT v127 = v125 + v3;
  char unsigned * RADRESTRICT v63 = v53 + v2;
  __m128i v67 = _mm_loadl_epi64((__m128i *)(v63 + 1));
  __m128i v68 = _mm_unpacklo_epi8(v67, _mm_setzero_si128());
  __m128i v69 = _mm_slli_epi16(v68, 4);
  __m128i v64 = _mm_loadl_epi64((__m128i *)(v63 + 0));
  __m128i v65 = _mm_unpacklo_epi8(v64, _mm_setzero_si128());
  __m128i v66 = _mm_slli_epi16(v65, 4);
  __m128i v70 = _mm_sub_epi16(v66, v69);
  __m128i v71 = _mm_srai_epi16(v70, 2);
  __m128i v72 = _mm_add_epi16(v69, v71);
  __m128i v109 = _mm_avg_epu16(v62, v72);
  __m128i v110 = _mm_srai_epi16(_mm_add_epi16(v109, _mm_set1_epi16(8)), 4);
  __m128i v111 = _mm_packus_epi16(v110, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v127 + 0), v111);

  {
  char unsigned * RADRESTRICT v129 = v127 + v3;
  char unsigned * RADRESTRICT v73 = v63 + v2;
  __m128i v77 = _mm_loadl_epi64((__m128i *)(v73 + 1));
  __m128i v78 = _mm_unpacklo_epi8(v77, _mm_setzero_si128());
  __m128i v79 = _mm_slli_epi16(v78, 4);
  __m128i v74 = _mm_loadl_epi64((__m128i *)(v73 + 0));
  __m128i v75 = _mm_unpacklo_epi8(v74, _mm_setzero_si128());
  __m128i v76 = _mm_slli_epi16(v75, 4);
  __m128i v80 = _mm_sub_epi16(v76, v79);
  __m128i v81 = _mm_srai_epi16(v80, 2);
  __m128i v82 = _mm_add_epi16(v79, v81);
  __m128i v112 = _mm_avg_epu16(v72, v82);
  __m128i v113 = _mm_srai_epi16(_mm_add_epi16(v112, _mm_set1_epi16(8)), 4);
  __m128i v114 = _mm_packus_epi16(v113, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v129 + 0), v114);

  {
  char unsigned * RADRESTRICT v131 = v129 + v3;
  char unsigned * RADRESTRICT v83 = v73 + v2;
  __m128i v87 = _mm_loadl_epi64((__m128i *)(v83 + 1));
  __m128i v88 = _mm_unpacklo_epi8(v87, _mm_setzero_si128());
  __m128i v89 = _mm_slli_epi16(v88, 4);
  __m128i v84 = _mm_loadl_epi64((__m128i *)(v83 + 0));
  __m128i v85 = _mm_unpacklo_epi8(v84, _mm_setzero_si128());
  __m128i v86 = _mm_slli_epi16(v85, 4);
  __m128i v90 = _mm_sub_epi16(v86, v89);
  __m128i v91 = _mm_srai_epi16(v90, 2);
  __m128i v92 = _mm_add_epi16(v89, v91);
  __m128i v115 = _mm_avg_epu16(v82, v92);
  __m128i v116 = _mm_srai_epi16(_mm_add_epi16(v115, _mm_set1_epi16(8)), 4);
  __m128i v117 = _mm_packus_epi16(v116, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v131 + 0), v117);
  }
  }
  }
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void bilinear_8x8_h75_v75(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int sw, int dw)
{
  {
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dest);
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(src);
  int v2 = sw;
  char unsigned * RADRESTRICT v13 = v0 + v2;
  __m128i v17 = _mm_loadl_epi64((__m128i *)(v13 + 1));
  __m128i v18 = _mm_unpacklo_epi8(v17, _mm_setzero_si128());
  __m128i v19 = _mm_slli_epi16(v18, 4);
  __m128i v14 = _mm_loadl_epi64((__m128i *)(v13 + 0));
  __m128i v15 = _mm_unpacklo_epi8(v14, _mm_setzero_si128());
  __m128i v16 = _mm_slli_epi16(v15, 4);
  __m128i v20 = _mm_sub_epi16(v16, v19);
  __m128i v21 = _mm_srai_epi16(v20, 2);
  __m128i v22 = _mm_add_epi16(v19, v21);
  __m128i v7 = _mm_loadl_epi64((__m128i *)(v0 + 1));
  __m128i v8 = _mm_unpacklo_epi8(v7, _mm_setzero_si128());
  __m128i v9 = _mm_slli_epi16(v8, 4);
  __m128i v4 = _mm_loadl_epi64((__m128i *)(v0 + 0));
  __m128i v5 = _mm_unpacklo_epi8(v4, _mm_setzero_si128());
  __m128i v6 = _mm_slli_epi16(v5, 4);
  __m128i v10 = _mm_sub_epi16(v6, v9);
  __m128i v11 = _mm_srai_epi16(v10, 2);
  __m128i v12 = _mm_add_epi16(v9, v11);
  __m128i v94 = _mm_sub_epi16(v12, v22);
  __m128i v95 = _mm_srai_epi16(v94, 2);
  __m128i v96 = _mm_add_epi16(v22, v95);
  __m128i v97 = _mm_srai_epi16(_mm_add_epi16(v96, _mm_set1_epi16(8)), 4);
  __m128i v98 = _mm_packus_epi16(v97, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v1 + 0), v98);

  {
  int v3 = dw;
  char unsigned * RADRESTRICT v135 = v1 + v3;
  char unsigned * RADRESTRICT v23 = v13 + v2;
  __m128i v27 = _mm_loadl_epi64((__m128i *)(v23 + 1));
  __m128i v28 = _mm_unpacklo_epi8(v27, _mm_setzero_si128());
  __m128i v29 = _mm_slli_epi16(v28, 4);
  __m128i v24 = _mm_loadl_epi64((__m128i *)(v23 + 0));
  __m128i v25 = _mm_unpacklo_epi8(v24, _mm_setzero_si128());
  __m128i v26 = _mm_slli_epi16(v25, 4);
  __m128i v30 = _mm_sub_epi16(v26, v29);
  __m128i v31 = _mm_srai_epi16(v30, 2);
  __m128i v32 = _mm_add_epi16(v29, v31);
  __m128i v99 = _mm_sub_epi16(v22, v32);
  __m128i v100 = _mm_srai_epi16(v99, 2);
  __m128i v101 = _mm_add_epi16(v32, v100);
  __m128i v102 = _mm_srai_epi16(_mm_add_epi16(v101, _mm_set1_epi16(8)), 4);
  __m128i v103 = _mm_packus_epi16(v102, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v135 + 0), v103);

  {
  char unsigned * RADRESTRICT v137 = v135 + v3;
  char unsigned * RADRESTRICT v33 = v23 + v2;
  __m128i v37 = _mm_loadl_epi64((__m128i *)(v33 + 1));
  __m128i v38 = _mm_unpacklo_epi8(v37, _mm_setzero_si128());
  __m128i v39 = _mm_slli_epi16(v38, 4);
  __m128i v34 = _mm_loadl_epi64((__m128i *)(v33 + 0));
  __m128i v35 = _mm_unpacklo_epi8(v34, _mm_setzero_si128());
  __m128i v36 = _mm_slli_epi16(v35, 4);
  __m128i v40 = _mm_sub_epi16(v36, v39);
  __m128i v41 = _mm_srai_epi16(v40, 2);
  __m128i v42 = _mm_add_epi16(v39, v41);
  __m128i v104 = _mm_sub_epi16(v32, v42);
  __m128i v105 = _mm_srai_epi16(v104, 2);
  __m128i v106 = _mm_add_epi16(v42, v105);
  __m128i v107 = _mm_srai_epi16(_mm_add_epi16(v106, _mm_set1_epi16(8)), 4);
  __m128i v108 = _mm_packus_epi16(v107, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v137 + 0), v108);

  {
  char unsigned * RADRESTRICT v139 = v137 + v3;
  char unsigned * RADRESTRICT v43 = v33 + v2;
  __m128i v47 = _mm_loadl_epi64((__m128i *)(v43 + 1));
  __m128i v48 = _mm_unpacklo_epi8(v47, _mm_setzero_si128());
  __m128i v49 = _mm_slli_epi16(v48, 4);
  __m128i v44 = _mm_loadl_epi64((__m128i *)(v43 + 0));
  __m128i v45 = _mm_unpacklo_epi8(v44, _mm_setzero_si128());
  __m128i v46 = _mm_slli_epi16(v45, 4);
  __m128i v50 = _mm_sub_epi16(v46, v49);
  __m128i v51 = _mm_srai_epi16(v50, 2);
  __m128i v52 = _mm_add_epi16(v49, v51);
  __m128i v109 = _mm_sub_epi16(v42, v52);
  __m128i v110 = _mm_srai_epi16(v109, 2);
  __m128i v111 = _mm_add_epi16(v52, v110);
  __m128i v112 = _mm_srai_epi16(_mm_add_epi16(v111, _mm_set1_epi16(8)), 4);
  __m128i v113 = _mm_packus_epi16(v112, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v139 + 0), v113);

  {
  char unsigned * RADRESTRICT v141 = v139 + v3;
  char unsigned * RADRESTRICT v53 = v43 + v2;
  __m128i v57 = _mm_loadl_epi64((__m128i *)(v53 + 1));
  __m128i v58 = _mm_unpacklo_epi8(v57, _mm_setzero_si128());
  __m128i v59 = _mm_slli_epi16(v58, 4);
  __m128i v54 = _mm_loadl_epi64((__m128i *)(v53 + 0));
  __m128i v55 = _mm_unpacklo_epi8(v54, _mm_setzero_si128());
  __m128i v56 = _mm_slli_epi16(v55, 4);
  __m128i v60 = _mm_sub_epi16(v56, v59);
  __m128i v61 = _mm_srai_epi16(v60, 2);
  __m128i v62 = _mm_add_epi16(v59, v61);
  __m128i v114 = _mm_sub_epi16(v52, v62);
  __m128i v115 = _mm_srai_epi16(v114, 2);
  __m128i v116 = _mm_add_epi16(v62, v115);
  __m128i v117 = _mm_srai_epi16(_mm_add_epi16(v116, _mm_set1_epi16(8)), 4);
  __m128i v118 = _mm_packus_epi16(v117, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v141 + 0), v118);

  {
  char unsigned * RADRESTRICT v143 = v141 + v3;
  char unsigned * RADRESTRICT v63 = v53 + v2;
  __m128i v67 = _mm_loadl_epi64((__m128i *)(v63 + 1));
  __m128i v68 = _mm_unpacklo_epi8(v67, _mm_setzero_si128());
  __m128i v69 = _mm_slli_epi16(v68, 4);
  __m128i v64 = _mm_loadl_epi64((__m128i *)(v63 + 0));
  __m128i v65 = _mm_unpacklo_epi8(v64, _mm_setzero_si128());
  __m128i v66 = _mm_slli_epi16(v65, 4);
  __m128i v70 = _mm_sub_epi16(v66, v69);
  __m128i v71 = _mm_srai_epi16(v70, 2);
  __m128i v72 = _mm_add_epi16(v69, v71);
  __m128i v119 = _mm_sub_epi16(v62, v72);
  __m128i v120 = _mm_srai_epi16(v119, 2);
  __m128i v121 = _mm_add_epi16(v72, v120);
  __m128i v122 = _mm_srai_epi16(_mm_add_epi16(v121, _mm_set1_epi16(8)), 4);
  __m128i v123 = _mm_packus_epi16(v122, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v143 + 0), v123);

  {
  char unsigned * RADRESTRICT v145 = v143 + v3;
  char unsigned * RADRESTRICT v73 = v63 + v2;
  __m128i v77 = _mm_loadl_epi64((__m128i *)(v73 + 1));
  __m128i v78 = _mm_unpacklo_epi8(v77, _mm_setzero_si128());
  __m128i v79 = _mm_slli_epi16(v78, 4);
  __m128i v74 = _mm_loadl_epi64((__m128i *)(v73 + 0));
  __m128i v75 = _mm_unpacklo_epi8(v74, _mm_setzero_si128());
  __m128i v76 = _mm_slli_epi16(v75, 4);
  __m128i v80 = _mm_sub_epi16(v76, v79);
  __m128i v81 = _mm_srai_epi16(v80, 2);
  __m128i v82 = _mm_add_epi16(v79, v81);
  __m128i v124 = _mm_sub_epi16(v72, v82);
  __m128i v125 = _mm_srai_epi16(v124, 2);
  __m128i v126 = _mm_add_epi16(v82, v125);
  __m128i v127 = _mm_srai_epi16(_mm_add_epi16(v126, _mm_set1_epi16(8)), 4);
  __m128i v128 = _mm_packus_epi16(v127, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v145 + 0), v128);

  {
  char unsigned * RADRESTRICT v147 = v145 + v3;
  char unsigned * RADRESTRICT v83 = v73 + v2;
  __m128i v87 = _mm_loadl_epi64((__m128i *)(v83 + 1));
  __m128i v88 = _mm_unpacklo_epi8(v87, _mm_setzero_si128());
  __m128i v89 = _mm_slli_epi16(v88, 4);
  __m128i v84 = _mm_loadl_epi64((__m128i *)(v83 + 0));
  __m128i v85 = _mm_unpacklo_epi8(v84, _mm_setzero_si128());
  __m128i v86 = _mm_slli_epi16(v85, 4);
  __m128i v90 = _mm_sub_epi16(v86, v89);
  __m128i v91 = _mm_srai_epi16(v90, 2);
  __m128i v92 = _mm_add_epi16(v89, v91);
  __m128i v129 = _mm_sub_epi16(v82, v92);
  __m128i v130 = _mm_srai_epi16(v129, 2);
  __m128i v131 = _mm_add_epi16(v92, v130);
  __m128i v132 = _mm_srai_epi16(_mm_add_epi16(v131, _mm_set1_epi16(8)), 4);
  __m128i v133 = _mm_packus_epi16(v132, _mm_setzero_si128());
  _mm_storel_epi64((__m128i * RADRESTRICT)(v147 + 0), v133);
  }
  }
  }
  }
  }
  }
  }
  }
}

typedef void bilinear_8x8_jump(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int sw, int dw);
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

