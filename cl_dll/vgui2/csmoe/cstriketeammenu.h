#ifndef CSTEAMMENU_H
#define CSTEAMMENU_H

#ifdef _WIN32
#pragma once
#endif

#include "game_controls\teammenu.h"

class CCSTeamMenu : public CTeamMenu
{
private:
	DECLARE_CLASS_SIMPLE(CCSTeamMenu, CTeamMenu);

public:
	CCSTeamMenu(void);
	~CCSTeamMenu(void);

public:
	void Update(void);
	bool NeedsUpdate(void) { return true; }
	void ShowPanel(bool bShow);
	void SetVisible(bool state);

private:
	enum { NUM_TEAMS = 3 };

private:
	void OnCommand(const char *command);
	void SetVisibleButton(const char *textEntryName, bool state);

private:
	bool m_bVIPMap;

public:
	virtual void PaintBackground(void);
	virtual void PerformLayout(void);
	virtual void ApplySchemeSettings(vgui2::IScheme *pScheme);

protected:
	virtual MouseOverPanelButton *CreateNewMouseOverPanelButton(vgui2::EditablePanel *panel);
};

#endif