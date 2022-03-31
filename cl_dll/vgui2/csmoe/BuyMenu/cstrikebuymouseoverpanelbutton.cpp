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

CSBuyMouseOverPanelButton::CSBuyMouseOverPanelButton(vgui2::Panel *parent, const char *panelName, vgui2::EditablePanel *panel)
	: BaseClass(parent, panelName, panel)
{
	if (m_pPanel)
		delete m_pPanel;
	m_pPanel = new CSBuyMouseOverPanel(parent, "ItemInfo");

	m_pWeaponImage = new vgui2::ImagePanel(this, "WeaponImage");
	m_pWeaponImage->SetShouldScaleImage(true);
	m_pWeaponImage->SetShouldCenterImage(true);
	m_pWeaponImage->SetMouseInputEnabled(false);
	m_pWeaponImage->SetKeyBoardInputEnabled(false);
}

void CSBuyMouseOverPanelButton::UpdateWeapon(const char *weapon)
{
	CSBuyMouseOverPanel *panel = dynamic_cast<CSBuyMouseOverPanel *>(m_pPanel);
	if (panel)
		panel->UpdateWeapon(weapon);

	char szBuffer[64];
	sprintf(szBuffer, "gfx\\vgui\\%s", weapon);
	m_pWeaponImage->SetImage(szBuffer);
}

void CSBuyMouseOverPanelButton::Paint()
{
	Color col(200, 200, 200, 255);
	if (m_iTeam == TERRORIST)
	{
		col = Color(255, 65, 65, 255);
	}
	else if (m_iTeam == CT)
	{
		col = Color(65, 65, 255, 255);
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