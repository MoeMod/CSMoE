// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

static CODEGEN_ATTR void ssse3_muratori32_16x16_h0_v0(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int sw, int dw)
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

static CODEGEN_ATTR void ssse3_muratori32_16x16_h0_v1(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int sw, int dw)
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
  char unsigned * RADRESTRICT v11 = v5 + v2;
  char unsigned * RADRESTRICT v13 = v11 + v2;
  char unsigned * RADRESTRICT v15 = v13 + v2;
  char unsigned * RADRESTRICT v17 = v15 + v2;
  char unsigned * RADRESTRICT v19 = v17 + v2;
  __m128i v20 = _mm_loadu_si128((__m128i *)(v19 + 0));
  __m128i v21_0 = _mm_maddubs_epi16(_mm_unpacklo_epi8(v10, v20), _mm_set1_epi8(1));
  __m128i v22_0 = _mm_srai_epi16(v21_0, 1);
  __m128i v14 = _mm_loadu_si128((__m128i *)(v13 + 0));
  __m128i v16 = _mm_loadu_si128((__m128i *)(v15 + 0));
  __m128i v24_0 = _mm_maddubs_epi16(_mm_unpacklo_epi8(v14, v16), _mm_set1_epi8(19));
  __m128i v25_0 = _mm_srai_epi16(v24_0, 1);
  __m128i v26_0 = _mm_add_epi16(v22_0, v25_0);
  __m128i v12 = _mm_loadu_si128((__m128i *)(v11 + 0));
  __m128i v18 = _mm_loadu_si128((__m128i *)(v17 + 0));
  __m128i v23_0 = _mm_maddubs_epi16(_mm_unpacklo_epi8(v12, v18), _mm_set1_epi8(2));
  __m128i v27_0 = _mm_sub_epi16(v26_0, v23_0);
  __m128i v28_0 = _mm_srai_epi16(_mm_add_epi16(v27_0, _mm_set1_epi16(8)), 4);
  __m128i v21_1 = _mm_maddubs_epi16(_mm_unpackhi_epi8(v10, v20), _mm_set1_epi8(1));
  __m128i v22_1 = _mm_srai_epi16(v21_1, 1);
  __m128i v24_1 = _mm_maddubs_epi16(_mm_unpackhi_epi8(v14, v16), _mm_set1_epi8(19));
  __m128i v25_1 = _mm_srai_epi16(v24_1, 1);
  __m128i v26_1 = _mm_add_epi16(v22_1, v25_1);
  __m128i v23_1 = _mm_maddubs_epi16(_mm_unpackhi_epi8(v12, v18), _mm_set1_epi8(2));
  __m128i v27_1 = _mm_sub_epi16(v26_1, v23_1);
  __m128i v28_1 = _mm_srai_epi16(_mm_add_epi16(v27_1, _mm_set1_epi16(8)), 4);
  __m128i v29 = _mm_packus_epi16(v28_0, v28_1);
  _mm_store_si128((__m128i * RADRESTRICT)(v1 + 0), v29);

  {
  char unsigned * RADRESTRICT v31 = v5 + v2;
  v5 = v31;

  {
  char unsigned * RADRESTRICT v33 = v1 + v3;
  v1 = v33;
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

static CODEGEN_ATTR void ssse3_muratori32_16x16_h1_v0(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int sw, int dw)
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
  __m128i v14 = _mm_loadu_si128((__m128i *)(v4 + 5));
  __m128i v15_0 = _mm_maddubs_epi16(_mm_unpacklo_epi8(v9, v14), _mm_set1_epi8(1));
  __m128i v16_0 = _mm_srai_epi16(v15_0, 1);
  __m128i v11 = _mm_loadu_si128((__m128i *)(v4 + 2));
  __m128i v12 = _mm_loadu_si128((__m128i *)(v4 + 3));
  __m128i v18_0 = _mm_maddubs_epi16(_mm_unpacklo_epi8(v11, v12), _mm_set1_epi8(19));
  __m128i v19_0 = _mm_srai_epi16(v18_0, 1);
  __m128i v20_0 = _mm_add_epi16(v16_0, v19_0);
  __m128i v10 = _mm_loadu_si128((__m128i *)(v4 + 1));
  __m128i v13 = _mm_loadu_si128((__m128i *)(v4 + 4));
  __m128i v17_0 = _mm_maddubs_epi16(_mm_unpacklo_epi8(v10, v13), _mm_set1_epi8(2));
  __m128i v21_0 = _mm_sub_epi16(v20_0, v17_0);
  __m128i v22_0 = _mm_srai_epi16(_mm_add_epi16(v21_0, _mm_set1_epi16(8)), 4);
  __m128i v15_1 = _mm_maddubs_epi16(_mm_unpackhi_epi8(v9, v14), _mm_set1_epi8(1));
  __m128i v16_1 = _mm_srai_epi16(v15_1, 1);
  __m128i v18_1 = _mm_maddubs_epi16(_mm_unpackhi_epi8(v11, v12), _mm_set1_epi8(19));
  __m128i v19_1 = _mm_srai_epi16(v18_1, 1);
  __m128i v20_1 = _mm_add_epi16(v16_1, v19_1);
  __m128i v17_1 = _mm_maddubs_epi16(_mm_unpackhi_epi8(v10, v13), _mm_set1_epi8(2));
  __m128i v21_1 = _mm_sub_epi16(v20_1, v17_1);
  __m128i v22_1 = _mm_srai_epi16(_mm_add_epi16(v21_1, _mm_set1_epi16(8)), 4);
  __m128i v23 = _mm_packus_epi16(v22_0, v22_1);
  _mm_store_si128((__m128i * RADRESTRICT)(v1 + 0), v23);

  {
  char unsigned * RADRESTRICT v25 = v1 + v3;
  v1 = v25;

  {
  char unsigned * RADRESTRICT v27 = v4 + v2;
  v4 = v27;
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

static CODEGEN_ATTR void ssse3_muratori32_16x16_h1_v1(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int sw, int dw)
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
  __m128i v18 = _mm_loadu_si128((__m128i *)(v8 + 5));
  __m128i v19_0 = _mm_maddubs_epi16(_mm_unpacklo_epi8(v13, v18), _mm_set1_epi8(1));
  __m128i v20_0 = _mm_srai_epi16(v19_0, 1);
  __m128i v15 = _mm_loadu_si128((__m128i *)(v8 + 2));
  __m128i v16 = _mm_loadu_si128((__m128i *)(v8 + 3));
  __m128i v22_0 = _mm_maddubs_epi16(_mm_unpacklo_epi8(v15, v16), _mm_set1_epi8(19));
  __m128i v23_0 = _mm_srai_epi16(v22_0, 1);
  __m128i v24_0 = _mm_add_epi16(v20_0, v23_0);
  __m128i v14 = _mm_loadu_si128((__m128i *)(v8 + 1));
  __m128i v17 = _mm_loadu_si128((__m128i *)(v8 + 4));
  __m128i v21_0 = _mm_maddubs_epi16(_mm_unpacklo_epi8(v14, v17), _mm_set1_epi8(2));
  __m128i v25_0 = _mm_sub_epi16(v24_0, v21_0);
  __m128i v26_0 = _mm_srai_epi16(_mm_add_epi16(v25_0, _mm_set1_epi16(8)), 4);
  __m128i v19_1 = _mm_maddubs_epi16(_mm_unpackhi_epi8(v13, v18), _mm_set1_epi8(1));
  __m128i v20_1 = _mm_srai_epi16(v19_1, 1);
  __m128i v22_1 = _mm_maddubs_epi16(_mm_unpackhi_epi8(v15, v16), _mm_set1_epi8(19));
  __m128i v23_1 = _mm_srai_epi16(v22_1, 1);
  __m128i v24_1 = _mm_add_epi16(v20_1, v23_1);
  __m128i v21_1 = _mm_maddubs_epi16(_mm_unpackhi_epi8(v14, v17), _mm_set1_epi8(2));
  __m128i v25_1 = _mm_sub_epi16(v24_1, v21_1);
  __m128i v26_1 = _mm_srai_epi16(_mm_add_epi16(v25_1, _mm_set1_epi16(8)), 4);
  v6_0[v9] = v26_0;
  v6_1[v9] = v26_1;

  {
  char unsigned * RADRESTRICT v28 = v8 + v2;
  v8 = v28;
  }
  }
}

  {
{int v30;
for(v30 = 0; v30 < 16; v30 += 1)
  {
  int v34 = v30 + 0;
  __m128i v35_0 = v6_0[v34];
  int v44 = v30 + 5;
  __m128i v45_0 = v6_0[v44];
  __m128i v46_0 = _mm_add_epi16(v35_0, v45_0);
  __m128i v47_0 = _mm_srai_epi16(v46_0, 1);
  int v38 = v30 + 2;
  __m128i v39_0 = v6_0[v38];
  int v40 = v30 + 3;
  __m128i v41_0 = v6_0[v40];
  __m128i v50_0 = _mm_add_epi16(v39_0, v41_0);
  __m128i v51_0 = _mm_mullo_epi16(v50_0, _mm_set1_epi16(19));
  __m128i v52_0 = _mm_srai_epi16(v51_0, 1);
  __m128i v53_0 = _mm_add_epi16(v47_0, v52_0);
  int v36 = v30 + 1;
  __m128i v37_0 = v6_0[v36];
  int v42 = v30 + 4;
  __m128i v43_0 = v6_0[v42];
  __m128i v48_0 = _mm_add_epi16(v37_0, v43_0);
  __m128i v49_0 = _mm_slli_epi16(v48_0, 1);
  __m128i v54_0 = _mm_sub_epi16(v53_0, v49_0);
  __m128i v55_0 = _mm_srai_epi16(_mm_add_epi16(v54_0, _mm_set1_epi16(8)), 4);
  __m128i v35_1 = v6_1[v34];
  __m128i v45_1 = v6_1[v44];
  __m128i v46_1 = _mm_add_epi16(v35_1, v45_1);
  __m128i v47_1 = _mm_srai_epi16(v46_1, 1);
  __m128i v39_1 = v6_1[v38];
  __m128i v41_1 = v6_1[v40];
  __m128i v50_1 = _mm_add_epi16(v39_1, v41_1);
  __m128i v51_1 = _mm_mullo_epi16(v50_1, _mm_set1_epi16(19));
  __m128i v52_1 = _mm_srai_epi16(v51_1, 1);
  __m128i v53_1 = _mm_add_epi16(v47_1, v52_1);
  __m128i v37_1 = v6_1[v36];
  __m128i v43_1 = v6_1[v42];
  __m128i v48_1 = _mm_add_epi16(v37_1, v43_1);
  __m128i v49_1 = _mm_slli_epi16(v48_1, 1);
  __m128i v54_1 = _mm_sub_epi16(v53_1, v49_1);
  __m128i v55_1 = _mm_srai_epi16(_mm_add_epi16(v54_1, _mm_set1_epi16(8)), 4);
  __m128i v56 = _mm_packus_epi16(v55_0, v55_1);
  _mm_store_si128((__m128i * RADRESTRICT)(v1 + 0), v56);

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

typedef void ssse3_muratori32_16x16_jump(char unsigned * RADRESTRICT src, char unsigned * RADRESTRICT dest, int sw, int dw);
#if !defined(BINK2_CODEGEN_NO_TABLE)
#ifdef _MSC_VER
_declspec(selectany)
#else
static
#endif
ssse3_muratori32_16x16_jump *ssse3_muratori32_16x16[] =
{
  ssse3_muratori32_16x16_h0_v0,
  ssse3_muratori32_16x16_h0_v1,
  ssse3_muratori32_16x16_h1_v0,
  ssse3_muratori32_16x16_h1_v1,
};
#endif

