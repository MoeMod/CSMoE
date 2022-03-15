#ifndef BACKGROUNDMENUBUTTON_H
#define BACKGROUNDMENUBUTTON_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Button.h>

class CBackgroundMenuButton : public vgui2::Button
{
public:
	CBackgroundMenuButton(vgui2::Panel *parent, const char *name);
	~CBackgroundMenuButton(void);

public:
	virtual void SetVisible(bool state);
	virtual void ApplySchemeSettings(vgui2::IScheme *pScheme);

public:
	virtual void OnKillFocus(void);
	virtual void OnCommand(const char *command);

protected:
	vgui2::Menu *RecursiveLoadGameMenu(KeyValues *datafile);
	vgui2::Menu *m_pMenu;

private:
	vgui2::IImage *m_pImage, *m_pMouseOverImage;
	typedef vgui2::Button BaseClass;
};

#endif