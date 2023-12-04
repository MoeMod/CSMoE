#pragma once
#include <string>
#include <vector>
#include <unordered_map>


#include <vgui_controls/PHandle.h>
#include <vgui_controls/MenuItem.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/ScrollBar.h>
#include <vgui_controls/TextImage.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/PropertyDialog.h>
#include <vgui_controls/PropertyPage.h>
#include <vgui_controls/PropertySheet.h>
#include <vgui_controls/ScrollBar.h>
#include <vgui_controls/MessageBox.h>
#include <vgui_controls/KeyRepeat.h>

#include <tier1/KeyValues.h>
#include <FileSystem.h>

#include <vgui/IInput.h>
#include <vgui/IInputInternal.h>
#include <vgui/ISurface.h>

using namespace vgui2;

#define MAX_TAB_TYPE    3
enum InventoryTab_Type
{
    InventoryTab_Weapon = 0,
    InventoryTab_Character,
    InventoryTab_Zombie
};


class CInventoryMainMenu : public vgui2::PropertyDialog
{
    DECLARE_CLASS_SIMPLE(CInventoryMainMenu, PropertyDialog);

public:
    CInventoryMainMenu(vgui2::Panel* parent);

public:
    void Run();
    void Activate();
    void OnClose();

    void AddMenuTab(class CInventoryMainMenuTab* child, const char* name,int i);
    void AddSubPageToTab(int iSlot,std::pair<vgui2::Panel*, std::string> panelinfo);
    void SearchFuncsForTab(int index);

public:
    MESSAGE_FUNC_INT(OnMainTabSwitched, "MainTabSwitched", index);

private:
    
    class CWeaponEnchant* m_pWeaponEnchant;
    class CWeaponPaint* m_pWeaponPaint;

    class CCharacterManager* m_pCharacterManager;

    class CInventoryMainMenuTab* m_pMainMenuImageTab[MAX_TAB_TYPE];

    std::vector<std::pair<vgui2::Panel*, std::string>> m_vecFuncs[MAX_TAB_TYPE];

protected:
    virtual void PerformLayout();
};

class CInventoryMainMenuTab : public vgui2::ImagePanel
{
    DECLARE_CLASS_SIMPLE(CInventoryMainMenuTab, ImagePanel);

public:
    CInventoryMainMenuTab(vgui2::Panel* parent, const char* name);

public:
    virtual void OnMousePressed(vgui2::MouseCode code);

    vgui2::Label* GetTabLabel();

    void SetTabSlot(int i)
    {
        iActiveTab = i;
    };
    int GetActiveTabSlot()
    {
        return iActiveTab;
    };

    void SetTabIconInternal(const char* szIcon, const char* szIcon2);
    void SetTabIconExternal(bool bActive);

protected:
    //virtual void ApplySchemeSettings(vgui2::IScheme* pScheme);

private:
    std::pair<std::string, std::string> m_pIcon;

    vgui2::ImagePanel* m_pItemBackground;
    vgui2::ImagePanel* m_pItemBackgroundSelected;

    vgui2::Label* m_pText;

    int iActiveTab;
};
