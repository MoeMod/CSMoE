
#include "vgui_int.h"
#include <vgui/ISurface.h>
#include "CSBackGroundPanel.h"

using namespace vgui2;

#define DEBUG_WINDOW_RESIZING 0
#define DEBUG_WINDOW_REPOSITIONING 0

extern engine_studio_api_t IEngineStudio;

CCSBackGroundPanel::CCSBackGroundPanel(Panel *parent) : BaseClass(parent)
{
	m_pTopLeftPanel = new CBitmapImagePanel(this, "TopLeftPanel", "gfx/vgui/round_corner_nw");
	m_pTopRightPanel = new CBitmapImagePanel(this, "TopRightPanel", "gfx/vgui/round_corner_ne");
	m_pBottomLeftPanel = new CBitmapImagePanel(this, "BottomLeftPanel", "gfx/vgui/round_corner_sw");
	m_pBottomRightPanel = new CBitmapImagePanel(this, "BottomRightPanel", "gfx/vgui/round_corner_se");

	m_pGapPanel = new Panel(this, "GapPanel");
	m_pTitleLabel = new Label(this, "CaptionLabel", "");

	m_pExclamationPanel = new CBitmapImagePanel(this, "ExclamationPanel", "gfx/vgui/CS_logo");

	m_offsetX = 0;
	m_offsetY = 0;

	LoadControlSettings("Resource/UI/BackgroundPanel.res", "GAME");
	m_enabled = true;

	m_pTopLeftPanel->SetVisible(m_enabled);
	m_pTopRightPanel->SetVisible(m_enabled);
	m_pBottomLeftPanel->SetVisible(m_enabled);
	m_pBottomRightPanel->SetVisible(m_enabled);
	m_pGapPanel->SetVisible(m_enabled);
	m_pTitleLabel->SetVisible(m_enabled);

	m_pExclamationPanel->SetVisible(false);
	
}

void CCSBackGroundPanel::SetTitleText(const wchar_t *text)
{
	m_pTitleLabel->SetText(text);
}

void CCSBackGroundPanel::SetTitleText(const char *text)
{
	m_pTitleLabel->SetText(text);
}

void CCSBackGroundPanel::PaintBackground(void)
{
	if (!m_enabled)
		return;

	surface()->DrawSetColor(m_bgColor);

	if (IEngineStudio.IsHardware())
	{
		int x1, y1, x2, y2, x3, y3;

		if (m_pGapPanel->GetTall() > m_pGapPanel->GetWide())
		{
			m_pTopLeftPanel->GetPos(x1, y1);
			m_pBottomLeftPanel->GetPos(x2, y2);

			surface()->DrawFilledRect(x1, y1 + m_pTopLeftPanel->GetTall(), x2 + m_pBottomLeftPanel->GetWide(), y2);

			m_pTopLeftPanel->GetPos(x1, y1);
			m_pGapPanel->GetPos(x2, y2);

			surface()->DrawFilledRect(x1 + m_pTopLeftPanel->GetWide(), y1, x2, y2 + m_pGapPanel->GetTall());

			m_pGapPanel->GetPos(x1, y1);
			m_pBottomRightPanel->GetPos(x2, y2);

			surface()->DrawFilledRect(x1 + m_pGapPanel->GetWide(), y1, x2, y2 + m_pBottomRightPanel->GetTall());

			m_pTopRightPanel->GetPos(x1, y1);
			m_pBottomRightPanel->GetPos(x2, y2);

			surface()->DrawFilledRect(x1, y1 + m_pTopRightPanel->GetTall(), x2 + m_pBottomRightPanel->GetWide(), y2);
		}
		else
		{
			m_pTopLeftPanel->GetPos(x1, y1);
			m_pTopRightPanel->GetPos(x2, y2);

			surface()->DrawFilledRect(x1 + m_pTopLeftPanel->GetWide(), y1, x2, y2 + m_pTopRightPanel->GetTall());

			m_pTopLeftPanel->GetPos(x1, y1);
			m_pGapPanel->GetPos(x2, y2);
			m_pTopRightPanel->GetPos(x3, y3);

			surface()->DrawFilledRect(x1, y1 + m_pTopLeftPanel->GetTall(), x3 + m_pTopRightPanel->GetWide(), y2);

			m_pGapPanel->GetPos(x1, y1);
			m_pBottomRightPanel->GetPos(x2, y2);

			surface()->DrawFilledRect(x1, y1 + m_pGapPanel->GetTall(), x2 + m_pBottomRightPanel->GetWide(), y2);

			m_pBottomLeftPanel->GetPos(x1, y1);
			m_pBottomRightPanel->GetPos(x2, y2);

			surface()->DrawFilledRect(x1 + m_pBottomLeftPanel->GetWide(), y1, x2, y2 + m_pBottomRightPanel->GetTall());
		}
	}
}

static int GetAlternateProportionalValueFromNormal(int normalizedValue)
{
	int wide, tall;
	GetHudSize(wide, tall);

	int proH, proW;
	surface()->GetProportionalBase(proW, proH);

	double scaleH = (double)tall / (double)proH;
	double scaleW = (double)wide / (double)proW;
	double scale = (scaleW < scaleH) ? scaleW : scaleH;

	return (int)(normalizedValue * scale);
}

