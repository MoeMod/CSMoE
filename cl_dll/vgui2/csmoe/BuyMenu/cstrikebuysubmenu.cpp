#include "hud.h"
#include "CBaseViewport.h"
#include "cdll_dll.h"
#include "cl_util.h"
#include "cstrikebuymenu.h"
#include "cstrikebuysubmenu.h"
#include "shared_util.h"
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include "vgui_controls/RichText.h"
#include "vgui_controls/CheckButton.h"
#include "vgui_controls/TextEntry.h"
#include "vgui_controls/MessageBox.h"
#include "buymouseoverpanelbutton.h"
#include "cso_controls/ButtonGlow.h"
#include "cso_controls/DarkTextEntry.h"

#include <nlohmann/json.hpp>

#include <string>

using namespace vgui2;
using namespace cl;

const Color COL_NONE = { 255, 255, 255, 255 };
const Color COL_CT = { 192, 205, 224, 255 };
const Color COL_TR = { 216, 182, 183, 255 };

static const char *EQUIPMENT_BUYLIST[] = { "vest","vesthelm","flash","hegrenade","sgren","defuser","nvgs" };
static const char *EQUIPMENT_BUYLIST_TEXT[] = { "#Cstrike_Kevlar","#Cstrike_Kevlar_Helmet","#Cstrike_Flashbang","#Cstrike_HE_Grenade","#Cstrike_Smoke_Grenade","#Cstrike_Defuser","#Cstrike_NightVision_Button_CT" };
static const char *EQUIPMENT_BUYLIST_CMD[] = { "vest","vesthelm","flash","VGUI_BuyMenu_BuyWeapon weapon_hegrenade","sgren","defuser","nvgs" };

CCSBuySubMenu::CCSBuySubMenu(vgui2::Panel *parent, const char *name) : CBuySubMenu(parent, name)
{
	m_pTitleLabel = new vgui2::Label(this, "CaptionLabel", "#CSO_WeaponSelectMenu");

	char buffer[64];

	m_pShowCTWeapon = new NewTabButton(this, "ShowCTWeapon", "#CSO_BuyShowCT");
	m_pShowTERWeapon = new NewTabButton(this, "ShowTERWeapon", "#CSO_BuyShowTER");
	m_pShowCTWeapon->SetTextColor(COL_CT);
	m_pShowTERWeapon->SetTextColor(COL_TR);

	for (int i = 0; i < 10; i++)
	{
		sprintf(buffer, "slot%d", i);
		m_pSlotButtons[i] = new CSBuyMouseOverPanelButton(this, buffer, m_pPanel);
		m_pSlotButtons[i]->GetClassPanel()->SetName("ItemInfo");
	}
	m_pPrevBtn = new vgui2::Button(this, "prevBtn", "#CSO_PrevBuy");
	m_pNextBtn = new vgui2::Button(this, "nextBtn", "#CSO_NextBuy");

	m_pRebuyButton = new ButtonGlow(this, "RebuyButton", "#Cstrike_BuyMenuRebuy");
	m_pAutobuyButton = new ButtonGlow(this, "AutobuyButton", "#Cstrike_BuyMenuAutobuy");

	m_pBasketClear = new vgui2::Button(this, "BasketClear", "#CSO_BasketClear");
	m_pBasketBuy = new ButtonGlow(this, "BasketBuy", "#CSO_BasketBuy");
	m_pQuitButton = new vgui2::Button(this, "QuitButton", "#Cstrike_Cancel");

	primaryBG = new vgui2::ImagePanel(this, "primaryBG");
	secondaryBG = new vgui2::ImagePanel(this, "secondaryBG");
	knifeBG = new vgui2::ImagePanel(this, "knifeBG");
	grenadeBG = new vgui2::ImagePanel(this, "grenadeBG");
	equipBG = new vgui2::ImagePanel(this, "equipBG");

	pwpnBG = new WeaponImagePanel(this, "pwpnBG");
	pammoBG = new vgui2::ImagePanel(this, "pammoBG");
	swpnBG = new WeaponImagePanel(this, "swpnBG");
	sammoBG = new vgui2::ImagePanel(this, "sammoBG");
	hgrenBG = new WeaponImagePanel(this, "hgrenBG");
	sgrenBG = new vgui2::ImagePanel(this, "sgrenBG");
	fgrenBG = new vgui2::ImagePanel(this, "fgrenBG");
	fgren2BG = new vgui2::ImagePanel(this, "fgren2BG");
	dfBG = new vgui2::ImagePanel(this, "dfBG");
	nvBG = new vgui2::ImagePanel(this, "nvBG");
	kevBG = new vgui2::ImagePanel(this, "kevBG");
	newknifeBG = new WeaponImagePanel(this, "newknifeBG");

	for (int i = 0; i < 5; i++)
	{
		sprintf(buffer, "fav%d", i);
		m_pFavButtons[i] = new BuyPresetButton(this, buffer);
		sprintf(buffer, "fav_save%d", i);
		m_pFavSaveButtons[i] = new vgui2::Button(this, buffer, "#CSO_FavSave");
	}
	m_pFavDirectBuy = new vgui2::CheckButton(this, "fav_direct_buy_ckbtn", "#CSO_FavDirectBuy");

	account_num = new vgui2::Label(this, "account_num", "");
	buytime_num = new vgui2::Label(this, "buytime_num", "");
	moneyText = new vgui2::Label(this, "moneyText", "");

	account = new DarkTextEntry(this, "account");
	buytime = new DarkTextEntry(this, "buytime");
	moneyBack = new DarkTextEntry(this, "moneyBack");
	freezetime = new DarkTextEntry(this, "freezetime");

	m_pUpgradeTitle = new vgui2::Label(this, "UpgradeTitle", "");
	m_pOppZombiUpgradeTitle = new vgui2::Label(this, "OppZombiUpgradeTitle", "");

	m_pSetSelBg = new vgui2::ImagePanel(this, "SetSelBg");
	m_pSetLabel = new vgui2::Label(this, "SetLabel", "X Set");
	m_pPrevSetBtn = new vgui2::Button(this, "PrevSetBtn", "");
	m_pNextSetBtn = new vgui2::Button(this, "NextSetBtn", "");

	m_pEditDescLabel_DM = new vgui2::Label(this, "EditDescLabel_DM", "#CSO_BuySubMenu_EditDesc");
	m_pEditDescBg = new vgui2::ImagePanel(this, "EditDescBg");
	m_pEquipSample = new vgui2::ImagePanel(this, "EquipSample");

	//m_iniFavorite.OpenFile("quickbuy.ini");
	ReadFavoriteSets();

	SetupItems(BUY_NONE);
}

