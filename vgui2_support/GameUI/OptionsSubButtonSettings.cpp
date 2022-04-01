//========= Copyright ?1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
//=============================================================================
#include "EngineInterface.h"
#include "OptionsSubButtonSettings.h"

#include <vgui_controls/ListPanel.h>

#include "CvarToggleCheckButton.h"
#include "CvarSlider.h"
#include "LabeledCommandComboBox.h"
#include "VControlsListPanel.h"
#include "ModInfo.h"

#include <KeyValues.h>
// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>


COptionsSubButtonSettings::COptionsSubButtonSettings(vgui2::Panel *parent) : PropertyPage(parent, NULL)
{
	m_pButtonRSlider = new vgui2::Slider(this, "ColorR Slider");
	m_pButtonRSlider->SetRange(0, 255);

	m_pButtonGSlider = new vgui2::Slider(this, "ColorG Slider");
	m_pButtonGSlider->SetRange(0, 255);

	m_pButtonBSlider = new vgui2::Slider(this, "ColorB Slider");
	m_pButtonBSlider->SetRange(0, 255);

	m_pButtonASlider = new vgui2::Slider(this, "ColorAlpha Slider");
	m_pButtonASlider->SetRange(0, 255);

	m_pCommend = new vgui2::TextEntry(this, "Button Commend");
	m_pTexture = new vgui2::TextEntry(this, "Button Texture");
	m_pButtonName = new vgui2::TextEntry(this, "Button Name");

	m_pButtonHide = new vgui2::CheckButton(this, "Button_Hide", "#Button_Hide");
	m_pButtonSp = new vgui2::CheckButton(this, "Button_Sp", "#Button_Sp");
	m_pButtonMp = new vgui2::CheckButton(this, "Button_Mp", "#Button_Mp");
	m_pButtonAdditive = new vgui2::CheckButton(this, "Button_Additive", "#Button_Additive");
	m_pButtonLock = new vgui2::CheckButton(this, "Button_Lock", "#Button_Lock");
	m_pButtonPrecision = new vgui2::CheckButton(this, "Button_Precision", "#Button_Precision");

	m_pButtonSave = new vgui2::Button(this, "Save", "#Button_Save");
	m_pButtonSave->SetCommand("ButtonSave");

	m_pButtonDelete = new vgui2::Button(this, "Delete", "#Button_Delete");
	m_pButtonDelete->SetCommand("ButtonDelete");

	m_pButtonEditor = new vgui2::Button(this, "Editor", "#Button_Editor");
	m_pButtonEditor->SetCommand("ButtonEditor");

	m_pButtonReset = new vgui2::Button(this, "Reset", "#Button_Reset");
	m_pButtonReset->SetCommand("ButtonReset");

	m_pTextureList = new vgui2::ComboBox(this, "TextureListComboBox", 6, false);
	m_pButtonList = new vgui2::ComboBox(this,"ButtonListComboBox", 6, false);
	m_pButtonProfileList = new vgui2::ComboBox(this, "ButtonProfileListComboBox", 6, false);

	m_pProfileActivate = new vgui2::Button(this, "Profile Activate", "#Profile_Activate");
	m_pProfileActivate->SetCommand("ProfileActivate");

	m_pProfileDelete = new vgui2::Button(this, "Profile Delete", "#Profile_Delete");
	m_pProfileActivate->SetCommand("ProfileDelete");

	LoadControlSettings("Resource/optionssubbuttonsettings.res");
}

void COptionsSubButtonSettings::Com_EscapeCommand(char* newCommand, const char* oldCommand, int len)
{
	char c;
	int scripting = engine->pfnGetCvarFloat("cmd_scripting");

	while ((c = *oldCommand++) && len > 1)
	{
		if (c == '"')
		{
			*newCommand++ = '\\';
			len--;
		}

		if (scripting && c == '$')
		{
			*newCommand++ = '$';
			len--;
		}

		*newCommand++ = c;
		len--;
	}

	*newCommand++ = 0;
}

void COptionsSubButtonSettings::OnCommand(const char* command)
{
	if (!stricmp(command, "ButtonSave"))
	{
		char command[4096];
		char cmd[256];

		#define BUF_LENGTH 256
	

		//Com_EscapeCommand(cmd, this->command.GetBuffer(), 256);

		if (m_pButtonName->GetTextLength())
		{
			char szButtonName[BUF_LENGTH];
			char szTexture[BUF_LENGTH];
			m_pButtonName->GetText(szButtonName, BUF_LENGTH);
			m_pTexture->GetText(szTexture, BUF_LENGTH);

			snprintf(command, sizeof(command), "touch_addbutton \"%s\" \"%s\" \"%s\"\n",
				szButtonName,
				szTexture,
				cmd);
			engine->pfnClientCmd(command);

			snprintf(command, sizeof(command), "touch_setflags \"%s\" %i\n", szButtonName, curflags);
			engine->pfnClientCmd(command);
			snprintf(command, sizeof(command), "touch_setcolor \"%s\" %u %u %u %u\n", szButtonName,
				m_pButtonRSlider->GetValue(),
				m_pButtonGSlider->GetValue(),
				m_pButtonBSlider->GetValue(),
				m_pButtonASlider->GetValue());
			engine->pfnClientCmd(command);
			m_pButtonName->SetText("");
		}
		else
		{
			char szButtonName[BUF_LENGTH];
			char szTexture[BUF_LENGTH];
			m_pButtonName->GetText(szButtonName, BUF_LENGTH);
			m_pTexture->GetText(szTexture, BUF_LENGTH);

			snprintf(command, sizeof(command), "touch_settexture \"%s\" \"%s\"\n", szSelectedName, szTexture);
			engine->pfnClientCmd(command);
			snprintf(command, sizeof(command), "touch_setcommand \"%s\" \"%s\"\n", szSelectedName, cmd);
			engine->pfnClientCmd(command);
			snprintf(command, sizeof(command), "touch_setflags \"%s\" %i\n", szSelectedName, curflags);
			engine->pfnClientCmd(command);
			snprintf(command, sizeof(command), "touch_setcolor \"%s\" %u %u %u %u\n", szSelectedName,
				m_pButtonRSlider->GetValue(),
				m_pButtonGSlider->GetValue(),
				m_pButtonBSlider->GetValue(),
				m_pButtonASlider->GetValue());
			engine->pfnClientCmd(command);
		}
	}
	if (!stricmp(command, "ButtonDelete"))
	{

	}
	if (!stricmp(command, "ButtonEditor"))
	{

	}
	if (!stricmp(command, "ButtonReset"))
	{

	}
	if (!stricmp(command, "ProfileActivate"))
	{

	}
	if (!stricmp(command, "ProfileDelete"))
	{

	}

	BaseClass::OnCommand(command);
}

void COptionsSubButtonSettings::OnTextChanged(vgui2::Panel* panel)
{
	if (panel == m_pCommend)
		return;

	if (panel == m_pTexture)
		return;

	if (panel == m_pButtonName)
		return;

}

COptionsSubButtonSettings::~COptionsSubButtonSettings()
{
}

void COptionsSubButtonSettings::OnResetData()
{

	
}

void COptionsSubButtonSettings::OnApplyChanges()
{


}

void COptionsSubButtonSettings::OnControlModified()
{
	PostActionSignal(new KeyValues("ApplyButtonEnable"));
}