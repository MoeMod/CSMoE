// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

@echo off
setlocal

rem set your paths here
set MSVC15=c:\devel\vs15
set Win10SDK=c:\Program Files (x86)\Windows Kits\10\include\10.0.16299.0
set VulkanDir=c:\devel\libs\vulkan

set path=%MSVC15%\vc\bin\x86_amd64;%MSVC15%\VC\bin\amd64;%path%

"%MSVC15%\vc\bin\amd64_x86\cl.exe"  *.c *.cpp -nologo -c -Oi -GF -Gy -Gw -Gm- -Oy- -GR- -fp:except- -EHs- -EHc- -WX -W4 -Zl -wd4456 -wd4457 -wd4748 -wd4459 -wd4201 -wd4996 -wd4740 -wd4725 -wd4100 -Z7 -Oxb2 -O2 -arch:SSE -EHa- -Gs- -GS -Zp8 -DWIN32 -D_WIN32 -DNDEBUG  -D__RADFINAL__  -D USING_EGT  -DWRAP_PUBLICS=Bink    -DBINKTEXTURESINDIRECTBINKCALLS -DBINKTEXTURESGPUAPITYPE -DBUILDING_FOR_UNREAL_ONLY   -DBINK2W64DLL -DINC_BINK2 -DSUPPORT_BINKGPU -DNO_BINK20 -DNO_BINK10_SUPPORT -DNO_BINK_BLITTERS -D__RADINSTATICLIB__ -D_CRT_SECURE_NO_DEPRECATE -D_SECURE_SCL=0 -D_HAS_ITERATOR_DEBUGGING=0 -D_SCL_SECURE_NO_WARNINGS -D_USING_V110_SDK71_=1 -I"%VulkanDir%\include" -I"%MSVC15%\vc\include" -I"%Win10SDK%\ucrt" -I"%Win10SDK%\um" -I"%Win10SDK%\shared"


