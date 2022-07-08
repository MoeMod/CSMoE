#if !defined (VCONTROLSLISTPANEL_H)
#define VCONTROLSLISTPANEL_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/SectionedListPanel.h>

class VControlsListPanel : public vgui2::SectionedListPanel
{
public:
	VControlsListPanel(vgui2::Panel *parent, const char *listName);
	virtual ~VControlsListPanel(void);

public:
	virtual void StartCaptureMode(vgui2::HCursor hCursor = NULL);
	virtual void EndCaptureMode(vgui2::HCursor hCursor = NULL);
	virtual bool IsCapturing(void);
	virtual void SetItemOfInterest(int itemID);
	virtual int GetItemOfInterest(void);
	virtual void OnMousePressed(vgui2::MouseCode code);
	virtual void OnMouseDoublePressed(vgui2::MouseCode code);

private:
	void ApplySchemeSettings(vgui2::IScheme *pScheme);

private:
	class CInlineEditPanel *m_pInlineEditPanel;

private:
	bool m_bCaptureMode;
	int m_nClickRow;
	vgui2::HFont m_hFont;
	int m_iMouseX, m_iMouseY;
	typedef vgui2::SectionedListPanel BaseClass;
};

#endif