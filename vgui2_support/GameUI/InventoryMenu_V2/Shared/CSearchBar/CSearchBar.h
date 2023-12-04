#pragma once

#include "GameUI/InventoryMenu_V2/InventoryMainMenu.h"

class CSearchBar : public vgui2::ComboBox
{
    DECLARE_CLASS_SIMPLE(CSearchBar, vgui2::ComboBox);

public:
    CSearchBar(vgui2::Panel* parent, const char* panelName, int numLines = 6) : vgui2::ComboBox(parent, panelName, numLines, true)
    {
        if (auto menu = this->GetMenu())
            menu->SetKeyBoardInputEnabled(false);
    }

public:
    virtual void OnSetFocus()
    {
        vgui2::TextEntry::OnSetFocus();
    }

    MESSAGE_FUNC(OnMenuItemSelected, "MenuItemSelected");
};