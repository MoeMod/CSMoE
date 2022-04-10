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
}