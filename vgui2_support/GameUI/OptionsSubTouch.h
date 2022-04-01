//========= Copyright ?1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef OPTIONS_SUB_TOUCH_H
#define OPTIONS_SUB_TOUCH_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/PropertyPage.h>
class CLabeledCommandComboBox;
class CCvarSlider;
class CCvarToggleCheckButton;
//-----------------------------------------------------------------------------
// Purpose: Touch Details, Part of OptionsDialog
//-----------------------------------------------------------------------------
class COptionsSubTouch : public vgui2::PropertyPage
{
	DECLARE_CLASS_SIMPLE(COptionsSubTouch, vgui2::PropertyPage );

public:
	COptionsSubTouch(vgui2::Panel *parent);
	~COptionsSubTouch();

	virtual void OnResetData();
	virtual void OnApplyChanges();

private:
	MESSAGE_FUNC( OnControlModified, "ControlModified" );

	CCvarToggleCheckButton		*m_pEAXCheckButton;
	CCvarToggleCheckButton		*m_pA3DCheckButton;

	CCvarSlider					*m_pSFXSlider;
	CCvarSlider					*m_pHEVSlider;
	CCvarSlider					*m_pMP3Slider;
	CLabeledCommandComboBox		*m_pSoundQualityCombo;


	CCvarSlider* m_pLookXSlider;
	CCvarSlider* m_pLookYSlider;
	CCvarSlider* m_pMoveXSlider;
	CCvarSlider* m_pMoveYSlider;
	CCvarSlider* m_pPowerSlider;
	CCvarSlider* m_pMultiplierSlider;
	CCvarSlider* m_pExponentSlider;

	CCvarToggleCheckButton* m_pGridButton;
	CCvarToggleCheckButton* m_pEnableTouchButton;
	CCvarToggleCheckButton* m_pNomouseButton;
	CCvarToggleCheckButton* m_pAccelerationButton;

	//CCvarTextEntry* m_pProfileName;

	vgui2::ComboBox* m_pProfiles;
	vgui2::Button* reset;
	vgui2::Button* save;
	vgui2::Button* remove;
	vgui2::Button* apply;
	vgui2::Button* done;

	//CMenuSpinControl gridsize;


};



#endif // OPTIONS_SUB_AUDIO_H