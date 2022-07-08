#ifndef GAMEUI_INTERFACE_H
#define GAMEUI_INTERFACE_H
#pragma once

#include "GameUI/IGameUI.h"

class CGameUI : public IGameUI
{
private:
	enum
	{
		MAX_NUM_FACTORIES = 5
	};

public:
	CGameUI(void);
	~CGameUI(void);

public:
	virtual void Initialize(CreateInterfaceFn *factories, int count);
	virtual void Start(struct cl_enginefuncs_s *engineFuncs, int interfaceVersion, void *system);
	virtual void Shutdown(void);
	virtual int ActivateGameUI(void);
	virtual int ActivateDemoUI(void);
	virtual int HasExclusiveInput(void);
	virtual void RunFrame(void);
	virtual void ConnectToServer(const char *game, int IP, int port);
	virtual void DisconnectFromServer(void);
	virtual void HideGameUI(void);
	virtual bool IsGameUIActive(void);
	virtual void LoadingStarted(const char *resourceType, const char *resourceName);
	virtual void LoadingFinished(const char *resourceType, const char *resourceName);
	virtual void StartProgressBar(const char *progressType, int progressSteps);
	virtual int ContinueProgressBar(int progressPoint, float progressFraction);
	virtual void StopProgressBar(bool bError, const char *failureReason, const char *extendedReason = NULL);
	virtual int SetProgressBarStatusText(const char *statusText);
	virtual void SetSecondaryProgressBar(float progress);
	virtual void SetSecondaryProgressBarText(const char *statusText);
	virtual void ValidateCDKey(bool force, bool inConnect);

public:
	virtual void PaddingFunction1(int param1, int param2) {}
	virtual void PaddingFunction2(int param1, int param2) {}
	virtual void PaddingFunction3(int param1, int param2) {}
	virtual void PaddingFunction4(int param1, int param2) {}
	virtual void PaddingFunction5(int param1, int param2) {}

public:
	bool IsServerBrowserValid(void);
	void ActivateServerBrowser(void);

public:
	bool IsInLevel(void);
	bool IsInMultiplayer(void);

private:
	bool m_bLoadlingLevel;
	char m_szPreviousStatusText[128];

	int m_iNumFactories;
	CreateInterfaceFn m_FactoryList[MAX_NUM_FACTORIES];
};

extern CGameUI &GameUI(void);
extern vgui2::Panel *StaticPanel(void);

#endif