void CCSBuySubMenu::OnCommand(const char *command)
{
	if (!strncmp(command, "VGUI_BuyMenu_Show", 17))
	{
        MoEWeaponBuyType iType;
		if (command[17] == '\0')
			iType = BUY_NONE;
		else
			sscanf(command + 17, "%d", &iType); //"Pistol 1"

		SetupItems(iType);
		
		GotoNextSubPanel();
		return;
	}
	else if (!strncmp(command, "VGUI_BuyMenu_SetTeam", 20))
	{
		TeamName team;
		if (command[20] == '\0')
			team = UNASSIGNED;
		else
			sscanf(command + 20, "%d", &team); //"Pistol 1"

		SetTeam(team);

		GotoNextSubPanel();
		return;
	}
	else if (!strncmp(command, "fav", 3) && command[4] == '\0')
	{
		int n = command[3] - '0';
		OnSelectFavoriteWeapons(n);
		return;
	}
	else if (!strncmp(command, "favSav", 6) && command[7] == '\0')
	{
		int n = command[6] - '0';
		OnSaveFavoriteWeapons(n);
		return;
	}
	else if (!strncmp(command, "VGUI_BuyMenu_BuyWeapon ", 23))
	{
		OnSelectWeapon(command + 23);
		return;
	}
	else if (!Q_strcmp(command, "showctwpn"))
	{
		SetTeam(CT);
		SetupItems(BUY_NONE);
		return;
	}
	else if (!Q_strcmp(command, "showterwpn"))
	{
		SetTeam(TERRORIST);
		SetupItems(BUY_NONE);
		return;
	}
	else if (!Q_strcmp(command, "buy_unavailable"))
	{
		BaseClass::OnCommand("vguicancel");
		return;
	}
	else if (!Q_strcmp(command, "autobuy_in"))
	{
		if (m_iTeam == CT)
			gEngfuncs.pfnClientCmd("moe_buy weapon_m4a1;moe_buy weapon_usp;moe_buy weapon_hegrenade;moe_buy weapon_knife;secammo;primammo;vesthelm;defuser");
		else
            gEngfuncs.pfnClientCmd("moe_buy weapon_ak47;moe_buy weapon_glock18;moe_buy weapon_hegrenade;moe_buy weapon_knife;secammo;primammo;vesthelm");
		BaseClass::OnCommand("vguicancel");
		return;
	}
	else if (!Q_strcmp(command, "rebuy_in"))
	{
		gHUD.m_Ammo.UserCmd_Rebuy();
		BaseClass::OnCommand("vguicancel");
		return;
	}
	else if (!Q_strcmp(command, "basketbuy"))
	{
		OnBuySelectedItems();
		BaseClass::OnCommand("vguicancel");
		return;
	}
	else if (!Q_strcmp(command, "prevpage"))
	{
		SetupPage(m_iCurrentPage - 1);
		return;
	}
	else if (!Q_strcmp(command, "nextpage"))
	{
		SetupPage(m_iCurrentPage + 1);
		return;
	}
	else if (!Q_strcmp(command, "basketclear"))
	{
		OnClearSelectedItems();
		return;
	}

	BaseClass::OnCommand(command);
}

