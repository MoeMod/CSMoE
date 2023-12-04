#pragma once

#include "GameUI/InventoryMenu_V2/InventoryMainMenu.h"
#include "GameUI/MoEBulletIn.h"

class CWeaponDescription : public CCSBTEBulletinFrameImage
{
    DECLARE_CLASS_SIMPLE(CWeaponDescription, CCSBTEBulletinFrameImage);

public:
    CWeaponDescription(vgui2::Panel* parent, const char* panelName);
    virtual void SetWeapon(const char* weapon, bool inuse);

protected:
    virtual void PerformLayout();
    virtual void ApplySchemeSettings(vgui2::IScheme* pScheme) override;

private:
    const char* m_szRankImage[7] = {
        "resource/tooltip_d",
        "resource/tooltip_c",
        "resource/tooltip_b",
        "resource/tooltip_a",
        "resource/tooltip_s",
        "resource/tooltip_ss",
        "resource/tooltip_sss",
    };

    const char* m_szModeText[3] = {
        "#CSO_Tooltip_Desc_ModeType_Normal",
        "#CSO_Tooltip_Desc_ModeType_Zombie",
        "#CSO_Tooltip_Desc_ModeType_ZombieScen"
    };

private:
    void ReCalcBounds();

    vgui2::Label* m_pTitle;
    vgui2::ImagePanel* m_pTextLine;

    vgui2::ImagePanel* m_pGradeImage[3];
    vgui2::Label* m_pGradeText[3];
    vgui2::ImagePanel* m_pGradeLine[2];
    int m_iGrade;

    vgui2::RichText* m_pText;
    vgui2::ImagePanel* m_pDescImage;

    vgui2::ImagePanel* m_pFavOnImage;
};
