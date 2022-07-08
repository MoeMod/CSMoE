//========= Copyright ?1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
//=============================================================================
#include "EngineInterface.h"
#include "OptionsSubTouch.h"

#include "CvarToggleCheckButton.h"
#include "CvarSlider.h"
#include "LabeledCommandComboBox.h"
#include "ModInfo.h"

#include <KeyValues.h>
// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

COptionsSubTouch::COptionsSubTouch(vgui2::Panel *parent) : PropertyPage(parent, NULL)
{
	m_pLookXSlider = new CCvarSlider(this, "LookX Slider", "#GameUI_LookX",
		50.0f, 500.0f, "touch_yaw");

	m_pLookYSlider = new CCvarSlider(this, "LookY Slider", "#GameUI_LookY",
		50.0f, 500.0f, "touch_pitch");

	m_pMoveXSlider = new CCvarSlider(this, "MoveX Slider", "#GameUI_MoveX",
		0.02f, 1.0f, "touch_sidezone");

	m_pMoveYSlider = new CCvarSlider(this, "MoveY Slider", "#GameUI_MoveY",
		0.02f, 1.0f, "touch_forwardzone");

	m_pPowerSlider = new CCvarSlider(this, "Power Slider", "#GameUI_Power",
		1.0f, 1.7f, "touch_pow_factor");

	m_pMultiplierSlider = new CCvarSlider(this, "Multiplier Slider", "#GameUI_Multiplier",
		100.0f, 1000.0f, "touch_pow_mult");

	m_pExponentSlider = new CCvarSlider(this, "Exponent Slider", "#GameUI_Exponent",
		0.0f, 100.0f, "touch_exp_mult");

	m_pGridButton = new  CCvarToggleCheckButton(
		this,
		"EnableGrid",
		"#GameUI_EnableGrid",
		"touch_grid_enable");

	m_pEnableTouchButton = new  CCvarToggleCheckButton(
		this,
		"EnableTouchButton",
		"#GameUI_EnableTouchButton",
		"touch_enable");

	m_pNomouseButton = new  CCvarToggleCheckButton(
		this,
		"IgnoreMouse",
		"#GameUI_IgnoreMouse",
		"m_ignore");

	m_pAccelerationButton = new  CCvarToggleCheckButton(
		this,
		"EnableAcceleration",
		"#GameUI_EnableAcceleration",
		"touch_nonlinear_look");

	//m_pProfileName = new CCvarTextEntry(this, "NameEntry", "name");

	LoadControlSettings("Resource/optionssubtouch.res");
}

COptionsSubTouch::~COptionsSubTouch()
{
}

void COptionsSubTouch::OnResetData()
{
	m_pLookXSlider->Reset();
	m_pLookYSlider->Reset();
	m_pMoveXSlider->Reset();
	m_pMoveYSlider->Reset();
	m_pPowerSlider->Reset();
	m_pMultiplierSlider->Reset();
	m_pExponentSlider->Reset();
	m_pGridButton->Reset();
	m_pEnableTouchButton->Reset();
	m_pNomouseButton->Reset();
	m_pAccelerationButton->Reset();
}

void COptionsSubTouch::OnApplyChanges()
{
	m_pLookXSlider->ApplyChanges();
	m_pLookYSlider->ApplyChanges();
	m_pMoveXSlider->ApplyChanges();
	m_pMoveYSlider->ApplyChanges();
	m_pPowerSlider->ApplyChanges();
	m_pMultiplierSlider->ApplyChanges();
	m_pExponentSlider->ApplyChanges();
	m_pGridButton->ApplyChanges();
	m_pEnableTouchButton->ApplyChanges();
	m_pNomouseButton->ApplyChanges();
	m_pAccelerationButton->ApplyChanges();
}

void COptionsSubTouch::OnControlModified()
{
	PostActionSignal(new KeyValues("ApplyButtonEnable"));
}