void CCSBuySubMenu::PerformLayout()
{
	// fix these not in .res
	int  w, h;
	GetSize(w, h);
	int w2, h2;
	m_pTitleLabel->GetSize(w2, h2);
	m_pTitleLabel->SetPos(w / 2 - w2 / 2, vgui2::scheme()->GetProportionalScaledValue(12));
	
	m_pShowTERWeapon->SetBounds(
		vgui2::scheme()->GetProportionalScaledValue(12), 
		vgui2::scheme()->GetProportionalScaledValue(32), 
		vgui2::scheme()->GetProportionalScaledValue(98), 
		vgui2::scheme()->GetProportionalScaledValue(16)
	);
	m_pShowCTWeapon->SetBounds(
		vgui2::scheme()->GetProportionalScaledValue(108), 
		vgui2::scheme()->GetProportionalScaledValue(32), 
		vgui2::scheme()->GetProportionalScaledValue(98), 
		vgui2::scheme()->GetProportionalScaledValue(16)
	);
	newknifeBG->SetBounds(
		vgui2::scheme()->GetProportionalScaledValue(505), 
		vgui2::scheme()->GetProportionalScaledValue(327), 
		vgui2::scheme()->GetProportionalScaledValue(75), 
		vgui2::scheme()->GetProportionalScaledValue(50)
	);

	for (int i = 0; i < 10; ++i)
	{
		m_pSlotButtons[i]->GetClassPanel()->SetBounds(
			vgui2::scheme()->GetProportionalScaledValue(216),
			vgui2::scheme()->GetProportionalScaledValue(60),
			vgui2::scheme()->GetProportionalScaledValue(152),
			vgui2::scheme()->GetProportionalScaledValue(145)
		); 
	}

	BaseClass::PerformLayout();
}

void CCSBuySubMenu::OnThink()
{
	BaseClass::OnThink();

	account_num->SetText(std::to_wstring(gHUD.m_Money.m_iMoneyCount).c_str());
	buytime_num->SetText(std::to_wstring(0).c_str());
}

