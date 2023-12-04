#pragma once

#include "GameUI/InventoryMenu_V2/InventoryMainMenu.h"

class CWeaponPaint : public vgui2::PropertyPage
{
    DECLARE_CLASS_SIMPLE(CWeaponPaint, PropertyPage);

public:
    CWeaponPaint(vgui2::Panel* parent);

public:
    virtual void PerformLayout();

private:
    vgui2::Label* m_pLabel;

    vgui2::ScrollBar* m_pScrollBar;
};