/***********************************************
	CWeaponPaint
	Build in 2023/11
************************************************/

#include "CWeaponPaint.h"
#include "GameUI/InventoryMenu_V2/Util/Util.h"

CWeaponPaint::CWeaponPaint(Panel* parent) : PropertyPage(parent, "WeaponPaint")
{
	SetScheme("ClientScheme");

	m_pScrollBar = new ScrollBar(this, "VerticalScrollBar", true);
	m_pScrollBar->AddActionSignalTarget(this);
	m_pScrollBar->SetWide(16);

	IScheme* pScheme = scheme()->GetIScheme(GetScheme());
	HFont hFont = pScheme->GetFont("CreditsTitle", false);

	m_pLabel = new Label(this, "WpnLabel", "#CSMOE_InventoryMneu_NoContent");
	m_pLabel->SetContentAlignment(Label::a_center);
	m_pLabel->SizeToContents();
	m_pLabel->SetFont(hFont);
}

void CWeaponPaint::PerformLayout()
{
	BaseClass::PerformLayout();

	SetBounds(0, 20, 1360, 580);

	int iW, iH;
	GetParent()->GetSize(iW, iH);

	iH -= 100;

	int iW2, iH2;
	m_pLabel->SetSize(500, 30);
	m_pLabel->GetSize(iW2, iH2);
	m_pLabel->SetPos((iW - iW2) / 2, (iH - iH2) / 2);


	m_pScrollBar->SetAutoResize(PIN_TOPRIGHT, AUTORESIZE_DOWN, 0, 0, 0, 0);
	m_pScrollBar->SetRange(0,50);
	m_pScrollBar->SetRangeWindow(4);
	m_pScrollBar->SetButtonPressedScrollValue(4);
}