void CCSBuySubMenu::SetupItems(MoEWeaponBuyType type)
{
	m_BuyItemList.clear();
	if (type == BUY_NONE)
	{
		static const char szTitles[10][32] = { "#Cstrike_Pistols", "#Cstrike_Shotguns", "#Cstrike_SubMachineGuns", "#Cstrike_Rifles",
			"#Cstrike_MachineGuns", "#Cstrike_Prim_Ammo", "#Cstrike_Sec_Ammo", "#Cstrike_Equipment",
			"&9 近身武器", "#Cstrike_Cancel" };
		static const char szCommands[10][32] = { "VGUI_BuyMenu_Show 0", "VGUI_BuyMenu_Show 1", "VGUI_BuyMenu_Show 2", "VGUI_BuyMenu_Show 3",
			"VGUI_BuyMenu_Show 4", "primammo", "secammo", "VGUI_BuyMenu_Show 5",
			"VGUI_BuyMenu_Show 6", "vguicancel" };

		for (int i = 0; i < 10; ++i)
		{
			m_pSlotButtons[i]->SetBanWeapon("");
			m_pSlotButtons[i]->SetEnabled(true);
			m_pSlotButtons[i]->SetText(szTitles[i]);
			m_pSlotButtons[i]->SetCommand(szCommands[i]);
			m_pSlotButtons[i]->SetHotkey('0' + i + 1);
			m_pSlotButtons[i]->SetVisible(true);
			m_pSlotButtons[i]->UpdateWeapon("");
		}
		m_pSlotButtons[9]->SetBanWeapon("");
		m_pSlotButtons[9]->SetHotkey('0');
		m_pPrevBtn->SetVisible(false);
		m_pNextBtn->SetVisible(false);
		return;
	}
	else
	{
		if (type == BUY_EQUIP)
		{
			for (int i = 0; i < 7; ++i)
			{
				m_BuyItemList.push_back({ EQUIPMENT_BUYLIST[i], EQUIPMENT_BUYLIST_TEXT[i], EQUIPMENT_BUYLIST_CMD[i] });
			}
		}

		for (auto &x : g_MoEWeaponBuyInfo)
		{
			if (type != x.iMenu)
				continue;
			if (x.iTeam != UNASSIGNED && m_iTeam != x.iTeam)
				continue;
			if (type == BUY_EQUIP && x.iSlot == GRENADE_SLOT) {
				if (gHUD.m_setBanGrenade.find(x.iID) != gHUD.m_setBanGrenade.end())
					continue;
			}
			else if (type == BUY_KNIFE) {
				if(gHUD.m_setBanKnife.find(x.iID) != gHUD.m_setBanKnife.end())
					continue;
			}
			else {
				if (gHUD.m_setBanWeapon.find(x.iID) != gHUD.m_setBanWeapon.end())
					continue;
			}

			const char *name = x.pszClassName;

			if (type == BUY_EQUIP)
			{
				if (!strcmp(name, "weapon_flashbang") || !strcmp(name, "weapon_hegrenade") || !strcmp(name, "weapon_smokegrenade"))
					continue;
			}

			m_BuyItemList.push_back(ItemInfo{ name, x.pszDisplayName, std::string("VGUI_BuyMenu_BuyWeapon ") + name ,x.iLevel });
		}

		m_pPrevBtn->SetHotkey(L'-');
		m_pNextBtn->SetHotkey(L'=');
		SetupPage(0);
	}
}

void CCSBuySubMenu::SetupPage(size_t iPage)
{
	int totalpages = (m_BuyItemList.size() - 1) / 9 + 1;
	if (iPage <= 0) iPage = 0;
	else if (iPage >= totalpages)
		iPage = totalpages - 1;

	m_iCurrentPage = iPage;
	
	// page buttons
	m_pPrevBtn->SetVisible(iPage != 0);
	m_pNextBtn->SetVisible(m_iCurrentPage != totalpages - 1);
	
	for (int i = 0; i < 9; ++i)
	{
		int iElement = m_iCurrentPage * 9 + i;
		if (iElement >= m_BuyItemList.size())
		{
			m_pSlotButtons[i]->SetBanWeapon("");
			m_pSlotButtons[i]->SetText("");
			m_pSlotButtons[i]->SetCommand("");
			m_pSlotButtons[i]->SetVisible(false);
			m_pSlotButtons[i]->SetHotkey('\0');
		}
		else
		{
			m_pSlotButtons[i]->SetBanWeapon("");
			m_pSlotButtons[i]->SetEnabled(true);
			

			const char *weapon = m_BuyItemList[iElement].name.c_str();
			const char *showname = m_BuyItemList[iElement].showname.c_str();
			const char *cmd = m_BuyItemList[iElement].command.c_str();
			m_pSlotButtons[i]->SetText(showname);
			m_pSlotButtons[i]->SetCommand(cmd);
			m_pSlotButtons[i]->SetVisible(true);
			m_pSlotButtons[i]->SetHotkey('0' + i + 1);
			m_pSlotButtons[i]->UpdateWeapon(weapon);

			//如果武器等级＞玩家等级，该武器上锁
			if ((cl::gHUD.IsZombieMod() && (cl::gHUD.m_iModRunning == MOD_ZBZ)))
			{
				if (m_BuyItemList[iElement].level > cl::gHUD.m_iZlevel)
				{
					m_pSlotButtons[i]->SetEnabled(false);
					m_pSlotButtons[i]->SetBanWeapon(weapon, m_BuyItemList[iElement].level);
				}
			}
		}
	}
	
	m_pSlotButtons[9]->SetBanWeapon("");
	m_pSlotButtons[9]->SetEnabled(true);
	m_pSlotButtons[9]->SetText("#CSO_PrevWpnBuy");
	m_pSlotButtons[9]->SetCommand("VGUI_BuyMenu_Show");
	m_pSlotButtons[9]->SetHotkey('0');
	


}

