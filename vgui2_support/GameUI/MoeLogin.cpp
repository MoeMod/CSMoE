
#include "MoeLogin.h"
#include "EngineInterface.h"
#include "OptionsSubButtonSettings.h"

#include <vgui/IScheme.h>
#include <vgui/IVGui.h>
#include <vgui/ISurface.h>
#include <vgui_controls/ListPanel.h>

#include "CvarToggleCheckButton.h"
#include "CvarSlider.h"
#include "LabeledCommandComboBox.h"
#include "VControlsListPanel.h"
#include "ModInfo.h"
#include "filesystem.h"
#include "vgui_controls/MessageBox.h"

#include <KeyValues.h>

using namespace vgui2;

MoeLogIn::MoeLogIn(void) : Frame(NULL, CMOELOGIN_NAME)
{
	SetScheme("ClientScheme");

	int screenWide, screenTall;
	surface()->GetScreenSize(screenWide, screenTall);
	int wide, tall;
	GetSize(wide, tall);
	SetBounds((screenWide / 2) - wide / 2, (screenTall / 2) - tall / 2,
		wide, tall);

	SetSizeable(false);

	/*m_pAccount = new vgui2::TextEntry(this, "TextEntry_Account");
	m_pPassword = new vgui2::TextEntry(this, "TextEntry_PassWord");

	m_pValidation = new vgui2::Button(this, "validation", "#validation");
	m_pValidation->SetCommand("Validation");
	m_pLogin = new vgui2::Button(this, "Account_Button", "#Account_Button");
	m_pLogin->SetCommand("LogIn");
	m_pExit = new vgui2::Button(this, "Close_Button", "#Close_Button");
	m_pExit->SetCommand("Exit");

	m_pAutoSave = new vgui2::CheckButton(this, "Password_AutoSave", "#Password_AutoSave");
	*/
	LoadControlSettings("resource/moelogin.res");

}

MoeLogIn::~MoeLogIn(void)
{
}

void MoeLogIn::ApplySchemeSettings(IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}

void MoeLogIn::SetVisible(bool bShow)
{
	BaseClass::SetVisible(bShow);
}

void MoeLogIn::OnClose(void)
{
	BaseClass::OnClose();
}

void MoeLogIn::OnCommand(const char* command)
{
	BaseClass::OnCommand(command);
	/*char cmd[64];
	
	if (!stricmp(command, "get_token"))
	{
		char szAccount[16];
		m_pAccount->GetText(szAccount, 16);
		snprintf(m_szAccount, sizeof(m_szAccount), szAccount);
		snprintf(cmd, sizeof(cmd), "setinfo qq %s;get_token %s", szAccount, szAccount);
		engine->pfnClientCmd(cmd);
		return;
	}

	if (!stricmp(command, "userlogin"))
	{
		engine->pfnClientCmd("login");
		BaseClass::Close();
		return;
	}

	if (!stricmp(command, "Validation"))
	{ 
		if (!m_pAccount->GetTextLength())
		{
			auto msgbox = new vgui2::MessageBox("#CSMoE_Tips", "#CSMoE_VarifyWarning_1", this);
			msgbox->SetOKButtonText("#CSO_OKl_Btn");
			msgbox->SetBounds(GetWide() / 2 - 150, GetTall() / 2 - 100, 300, 200);
			msgbox->DoModal();
			msgbox->Activate();
			return;
		}
		else if (m_pAccount->GetTextLength() < 8)
		{
			auto msgbox = new vgui2::MessageBox("#CSMoE_Tips", "#CSMoE_VarifyWarning_2", this);
			msgbox->SetOKButtonText("#CSO_OKl_Btn");
			msgbox->SetBounds(GetWide() / 2 - 150, GetTall() / 2 - 100, 300, 200);
			msgbox->DoModal();
			msgbox->Activate();
			return;
		}
		else
		{
			auto msgbox = new vgui2::MessageBox("#CSMoE_Tips", "#CSMoE_VarifyWarning_3", this);
			msgbox->SetOKButtonText("#CSO_OKl_Btn");
			msgbox->SetBounds(GetWide() / 2 - 150, GetTall() / 2 - 100, 300, 200);
			msgbox->DoModal();
			msgbox->Activate();
			msgbox->SetCommand("get_token");
			return;
		}	
	}
	if (!stricmp(command, "LogIn"))
	{
		if (m_pAccount->GetTextLength() > 8)
		{
			char szAccount[16];
			m_pAccount->GetText(szAccount, 16);
			if (!stricmp(m_szAccount, szAccount))
			{
				if (m_pPassword->GetTextLength() < 6)
				{
					auto msgbox = new vgui2::MessageBox("#CSMoE_Tips", "#CSMoE_VarifyWarning_5", this);
					msgbox->SetOKButtonText("#CSO_OKl_Btn");
					msgbox->SetBounds(GetWide() / 2 - 150, GetTall() / 2 - 100, 300, 200);
					msgbox->DoModal();
					msgbox->Activate();
					return;
				}
				else
				{
					char szPassword[6];
					m_pPassword->GetText(szPassword, 6);
					//snprintf(cmd, sizeof(cmd), "setinfo token %s", szPassword);
					snprintf(cmd, sizeof(cmd), "login %s %s", m_szAccount, szPassword);
					engine->pfnClientCmd(cmd);
					return;
				}
			}
			else
			{
				auto msgbox = new vgui2::MessageBox("#CSMoE_Tips", "#CSMoE_VarifyWarning_4", this);
				msgbox->SetOKButtonText("#CSO_OKl_Btn");
				msgbox->SetBounds(GetWide() / 2 - 150, GetTall() / 2 - 100, 300, 200);
				msgbox->DoModal();
				msgbox->Activate();
				return;
			}

		}
		else
		{
			auto msgbox = new vgui2::MessageBox("#CSMoE_Tips", "#CSMoE_VarifyWarning_4", this);
			msgbox->SetOKButtonText("#CSO_OKl_Btn");
			msgbox->SetBounds(GetWide() / 2 - 150, GetTall() / 2 - 100, 300, 200);
			msgbox->DoModal();
			msgbox->Activate();
			return;
		}
	}
	if (!stricmp(command, "Exit"))
	{
		engine->pfnClientCmd("disconnect");
		BaseClass::Close();
	}*/
}

void MoeLogIn::OnSetFocus()
{
	BaseClass::OnSetFocus();
}
