#pragma once

// 横向迭代专用，ENUM_PARAMS(x, 3) => x1, x2, x3

#define ENUM_PARAMS_0(x)
#define ENUM_PARAMS_1(x) x ## 1
#define ENUM_PARAMS_2(x) ENUM_PARAMS_1(x), x ## 2
#define ENUM_PARAMS_3(x) ENUM_PARAMS_2(x), x ## 3
#define ENUM_PARAMS_4(x) ENUM_PARAMS_3(x), x ## 4
#define ENUM_PARAMS_5(x) ENUM_PARAMS_4(x), x ## 5
#define ENUM_PARAMS_6(x) ENUM_PARAMS_5(x), x ## 6
#define ENUM_PARAMS_7(x) ENUM_PARAMS_6(x), x ## 7
#define ENUM_PARAMS_8(x) ENUM_PARAMS_7(x), x ## 8
#define ENUM_PARAMS_9(x) ENUM_PARAMS_8(x), x ## 9
#define ENUM_PARAMS_10(x) ENUM_PARAMS_9(x), x ## 10
#define ENUM_PARAMS_11(x) ENUM_PARAMS_10(x), x ## 11
#define ENUM_PARAMS_12(x) ENUM_PARAMS_11(x), x ## 12
#define ENUM_PARAMS_13(x) ENUM_PARAMS_12(x), x ## 13
#define ENUM_PARAMS_14(x) ENUM_PARAMS_13(x), x ## 14
#define ENUM_PARAMS_15(x) ENUM_PARAMS_14(x), x ## 15
#define ENUM_PARAMS_16(x) ENUM_PARAMS_15(x), x ## 16
#define ENUM_PARAMS_17(x) ENUM_PARAMS_16(x), x ## 17
#define ENUM_PARAMS_18(x) ENUM_PARAMS_17(x), x ## 18
#define ENUM_PARAMS_19(x) ENUM_PARAMS_18(x), x ## 19
#define ENUM_PARAMS_20(x) ENUM_PARAMS_19(x), x ## 20
#define ENUM_PARAMS_21(x) ENUM_PARAMS_20(x), x ## 21
#define ENUM_PARAMS_22(x) ENUM_PARAMS_21(x), x ## 22
#define ENUM_PARAMS_23(x) ENUM_PARAMS_22(x), x ## 23
#define ENUM_PARAMS_24(x) ENUM_PARAMS_23(x), x ## 24
#define ENUM_PARAMS_25(x) ENUM_PARAMS_24(x), x ## 25
#define ENUM_PARAMS_26(x) ENUM_PARAMS_25(x), x ## 26
#define ENUM_PARAMS_27(x) ENUM_PARAMS_26(x), x ## 27
#define ENUM_PARAMS_28(x) ENUM_PARAMS_27(x), x ## 28
#define ENUM_PARAMS_29(x) ENUM_PARAMS_28(x), x ## 29
#define ENUM_PARAMS_30(x) ENUM_PARAMS_29(x), x ## 30
#define ENUM_PARAMS_31(x) ENUM_PARAMS_30(x), x ## 31
#define ENUM_PARAMS_32(x) ENUM_PARAMS_31(x), x ## 32

#define ENUM_PARAMS(x, N) ENUM_PARAMS_##N(x)