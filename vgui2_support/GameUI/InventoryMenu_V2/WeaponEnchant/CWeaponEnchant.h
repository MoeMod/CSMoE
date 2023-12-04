#pragma once
#include "GameUI/InventoryMenu_V2/InventoryMainMenu.h"

#include "Class/CWeaponListLeftPanel.h"
#include "Class/CWeaponListRightPanel.h"
#include "Class/CWeaponListRightPanelEle.h"


#define MAX_WEAPONS_INPAGE      8

enum EnchantList_Type
{
    EnchantList_Common = 0,
    EnchantList_Knife,
    EnchantList_Grenade
};

enum WeaponEnchantInfo_TypeName
{
    TYPE_Damage = 0,
    TYPE_Accuracy,
    TYPE_KickBack,
    TYPE_Weight,
    TYPE_FireRate,
    TYPE_Ammo,
    TYPE_NULL
};
enum WeaponEnchantSort_TypeName
{
    TYPE_Whole = 0,
    TYPE_Pistol,
    TYPE_ShotGun,
    TYPE_SubMachineGun,
    TYPE_Rifle,
    TYPE_MachineGun,
    TYPE_Knife,
    TYPE_Equipment,
    TYPE_NO
};

struct WeaponEnchantInfo
{
    //We should know its model name?
    char szName[64];

    int iId;
    int iEnchantInfo[6];
    int iCustomInfo[6];

    int iMaxEnhanceLevel;
    int iMaxCustomLevel;
    bool bOnProcess;

    WeaponEnchantSort_TypeName iType;

    bool operator==(const WeaponEnchantInfo& p) const {
        return (iId == p.iId);
    }
};

struct WeaponPaintInfo
{

};

//The previous ones are actually the section names in .csv
using WeaponEnchantPropertyType = std::pair<const std::string, enum WeaponEnchantInfo_TypeName>;
static const WeaponEnchantPropertyType WeaponEnchantProperty[] = {
    { "Damage", WeaponEnchantInfo_TypeName::TYPE_Damage },
    { "Accuracy", WeaponEnchantInfo_TypeName::TYPE_Accuracy },
    { "Rebound", WeaponEnchantInfo_TypeName::TYPE_KickBack },
    { "Weight", WeaponEnchantInfo_TypeName::TYPE_Weight },
    { "Repeatedly", WeaponEnchantInfo_TypeName::TYPE_FireRate },
    { "Load", WeaponEnchantInfo_TypeName::TYPE_Ammo },
    { "NULL", WeaponEnchantInfo_TypeName::TYPE_NULL }
};

using WeaponEnchantSortType = std::pair<const std::string, enum WeaponEnchantSort_TypeName>;
static const WeaponEnchantSortType WeaponEnchantSort[] = {
    { "Whole",WeaponEnchantSort_TypeName::TYPE_Whole},
    { "Pistol",WeaponEnchantSort_TypeName::TYPE_Pistol},
    { "ShotGun",WeaponEnchantSort_TypeName::TYPE_ShotGun},
    { "SubMachineGun",WeaponEnchantSort_TypeName::TYPE_SubMachineGun},
    { "Rifle",WeaponEnchantSort_TypeName::TYPE_Rifle},
    { "MachineGun",WeaponEnchantSort_TypeName::TYPE_MachineGun},
    { "Knife",WeaponEnchantSort_TypeName::TYPE_Knife},
    { "Equipment",WeaponEnchantSort_TypeName::TYPE_Equipment},
    { "NO",WeaponEnchantSort_TypeName::TYPE_NO}
};


class CWeaponEnchant : public vgui2::PropertyPage
{
    DECLARE_CLASS_SIMPLE(CWeaponEnchant, PropertyPage);

public:
    CWeaponEnchant(vgui2::Panel* parent);

public:
    virtual void PerformLayout();
    virtual void OnMouseWheeled(int delta);

    // WpnList Changed?
    MESSAGE_FUNC(OnScrollBarSliderMoved, "ScrollBarSliderMoved");
    MESSAGE_FUNC_PTR(OnPageTabActivated, "PageTabActivated", panel);

    // Left Panel Changed?
    MESSAGE_FUNC_INT(OnElementSelected, "ElementSelected", index);
    MESSAGE_FUNC(OnElementReleased, "ElementReleased");

    MESSAGE_FUNC(OnTextChanged, "TextChanged");

    WeaponEnchantInfo& FindWeaponInfoByID(int id);
    CWeaponListRightPanelEle* FindPanelBySlot(int iSlot);

    int GetOnProcessWeapon();
    void SaveEnhancementData(WeaponEnchantInfo& pInfo);
    void FetchEnhancementData();

    unsigned int PackEnhancementData(const int* pData);

    CWeaponListLeftPanel* GetLeftPanel()
    {
        return m_pLeftSubPanel;
    };
    int GetScrollBarLine()
    {
        return m_iLine;
    };

public:
    void LoadWeaponList();

    void SwitchToLine(int iLine);
    void RefreshList();
    char* UpdateWeapon(int iTotalEnhancedLevel, const char* szOrigin);

protected:
    void OnCommand(const char* command);

private:
    int m_iLine;

    vgui2::ScrollBar* m_pScrollBar;
    vgui2::PropertySheet* m_pPropertySheet;

    CWeaponListRightPanel* m_pPropertyPage[8];
    CWeaponListRightPanelEle* m_pElements[MAX_WEAPONS_INPAGE];
    CWeaponListRightPanelEleDrag* m_pWpnDragImage;

    CWeaponListLeftPanel* m_pLeftSubPanel;

    //MoE ID & Info Struct
    std::unordered_map<int, WeaponEnchantInfo> m_vecWeaponEnchantList;
    //Use it to locate weapon in map?
    std::vector<int> m_vecWeaponDisplayId;

    int iActiveEnchantWeapon;
};