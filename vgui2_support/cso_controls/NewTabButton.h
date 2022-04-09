#ifndef NEWTABBUTTON_H
#define NEWTABBUTTON_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/IBorder.h>
#include <vgui/IScheme.h>
#include <KeyValues.h>

#include <vgui_controls/Button.h>
#include <vgui_controls/ImagePanel.h>

class NewTabButton : public vgui2::Button
{
	typedef vgui2::Button BaseClass;

private:
	Color _replaceColor;
	int m_bMaxTabWidth;

public:
	NewTabButton(vgui2::Panel *parent, const char *panelName, const char *text, int maxTabWidth = 64) :
		BaseClass(parent, panelName, text)
	{
		SetCommand(new KeyValues("TabPressed"));
		m_bMaxTabWidth = maxTabWidth;
	}

	virtual void ApplySchemeSettings(vgui2::IScheme *pScheme)
	{
		// set up the scheme settings
		Button::ApplySchemeSettings(pScheme);

		//GetTextImage()->SetDrawWidth(vgui2::scheme()->GetProportionalScaledValue(1));
		SetTextInset(6, 2);

		int wide, tall;
		int contentWide, contentTall;
		GetSize(wide, tall);
		GetContentSize(contentWide, contentTall);

		wide = max(m_bMaxTabWidth, contentWide + 10);  // 10 = 5 pixels margin on each side
		SetSize(wide, tall);

		SetDefaultBorder(pScheme->GetBorder("TabBorder"));
		SetKeyFocusBorder(pScheme->GetBorder("TabBorder"));
		SetDepressedBorder(pScheme->GetBorder("TabActiveBorder"));

		_imageBackground = true;
		_depressedImage[0] = vgui2::scheme()->GetImage("resource/control/tabbutton/tab_small_abled_left_c", true);
		_depressedImage[1] = vgui2::scheme()->GetImage("resource/control/tabbutton/tab_small_abled_center_c", true);
		_depressedImage[2] = vgui2::scheme()->GetImage("resource/control/tabbutton/tab_small_abled_right_c", true);
		_defaultImage[0] = vgui2::scheme()->GetImage("resource/control/tabbutton/tab_small_disabled_left_n", true);
		_defaultImage[1] = vgui2::scheme()->GetImage("resource/control/tabbutton/tab_small_disabled_center_n", true);
		_defaultImage[2] = vgui2::scheme()->GetImage("resource/control/tabbutton/tab_small_disabled_right_n", true);
		_armedImage[0] = vgui2::scheme()->GetImage("resource/control/tabbutton/tab_small_abled_left_o", true);
		_armedImage[1] = vgui2::scheme()->GetImage("resource/control/tabbutton/tab_small_abled_center_o", true);
		_armedImage[2] = vgui2::scheme()->GetImage("resource/control/tabbutton/tab_small_abled_right_o", true);
	}

	virtual void SetActive(bool state)
	{
		ForceDepressed(state);

		InvalidateLayout();
		Repaint();
	}

	virtual bool CanBeDefaultButton(void)
	{
		return false;
	}

	virtual void PaintBackground() override
	{
		if (_imageBackground)
		{
			int wide, tall;
			GetSize(wide, tall);

			const int iOffset = 0;
			vgui2::IImage **ppimage;
			
			int y = vgui2::scheme()->GetProportionalScaledValue(2);

			if (IsDepressed())
			{
				ppimage = _depressedImage;
				y = 0;
			}
			else if (IsArmed())
				ppimage = _armedImage;
			else
				ppimage = _defaultImage;

			ppimage[0]->SetPos(0, y);
			ppimage[0]->SetSize(10, tall);
			ppimage[0]->Paint();
			ppimage[1]->SetPos(10, y);
			ppimage[1]->SetSize(wide - 20 - iOffset, tall);
			ppimage[1]->Paint();
			ppimage[2]->SetPos(wide - 10 - iOffset, y);
			ppimage[2]->SetSize(10 - iOffset, tall);
			ppimage[2]->Paint();
			return;
		}
		BaseClass::PaintBackground();
	}

	virtual Color GetButtonFgColor()
	{
		if (IsDepressed())
			return _replaceColor;
		else if (IsArmed())
			GetFgColor();
		return GetDisabledFgColor1();
	}

	void SetTextColor(Color col)
	{
		_replaceColor = col;
	}
};

#endif