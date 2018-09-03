[English](#csbte-mobile) | [中文(简体)](#csbte-mobile项目)  

# CSBTE-Mobile
Here is the source code of CSBTE-Mobile (windows and linux only). \
For Android or iOS launcher, check them in other repo. \

## About Counter-Strike:BreakThrough Edition Mobile
CSBTE-Mobile is a combination of CS1.6 mods that are implemented based on Xash3D engine. \
Like CSBTE but not the same , it is aimed to act like CSO (aka CS:NZ), however the traditional one in pre-2009. \
Cross-platform designed, it can be run on Windows(x86), Linux(x86), Android(ARMv7,x86), iOS(ARMv7,ARM64).

## Features
* [x] New HUD elements (ScoreBoard, KillerEffects, C4 follow icons)
* [x] Custom Crosshair
* [x] GameMode : TeamDeathmatch (TDM)
* [x] GameMode : Deathmatch (DM) with BOT support
* [ ] HUD : Respawn Bar
* [x] Player spawn protection
* [x] Zombie knife, sound, models, effects, properties
* [ ] Zombie knockback
* [x] GameMode : Zombie Mod 1
* [ ] Zombie classes, skills
* [ ] SupplyBox
* [ ] Zombie evolution
* [ ] GameMode : Zombie Mod 2
* [ ] GameMode : Zombie United
* [ ] GameMode : Zombie Mod 3 (classic)
* [ ] GameMode : Zombie Scenario (classic)
* [ ] UI : BuyMenu

## Difference from other CSBTE(s) now
* No more custom weapons and player classes
* Simplified gameplay
* No AMXX, all C/C++ native code instead
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
### Linux (inconvenient to test)
### Android - Huawei Ascend P9
* SoC : Kirin 950
* RAM : 4 GB
* Resolution : 1920 x 1080 FHD
* Result : Playable, 40~60fps
### iOS - Apple iPhone 7
* SoC : A10 Fusion
* Resolution : 1334 x 750
* Result : Smooth, 55~60fps
### iOS - iPad 6th gen (2018)
* SoC : A10 Fusion
* Resolution : 2048 x 1536
* Result : Playable, 50~60fps

## Installation
### Windows
1. download ***-win32-i386 and ***-GameDir and extract them together
2. purchase and download Counter-Strike on Steam
3. copy cstrike and valve directory to where the csbtem locates
4. run csbtem_win32.exe
### Linux
1. download ***-linux-i386 and ***-GameDir and extract them together
2. purchase and download Counter-Strike on Steam
3. copy cstrike and valve directory to where the csbtem locates
4. launch terminal, cd to the dir, type 
  sudo chmod +x xash3d
5. type the cmd below to run the game
  bash csbtem_linux.sh
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
5. launch iTunes and select the CSBTE-Mobile APP in file sharing
6. drag csbtem, cstrike, valve into iTunes
7. run the CSBTE-Mobile app on the SpringBoard

## License
This repo is licensed under GPLv3 license, see LICENSE file for details.

## Contributing
* explain what's wrong & what's to be added
* commit a possibe solution

 DO NOT commit suggestions about gameplay such as When will the new annual weapon be updated.\
 For non-developers, turn to https://tieba.baidu.com/f?kw=csoldjb for more information.

# CSBTE-Mobile项目
这里提供CSBTE Mobile的部分源码（用于Windows和Linux）。\
安卓和iOS版本的启动器源码请参考我的其他项目。

## 关于 Counter-Strike:BreakThrough Edition
CSBTE用于模拟CSOL游戏体验的一个CS1.6模组。\
最初用于突破CSO-NST在CSOL单机方面的垄断地位，现已成为国内最完美的CSOL单机版本

## 如何安装？
请参考csoldjb吧内的相关图文教程。

## 本项目的相关协议
CSBTE-Mobile继承依赖模块（xash3d、regamedll、cs16client）采用GPLv3开源协议。这意味着：
* 你拥有在非商业用途范围内免费使用、修改、发布本软件的自由，但是您必须详细注明您修改的部分并提供修改后的源代码。
* 使用本软件的任何部分（包括但不限于抄写代码、绑定链接、逆向工程等手段）都必须在产品中使用GPLv3及其兼容开源协议。
* 您可以转载或分发本软件，但需要注明原作者 New BTE Team/Moemod 及其他贡献者（建议附上此GitHub页面的链接）。 \
(请查看 GPLv3 LICENSE 文件.)

## 基于本项目提出建议
* 提出 issue
* 请详细说明修改/增加了何处，并 commit 一个可行的解决方案

 这里仅接受BUG提交，请不要提交关于游戏内容的建议（例如要求更新AK47-火麒麟）\
 非开发者请访问 https://tieba.baidu.com/f?kw=csoldjb 获取更多信息.
