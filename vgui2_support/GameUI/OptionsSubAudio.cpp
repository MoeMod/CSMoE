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

COptionsSubAudio::COptionsSubAudio(vgui2::Panel *parent) : PropertyPage(parent, NULL)
{
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

COptionsSubAudio::~COptionsSubAudio()
{
}

void COptionsSubAudio::OnResetData()
{
	m_pEAXCheckButton->Reset();
	m_pA3DCheckButton->Reset();
	m_pSFXSlider->Reset();
	m_pHEVSlider->Reset();
	m_pMP3Slider->Reset();
	m_pSoundQualityCombo->Reset();
}

void COptionsSubAudio::OnApplyChanges()
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

void COptionsSubAudio::OnControlModified()
{
	PostActionSignal(new KeyValues("ApplyButtonEnable"));
}