void CCSBuySubMenu::SetTeam(TeamName team)
{
	m_iTeam = team;

	for (int i = 0; i < 10; ++i)
	{
		m_pSlotButtons[i]->SetTeam(team);
	}
	
	m_pShowCTWeapon->SetActive(false);
	m_pShowTERWeapon->SetActive(false);
	if (team== CT)
		m_pShowCTWeapon->SetActive(true);
	if (team == TERRORIST)
		m_pShowTERWeapon->SetActive(true);

	Color col = { 255,255,255,255 };
	Color bg = { 255,255,255,255 };
	if (m_iTeam == TERRORIST)
	{
		col = COL_TR;
	}
	else if (m_iTeam == CT)
	{
		col = COL_CT;
	}

}

void CCSBuySubMenu::OnSelectWeapon(const char *weapon)
{
    auto iter = std::find_if(std::begin(g_MoEWeaponBuyInfo), std::end(g_MoEWeaponBuyInfo), [weapon](const MoEWeaponBuyInfo_s &info){ return !stricmp(info.pszClassName, weapon); });
	if(iter == std::end(g_MoEWeaponBuyInfo))
        return;
    
    switch (iter->iSlot)
	{
	case 1: m_SelectedItems.Primary = weapon; break;
	case 2: m_SelectedItems.Secondary = weapon; break;
	case 3: m_SelectedItems.Melee = weapon; break;
	case 4: m_SelectedItems.HEGrenade = weapon; break;
	}
	SetupItems(BUY_NONE);
	UpdateFavoriteSetsControls();
	SaveFavoriteSets();
}

void CCSBuySubMenu::ReadFavoriteSets()
{
    nlohmann::json j;
    FileHandle_t fh = vgui2::filesystem()->Open("moe_buy.json", "rb", "CONFIG");
    if (fh != FILESYSTEM_INVALID_HANDLE)
    {
        int len = vgui2::filesystem()->Size(fh);
        if (len > 0)
        {
            char *buf = (char *)_alloca(len + 1);
            vgui2::filesystem()->Read(buf, len, fh);
            buf[len] = 0;
            j.parse(buf, buf + len);
        }

        vgui2::filesystem()->Close(fh);
    }

    try {
        m_SelectedItems = {
                j["Primary"] ,
                j["Secondary"] ,
                j["Knife"],
                j["Grenade"],
                0,0,0,0, ArmorType::ARMOR_HELMET
        };

        for(int i = 1; i <= 5; ++i)
        {
            std::string app = "QuickBuy";
            app += std::to_string(i);

            auto &keyvalue = j[app];

            m_FavoriteItems[i - 1] = {
                    keyvalue["Primary"] ,
                    keyvalue["Secondary"] ,
                    keyvalue["Knife"],
                    keyvalue["Grenade"],
                    0,0,0,0, ArmorType::ARMOR_HELMET
            };
        }
    } catch(const std::exception &e)
    {
        // ignore json error
        gEngfuncs.Con_DPrintf("moe_buy error: %s", e.what());
    }
	UpdateFavoriteSetsControls(); // update images
}

void CCSBuySubMenu::SaveFavoriteSets()
{
    std::string content;
    try {
        nlohmann::json j;
        for (int i = 1; i <= 5; ++i)
        {
            std::string app = "QuickBuy";
            app += std::to_string(i);

            auto &keyvalue = j[app];
            keyvalue["Primary"] = m_FavoriteItems[i].Primary;
            keyvalue["Secondary"] = m_FavoriteItems[i].Secondary;
            keyvalue["Knife"] = m_FavoriteItems[i].Melee;
            keyvalue["Grenade"] = m_FavoriteItems[i].HEGrenade;
        }

        auto &keyvalue = j[std::string("QuickBuy0")];
        keyvalue["Primary"] = m_SelectedItems.Primary;
        keyvalue["Secondary"] = m_SelectedItems.Secondary;
        keyvalue["Knife"] = m_SelectedItems.Melee;
        keyvalue["Grenade"] = m_SelectedItems.HEGrenade;

        content = j.dump();
    } catch(const std::exception &e)
    {
        // ignore json error
        gEngfuncs.Con_DPrintf("moe_buy error: %s", e.what());
    }

    FileHandle_t fh = vgui2::filesystem()->Open("moe_buy.json", "wb", "CONFIG");
    if (fh != FILESYSTEM_INVALID_HANDLE)
    {
        vgui2::filesystem()->Write(content.c_str(), content.length(), fh);
        vgui2::filesystem()->Close(fh);
    }
	UpdateFavoriteSetsControls(); // update images
}

