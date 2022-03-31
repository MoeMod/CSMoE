
#ifndef DARKALPHATEXTENTRY_H
#define DARKALPHATEXTENTRY_H

#ifdef _WIN32
#pragma once
#endif

#include <VGUI/IBorder.h>
#include <VGUI/IScheme.h>
#include <KeyValues.h>

#include <vgui_controls/TextEntry.h>
#include <vgui_controls/ImagePanel.h>

class DarkAlphaTextEntry : public vgui2::TextEntry
{
	DECLARE_CLASS_SIMPLE(DarkAlphaTextEntry, TextEntry);

public:
	DarkAlphaTextEntry(vgui2::Panel *parent, const char *panelName) :
		TextEntry(parent, panelName) {}

	virtual void ApplySchemeSettings(vgui2::IScheme *pScheme) override
	{
		BaseClass::ApplySchemeSettings(pScheme);
		m_bImageBackground = true;
		m_pTopBackground[0] = vgui2::scheme()->GetImage("resource/control/textentry/Tooltip_default_top_left@n", true);
		m_pTopBackground[1] = vgui2::scheme()->GetImage("resource/control/textentry/Tooltip_default_top_center@n", true);
		m_pTopBackground[2] = vgui2::scheme()->GetImage("resource/control/textentry/Tooltip_default_top_right@n", true);
		m_pCenterBackground[0] = vgui2::scheme()->GetImage("resource/control/textentry/Tooltip_default_center_left@n", true);
		m_pCenterBackground[1] = vgui2::scheme()->GetImage("resource/control/textentry/Tooltip_default_center_center@n", true);
		m_pCenterBackground[2] = vgui2::scheme()->GetImage("resource/control/textentry/Tooltip_default_center_right@n", true);
		m_pBottomBackground[0] = vgui2::scheme()->GetImage("resource/control/textentry/Tooltip_default_bottom_left@n", true);
		m_pBottomBackground[1] = vgui2::scheme()->GetImage("resource/control/textentry/Tooltip_default_bottom_center@n", true);
		m_pBottomBackground[2] = vgui2::scheme()->GetImage("resource/control/textentry/Tooltip_default_bottom_Right@n", true);

		SetFgColor(GetSchemeColor("LabelDimText", pScheme));
	}
};

#endif