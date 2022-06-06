//========= Copyright ?1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
//=============================================================================
#include "EngineInterface.h"
#include "OptionsSubButtonSettings.h"

#include <vgui/IVGui.h>
#include <vgui_controls/ListPanel.h>

#include "CvarToggleCheckButton.h"
#include "CvarSlider.h"
#include "LabeledCommandComboBox.h"
#include "VControlsListPanel.h"
#include "ModInfo.h"
#include "filesystem.h"
#include "vgui_controls/MessageBox.h"

#include <KeyValues.h>
// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

struct touch_buttons_s
{
	char szName[128];
	char szTexture[128];
	char szCommand[128];
	byte bColors[4];
	int  iFlags;
};

std::vector<touch_buttons_s> g_vecTouchButtons;
bool g_bAddTouchButtonToList = false;
// Engine callback
extern "C" EXPORT void AddTouchButtonToList(const char* name, const char* texture, const char* command, unsigned char* color, int flags)
{
	touch_buttons_s temp;
	Q_strncpy(temp.szName, name, sizeof(temp.szName));
	Q_strncpy(temp.szTexture, texture, sizeof(temp.szTexture));
	Q_strncpy(temp.szCommand, command, sizeof(temp.szCommand));
	memcpy(temp.bColors, color, sizeof(temp.bColors));
	temp.iFlags = flags;
	g_vecTouchButtons.push_back(temp);
	g_bAddTouchButtonToList = true;
}

void COptionsSubButtonSettings::UpdateButtonList()
{
	if (!m_pButtonList) return;

	g_vecTouchButtons.clear();
	m_pButtonList->RemoveAll();
	vgui2::ivgui()->AddTickSignal(GetVPanel(), 1);
	engine->pfnClientCmd("touch_list\n");
}

void COptionsSubButtonSettings::UpdateProfileList()
{
	if (!m_pButtonProfileList) return;

	m_vecProfiles.clear();
	m_pButtonProfileList->RemoveAll();
	FileFindHandle_t findHandle = NULL;
	const char* pszFilename = vgui2::filesystem()->FindFirst("touch_presets/*.cfg", &findHandle);
	while (pszFilename)
	{
		m_vecProfiles.push_back(pszFilename);
		m_pButtonProfileList->AddItem(pszFilename, NULL);
		pszFilename = vgui2::filesystem()->FindNext(findHandle);
	}
	vgui2::filesystem()->FindClose(findHandle);

	findHandle = NULL;
	pszFilename = vgui2::filesystem()->FindFirst("touch_profiles/*.cfg", &findHandle);
	while (pszFilename)
	{
		m_vecProfiles.push_back(pszFilename);
		m_pButtonProfileList->AddItem(pszFilename, NULL);
		pszFilename = vgui2::filesystem()->FindNext(findHandle);
	}
	vgui2::filesystem()->FindClose(findHandle);
}

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
	m_pProfileDelete->SetCommand("ProfileDelete");

	LoadControlSettings("Resource/optionssubbuttonsettings.res");

	UpdateProfileList();
	m_pButtonProfileList->ActivateItem(0);

	UpdateButtonList();
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
		if (!m_pButtonName->GetTextLength() || !m_pCommend->GetTextLength() || !m_pTexture->GetTextLength())
		{
			auto msgbox = new vgui2::MessageBox("提示", "请完善按钮信息（名称、贴图、指令）", this);
			msgbox->SetOKButtonText("#CSO_OKl_Btn");
			msgbox->SetBounds(GetWide() / 2 - 150, GetTall() / 2 - 100, 300, 200);
			msgbox->DoModal();
			msgbox->Activate();
		}
		else
		{
#define BUF_LENGTH 256
			char command[4096];
			char cmd[BUF_LENGTH];

			int curflags = 0;
			if (m_pButtonHide->IsSelected()) curflags |= TOUCH_FL_HIDE;
			if (m_pButtonSp->IsSelected()) curflags |= TOUCH_FL_SP;
			if (m_pButtonMp->IsSelected()) curflags |= TOUCH_FL_MP;
			if (m_pButtonAdditive->IsSelected()) curflags |= TOUCH_FL_DRAW_ADDITIVE;
			if (m_pButtonLock->IsSelected()) curflags |= TOUCH_FL_NOEDIT;
			if (m_pButtonPrecision->IsSelected()) curflags |= TOUCH_FL_PRECISION;

			char szButtonName[BUF_LENGTH];
			char szTexture[BUF_LENGTH];
			m_pButtonName->GetText(szButtonName, BUF_LENGTH);
			m_pTexture->GetText(szTexture, BUF_LENGTH);
			m_pCommend->GetText(cmd, BUF_LENGTH);

			// TODO: after button updated, combox select the new one or edit one;
			auto iter = std::find_if(g_vecTouchButtons.begin(), g_vecTouchButtons.end(), [szButtonName](const touch_buttons_s& info) { return !stricmp(info.szName, szButtonName); });
			if (iter == g_vecTouchButtons.end())
			{
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
				snprintf(command, sizeof(command), "touch_settexture \"%s\" \"%s\"\n", szButtonName, szTexture);
				engine->pfnClientCmd(command);
				snprintf(command, sizeof(command), "touch_setcommand \"%s\" \"%s\"\n", szButtonName, cmd);
				engine->pfnClientCmd(command);
				snprintf(command, sizeof(command), "touch_setflags \"%s\" %i\n", szButtonName, curflags);
				engine->pfnClientCmd(command);
				snprintf(command, sizeof(command), "touch_setcolor \"%s\" %u %u %u %u\n", szButtonName,
					m_pButtonRSlider->GetValue(),
					m_pButtonGSlider->GetValue(),
					m_pButtonBSlider->GetValue(),
					m_pButtonASlider->GetValue());
				engine->pfnClientCmd(command);
			}
			UpdateButtonList();
		}
	}
	else if (!stricmp(command, "ButtonEditor"))
	{
		engine->pfnClientCmd("touch_enable \"1\";touch_in_menu \"1\";touch_enableedit");
	}
	else if (!stricmp(command, "ButtonReset"))
	{
		engine->pfnClientCmd("touch_removeall\n");
		engine->pfnClientCmd("touch_loaddefaults\n");
		UpdateButtonList();
	}
	else if (!g_vecTouchButtons.empty() && !stricmp(command, "ButtonDelete"))
	{
		char command[512];
		snprintf(command, 512, "touch_removebutton \"%s\"\n", g_vecTouchButtons[m_pButtonList->GetActiveItem()].szName);
		engine->pfnClientCmd(command);
		UpdateButtonList();
	}
	else if (!m_vecProfiles.empty())
	{
		if (!stricmp(command, "ProfileActivate"))
		{
			char szTips[512];
			snprintf(szTips, 512, "是否使用 %s 按键配置", m_vecProfiles[m_pButtonProfileList->GetActiveItem()].c_str());
			auto msgbox = new vgui2::MessageBox("提示", szTips, this);
			msgbox->SetOKButtonText("#CSO_OKl_Btn");
			msgbox->SetCancelButtonText("#CSO_Cancel_Btn");
			msgbox->SetCancelButtonVisible(true);
			msgbox->SetBounds(GetWide() / 2 - 150, GetTall() / 2 - 100, 300, 200);
			msgbox->SetCommand("Confirm_ProfileActivate");
			msgbox->DoModal();
			msgbox->Activate();
		}
		else if (!stricmp(command, "ProfileDelete"))
		{
			char szTips[512];
			snprintf(szTips, 512, "是否删除 %s 按键配置", m_vecProfiles[m_pButtonProfileList->GetActiveItem()].c_str());
			auto msgbox = new vgui2::MessageBox("提示", szTips, this);
			msgbox->SetOKButtonText("#CSO_OKl_Btn");
			msgbox->SetCancelButtonText("#CSO_Cancel_Btn");
			msgbox->SetCancelButtonVisible(true);
			msgbox->SetBounds(GetWide() / 2 - 150, GetTall() / 2 - 100, 300, 200);
			msgbox->SetCommand("Confirm_ProfileDelete");
			msgbox->DoModal();
			msgbox->Activate();
		}
		else if (!stricmp(command, "Confirm_ProfileActivate"))
		{
			char szCommand[256];
			snprintf(szCommand, 256, "exec \"%s\"\n", m_vecProfiles[m_pButtonProfileList->GetActiveItem()].c_str());
			engine->pfnClientCmd(szCommand);
			UpdateButtonList();
		}
		else if (!stricmp(command, "Confirm_ProfileDelete"))
		{
			char szCommand[256];
			snprintf(szCommand, 256, "touch_deleteprofile \"%s\"\n", m_vecProfiles[m_pButtonProfileList->GetActiveItem()].c_str());
			engine->pfnClientCmd(szCommand);
			UpdateProfileList();
		}
	}

	BaseClass::OnCommand(command);
}

