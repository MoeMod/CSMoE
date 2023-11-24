// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

There are no build projects supplied for Bink (we use an internal build system).

Here are the switches we use (the defines are critical):

-arch arm64 -c -pipe -fvisibility=hidden -fno-strict-aliasing -fno-omit-frame-pointer -fno-vectorize -fno-unroll-loops -Wall -Wshorten-64-to-32 -Wno-overriding-t-option -Wno-unused-value -Wno-unused-variable -Wno-unused-label -Wno-unused-function -Wno-unused-local-typedef -Wno-self-assign -Wno-parentheses -miphoneos-version-min=10.3 -DNDEBUG -O2 -D__RADFINAL__ -D USING_EGT -DWRAP_PUBLICS=Bink   -DBINKTEXTURESINDIRECTBINKCALLS -DBINKTEXTURESGPUAPITYPE -DBUILDING_FOR_UNREAL_ONLY  -DINC_BINK2 -DSUPPORT_BINKGPU -DNO_BINK20 -DNO_BINK10_SUPPORT -DNO_BINK_BLITTERS -Wno-unknown-attributes -Wno-missing-noescape -Wno-nullability-completeness-on-arrays -Wno-objc-property-no-attribute -Wno-deprecated-register -Wno-address-of-packed-member  -Wno-pragma-pack -momit-leaf-frame-pointer -mllvm -inline-threshold=64 -D__RADINSTATICLIB__ 