void CCSBuySubMenu::UpdateFavoriteSetsControls()
{
	for (int i = 0; i < 5; ++i)
	{
		m_pFavButtons[i]->SetPrimaryWeapon(m_FavoriteItems[i].Primary.c_str());
		m_pFavButtons[i]->SetSecondaryWeapon(m_FavoriteItems[i].Secondary.c_str());
		m_pFavButtons[i]->SetKnifeWeapon(m_FavoriteItems[i].Melee.c_str());
	}

	pwpnBG->SetWeapon(m_SelectedItems.Primary.c_str());
	swpnBG->SetWeapon(m_SelectedItems.Secondary.c_str());
	hgrenBG->SetWeapon(m_SelectedItems.HEGrenade.c_str());
	newknifeBG->SetWeapon(m_SelectedItems.Melee.c_str());

	

	fgrenBG->SetImage(m_SelectedItems.nFlashBang ? "gfx/vgui/basket/flash" : "");
	sgrenBG->SetImage(m_SelectedItems.nSmokeGrenade ? "gfx/vgui/basket/sgren" : "");
	dfBG->SetImage(m_SelectedItems.bDefuser ? "gfx/vgui/basket/defuser" : "");
	nvBG->SetImage(m_SelectedItems.bDefuser ? "gfx/vgui/basket/nvgs" : "");

	switch (m_SelectedItems.iKelmet)
	{
	case ArmorType::NONE: kevBG->SetImage(""); break;
	case ArmorType::ARMOR: kevBG->SetImage("gfx/vgui/basket/vest"); break;
	case ArmorType::ARMOR_HELMET: kevBG->SetImage("gfx/vgui/basket/vesthelm"); break;
	}
}

void CCSBuySubMenu::OnSelectFavoriteWeapons(int i)
{
	Assert(i >= 0 && i < 5);
	m_SelectedItems = m_FavoriteItems[i];
	if (m_pFavDirectBuy->IsSelected())
		OnBuySelectedItems();
	SaveFavoriteSets();
}

void CCSBuySubMenu::OnSaveFavoriteWeapons(int i)
{
	Assert(i >= 0 && i < 5);
	m_FavoriteItems[i] = m_SelectedItems;
	SaveFavoriteSets();
}

void CCSBuySubMenu::OnClearSelectedItems()
{
	m_SelectedItems = {
		"" ,
		"" ,
		"" ,
		"" ,
		0,0,0,0, ArmorType::NONE
	};
	UpdateFavoriteSetsControls();
}

void CCSBuySubMenu::OnBuySelectedItems()
{
	std::string szCommand;
	for(const std::string &wpn : { m_SelectedItems.Primary,m_SelectedItems.Secondary,m_SelectedItems.Melee,m_SelectedItems.HEGrenade })
		if (wpn.size())
		{
			szCommand += "moe_buy ";
			szCommand += wpn;
			szCommand += ';';
		}
	szCommand += "secammo;primammo;vesthelm";
	if (cl::gHUD.m_iModRunning == MOD_NONE && cl::gHUD.m_FollowIcon.m_iBombTargetsNum > 0) {
		szCommand += ";defuser";
	}
	gEngfuncs.pfnClientCmd(const_cast<char *>(szCommand.c_str()));

	OnCommand("vguicancel");
}

CSBuyMouseOverPanelButton *CCSBuySubMenu::CreateNewMouseOverPanelButton(EditablePanel *panel)
{
	return new CSBuyMouseOverPanelButton(this, NULL, panel);
}

CCSBuySubMenu *CCSBuySubMenu::CreateNewSubMenu(const char *name)
{
	return new CCSBuySubMenu(this, name);
}

