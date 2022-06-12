
#include "hud.h"
#include "CBaseViewport.h"
#include "zshelterteamhousingdlg.h"

#include <vgui/IScheme.h>
#include <vgui/IVGui.h>
#include <vgui/ISurface.h>
#include <vgui2/src/VPanel.h>

#include <vgui_controls/Tooltip.h>
#include <string>
#include <algorithm>

#include "configs/cfg_zsht.h"

using namespace vgui2;

ZShelterTeamHousingDialog::ZShelterTeamHousingDialog(void) : Frame(NULL, CZSHELTERTEAMHOUSINGDLG_NAME)
{
	SetScheme("ClientScheme");
	SetTitle("#CSO_zsht_HousingDialog", true);

	SetMoveable(false);
	SetSizeable(false);
	SetProportional(false);

	SetTitleBarVisible(false);
	SetAutoDelete(false);

	LoadControlSettings("resource/popup_zshelterteamhousingdlg.res", "GAME");

	int screenWide, screenTall;
	surface()->GetScreenSize(screenWide, screenTall);
	int wide, tall;
	GetSize(wide, tall);
	SetPos((screenWide / 2) - wide / 2, (screenTall / 2) - tall / 2);

	FindControl<Label>("TitleLabel")->SetScheme("BaseUI");

	m_pSubDialog = new ZShelterTeamHousingSubDialog(this, "ZShelterTeamHousingSubDialog");
	m_pSubDialog->ShowPanel(false);

	auto cfg = cl::ZombieShelterTeam_BuildMenuCfg();
	for (size_t i = 0; i < CZSHELTERTEAMHOUSINGDLG_BUILD_NUM; i++)
	{
		m_HousingSlot[i].m_BtnImagePanel_n = FindControl<SignalImagePanel>((std::string("BtnImagePanel_n_") + std::to_string(i + 1)).c_str());
		m_HousingSlot[i].m_BtnImagePanel_d = FindControl<ImagePanel>((std::string("BtnImagePanel_d_") + std::to_string(i + 1)).c_str());
		m_HousingSlot[i].m_BtnLabel = FindControl<Label>((std::string("BtnLabel_") + std::to_string(i + 1)).c_str());
		m_HousingSlot[i].m_CompleteImagePanel = FindControl<ImagePanel>((std::string("CompletePanel_") + std::to_string(i + 1)).c_str());
		m_HousingSlot[i].m_WoodIcon = FindControl<ImagePanel>((std::string("WoodIcon_") + std::to_string(i + 1)).c_str());
		m_HousingSlot[i].m_IronIcon = FindControl<ImagePanel>((std::string("IronIcon_") + std::to_string(i + 1)).c_str());
		m_HousingSlot[i].m_ElectricIcon = FindControl<ImagePanel>((std::string("ElectricIcon_") + std::to_string(i + 1)).c_str());
		m_HousingSlot[i].m_WoodLabel = FindControl<Label>((std::string("WoodLabel_") + std::to_string(i + 1)).c_str());
		m_HousingSlot[i].m_IronLabel = FindControl<Label>((std::string("IronLabel_") + std::to_string(i + 1)).c_str());
		m_HousingSlot[i].m_ElectricLabel = FindControl<Label>((std::string("ElectricLabel_") + std::to_string(i + 1)).c_str());
		m_HousingSlot[i].m_BtnImagePanel_d->SetVisible(false);
		m_HousingSlot[i].m_CompleteImagePanel->SetVisible(false);
#ifndef DISABLE_MOE_VGUI2_EXT
		m_HousingSlot[i].m_BtnLabel->SetTouchInputEnabled(false);
#endif
		m_HousingSlot[i].m_BtnLabel->SetMouseInputEnabled(false);

		auto iter = std::find_if(cfg.begin(), cfg.end(), [i](const cl::zsht_buildmenu_s& data) { return i == data.id; });
		if (iter != cfg.end())
		{
			if (iter->wood == 0 && iter->iron == 0 && iter->power == 0) {
				m_HousingSlot[i].m_WoodLabel->SetVisible(false);
				m_HousingSlot[i].m_IronLabel->SetVisible(false);
				m_HousingSlot[i].m_ElectricLabel->SetVisible(false);
				m_HousingSlot[i].m_WoodIcon->SetVisible(false);
				m_HousingSlot[i].m_IronIcon->SetVisible(false);
				m_HousingSlot[i].m_ElectricIcon->SetVisible(false);
			}
			else
			{
				m_HousingSlot[i].m_WoodLabel->SetText(std::to_string(iter->wood).c_str());
				m_HousingSlot[i].m_IronLabel->SetText(std::to_string(iter->iron).c_str());
				m_HousingSlot[i].m_ElectricLabel->SetText(std::to_string(iter->power).c_str());
			}
			m_HousingSlot[i].m_BtnImagePanel_n->SetCommand(iter->command.c_str());
		}

		if (i == 11)
			m_HousingSlot[i].m_BtnLabel->SetText("#CSO_zsht_Build_Menu_37");
		else if (i >= 3)
			m_HousingSlot[i].m_BtnLabel->SetText((std::string("#CSO_zsht_Build_Menu_") + std::to_string(i + 2)).c_str());

		m_HousingSlot[i].m_BtnImagePanel_n->GetTooltip()->SetText((std::string("#CSO_zsht_Build_Menu_Tooltip_") + std::to_string(i + (i >= 3 ? 2 : 1))).c_str());
		m_HousingSlot[i].m_BtnLabel->GetTooltip()->SetText(m_HousingSlot[i].m_BtnImagePanel_n->GetTooltip()->GetText());
	}
}

