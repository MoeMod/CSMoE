## Counter-Strike Mobile-oriented Edition
Here are the source code of engine, server, client (windows and linux only). \
For Android or iOS launcher, check them in other repo. \
CSMoE is a combination of CS1.6 mods that are implemented based on Xash3D engine, \
aiming to act like CSO (aka CS:NZ), however the traditional one in pre-2009. \
Cross-platform designed, it can be run on Windows(x86,x64), Linux(x86), macOS(x64), Android(ARMv7,x86), iOS(ARMv7,ARM64).

## Features
* [x] New HUD elements (ScoreBoard, KillerEffects, C4 follow icons, Custom Crosshair)
* [x] GameMode : TeamDeathmatch (TDM)
* [x] GameMode : Deathmatch (DM) with BOT support
* [x] GameMode : Zombie Mod 1
* [x] GameMode : Zombie Mod 2
* [ ] GameMode : Zombie United
* [x] GameMode : Zombie Mod 3 (classic)
* [x] GameMode : Zombie Scenario (classic) with BOT support
* [ ] GameMode : GunDeath
* [ ] GameMode : Zombie Mod 4

## Difference from other CSO-like mods now
* No more dominating weapons and immortal player classes
* Simplified gameplay
* Without AMXX, all C/C++ native code instead
* No VGUI2 support, English only
* Low-end system requirements
* Cross-platform
* Fully open-source under GPLv3 licence

## Tested systems
### Windows 10 ( Version 1703 ) - Surface Pro 5 (2017)
* CPU : Intel Core i5-7300U
* GPU : Intel HD620
* RAM : 8 GB
* Resolution : 2736 x 1824 PixelSense
* Result : Smooth but hot, 80+ fps
### Linux ( Ubuntu 19.04 ) - Google Pixelbook (2017)
* CPU : Intel Core i5-7Y57
* GPU : Intel HD615
* Resolution : 2400 x 1600
* RAM : 8 GB
* Result : 60+ fps
### Android - Google Pixel C (2015)
* SoC : nVIDIA Tegra X1
* RAM : 4 GB
* Resolution : 2560 x 1800
* Result : Smooth, 90+ fps
### iOS - Apple iPad 6th Gen (2018)
* SoC : A10 Fusion
* Resolution : 2048 x 1536 Retina
* Result : Smooth, 55~60fps
### macOS - MacBook Pro (2018)
* CPU : Intel Core i5-8259U
* GPU : Intel Iris Plus 655
* RAM : 8GB
* Resolution : 2560 x 1600 Retina
* Result : Smooth but hot, 80+ fps

## Installation
### Windows
1. download ***-win32-i386 and ***-GameDir and extract them together
2. purchase and download Counter-Strike on Steam
3. copy cstrike and valve directory to where the csmoe locates
4. run csmoe_win32.exe \
Note : If you get errors about MSVCR140.dll and so on, install Microsoft VC++ Redist on \
https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads
### Linux
1. download ***-linux-i386 and ***-GameDir and extract them together
2. purchase and download Counter-Strike on Steam
3. copy cstrike and valve directory to where the csmoe locates
4. launch terminal, cd to the dir, type 
  sudo chmod +x xash3d
5. type the cmd below to run the game
  bash csmoe_linux.sh \
Note : You may need to install libSDL2 and other dependencies.
### Android
1. download ***-GameDir and extract it into an empty folder on your phone
2. purchase and download Counter-Strike on Steam
3. copy cstrike and valve directory to where the csmoe locates
4. download and install ***-Android.apk
5. run the game and follow the instructions
### iOS
1. download and install the ipa using Impactor 
  * armv7 for iPhone 5, iPad 4, iPod Touch 5 or earlier
  * arm64 for iPhone 5s, iPad Air 1, iPod Touch 6 or newer
2. download ***-GameDir and extract it
3. purchase and download Counter-Strike on Steam
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

## License
This repo is licensed under GPLv3 license, check LICENSE file for details.

## Contributing
* star / fork
* explain what's wrong & what's to be added
* commit a possibe solution \

 For non-developers, turn to QQ group 3266216 for more information.