void CCSBuySubMenu::LoadControlSettings(const char *dialogResourceName, const char *pathID, KeyValues *pPreloadedKeyValues)
{
	BaseClass::LoadControlSettings(dialogResourceName, pathID, pPreloadedKeyValues);

	// 0->exit
	m_pSlotButtons[9]->SetText("#Cstrike_Cancel");
	m_pSlotButtons[9]->SetCommand("vguicancel");

	account->SetText("#Cstrike_Current_Money");
	buytime->SetText("#CSO_BuyTime");
	for (vgui2::TextEntry *p : { account , buytime })
	{
		p->SetMouseInputEnabled(false);
	}

	m_pTitleLabel->SetFont(scheme()->GetIScheme(m_pTitleLabel->GetScheme())->GetFont("Title", IsProportional()));
	m_pTitleLabel->SizeToContents();

	const wchar key[5] = { L'S',L'D', L'F', L'G', L'H' };
	for (int i = 0; i < 5; ++i)
	{
		m_pFavButtons[i]->SetHotkey(key[i]);
	}

	float scale = vgui2::scheme()->GetProportionalScaledValue(4096) / 4096.0f;
	for (vgui2::ImagePanel * pPanel : { pwpnBG, swpnBG, hgrenBG, newknifeBG })
	{
		pPanel->SetShouldScaleImage(true);
		pPanel->SetShouldCenterImage(true);
	}
	hgrenBG->SetScaleAmount(0.54f * scale);
	swpnBG->SetScaleAmount(0.375f * scale);
	newknifeBG->SetScaleAmount(0.375f * scale);
	for (vgui2::ImagePanel * pPanel : { sgrenBG, fgrenBG, fgren2BG, dfBG, nvBG, kevBG })
	{
		pPanel->SetShouldScaleImage(true);
		pPanel->SetShouldCenterImage(true);
		pPanel->SetScaleAmount(0.54f * scale);
	}
	

	m_pShowCTWeapon->SetCommand("showctwpn");
	m_pShowTERWeapon->SetCommand("showterwpn");

	UpdateFavoriteSetsControls();
}

void CCSBuySubMenu_DefaultMode::LoadControlSettings(const char *dialogResourceName, const char *pathID, KeyValues *pPreloadedKeyValues)
{
	BaseClass::LoadControlSettings(dialogResourceName, pathID, pPreloadedKeyValues);
	BaseClass::LoadControlSettings("Resource/UI/cso_buysubmenu_ver2.res", "GAME");

	// hide zbs
	m_pUpgradeTitle->SetVisible(false);
	m_pOppZombiUpgradeTitle->SetVisible(false);

	// hide dm set
	m_pSetSelBg->SetVisible(false);
	m_pSetLabel->SetVisible(false);
	m_pPrevSetBtn->SetVisible(false);
	m_pNextSetBtn->SetVisible(false);
	m_pEditDescLabel_DM->SetVisible(false);
	m_pEditDescBg->SetVisible(false);
	m_pEquipSample->SetVisible(false);

	// Lower Right Corner
	m_pBasketClear->SetVisible(false);
	m_pBasketBuy->SetVisible(false);
	m_pQuitButton->SetVisible(false);

	m_pFavDirectBuy->SetVisible(false);
	m_pFavDirectBuy->SetSelected(true);

	for (vgui2::Button * pPanel : m_pFavSaveButtons)
	{
		pPanel->SetVisible(false);
	}

	// hide set
	for (vgui2::ImagePanel * pPanel : { primaryBG, secondaryBG, knifeBG, grenadeBG, equipBG })
	{
		pPanel->SetVisible(false);
	}

	for (vgui2::ImagePanel * pPanel : { pwpnBG, swpnBG, hgrenBG, newknifeBG })
	{
		pPanel->SetVisible(false);
	}
	for (vgui2::ImagePanel * pPanel : { sgrenBG, fgrenBG, fgren2BG, dfBG, nvBG, kevBG })
	{
		pPanel->SetVisible(false);
	}

	m_pShowCTWeapon->SetVisible(false);
	m_pShowTERWeapon->SetVisible(false);

	moneyText->SetVisible(false);
	moneyBack->SetVisible(false);
	freezetime->SetVisible(false);
}

void CCSBuySubMenu_DefaultMode::PerformLayout()
{
	BaseClass::PerformLayout();
	for (int i = 0; i < 10; ++i)
	{
		m_pSlotButtons[i]->SetPos(
			vgui2::scheme()->GetProportionalScaledValue(16),
			vgui2::scheme()->GetProportionalScaledValue(50 + i * 25)
		);
	}
}

void CCSBuySubMenu_ZombieMod::LoadControlSettings(const char *dialogResourceName, const char *pathID, KeyValues *pPreloadedKeyValues)
{
	BaseClass::LoadControlSettings(dialogResourceName, pathID, pPreloadedKeyValues);
	BaseClass::LoadControlSettings("Resource/UI/cso_buysubmenu_ver5.res", "GAME");

	// hide zbs
	m_pUpgradeTitle->SetVisible(false);
	m_pOppZombiUpgradeTitle->SetVisible(false);

	// hide dm set
	m_pSetSelBg->SetVisible(false);
	m_pSetLabel->SetVisible(false);
	m_pPrevSetBtn->SetVisible(false);
	m_pNextSetBtn->SetVisible(false);
	m_pEditDescLabel_DM->SetVisible(false);
	m_pEditDescBg->SetVisible(false);
	m_pEquipSample->SetVisible(false);

	// hide money
	for(auto pPanel : { account_num ,moneyText })
		pPanel->SetVisible(false);
	for (auto pPanel : { freezetime, account, moneyBack })
		pPanel->SetVisible(false);
}

