#pragma once

#include "GameUI/InventoryMenu_V2/WeaponEnchant/Class/CWeaponLevelGauge.h"
#include "GameUI/InventoryMenu_V2/WeaponEnchant/Class/CWeaponDescription.h"
#include "GameUI/InventoryMenu_V2/Shared/CSearchBar/CSearchBar.h"
#include "GameUI/InventoryMenu_V2/Shared/CTexturedButton/CTexturedButton.h"


class CWeaponListLeftPanel : public vgui2::Panel
{
    DECLARE_CLASS_SIMPLE(CWeaponListLeftPanel, Panel);

public:
    CWeaponListLeftPanel(vgui2::Panel* parent, const char* panelName);
    virtual void PerformLayout();

public:
    int GetOnProcessWeapon();
    void SetOnProcessWeapon(int iId, const char* szName);

    void ResetPanel(bool bEnable);
    Color UpdateEnchantColor(int iLevel);
    int GetOnProcessWeaponCurrentTotalLevel();

    void SetDescWeapon(const char* szWpnName, bool bUsing);
    CSearchBar* GetSearchBox() { return m_pSearchBar; };

protected:
    MESSAGE_FUNC_PTR(OnGaugeClicked, "GaugeClicked", gauge);
    MESSAGE_FUNC(OnSaveEnhancementData, "SaveEnhancementData");

    virtual void ApplySchemeSettings(vgui2::IScheme* pScheme);

private:
    vgui2::ImagePanel* m_pCraftBackground;
    vgui2::ImagePanel* m_pCraftImage;
    vgui2::Label* m_pCraftName;
    vgui2::Label* m_pCraftDesc;

    CSearchBar* m_pSearchBar;
    vgui2::Label* m_pSearchText;
    vgui2::Label* m_pSearchTip;

    vgui2::ImagePanel* m_pWeaponLabelBG;
    vgui2::Label* m_pWeaponLabel;
    vgui2::ImagePanel* m_pWeaponImage;
    vgui2::ImagePanel* m_pWeaponBackground;

    vgui2::Label* m_pRemainingChances;
    vgui2::Label* m_pWeaponNameLabel;

    vgui2::Label* m_pEnhanceCategories[6];
    vgui2::Label* m_pEnhanceCategoryLevels[6];
    CWeaponLevelGauge* m_pEnhanceLevelGauge[6][5];

    int m_iEnhanceLevelGauge[6];
    int m_iActiveEnchantWeapon_Internal;
    bool m_bHasUserMadeAnyChanges;

    vgui2::ImagePanel* m_pWeaponBackgroundLevel6;
    vgui2::ImagePanel* m_pWeaponBackgroundLevel8;

    vgui2::ImagePanel* m_pEnhancedTagLevel6;
    vgui2::ImagePanel* m_pEnhancedTagLevel8;

    vgui2::ImagePanel* m_pEnhanceLevelName;

    //Replaced with +6/+8 weapon image...
    vgui2::ImagePanel* m_pEnhancedImageLevel6;
    vgui2::ImagePanel* m_pEnhancedImageLevel8;

    //Some weapons do not have +6/+8,no images for them,using default.
    vgui2::ImagePanel* m_pLevel6UnreachableImage;
    vgui2::ImagePanel* m_pLevel8UnreachableImage;

    CWeaponDescription* m_pDescriptionPanel;

    vgui2::Button* m_pApplyButton;
};