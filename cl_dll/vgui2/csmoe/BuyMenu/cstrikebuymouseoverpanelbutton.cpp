#include <stdio.h>
#include <wchar.h>
#include <UtlSymbol.h>

#include <vgui/IBorder.h>
#include <vgui/IInput.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui/IVGui.h>
#include <vgui/ILocalize.h>
#include <vgui/MouseCode.h>
#include <vgui/KeyCode.h>
#include <KeyValues.h>

#include <vgui_controls/TextImage.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/EditablePanel.h>

#include "CBaseViewPort.h"

#include "cstrikebuymouseoverpanelbutton.h"
#include "cstrikebuymouseoverpanel.h"

#include <string>

using namespace vgui2;

const Color COL_NONE = { 255, 255, 255, 255 };
const Color COL_CT = { 192, 205, 224, 255 };
const Color COL_TR = { 216, 182, 183, 255 };

CSBuyMouseOverPanelButton::CSBuyMouseOverPanelButton(vgui2::Panel *parent, const char *panelName, vgui2::EditablePanel *panel)
	: BaseClass(parent, panelName, panel)
{
	if (m_pPanel)
		delete m_pPanel;
	m_pPanel = new CSBuyMouseOverPanel(parent, "ItemInfo");

	m_pWeaponImage = new WeaponImagePanel(this, "WeaponImage");
	m_pWeaponImage->SetShouldScaleImage(true);
	m_pWeaponImage->SetShouldCenterImage(true);
	m_pWeaponImage->SetMouseInputEnabled(false);
	m_pWeaponImage->SetKeyBoardInputEnabled(false);

	m_pLockedImage = new vgui2::ImagePanel(this, "LevelLock");
	m_pLockedImageBg = new vgui2::ImagePanel(this, "LevelLock_Bg");

	m_pBlankBg = new vgui2::ImagePanel(this, "Blank_Bg");
	m_pBlankBg->SetShouldScaleImage(true);

	m_pLevelText = new vgui2::Label(this, "LevelText", "");
}

void CSBuyMouseOverPanelButton::UpdateWeapon(const char *weapon)
{
    if(weapon && weapon[0])
    {
        m_pWeaponImage->SetVisible(true);
        CSBuyMouseOverPanel *panel = dynamic_cast<CSBuyMouseOverPanel *>(m_pPanel);
        if (panel)
            panel->UpdateWeapon(weapon);

        m_pWeaponImage->SetWeapon(weapon);
    }
    else
    {
        m_pWeaponImage->SetVisible(false);
    }
}
void CSBuyMouseOverPanelButton::SetBanWeapon(const char* weapon, int iLevel)
{
	if (weapon && weapon[0])
	{
		char SzLevel[32]; sprintf(SzLevel, "Lv.%d", iLevel);
		m_pLevelText->SetText(SzLevel);
		m_pLevelText->SetFgColor({ 255,99,71,255 });

		m_pLockedImage->SetImage("resource/zombiez/level_lock_l");
		m_pLockedImageBg->SetImage("resource/zombiez/level_lock_bg");

		m_pBlankBg->SetImage("resource/pcbangpremium_bg_red");
		m_pBlankBg->SetBgColor({ 255,99,71,255 });

		m_pBlankBg->SetVisible(true);
		m_pLevelText->SetVisible(true);
		m_pLockedImage->SetVisible(true);
		m_pLockedImageBg->SetVisible(true);
	}
	else
	{
		m_pLevelText->SetText("");
		m_pLockedImage->SetImage("");
		m_pLevelText->SetVisible(false);
		m_pLockedImage->SetVisible(false);
		m_pLockedImageBg->SetVisible(false);
		m_pBlankBg->SetVisible(false);
	}
}
void CSBuyMouseOverPanelButton::Paint()
{
	Color col(200, 200, 200, 255);
	if (m_iTeam == TERRORIST)
	{
		col = COL_TR;
	}
	else if (m_iTeam == CT)
	{
		col = COL_CT;
	}
	SetFgColor(col);
	BaseClass::Paint();
}

void CSBuyMouseOverPanelButton::PerformLayout()
{
	BaseClass::PerformLayout();
	int  x, y, w, h;
	GetBounds(x, y, w, h);

	int newWide = h * 2.9;
	m_pWeaponImage->SetBounds(w - newWide, 0, newWide, h);

	m_pLockedImage->SetPos(vgui2::scheme()->GetProportionalScaledValue(6), 0 + vgui2::scheme()->GetProportionalScaledValue(6));
	m_pLockedImageBg->SetPos(w - newWide / 1.5 - vgui2::scheme()->GetProportionalScaledValue(3), vgui2::scheme()->GetProportionalScaledValue(8));
	m_pLevelText->SetPos(w - newWide / 1.5 + vgui2::scheme()->GetProportionalScaledValue(3), vgui2::scheme()->GetProportionalScaledValue(5));

	m_pBlankBg->SetSize(w, h);
	m_pBlankBg->SetPos(0, 0);
}