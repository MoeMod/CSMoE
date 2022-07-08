/*
player_msg.h - CSMoE Server : CBasePlayer messages
Copyright (C) 2019 Moemod Hyakuya

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef PLAYER_MSG_H
#define PLAYER_MSG_H
#ifdef _WIN32
#pragma once
#endif

struct ItemInfo;

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

extern int giPrecacheGrunt;
extern int gmsgWeapPickup;
extern int gmsgHudText;
extern int gmsgHudTextArgs;
extern int gmsgShake;
extern int gmsgFade;
extern int gmsgFlashlight;
extern int gmsgFlashBattery;
extern int gmsgResetHUD;
extern int gmsgInitHUD;
extern int gmsgViewMode;
extern int gmsgShowGameTitle;
extern int gmsgCurWeapon;
extern int gmsgHealth;
extern int gmsgDamage;
extern int gmsgBattery;
extern int gmsgTrain;
extern int gmsgLogo;
extern int gmsgWeaponList;
extern int gmsgAmmoX;
extern int gmsgDeathMsg;
extern int gmsgScoreAttrib;
extern int gmsgScoreInfo;
extern int gmsgTeamInfo;
extern int gmsgTeamScore;
extern int gmsgGameMode;
extern int gmsgMOTD;
extern int gmsgServerName;
extern int gmsgAmmoPickup;
extern int gmsgItemPickup;
extern int gmsgHideWeapon;
extern int gmsgSayText;
extern int gmsgTextMsg;
extern int gmsgSetFOV;
extern int gmsgShowMenu;
extern int gmsgSendAudio;
extern int gmsgRoundTime;
extern int gmsgMoney;
extern int gmsgBlinkAcct;
extern int gmsgArmorType;
extern int gmsgStatusValue;
extern int gmsgStatusText;
extern int gmsgStatusIcon;
extern int gmsgBarTime;
extern int gmsgReloadSound;
extern int gmsgCrosshair;
extern int gmsgNVGToggle;
extern int gmsgRadar;
extern int gmsgSpectator;
extern int gmsgVGUIMenu;
extern int gmsgCZCareer;
extern int gmsgCZCareerHUD;
extern int gmsgTaskTime;
extern int gmsgTutorText;
extern int gmsgTutorLine;
extern int gmsgShadowIdx;
extern int gmsgTutorState;
extern int gmsgTutorClose;
extern int gmsgAllowSpec;
extern int gmsgBombDrop;
extern int gmsgBombPickup;
extern int gmsgHostagePos;
extern int gmsgHostageK;
extern int gmsgGeigerRange;
extern int gmsgSendCorpse;
extern int gmsgHLTV;
extern int gmsgSpecHealth;
extern int gmsgForceCam;
extern int gmsgADStop;
extern int gmsgReceiveW;
extern int gmsgScenarioIcon;
extern int gmsgBotVoice;
extern int gmsgBuyClose;
extern int gmsgItemStatus;
extern int gmsgLocation;
extern int gmsgSpecHealth2;
extern int gmsgBarTime2;
extern int gmsgBotProgress;
extern int gmsgBrass;
extern int gmsgFog;
extern int gmsgShowTimer;
extern int gmsgShowWin;
extern int gmsgSpecialCrossHair;
extern int gmsgHitMsg;

extern int gmsgHeadIcon;
extern int gmsgZBSTip;
extern int gmsgZBSLevel;
extern int gmsgBTEWeapon;
extern int gmsgZB2Msg;
extern int gmsgZB3Msg;
extern int gmsgZBZMsg;
extern int gmsgGunDeath;
extern int gmsgWeapDrop;
extern int gmsgOperationSystem;
extern int gmsgZ4Msg;
extern int gmsgMVPInfo;
extern int gmsgMPToCL;
extern int gmsgNewHudMsg;

void LinkUserMessages();
void WriteSigonMessages();
void SendItemStatus(CBasePlayer *pPlayer);

void SetScoreAttrib(CBasePlayer *dest, CBasePlayer *src);

void WriteWeaponInfo(CBasePlayer *pPlayer, const ItemInfo &II);

}

#endif