void COptionsSubButtonSettings::OnTextChanged(vgui2::Panel* panel, const char* pszText)
{
	if (panel == m_pCommend)
		return;

	if (panel == m_pTexture)
		return;

	if (panel == m_pButtonName)
		return;

	if (panel == m_pButtonList)
	{
		auto iter = std::find_if(g_vecTouchButtons.begin(), g_vecTouchButtons.end(), [pszText](const touch_buttons_s& info) { return !stricmp(info.szName, pszText); });
		if (iter == g_vecTouchButtons.end())
			return;

		m_pButtonRSlider->SetValue(iter->bColors[0]);
		m_pButtonGSlider->SetValue(iter->bColors[1]);
		m_pButtonBSlider->SetValue(iter->bColors[2]);
		m_pButtonASlider->SetValue(iter->bColors[3]);
		m_pCommend->SetText(iter->szCommand);
		m_pTexture->SetText(iter->szTexture);
		m_pButtonName->SetText(iter->szName);

		m_pButtonHide->SetSelected(!!(iter->iFlags & TOUCH_FL_HIDE));
		m_pButtonSp->SetSelected(!!(iter->iFlags & TOUCH_FL_SP));
		m_pButtonMp->SetSelected(!!(iter->iFlags & TOUCH_FL_MP));
		m_pButtonAdditive->SetSelected(!!(iter->iFlags & TOUCH_FL_DRAW_ADDITIVE));
		m_pButtonLock->SetSelected(!!(iter->iFlags & TOUCH_FL_NOEDIT));
		m_pButtonPrecision->SetSelected(!!(iter->iFlags & TOUCH_FL_PRECISION));
	}
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

void COptionsSubButtonSettings::OnTick()
{
	if (!g_bAddTouchButtonToList) return;
	g_bAddTouchButtonToList = false;
	vgui2::ivgui()->RemoveTickSignal(GetVPanel());

	if (!m_pButtonList) return;
	for (auto iter = g_vecTouchButtons.begin(); iter != g_vecTouchButtons.end(); ++iter)
	{
		auto kv = new KeyValues("button");
		kv->SetPtr("data", &iter);
		m_pButtonList->AddItem(iter->szName, kv);
	}
	m_pButtonList->ActivateItem(0);
}
