#include "EngineInterface.h"
#include "VControlsListPanel.h"

#include <vgui/IInput.h>
#include <vgui/IInputInternal.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui/Cursor.h>
#include <KeyValues.h>

class CInlineEditPanel : public vgui2::Panel
{
public:
	CInlineEditPanel(void) : vgui2::Panel(NULL, "InlineEditPanel")
	{
	}

	virtual void Paint(void)
	{
		int x = 0, y = 0, wide, tall;
		GetSize(wide, tall);

		vgui2::surface()->DrawSetColor(255, 165, 0, 255);
		vgui2::surface()->DrawFilledRect(x, y, x + wide, y + tall);
	}

	virtual void OnKeyCodeTyped(vgui2::KeyCode code)
	{
		if (GetParent())
			GetParent()->OnKeyCodeTyped(code);
	}

	virtual void ApplySchemeSettings(vgui2::IScheme *pScheme)
	{
		Panel::ApplySchemeSettings(pScheme);
		SetBorder(pScheme->GetBorder("DepressedButtonBorder"));
	}

	void OnMousePressed(vgui2::MouseCode code)
	{
		if (GetParent())
			GetParent()->OnMousePressed(code);
	}
};

VControlsListPanel::VControlsListPanel(vgui2::Panel *parent, const char *listName) : vgui2::SectionedListPanel(parent, listName)
{
	m_bCaptureMode = false;
	m_nClickRow = 0;
	m_pInlineEditPanel = new CInlineEditPanel();
	m_hFont = vgui2::INVALID_FONT;
}

VControlsListPanel::~VControlsListPanel(void)
{
	m_pInlineEditPanel->MarkForDeletion();
}

void VControlsListPanel::ApplySchemeSettings(vgui2::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	m_hFont = pScheme->GetFont("Default", IsProportional());
}

void VControlsListPanel::StartCaptureMode(vgui2::HCursor hCursor)
{
	m_bCaptureMode = true;
	EnterEditMode(m_nClickRow, 1, m_pInlineEditPanel);
	vgui2::input()->SetMouseFocus(m_pInlineEditPanel->GetVPanel());
	vgui2::input()->SetMouseCapture(m_pInlineEditPanel->GetVPanel());

	if (hCursor)
	{
		m_pInlineEditPanel->SetCursor(hCursor);
		vgui2::input()->GetCursorPos(m_iMouseX, m_iMouseY);
	}
}

void VControlsListPanel::EndCaptureMode(vgui2::HCursor hCursor)
{
	m_bCaptureMode = false;
	vgui2::input()->SetMouseCapture(NULL);
	LeaveEditMode();
	RequestFocus();
	vgui2::input()->SetMouseFocus(GetVPanel());

	if (hCursor)
	{
		m_pInlineEditPanel->SetCursor(hCursor);
		vgui2::surface()->SetCursor(hCursor);

		if (hCursor != vgui2::dc_none)
			vgui2::input()->SetCursorPos(m_iMouseX, m_iMouseY);
	}
}

void VControlsListPanel::SetItemOfInterest(int itemID)
{
	m_nClickRow = itemID;
}

int VControlsListPanel::GetItemOfInterest(void)
{
	return m_nClickRow;
}

bool VControlsListPanel::IsCapturing(void)
{
	return m_bCaptureMode;
}

void VControlsListPanel::OnMousePressed(vgui2::MouseCode code)
{
	if (IsCapturing())
	{
		if (GetParent())
			GetParent()->OnMousePressed(code);
	}
	else
		BaseClass::OnMousePressed(code);
}

void VControlsListPanel::OnMouseDoublePressed(vgui2::MouseCode code)
{
	if (IsItemIDValid(GetSelectedItem()))
		OnKeyCodePressed(vgui2::KEY_ENTER);
	else
		BaseClass::OnMouseDoublePressed(code);
}