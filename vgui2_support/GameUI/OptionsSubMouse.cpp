#include "EngineInterface.h"
#include "OptionsSubMouse.h"
#include "KeyToggleCheckButton.h"
#include "CvarNegateCheckButton.h"
#include "CvarToggleCheckButton.h"
#include "CvarSlider.h"

#include <KeyValues.h>
#include <vgui/IScheme.h>
#include <stdio.h>
#include <vgui_controls/TextEntry.h>

COptionsSubMouse::COptionsSubMouse(vgui2::Panel *parent) : PropertyPage(parent, NULL)
{
	m_pReverseMouseCheckBox = new CCvarNegateCheckButton(this, "ReverseMouse", "#GameUI_ReverseMouse", "m_pitch");
	m_pMouseLookCheckBox = new CKeyToggleCheckButton(this, "MouseLook", "#GameUI_MouseLook", "in_mlook", "mlook");
	m_pMouseFilterCheckBox = new CCvarToggleCheckButton(this, "MouseFilter", "#GameUI_MouseFilter", "m_filter");

	m_pJoystickCheckBox = new CCvarToggleCheckButton(this, "Joystick", "#GameUI_Joystick", "joystick");
	m_pJoystickLookCheckBox = new CKeyToggleCheckButton(this, "JoystickLook", "#GameUI_JoystickLook", "in_jlook", "jlook");
	m_pMouseSensitivitySlider = new CCvarSlider(this, "Slider", "#GameUI_MouseSensitivity", 1.0f, 20.0f, "sensitivity", true);

	m_pMouseSensitivityLabel = new vgui2::TextEntry(this, "SensitivityLabel");
	m_pMouseSensitivityLabel->AddActionSignalTarget(this);

	m_pAutoAimCheckBox = new CCvarToggleCheckButton(this, "Auto-Aim", "#GameUI_AutoAim", "sv_aim");

	LoadControlSettings("Resource/OptionsSubMouse.res");

	float sensitivity = engine->pfnGetCvarFloat("sensitivity");
	char buf[64];
	_snprintf(buf, sizeof(buf) - 1, " %.1f", sensitivity);
	buf[sizeof(buf) - 1] = 0;
	m_pMouseSensitivityLabel->SetText(buf);
}

COptionsSubMouse::~COptionsSubMouse(void)
{
}

void COptionsSubMouse::OnPageShow(void)
{
}

void COptionsSubMouse::OnResetData(void)
{
	m_pReverseMouseCheckBox->Reset();
	m_pMouseLookCheckBox->Reset();
	m_pMouseFilterCheckBox->Reset();
	m_pJoystickCheckBox->Reset();
	m_pJoystickLookCheckBox->Reset();
	m_pMouseSensitivitySlider->Reset();
	m_pAutoAimCheckBox->Reset();
}

void COptionsSubMouse::OnApplyChanges(void)
{
	m_pReverseMouseCheckBox->ApplyChanges();
	m_pMouseLookCheckBox->ApplyChanges();
	m_pMouseFilterCheckBox->ApplyChanges();
	m_pJoystickCheckBox->ApplyChanges();
	m_pJoystickLookCheckBox->ApplyChanges();
	m_pMouseSensitivitySlider->ApplyChanges();
	m_pAutoAimCheckBox->ApplyChanges();
}

void COptionsSubMouse::ApplySchemeSettings(vgui2::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_pMouseSensitivityLabel->SetBorder(pScheme->GetBorder("ButtonDepressedBorder"));
}

void COptionsSubMouse::OnControlModified(Panel *panel)
{
	PostActionSignal(new KeyValues("ApplyButtonEnable"));

	if (panel == m_pMouseSensitivitySlider && m_pMouseSensitivitySlider->HasBeenModified())
		UpdateSensitivityLabel();
}

void COptionsSubMouse::OnTextChanged(Panel *panel)
{
	if (panel == m_pMouseSensitivityLabel)
	{
		char buf[64];
		m_pMouseSensitivityLabel->GetText(buf, 64);

		float fValue = (float)atof(buf);

		if (fValue >= 1.0)
		{
			m_pMouseSensitivitySlider->SetSliderValue(fValue);
			PostActionSignal(new KeyValues("ApplyButtonEnable"));
		}
	}
}

void COptionsSubMouse::UpdateSensitivityLabel(void)
{
	char buf[64];
	Q_snprintf(buf, sizeof(buf), " %.1f", m_pMouseSensitivitySlider->GetSliderValue());
	m_pMouseSensitivityLabel->SetText(buf);
}