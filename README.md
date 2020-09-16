[![Build Status](https://travis-ci.com/MoeMod/CSMoE.svg?branch=master)](https://travis-ci.com/MoeMod/CSMoE) [![Build status](https://ci.appveyor.com/api/projects/status/7e8u0ssq80vpy4ba/branch/master?svg=true)](https://ci.appveyor.com/project/MoeMod/csmoe/branch/master) \
[English](#Counter-Strike_Mobile-oriented_Edition) | [Benchmark](#Benchmark) | [Supported Platforms](#Platforms) \
[中文(简体)](#关于本项目) | [测试](#Benchmark) | [支援平台](#Platforms)

## Counter-Strike Mobile-oriented Edition
CSMoE is a project that based on Xash3D Engine and CS1.6 Mods to restore like Counter-Strike Online (CSO) or Counter-Strike Nexon: Studio (CSN:S known as CSN:Z)

What makes this special, Because CSMoE are cross-platform designed. Click [Here](#Platforms) To See Supported Platform.

This project includes CSMoE's Engine, Server and Client.

## Current Features
* [x] New HUD elements (ScoreBoard, KillerEffects, C4 follow icons, Custom Crosshair)
* [x] GameMode : Team Deathmatch (TDM)
* [x] GameMode : Deathmatch (DM) with BOT support
* [x] GameMode : Zombie Mod 1
* [x] GameMode : Zombie Mod 2
* [ ] GameMode : Zombie United
* [x] GameMode : Zombie Mod 3 (Classic)
* [x] GameMode : Zombie Scenario (Classic) with BOT support
* [ ] GameMode : GunDeath
* [ ] GameMode : Zombie Mod 4

## Differ with other CSO-like mods
* No dominating weapons and immortal player classes
* Simplified gameplay
* No AMXX, all written as C/C++ code 
* Lightweight implement, VGUI2-less
* IMGUI Support
* English only
* Support low-end specs
* Cross-platform
* Fully open-source under GPLv3 licence

## Installation
Go [Here](https://github.com/MoeMod/CSMoE/releases) to download client and [Here](https://github.com/MoeMod/CSMoE-GameDir/archive/master.zip) for resources.

Note: win32(or x86) and win32-x64(or x86_64) is different. Please watch carefully.
### Windows (Win32)
1. Purchase and download Counter-Strike 1.6 on Steam
2. Download client and resource and extract them out
3. Copy steamapps\common\Half-Life cstrike and valve directory and GameDir\csmoe to where the csmoe locates
4. Run csmoe_win32.exe

Note : If you get errors about MSVCR140.dll and so on, download Microsoft VC++ Redist on [Here](https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads), and install all of x86 and x84. (ARM64 user should download ARM64?)

### Windows (UWP)
Now CI not building. Build it by yourself.

1. Purchase and download Counter-Strike 1.6 on Steam
2. Download client and resource and extract them out
3. Install the .cer to system-wide Trusted Root Certification Authorities certificate store
4. Install *.appx or *.appxbundle
5. Launch from Start Menu.

### Linux
1. Purchase and download Counter-Strike 1.6 on Steam
2. Download client and resource and extract them out
3. Copy steamapps\common\Half-Life cstrike and valve directory and GameDir\csmoe to where the csmoe locates
4. Launch terminal, cd to the dir, type 
  sudo chmod +x xash3d
5. type the cmd below to run the game
  bash csmoe_linux.sh \
Note : You may need to install libSDL2 and other dependencies.

### Android
1. Purchase and download Counter-Strike 1.6 on Steam
2. Download client and resource and extract them out
3. Copy steamapps\common\Half-Life cstrike and valve directory and GameDir\csmoe to where the csmoe locates
4. Download and install ***-Android.apk
5. Run the game and follow the instructions

### iOS
1. Purchase and download Counter-Strike 1.6 on Steam
2. Download client and resource and extract them out
3. Install the ipa using Impactor 
  * armv7 for iPhone 5, iPad 4, iPod Touch 5 or earlier
  * arm64 for iPhone 5s, iPad Air 1, iPod Touch 6 or newer
  * universal for both armv7 and arm64
4. Find steamapps\common\Half-Life cstrike and valve directory
5. Launch iTunes and select the APP in file sharing
6. Drag csmoe, cstrike, valve into iTunes
7. Run the app from SpringBoard

Note : You must reinstall it every week or it will crash (no need to recopy resources). ~~Thanks to fxxking limitations~~

Note : Due Impactor not updating right now, iOS 13 and newer has no way to installing it. So figure it out by yourself. **Not very recommend users jailbreaking for installing.**

### macOS
1. Purchase and download Counter-Strike 1.6 on Steam
2. Download client and resource and extract them out
3. Copy steamapps\common\Half-Life cstrike and valve directory and GameDir\csmoe to where the csmoe locates
4. Run csmoe_macos.sh with Terminal \
Note : If it results in a black window, try to drag it and then the menu will appear.

## Building
### Generic
- This project uses C++14 standard, and please ensure that your compiler supports it.
- Tested compilers : MSVC 19.10, AppleClang 10, ICC 19, GCC 8.1
- Project In Use:
  - SDL2 2.0.10 (Note: If newer version compatible, please feedback to us)

### Windows (UWP)
- Requirement:
  - Visual Studio 2017 or above
  - Universal Windows Platform Deployment (UWP)
  - C++ Deployment
  - Windows SDK (10.0.18362.0 as known as 1803) (Maybe require VS2019 to download and install it?)
- Download SDL2-2.0.10.zip from http://www.libsdl.org/download-2.0.php , extract and rename folder to SDL2/
- Projects can be found in msvc/csmoe.sln

### Windows (Win32)
- Requirement:
  - Visual Studio 2017 or above
  - C++ Deployment
  - Windows SDK (10.0.18362.0 as known as 1803) (Maybe require VS2019 to download and install it?)
  - CMake Support
- Download SDL2-2.0.10.zip from http://www.libsdl.org/download-2.0.php , extract and rename folder to SDL2/
- Open VS and use File>Open>CMake to select CMakeLists.txt or install CMake and launch CMake GUI to generate.
  Note : You can also use cmake to build with mingw, etc.

### macOS & Linux
- Build with CMake
- Install packages : libfontconfig1-dev libsdl2-dev
- It should be OK to build with both x86 and x64.
  
### Android
- Project Folder is 'Android' folder
- Build APK with Android Studio, tested with Android SDK level 28 and Android NDK r20

### iOS
- Projects can be found in Xcode-iOS/CSMoE-iOS.xcodeproj
- Download SDL2-2.0.10.zip from http://www.libsdl.org/download-2.0.php , extract and rename folder to SDL2/
- Supports armv7 and arm64

## License
This repo is licensed under GPLv3 license, check [here](https://github.com/MoeMod/CSMoE/blob/master/LICENSE.md) for more details.

## Contributing
Feel free to star/fork this project if you love it.

Currently there has not any official group or community.

For developer and committer:
 - Make a pull request if you can fix something that we're not fixing yet.
 - Write human readable code.
 - Test it on your own at least 1 platform. (Mainly focus on Windows and Android first)
 - Summit a issue to talk about related fixes if requires.

For player:
 - Summit a issue if there has any errors that we haven't fix yet or reported by other players.
 - Write more detail messages for us. (It's better if there has picture or video)

## 关于本项目
本项目 Counter-Strike Mobile-oriented Edition(下称'CSMoE') 是一款基于Xash3D引擎做出的CS1.6 Mod, 旨在让其还原CSO(又名 CSN:S 或 CSN:Z), 在 2009 年初期兴起的一款在线CS游戏 \
同时CSMoE可以跨平台运行, 前往这里查看[支援平台](#Platforms)

该项目包含了引擎，客户端以及服务端的源代码

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
* 不使用外置 AMXX 插件, 所有功能由内置 C/C++ 代码实现
* 无 VGUI2 支持
* 新的GUI: IMGUI
* 仅支持英语
* 支持低端规格
* 可跨平台
* 遵循 GPL v3 协议完全开源

## 安装
到[此处](https://github.com/MoeMod/CSMoE/releases)下载客户端, 然后到[此处](https://github.com/MoeMod/CSMoE-GameDir/archive/master.zip)获取资源

注意: Win32(或x86)和Win32_x64(或x86_64)是不同的，请看清楚。

### Windows (Win32)
1. 在 Steam 上购买并下载 Counter-Strike 1.6 (请支持正版!)
2. 下载客户端和资源并解压到一起
3. 复制 steamapps\common\Half-Life里的 'cstrike' 和 'valve' 和 GameDir\csmoe 目录至 CSMoE 目录下
4. 运行 csmoe_win32.exe \
Note : 如果有关于 MSVCR140.dll 或有类似的错误, 点击[这里](https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads)下载 Microsoft VC++ Redist，并且安装x86以及x64。

### macOS
1. 在 Steam 上购买并下载 Counter-Strike 1.6 (请支持正版!)
2. 下载客户端和资源并解压到一起
3. 复制 steamapps\common\Half-Life里的 'cstrike' 和 'valve' 和 GameDir\csmoe 目录至 CSMoE 目录下
4. 以管理员权限运行 csmoe_macos.sh \
Note : 如果运行后黑屏, 试试看拖拉窗口, 之后菜单应该就出来了.

### Linux
1. 在 Steam 上购买并下载 Counter-Strike 1.6 (请支持正版!)
2. 下载客户端和资源并解压到一起
3. 复制 steamapps\common\Half-Life里的 'cstrike' 和 'valve' 和 GameDir\csmoe 目录至 CSMoE 目录下
4. 运行命令号, cd 到 CSMoE 目录, 输入 
  sudo chmod +x xash3d
5. 输入以下命令即可运行游戏
  bash csmoe_linux.sh \
Note : 你可能需要安装 libSDL2 以及其他需求插件

### Android
1. 在 Steam 上购买并下载 Counter-Strike 1.6 (请支持正版!)
2. 下载客户端和资源并解压到一起
3. 复制 steamapps\common\Half-Life里的 'cstrike' 和 'valve' 和 GameDir\csmoe 目录至 CSMoE 目录下
4. 下载并安装 ***-Android.apk
5. 跟随说明运行游戏

### iOS
1. 在 Steam 上购买并下载 Counter-Strike 1.6 (请支持正版!)
2. 下载客户端和资源并解压到一起
3. 使用 Impactor 安装 ipa
  * armv7 <- iPhone 5, iPad 4, iPod Touch 5 及更早的平台
  * arm64 <- iPhone 5s, iPad Air 1, iPod Touch 6 及更早的平台
  * universal <- armv7与arm64通用
4. 寻找 steamapps\common\Half-Life里的 'cstrike' 和 'valve' 和 GameDir\csmoe 目录
5. 运行 iTunes 并在分享文件内选择 APP
6. 将 csmoe, cstrike, valve 拉到 iTunes 内
7. 在桌面运行 CSMoE \
Note: 你必须每周安装一次 CSMoE , 否则它会崩溃 (不需要重新复制资源). ~~由于傻逼 AppStore 的限制~~

Note: 由于Impactor至今未更新，所以iOS 13以上都没办法安装，请自行摸索。**不建议为了安装游戏而Jailbreak来安装**

## 工程
### 通常
- 本项目使用 C++14 标准, 请确保你的编译器支持它.
- 已测试的编译器 : MSVC 19.10, AppleClang 10, ICC 19, GCC 8.1
- 使用中的项目
  - SDL2 2.0.10 (注：若新版本能通过编译且没问题请汇报)

### Windows 10 UWP
- 由于#133 缘故, 以后的Windows项目会编译成UWP以代替Win32, 请确保你是Windows 10用户以及安装了UWP开发
- 下载 SDL2-2.0.10.zip (http://www.libsdl.org/download-2.0.php) 并将文件夹改成SDL2放到目录里
- 打开msvc\csmoe.sln即可

### Windows 10 Win32
- 由于#133 缘故, 请使用CMake来生成Win32项目. 
- 下载 SDL2-2.0.10.zip (http://www.libsdl.org/download-2.0.php) 并将文件夹改成SDL2放到目录里
- 通过CMake生成路径打开sln即可
- 本项目需要使用OpenGL 1.1，因此不支持arm32及arm64原生编译，arm64可利用x86模拟器运行。

### macOS & Linux
- 通过 CMake 载入工程
- 安装: libfontconfig1-dev以及libsdl2-dev
- 一般来说这样就可以在 x86 和 x64 环境下正常运行了.

### Android
- 安卓 APP 项目在 Android 文件夹内
- 通过 Android Studio 来构建 App, 建议使用 Android SDK level 28 和 Android NDK r20 测试.

### iOS
- iOS APP 项目在 Xcode-iOS 文件夹内
- 通过 Xcode 打开项目并编译、部署、调试
- 支持 armv7, arm64.

## 协议
本项目遵循 GPLv3 协议, 您可以点击[这里](https://github.com/MoeMod/CSMoE/blob/master/LICENSE.md)获取更多信息.

## 参与合作
若你喜欢，欢迎点击Star或者Fork！

目前没有任何官方群或者组织，请前往[csoldjb吧](https://tieba.baidu.com/f?kw=csoldjb)获取更多信息。

给制作者或提交者:
 - 如果你能帮助我们修复问题，请打开PR帮助我们修复
 - 写人能懂的代码
 - 在本地测试可否使用且其他平台没多大问题 (主要专注Windows和安卓)
 - 若需要讨论相关问题，打开Issue来进行讨论

给玩家:
 - 打开Issue来提交任何你在游戏上遇到的问题，同时请确保其他玩家也没提交的以防重复问题。
 - 详细的描述你的问题（若能提供照片或视频是最好的）

# Other
## Platforms
| |x86 | x64 | armv7 | arm64 |
|:----:|:----:|:----:|:----:|:----:|
| Windows Desktop|√|√|×|×
| Windows 10 UWP|×|√|√|?
| macOS|?|√|×|?
| Linux|√|√|?|?
| iOS|×|×|√|√
| Android|√|√|√|√

``` 
√ - Supported
? - Maybe Support
× - Not Supported
```

## Benchmark
### Windows 10 UWP x64 - Surface Book 2 (2017)
* Specs : Intel Core i5-8650U, NVIDIA GTX 1050, 8 GB
* Input : Mouse, Keyboard, Touchpad, Touchscreen
* Output : Display(3000 x 2000), Sound
* Result : 60 fps
### Windows 10 UWP ARM32 - Surface Pro X (2019)
* Specs : Microsoft SQ1 (aka 8cx)
* Input : Mouse, Keyboard, Touchpad, Touchscreen
* Output : Display(2880 x 1920), Sound
* Result : ~50 fps - Good enough.
### Windows 10 UWP ARM32 - Surface RT (2012)
* Specs : Nvidia Tegra 2
* Input : Mouse, Keyboard, Touchpad, Touchscreen
* Output : Display(1366 x 768), Sound
* Result : 3 fps - A little closer to playable
### Windows 10 Desktop (1703) - Surface Pro 5 (2017)
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
