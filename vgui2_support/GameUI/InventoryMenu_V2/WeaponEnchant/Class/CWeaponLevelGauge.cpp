/***********************************************
	CWeaponLevelGauge
	Build in 2023/11
************************************************/

#include "GameUI/InventoryMenu_V2/WeaponEnchant/CWeaponEnchant.h"
#include "CWeaponLevelGauge.h"

#undef PostMessage

void CWeaponLevelGauge::OnMousePressed(vgui2::MouseCode code)
{
	if (code == vgui2::MouseCode::MOUSE_LEFT)
	{
		KeyValues* msg = new KeyValues("GaugeClicked");
		msg->SetPtr("gauge", this);

		Panel::PostMessage(GetParent(), msg);
	}

	ImagePanel::OnMousePressed(code);
}