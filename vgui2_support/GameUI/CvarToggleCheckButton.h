#ifndef CVARTOGGLECHECKBUTTON_H
#define CVARTOGGLECHECKBUTTON_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/CheckButton.h>

class CCvarToggleCheckButton : public vgui2::CheckButton
{
	DECLARE_CLASS_SIMPLE(CCvarToggleCheckButton, vgui2::CheckButton);

public:
	CCvarToggleCheckButton(vgui2::Panel *parent, const char *panelName, const char *text, char const *cvarname);
	~CCvarToggleCheckButton(void);

public:
	virtual void SetSelected(bool state);
	virtual void Paint(void);

public:
	void Reset(void);
	void ApplyChanges(void);
	bool HasBeenModified(void);
	void ApplySettings(KeyValues *inResourceData);

private:
	MESSAGE_FUNC(OnButtonChecked, "CheckButtonChecked");

private:
	char *m_pszCvarName;
	bool m_bStartValue;
};

#endif