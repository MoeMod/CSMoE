#include "hud.h"
#include "classmenu.h"

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui_controls/ImageList.h>
#include <FileSystem.h>
#include <IGameUIFuncs.h>

#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Panel.h>

#include "game_controls/mouseoverpanelbutton.h"

using namespace vgui2;

CClassMenu::CClassMenu(IViewport* pViewPort) : Frame(NULL, PANEL_CLASS), m_pViewPort(pViewPort)
{
	m_iTeam = 0;

	SetTitle("", true);
	SetScheme("ClientScheme");
	SetMoveable(false);
	SetSizeable(false);

	SetTitleBarVisible(false);
	SetProportional(true);

	m_pPanel = new EditablePanel(this, "ClassInfo");

	LoadControlSettings("Resource/UI/ClassMenu.res", "GAME");
}

CClassMenu::~CClassMenu(void)
{
}

MouseOverPanelButton *CClassMenu::CreateNewMouseOverPanelButton(EditablePanel *panel)
{
	return new MouseOverPanelButton(this, "MouseOverPanelButton", panel);
}

Panel *CClassMenu::CreateControlByName(const char *controlName)
{
	if (!Q_stricmp("MouseOverPanelButton", controlName))
	{
		MouseOverPanelButton *newButton = CreateNewMouseOverPanelButton(m_pPanel);
		m_mouseoverButtons.AddToTail(newButton);
		return newButton;
	}
	else
	{
		return BaseClass::CreateControlByName(controlName);
	}
}

void CClassMenu::Reset(void)
{
	for (int i = 0; i < GetChildCount(); ++i)
	{
		MouseOverPanelButton *pPanel = dynamic_cast<MouseOverPanelButton *>(GetChild(i));

		if (pPanel)
			pPanel->HidePage();
	}

	for (int i = 0; i < m_mouseoverButtons.Count(); ++i)
		m_mouseoverButtons[i]->HidePage();
}

void CClassMenu::OnCommand(const char *command)
{
	if (Q_stricmp(command, "vguicancel"))
	{
		cl::gEngfuncs.pfnClientCmd(command);
	}

	Close();

	m_pViewPort->ShowBackGround(false);

	BaseClass::OnCommand(command);
}

void CClassMenu::ShowPanel(bool bShow)
{
	if (BaseClass::IsVisible() == bShow)
		return;

	if (bShow)
	{
		Activate();
		SetMouseInputEnabled(true);

		for (int i = 0; i < m_mouseoverButtons.Count(); ++i)
		{
			if (i == 0)
				m_mouseoverButtons[i]->ShowPage();
			else
				m_mouseoverButtons[i]->HidePage();
		}
	}
	else
	{
		SetVisible(false);
		SetMouseInputEnabled(false);
	}

	m_pViewPort->ShowBackGround(bShow);
}

void CClassMenu::SetData(KeyValues *data)
{
	m_iTeam = data->GetInt("team");
}

void CClassMenu::SetLabelText(const char *textEntryName, const char *text)
{
	Label *entry = dynamic_cast<Label *>(FindChildByName(textEntryName));

	if (entry)
		entry->SetText(text);
}

void CClassMenu::SetEnableButton(const char *textEntryName, bool state)
{
	Button *entry = dynamic_cast<Button *>(FindChildByName(textEntryName));

	if (entry)
		entry->SetEnabled(state);
}

void CClassMenu::SetVisibleButton(const char *textEntryName, bool state)
{
	Button *entry = dynamic_cast<Button *>(FindChildByName(textEntryName));

	if (entry)
		entry->SetVisible(state);
}

void CClassMenu::OnKeyCodePressed(KeyCode code)
{
	BaseClass::OnKeyCodePressed(code);
}