## Counter-Strike Mobile-oriented Edition
本项目提供了 Counter-Strike Mobile-oriented Edition(下称'CSMoE')的 引擎, 服务端, 以及客户端的源代码(仅Windows及Linux). \
关于 Android 系统或 iOs 系统的 CSMoE 启动器, 请查看其他项目. \
CSMoE 是一个基于 Xash3D 引擎的 CS1.6 Mod 合集, \
旨在让其还原 CSO (又名 CS:NZ), 在 2009 年初期兴起的一款在线CS游戏. \
CSMoE 可以跨平台运行, 支持的平台包括: Windows(x86,x64), Linux(x86), macOS(x64), Android(ARMv7,x86), iOS(ARMv7,ARM64).

## 功能
* [x] 全新的 Hud 元素 (计分板, 击杀效果, 可跟随的C4 TGA, 个性化准星)
* [x] 游戏模式 : 团队竞技 (TDM)
* [x] 游戏模式 : 支持 BOT 的 个人竞技 (DM)
* [x] 游戏模式 : 生化模式 1
* [x] 游戏模式 : 生化模式 2
* [ ] 游戏模式 : 生化盟约
* [x] 游戏模式 : 生化模式 3 (经典)
* [x] 游戏模式 : 支持 BOT 的 大灾变 (经典)
* [ ] 游戏模式 : 枪王之王
* [ ] 游戏模式 : 生化模式 4

## 与其他仿 CSO 模组的差别
* 没有神器和牛逼的人物
* 简单粗暴的游戏模式
* 不再使用外置 AMXX 插件, 所有功能由内置 C/C++ 代码实现
* 无 VGUI2 支持, 仅支持英语
* 终端最低配置需求较低
* 跨平台可用
* 遵循 GPL v3 协议完全开源

## 已测试平台
### Windows 10 (1703) - Surface Pro 5 (2017)
* Specs : Intel Core i5-7300U, Intel HD620, 8 GB
* Input : Mouse, Keyboard, Touchpad, Touchscreen
* Output : Display(2736 x 1824 PixelSense), Sound
* Result : 80+ fps
### Linux ( Ubuntu 19.04 ) - Google Pixelbook (2017)
* CPU : Intel Core i5-7Y57, Intel HD615, 8GB
* Input : Mouse, Keyboard, Touchpad, Touchscreen
* Output : Display(2400 x 1600), Sound
* Result : 80+ fps
### Android (8.1) - Google Pixel C (2015)
* Specs : nVIDIA Tegra X1, 4 GB
* Input : Touchscreen
* Output : Display(2560 x 1800), Sound, Vibration(not available on Pixel C)
* Result : ~90 fps
### iOS (11.2) - Apple iPhone X (2017)
* Specs : A11 Fusion
* Input : Touchscreen, 3D Touch
* Output : Display(2436 x 1125 Super-Retina), Sound, Vibration(Tapic-Engine)
* Result : ~60 fps
### macOS (10.14) - MacBook Pro (2018)
* Specs : Intel Core i5-8259U, Intel Iris Plus 655, 8GB
* Input : Mouse, Keyboard, Touchpad, Multi-touch Bar
* Output : Display(2560 x 1600 Retina), Sound
* Result : 80+ fps

## 安装
### Windows
1. 下载 ***-win32-i386 和 ***-GameDir 并将他们解压到一起
2. 在 Steam 上购买并下载 Counter-Strike (请支持正版!)
3. 复制 'cstrike' 和 'valve' 目录至 CSMoE 目录下
4. 运行 csmoe_win32.exe \
Note : 如果有关于 MSVCR140.dll 或有类似的错误, 点击下面链接安装 Microsoft VC++ Redist 即可 \
https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads
### Linux
1. 下载 ***-linux-i386 and ***-GameDir and 并将他们解压到一起
2. 在 Steam 上购买并下载 Counter-Strike (请支持正版!)
3. 复制 'cstrike' 和 'valve' 目录至 CSMoE 目录下
4. 运行命令号, cd 到 CSMoE 目录, 输入 
  sudo chmod +x xash3d
5. 输入以下命令即可运行游戏
  bash csmoe_linux.sh \
Note : 你可能需要安装 libSDL2 以及其他需求插件
### Android
1. 下载 ***-GameDir 并将它解压到手机内的一个空文件夹内
2. 在 Steam 上购买并下载 Counter-Strike (请支持正版!)
3. 复制 'cstrike' 和 'valve' 目录至 CSMoE 目录下
4. 下载并安装 ***-Android.apk
5. 跟随说明运行游戏
### iOS
1. download and install the ipa using Impactor 
  * armv7 for iPhone 5, iPad 4, iPod Touch 5 or earlier
  * arm64 for iPhone 5s, iPad Air 1, iPod Touch 6 or newer
2. 下载并解压 ***-GameDir
2. 在 Steam 上购买并下载 Counter-Strike (请支持正版!)
4. find cstrike and valve directory in steamapps
5. launch iTunes and select the APP in file sharing
6. drag csmoe, cstrike, valve into iTunes
7. run the app on the SpringBoard \
Note : Thanks to the fxxking App Store limitations, you must reinstall it every week or it will crash (no need to recopy resources).
### macOS
1. download ***-macos-x64 and ***-GameDir and extract them together
2. purchase and download Counter-Strike on Steam
3. copy cstrike and valve directory to where the csmoe locates
4. run csmoe_macos.sh with Terminal \
Note : If it results in a black window, try to drag it and then the menu will appear.

## Building
### Generic
- This project uses C++11 standard, and please ensure that your compiler supports it.
- Tested compilers : MSVC 19.10, AppleClang 10, ICC 19, GCC 8.1
### Windows
- Visual Studio 2017 projects can be found in msvc/csmoe.sln
- Download SDL2-devel-2.0.9-VC.zip from http://www.libsdl.org/ and put them into SDL2/
- It should be OK to build with both x86 and x64. \
  Note : You can also use cmake to build with mingw, etc.
### macOS & Linux
- Build with CMake
- ~~Set SDL2_PATH to where the SDL2 locates~~
- Install packages : libfontconfig1-dev libsdl2-dev
- It should be OK to build with both x86 and x64.
### Android
- Put this repo into jni/src/xash3d
- ~~Build APK with elipse, Android SDK level 21, Android NDK r12b or maybe newer version~~
- Build APK with Android Studio, tested with Android SDK level 28 and Android NDK r20
- arm64-v8a is not supported. Please build armeabi-v7a, x86 only.
### iOS
- please refer to mittorn/xash3d-ios
- supports both armv7, arm64.

## 协议
本项目遵循 GPLv3 协议, 您可以点击目录下的 'LICENSE' 文件获取更多信息.

## 参与合作
* star / fork
* 说明哪里有问题 & 什么需要被添加
* commit 可行方案 \

 如果您不是开发者或无法读懂此说明, 可以添加我们的QQ群: 3266216 获取更多信息.
