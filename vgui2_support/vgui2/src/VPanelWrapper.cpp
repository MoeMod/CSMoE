
#include <vgui_controls/Controls.h>

#include "VPanel.h"
#include "VPanelWrapper.h"

#include <vgui/IClientPanel.h>
#include <vgui/IPanel.h>
#include <vgui/ISurface.h>


using vgui2::IPanel;

EXPOSE_SINGLE_INTERFACE(VPanelWrapper, IPanel, VGUI_PANEL_INTERFACE_VERSION);

void VPanelWrapper::Init(vgui2::VPANEL vguiPanel, vgui2::IClientPanel *panel)
{
	reinterpret_cast<vgui2::VPanel*>(vguiPanel)->Init(panel);
}

void VPanelWrapper::SetPos(vgui2::VPANEL vguiPanel, int x, int y)
{
	reinterpret_cast<vgui2::VPanel*>(vguiPanel)->SetPos(x, y);
}

void VPanelWrapper::GetPos(vgui2::VPANEL vguiPanel, int &x, int &y)
{
	reinterpret_cast<vgui2::VPanel*>(vguiPanel)->GetPos(x, y);
}

void VPanelWrapper::SetSize(vgui2::VPANEL vguiPanel, int wide, int tall)
{
	reinterpret_cast<vgui2::VPanel*>(vguiPanel)->SetSize(wide, tall);
}

void VPanelWrapper::GetSize(vgui2::VPANEL vguiPanel, int &wide, int &tall)
{
	reinterpret_cast<vgui2::VPanel*>(vguiPanel)->GetSize(wide, tall);
}

void VPanelWrapper::SetMinimumSize(vgui2::VPANEL vguiPanel, int wide, int tall)
{
	reinterpret_cast<vgui2::VPanel*>(vguiPanel)->SetMinimumSize(wide, tall);
}

void VPanelWrapper::GetMinimumSize(vgui2::VPANEL vguiPanel, int &wide, int &tall)
{
	reinterpret_cast<vgui2::VPanel*>(vguiPanel)->GetMinimumSize(wide, tall);
}

void VPanelWrapper::SetZPos(vgui2::VPANEL vguiPanel, int z)
{
	reinterpret_cast<vgui2::VPanel*>(vguiPanel)->SetZPos(z);
}

int VPanelWrapper::GetZPos(vgui2::VPANEL vguiPanel)
{
	return reinterpret_cast<vgui2::VPanel*>(vguiPanel)->GetZPos();
}

void VPanelWrapper::GetAbsPos(vgui2::VPANEL vguiPanel, int &x, int &y)
{
	reinterpret_cast<vgui2::VPanel*>(vguiPanel)->GetAbsPos(x, y);
}

void VPanelWrapper::GetClipRect(vgui2::VPANEL vguiPanel, int &x0, int &y0, int &x1, int &y1)
{
	reinterpret_cast<vgui2::VPanel*>(vguiPanel)->GetClipRect(x0, y0, x1, y1);
}

void VPanelWrapper::SetInset(vgui2::VPANEL vguiPanel, int left, int top, int right, int bottom)
{
	reinterpret_cast<vgui2::VPanel*>(vguiPanel)->SetInset(left, top, right, bottom);
}

void VPanelWrapper::GetInset(vgui2::VPANEL vguiPanel, int &left, int &top, int &right, int &bottom)
{
	reinterpret_cast<vgui2::VPanel*>(vguiPanel)->GetInset(left, top, right, bottom);
}

void VPanelWrapper::SetVisible(vgui2::VPANEL vguiPanel, bool state)
{
	reinterpret_cast<vgui2::VPanel*>(vguiPanel)->SetVisible(state);
}

bool VPanelWrapper::IsVisible(vgui2::VPANEL vguiPanel)
{
	return reinterpret_cast<vgui2::VPanel*>(vguiPanel)->IsVisible();
}

void VPanelWrapper::SetParent(vgui2::VPANEL vguiPanel, vgui2::VPANEL newParent)
{
	reinterpret_cast<vgui2::VPanel*>(vguiPanel)->SetParent(reinterpret_cast<vgui2::VPanel*>(newParent));
}

int VPanelWrapper::GetChildCount(vgui2::VPANEL vguiPanel)
{
	return reinterpret_cast<vgui2::VPanel*>(vguiPanel)->GetChildCount();
}

