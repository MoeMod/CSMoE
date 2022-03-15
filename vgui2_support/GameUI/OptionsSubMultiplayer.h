#ifndef OPTIONSSUBMULTIPLAYER_H
#define OPTIONSSUBMULTIPLAYER_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/PropertyPage.h>

class CLabeledCommandComboBox;
class CBitmapImagePanel;

class CCvarToggleCheckButton;
class CCvarTextEntry;
class CCvarSlider;

class CrosshairImagePanel;
class CMultiplayerAdvancedDialog;

class COptionsSubMultiplayer : public vgui2::PropertyPage
{
	DECLARE_CLASS_SIMPLE(COptionsSubMultiplayer, vgui2::PropertyPage);

public:
	COptionsSubMultiplayer(vgui2::Panel *parent);
	~COptionsSubMultiplayer(void);

public:
	virtual vgui2::Panel *CreateControlByName(const char *controlName);

protected:
	virtual void OnPageShow(void);
	virtual void OnResetData(void);
	virtual void OnApplyChanges(void);
	virtual void OnCommand(const char *command);

private:
	void InitModelList(CLabeledCommandComboBox *cb);
	void InitLogoList(CLabeledCommandComboBox *cb);
	void InitLogoColorEntries(void);
	void RemapModel(void);
	void RemapLogo(void);

private:
	MESSAGE_FUNC_PTR(OnTextChanged, "TextChanged", panel);
	MESSAGE_FUNC_PARAMS(OnSliderMoved, "SliderMoved", data);
	MESSAGE_FUNC(OnApplyButtonEnable, "ControlModified");

private:
	void InitCrosshairSizeList(CLabeledCommandComboBox *cb);
	void InitCrosshairTypeList(CLabeledCommandComboBox *cb);
	void InitCrosshairColorEntries(void);
	void ApplyCrosshairColorChanges(void);
	void RedrawCrosshairImage(void);
	void ColorForName(char const *pszColorName, int &r, int &g, int &b);
	void RemapLogoPalette(char *filename, int r, int g, int b);

private:
	CCvarTextEntry *m_pNameTextEntry;

	CBitmapImagePanel *m_pLogoImage;
	CLabeledCommandComboBox *m_pLogoList;
	CLabeledCommandComboBox *m_pColorList;
	char m_LogoName[128];

	CCvarToggleCheckButton *m_pHighQualityModelCheckBox;

	vgui2::Dar<CCvarToggleCheckButton *> m_cvarToggleCheckButtons;

	CLabeledCommandComboBox *m_pCrosshairSize;
	CLabeledCommandComboBox *m_pCrosshairType;
	CCvarToggleCheckButton *m_pCrosshairTranslucencyCheckbox;
	CLabeledCommandComboBox *m_pCrosshairColorComboBox;
	CrosshairImagePanel *m_pCrosshairImage;

	int m_nLogoR, m_nLogoG, m_nLogoB;

	vgui2::DHANDLE<CMultiplayerAdvancedDialog> m_hMultiplayerAdvancedDialog;
};

#endif