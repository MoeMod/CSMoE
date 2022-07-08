#pragma once

#include "vgui_controls/Label.h"
#include "KeyValues.h"

class ForceColoredLabel : public vgui2::Label
{
	using BaseClass = vgui2::Label;
public:
	ForceColoredLabel(Panel *parent, const char *panelName, const char *text) : BaseClass(parent, panelName, text)
	{
		m_bForceTextColor = false;
	}
	ForceColoredLabel(Panel *parent, const char *panelName, const wchar_t *text) : BaseClass(parent, panelName, text)
	{
		m_bForceTextColor = false;
	}

	virtual void ApplySettings(KeyValues *inResourceData) override
	{
		BaseClass::ApplySettings(inResourceData);

		m_bForceTextColor = inResourceData->GetInt("EnableForceTextColor", 0) > 0;
		if (m_bForceTextColor)
		{
			SetForceTextColor(inResourceData->GetColor("ForceTextColor"));
		}

		SetMouseInputEnabled(inResourceData->GetInt("EnableMouseInput", 0) > 0);
	}
	virtual void Paint() override
	{
		if (m_bForceTextColor)
		{
			SetFgColor(m_forceColor);
			SetDisabledFgColor1(m_forceColor);
		}
		BaseClass::Paint();
	}

	bool m_bForceTextColor;
	Color m_forceColor;
	void SetForceTextColor(Color col)
	{
		m_bForceTextColor = true;
		m_forceColor = col;
	}
};