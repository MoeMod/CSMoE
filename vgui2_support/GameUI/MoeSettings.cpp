#include "GameUI/MoeSettings.h"

#include "vgui_controls/Button.h"
#include "vgui_controls/CheckButton.h"
#include "vgui_controls/PropertySheet.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/QueryBox.h"

#include "vgui/ILocalize.h"
#include "vgui/ISurface.h"
#include "vgui/ISystem.h"
#include "vgui/IVGui.h"

#include "OptionsSubButtonSettings.h"
#include "OptionsSubMoeSettings.h"
#include "OptionsSubTouch.h"
#include "ModInfo.h"

#include "KeyValues.h"

CMoeSettings::CMoeSettings(vgui2::Panel *parent) : PropertyDialog(parent, "OptionsDialog")
{
	int offset = 40;
	int swide, stall;

	vgui2::surface()->GetScreenSize(swide, stall);

	SetBounds(offset, offset,
		600, 400);
	SetSizeable(false);
	SetTitle("#GameUI_CSMoESettings", true);

	m_pOptionsSubMoeSettings = NULL;
	m_pOptionsSubButtonSettings = NULL;
	m_pOptionsSubTouch = NULL;

	m_pOptionsSubMoeSettings = new COptionsSubMoeSettings(this);
	m_pOptionsSubButtonSettings = new COptionsSubButtonSettings(this);
	m_pOptionsSubTouch = new COptionsSubTouch(this);

	AddPage(m_pOptionsSubMoeSettings, "#GameUI_CSMoESettings");
	AddPage(m_pOptionsSubButtonSettings, "#GameUI_TouchSettings");
	AddPage(m_pOptionsSubTouch, "#GameUI_Touch");

	SetApplyButtonVisible(true);
	GetPropertySheet()->SetTabWidth(150);
}

CMoeSettings::~CMoeSettings(void)
{
}

void CMoeSettings::Activate(void)
{
	BaseClass::Activate();
	
	ResetAllData();
	EnableApplyButton(true);
}

void CMoeSettings::Run(void)
{
	SetTitle("#GameUI_TouchButtonSettings", true);
	Activate();
}

void CMoeSettings::OnClose(void)
{
	BaseClass::OnClose();
}

void CMoeSettings::OnGameUIHidden(void)
{
	for (int i = 0; i < GetChildCount(); i++)
	{
		Panel *pChild = GetChild(i);

		if (pChild)
			PostMessage(pChild, new KeyValues("GameUIHidden"));
	}
}