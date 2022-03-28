//========= Copyright ?1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
//=============================================================================
#include "EngineInterface.h"
#include "OptionsSubAudio.h"

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

	m_pEnableButton = new  CCvarToggleCheckButton(
		this,
		"EnableButton",
		"#GameUI_EnableButton",
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

	m_pProfileName = new CCvarTextEntry(this, "NameEntry", "name");



	m_pEAXCheckButton = new CCvarToggleCheckButton( 
		this, 
		"EAX",
		"#GameUI_EnableEAX",
		"s_eax" );

	m_pA3DCheckButton = new CCvarToggleCheckButton( 
		this, 
		"A3D",
		"#GameUI_EnableA3D",
		"s_a3d" );

	m_pSFXSlider = new CCvarSlider( this, "SFX Slider", "#GameUI_SoundEffectVolume",
		0.0f, 2.0f, "volume");

	m_pHEVSlider = new CCvarSlider( this, "Suit Slider", "#GameUI_HEVSuitVolume",
		0.0f, 2.0f, "suitvolume");

	m_pMP3Slider = new CCvarSlider( this, "MP3 Volume", "#GameUI_MP3Volume",
		0.0f, 1.0f, "mp3volume" );

	m_pSoundQualityCombo = new CLabeledCommandComboBox( this, "Sound Quality" );

	//!! bug no command given
	m_pSoundQualityCombo->AddItem( "#GameUI_High", "hisound 1" );
	m_pSoundQualityCombo->AddItem( "#GameUI_Low", "hisound 0" );
	m_pSoundQualityCombo->SetInitialItem(engine->pfnGetCvarFloat("hisound") != 0.0f ? 0 : 1);
		


	LoadControlSettings("Resource/OptionsSubAudio.res");

	// override, hide the HEV suit volume when not in half-life
	if (ModInfo().IsMultiplayerOnly())
	{
		Panel *child = FindChildByName("suit label");
		if (child)
		{
			child->SetVisible(false);
		}
		child = FindChildByName("Suit Slider");
		if (child)
		{
			child->SetVisible(false);
		}
	}

	// hide expired cvar
	if (!engine->pfnGetCvarPointer("s_eax"))
	{
		m_pEAXCheckButton->SetVisible( false );
	}
	if (!engine->pfnGetCvarPointer("s_a3d"))
	{
		m_pA3DCheckButton->SetVisible( false );
	}
}

COptionsSubTouch::~COptionsSubTouch()
{
}

void COptionsSubTouch::OnResetData()
{
	m_pEAXCheckButton->Reset();
	m_pA3DCheckButton->Reset();
	m_pSFXSlider->Reset();
	m_pHEVSlider->Reset();
	m_pMP3Slider->Reset();
	m_pSoundQualityCombo->Reset();
}

void COptionsSubTouch::OnApplyChanges()
{
	if (engine->pfnGetCvarPointer("s_eax"))
		m_pEAXCheckButton->ApplyChanges();
	if ( engine->pfnGetCvarPointer( "s_a3d" ) )
		m_pA3DCheckButton->ApplyChanges();

	m_pSFXSlider->ApplyChanges();
	m_pHEVSlider->ApplyChanges();
	m_pMP3Slider->ApplyChanges();
	m_pSoundQualityCombo->ApplyChanges();
}

void COptionsSubTouch::OnControlModified()
{
	PostActionSignal(new KeyValues("ApplyButtonEnable"));
}