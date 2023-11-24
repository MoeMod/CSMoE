// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

static CODEGEN_ATTR void mublock16_UtoD(char unsigned * RADRESTRICT pixels, int stride, char unsigned * RADRESTRICT mask0, char unsigned * RADRESTRICT mask1, char unsigned * RADRESTRICT mask2, char unsigned * RADRESTRICT mask3)
{
  {
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(pixels);
  int v1 = stride;
  __m128i v10_0_lo = _mm_loadl_epi64((__m128i const *) (v0 + 0*v1));
  __m128i v10_0_hi = _mm_loadl_epi64((__m128i const *) (v0 + 4*v1));
  __m128i v10_0 = _mm_unpacklo_epi16(v10_0_lo, v10_0_hi);
  __m128i v10_2_lo = _mm_loadl_epi64((__m128i const *) (v0 + 2*v1));
  __m128i v10_2_hi = _mm_loadl_epi64((__m128i const *) (v0 + 6*v1));
  __m128i v10_2 = _mm_unpacklo_epi16(v10_2_lo, v10_2_hi);
  __m128i v11_0 = _mm_unpacklo_epi16(v10_0, v10_2);
  __m128i v10_1_lo = _mm_loadl_epi64((__m128i const *) (v0 + 1*v1));
  __m128i v10_1_hi = _mm_loadl_epi64((__m128i const *) (v0 + 5*v1));
  __m128i v10_1 = _mm_unpacklo_epi16(v10_1_lo, v10_1_hi);
  __m128i v10_3_lo = _mm_loadl_epi64((__m128i const *) (v0 + 3*v1));
  __m128i v10_3_hi = _mm_loadl_epi64((__m128i const *) (v0 + 7*v1));
  __m128i v10_3 = _mm_unpacklo_epi16(v10_3_lo, v10_3_hi);
  __m128i v11_2 = _mm_unpacklo_epi16(v10_1, v10_3);
  __m128i v12_0 = _mm_unpacklo_epi16(v11_0, v11_2);
  __m128i v11_1 = _mm_unpackhi_epi16(v10_0, v10_2);
  __m128i v11_3 = _mm_unpackhi_epi16(v10_1, v10_3);
  __m128i v12_2 = _mm_unpacklo_epi16(v11_1, v11_3);
  __m128i v12_1 = _mm_unpackhi_epi16(v11_0, v11_2);
  __m128i v17 = _mm_sub_epi16(v12_2, v12_1);
  __m128i v20 = _mm_add_epi16(v17, _mm_set1_epi16(4));
  __m128i v21 = _mm_srai_epi16(v20, 3);
  char unsigned * RADRESTRICT v2 = (char unsigned * RADRESTRICT)(mask0);
  __m128i v6 = _mm_load_si128((__m128i *)(v2 + 0));
  __m128i v22 = _mm_and_si128(v21, v6);
  __m128i v23 = _mm_add_epi16(v12_0, v22);
  __m128i v18 = _mm_add_epi16(v17, _mm_set1_epi16(2));
  __m128i v19 = _mm_srai_epi16(v18, 2);
  char unsigned * RADRESTRICT v3 = (char unsigned * RADRESTRICT)(mask1);
  __m128i v7 = _mm_load_si128((__m128i *)(v3 + 0));
  __m128i v24 = _mm_and_si128(v19, v7);
  __m128i v25 = _mm_add_epi16(v12_1, v24);
  char unsigned * RADRESTRICT v4 = (char unsigned * RADRESTRICT)(mask2);
  __m128i v8 = _mm_load_si128((__m128i *)(v4 + 0));
  __m128i v26 = _mm_and_si128(v19, v8);
  __m128i v27 = _mm_sub_epi16(v12_2, v26);
  __m128i v12_3 = _mm_unpackhi_epi16(v11_1, v11_3);
  char unsigned * RADRESTRICT v5 = (char unsigned * RADRESTRICT)(mask3);
  __m128i v9 = _mm_load_si128((__m128i *)(v5 + 0));
  __m128i v28 = _mm_and_si128(v21, v9);
  __m128i v29 = _mm_sub_epi16(v12_3, v28);
  char unsigned * RADRESTRICT v33_row1 = v0 + v1;
  char unsigned * RADRESTRICT v33_row2 = v33_row1 + v1;
  char unsigned * RADRESTRICT v33_row3 = v33_row2 + v1;
  char unsigned * RADRESTRICT v33_row4 = v33_row3 + v1;
  char unsigned * RADRESTRICT v33_row5 = v33_row4 + v1;
  char unsigned * RADRESTRICT v33_row6 = v33_row5 + v1;
  char unsigned * RADRESTRICT v33_row7 = v33_row6 + v1;
  __m128i v33_i0 = _mm_unpacklo_epi16(v23, v25);
  __m128i v33_i1 = _mm_unpackhi_epi16(v23, v25);
  __m128i v33_i2 = _mm_unpacklo_epi16(v27, v29);
  __m128i v33_i3 = _mm_unpackhi_epi16(v27, v29);
  *((int *) (v0 + 0*4)) = _mm_cvtsi128_si32(v33_i0);
  *((int *) (v0 + 1*4)) = _mm_cvtsi128_si32(v33_i2);
  *((int *) (v33_row1 + 0*4)) = _mm_cvtsi128_si32(_mm_shuffle_epi32(v33_i0, _MM_SHUFFLE(1, 1, 1, 1)));
  *((int *) (v33_row1 + 1*4)) = _mm_cvtsi128_si32(_mm_shuffle_epi32(v33_i2, _MM_SHUFFLE(1, 1, 1, 1)));
  *((int *) (v33_row2 + 0*4)) = _mm_cvtsi128_si32(_mm_shuffle_epi32(v33_i0, _MM_SHUFFLE(2, 2, 2, 2)));
  *((int *) (v33_row2 + 1*4)) = _mm_cvtsi128_si32(_mm_shuffle_epi32(v33_i2, _MM_SHUFFLE(2, 2, 2, 2)));
  *((int *) (v33_row3 + 0*4)) = _mm_cvtsi128_si32(_mm_shuffle_epi32(v33_i0, _MM_SHUFFLE(3, 3, 3, 3)));
  *((int *) (v33_row3 + 1*4)) = _mm_cvtsi128_si32(_mm_shuffle_epi32(v33_i2, _MM_SHUFFLE(3, 3, 3, 3)));
  *((int *) (v33_row4 + 0*4)) = _mm_cvtsi128_si32(v33_i1);
  *((int *) (v33_row4 + 1*4)) = _mm_cvtsi128_si32(v33_i3);
  *((int *) (v33_row5 + 0*4)) = _mm_cvtsi128_si32(_mm_shuffle_epi32(v33_i1, _MM_SHUFFLE(1, 1, 1, 1)));
  *((int *) (v33_row5 + 1*4)) = _mm_cvtsi128_si32(_mm_shuffle_epi32(v33_i3, _MM_SHUFFLE(1, 1, 1, 1)));
  *((int *) (v33_row6 + 0*4)) = _mm_cvtsi128_si32(_mm_shuffle_epi32(v33_i1, _MM_SHUFFLE(2, 2, 2, 2)));
  *((int *) (v33_row6 + 1*4)) = _mm_cvtsi128_si32(_mm_shuffle_epi32(v33_i3, _MM_SHUFFLE(2, 2, 2, 2)));
  *((int *) (v33_row7 + 0*4)) = _mm_cvtsi128_si32(_mm_shuffle_epi32(v33_i1, _MM_SHUFFLE(3, 3, 3, 3)));
  *((int *) (v33_row7 + 1*4)) = _mm_cvtsi128_si32(_mm_shuffle_epi32(v33_i3, _MM_SHUFFLE(3, 3, 3, 3)));
  }
}