void CCSBuySubMenu_DeathMatch::LoadControlSettings(const char *dialogResourceName, const char *pathID, KeyValues *pPreloadedKeyValues)
{
	BaseClass::LoadControlSettings(dialogResourceName, pathID, pPreloadedKeyValues);
	BaseClass::LoadControlSettings("Resource/UI/cso_buysubmenu_ver5.res", "GAME");

	// hide zbs
	m_pUpgradeTitle->SetVisible(false);
	m_pOppZombiUpgradeTitle->SetVisible(false);

	// Right Fav List Hide
	for (int i = 0; i < 5; i++)
	{
		m_pFavButtons[i]->SetVisible(false);
		m_pFavSaveButtons[i]->SetVisible(false);
	}
	m_pFavDirectBuy->SetVisible(false);

	// hide money & buy time
	for (auto pPanel : { account_num ,buytime_num ,moneyText })
		pPanel->SetVisible(false);
	for (auto pPanel : { freezetime, account, buytime, moneyBack })
		pPanel->SetVisible(false);

	// hide rebuy & autobuy
	m_pRebuyButton->SetVisible(false);
	m_pAutobuyButton->SetVisible(false);
}

void CCSBuySubMenu_DefaultMode::SetupPage(size_t iPage)
{
	BaseClass::SetupPage(iPage);
	m_pSlotButtons[9]->SetText("#CSO_EndWpnBuy");
	m_pSlotButtons[9]->SetCommand("vguicancel");
	m_pSlotButtons[9]->SetHotkey('0');
}

void CCSBuySubMenu_DefaultMode::OnSelectWeapon(const char *weapon)
{
	char szBuffer[64];

	sprintf(szBuffer, "moe_buy %s", weapon);
	gEngfuncs.pfnClientCmd(szBuffer);
	OnCommand("vguicancel");
}

void CCSBuySubMenu_DefaultMode::OnSelectFavoriteWeapons(int iSet)
{
	BaseClass::OnSelectFavoriteWeapons(iSet);
}

void CCSBuySubMenu_ZombieMod::OnCommand(const char *command)
{
	if (!Q_strcmp(command, "vest"))
	{
		m_SelectedItems.iKelmet = ArmorType::ARMOR;
		UpdateFavoriteSetsControls();
		BaseClass::SetupItems(BUY_NONE);
		return;
	}
	else if (!Q_strcmp(command, "vesthelm"))
	{
		m_SelectedItems.iKelmet = ArmorType::ARMOR_HELMET;
		UpdateFavoriteSetsControls();
		BaseClass::SetupItems(BUY_NONE);
		return;
	}
	else if (!Q_strcmp(command, "flash") || !Q_strcmp(command, "sgren") || !Q_strcmp(command, "defuser") || !Q_strcmp(command, "nvgs"))
	{
		auto msgbox = new vgui2::MessageBox("#CSO_MODE_LOCK_H", "#CSO_MODE_LOCK_B", this);
		msgbox->SetOKButtonText("#CSO_OKl_Btn");
		msgbox->SetOKButtonVisible(true);
		msgbox->SetBounds(GetWide() / 2 - 150, GetTall() / 2 - 100, 300, 200);
		msgbox->DoModal();
		msgbox->Activate();
		BaseClass::SetupItems(BUY_NONE);
		return;
	}
	BaseClass::OnCommand(command);
}

void CCSBuySubMenu_ZombieMod::OnSelectWeapon(const char *weapon)
{
	BaseClass::OnSelectWeapon(weapon);
}

void CCSBuySubMenu_ZombieMod::SetupItems(MoEWeaponBuyType type)
{
	BaseClass::SetupItems(type);

	if (type == BUY_NONE)
	{
		for (int i : {5,6})
		{
			m_pSlotButtons[i]->SetText("");
			m_pSlotButtons[i]->SetCommand("VGUI_BuyMenu_Show");
			m_pSlotButtons[i]->SetVisible(true);
			m_pSlotButtons[i]->SetHotkey('0' + i + 1);
		}
		m_pSlotButtons[9]->SetCommand("VGUI_BuyMenu_Show");
	}
}