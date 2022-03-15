#ifndef OPTIONS_SUB_MOUSE_H
#define OPTIONS_SUB_MOUSE_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/PropertyPage.h>

class CCvarNegateCheckButton;
class CKeyToggleCheckButton;
class CCvarToggleCheckButton;
class CCvarSlider;

namespace vgui2
{
	class Label;
	class Panel;
}

class COptionsSubMouse : public vgui2::PropertyPage
{
	DECLARE_CLASS_SIMPLE(COptionsSubMouse, vgui2::PropertyPage);

public:
	COptionsSubMouse(vgui2::Panel *parent);
	~COptionsSubMouse(void);

public:
	virtual void OnPageShow(void);
	virtual void OnResetData(void);
	virtual void OnApplyChanges(void);

protected:
	virtual void ApplySchemeSettings(vgui2::IScheme *pScheme);

private:
	MESSAGE_FUNC_PTR(OnControlModified, "ControlModified", panel);
	MESSAGE_FUNC_PTR(OnTextChanged, "TextChanged", panel);

	MESSAGE_FUNC_PTR(OnCheckButtonChecked, "CheckButtonChecked", panel)
	{
		OnControlModified(panel);
	}

private:
	void UpdateSensitivityLabel(void);

private:
	CCvarNegateCheckButton *m_pReverseMouseCheckBox;
	CKeyToggleCheckButton *m_pMouseLookCheckBox;
	CCvarToggleCheckButton *m_pMouseFilterCheckBox;
	CCvarToggleCheckButton *m_pJoystickCheckBox;
	CKeyToggleCheckButton *m_pJoystickLookCheckBox;

	CCvarSlider *m_pMouseSensitivitySlider;
	vgui2::TextEntry *m_pMouseSensitivityLabel;

	CCvarToggleCheckButton *m_pAutoAimCheckBox;
};

#endif