static int GetAlternateProportionalValueFromScaled(HScheme hScheme, int scaledValue)
{
	return GetAlternateProportionalValueFromNormal(scheme()->GetProportionalNormalizedValueEx(hScheme, scaledValue));
}

static void RepositionControl(Panel *pPanel)
{
	int x, y, w, h;
	pPanel->GetBounds(x, y, w, h);

#if DEBUG_WINDOW_RESIZING
	int x1, y1, w1, h1;
	pPanel->GetBounds(x1, y1, w1, h1);

	int x2, y2, w2, h2;
	x2 = scheme()->GetProportionalNormalizedValueEx(pPanel->GetScheme(), x1);
	y2 = scheme()->GetProportionalNormalizedValueEx(pPanel->GetScheme(), y1);
	w2 = scheme()->GetProportionalNormalizedValueEx(pPanel->GetScheme(), w1);
	h2 = scheme()->GetProportionalNormalizedValueEx(pPanel->GetScheme(), h1);
#endif

	x = GetAlternateProportionalValueFromScaled(pPanel->GetScheme(), x);
	y = GetAlternateProportionalValueFromScaled(pPanel->GetScheme(), y);
	w = GetAlternateProportionalValueFromScaled(pPanel->GetScheme(), w);
	h = GetAlternateProportionalValueFromScaled(pPanel->GetScheme(), h);

	pPanel->SetBounds(x, y, w, h);

#if DEBUG_WINDOW_RESIZING
	DevMsg("Resizing '%s' from (%d,%d) %dx%d to (%d,%d) %dx%d -- initially was (%d,%d) %dx%d\n", pPanel->GetName(), x1, y1, w1, h1, x, y, w, h, x2, y2, w2, h2);
#endif
}

static void ResizeWindowControls(EditablePanel *pWindow, int tall, int wide, int offsetX, int offsetY)
{
	if (!pWindow || !pWindow->GetBuildGroup() || !pWindow->GetBuildGroup()->GetPanelList())
		return;

	CUtlVector<PHandle> *panelList = pWindow->GetBuildGroup()->GetPanelList();
	CUtlVector<Panel *> resizedPanels;
	CUtlVector<Panel *> movedPanels;

	for (int i = 0; i < panelList->Size(); ++i)
	{
		PHandle handle = (*panelList)[i];
		Panel *panel = handle.Get();
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
		Panel *panel = handle.Get();
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

#if DEBUG_WINDOW_REPOSITIONING
			DevMsg("Repositioning '%s' from (%d,%d) to (%d,%d) -- a distance of (%d,%d)\n", panel->GetName(), x, y, x + offsetX, y + offsetY, offsetX, offsetY);
#endif
		}
	}
}

void CCSBackGroundPanel::Activate(void)
{
	if (!m_enabled)
		return;

	SetPos(0, 0);

	BaseClass::Activate();

	if (IsProportional())
	{
		int screenW, screenH;
		GetHudSize(screenW, screenH);

		int wide, tall;
		GetSize(wide, tall);

		if (wide != screenW || tall != screenH)
		{
			SetSize(screenW, screenH);

			wide = GetAlternateProportionalValueFromScaled(GetScheme(), scheme()->GetProportionalNormalizedValue(640));
			tall = GetAlternateProportionalValueFromScaled(GetScheme(), scheme()->GetProportionalNormalizedValue(480));

			int offsetX = (screenW - wide) / 2;
			int offsetY = (screenH - tall) / 2;

			if (offsetX != 0 && offsetY != 0)
			{
				if (offsetX > offsetY)
				{
					offsetX = offsetX - offsetY;
					offsetY = 0;
				}
				else
				{
					offsetY = offsetY - offsetX;
					offsetX = 0;
				}
			}

			ResizeWindowControls(this, tall, wide, offsetX, offsetY);
		}
	}
}

void CCSBackGroundPanel::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_titleColor = pScheme->GetColor("TitleIcon", Color(0, 0, 0, 200));
	m_bgColor = pScheme->GetColor("BgColor", Color(0, 0, 0, 200));
	m_borderColor = Color(0, 0, 0, 200);

	if (!IEngineStudio.IsHardware())
	{
		m_pTopLeftPanel->setImageColor(pScheme->GetColor("BorderBright", Color(0, 0, 0, 0)));
		m_pTopRightPanel->setImageColor(Color(0, 0, 0, 0));
		m_pBottomLeftPanel->setImageColor(Color(0, 0, 0, 0));
		m_pBottomRightPanel->setImageColor(Color(0, 0, 0, 0));
	}
	else
	{
		Color bgColor = Color(255, 255, 255, pScheme->GetColor("BgColor", Color(0, 0, 0, 0))[3]);

		m_pTopLeftPanel->setImageColor(bgColor);
		m_pTopRightPanel->setImageColor(bgColor);
		m_pBottomLeftPanel->setImageColor(bgColor);
		m_pBottomRightPanel->setImageColor(bgColor);
	}
}

void CCSBackGroundPanel::PerformLayout(void)
{
	BaseClass::PerformLayout();
}