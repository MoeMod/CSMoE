#include "InventoryMainMenu.h"

#include "WeaponEnchant/CWeaponEnchant.h"
#include "WeaponPaint/CWeaponPaint.h"

#include "CharacterManager/CCharacterManager.h"

using namespace vgui2;

const char* szMainMenuTabName[3][2] =
{
	{"WeaponInventory","#CSO_ItemInventoryWeaponTab"},
	{"CharacterInventory","#CSO_ItemInventoryClassTab"},
	{"ZombieInventory","#CSO_CategoryModeZombieSkin"}
};

const char* szMainMenuTabIcon[3][2] =
{
	{"resource/taskbar/btn_weaponmaking@n","resource/taskbar/btn_weaponmaking_new@n"},
	{"resource/taskbar/btn_weaponmaking@n","resource/taskbar/btn_weaponmaking_new@n"},
	{"resource/taskbar/btn_weaponmaking@n","resource/taskbar/btn_weaponmaking_new@n"}
};

CInventoryMainMenu::CInventoryMainMenu(Panel* parent) : PropertyDialog(parent, "InventoryMenu")
{
	SetBounds(0, 0, 1440, 720);

	SetSizeable(false);
	SetTitle("#CSO_Inventory", true);

	//Set Offset For ImageButton Control?
	SetOffset(10, 68);
	SetSizeOffset(-4, -52);

	//Now we set the tab width
	GetPropertySheet()->SetTabWidth(64);

	//Init All Funcs Firstly!
	m_pWeaponEnchant = new CWeaponEnchant(this);
	m_pWeaponPaint = new CWeaponPaint(this);

	m_pCharacterManager = new CCharacterManager(this);

	//Add MainMenu Tabs(Main)
	for (int i = 0; i < 3; i++)
	{
		m_pMainMenuImageTab[i] = new CInventoryMainMenuTab(this, szMainMenuTabName[i][0]);
		AddMenuTab(m_pMainMenuImageTab[i], szMainMenuTabName[i][1],i);

		m_pMainMenuImageTab[i]->SetShouldScaleImage(false);
		m_pMainMenuImageTab[i]->SetTabIconInternal(szMainMenuTabIcon[i][0], szMainMenuTabIcon[i][1]);
	}

	//Tab 1 ->Weapon. Sub Funcs Added.
	AddSubPageToTab(InventoryTab_Weapon, std::make_pair(m_pWeaponEnchant, "#CSO_WeaponEnchant_Tab"));
	AddSubPageToTab(InventoryTab_Weapon, std::make_pair(m_pWeaponPaint, "#CSO_WeaponPaint_Tab"));

	//Tab 2 ->Character
	AddSubPageToTab(InventoryTab_Character, std::make_pair(m_pCharacterManager, "#CSO_ItemInventoryClassTab"));

	//Default? whit "Activate" to avoid display bug!
	SearchFuncsForTab(InventoryTab_Weapon);

	SetApplyButtonVisible(false);
	SetCancelButtonVisible(false);
	SetOKButtonVisible(true);
}
void CInventoryMainMenu::PerformLayout()
{
	BaseClass::PerformLayout();

	for (int i = 0; i < 3; i++)
	{
		if (!m_pMainMenuImageTab[i])
			continue;

		m_pMainMenuImageTab[i]->SetBounds(10 + i * 65, 40, 62, 50);
	}
}

void CInventoryMainMenu::Run()
{
	SetTitle("#CSO_Inventory", true);
	Activate();
}

void CInventoryMainMenu::OnClose()
{
	BaseClass::OnClose();
}
void CInventoryMainMenu::Activate()
{
	BaseClass::Activate();

	//Set it to the main window?
	if (m_pWeaponEnchant)
	{
		if (GetActivePage() != m_pWeaponEnchant)
		{
			if(m_pMainMenuImageTab[0])
				m_pMainMenuImageTab[0]->PostActionSignal(new KeyValues("MainTabSwitched", "index", 1));

			GetPropertySheet()->SetActivePage(m_pWeaponEnchant);
		}
	}

	ResetAllData();
	EnableApplyButton(false);
}

void CInventoryMainMenu::AddMenuTab(CInventoryMainMenuTab* child, const char* name, int i)
{
	if (child)
	{
		//we can link the tabs with the mainmenu
		child->AddActionSignalTarget(this);

		if (child->GetTabLabel() != nullptr)
		{
			child->GetTabLabel()->SetText(name);
		}

		child->SetTabSlot(i);
	}
}

void CInventoryMainMenu::AddSubPageToTab(int iSlot, std::pair<vgui2::Panel*, std::string> panelinfo)
{
	if (iSlot < 0 || iSlot >= MAX_TAB_TYPE || !panelinfo.first)
		return;

	m_vecFuncs[iSlot].emplace_back(panelinfo);
}

void CInventoryMainMenu::SearchFuncsForTab(int index)
{
	if (index < 0 || index >= MAX_TAB_TYPE)
		return;

	//Add funcs to sub page first
	for (std::vector<std::pair<vgui2::Panel*, std::string>>::const_iterator iter = m_vecFuncs[index].cbegin(); iter != m_vecFuncs[index].cend(); iter++)
	{
		auto pPanelInfo = *iter;

		auto pPanel = pPanelInfo.first;
		auto pPanelName = pPanelInfo.second;

		if (pPanel)
		{
			AddPage(pPanel, pPanelName.c_str());
		}
	}

	//Change tab state...
	for (int i = 0; i < MAX_TAB_TYPE; i++)
	{
		//Maybe using key values is better??
		const auto pPanel = m_pMainMenuImageTab[i];

		if (i == index)
		{
			pPanel->SetTabIconExternal(true);
			continue;
		}
			
		pPanel->SetTabIconExternal(false);
	}
}

void CInventoryMainMenu::OnMainTabSwitched(int index)
{
	if (GetPropertySheet()->GetNumPages())
	{
		GetPropertySheet()->RemoveAllPages();
	}

	SearchFuncsForTab(index);
	InvalidateLayout();
}

/* MyWpnEditor */
CInventoryMainMenuTab::CInventoryMainMenuTab(Panel* parent, const char* name) : ImagePanel(parent, name)
{
	SetShouldScaleImage(true);
	SetImage(scheme()->GetImage("resource/itembox", false));
	SetTouchInputEnabled(true);

	m_pText = new Label(this, "Label", "Default Label");
	m_pText->SetPaintBackgroundEnabled(false);
	m_pText->SetBounds(0, 0, 50, 25);
	m_pText->SetMouseInputEnabled(false);
	m_pText->SetVisible(false);

}

void CInventoryMainMenuTab::OnMousePressed(vgui2::MouseCode code)
{
	if (code != vgui2::MOUSE_LEFT)
		return;

	//Tell the main menu to switch.
	PostActionSignal(new KeyValues("MainTabSwitched", "index", iActiveTab));

	//Self Changes : Color,Selected image...
}

Label* CInventoryMainMenuTab::GetTabLabel()
{
	if (m_pText)
	{
		return m_pText;
	}
	else
	{
		return nullptr;
	}
}

void CInventoryMainMenuTab::SetTabIconInternal(const char* szIcon,const char* szIcon2)
{
	if (!szIcon || !szIcon2)
		return;

	std::string szIconPath; szIconPath.append(szIcon);
	std::string szIconPath2; szIconPath2.append(szIcon2);

	m_pIcon = make_pair(szIconPath, szIconPath2);
}

void CInventoryMainMenuTab::SetTabIconExternal(bool bActive)
{
	SetImage(scheme()->GetImage((bActive ? m_pIcon.second : m_pIcon.first).c_str(), false));
}