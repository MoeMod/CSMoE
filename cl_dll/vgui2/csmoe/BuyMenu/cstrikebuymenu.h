#ifndef CSTRIKEBUYMENU_H
#define CSTRIKEBUYMENU_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/WizardPanel.h>
#include "game_controls/buymenu.h"
#include "IViewPort.h"

namespace vgui
{
	class Panel;
	class Button;
	class Label;
}

class BuyMouseOverPanelButton;

enum
{
	NUM_BUY_PRESET_BUTTONS = 5,
};

class CCSBaseBuyMenu : public CBuyMenu
{
	typedef CBuyMenu BaseClass;

public:
	CCSBaseBuyMenu(IViewport *pViewPort);

protected:
	void SetupControlSettings();
public:
	void SetTeam(int iTeam);
	void UpdateGameMode();

public:
	virtual void Init(void);
	virtual void VidInit(void);
	virtual void ShowPanel(bool bShow);
	virtual void Paint(void);
	virtual void SetVisible(bool state);
	virtual void GotoMenu(int iMenu);
	virtual void ActivateMenu(int iMenu);

public:
	virtual void PaintBackground(void);
	virtual void PerformLayout(void);
};

#endif