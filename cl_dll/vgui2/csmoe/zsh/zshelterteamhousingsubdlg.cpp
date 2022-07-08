
#include "hud.h"
#include "CBaseViewport.h"
#include "zshelterteamhousingsubdlg.h"

#include <tier1/KeyValues.h>

#include <vgui/IVGUI.h>
#include <vgui/ISurface.h>
#include <vgui/IPanel.h>

#include <vgui_controls/Tooltip.h>
#include <string>

using namespace vgui2;

ZShelterTeamHousingSubDialog::ZShelterTeamHousingSubDialog(Panel* parent, const char* panelName) : Frame(parent, panelName)
{
	SetScheme("ClientScheme");

	SetMoveable(false);
	SetSizeable(false);
	SetProportional(false);

	SetTitleBarVisible(false);
	SetAutoDelete(false);
	AddActionSignalTarget(parent);

	LoadControlSettings("resource/popup_zshelterteamhousingsubdlg.res", "GAME");

	int screenWide, screenTall;
	surface()->GetScreenSize(screenWide, screenTall);
	int wide, tall;
	GetSize(wide, tall);
	SetPos((screenWide / 2) - wide / 2, (screenTall / 2) - tall / 2);

	FindControl<Label>("TitleLabel")->SetText(panelName);
	for (size_t i = 0; i < CZSHELTERTEAMHOUSINGSUBDLG_BUILD_NUM; i++)
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
		
	}

	m_BlockingPanel = new Panel(parent, "Blocking");
	int w, t;
	parent->GetSize(w, t);
	m_BlockingPanel->SetSize(w, t);
	m_BlockingPanel->SetVisible(false);
}

ZShelterTeamHousingSubDialog::~ZShelterTeamHousingSubDialog(void)
{
}

void ZShelterTeamHousingSubDialog::ApplySchemeSettings(IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	auto pSchemeBaseUI = vgui2::scheme()->GetIScheme(vgui2::scheme()->GetScheme("BaseUI"));
	for (size_t i = 0; i < CZSHELTERTEAMHOUSINGSUBDLG_BUILD_NUM; i++)
	{
		if (m_HousingSlot[i].m_BtnImagePanel_n)
		{
			m_HousingSlot[i].m_BtnImagePanel_n->GetTooltip()->SetBackgroundSkin(pSchemeBaseUI, "DarkAlpha");
		}
	}
	m_BlockingPanel->SetBgColor(Color(25, 25, 25, 200));
	m_BlockingPanel->SetPaintBackgroundEnabled(true);
	m_BlockingPanel->SetZPos(1);
}

void ZShelterTeamHousingSubDialog::ShowPanel(bool bShow)
{
	if (BaseClass::IsVisible() == bShow)
		return;

	if (bShow)
	{
		if (m_BlockingPanel) m_BlockingPanel->SetVisible(true);
		Activate();
	}
	else
	{
		if (m_BlockingPanel) m_BlockingPanel->SetVisible(false);
		SetVisible(false);
	}
}

Panel* ZShelterTeamHousingSubDialog::CreateControlByName(const char* controlName)
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

void ZShelterTeamHousingSubDialog::OnCommand(const char* command)
{
	if (!strncmp(command, "zsht_RequestBuild", 17))
	{
		ShowPanel(false);
		PostActionSignal(new KeyValues("Command", "command", command));
		return;
	}

	BaseClass::OnCommand(command);
}

void ZShelterTeamHousingSubDialog::OnKillFocus()
{
	ShowPanel(false);
	BaseClass::OnKillFocus();
}

void ZShelterTeamHousingSubDialog::ShowByID(int id)
{
	if (id > 3)
	{
		FindControl<Label>("TitleLabel")->SetText((std::string("#CSO_zsht_Build_Menu_") + std::to_string(id + 1)).c_str());
	}
	else
	{
		FindControl<Label>("TitleLabel")->SetText((std::string("#CSO_zsht_Build_Menu_") + std::to_string(id)).c_str());
	}

	int idx = 0;
	auto cfg = cl::ZombieShelterTeam_BuildMenuCfg();
	for (auto iter = cfg.begin(); iter != cfg.end(); ++iter)
	{
		if (iter->submenu != id)
			continue;

		UpdateHousingSlot(idx++, *iter);
	}
	for (; idx < CZSHELTERTEAMHOUSINGSUBDLG_BUILD_NUM; idx++)
	{
		ShowHousingSlot(idx, false);
	}
	ShowPanel(true);
}

void ZShelterTeamHousingSubDialog::UpdateHousingSlot(int index, cl::zsht_buildmenu_s data)
{
	if (index >= CZSHELTERTEAMHOUSINGSUBDLG_BUILD_NUM) return;

	ShowHousingSlot(index, true);
	m_HousingSlot[index].m_BtnImagePanel_n->SetCommand(data.command.c_str());
	m_HousingSlot[index].m_BtnImagePanel_n->GetTooltip()->SetText(data.Tooltip.c_str());
	m_HousingSlot[index].m_BtnLabel->GetTooltip()->SetText(m_HousingSlot[index].m_BtnImagePanel_n->GetTooltip()->GetText());
	m_HousingSlot[index].m_BtnLabel->SetText(data.Name.c_str());
	m_HousingSlot[index].m_WoodLabel->SetText(std::to_string(data.wood).c_str());
	m_HousingSlot[index].m_IronLabel->SetText(std::to_string(data.iron).c_str());
	m_HousingSlot[index].m_ElectricLabel->SetText(std::to_string(data.power).c_str());
}

void ZShelterTeamHousingSubDialog::ShowHousingSlot(int index, bool value)
{
	// TODO fix empty slot would show tooltips
	m_HousingSlot[index].m_BtnImagePanel_n->GetTooltip()->SetText("");
	m_HousingSlot[index].m_BtnLabel->GetTooltip()->SetText("");
	m_HousingSlot[index].m_BtnImagePanel_n->SetEnabled(value);
	m_HousingSlot[index].m_BtnLabel->SetVisible(value);
	m_HousingSlot[index].m_WoodIcon->SetVisible(value);
	m_HousingSlot[index].m_IronIcon->SetVisible(value);
	m_HousingSlot[index].m_ElectricIcon->SetVisible(value);
	m_HousingSlot[index].m_WoodLabel->SetVisible(value);
	m_HousingSlot[index].m_IronLabel->SetVisible(value);
	m_HousingSlot[index].m_ElectricLabel->SetVisible(value);
	
}