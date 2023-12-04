#pragma once

#include "GameUI/InventoryMenu_V2/InventoryMainMenu.h"

class CWeaponLevelGauge : public vgui2::ImagePanel
{
public:
	CWeaponLevelGauge(vgui2::Panel* parent, const char* panelName) : vgui2::ImagePanel(parent, panelName)
	{
	}

protected:
	void OnMousePressed(vgui2::MouseCode code);
};