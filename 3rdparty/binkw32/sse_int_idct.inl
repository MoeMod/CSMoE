// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

static CODEGEN_ATTR void int_idct_default(char unsigned * RADRESTRICT out_ptr, int out_stride, char unsigned * RADRESTRICT in_ptr)
{
  {
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(out_ptr);
  char unsigned * RADRESTRICT v2 = (char unsigned * RADRESTRICT)(in_ptr);
  __m128i v3 = _mm_load_si128((__m128i *)(v2 + 0));
  __m128i v7 = _mm_load_si128((__m128i *)(v2 + 64));
  __m128i v37 = _mm_add_epi16(v3, v7);
  __m128i v9 = _mm_load_si128((__m128i *)(v2 + 96));
  __m128i v41 = _mm_srai_epi16(v9, 1);
  __m128i v5 = _mm_load_si128((__m128i *)(v2 + 32));
  __m128i v39 = _mm_srai_epi16(v5, 2);
  __m128i v40 = _mm_add_epi16(v5, v39);
  __m128i v42 = _mm_add_epi16(v41, v40);
  __m128i v47 = _mm_add_epi16(v37, v42);
  __m128i v6 = _mm_load_si128((__m128i *)(v2 + 48));
  __m128i v8 = _mm_load_si128((__m128i *)(v2 + 80));
  __m128i v12 = _mm_sub_epi16(v6, v8);
  __m128i v10 = _mm_load_si128((__m128i *)(v2 + 112));
  __m128i v16 = _mm_add_epi16(v12, v10);
  __m128i v18 = _mm_srai_epi16(v16, 2);
  __m128i v4 = _mm_load_si128((__m128i *)(v2 + 16));
  __m128i v11 = _mm_add_epi16(v6, v8);
  __m128i v13 = _mm_add_epi16(v4, v11);
  __m128i v17 = _mm_srai_epi16(v13, 2);
  __m128i v20 = _mm_add_epi16(v13, v17);
  __m128i v19 = _mm_srai_epi16(v13, 4);
  __m128i v21 = _mm_sub_epi16(v20, v19);
  __m128i v22 = _mm_add_epi16(v18, v21);
  __m128i v51 = _mm_add_epi16(v47, v22);
  __m128i v50 = _mm_sub_epi16(v37, v42);
  __m128i v24 = _mm_add_epi16(v16, v18);
  __m128i v23 = _mm_srai_epi16(v16, 4);
  __m128i v25 = _mm_sub_epi16(v24, v23);
  __m128i v26 = _mm_sub_epi16(v17, v25);
  __m128i v55 = _mm_sub_epi16(v50, v26);
  __m128i v66_0 = _mm_unpacklo_epi16(v51, v55);
  __m128i v38 = _mm_sub_epi16(v3, v7);
  __m128i v45 = _mm_srai_epi16(v5, 1);
  __m128i v43 = _mm_srai_epi16(v9, 2);
  __m128i v44 = _mm_add_epi16(v9, v43);
  __m128i v46 = _mm_sub_epi16(v45, v44);
  __m128i v49 = _mm_sub_epi16(v38, v46);
  __m128i v14 = _mm_sub_epi16(v4, v11);
  __m128i v33 = _mm_srai_epi16(v14, 2);
  __m128i v34 = _mm_sub_epi16(v14, v33);
  __m128i v32 = _mm_srai_epi16(v14, 4);
  __m128i v35 = _mm_sub_epi16(v34, v32);
  __m128i v15 = _mm_sub_epi16(v12, v10);
  __m128i v36 = _mm_sub_epi16(v35, v15);
  __m128i v53 = _mm_add_epi16(v49, v36);
  __m128i v48 = _mm_add_epi16(v38, v46);
  __m128i v28 = _mm_srai_epi16(v15, 2);
  __m128i v29 = _mm_sub_epi16(v15, v28);
  __m128i v27 = _mm_srai_epi16(v15, 4);
  __m128i v30 = _mm_sub_epi16(v29, v27);
  __m128i v31 = _mm_add_epi16(v30, v14);
  __m128i v57 = _mm_sub_epi16(v48, v31);
  __m128i v66_4 = _mm_unpacklo_epi16(v53, v57);
  __m128i v67_0 = _mm_unpacklo_epi16(v66_0, v66_4);
  __m128i v52 = _mm_add_epi16(v48, v31);
  __m128i v56 = _mm_sub_epi16(v49, v36);
  __m128i v66_2 = _mm_unpacklo_epi16(v52, v56);
  __m128i v54 = _mm_add_epi16(v50, v26);
  __m128i v58 = _mm_sub_epi16(v47, v22);
  __m128i v66_6 = _mm_unpacklo_epi16(v54, v58);
  __m128i v67_4 = _mm_unpacklo_epi16(v66_2, v66_6);
  __m128i v68_0 = _mm_unpacklo_epi16(v67_0, v67_4);
  __m128i v66_1 = _mm_unpackhi_epi16(v51, v55);
  __m128i v66_5 = _mm_unpackhi_epi16(v53, v57);
  __m128i v67_2 = _mm_unpacklo_epi16(v66_1, v66_5);
  __m128i v66_3 = _mm_unpackhi_epi16(v52, v56);
  __m128i v66_7 = _mm_unpackhi_epi16(v54, v58);
  __m128i v67_6 = _mm_unpacklo_epi16(v66_3, v66_7);
  __m128i v68_4 = _mm_unpacklo_epi16(v67_2, v67_6);
  __m128i v103 = _mm_add_epi16(v68_0, v68_4);
  __m128i v67_3 = _mm_unpackhi_epi16(v66_1, v66_5);
  __m128i v67_7 = _mm_unpackhi_epi16(v66_3, v66_7);
  __m128i v68_6 = _mm_unpacklo_epi16(v67_3, v67_7);
  __m128i v107 = _mm_srai_epi16(v68_6, 1);
  __m128i v67_1 = _mm_unpackhi_epi16(v66_0, v66_4);
  __m128i v67_5 = _mm_unpackhi_epi16(v66_2, v66_6);
  __m128i v68_2 = _mm_unpacklo_epi16(v67_1, v67_5);
  __m128i v105 = _mm_srai_epi16(v68_2, 2);
  __m128i v106 = _mm_add_epi16(v68_2, v105);
  __m128i v108 = _mm_add_epi16(v107, v106);
  __m128i v113 = _mm_add_epi16(v103, v108);
  __m128i v68_3 = _mm_unpackhi_epi16(v67_1, v67_5);
  __m128i v68_5 = _mm_unpackhi_epi16(v67_2, v67_6);
  __m128i v78 = _mm_sub_epi16(v68_3, v68_5);
  __m128i v68_7 = _mm_unpackhi_epi16(v67_3, v67_7);
  __m128i v82 = _mm_add_epi16(v78, v68_7);
  __m128i v84 = _mm_srai_epi16(v82, 2);
  __m128i v68_1 = _mm_unpackhi_epi16(v67_0, v67_4);
  __m128i v77 = _mm_add_epi16(v68_3, v68_5);
  __m128i v79 = _mm_add_epi16(v68_1, v77);
  __m128i v83 = _mm_srai_epi16(v79, 2);
  __m128i v86 = _mm_add_epi16(v79, v83);
  __m128i v85 = _mm_srai_epi16(v79, 4);
  __m128i v87 = _mm_sub_epi16(v86, v85);
  __m128i v88 = _mm_add_epi16(v84, v87);
  __m128i v117 = _mm_add_epi16(v113, v88);
  __m128i v125 = _mm_srai_epi16(v117, 6);
  _mm_store_si128((__m128i * RADRESTRICT)(v0 + 0), v125);

  {
  int v1 = out_stride;
  char unsigned * RADRESTRICT v127 = v0 + v1;
  __m128i v104 = _mm_sub_epi16(v68_0, v68_4);
  __m128i v111 = _mm_srai_epi16(v68_2, 1);
  __m128i v109 = _mm_srai_epi16(v68_6, 2);
  __m128i v110 = _mm_add_epi16(v68_6, v109);
  __m128i v112 = _mm_sub_epi16(v111, v110);
  __m128i v114 = _mm_add_epi16(v104, v112);
  __m128i v81 = _mm_sub_epi16(v78, v68_7);
  __m128i v94 = _mm_srai_epi16(v81, 2);
  __m128i v95 = _mm_sub_epi16(v81, v94);
  __m128i v93 = _mm_srai_epi16(v81, 4);
  __m128i v96 = _mm_sub_epi16(v95, v93);
  __m128i v80 = _mm_sub_epi16(v68_1, v77);
  __m128i v97 = _mm_add_epi16(v96, v80);
  __m128i v118 = _mm_add_epi16(v114, v97);
  __m128i v128 = _mm_srai_epi16(v118, 6);
  _mm_store_si128((__m128i * RADRESTRICT)(v127 + 0), v128);

  {
  char unsigned * RADRESTRICT v130 = v127 + v1;
  __m128i v115 = _mm_sub_epi16(v104, v112);
  __m128i v99 = _mm_srai_epi16(v80, 2);
  __m128i v100 = _mm_sub_epi16(v80, v99);
  __m128i v98 = _mm_srai_epi16(v80, 4);
  __m128i v101 = _mm_sub_epi16(v100, v98);
  __m128i v102 = _mm_sub_epi16(v101, v81);
  __m128i v119 = _mm_add_epi16(v115, v102);
  __m128i v131 = _mm_srai_epi16(v119, 6);
  _mm_store_si128((__m128i * RADRESTRICT)(v130 + 0), v131);

  {
  char unsigned * RADRESTRICT v133 = v130 + v1;
  __m128i v116 = _mm_sub_epi16(v103, v108);
  __m128i v90 = _mm_add_epi16(v82, v84);
  __m128i v89 = _mm_srai_epi16(v82, 4);
  __m128i v91 = _mm_sub_epi16(v90, v89);
  __m128i v92 = _mm_sub_epi16(v83, v91);
  __m128i v120 = _mm_add_epi16(v116, v92);
  __m128i v134 = _mm_srai_epi16(v120, 6);
  _mm_store_si128((__m128i * RADRESTRICT)(v133 + 0), v134);

  {
  char unsigned * RADRESTRICT v136 = v133 + v1;
  __m128i v121 = _mm_sub_epi16(v116, v92);
  __m128i v137 = _mm_srai_epi16(v121, 6);
  _mm_store_si128((__m128i * RADRESTRICT)(v136 + 0), v137);

  {
  char unsigned * RADRESTRICT v139 = v136 + v1;
  __m128i v122 = _mm_sub_epi16(v115, v102);
  __m128i v140 = _mm_srai_epi16(v122, 6);
  _mm_store_si128((__m128i * RADRESTRICT)(v139 + 0), v140);

  {
  char unsigned * RADRESTRICT v142 = v139 + v1;
  __m128i v123 = _mm_sub_epi16(v114, v97);
  __m128i v143 = _mm_srai_epi16(v123, 6);
  _mm_store_si128((__m128i * RADRESTRICT)(v142 + 0), v143);

  {
  char unsigned * RADRESTRICT v145 = v142 + v1;
  __m128i v124 = _mm_sub_epi16(v113, v88);
  __m128i v146 = _mm_srai_epi16(v124, 6);
  _mm_store_si128((__m128i * RADRESTRICT)(v145 + 0), v146);
  }
  }
  }
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void int_idct_sparse(char unsigned * RADRESTRICT out_ptr, int out_stride, char unsigned * RADRESTRICT in_ptr)
{
  {
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(out_ptr);
  char unsigned * RADRESTRICT v2 = (char unsigned * RADRESTRICT)(in_ptr);
  __m128i v3 = _mm_load_si128((__m128i *)(v2 + 0));
  __m128i v5 = _mm_load_si128((__m128i *)(v2 + 32));
  __m128i v26 = _mm_srai_epi16(v5, 2);
  __m128i v27 = _mm_add_epi16(v5, v26);
  __m128i v29 = _mm_add_epi16(v3, v27);
  __m128i v6 = _mm_load_si128((__m128i *)(v2 + 48));
  __m128i v10 = _mm_srai_epi16(v6, 2);
  __m128i v4 = _mm_load_si128((__m128i *)(v2 + 16));
  __m128i v7 = _mm_add_epi16(v4, v6);
  __m128i v9 = _mm_srai_epi16(v7, 2);
  __m128i v14 = _mm_add_epi16(v7, v9);
  __m128i v13 = _mm_srai_epi16(v7, 4);
  __m128i v15 = _mm_sub_epi16(v14, v13);
  __m128i v16 = _mm_add_epi16(v10, v15);
  __m128i v33 = _mm_add_epi16(v29, v16);
  __m128i v32 = _mm_sub_epi16(v3, v27);
  __m128i v11 = _mm_srai_epi16(v6, 4);
  __m128i v12 = _mm_sub_epi16(v6, v11);
  __m128i v17 = _mm_add_epi16(v12, v10);
  __m128i v18 = _mm_sub_epi16(v9, v17);
  __m128i v37 = _mm_sub_epi16(v32, v18);
  __m128i v48_0 = _mm_unpacklo_epi16(v33, v37);
  __m128i v28 = _mm_srai_epi16(v5, 1);
  __m128i v31 = _mm_sub_epi16(v3, v28);
  __m128i v8 = _mm_sub_epi16(v4, v6);
  __m128i v22 = _mm_srai_epi16(v8, 2);
  __m128i v23 = _mm_sub_epi16(v8, v22);
  __m128i v21 = _mm_srai_epi16(v8, 4);
  __m128i v24 = _mm_sub_epi16(v23, v21);
  __m128i v25 = _mm_sub_epi16(v24, v6);
  __m128i v35 = _mm_add_epi16(v31, v25);
  __m128i v30 = _mm_add_epi16(v3, v28);
  __m128i v19 = _mm_sub_epi16(v12, v10);
  __m128i v20 = _mm_add_epi16(v19, v8);
  __m128i v39 = _mm_sub_epi16(v30, v20);
  __m128i v48_4 = _mm_unpacklo_epi16(v35, v39);
  __m128i v49_0 = _mm_unpacklo_epi16(v48_0, v48_4);
  __m128i v34 = _mm_add_epi16(v30, v20);
  __m128i v38 = _mm_sub_epi16(v31, v25);
  __m128i v48_2 = _mm_unpacklo_epi16(v34, v38);
  __m128i v36 = _mm_add_epi16(v32, v18);
  __m128i v40 = _mm_sub_epi16(v29, v16);
  __m128i v48_6 = _mm_unpacklo_epi16(v36, v40);
  __m128i v49_4 = _mm_unpacklo_epi16(v48_2, v48_6);
  __m128i v50_0 = _mm_unpacklo_epi16(v49_0, v49_4);
  __m128i v49_1 = _mm_unpackhi_epi16(v48_0, v48_4);
  __m128i v49_5 = _mm_unpackhi_epi16(v48_2, v48_6);
  __m128i v50_2 = _mm_unpacklo_epi16(v49_1, v49_5);
  __m128i v78 = _mm_srai_epi16(v50_2, 2);
  __m128i v79 = _mm_add_epi16(v50_2, v78);
  __m128i v81 = _mm_add_epi16(v50_0, v79);
  __m128i v50_3 = _mm_unpackhi_epi16(v49_1, v49_5);
  __m128i v62 = _mm_srai_epi16(v50_3, 2);
  __m128i v50_1 = _mm_unpackhi_epi16(v49_0, v49_4);
  __m128i v59 = _mm_add_epi16(v50_1, v50_3);
  __m128i v61 = _mm_srai_epi16(v59, 2);
  __m128i v66 = _mm_add_epi16(v59, v61);
  __m128i v65 = _mm_srai_epi16(v59, 4);
  __m128i v67 = _mm_sub_epi16(v66, v65);
  __m128i v68 = _mm_add_epi16(v62, v67);
  __m128i v85 = _mm_add_epi16(v81, v68);
  __m128i v93 = _mm_srai_epi16(v85, 6);
  _mm_store_si128((__m128i * RADRESTRICT)(v0 + 0), v93);

  {
  int v1 = out_stride;
  char unsigned * RADRESTRICT v95 = v0 + v1;
  __m128i v80 = _mm_srai_epi16(v50_2, 1);
  __m128i v82 = _mm_add_epi16(v50_0, v80);
  __m128i v63 = _mm_srai_epi16(v50_3, 4);
  __m128i v64 = _mm_sub_epi16(v50_3, v63);
  __m128i v71 = _mm_sub_epi16(v64, v62);
  __m128i v60 = _mm_sub_epi16(v50_1, v50_3);
  __m128i v72 = _mm_add_epi16(v71, v60);
  __m128i v86 = _mm_add_epi16(v82, v72);
  __m128i v96 = _mm_srai_epi16(v86, 6);
  _mm_store_si128((__m128i * RADRESTRICT)(v95 + 0), v96);

  {
  char unsigned * RADRESTRICT v98 = v95 + v1;
  __m128i v83 = _mm_sub_epi16(v50_0, v80);
  __m128i v74 = _mm_srai_epi16(v60, 2);
  __m128i v75 = _mm_sub_epi16(v60, v74);
  __m128i v73 = _mm_srai_epi16(v60, 4);
  __m128i v76 = _mm_sub_epi16(v75, v73);
  __m128i v77 = _mm_sub_epi16(v76, v50_3);
  __m128i v87 = _mm_add_epi16(v83, v77);
  __m128i v99 = _mm_srai_epi16(v87, 6);
  _mm_store_si128((__m128i * RADRESTRICT)(v98 + 0), v99);

  {
  char unsigned * RADRESTRICT v101 = v98 + v1;
  __m128i v84 = _mm_sub_epi16(v50_0, v79);
  __m128i v69 = _mm_add_epi16(v64, v62);
  __m128i v70 = _mm_sub_epi16(v61, v69);
  __m128i v88 = _mm_add_epi16(v84, v70);
  __m128i v102 = _mm_srai_epi16(v88, 6);
  _mm_store_si128((__m128i * RADRESTRICT)(v101 + 0), v102);

  {
  char unsigned * RADRESTRICT v104 = v101 + v1;
  __m128i v89 = _mm_sub_epi16(v84, v70);
  __m128i v105 = _mm_srai_epi16(v89, 6);
  _mm_store_si128((__m128i * RADRESTRICT)(v104 + 0), v105);

  {
  char unsigned * RADRESTRICT v107 = v104 + v1;
  __m128i v90 = _mm_sub_epi16(v83, v77);
  __m128i v108 = _mm_srai_epi16(v90, 6);
  _mm_store_si128((__m128i * RADRESTRICT)(v107 + 0), v108);

  {
  char unsigned * RADRESTRICT v110 = v107 + v1;
  __m128i v91 = _mm_sub_epi16(v82, v72);
  __m128i v111 = _mm_srai_epi16(v91, 6);
  _mm_store_si128((__m128i * RADRESTRICT)(v110 + 0), v111);

  {
  char unsigned * RADRESTRICT v113 = v110 + v1;
  __m128i v92 = _mm_sub_epi16(v81, v68);
  __m128i v114 = _mm_srai_epi16(v92, 6);
  _mm_store_si128((__m128i * RADRESTRICT)(v113 + 0), v114);
  }
  }
  }
  }
  }
  }
  }
  }
}

static CODEGEN_ATTR void int_idct_fill(char unsigned * RADRESTRICT out_ptr, int out_stride, char unsigned * RADRESTRICT in_ptr)
{
  {
  char unsigned * RADRESTRICT v0 = (char unsigned * RADRESTRICT)(out_ptr);
  char unsigned * RADRESTRICT v2 = (char unsigned * RADRESTRICT)(in_ptr);
  __m128i v3 = _mm_loadu_si128((__m128i *)(v2 + 0));
  __m128i v4 = _mm_shuffle_epi32(_mm_unpacklo_epi16(v3, v3), _MM_SHUFFLE(0, 0, 0, 0));
  __m128i v5 = _mm_srai_epi16(v4, 6);
  _mm_store_si128((__m128i * RADRESTRICT)(v0 + 0), v5);

  {
  int v1 = out_stride;
  char unsigned * RADRESTRICT v7 = v0 + v1;
  _mm_store_si128((__m128i * RADRESTRICT)(v7 + 0), v5);

  {
  char unsigned * RADRESTRICT v9 = v7 + v1;
  _mm_store_si128((__m128i * RADRESTRICT)(v9 + 0), v5);

  {
  char unsigned * RADRESTRICT v11 = v9 + v1;
  _mm_store_si128((__m128i * RADRESTRICT)(v11 + 0), v5);

  {
  char unsigned * RADRESTRICT v13 = v11 + v1;
  _mm_store_si128((__m128i * RADRESTRICT)(v13 + 0), v5);

  {
  char unsigned * RADRESTRICT v15 = v13 + v1;
  _mm_store_si128((__m128i * RADRESTRICT)(v15 + 0), v5);

  {
  char unsigned * RADRESTRICT v17 = v15 + v1;
  _mm_store_si128((__m128i * RADRESTRICT)(v17 + 0), v5);

  {
  char unsigned * RADRESTRICT v19 = v17 + v1;
  _mm_store_si128((__m128i * RADRESTRICT)(v19 + 0), v5);
  }
  }
  }
  }
  }
  }
  }
  }
}

