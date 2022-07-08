#ifndef ZSHELTERTEAMHOUSINGSUBDLG_H
#define ZSHELTERTEAMHOUSINGSUBDLG_H

#ifdef _WIN32
#pragma once
#endif


#include "IViewportPanel.h"

#include <VGUI/IScheme.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Frame.h>

#include "cso_controls/RoundPanel.h"
#include "cso_controls/SignalImagePanel.h"

#include "configs/cfg_zsht.h"

constexpr const char* CZSHELTERTEAMHOUSINGSUBDLG_NAME = "ZShelterTeamHousingSubDialog";
constexpr const int CZSHELTERTEAMHOUSINGSUBDLG_BUILD_NUM = 6;

class ZShelterTeamHousingSubDialog : public vgui2::Frame, public IViewportPanel
{
private:
	DECLARE_CLASS_SIMPLE(ZShelterTeamHousingSubDialog, vgui2::Frame);

public:
	ZShelterTeamHousingSubDialog(Panel* parent, const char* panelName);
	~ZShelterTeamHousingSubDialog(void);

// IViewportPanel
public:
	void Init(void) override {};
	void VidInit(void) override { /*SetVisible(false);*/ }
	const char *GetName(void) override { return CZSHELTERTEAMHOUSINGSUBDLG_NAME; }
	void SetData(KeyValues *data) override {}
	void Reset(void) override {}
	void Update(void) override {}
	bool NeedsUpdate(void) override { return false; }
	bool HasInputElements(void) override { return true; }
	void ShowPanel(bool bShow) override;

// vgui2::Frame
public:
	vgui2::Panel* CreateControlByName(const char* controlName) override;
	void OnCommand(const char* command) override;
	void OnKillFocus() override;

protected:
	virtual void ApplySchemeSettings(vgui2::IScheme* pScheme) override;

public:
	DECLARE_VIEWPORT_PANEL_SIMPLE();
	void ShowByID(int id);
	void UpdateHousingSlot(int index, cl::zsht_buildmenu_s data);
	void ShowHousingSlot(int index, bool value);

private:
	struct HousingSlot_s
	{
		vgui2::SignalImagePanel* m_BtnImagePanel_n;
		vgui2::ImagePanel* m_BtnImagePanel_d;
		vgui2::Label* m_BtnLabel;
		vgui2::ImagePanel* m_CompleteImagePanel;
		vgui2::ImagePanel* m_WoodIcon;
		vgui2::ImagePanel* m_IronIcon;
		vgui2::ImagePanel* m_ElectricIcon;
		vgui2::Label* m_WoodLabel;
		vgui2::Label* m_IronLabel;
		vgui2::Label* m_ElectricLabel;
	};
	HousingSlot_s m_HousingSlot[CZSHELTERTEAMHOUSINGSUBDLG_BUILD_NUM];
	vgui2::Panel* m_BlockingPanel;
};

#endif