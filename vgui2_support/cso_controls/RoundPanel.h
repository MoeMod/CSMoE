//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef ROUND_PANEL_H
#define ROUND_PANEL_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI.h>
#include <vgui_controls/Panel.h>

namespace vgui2
{
class RoundPanel : public Panel
{
	DECLARE_CLASS_SIMPLE(RoundPanel, Panel);

public:
	enum ImageType
	{
		SELECTED = 1,
		WINDOW_DEFAULT,
		WINDOW_NOROUND,
		VERTICALTAB_ON,
		ROOM_SELECTED,
		FLOATINGMENU_PULLDOWN,
		WINDOW_BALLOON,
		WINDOW_BUFF,
		TEXTENTRY_OUPUT,
		SELECTED_OVER,
		SELECTED_DISABLE,
		TEXTENTRY_TOOLTIP,
		ZOMBIEUX_DISABLE,
		ROOM_SELECTED_RED,
		ROOM_SELECTED_GREEN,
		ROOM_SELECTED_BLUE,
		TOOLTIP_LIST,

		NUM_TYPES
	};

public:
	RoundPanel(vgui2::Panel *parent, const char *panelName, ImageType type);
	~RoundPanel();

public:
	void SetImage(ImageType type);

protected:
	void Paint(void);

private:
	IImage *m_pPics[3][3];
};
}
#endif // ROUND_PANEL_H