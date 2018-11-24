## About
Here is the source code of engine, server, client (windows and linux only). \
For Android or iOS launcher, check them in other repo. \
This is a combination of CS1.6 mods that are implemented based on Xash3D engine, \
aiming to act like CSO (aka CS:NZ), however the traditional one in pre-2009. \
Cross-platform designed, it can be run on Windows(x86), Linux(x86), macOS(x64), Android(ARMv7,x86), iOS(ARMv7,ARM64).

## Features
* [x] New HUD elements (ScoreBoard, KillerEffects, C4 follow icons, Custom Crosshair)
* [x] GameMode : TeamDeathmatch (TDM)
* [x] GameMode : Deathmatch (DM) with BOT support
* [x] GameMode : Zombie Mod 1
* [x] GameMode : Zombie Mod 2
* [ ] GameMode : Zombie United
* [ ] GameMode : Zombie Mod 3 (classic)
* [x] GameMode : Zombie Scenario (classic)
* [ ] GameMode : GunDeath
* [ ] UI : BuyMenu

## Difference from other CSO-like mods now
* No more dominating weapons and immortal player classes
* Simplified gameplay
* Without AMXX, all C/C++ native code instead
* No VGUI2 support, English only
* Low-end system requirements
* Cross-platform
* Fully open-source under GPLv3 licence

## Tested systems
### Windows - Surface Pro
* CPU : Intel Core i5-7300U
* GPU : Intel HD620
* RAM : 8 GB
* Resolution : 2736 x 1824
* Result : Smooth but hot, 80+fps
### Linux (in Chrome OS) - Google Pixelbook
* CPU : Intel Core i5-7Y57
* GPU : Intel HD615 (without full driver)
* RAM : 8 GB
* Result : Due to lack of OpenGL driver on Chrome OS, it can only run dedicated server.
### Android - Huawei Ascend P9
* SoC : Kirin 950
* RAM : 4 GB
* Resolution : 1920 x 1080 FHD
* Result : Playable, 40~60fps
### iOS - Apple iPhone 7
* SoC : A10 Fusion
* Resolution : 1334 x 750
* Result : Smooth, 55~60fps
### macOS - MacBook Pro (2018)
* CPU : Intel Core i5-8259U
* GPU : Intel Iris Plus 655
* RAM : 8GB
* Resolution : 2560 x 1600
* Result : Smooth but hot, 80+fps

## Installation
### Windows
1. download ***-win32-i386 and ***-GameDir and extract them together
2. purchase and download Counter-Strike on Steam
3. copy cstrike and valve directory to where the csbtem locates
4. run csbtem_win32.exe \
Note : If you get errors about MSVCR140.dll and so on, install Microsoft VC++ Redist on \
https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads
### Linux
1. download ***-linux-i386 and ***-GameDir and extract them together
2. purchase and download Counter-Strike on Steam
3. copy cstrike and valve directory to where the csbtem locates
4. launch terminal, cd to the dir, type 
  sudo chmod +x xash3d
5. type the cmd below to run the game
  bash csbtem_linux.sh \
Note : You may need to install libSDL2 and other dependency.
### Android
1. download ***-GameDir and extract it into an empty folder on your phone
2. purchase and download Counter-Strike on Steam
3. copy cstrike and valve directory to where the csbtem locates
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
6. drag csbtem, cstrike, valve into iTunes
7. run the app on the SpringBoard \
Note : Thanks to the fxxking App Store limitations, you must reinstall it every week or it will crash (no need to recopy resources).
### macOS
1. download ***-macos-x64 and ***-GameDir and extract them together
2. purchase and download Counter-Strike on Steam
3. copy cstrike and valve directory to where the csbtem locates
4. run csbtem_macos.sh with Terminal \
Note : If it results in a black window, try to drag it and then the menu will appear.

## License
This repo is licensed under GPLv3 license, check LICENSE file for details.

## Contributing
* explain what's wrong & what's to be added
* commit a possibe solution

 DO NOT commit suggestions about gameplay such as When will the new annual weapon be updated.\
 For non-developers, turn to QQ group 3266216 for more information.
