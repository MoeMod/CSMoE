/*
ICSViewPort.h - dumped interface from client.dll
Copyright (C) 2019 Moemod Yanase

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#pragma once

#include <vgui/VGUI2.h>

typedef struct VGuiLibraryInterface_s VGuiLibraryInterface_t;
typedef struct VGuiLibraryTeamInfo_s VGuiLibraryTeamInfo_t;
namespace vgui2 {
	class Panel;
}
class IMapOverview;
class ISpectatorInterface;

class IViewPort
{
public:
	virtual VGuiLibraryInterface_t *GetClientDllInterface() = 0;
	virtual void SetClientDllInterface(VGuiLibraryInterface_t *clientInterface) = 0;

	//death.cpp
	virtual void UpdateScoreBoard() = 0;
	virtual bool AllowedToPrintText() = 0; // also saytext.cpp and text_message.cpp
	virtual void GetAllPlayersInfo(void) = 0;
	virtual void DeathMsg(int killer, int victim) = 0;

	// hud_redraw.cpp
	virtual void ShowScoreBoard(void) = 0; // also input.cpp
	virtual bool CanShowScoreBoard(void) = 0;
	virtual void HideAllVGUIMenu(void) = 0;

	virtual void UpdateSpectatorPanel(void) = 0; // GoldSrc added

	// input.cpp
	virtual bool IsScoreBoardVisible() = 0; // also menu.cpp
	virtual void HideScoreBoard(void) = 0;
	virtual int	 KeyInput(int down, int keynum, const char *pszCurrentBinding) = 0;

	virtual void ShowVGUIMenu(int Menu) = 0;
	virtual void HideVGUIMenu(int iMenu) = 0; // GoldSrc added

	virtual void ShowTutorTextWindow(const wchar_t *szString, int id, int msgClass, int isSpectator) = 0; // GoldSrc added
	virtual void ShowTutorLine(int entindex, int id) = 0; // GoldSrc added
	virtual void ShowTutorState(const wchar_t *szString) = 0; // GoldSrc added
	virtual void CloseTutorTextWindow(void) = 0; // GoldSrc added
	virtual bool IsTutorTextWindowOpen(void) = 0; // GoldSrc added
	virtual void ShowSpectatorGUI(void) = 0; // GoldSrc added
	virtual void ShowSpectatorGUIBar(void) = 0; // GoldSrc added
	virtual void HideSpectatorGUI(void) = 0; // GoldSrc added
	virtual void DeactivateSpectatorGUI(void) = 0; // GoldSrc added
	virtual bool IsSpectatorGUIVisible(void) = 0; // GoldSrc added
	virtual bool IsSpectatorBarVisible(void) = 0; // GoldSrc added
	virtual int MsgFunc_ResetFade(const char *pszName, int iSize, void *pbuf) = 0; // GoldSrc added
	virtual void SetSpectatorBanner(const char *image) = 0; // GoldSrc added
	virtual void SpectatorGUIEnableInsetView(int value) = 0; // GoldSrc added

	virtual void ShowCommandMenu() = 0;
	virtual void UpdateCommandMenu() = 0;
	virtual void HideCommandMenu() = 0;
	virtual int IsCommandMenuVisible() = 0;

	virtual int GetValidClasses(int iTeam) = 0;
	virtual int GetNumberOfTeams(int iTeam) = 0; // GoldSrc modified from int GetNumberOfTeams();
	virtual bool GetIsFeigning() = 0;
	virtual int GetIsSettingDetpack() = 0;
	virtual int GetBuildState() = 0;
	virtual int IsRandomPC() = 0;
	virtual char *GetTeamName(int iTeam) = 0;
	virtual int	GetCurrentMenuID() = 0;
	virtual const char *GetMapName(void) = 0; // GoldSrc added
	virtual const char *GetServerName() = 0;

	virtual void InputPlayerSpecial() = 0;

	virtual void OnTick() = 0;

	virtual int GetViewPortScheme() = 0;

	virtual vgui2::VPANEL GetViewPortPanel() = 0; // NOT vgui2::Panel *

	//virtual IMapOverview * GetMapOverviewInterface() = 0; // GoldSrc deleted
	//virtual ISpectatorInterface * GetSpectatorInterface() = 0; // GoldSrc deleted

	virtual int GetAllowSpectators(void) = 0;  // GoldSrc added

	virtual void OnLevelChange(const char * mapname) = 0;

	virtual void HideBackGround() = 0;

	virtual void ChatInputPosition(int *x, int *y) = 0;

	virtual int GetSpectatorBottomBarHeight(void) = 0; // GoldSrc added
	virtual int GetSpectatorTopBarHeight(void) = 0; // GoldSrc added

	// Input
	virtual bool SlotInput(int iSlot) = 0;

	// viewport
	virtual VGuiLibraryTeamInfo_t GetPlayerTeamInfo(int playerIndex) = 0;

	virtual void MakeSafeName(const char *oldName, char *newName, int newNameBufSize) = 0; // GoldSrc added
};