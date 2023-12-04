#pragma once

#include <VGUI/IScheme.h>
#include <tier1/KeyValues.h>

#include <vgui_controls/Button.h>
#include <vgui_controls/ImagePanel.h>


class CTexturedButton : public vgui2::Button
{
	DECLARE_CLASS_SIMPLE(CTexturedButton, Button);

public:
	CTexturedButton(Panel* parent, const char* panelName, const char* text = "", Panel* pActionSignalTarget = NULL, const char* pCmd = NULL) : BaseClass(parent, panelName, text, pActionSignalTarget, pCmd)
	{
		for (int i = 0; i < 3; i++)
		{
			m_pImage[i] = NULL;
		}
	}
	CTexturedButton(Panel* parent, const char* panelName, const wchar_t* text, Panel* pActionSignalTarget = NULL, const char* pCmd = NULL) : BaseClass(parent, panelName, text, pActionSignalTarget, pCmd)
	{
		for (int i = 0; i < 3; i++)
		{
			m_pImage[i] = NULL;
		}
	}

	void SetImage(char* c, char* n, char* o);
	void SetDisabledImage(char* d);

protected:
	// Paint button on screen
	virtual void Paint(void);
	virtual void PaintBorder(void) { return; }
	virtual void PaintBackground(void);
	virtual void ApplySettings(KeyValues* resourceData);

private:
	vgui2::IImage* m_pImage[4];
};