ZShelterTeamHousingDialog::~ZShelterTeamHousingDialog(void)
{
}


void ZShelterTeamHousingDialog::ApplySchemeSettings(IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	auto pSchemeBaseUI = vgui2::scheme()->GetIScheme(vgui2::scheme()->GetScheme("BaseUI"));
	for (size_t i = 0; i < CZSHELTERTEAMHOUSINGDLG_BUILD_NUM; i++)
	{
		if (m_HousingSlot[i].m_BtnImagePanel_n)
		{
			m_HousingSlot[i].m_BtnImagePanel_n->GetTooltip()->SetBackgroundSkin(pSchemeBaseUI, "DarkAlpha");
		}
	}
}

void ZShelterTeamHousingDialog::SetVisible(bool bShow)
{
	BaseClass::SetVisible(bShow);
}

void ZShelterTeamHousingDialog::ShowPanel(bool bShow)
{
	if (BaseClass::IsVisible() == bShow)
		return;

	if (bShow)
	{
		Activate();
		SetMouseInputEnabled(true);
	}
	else
	{
		SetVisible(false);
		SetMouseInputEnabled(false);
	}
}

void ZShelterTeamHousingDialog::OnClose(void)
{
	BaseClass::OnClose();
}

Panel* ZShelterTeamHousingDialog::CreateControlByName(const char* controlName)
{
	if (!Q_stricmp("SignalImagePanel", controlName))
	{
		auto pPanel = new SignalImagePanel(this, controlName);
		return pPanel;
	}
	else if (!Q_stricmp("RoundPanel", controlName))
	{
		auto pPanel = new RoundPanel(this, controlName, RoundPanel::TOOLTIP_LIST);
		return pPanel;
	}
	else
		return BaseClass::CreateControlByName(controlName);
}

void ZShelterTeamHousingDialog::OnCommand(const char* command)
{
	if (!strncmp(command, "zsht_RequestBuild", 17))
	{
		cl::gEngfuncs.pfnClientCmd(command);
		ShowPanel(false);
		return;
	}
	else if (!strncmp(command, "zsht_opensubmenu", 16))
	{
		m_pSubDialog->ShowByID(command[17] - '0');
		return;
	}
	
	BaseClass::OnCommand(command);
}

void ZShelterTeamHousingDialog::OnSetFocus()
{
	BaseClass::OnSetFocus();
}
