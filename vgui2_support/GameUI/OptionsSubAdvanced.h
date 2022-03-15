#ifndef OPTIONS_SUB_LANGUAGE_H
#define OPTIONS_SUB_LANGUAGE_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Label.h>
#include <vgui_controls/PropertyPage.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/MessageBox.h>

class CCvarSlider;
class CCvarToggleCheckButton;

class COptionsSubAdvanced : public vgui2::PropertyPage
{
	DECLARE_CLASS_SIMPLE(COptionsSubAdvanced, vgui2::PropertyPage);

public:
	COptionsSubAdvanced(vgui2::Panel *parent);
	~COptionsSubAdvanced(void);

public:
	virtual void OnPageShow(void);
	virtual void OnResetData(void);
	virtual void OnApplyChanges(void);

private:
	virtual void OnThink(void);
	virtual void OnCommand(const char *command);

private:
	MESSAGE_FUNC(OnPageHide, "PageHide");
	MESSAGE_FUNC_INT(OnSliderMoved, "SliderMoved", position);
	MESSAGE_FUNC_INT(OnCheckButtonChecked, "CheckButtonChecked", state);
	MESSAGE_FUNC(OnControlModified, "ControlModified");
	MESSAGE_FUNC_CHARPTR(OnTextChanged, "TextChanged", text);
};

#endif