// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

There are no build projects supplied for Bink (we use an internal build system).

Here are the switches we use (the defines are critical):

-DANDROID_NDK -DANDROID -D__ANDROID_API__=21 -Wno-multichar -Wno-unused-value -Wno-unused-variable -Wno-unused-label -Wno-comment -Wno-duplicate-decl-specifier -Wno-address-of-packed-member -Wno-deprecated-register -Wno-switch -fno-exceptions -fno-unwind-tables -fno-strict-aliasing -fno-omit-frame-pointer -fno-vectorize -fvisibility=hidden -fno-unroll-loops -fsigned-char -fwrapv -ffunction-sections -fstack-protector-strong -fno-short-enums -fno-rtti -fpic -O3  -DNDEBUG -D__RADFINAL__ -D USING_EGT -DWRAP_PUBLICS=Bink -DBINKTEXTURESINDIRECTBINKCALLS -DBINKTEXTURESGPUAPITYPE -DBUILDING_FOR_UNREAL_ONLY  -DINC_BINK2 -DSUPPORT_BINKGPU -DNO_BINK20 -DNO_BINK10_SUPPORT -DNO_BINK_BLITTERS  -target aarch64-none-linux-android -momit-leaf-frame-pointer -mllvm -inline-threshold=64 -D__RADINDLL__ 