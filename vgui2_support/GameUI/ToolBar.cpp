#include "EngineInterface.h"
#include "ToolBar.h"
#include <vgui/ILocalize.h>
#include <vgui/IScheme.h>

CToolBar::CToolBar(vgui2::Panel *parent, const char *panelName) : BaseClass(parent, panelName)
{
#if 0
	SetSizeable(false);
	SetTitleBarVisible(false);
#endif
	SetPaintBackgroundEnabled(true);
}

CToolBar::~CToolBar(void)
{
}

void CToolBar::ApplySchemeSettings(vgui2::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

#ifndef _DEBUG
	SetBgColor(Color(128, 128, 128, 64));
#else
	SetBgColor(Color(0, 0, 0, 0));
#endif
}

void CToolBar::PerformLayout(void)
{
	BaseClass::PerformLayout();
}

void CToolBar::PaintBackground(void)
{
	BaseClass::PaintBackground();
}