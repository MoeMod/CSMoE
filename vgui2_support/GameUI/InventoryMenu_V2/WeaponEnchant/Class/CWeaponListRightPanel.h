#pragma once

#include "GameUI/InventoryMenu_V2/InventoryMainMenu.h"

class CWeaponListRightPanel : public vgui2::PropertyPage
{
    DECLARE_CLASS_SIMPLE(CWeaponListRightPanel, PropertyPage);

public:
    CWeaponListRightPanel(Panel* parent, const char* panelName) : PropertyPage(parent, panelName) {}

    MESSAGE_FUNC_PTR(OnPageTabActivated, "PageTabActivated", panel);
};