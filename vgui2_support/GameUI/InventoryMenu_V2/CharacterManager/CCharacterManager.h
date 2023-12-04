#pragma once

#include "GameUI/InventoryMenu_V2/InventoryMainMenu.h"

class CCharacterManager : public vgui2::PropertyPage
{
    DECLARE_CLASS_SIMPLE(CCharacterManager, PropertyPage);

public:
    CCharacterManager(vgui2::Panel* parent);

public:
    virtual void PerformLayout();
};