//========= Copyright ?1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef OPTIONS_SUB_MOESETTINGS_H
#define OPTIONS_SUB_MOESETTINGS_H
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
class COptionsSubMoeSettings : public vgui2::PropertyPage
{
	DECLARE_CLASS_SIMPLE(COptionsSubMoeSettings, vgui2::PropertyPage );

public:
	COptionsSubMoeSettings(vgui2::Panel *parent);
	~COptionsSubMoeSettings();
	void InitFloatingDamageTypeList(CLabeledCommandComboBox* cb, bool read);
	void RemapDamage(void);
	void InitSiFiAmmoStyleList(CLabeledCommandComboBox* cb);
	void InitHudStyleList(CLabeledCommandComboBox* cb);
	void InitTexlruList(CLabeledCommandComboBox* cb);
	void InitHudScaleList(CLabeledCommandComboBox* cb);

	virtual void OnResetData();
	virtual void OnApplyChanges();
	virtual void OnPageShow(void);
private:
	MESSAGE_FUNC_PTR(OnTextChanged, "TextChanged", panel);
	MESSAGE_FUNC( OnControlModified, "ControlModified" );

	vgui2::ImagePanel* m_pDamageImage;
	CLabeledCommandComboBox* m_pFloatingDamageType;
	CLabeledCommandComboBox* m_pSiFiAmmoType;
	CLabeledCommandComboBox* m_pHudStyle;
	CLabeledCommandComboBox* m_pTexlru;
	CLabeledCommandComboBox* m_pHudScale;
	CCvarSlider* m_pGammaSlider;


	//CMenuSpinControl gridsize;


};



#endif // OPTIONS_SUB_AUDIO_H