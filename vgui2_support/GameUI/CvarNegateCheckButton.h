#ifndef CVARNEGATECHECKBUTTON_H
#define CVARNEGATECHECKBUTTON_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/CheckButton.h>

class CCvarNegateCheckButton : public vgui2::CheckButton
{
	DECLARE_CLASS_SIMPLE(CCvarNegateCheckButton, vgui2::CheckButton);

public:
	CCvarNegateCheckButton(vgui2::Panel *parent, const char *panelName, const char *text, char const *cvarname);
	~CCvarNegateCheckButton(void);

public:
	virtual void SetSelected(bool state);
	virtual void Paint(void);

public:
	void Reset(void);
	void ApplyChanges(void);
	bool HasBeenModified(void);

private:
	MESSAGE_FUNC(OnButtonChecked, "CheckButtonChecked");

private:
	char *m_pszCvarName;
	bool m_bStartState;
};

#endif