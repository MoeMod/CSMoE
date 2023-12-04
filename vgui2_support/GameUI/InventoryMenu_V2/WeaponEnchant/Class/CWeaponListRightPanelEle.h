#pragma once

#include "GameUI/InventoryMenu_V2/InventoryMainMenu.h"

class CWeaponListRightPanelEle : public vgui2::ImagePanel
{
    DECLARE_CLASS_SIMPLE(CWeaponListRightPanelEle, ImagePanel);

public:
    CWeaponListRightPanelEle(vgui2::Panel* parent, const char* name);

public:
    void SetId(int id);
    void SetWeapon(char* szWpnName,int iCurrentLevel = 0);
    void SetSelected(bool bSelected);
    void ReleaseIcon();

    void ClickToSelect();

public:
    virtual void OnMousePressed(vgui2::MouseCode code);
    virtual void OnMouseDoublePressed(vgui2::MouseCode code);

protected:
    virtual void ApplySchemeSettings(vgui2::IScheme* pScheme);

private:
    vgui2::ImagePanel* m_pTextLine;
    vgui2::ImagePanel* m_pWeaponImage;
    vgui2::ImagePanel* m_pItemBackground;
    vgui2::ImagePanel* m_pSelectedBackground;

    vgui2::Label* m_pText;
    vgui2::Label* m_pText2;

    vgui2::Button* m_pSelect;
};

class CWeaponListRightPanelEleDrag : public vgui2::ImagePanel
{
    DECLARE_CLASS_SIMPLE(CWeaponListRightPanelEleDrag, ImagePanel);

public:
    CWeaponListRightPanelEleDrag(vgui2::Panel* parent, const char* panelName) : ImagePanel(parent, panelName)
    {
        SetBounds(0, 0, 235, 81);
        SetMouseInputEnabled(false);
        SetKeyBoardInputEnabled(false);
    }

    int iSlot;
    int iWpnID;

protected:
    virtual void OnThink();
};