vgui2::VPANEL VPanelWrapper::GetChild(vgui2::VPANEL vguiPanel, int index)
{
	return reinterpret_cast<vgui2::VPANEL>(reinterpret_cast<vgui2::VPanel*>(vguiPanel)->GetChild(index));
}

vgui2::VPANEL VPanelWrapper::GetParent(vgui2::VPANEL vguiPanel)
{
	return reinterpret_cast<vgui2::VPANEL>(reinterpret_cast<vgui2::VPanel*>(vguiPanel)->GetParent());
}

void VPanelWrapper::MoveToFront(vgui2::VPANEL vguiPanel)
{
	reinterpret_cast<vgui2::VPanel*>(vguiPanel)->MoveToFront();
}

void VPanelWrapper::MoveToBack(vgui2::VPANEL vguiPanel)
{
	reinterpret_cast<vgui2::VPanel*>(vguiPanel)->MoveToBack();
}

bool VPanelWrapper::HasParent(vgui2::VPANEL vguiPanel, vgui2::VPANEL potentialParent)
{
	if (!vguiPanel)
		return false;

	return reinterpret_cast<vgui2::VPanel*>(vguiPanel)->HasParent(reinterpret_cast<vgui2::VPanel*>(potentialParent));
}

bool VPanelWrapper::IsPopup(vgui2::VPANEL vguiPanel)
{
	return reinterpret_cast<vgui2::VPanel*>(vguiPanel)->IsPopup();
}

void VPanelWrapper::SetPopup(vgui2::VPANEL vguiPanel, bool state)
{
	reinterpret_cast<vgui2::VPanel*>(vguiPanel)->SetPopup(state);
}

bool VPanelWrapper::Render_GetPopupVisible(vgui2::VPANEL vguiPanel)
{
	return reinterpret_cast<vgui2::VPanel*>(vguiPanel)->Render_IsPopupPanelVisible();
}

void VPanelWrapper::Render_SetPopupVisible(vgui2::VPANEL vguiPanel, bool state)
{
	reinterpret_cast<vgui2::VPanel*>(vguiPanel)->Render_SetPopupVisible(state);
}

vgui2::HScheme VPanelWrapper::GetScheme(vgui2::VPANEL vguiPanel)
{
	return reinterpret_cast<vgui2::VPanel*>(vguiPanel)->GetScheme();
}

bool VPanelWrapper::IsProportional(vgui2::VPANEL vguiPanel)
{
	return Client(vguiPanel)->IsProportional();
}

bool VPanelWrapper::IsAutoDeleteSet(vgui2::VPANEL vguiPanel)
{
	return Client(vguiPanel)->IsAutoDeleteSet();
}

void VPanelWrapper::DeletePanel(vgui2::VPANEL vguiPanel)
{
	Client(vguiPanel)->DeletePanel();
}

void VPanelWrapper::SetKeyBoardInputEnabled(vgui2::VPANEL vguiPanel, bool state)
{
	reinterpret_cast<vgui2::VPanel*>(vguiPanel)->SetKeyBoardInputEnabled(state);
}

void VPanelWrapper::SetMouseInputEnabled(vgui2::VPANEL vguiPanel, bool state)
{
	reinterpret_cast<vgui2::VPanel*>(vguiPanel)->SetMouseInputEnabled(state);
}

bool VPanelWrapper::IsKeyBoardInputEnabled(vgui2::VPANEL vguiPanel)
{
	return reinterpret_cast<vgui2::VPanel*>(vguiPanel)->IsKeyBoardInputEnabled();
}

bool VPanelWrapper::IsMouseInputEnabled(vgui2::VPANEL vguiPanel)
{
	return reinterpret_cast<vgui2::VPanel*>(vguiPanel)->IsMouseInputEnabled();
}

void VPanelWrapper::Solve(vgui2::VPANEL vguiPanel)
{
	reinterpret_cast<vgui2::VPanel*>(vguiPanel)->Solve();
}

const char *VPanelWrapper::GetName(vgui2::VPANEL vguiPanel)
{
	return reinterpret_cast<vgui2::VPanel*>(vguiPanel)->GetName();
}

const char *VPanelWrapper::GetClassName(vgui2::VPANEL vguiPanel)
{
	return reinterpret_cast<vgui2::VPanel*>(vguiPanel)->GetClassName();
}

void VPanelWrapper::SendMessage(vgui2::VPANEL vguiPanel, KeyValues *params, vgui2::VPANEL ifromPanel)
{
	reinterpret_cast<vgui2::VPanel*>(vguiPanel)->SendMessage(params, ifromPanel);
}

