// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

static CODEGEN_ATTR void sse2_muratori32_16x16_h0_v0(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int sw, int dw)
{
  {
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(src);

  {
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dest);

  {
  int v2 = sw;

  {
  int v3 = dw;

  {
{int v4;
for(v4 = 0; v4 < 16; v4 += 1)
  {
  __m128i v8 = _mm_loadu_si128((__m128i *)(v0 + 0));
  _mm_store_si128((__m128i * RADRESTRICT)(v1 + 0), v8);

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

static CODEGEN_ATTR void sse2_muratori32_16x16_h0_v1(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int sw, int dw)
{
  {
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(src);
  int v2 = sw;
  int v4 = v2 * -2;
  char unsigned * RADRESTRICT v5 = v0 + v4;

  {
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dest);

  {

  {
  int v3 = dw;

  {
{int v6;
for(v6 = 0; v6 < 16; v6 += 1)
  {
  __m128i v10 = _mm_loadu_si128((__m128i *)(v5 + 0));
  __m128i v21_0 = _mm_unpacklo_epi8(v10, _mm_setzero_si128());
  char unsigned * RADRESTRICT v11 = v5 + v2;
  char unsigned * RADRESTRICT v13 = v11 + v2;
  char unsigned * RADRESTRICT v15 = v13 + v2;
  char unsigned * RADRESTRICT v17 = v15 + v2;
  char unsigned * RADRESTRICT v19 = v17 + v2;
  __m128i v20 = _mm_loadu_si128((__m128i *)(v19 + 0));
  __m128i v22_0 = _mm_unpacklo_epi8(v20, _mm_setzero_si128());
  __m128i v23_0 = _mm_add_epi16(v21_0, v22_0);
  __m128i v24_0 = _mm_srai_epi16(v23_0, 1);
  __m128i v14 = _mm_loadu_si128((__m128i *)(v13 + 0));
  __m128i v29_0 = _mm_unpacklo_epi8(v14, _mm_setzero_si128());
  __m128i v16 = _mm_loadu_si128((__m128i *)(v15 + 0));
  __m128i v30_0 = _mm_unpacklo_epi8(v16, _mm_setzero_si128());
  __m128i v31_0 = _mm_add_epi16(v29_0, v30_0);
  __m128i v32_0 = _mm_mullo_epi16(v31_0, _mm_set1_epi16(19));
  __m128i v33_0 = _mm_srai_epi16(v32_0, 1);
  __m128i v34_0 = _mm_add_epi16(v24_0, v33_0);
  __m128i v12 = _mm_loadu_si128((__m128i *)(v11 + 0));
  __m128i v25_0 = _mm_unpacklo_epi8(v12, _mm_setzero_si128());
  __m128i v18 = _mm_loadu_si128((__m128i *)(v17 + 0));
  __m128i v26_0 = _mm_unpacklo_epi8(v18, _mm_setzero_si128());
  __m128i v27_0 = _mm_add_epi16(v25_0, v26_0);
  __m128i v28_0 = _mm_slli_epi16(v27_0, 1);
  __m128i v35_0 = _mm_sub_epi16(v34_0, v28_0);
  __m128i v36_0 = _mm_srai_epi16(_mm_add_epi16(v35_0, _mm_set1_epi16(8)), 4);
  __m128i v21_1 = _mm_unpackhi_epi8(v10, _mm_setzero_si128());
  __m128i v22_1 = _mm_unpackhi_epi8(v20, _mm_setzero_si128());
  __m128i v23_1 = _mm_add_epi16(v21_1, v22_1);
  __m128i v24_1 = _mm_srai_epi16(v23_1, 1);
  __m128i v29_1 = _mm_unpackhi_epi8(v14, _mm_setzero_si128());
  __m128i v30_1 = _mm_unpackhi_epi8(v16, _mm_setzero_si128());
  __m128i v31_1 = _mm_add_epi16(v29_1, v30_1);
  __m128i v32_1 = _mm_mullo_epi16(v31_1, _mm_set1_epi16(19));
  __m128i v33_1 = _mm_srai_epi16(v32_1, 1);
  __m128i v34_1 = _mm_add_epi16(v24_1, v33_1);
  __m128i v25_1 = _mm_unpackhi_epi8(v12, _mm_setzero_si128());
  __m128i v26_1 = _mm_unpackhi_epi8(v18, _mm_setzero_si128());
  __m128i v27_1 = _mm_add_epi16(v25_1, v26_1);
  __m128i v28_1 = _mm_slli_epi16(v27_1, 1);
  __m128i v35_1 = _mm_sub_epi16(v34_1, v28_1);
  __m128i v36_1 = _mm_srai_epi16(_mm_add_epi16(v35_1, _mm_set1_epi16(8)), 4);
  __m128i v37 = _mm_packus_epi16(v36_0, v36_1);
  _mm_store_si128((__m128i * RADRESTRICT)(v1 + 0), v37);

  {
  char unsigned * RADRESTRICT v39 = v5 + v2;
  v5 = v39;

  {
  char unsigned * RADRESTRICT v41 = v1 + v3;
  v1 = v41;
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

static CODEGEN_ATTR void sse2_muratori32_16x16_h1_v0(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int sw, int dw)
{
  {
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(src);
  char unsigned * RADRESTRICT v4 = v0 + -2;

  {
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dest);

  {
  int v2 = sw;

  {
  int v3 = dw;

  {
{int v5;
for(v5 = 0; v5 < 16; v5 += 1)
  {
  __m128i v9 = _mm_loadu_si128((__m128i *)(v4 + 0));
  __m128i v15_0 = _mm_unpacklo_epi8(v9, _mm_setzero_si128());
  __m128i v14 = _mm_loadu_si128((__m128i *)(v4 + 5));
  __m128i v16_0 = _mm_unpacklo_epi8(v14, _mm_setzero_si128());
  __m128i v17_0 = _mm_add_epi16(v15_0, v16_0);
  __m128i v18_0 = _mm_srai_epi16(v17_0, 1);
  __m128i v11 = _mm_loadu_si128((__m128i *)(v4 + 2));
  __m128i v23_0 = _mm_unpacklo_epi8(v11, _mm_setzero_si128());
  __m128i v12 = _mm_loadu_si128((__m128i *)(v4 + 3));
  __m128i v24_0 = _mm_unpacklo_epi8(v12, _mm_setzero_si128());
  __m128i v25_0 = _mm_add_epi16(v23_0, v24_0);
  __m128i v26_0 = _mm_mullo_epi16(v25_0, _mm_set1_epi16(19));
  __m128i v27_0 = _mm_srai_epi16(v26_0, 1);
  __m128i v28_0 = _mm_add_epi16(v18_0, v27_0);
  __m128i v10 = _mm_loadu_si128((__m128i *)(v4 + 1));
  __m128i v19_0 = _mm_unpacklo_epi8(v10, _mm_setzero_si128());
  __m128i v13 = _mm_loadu_si128((__m128i *)(v4 + 4));
  __m128i v20_0 = _mm_unpacklo_epi8(v13, _mm_setzero_si128());
  __m128i v21_0 = _mm_add_epi16(v19_0, v20_0);
  __m128i v22_0 = _mm_slli_epi16(v21_0, 1);
  __m128i v29_0 = _mm_sub_epi16(v28_0, v22_0);
  __m128i v30_0 = _mm_srai_epi16(_mm_add_epi16(v29_0, _mm_set1_epi16(8)), 4);
  __m128i v15_1 = _mm_unpackhi_epi8(v9, _mm_setzero_si128());
  __m128i v16_1 = _mm_unpackhi_epi8(v14, _mm_setzero_si128());
  __m128i v17_1 = _mm_add_epi16(v15_1, v16_1);
  __m128i v18_1 = _mm_srai_epi16(v17_1, 1);
  __m128i v23_1 = _mm_unpackhi_epi8(v11, _mm_setzero_si128());
  __m128i v24_1 = _mm_unpackhi_epi8(v12, _mm_setzero_si128());
  __m128i v25_1 = _mm_add_epi16(v23_1, v24_1);
  __m128i v26_1 = _mm_mullo_epi16(v25_1, _mm_set1_epi16(19));
  __m128i v27_1 = _mm_srai_epi16(v26_1, 1);
  __m128i v28_1 = _mm_add_epi16(v18_1, v27_1);
  __m128i v19_1 = _mm_unpackhi_epi8(v10, _mm_setzero_si128());
  __m128i v20_1 = _mm_unpackhi_epi8(v13, _mm_setzero_si128());
  __m128i v21_1 = _mm_add_epi16(v19_1, v20_1);
  __m128i v22_1 = _mm_slli_epi16(v21_1, 1);
  __m128i v29_1 = _mm_sub_epi16(v28_1, v22_1);
  __m128i v30_1 = _mm_srai_epi16(_mm_add_epi16(v29_1, _mm_set1_epi16(8)), 4);
  __m128i v31 = _mm_packus_epi16(v30_0, v30_1);
  _mm_store_si128((__m128i * RADRESTRICT)(v1 + 0), v31);

  {
  char unsigned * RADRESTRICT v33 = v1 + v3;
  v1 = v33;

  {
  char unsigned * RADRESTRICT v35 = v4 + v2;
  v4 = v35;
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

static CODEGEN_ATTR void sse2_muratori32_16x16_h1_v1(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int sw, int dw)
{
  {
  __m128i v6_0[21];
  __m128i v6_1[21];

  {
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(src);
  int v2 = sw;
  int v4 = v2 * -2;
  char unsigned * RADRESTRICT v5 = v0 + v4;
  char unsigned * RADRESTRICT v8 = v5 + -2;

  {
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(dest);

  {

  {
  int v3 = dw;

  {
{int v9;
for(v9 = 0; v9 < 21; v9 += 1)
  {
  __m128i v13 = _mm_loadu_si128((__m128i *)(v8 + 0));
  __m128i v19_0 = _mm_unpacklo_epi8(v13, _mm_setzero_si128());
  __m128i v18 = _mm_loadu_si128((__m128i *)(v8 + 5));
  __m128i v20_0 = _mm_unpacklo_epi8(v18, _mm_setzero_si128());
  __m128i v21_0 = _mm_add_epi16(v19_0, v20_0);
  __m128i v22_0 = _mm_srai_epi16(v21_0, 1);
  __m128i v15 = _mm_loadu_si128((__m128i *)(v8 + 2));
  __m128i v27_0 = _mm_unpacklo_epi8(v15, _mm_setzero_si128());
  __m128i v16 = _mm_loadu_si128((__m128i *)(v8 + 3));
  __m128i v28_0 = _mm_unpacklo_epi8(v16, _mm_setzero_si128());
  __m128i v29_0 = _mm_add_epi16(v27_0, v28_0);
  __m128i v30_0 = _mm_mullo_epi16(v29_0, _mm_set1_epi16(19));
  __m128i v31_0 = _mm_srai_epi16(v30_0, 1);
  __m128i v32_0 = _mm_add_epi16(v22_0, v31_0);
  __m128i v14 = _mm_loadu_si128((__m128i *)(v8 + 1));
  __m128i v23_0 = _mm_unpacklo_epi8(v14, _mm_setzero_si128());
  __m128i v17 = _mm_loadu_si128((__m128i *)(v8 + 4));
  __m128i v24_0 = _mm_unpacklo_epi8(v17, _mm_setzero_si128());
  __m128i v25_0 = _mm_add_epi16(v23_0, v24_0);
  __m128i v26_0 = _mm_slli_epi16(v25_0, 1);
  __m128i v33_0 = _mm_sub_epi16(v32_0, v26_0);
  __m128i v34_0 = _mm_srai_epi16(_mm_add_epi16(v33_0, _mm_set1_epi16(8)), 4);
  __m128i v19_1 = _mm_unpackhi_epi8(v13, _mm_setzero_si128());
  __m128i v20_1 = _mm_unpackhi_epi8(v18, _mm_setzero_si128());
  __m128i v21_1 = _mm_add_epi16(v19_1, v20_1);
  __m128i v22_1 = _mm_srai_epi16(v21_1, 1);
  __m128i v27_1 = _mm_unpackhi_epi8(v15, _mm_setzero_si128());
  __m128i v28_1 = _mm_unpackhi_epi8(v16, _mm_setzero_si128());
  __m128i v29_1 = _mm_add_epi16(v27_1, v28_1);
  __m128i v30_1 = _mm_mullo_epi16(v29_1, _mm_set1_epi16(19));
  __m128i v31_1 = _mm_srai_epi16(v30_1, 1);
  __m128i v32_1 = _mm_add_epi16(v22_1, v31_1);
  __m128i v23_1 = _mm_unpackhi_epi8(v14, _mm_setzero_si128());
  __m128i v24_1 = _mm_unpackhi_epi8(v17, _mm_setzero_si128());
  __m128i v25_1 = _mm_add_epi16(v23_1, v24_1);
  __m128i v26_1 = _mm_slli_epi16(v25_1, 1);
  __m128i v33_1 = _mm_sub_epi16(v32_1, v26_1);
  __m128i v34_1 = _mm_srai_epi16(_mm_add_epi16(v33_1, _mm_set1_epi16(8)), 4);
  v6_0[v9] = v34_0;
  v6_1[v9] = v34_1;

  {
  char unsigned * RADRESTRICT v36 = v8 + v2;
  v8 = v36;
  }
  }
}

  {
{int v38;
for(v38 = 0; v38 < 16; v38 += 1)
  {
  int v42 = v38 + 0;
  __m128i v43_0 = v6_0[v42];
  int v52 = v38 + 5;
  __m128i v53_0 = v6_0[v52];
  __m128i v54_0 = _mm_add_epi16(v43_0, v53_0);
  __m128i v55_0 = _mm_srai_epi16(v54_0, 1);
  int v46 = v38 + 2;
  __m128i v47_0 = v6_0[v46];
  int v48 = v38 + 3;
  __m128i v49_0 = v6_0[v48];
  __m128i v58_0 = _mm_add_epi16(v47_0, v49_0);
  __m128i v59_0 = _mm_mullo_epi16(v58_0, _mm_set1_epi16(19));
  __m128i v60_0 = _mm_srai_epi16(v59_0, 1);
  __m128i v61_0 = _mm_add_epi16(v55_0, v60_0);
  int v44 = v38 + 1;
  __m128i v45_0 = v6_0[v44];
  int v50 = v38 + 4;
  __m128i v51_0 = v6_0[v50];
  __m128i v56_0 = _mm_add_epi16(v45_0, v51_0);
  __m128i v57_0 = _mm_slli_epi16(v56_0, 1);
  __m128i v62_0 = _mm_sub_epi16(v61_0, v57_0);
  __m128i v63_0 = _mm_srai_epi16(_mm_add_epi16(v62_0, _mm_set1_epi16(8)), 4);
  __m128i v43_1 = v6_1[v42];
  __m128i v53_1 = v6_1[v52];
  __m128i v54_1 = _mm_add_epi16(v43_1, v53_1);
  __m128i v55_1 = _mm_srai_epi16(v54_1, 1);
  __m128i v47_1 = v6_1[v46];
  __m128i v49_1 = v6_1[v48];
  __m128i v58_1 = _mm_add_epi16(v47_1, v49_1);
  __m128i v59_1 = _mm_mullo_epi16(v58_1, _mm_set1_epi16(19));
  __m128i v60_1 = _mm_srai_epi16(v59_1, 1);
  __m128i v61_1 = _mm_add_epi16(v55_1, v60_1);
  __m128i v45_1 = v6_1[v44];
  __m128i v51_1 = v6_1[v50];
  __m128i v56_1 = _mm_add_epi16(v45_1, v51_1);
  __m128i v57_1 = _mm_slli_epi16(v56_1, 1);
  __m128i v62_1 = _mm_sub_epi16(v61_1, v57_1);
  __m128i v63_1 = _mm_srai_epi16(_mm_add_epi16(v62_1, _mm_set1_epi16(8)), 4);
  __m128i v64 = _mm_packus_epi16(v63_0, v63_1);
  _mm_store_si128((__m128i * RADRESTRICT)(v1 + 0), v64);

  {
  char unsigned * RADRESTRICT v66 = v8 + v2;
  v8 = v66;

  {
  char unsigned * RADRESTRICT v68 = v1 + v3;
  v1 = v68;
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

typedef void sse2_muratori32_16x16_jump(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int sw, int dw);
#if !defined(BINK2_CODEGEN_NO_TABLE)
#ifdef _MSC_VER
_declspec(selectany)
#else
static
#endif
sse2_muratori32_16x16_jump *sse2_muratori32_16x16[] =
{
  sse2_muratori32_16x16_h0_v0,
  sse2_muratori32_16x16_h0_v1,
  sse2_muratori32_16x16_h1_v0,
  sse2_muratori32_16x16_h1_v1,
};
#endif

