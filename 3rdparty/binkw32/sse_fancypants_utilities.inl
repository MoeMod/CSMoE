// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#define simd4f __m128
#define simd4i __m128i
#define simd4f_0 (_mm_setzero_ps())
#define simd4i_0 (_mm_setzero_si128())

static CODEGEN_ATTR void move_u8_16x16(char unsigned * RADRESTRICT d, int dw, char unsigned * RADRESTRICT s, int sw)
{
  {
  {
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(d);
  char unsigned * RADRESTRICT v2 = (char unsigned * RADRESTRICT)(s);
  __m128i v8 = _mm_load_si128((__m128i *)(v2 + 0));
  _mm_store_si128((__m128i * RADRESTRICT)(v0 + 0), v8);

  {
  int v1 = dw;
  char unsigned * RADRESTRICT v11 = v0 + v1;
  int v3 = sw;
  char unsigned * RADRESTRICT v10 = v2 + v3;
  __m128i v12 = _mm_load_si128((__m128i *)(v10 + 0));
  _mm_store_si128((__m128i * RADRESTRICT)(v11 + 0), v12);

  {
  char unsigned * RADRESTRICT v15 = v11 + v1;
  char unsigned * RADRESTRICT v14 = v10 + v3;
  __m128i v16 = _mm_load_si128((__m128i *)(v14 + 0));
  _mm_store_si128((__m128i * RADRESTRICT)(v15 + 0), v16);

  {
  char unsigned * RADRESTRICT v19 = v15 + v1;
  char unsigned * RADRESTRICT v18 = v14 + v3;
  __m128i v20 = _mm_load_si128((__m128i *)(v18 + 0));
  _mm_store_si128((__m128i * RADRESTRICT)(v19 + 0), v20);

  {
  char unsigned * RADRESTRICT v23 = v19 + v1;
  char unsigned * RADRESTRICT v22 = v18 + v3;
  __m128i v24 = _mm_load_si128((__m128i *)(v22 + 0));
  _mm_store_si128((__m128i * RADRESTRICT)(v23 + 0), v24);

  {
  char unsigned * RADRESTRICT v27 = v23 + v1;
  char unsigned * RADRESTRICT v26 = v22 + v3;
  __m128i v28 = _mm_load_si128((__m128i *)(v26 + 0));
  _mm_store_si128((__m128i * RADRESTRICT)(v27 + 0), v28);

  {
  char unsigned * RADRESTRICT v31 = v27 + v1;
  char unsigned * RADRESTRICT v30 = v26 + v3;
  __m128i v32 = _mm_load_si128((__m128i *)(v30 + 0));
  _mm_store_si128((__m128i * RADRESTRICT)(v31 + 0), v32);

  {
  char unsigned * RADRESTRICT v35 = v31 + v1;
  char unsigned * RADRESTRICT v34 = v30 + v3;
  __m128i v36 = _mm_load_si128((__m128i *)(v34 + 0));
  _mm_store_si128((__m128i * RADRESTRICT)(v35 + 0), v36);

  {
  char unsigned * RADRESTRICT v39 = v35 + v1;
  char unsigned * RADRESTRICT v38 = v34 + v3;
  __m128i v40 = _mm_load_si128((__m128i *)(v38 + 0));
  _mm_store_si128((__m128i * RADRESTRICT)(v39 + 0), v40);

  {
  char unsigned * RADRESTRICT v43 = v39 + v1;
  char unsigned * RADRESTRICT v42 = v38 + v3;
  __m128i v44 = _mm_load_si128((__m128i *)(v42 + 0));
  _mm_store_si128((__m128i * RADRESTRICT)(v43 + 0), v44);

  {
  char unsigned * RADRESTRICT v47 = v43 + v1;
  char unsigned * RADRESTRICT v46 = v42 + v3;
  __m128i v48 = _mm_load_si128((__m128i *)(v46 + 0));
  _mm_store_si128((__m128i * RADRESTRICT)(v47 + 0), v48);

  {
  char unsigned * RADRESTRICT v51 = v47 + v1;
  char unsigned * RADRESTRICT v50 = v46 + v3;
  __m128i v52 = _mm_load_si128((__m128i *)(v50 + 0));
  _mm_store_si128((__m128i * RADRESTRICT)(v51 + 0), v52);

  {
  char unsigned * RADRESTRICT v55 = v51 + v1;
  char unsigned * RADRESTRICT v54 = v50 + v3;
  __m128i v56 = _mm_load_si128((__m128i *)(v54 + 0));
  _mm_store_si128((__m128i * RADRESTRICT)(v55 + 0), v56);

  {
  char unsigned * RADRESTRICT v59 = v55 + v1;
  char unsigned * RADRESTRICT v58 = v54 + v3;
  __m128i v60 = _mm_load_si128((__m128i *)(v58 + 0));
  _mm_store_si128((__m128i * RADRESTRICT)(v59 + 0), v60);

  {
  char unsigned * RADRESTRICT v63 = v59 + v1;
  char unsigned * RADRESTRICT v62 = v58 + v3;
  __m128i v64 = _mm_load_si128((__m128i *)(v62 + 0));
  _mm_store_si128((__m128i * RADRESTRICT)(v63 + 0), v64);

  {
  char unsigned * RADRESTRICT v67 = v63 + v1;
  char unsigned * RADRESTRICT v66 = v62 + v3;
  __m128i v68 = _mm_load_si128((__m128i *)(v66 + 0));
  _mm_store_si128((__m128i * RADRESTRICT)(v67 + 0), v68);
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

static CODEGEN_ATTR void move_u8_16x8(char unsigned * RADRESTRICT d, int dw, char unsigned * RADRESTRICT s, int sw)
{
  {
  {
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(d);
  char unsigned * RADRESTRICT v2 = (char unsigned * RADRESTRICT)(s);
  __m128i v8 = _mm_load_si128((__m128i *)(v2 + 0));
  _mm_store_si128((__m128i * RADRESTRICT)(v0 + 0), v8);

  {
  int v1 = dw;
  char unsigned * RADRESTRICT v11 = v0 + v1;
  int v3 = sw;
  char unsigned * RADRESTRICT v10 = v2 + v3;
  __m128i v12 = _mm_load_si128((__m128i *)(v10 + 0));
  _mm_store_si128((__m128i * RADRESTRICT)(v11 + 0), v12);

  {
  char unsigned * RADRESTRICT v15 = v11 + v1;
  char unsigned * RADRESTRICT v14 = v10 + v3;
  __m128i v16 = _mm_load_si128((__m128i *)(v14 + 0));
  _mm_store_si128((__m128i * RADRESTRICT)(v15 + 0), v16);

  {
  char unsigned * RADRESTRICT v19 = v15 + v1;
  char unsigned * RADRESTRICT v18 = v14 + v3;
  __m128i v20 = _mm_load_si128((__m128i *)(v18 + 0));
  _mm_store_si128((__m128i * RADRESTRICT)(v19 + 0), v20);

  {
  char unsigned * RADRESTRICT v23 = v19 + v1;
  char unsigned * RADRESTRICT v22 = v18 + v3;
  __m128i v24 = _mm_load_si128((__m128i *)(v22 + 0));
  _mm_store_si128((__m128i * RADRESTRICT)(v23 + 0), v24);

  {
  char unsigned * RADRESTRICT v27 = v23 + v1;
  char unsigned * RADRESTRICT v26 = v22 + v3;
  __m128i v28 = _mm_load_si128((__m128i *)(v26 + 0));
  _mm_store_si128((__m128i * RADRESTRICT)(v27 + 0), v28);

  {
  char unsigned * RADRESTRICT v31 = v27 + v1;
  char unsigned * RADRESTRICT v30 = v26 + v3;
  __m128i v32 = _mm_load_si128((__m128i *)(v30 + 0));
  _mm_store_si128((__m128i * RADRESTRICT)(v31 + 0), v32);

  {
  char unsigned * RADRESTRICT v35 = v31 + v1;
  char unsigned * RADRESTRICT v34 = v30 + v3;
  __m128i v36 = _mm_load_si128((__m128i *)(v34 + 0));
  _mm_store_si128((__m128i * RADRESTRICT)(v35 + 0), v36);
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
  __m128i v2 = _mm_loadu_si128((__m128i *)(v0 + 0)); \
  __m128i v3 = _mm_slli_epi16(v2, 4); \
  __m128i v4_0 = _mm_unpacklo_epi16(_mm_setzero_si128(), v3); \
  __m128i v6 = _mm_srai_epi32(v4_0, 20); \
  _mm_store_si128((__m128i * RADRESTRICT)(v1 + 0), v6); \
 \
  { \
  __m128i v4_1 = _mm_unpackhi_epi16(_mm_setzero_si128(), v3); \
  __m128i v9 = _mm_srai_epi32(v4_1, 20); \
  _mm_store_si128((__m128i * RADRESTRICT)(v1 + 16), v9); \
  } \
  } \
}

#define dcs_unpackx4(input, output) \
{\
  { \
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(output); \
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(input); \
  __m128i v2 = _mm_loadu_si128((__m128i *)(v0 + 0)); \
  __m128i v3 = _mm_slli_epi16(v2, 4); \
  __m128i v4_0 = _mm_unpacklo_epi16(_mm_setzero_si128(), v3); \
  __m128i v6 = _mm_srai_epi32(v4_0, 20); \
  _mm_store_si128((__m128i * RADRESTRICT)(v1 + 0), v6); \
  } \
}

static CODEGEN_ATTR void calc_average8_u8(char unsigned * RADRESTRICT out, char unsigned * RADRESTRICT ptr, int w)
{
  {
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(ptr);
  __m128i v3 = _mm_loadl_epi64((__m128i *)(v1 + 0));
  int v2 = w;
  char unsigned * RADRESTRICT v4 = v1 + v2;
  __m128i v5 = _mm_loadl_epi64((__m128i *)(v4 + 0));
  __m128i v23 = _mm_avg_epu8(v3, v5);
  char unsigned * RADRESTRICT v6 = v4 + v2;
  __m128i v7 = _mm_loadl_epi64((__m128i *)(v6 + 0));
  char unsigned * RADRESTRICT v8 = v6 + v2;
  __m128i v9 = _mm_loadl_epi64((__m128i *)(v8 + 0));
  __m128i v22 = _mm_avg_epu8(v7, v9);
  __m128i v24 = _mm_avg_epu8(v23, v22);
  char unsigned * RADRESTRICT v10 = v8 + v2;
  __m128i v11 = _mm_loadl_epi64((__m128i *)(v10 + 0));
  char unsigned * RADRESTRICT v12 = v10 + v2;
  __m128i v13 = _mm_loadl_epi64((__m128i *)(v12 + 0));
  __m128i v20 = _mm_avg_epu8(v11, v13);
  char unsigned * RADRESTRICT v14 = v12 + v2;
  __m128i v15 = _mm_loadl_epi64((__m128i *)(v14 + 0));
  char unsigned * RADRESTRICT v16 = v14 + v2;
  __m128i v17 = _mm_loadl_epi64((__m128i *)(v16 + 0));
  __m128i v19 = _mm_avg_epu8(v15, v17);
  __m128i v21 = _mm_avg_epu8(v20, v19);
  __m128i v25 = _mm_avg_epu8(v24, v21);
  __m128i P0 = _mm_unpacklo_epi8(v25, _mm_setzero_si128());
  __m128i F0 = _mm_add_epi16(_mm_srli_si128(P0, 8), P0);
  __m128i F1 = _mm_add_epi16(_mm_srli_si128(F0, 4), F0);
  __m128i F2 = _mm_add_epi16(_mm_srli_si128(F1, 2), F1);
  __m128i v26 = _mm_unpacklo_epi16(F2, _mm_setzero_si128());
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(out);
  *(int *)(v0 + 0) = _mm_cvtsi128_si32(v26);
  }
}

static CODEGEN_ATTR void calc_average2x8_u8(char unsigned * RADRESTRICT out, char unsigned * RADRESTRICT ptr, int w)
{
  {
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(out);
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(ptr);
  __m128i v3 = _mm_load_si128((__m128i *)(v1 + 0));
  int v2 = w;
  char unsigned * RADRESTRICT v5 = v1 + v2;
  __m128i v6 = _mm_load_si128((__m128i *)(v5 + 0));
  __m128i v31 = _mm_avg_epu8(v3, v6);
  char unsigned * RADRESTRICT v8 = v5 + v2;
  __m128i v9 = _mm_load_si128((__m128i *)(v8 + 0));
  char unsigned * RADRESTRICT v11 = v8 + v2;
  __m128i v12 = _mm_load_si128((__m128i *)(v11 + 0));
  __m128i v30 = _mm_avg_epu8(v9, v12);
  __m128i v32 = _mm_avg_epu8(v31, v30);
  char unsigned * RADRESTRICT v14 = v11 + v2;
  __m128i v15 = _mm_load_si128((__m128i *)(v14 + 0));
  char unsigned * RADRESTRICT v17 = v14 + v2;
  __m128i v18 = _mm_load_si128((__m128i *)(v17 + 0));
  __m128i v28 = _mm_avg_epu8(v15, v18);
  char unsigned * RADRESTRICT v20 = v17 + v2;
  __m128i v21 = _mm_load_si128((__m128i *)(v20 + 0));
  char unsigned * RADRESTRICT v23 = v20 + v2;
  __m128i v24 = _mm_load_si128((__m128i *)(v23 + 0));
  __m128i v27 = _mm_avg_epu8(v21, v24);
  __m128i v29 = _mm_avg_epu8(v28, v27);
  __m128i v33 = _mm_avg_epu8(v32, v29);
  __m128i TL = _mm_unpacklo_epi8(v33, _mm_setzero_si128());
  __m128i TR = _mm_unpackhi_epi8(v33, _mm_setzero_si128());
  __m128i BL = _mm_unpacklo_epi8(v33, _mm_setzero_si128());
  __m128i BR = _mm_unpackhi_epi8(v33, _mm_setzero_si128());
  __m128i L  = _mm_unpacklo_epi16(TL, BL);
  __m128i Ls = _mm_unpackhi_epi16(TL, BL);
  __m128i La = _mm_add_epi16(L, Ls);
  __m128i R  = _mm_unpacklo_epi16(TR, BR);
  __m128i Rs = _mm_unpackhi_epi16(TR, BR);
  __m128i Ra = _mm_add_epi16(R, Rs);
  __m128i C  = _mm_unpacklo_epi16(La, Ra);
  __m128i Cs = _mm_unpackhi_epi16(La, Ra);
  __m128i Ca = _mm_add_epi16(C, Cs);
  __m128i F  = _mm_unpacklo_epi16(Ca, _mm_setzero_si128());
  __m128i Fs = _mm_unpackhi_epi16(Ca, _mm_setzero_si128());
  __m128i v41 = _mm_add_epi16(F, Fs);
  _mm_storel_epi64((__m128i * RADRESTRICT)(v0 + 0), v41);
  }
}

static CODEGEN_ATTR void calc_average4x8_u8(char unsigned * RADRESTRICT out, char unsigned * RADRESTRICT ptr, int w)
{
  {
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(out);
  char unsigned * RADRESTRICT v1 = (char unsigned * RADRESTRICT)(ptr);
  __m128i v3 = _mm_load_si128((__m128i *)(v1 + 0));
  int v2 = w;
  char unsigned * RADRESTRICT v5 = v1 + v2;
  __m128i v6 = _mm_load_si128((__m128i *)(v5 + 0));
  __m128i v31 = _mm_avg_epu8(v3, v6);
  char unsigned * RADRESTRICT v8 = v5 + v2;
  __m128i v9 = _mm_load_si128((__m128i *)(v8 + 0));
  char unsigned * RADRESTRICT v11 = v8 + v2;
  __m128i v12 = _mm_load_si128((__m128i *)(v11 + 0));
  __m128i v30 = _mm_avg_epu8(v9, v12);
  __m128i v32 = _mm_avg_epu8(v31, v30);
  char unsigned * RADRESTRICT v14 = v11 + v2;
  __m128i v15 = _mm_load_si128((__m128i *)(v14 + 0));
  char unsigned * RADRESTRICT v17 = v14 + v2;
  __m128i v18 = _mm_load_si128((__m128i *)(v17 + 0));
  __m128i v28 = _mm_avg_epu8(v15, v18);
  char unsigned * RADRESTRICT v20 = v17 + v2;
  __m128i v21 = _mm_load_si128((__m128i *)(v20 + 0));
  char unsigned * RADRESTRICT v23 = v20 + v2;
  __m128i v24 = _mm_load_si128((__m128i *)(v23 + 0));
  __m128i v27 = _mm_avg_epu8(v21, v24);
  __m128i v29 = _mm_avg_epu8(v28, v27);
  __m128i v33 = _mm_avg_epu8(v32, v29);
  __m128i v4 = _mm_load_si128((__m128i *)(v1 + 16));
  __m128i v7 = _mm_load_si128((__m128i *)(v5 + 16));
  __m128i v38 = _mm_avg_epu8(v4, v7);
  __m128i v10 = _mm_load_si128((__m128i *)(v8 + 16));
  __m128i v13 = _mm_load_si128((__m128i *)(v11 + 16));
  __m128i v37 = _mm_avg_epu8(v10, v13);
  __m128i v39 = _mm_avg_epu8(v38, v37);
  __m128i v16 = _mm_load_si128((__m128i *)(v14 + 16));
  __m128i v19 = _mm_load_si128((__m128i *)(v17 + 16));
  __m128i v35 = _mm_avg_epu8(v16, v19);
  __m128i v22 = _mm_load_si128((__m128i *)(v20 + 16));
  __m128i v25 = _mm_load_si128((__m128i *)(v23 + 16));
  __m128i v34 = _mm_avg_epu8(v22, v25);
  __m128i v36 = _mm_avg_epu8(v35, v34);
  __m128i v40 = _mm_avg_epu8(v39, v36);
  __m128i TL = _mm_unpacklo_epi8(v33, _mm_setzero_si128());
  __m128i TR = _mm_unpackhi_epi8(v33, _mm_setzero_si128());
  __m128i BL = _mm_unpacklo_epi8(v40, _mm_setzero_si128());
  __m128i BR = _mm_unpackhi_epi8(v40, _mm_setzero_si128());
  __m128i L  = _mm_unpacklo_epi16(TL, BL);
  __m128i Ls = _mm_unpackhi_epi16(TL, BL);
  __m128i La = _mm_add_epi16(L, Ls);
  __m128i R  = _mm_unpacklo_epi16(TR, BR);
  __m128i Rs = _mm_unpackhi_epi16(TR, BR);
  __m128i Ra = _mm_add_epi16(R, Rs);
  __m128i C  = _mm_unpacklo_epi16(La, Ra);
  __m128i Cs = _mm_unpackhi_epi16(La, Ra);
  __m128i Ca = _mm_add_epi16(C, Cs);
  __m128i F  = _mm_unpacklo_epi16(Ca, _mm_setzero_si128());
  __m128i Fs = _mm_unpackhi_epi16(Ca, _mm_setzero_si128());
  __m128i v41 = _mm_add_epi16(F, Fs);
  _mm_store_si128((__m128i * RADRESTRICT)(v0 + 0), v41);
  }
}

static CODEGEN_ATTR void pack16x8_to_u8(char unsigned * RADRESTRICT dst, int dst_stride, char unsigned * RADRESTRICT src, int src_stride)
{
  {
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(dst);
  char unsigned * RADRESTRICT v2 = (char unsigned * RADRESTRICT)(src);
  __m128i v4_0 = _mm_load_si128((__m128i *)(v2 + 0));
  __m128i v4_1 = _mm_load_si128((__m128i *)(v2 + 16));
  __m128i v5 = _mm_packus_epi16(v4_0, v4_1);
  _mm_store_si128((__m128i * RADRESTRICT)(v0 + 0), v5);

  {
  int v1 = dst_stride;
  char unsigned * RADRESTRICT v8 = v0 + v1;
  int v3 = src_stride;
  char unsigned * RADRESTRICT v7 = v2 + v3;
  __m128i v9_0 = _mm_load_si128((__m128i *)(v7 + 0));
  __m128i v9_1 = _mm_load_si128((__m128i *)(v7 + 16));
  __m128i v10 = _mm_packus_epi16(v9_0, v9_1);
  _mm_store_si128((__m128i * RADRESTRICT)(v8 + 0), v10);

  {
  char unsigned * RADRESTRICT v13 = v8 + v1;
  char unsigned * RADRESTRICT v12 = v7 + v3;
  __m128i v14_0 = _mm_load_si128((__m128i *)(v12 + 0));
  __m128i v14_1 = _mm_load_si128((__m128i *)(v12 + 16));
  __m128i v15 = _mm_packus_epi16(v14_0, v14_1);
  _mm_store_si128((__m128i * RADRESTRICT)(v13 + 0), v15);

  {
  char unsigned * RADRESTRICT v18 = v13 + v1;
  char unsigned * RADRESTRICT v17 = v12 + v3;
  __m128i v19_0 = _mm_load_si128((__m128i *)(v17 + 0));
  __m128i v19_1 = _mm_load_si128((__m128i *)(v17 + 16));
  __m128i v20 = _mm_packus_epi16(v19_0, v19_1);
  _mm_store_si128((__m128i * RADRESTRICT)(v18 + 0), v20);

  {
  char unsigned * RADRESTRICT v23 = v18 + v1;
  char unsigned * RADRESTRICT v22 = v17 + v3;
  __m128i v24_0 = _mm_load_si128((__m128i *)(v22 + 0));
  __m128i v24_1 = _mm_load_si128((__m128i *)(v22 + 16));
  __m128i v25 = _mm_packus_epi16(v24_0, v24_1);
  _mm_store_si128((__m128i * RADRESTRICT)(v23 + 0), v25);

  {
  char unsigned * RADRESTRICT v28 = v23 + v1;
  char unsigned * RADRESTRICT v27 = v22 + v3;
  __m128i v29_0 = _mm_load_si128((__m128i *)(v27 + 0));
  __m128i v29_1 = _mm_load_si128((__m128i *)(v27 + 16));
  __m128i v30 = _mm_packus_epi16(v29_0, v29_1);
  _mm_store_si128((__m128i * RADRESTRICT)(v28 + 0), v30);

  {
  char unsigned * RADRESTRICT v33 = v28 + v1;
  char unsigned * RADRESTRICT v32 = v27 + v3;
  __m128i v34_0 = _mm_load_si128((__m128i *)(v32 + 0));
  __m128i v34_1 = _mm_load_si128((__m128i *)(v32 + 16));
  __m128i v35 = _mm_packus_epi16(v34_0, v34_1);
  _mm_store_si128((__m128i * RADRESTRICT)(v33 + 0), v35);

  {
  char unsigned * RADRESTRICT v38 = v33 + v1;
  char unsigned * RADRESTRICT v37 = v32 + v3;
  __m128i v39_0 = _mm_load_si128((__m128i *)(v37 + 0));
  __m128i v39_1 = _mm_load_si128((__m128i *)(v37 + 16));
  __m128i v40 = _mm_packus_epi16(v39_0, v39_1);
  _mm_store_si128((__m128i * RADRESTRICT)(v38 + 0), v40);
  }
  }
  }
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void pack16x8_to_u8_and_mocomp(char unsigned * RADRESTRICT dst, int dst_stride, char unsigned * RADRESTRICT src, int src_stride, char unsigned * RADRESTRICT motion, int motion_stride)
{
  {
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(dst);
  char unsigned * RADRESTRICT v2 = (char unsigned * RADRESTRICT)(src);
  __m128i v6_0 = _mm_load_si128((__m128i *)(v2 + 0));
  char unsigned * RADRESTRICT v4 = (char unsigned * RADRESTRICT)(motion);
  __m128i v7 = _mm_load_si128((__m128i *)(v4 + 0));
  __m128i v8_0 = _mm_unpacklo_epi8(v7, _mm_setzero_si128());
  __m128i v9_0 = _mm_add_epi16(v6_0, v8_0);
  __m128i v6_1 = _mm_load_si128((__m128i *)(v2 + 16));
  __m128i v8_1 = _mm_unpackhi_epi8(v7, _mm_setzero_si128());
  __m128i v9_1 = _mm_add_epi16(v6_1, v8_1);
  __m128i v10 = _mm_packus_epi16(v9_0, v9_1);
  _mm_store_si128((__m128i * RADRESTRICT)(v0 + 0), v10);

  {
  int v1 = dst_stride;
  char unsigned * RADRESTRICT v14 = v0 + v1;
  int v3 = src_stride;
  char unsigned * RADRESTRICT v13 = v2 + v3;
  __m128i v15_0 = _mm_load_si128((__m128i *)(v13 + 0));
  int v5 = motion_stride;
  char unsigned * RADRESTRICT v11 = v4 + v5;
  __m128i v16 = _mm_load_si128((__m128i *)(v11 + 0));
  __m128i v17_0 = _mm_unpacklo_epi8(v16, _mm_setzero_si128());
  __m128i v18_0 = _mm_add_epi16(v15_0, v17_0);
  __m128i v15_1 = _mm_load_si128((__m128i *)(v13 + 16));
  __m128i v17_1 = _mm_unpackhi_epi8(v16, _mm_setzero_si128());
  __m128i v18_1 = _mm_add_epi16(v15_1, v17_1);
  __m128i v19 = _mm_packus_epi16(v18_0, v18_1);
  _mm_store_si128((__m128i * RADRESTRICT)(v14 + 0), v19);

  {
  char unsigned * RADRESTRICT v23 = v14 + v1;
  char unsigned * RADRESTRICT v22 = v13 + v3;
  __m128i v24_0 = _mm_load_si128((__m128i *)(v22 + 0));
  char unsigned * RADRESTRICT v20 = v11 + v5;
  __m128i v25 = _mm_load_si128((__m128i *)(v20 + 0));
  __m128i v26_0 = _mm_unpacklo_epi8(v25, _mm_setzero_si128());
  __m128i v27_0 = _mm_add_epi16(v24_0, v26_0);
  __m128i v24_1 = _mm_load_si128((__m128i *)(v22 + 16));
  __m128i v26_1 = _mm_unpackhi_epi8(v25, _mm_setzero_si128());
  __m128i v27_1 = _mm_add_epi16(v24_1, v26_1);
  __m128i v28 = _mm_packus_epi16(v27_0, v27_1);
  _mm_store_si128((__m128i * RADRESTRICT)(v23 + 0), v28);

  {
  char unsigned * RADRESTRICT v32 = v23 + v1;
  char unsigned * RADRESTRICT v31 = v22 + v3;
  __m128i v33_0 = _mm_load_si128((__m128i *)(v31 + 0));
  char unsigned * RADRESTRICT v29 = v20 + v5;
  __m128i v34 = _mm_load_si128((__m128i *)(v29 + 0));
  __m128i v35_0 = _mm_unpacklo_epi8(v34, _mm_setzero_si128());
  __m128i v36_0 = _mm_add_epi16(v33_0, v35_0);
  __m128i v33_1 = _mm_load_si128((__m128i *)(v31 + 16));
  __m128i v35_1 = _mm_unpackhi_epi8(v34, _mm_setzero_si128());
  __m128i v36_1 = _mm_add_epi16(v33_1, v35_1);
  __m128i v37 = _mm_packus_epi16(v36_0, v36_1);
  _mm_store_si128((__m128i * RADRESTRICT)(v32 + 0), v37);

  {
  char unsigned * RADRESTRICT v41 = v32 + v1;
  char unsigned * RADRESTRICT v40 = v31 + v3;
  __m128i v42_0 = _mm_load_si128((__m128i *)(v40 + 0));
  char unsigned * RADRESTRICT v38 = v29 + v5;
  __m128i v43 = _mm_load_si128((__m128i *)(v38 + 0));
  __m128i v44_0 = _mm_unpacklo_epi8(v43, _mm_setzero_si128());
  __m128i v45_0 = _mm_add_epi16(v42_0, v44_0);
  __m128i v42_1 = _mm_load_si128((__m128i *)(v40 + 16));
  __m128i v44_1 = _mm_unpackhi_epi8(v43, _mm_setzero_si128());
  __m128i v45_1 = _mm_add_epi16(v42_1, v44_1);
  __m128i v46 = _mm_packus_epi16(v45_0, v45_1);
  _mm_store_si128((__m128i * RADRESTRICT)(v41 + 0), v46);

  {
  char unsigned * RADRESTRICT v50 = v41 + v1;
  char unsigned * RADRESTRICT v49 = v40 + v3;
  __m128i v51_0 = _mm_load_si128((__m128i *)(v49 + 0));
  char unsigned * RADRESTRICT v47 = v38 + v5;
  __m128i v52 = _mm_load_si128((__m128i *)(v47 + 0));
  __m128i v53_0 = _mm_unpacklo_epi8(v52, _mm_setzero_si128());
  __m128i v54_0 = _mm_add_epi16(v51_0, v53_0);
  __m128i v51_1 = _mm_load_si128((__m128i *)(v49 + 16));
  __m128i v53_1 = _mm_unpackhi_epi8(v52, _mm_setzero_si128());
  __m128i v54_1 = _mm_add_epi16(v51_1, v53_1);
  __m128i v55 = _mm_packus_epi16(v54_0, v54_1);
  _mm_store_si128((__m128i * RADRESTRICT)(v50 + 0), v55);

  {
  char unsigned * RADRESTRICT v59 = v50 + v1;
  char unsigned * RADRESTRICT v58 = v49 + v3;
  __m128i v60_0 = _mm_load_si128((__m128i *)(v58 + 0));
  char unsigned * RADRESTRICT v56 = v47 + v5;
  __m128i v61 = _mm_load_si128((__m128i *)(v56 + 0));
  __m128i v62_0 = _mm_unpacklo_epi8(v61, _mm_setzero_si128());
  __m128i v63_0 = _mm_add_epi16(v60_0, v62_0);
  __m128i v60_1 = _mm_load_si128((__m128i *)(v58 + 16));
  __m128i v62_1 = _mm_unpackhi_epi8(v61, _mm_setzero_si128());
  __m128i v63_1 = _mm_add_epi16(v60_1, v62_1);
  __m128i v64 = _mm_packus_epi16(v63_0, v63_1);
  _mm_store_si128((__m128i * RADRESTRICT)(v59 + 0), v64);

  {
  char unsigned * RADRESTRICT v68 = v59 + v1;
  char unsigned * RADRESTRICT v67 = v58 + v3;
  __m128i v69_0 = _mm_load_si128((__m128i *)(v67 + 0));
  char unsigned * RADRESTRICT v65 = v56 + v5;
  __m128i v70 = _mm_load_si128((__m128i *)(v65 + 0));
  __m128i v71_0 = _mm_unpacklo_epi8(v70, _mm_setzero_si128());
  __m128i v72_0 = _mm_add_epi16(v69_0, v71_0);
  __m128i v69_1 = _mm_load_si128((__m128i *)(v67 + 16));
  __m128i v71_1 = _mm_unpackhi_epi8(v70, _mm_setzero_si128());
  __m128i v72_1 = _mm_add_epi16(v69_1, v71_1);
  __m128i v73 = _mm_packus_epi16(v72_0, v72_1);
  _mm_store_si128((__m128i * RADRESTRICT)(v68 + 0), v73);
  }
  }
  }
  }
  }
  }
  }
  }
}

