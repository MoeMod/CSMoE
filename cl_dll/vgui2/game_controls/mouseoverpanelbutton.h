#ifndef MOUSEOVERPANELBUTTON_H
#define MOUSEOVERPANELBUTTON_H

#ifdef _WIN32
#pragma once
#endif

#include "vgui/IScheme.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/PHandle.h"
#include "vgui/KeyCode.h"
#include "FileSystem.h"

class MouseOverPanelButton : public vgui2::Button
{
	DECLARE_CLASS_SIMPLE(MouseOverPanelButton, vgui2::Button);

public:
    MouseOverPanelButton(vgui2::Panel *parent, const char *panelName, vgui2::EditablePanel *templatePanel) : Button(parent, panelName, "MouseOverButton")
	{
		m_pPanel = new vgui2::EditablePanel(parent, NULL);
		m_pPanel ->SetVisible(false);

		int x, y, wide, tall;
		templatePanel->GetBounds(x, y, wide, tall);

		int px, py;
		templatePanel->GetPinOffset(px, py);

		int rx, ry;
		templatePanel->GetResizeOffset(rx, ry);

		m_pPanel->SetBounds(x, y, wide, tall);
		m_pPanel->SetAutoResize(templatePanel->GetPinCorner(), templatePanel->GetAutoResize(), px, py, rx, ry);

		m_bPreserveArmedButtons = false;
		m_bUpdateDefaultButtons = false;
	}

	virtual void SetPreserveArmedButtons(bool bPreserve) { m_bPreserveArmedButtons = bPreserve; }
	virtual void SetUpdateDefaultButtons(bool bUpdate) { m_bUpdateDefaultButtons = bUpdate; }

	virtual void ShowPage(void)
	{
		if (s_lastPanel)
		{
			for (int i = 0; i < s_lastPanel->GetParent()->GetChildCount(); i++)
			{
                MouseOverPanelButton *pButton = dynamic_cast<MouseOverPanelButton *>(s_lastPanel->GetParent()->GetChild(i));

				if (pButton)
					pButton->HidePage();
			}
		}

		if (m_pPanel)
		{
			m_pPanel->SetVisible(true);
			m_pPanel->MoveToFront();
			s_lastPanel = m_pPanel;
		}
	}

	virtual void HidePage(void)
	{
		if (m_pPanel)
			m_pPanel->SetVisible(false);
	}

	virtual const char *GetClassPage(const char *className)
	{
		static char classPanel[_MAX_PATH];
		Q_snprintf(classPanel, sizeof(classPanel), "classes/%s.res", className);
		return classPanel;
	}

	virtual bool LoadClassPage(void)
	{
		const char *classPage = GetClassPage(GetName());

		m_pPanel->LoadControlSettings(classPage, "GAME");

		m_pPanel->LoadControlSettings("classes/default.res", "GAME");

		return true;
	}

	virtual void ApplySettings(KeyValues *resourceData)
	{
		BaseClass::ApplySettings(resourceData);

		LoadClassPage();
	}

	auto *GetClassPanel(void) { return m_pPanel; }

	virtual void OnCursorExited(void)
	{
		if (!m_bPreserveArmedButtons)
			BaseClass::OnCursorExited();
	}

	virtual void OnCursorEntered(void)
	{
		BaseClass::OnCursorEntered();

		if (!IsEnabled())
			return;

		if (m_bUpdateDefaultButtons)
			SetAsDefaultButton(1);

		if (m_bPreserveArmedButtons)
		{
			if (s_lastButton && s_lastButton != this)
                s_lastButton->SetArmed(false);

            s_lastButton = this;
		}

		if (m_pPanel)
		{
            if(!m_pPanel->IsVisible())
            {
                if (s_lastPanel && s_lastPanel->IsVisible())
                    s_lastPanel->SetVisible(false);

                ShowPage();
            }
		}
	}

	virtual void OnKeyCodeReleased(vgui2::KeyCode code)
	{
		BaseClass::OnKeyCodeReleased(code);

		if (m_bPreserveArmedButtons)
		{
			if (s_lastButton)
                s_lastButton->SetArmed(true);
		}
	}

protected:
	vgui2::EditablePanel *m_pPanel;
	bool m_bPreserveArmedButtons;
	bool m_bUpdateDefaultButtons;

public:
    static inline vgui2::DHANDLE<vgui2::Panel> s_lastPanel;
    static inline vgui2::DHANDLE<vgui2::Button> s_lastButton;
};

#endif
