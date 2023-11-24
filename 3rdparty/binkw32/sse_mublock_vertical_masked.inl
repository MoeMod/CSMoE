// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

static CODEGEN_ATTR void mublock_vertical_masked_2(char unsigned * RADRESTRICT px, int sw, char unsigned * RADRESTRICT mask_l, char unsigned * RADRESTRICT mask_r)
{
  {
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(px);
  __m128i v17 = _mm_load_si128((__m128i *)(v0 + 0));
  __m128i v21_0 = _mm_unpacklo_epi8(v17, _mm_setzero_si128());
  int v1 = sw;
  char unsigned * RADRESTRICT v2 = v0 + v1;
  char unsigned * RADRESTRICT v3 = v2 + v1;
  __m128i v19 = _mm_load_si128((__m128i *)(v3 + 0));
  __m128i v23_0 = _mm_unpacklo_epi8(v19, _mm_setzero_si128());
  __m128i v18 = _mm_load_si128((__m128i *)(v2 + 0));
  __m128i v22_0 = _mm_unpacklo_epi8(v18, _mm_setzero_si128());
  __m128i v33 = _mm_sub_epi16(v23_0, v22_0);
  __m128i v36 = _mm_slli_epi16(v33, 1);
  __m128i v39 = _mm_srai_epi16(_mm_add_epi16(v36, _mm_set1_epi16(8)), 4);
  char unsigned * RADRESTRICT v5 = (char unsigned * RADRESTRICT)(mask_l);
  __m128i v7 = _mm_load_si128((__m128i *)(v5 + 0));
  __m128i v9 = _mm_shuffle_epi32(_mm_unpacklo_epi16(v7, v7), _MM_SHUFFLE(0, 0, 0, 0));
  __m128i v40 = _mm_and_si128(v39, v9);
  __m128i v44 = _mm_add_epi16(v21_0, v40);
  __m128i v21_1 = _mm_unpackhi_epi8(v17, _mm_setzero_si128());
  __m128i v23_1 = _mm_unpackhi_epi8(v19, _mm_setzero_si128());
  __m128i v22_1 = _mm_unpackhi_epi8(v18, _mm_setzero_si128());
  __m128i v48 = _mm_sub_epi16(v23_1, v22_1);
  __m128i v51 = _mm_slli_epi16(v48, 1);
  __m128i v54 = _mm_srai_epi16(_mm_add_epi16(v51, _mm_set1_epi16(8)), 4);
  char unsigned * RADRESTRICT v6 = (char unsigned * RADRESTRICT)(mask_r);
  __m128i v8 = _mm_load_si128((__m128i *)(v6 + 0));
  __m128i v13 = _mm_shuffle_epi32(_mm_unpacklo_epi16(v8, v8), _MM_SHUFFLE(0, 0, 0, 0));
  __m128i v55 = _mm_and_si128(v54, v13);
  __m128i v59 = _mm_add_epi16(v21_1, v55);
  __m128i v64 = _mm_packus_epi16(v44, v59);
  _mm_store_si128((__m128i * RADRESTRICT)(v0 + 0), v64);

  {
  __m128i v35 = _mm_slli_epi16(v33, 2);
  __m128i v38 = _mm_srai_epi16(_mm_add_epi16(v35, _mm_set1_epi16(8)), 4);
  __m128i v10 = _mm_shuffle_epi32(_mm_unpacklo_epi16(v7, v7), _MM_SHUFFLE(1, 1, 1, 1));
  __m128i v41 = _mm_and_si128(v38, v10);
  __m128i v45 = _mm_add_epi16(v22_0, v41);
  __m128i v50 = _mm_slli_epi16(v48, 2);
  __m128i v53 = _mm_srai_epi16(_mm_add_epi16(v50, _mm_set1_epi16(8)), 4);
  __m128i v14 = _mm_shuffle_epi32(_mm_unpacklo_epi16(v8, v8), _MM_SHUFFLE(1, 1, 1, 1));
  __m128i v56 = _mm_and_si128(v53, v14);
  __m128i v60 = _mm_add_epi16(v22_1, v56);
  __m128i v66 = _mm_packus_epi16(v45, v60);
  _mm_store_si128((__m128i * RADRESTRICT)(v2 + 0), v66);

  {
  __m128i v11 = _mm_shuffle_epi32(_mm_unpacklo_epi16(v7, v7), _MM_SHUFFLE(2, 2, 2, 2));
  __m128i v42 = _mm_and_si128(v38, v11);
  __m128i v46 = _mm_sub_epi16(v23_0, v42);
  __m128i v15 = _mm_shuffle_epi32(_mm_unpacklo_epi16(v8, v8), _MM_SHUFFLE(2, 2, 2, 2));
  __m128i v57 = _mm_and_si128(v53, v15);
  __m128i v61 = _mm_sub_epi16(v23_1, v57);
  __m128i v68 = _mm_packus_epi16(v46, v61);
  _mm_store_si128((__m128i * RADRESTRICT)(v3 + 0), v68);

  {
  char unsigned * RADRESTRICT v4 = v3 + v1;
  __m128i v20 = _mm_load_si128((__m128i *)(v4 + 0));
  __m128i v24_0 = _mm_unpacklo_epi8(v20, _mm_setzero_si128());
  __m128i v12 = _mm_shuffle_epi32(_mm_unpacklo_epi16(v7, v7), _MM_SHUFFLE(3, 3, 3, 3));
  __m128i v43 = _mm_and_si128(v39, v12);
  __m128i v47 = _mm_sub_epi16(v24_0, v43);
  __m128i v24_1 = _mm_unpackhi_epi8(v20, _mm_setzero_si128());
  __m128i v16 = _mm_shuffle_epi32(_mm_unpacklo_epi16(v8, v8), _MM_SHUFFLE(3, 3, 3, 3));
  __m128i v58 = _mm_and_si128(v54, v16);
  __m128i v62 = _mm_sub_epi16(v24_1, v58);
  __m128i v70 = _mm_packus_epi16(v47, v62);
  _mm_store_si128((__m128i * RADRESTRICT)(v4 + 0), v70);
  }
  }
  }
  }
}

