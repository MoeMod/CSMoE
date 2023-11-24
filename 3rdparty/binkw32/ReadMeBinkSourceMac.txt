// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

There are no build projects supplied for Bink (we use an internal build system).

Here are the switches we use (the defines are critical):

Arm lib:  -arch arm64 --target=arm64-apple-darwin-macho  -Wno-unknown-attributes -Wno-missing-noescape -Wno-nullability-completeness-on-arrays -Wno-objc-property-no-attribute -Wno-deprecated-register -Wno-address-of-packed-member  -Wno-pragma-pack -fno-rtti -O3 -fno-vectorize -fno-unroll-loops -DNDEBUG -D__RADFINAL__ -D USING_EGT -DWRAP_PUBLICS=Bink -Wno-deprecated-declarations -D__GCC__ -c -fno-omit-frame-pointer -Wno-unused-function -fvisibility=hidden -fno-strict-aliasing -fpascal-strings -fasm-blocks  -fno-common -D__MACHO__ -DTARGET_API_MAC_CARBON -DOPAQUE_UPP_TYPES -DOPAQUE_TOOLBOX_STRUCTS -mmacosx-version-min=10.7 -fno-exceptions -Wno-parentheses-equality -Wshorten-64-to-32 -D__RADINSTATICLIB__   -DBINKTEXTURESINDIRECTBINKCALLS -DBINKTEXTURESGPUAPITYPE -DBUILDING_FOR_UNREAL_ONLY  -fno-stack-protector   -DINC_BINK2 -DSUPPORT_BINKGPU -DNO_BINK20 -DNO_BINK10_SUPPORT -DNO_BINK_BLITTERS  -momit-leaf-frame-pointer -mllvm -inline-threshold=64 

x64 lib:  -arch x86_64 --target=x86_64-apple-darwin-macho  -Wno-unknown-attributes -Wno-missing-noescape -Wno-nullability-completeness-on-arrays -Wno-objc-property-no-attribute -Wno-deprecated-register -Wno-address-of-packed-member  -Wno-pragma-pack -fno-rtti -O3 -fno-vectorize -fno-unroll-loops -DNDEBUG -D__RADFINAL__ -D USING_EGT -DWRAP_PUBLICS=Bink -Wno-deprecated-declarations -D__GCC__ -mdynamic-no-pic -c -fno-omit-frame-pointer -Wno-unused-function -fvisibility=hidden -fno-strict-aliasing -fpascal-strings -fasm-blocks  -fno-common -D__MACHO__ -DTARGET_API_MAC_CARBON -DOPAQUE_UPP_TYPES -DOPAQUE_TOOLBOX_STRUCTS -fno-exceptions -Wno-parentheses-equality -Wshorten-64-to-32 -msse -msse2 -msse3 -mssse3 -fno-omit-frame-pointer -mmacosx-version-min=10.7 -D__RADINSTATICLIB__   -DBINKTEXTURESINDIRECTBINKCALLS -DBINKTEXTURESGPUAPITYPE -DBUILDING_FOR_UNREAL_ONLY  -fno-stack-protector   -DINC_BINK2 -DSUPPORT_BINKGPU -DNO_BINK20 -DNO_BINK10_SUPPORT -DNO_BINK_BLITTERS  -momit-leaf-frame-pointer -mllvm -inline-threshold=64 


