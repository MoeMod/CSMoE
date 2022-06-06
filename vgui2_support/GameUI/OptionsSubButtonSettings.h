//========= Copyright ?1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef OPTIONS_SUB_BUTTONSETTINGS_H
#define OPTIONS_SUB_BUTTONSETTINGS_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/PropertyPage.h>
#include <vector>
#include <string>

#define TOUCH_FL_HIDE			(1U << 0)
#define TOUCH_FL_NOEDIT			(1U << 1)
#define TOUCH_FL_CLIENT			(1U << 2)
#define TOUCH_FL_MP				(1U << 3)
#define TOUCH_FL_SP				(1U << 4)
#define TOUCH_FL_DEF_SHOW		(1U << 5)
#define TOUCH_FL_DEF_HIDE		(1U << 6)
#define TOUCH_FL_DRAW_ADDITIVE	(1U << 7)
#define TOUCH_FL_STROKE			(1U << 8)
#define TOUCH_FL_PRECISION		(1U << 9)

//-----------------------------------------------------------------------------
// Purpose: Touch Details, Part of OptionsDialog
//-----------------------------------------------------------------------------
class COptionsSubButtonSettings : public vgui2::PropertyPage
{
	DECLARE_CLASS_SIMPLE(COptionsSubButtonSettings, vgui2::PropertyPage);

public:
	COptionsSubButtonSettings(vgui2::Panel *parent);
	~COptionsSubButtonSettings();
	void Com_EscapeCommand(char* newCommand, const char* oldCommand, int len);
	void UpdateProfileList();
	void UpdateButtonList();

	virtual void OnResetData();
	virtual void OnApplyChanges();
	virtual void OnCommand(const char* command);

private:
	MESSAGE_FUNC( OnControlModified, "ControlModified" );
	MESSAGE_FUNC_PTR_CHARPTR( OnTextChanged, "TextChanged", panel, text );
	MESSAGE_FUNC( OnTick, "Tick" );
	struct
	{
		char szName[128];
		char szTexture[128];
		char szCommand[128];
		byte bColors[4];
		int  iFlags;
	} buttons[1024];

	vgui2::Slider* m_pButtonRSlider;
	vgui2::Slider* m_pButtonGSlider;
	vgui2::Slider* m_pButtonBSlider;
	vgui2::Slider* m_pButtonASlider;

	vgui2::CheckButton* m_pButtonHide;
	vgui2::CheckButton* m_pButtonSp;
	vgui2::CheckButton* m_pButtonMp;
	vgui2::CheckButton* m_pButtonAdditive;
	vgui2::CheckButton* m_pButtonPrecision;
	vgui2::CheckButton* m_pButtonLock;

	vgui2::Button* m_pButtonSave;
	vgui2::Button* m_pButtonDelete;
	vgui2::Button* m_pButtonEditor;
	vgui2::Button* m_pButtonReset;
	vgui2::Button* m_pProfileActivate;
	vgui2::Button* m_pProfileDelete;

	vgui2::TextEntry* m_pCommend;
	vgui2::TextEntry* m_pTexture;
	vgui2::TextEntry* m_pButtonName;

	vgui2::ComboBox* m_pTextureList;
	vgui2::ComboBox* m_pButtonList;
	vgui2::ComboBox* m_pButtonProfileList;

	std::vector<std::string> m_vecProfiles;
};



#endif // OPTIONS_SUB_AUDIO_H