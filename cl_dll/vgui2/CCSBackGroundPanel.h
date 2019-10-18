#ifndef GAME_CLIENT_UI_VGUI2_CCSBACKGROUNDPANEL_H
#define GAME_CLIENT_UI_VGUI2_CCSBACKGROUNDPANEL_H

#include "CBackGroundPanel.h"

#include <vgui_controls/Panel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/BitmapImagePanel.h>

class CCSBackGroundPanel : public CBackGroundPanel
{
	typedef CBackGroundPanel BaseClass;

public:
	CCSBackGroundPanel(vgui2::Panel* parent);

public:
	void SetTitleText(const wchar_t* text);
	void SetTitleText(const char* text);

public:
	void Activate(void) override;
	void ApplySchemeSettings(vgui2::IScheme* pScheme) override;
	void PerformLayout(void) override;

private:
	vgui2::CBitmapImagePanel* m_pTopLeftPanel;
	vgui2::CBitmapImagePanel* m_pTopRightPanel;
	vgui2::CBitmapImagePanel* m_pBottomLeftPanel;
	vgui2::CBitmapImagePanel* m_pBottomRightPanel;
	vgui2::Label* m_pTitleLabel;
	vgui2::Panel* m_pGapPanel;
	vgui2::CBitmapImagePanel* m_pExclamationPanel;
	Color m_bgColor;
	Color m_titleColor;
	Color m_borderColor;
	int m_offsetX;
	int m_offsetY;
	bool m_enabled;
};

#endif //GAME_CLIENT_UI_VGUI2_CCSBACKGROUNDPANEL_H