static CODEGEN_ATTR void mublock16_LtoR(char unsigned * RADRESTRICT pixels, int stride, char unsigned * RADRESTRICT mask0, char unsigned * RADRESTRICT mask1, char unsigned * RADRESTRICT mask2, char unsigned * RADRESTRICT mask3)
{
  {
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(pixels);
  __m128i v10 = _mm_load_si128((__m128i *)(v0 + 0));
  int v1 = stride;
  char unsigned * RADRESTRICT v11 = v0 + v1;
  char unsigned * RADRESTRICT v13 = v11 + v1;
  __m128i v14 = _mm_load_si128((__m128i *)(v13 + 0));
  __m128i v12 = _mm_load_si128((__m128i *)(v11 + 0));
  __m128i v18 = _mm_sub_epi16(v14, v12);
  __m128i v21 = _mm_add_epi16(v18, _mm_set1_epi16(4));
  __m128i v22 = _mm_srai_epi16(v21, 3);
  char unsigned * RADRESTRICT v2 = (char unsigned * RADRESTRICT)(mask0);
  __m128i v6 = _mm_load_si128((__m128i *)(v2 + 0));
  __m128i v23 = _mm_and_si128(v22, v6);
  __m128i v24 = _mm_add_epi16(v10, v23);
  _mm_store_si128((__m128i * RADRESTRICT)(v0 + 0), v24);

  {
  __m128i v19 = _mm_add_epi16(v18, _mm_set1_epi16(2));
  __m128i v20 = _mm_srai_epi16(v19, 2);
  char unsigned * RADRESTRICT v3 = (char unsigned * RADRESTRICT)(mask1);
  __m128i v7 = _mm_load_si128((__m128i *)(v3 + 0));
  __m128i v25 = _mm_and_si128(v20, v7);
  __m128i v26 = _mm_add_epi16(v12, v25);
  _mm_store_si128((__m128i * RADRESTRICT)(v11 + 0), v26);

  {
  char unsigned * RADRESTRICT v4 = (char unsigned * RADRESTRICT)(mask2);
  __m128i v8 = _mm_load_si128((__m128i *)(v4 + 0));
  __m128i v27 = _mm_and_si128(v20, v8);
  __m128i v28 = _mm_sub_epi16(v14, v27);
  _mm_store_si128((__m128i * RADRESTRICT)(v13 + 0), v28);

  {
  char unsigned * RADRESTRICT v15 = v13 + v1;
  __m128i v16 = _mm_load_si128((__m128i *)(v15 + 0));
  char unsigned * RADRESTRICT v5 = (char unsigned * RADRESTRICT)(mask3);
  __m128i v9 = _mm_load_si128((__m128i *)(v5 + 0));
  __m128i v29 = _mm_and_si128(v22, v9);
  __m128i v30 = _mm_sub_epi16(v16, v29);
  _mm_store_si128((__m128i * RADRESTRICT)(v15 + 0), v30);
  }
  }
  }
  }
}

