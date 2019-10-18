#include <vgui/ISurface.h>

#include "CCSBackGroundPanel.h"
#include "vgui_int.h"

using namespace vgui2;

CCSBackGroundPanel::CCSBackGroundPanel(Panel* parent) : BaseClass(parent)
{
	SetBounds(scheme()->GetProportionalScaledValue(20), scheme()->GetProportionalScaledValue(20), scheme()->GetProportionalScaledValue(590), scheme()->GetProportionalScaledValue(430));
}

void CCSBackGroundPanel::SetTitleText(const wchar_t* text)
{
	m_pTitleLabel->SetText(text);
}

void CCSBackGroundPanel::SetTitleText(const char* text)
{
	m_pTitleLabel->SetText(text);
}

static int GetAlternateProportionalValueFromNormal(int normalizedValue)
{
	int wide, tall;
	surface()->GetScreenSize(wide, tall);

	int proH, proW;
	surface()->GetProportionalBase(proW, proH);

	double scaleH = (double)tall / (double)proH;
	double scaleW = (double)wide / (double)proW;
	double scale = (scaleW < scaleH) ? scaleW : scaleH;

	return (int)(normalizedValue * scale);
}

static int GetAlternateProportionalValueFromScaled(HScheme hScheme, int scaledValue)
{
	return GetAlternateProportionalValueFromNormal(scheme()->GetProportionalNormalizedValue(scaledValue));
}

static void RepositionControl(Panel* pPanel)
{
	int x, y, w, h;
	pPanel->GetBounds(x, y, w, h);

	x = GetAlternateProportionalValueFromScaled(pPanel->GetScheme(), x);
	y = GetAlternateProportionalValueFromScaled(pPanel->GetScheme(), y);
	w = GetAlternateProportionalValueFromScaled(pPanel->GetScheme(), w);
	h = GetAlternateProportionalValueFromScaled(pPanel->GetScheme(), h);

	pPanel->SetBounds(x, y, w, h);
}

void ResizeWindowControls(EditablePanel* pWindow, int tall, int wide, int offsetX, int offsetY)
{
	if (!pWindow || !pWindow->GetBuildGroup() || !pWindow->GetBuildGroup()->GetPanelList())
		return;

	CUtlVector<PHandle>* panelList = pWindow->GetBuildGroup()->GetPanelList();
	CUtlVector<Panel*> resizedPanels;
	CUtlVector<Panel*> movedPanels;

	for (int i = 0; i < panelList->Size(); ++i)
	{
		PHandle handle = (*panelList)[i];
		Panel* panel = handle.Get();
		bool found = false;

		for (int j = 0; j < resizedPanels.Size(); ++j)
		{
			if (panel == resizedPanels[j])
				found = true;
		}

		if (!panel || found)
		{
			continue;
		}

		resizedPanels.AddToTail(panel);

		if (panel != pWindow)
		{
			RepositionControl(panel);
		}
	}

	for (int i = 0; i < panelList->Size(); ++i)
	{
		PHandle handle = (*panelList)[i];
		Panel* panel = handle.Get();
		bool found = false;

		for (int j = 0; j < movedPanels.Size(); ++j)
		{
			if (panel == movedPanels[j])
				found = true;
		}

		if (!panel || found)
		{
			continue;
		}

		movedPanels.AddToTail(panel);

		if (panel != pWindow)
		{
			int x, y;

			panel->GetPos(x, y);
			panel->SetPos(x + offsetX, y + offsetY);
		}
	}
}

void CCSBackGroundPanel::Activate(void)
{
	if (!m_enabled)
		return;

	BaseClass::Activate();
	PerformLayout();
	
}

void CCSBackGroundPanel::ApplySchemeSettings(IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}

void CCSBackGroundPanel::PerformLayout(void)
{
	int screenW, screenH;

	surface()->GetScreenSize(screenW, screenH);

	SetPos(screenW / 2 - scheme()->GetProportionalScaledValue(303), screenH / 2 - scheme()->GetProportionalScaledValue(210));

	if (IsProportional())
	{
		SetSize(scheme()->GetProportionalScaledValue(606), scheme()->GetProportionalScaledValue(420));
	}
	
	BaseClass::PerformLayout();
}