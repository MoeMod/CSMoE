#ifndef ZB2ZOMBEKEEPER_H
#define ZB2ZOMBEKEEPER_H


#ifdef _WIN32
#pragma once
#endif

#include "game_controls/classmenu.h"
#include <vgui_controls/EditablePanel.h>
#include <FileSystem.h>
#include "vgui_controls/imagepanel.h"
#include "cso_controls/NewTabButton.h"
#include "shared_util.h"
#include "csmoe/newmouseoverpanelbutton.h"

#include "cso_controls/RoundPanel.h"
#include "cso_controls/SignalImagePanel.h"

#define PANEL_ZOMBIEKEEPER "ZombieKeeper"

using namespace vgui2;

class CZb2ZombieKeeper : public vgui2::Frame, public IViewportPanel
{
private:
	DECLARE_CLASS_SIMPLE(CZb2ZombieKeeper, vgui2::Frame);

public:
	CZb2ZombieKeeper(void);
	~CZb2ZombieKeeper(void);

	// IViewportPanel
public:
	void Init(void) override {};
	void VidInit(void) override { /*SetVisible(false);*/ }
	const char* GetName(void) override { return PANEL_ZOMBIEKEEPER; }
	void SetData(KeyValues* data) override {}
	void Reset(void) override {}
	void Update(void) override {}
	bool NeedsUpdate(void) override { return false; }
	bool HasInputElements(void) override { return true; }
	void ShowPanel(bool bShow) override;
	void SetVisible(bool bShow) override;

	// vgui2::Frame
public:
	virtual void PaintBackground() override;
	virtual void PerformLayout(void) override;
	virtual void OnThink() override;
	vgui2::Panel* CreateControlByName(const char* controlName) override;
	void OnClose(void) override;
	void OnCommand(const char* command) override;
	void OnSetFocus() override;
	void SetupPage(size_t iPage);

	void SetLockSlot(int i, int SzLevel);
	void ReSetLockSlot(int i);
	void SetBanSlot(int i);
	void ReSetBanSlot(int i);

protected:
	virtual void ApplySchemeSettings(vgui2::IScheme* pScheme) override;
	vgui2::EditablePanel* m_pPanel;

public:
	DECLARE_VIEWPORT_PANEL_SIMPLE();

private:

	vgui2::Label* m_pTitleLabel;

	size_t m_iCurrentPage;
	bool m_bBanned[10];

	struct ZombieKeeperSlot_s
	{
		vgui2::SignalImagePanel* m_BtnImagePanel;
		vgui2::SignalImagePanel* m_BtnControlPanel[3];

		vgui2::Label* m_BtnControlPanelText[3];
		vgui2::Label* m_pChooseTime;

		vgui2::ImagePanel* m_pLockedImage;
		vgui2::ImagePanel* m_pLockedImageBg;
		vgui2::ImagePanel* m_pLockedLevelBar;
		vgui2::Label* m_pLockedLevel;

		vgui2::ImagePanel* m_pBannedImage;
	};
	ZombieKeeperSlot_s m_ZombieKeeperSlot[17];

};
#endif

