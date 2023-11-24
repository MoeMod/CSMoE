// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

There are no build projects supplied for Bink (we use an internal build system).

Here are the switches we use (the defines are critical):

-pipe -x none -msse -msse2 -fno-stack-protector -fno-omit-frame-pointer -ffast-math -Wno-unused-function -finline  -fwrapv -fdata-sections -ffunction-sections -D_REENTRANT  -fno-strict-aliasing -fvisibility=hidden -fno-exceptions -Wno-unused-variable -Wno-comment -Wno-unused-function -Wno-unused-parameter -Wno-extra -Wno-address-of-packed-member -Wno-self-assign -Wno-unknown-warning-option -Wno-multichar -Wno-unused-label -Wno-unused-but-set-variable -Wno-unused-local-typedefs -fno-unroll-loops -Wno-deprecated-register -Wno-missing-braces -fno-rtti -D_GLIBCXX_USE_CXX11_ABI=0 -m64 --target=x86_64-unknown-linux-gnu -mbranches-within-32B-boundaries -O3 -DNDEBUG -fno-tree-vectorize -fno-slp-vectorize -D__RADFINAL__ -D USING_EGT -DWRAP_PUBLICS=Bink -D__RAD_NDA_PLATFORM__=stadia  -DBINKTEXTURESINDIRECTBINKCALLS -DBINKTEXTURESGPUAPITYPE -DBUILDING_FOR_UNREAL_ONLY  -DINC_BINK2 -DSUPPORT_BINKGPU -DNO_BINK20 -DNO_BINK10_SUPPORT -DNO_BINK_BLITTERS -DSTADIA -D__STADIA__ -D__ggp__  -D__RADINSTATICLIB__  -momit-leaf-frame-pointer -mllvm -inline-threshold=64 