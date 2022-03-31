#include "BackGroundPanel.h"

#include <vgui_controls/Panel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/BitmapImagePanel.h>

class CCSBackGroundPanel : public CBackGroundPanel
{
	typedef CBackGroundPanel BaseClass;

public:
	CCSBackGroundPanel(vgui2::Panel *parent);

public:
	void SetTitleText(const wchar_t *text);
	void SetTitleText(const char *text);

public:
	void PaintBackground(void);
	void Activate(void);
	void ApplySchemeSettings(vgui2::IScheme *pScheme);
	void PerformLayout(void);

private:
	vgui2::CBitmapImagePanel *m_pTopLeftPanel;
	vgui2::CBitmapImagePanel *m_pTopRightPanel;
	vgui2::CBitmapImagePanel *m_pBottomLeftPanel;
	vgui2::CBitmapImagePanel *m_pBottomRightPanel;
	vgui2::Label *m_pTitleLabel;
	vgui2::Panel *m_pGapPanel;
	vgui2::CBitmapImagePanel *m_pExclamationPanel;
	Color m_bgColor;
	Color m_titleColor;
	Color m_borderColor;
	int m_offsetX;
	int m_offsetY;
	bool m_enabled;
};