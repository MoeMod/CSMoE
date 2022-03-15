#ifndef KEYTOGGLECHECKBUTTON_H
#define KEYTOGGLECHECKBUTTON_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/CheckButton.h>

class CKeyToggleCheckButton : public vgui2::CheckButton
{
public:
	CKeyToggleCheckButton(vgui2::Panel *parent, const char *panelName, const char *text, char const *keyname, char const *cmdname);
	~CKeyToggleCheckButton(void);

public:
	void Paint(void);
	void Reset(void);
	void ApplyChanges(void);
	bool HasBeenModified(void);

private:
	typedef vgui2::CheckButton BaseClass;

private:
	char *m_pszKeyName;
	char *m_pszCmdName;
	bool m_bStartValue;
};

#endif
