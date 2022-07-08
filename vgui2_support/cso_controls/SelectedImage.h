
#ifndef SelectedTextEntry_H
#define SelectedTextEntry_H

#ifdef _WIN32
#pragma once
#endif

#include <VGUI/IBorder.h>
#include <VGUI/IScheme.h>
#include <KeyValues.h>

#include <vgui_controls/ImagePanel.h>

class SelectedImage : public vgui2::Panel
{
	DECLARE_CLASS_SIMPLE(SelectedImage, Panel);

public:
	SelectedImage(vgui2::Panel *parent, const char *panelName) :
		Panel(parent, panelName) {}

	virtual void ApplySchemeSettings(vgui2::IScheme *pScheme) override
	{
		BaseClass::ApplySchemeSettings(pScheme);
		m_bImageBackground = true;
		m_pTopBackground[0] = vgui2::scheme()->GetImage("resource/control/selected/selected_top_left", true);
		m_pTopBackground[1] = vgui2::scheme()->GetImage("resource/control/selected/selected_top_center", true);
		m_pTopBackground[2] = vgui2::scheme()->GetImage("resource/control/selected/selected_top_right", true);
		m_pCenterBackground[0] = vgui2::scheme()->GetImage("resource/control/selected/selected_center_left", true);
		m_pCenterBackground[1] = vgui2::scheme()->GetImage("resource/control/selected/selected_center_center", true);
		m_pCenterBackground[2] = vgui2::scheme()->GetImage("resource/control/selected/selected_center_right", true);
		m_pBottomBackground[0] = vgui2::scheme()->GetImage("resource/control/selected/selected_bottom_left", true);
		m_pBottomBackground[1] = vgui2::scheme()->GetImage("resource/control/selected/selected_bottom_center", true);
		m_pBottomBackground[2] = vgui2::scheme()->GetImage("resource/control/selected/selected_bottom_Right", true);

		SetFgColor(GetSchemeColor("LabelDimText", pScheme));
	}

	virtual void PaintBackground() override
	{
		if (m_bImageBackground)
		{
			const int iOffset = 0;
			const int iRound = 18;
			int wide, tall;
			GetSize(wide, tall);
			m_pTopBackground[0]->SetPos(0, 0);
			m_pTopBackground[0]->SetSize(iRound, iRound);
			m_pTopBackground[0]->Paint();
			m_pTopBackground[1]->SetPos(iRound, 0);
			m_pTopBackground[1]->SetSize(wide - iRound * 2 - iOffset, iRound);
			m_pTopBackground[1]->Paint();
			m_pTopBackground[2]->SetPos(wide - iRound - iOffset, 0);
			m_pTopBackground[2]->SetSize(iRound, iRound);
			m_pTopBackground[2]->Paint();

			m_pCenterBackground[0]->SetPos(0, iRound);
			m_pCenterBackground[0]->SetSize(iRound, tall - iRound * 2);
			m_pCenterBackground[0]->Paint();
			m_pCenterBackground[1]->SetPos(iRound, iRound);
			m_pCenterBackground[1]->SetSize(wide - iRound * 2 - iOffset, tall - iRound * 2);
			m_pCenterBackground[1]->Paint();
			m_pCenterBackground[2]->SetPos(wide - iRound - iOffset, iRound);
			m_pCenterBackground[2]->SetSize(iRound, tall - iRound * 2);
			m_pCenterBackground[2]->Paint();

			m_pBottomBackground[0]->SetPos(0, tall - iRound);
			m_pBottomBackground[0]->SetSize(iRound, iRound);
			m_pBottomBackground[0]->Paint();
			m_pBottomBackground[1]->SetPos(iRound, tall - iRound);
			m_pBottomBackground[1]->SetSize(wide - iRound * 2 - iOffset, iRound);
			m_pBottomBackground[1]->Paint();
			m_pBottomBackground[2]->SetPos(wide - iRound, tall - iRound);
			m_pBottomBackground[2]->SetSize(iRound, iRound);
			m_pBottomBackground[2]->Paint();
			return;
		}
		BaseClass::PaintBackground();
	}
protected:
	bool m_bImageBackground;
	vgui2::IImage *m_pTopBackground[3];
	vgui2::IImage *m_pCenterBackground[3];
	vgui2::IImage *m_pBottomBackground[3];
};

#endif