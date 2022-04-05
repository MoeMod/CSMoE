#ifndef BUYMENU_H
#define BUYMENU_H

#ifdef _WIN32
#pragma once
#endif

#define PANEL_BUY "BuyMenu"

#include "vgui_controls/WizardPanel.h"
#include "IViewportPanel.h"
#include "IViewPort.h"

class CBuySubMenu;

namespace vgui2
{
	class Panel;
}

class CBuyMenu : public vgui2::WizardPanel, public IViewportPanel
{
private:
	DECLARE_CLASS_SIMPLE(CBuyMenu, vgui2::WizardPanel);

public:
	CBuyMenu(IViewport *pViewPort);
	~CBuyMenu(void);

public:
	virtual void Init(void);
	virtual void VidInit(void);
	virtual const char *GetName(void) { return PANEL_BUY; }
	virtual void SetData(KeyValues *data) {}
	virtual void Reset(void) {}
	virtual void Update(void);
	virtual bool NeedsUpdate(void) { return false; }
	virtual bool HasInputElements(void) { return true; }
	virtual void ShowPanel(bool bShow);

public:
	DECLARE_VIEWPORT_PANEL_SIMPLE();

public:
	virtual void OnClose(void);

protected:
	CBuySubMenu *m_pMainMenu;
	IViewport	*m_pViewPort;

	int m_iTeam;
	int m_iClass;
};

#endif