void VPanelWrapper::Think(vgui2::VPANEL vguiPanel)
{
	Client(vguiPanel)->Think();
}

void VPanelWrapper::PerformApplySchemeSettings(vgui2::VPANEL vguiPanel)
{
	Client(vguiPanel)->PerformApplySchemeSettings();
}

void VPanelWrapper::PaintTraverse(vgui2::VPANEL vguiPanel, bool forceRepaint, bool allowForce)
{
	Client(vguiPanel)->PaintTraverse(forceRepaint, allowForce);
}

void VPanelWrapper::Repaint(vgui2::VPANEL vguiPanel)
{
	Client(vguiPanel)->Repaint();
}

vgui2::VPANEL VPanelWrapper::IsWithinTraverse(vgui2::VPANEL vguiPanel, int x, int y, bool traversePopups)
{
	return Client(vguiPanel)->IsWithinTraverse(x, y, traversePopups);
}

void VPanelWrapper::OnChildAdded(vgui2::VPANEL vguiPanel, vgui2::VPANEL child)
{
	Client(vguiPanel)->OnChildAdded(child);
}

void VPanelWrapper::OnSizeChanged(vgui2::VPANEL vguiPanel, int newWide, int newTall)
{
	Client(vguiPanel)->OnSizeChanged(newWide, newTall);
}

void VPanelWrapper::InternalFocusChanged(vgui2::VPANEL vguiPanel, bool lost)
{
	Client(vguiPanel)->InternalFocusChanged(lost);
}

bool VPanelWrapper::RequestInfo(vgui2::VPANEL vguiPanel, KeyValues *outputData)
{
	return Client(vguiPanel)->RequestInfo(outputData);
}

void VPanelWrapper::RequestFocus(vgui2::VPANEL vguiPanel, int direction)
{
	Client(vguiPanel)->RequestFocus(direction);
}

bool VPanelWrapper::RequestFocusPrev(vgui2::VPANEL vguiPanel, vgui2::VPANEL existingPanel)
{
	return Client(vguiPanel)->RequestFocusPrev(existingPanel);
}

bool VPanelWrapper::RequestFocusNext(vgui2::VPANEL vguiPanel, vgui2::VPANEL existingPanel)
{
	return Client(vguiPanel)->RequestFocusNext(existingPanel);
}

vgui2::VPANEL VPanelWrapper::GetCurrentKeyFocus(vgui2::VPANEL vguiPanel)
{
	return Client(vguiPanel)->GetCurrentKeyFocus();
}

int VPanelWrapper::GetTabPosition(vgui2::VPANEL vguiPanel)
{
	return Client(vguiPanel)->GetTabPosition();
}

vgui2::SurfacePlat *VPanelWrapper::Plat(vgui2::VPANEL vguiPanel)
{
	return reinterpret_cast<vgui2::VPanel*>(vguiPanel)->Plat();
}

void VPanelWrapper::SetPlat(vgui2::VPANEL vguiPanel, vgui2::SurfacePlat *Plat)
{
	reinterpret_cast<vgui2::VPanel*>(vguiPanel)->SetPlat(Plat);
}

vgui2::Panel *VPanelWrapper::GetPanel(vgui2::VPANEL vguiPanel, const char *destinationModule)
{
	if (vgui2::surface()->GetEmbeddedPanel() != vguiPanel &&
		!stricmp(GetModuleName(vguiPanel), destinationModule))
	{
		return static_cast<vgui2::Panel*>(Client(vguiPanel)->QueryInterface(vgui2::ICLIENTPANEL_STANDARD_INTERFACE));
	}

	return nullptr;
}

bool VPanelWrapper::IsEnabled(vgui2::VPANEL vguiPanel)
{
	return reinterpret_cast<vgui2::VPanel*>(vguiPanel)->IsEnabled();
}

void VPanelWrapper::SetEnabled(vgui2::VPANEL vguiPanel, bool state)
{
	reinterpret_cast<vgui2::VPanel*>(vguiPanel)->SetEnabled(state);
}

vgui2::IClientPanel* VPanelWrapper::Client(vgui2::VPANEL vguiPanel)
{
	return reinterpret_cast<vgui2::VPanel*>(vguiPanel)->Client();
}

const char* VPanelWrapper::GetModuleName(vgui2::VPANEL vguiPanel)
{
	return Client(vguiPanel)->GetModuleName();
}
