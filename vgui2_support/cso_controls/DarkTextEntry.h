
#ifndef DARKTEXTENTRY_H
#define DARKTEXTENTRY_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/IBorder.h>
#include <vgui/IScheme.h>
#include <KeyValues.h>

#include <vgui_controls/TextEntry.h>
#include <vgui_controls/ImagePanel.h>

class DarkTextEntry : public vgui2::TextEntry
{
	DECLARE_CLASS_SIMPLE(DarkTextEntry, TextEntry);

public:
	DarkTextEntry(vgui2::Panel *parent, const char *panelName) :
		TextEntry(parent, panelName) {}

	virtual void ApplySchemeSettings(vgui2::IScheme *pScheme) override
	{
		BaseClass::ApplySchemeSettings(pScheme);
		m_bImageBackground = true;
		m_pTopBackground[0] = vgui2::scheme()->GetImage("resource/control/textentry/Output_default_top_left@n", true);
		m_pTopBackground[1] = vgui2::scheme()->GetImage("resource/control/textentry/Output_default_top_center@n", true);
		m_pTopBackground[2] = vgui2::scheme()->GetImage("resource/control/textentry/Output_default_top_right@n", true);
		m_pCenterBackground[0] = vgui2::scheme()->GetImage("resource/control/textentry/Output_default_center_left@n", true);
		m_pCenterBackground[1] = vgui2::scheme()->GetImage("resource/control/textentry/Output_default_center_center@n", true);
		m_pCenterBackground[2] = vgui2::scheme()->GetImage("resource/control/textentry/Output_default_center_right@n", true);
		m_pBottomBackground[0] = vgui2::scheme()->GetImage("resource/control/textentry/Output_default_bottom_left@n", true);
		m_pBottomBackground[1] = vgui2::scheme()->GetImage("resource/control/textentry/Output_default_bottom_center@n", true);
		m_pBottomBackground[2] = vgui2::scheme()->GetImage("resource/control/textentry/Output_default_bottom_Right@n", true);
	}
	virtual Color GetSchemeColor(const char *keyName, vgui2::IScheme *pScheme) override
	{
		if (!stricmp(keyName, "TextEntryFgColor"))
			return BaseClass::GetSchemeColor("LabelDimText", pScheme);
		return BaseClass::GetSchemeColor(keyName, pScheme);
	}
	virtual Color GetSchemeColor(const char *keyName, Color defaultColor, vgui2::IScheme *pScheme) override
	{
		if (!stricmp(keyName, "TextEntryFgColor"))
			return BaseClass::GetSchemeColor("LabelDimText", defaultColor, pScheme);
		return BaseClass::GetSchemeColor(keyName, defaultColor, pScheme);
	}
};

#endif