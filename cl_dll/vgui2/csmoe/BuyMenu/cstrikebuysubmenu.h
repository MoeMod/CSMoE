#ifndef CSBUYSUBMENU_H
#define CSBUYSUBMENU_H
#ifdef _WIN32
#pragma once
#endif

#include "game_controls/buysubmenu.h"
#include "BuyPresetButton.h"
#include "cstrikebuymouseoverpanelbutton.h"
#include "cso_controls/NewTabButton.h"
#include "WeaponImagePanel.h"

class CCSBuySubMenu : public CBuySubMenu
{
private:
	DECLARE_CLASS_SIMPLE(CCSBuySubMenu, CBuySubMenu);

public:
	CCSBuySubMenu(vgui2::Panel *parent, const char *name = "BuySubMenu");

protected:
	virtual CSBuyMouseOverPanelButton *CreateNewMouseOverPanelButton(EditablePanel *panel) override;
	virtual CCSBuySubMenu *CreateNewSubMenu(const char *name = "BuySubMenu") override;

public:
	virtual void LoadControlSettings(const char *dialogResourceName, const char *pathID = NULL, KeyValues *pPreloadedKeyValues = NULL) override;
	virtual void OnCommand(const char *command) override;
	virtual void PerformLayout() override;
	virtual void OnThink() override;

	virtual void SetupItems(cl::MoEWeaponBuyType type);
	virtual void SetupPage(size_t page);
	virtual void SetTeam(TeamName team);
	
	virtual void OnSelectWeapon(const char *weapon);

	virtual void ReadFavoriteSets();
	virtual void SaveFavoriteSets();
	virtual void UpdateFavoriteSetsControls();
	virtual void OnSelectFavoriteWeapons(int iSet);
	virtual void OnSaveFavoriteWeapons(int iSet);

	virtual void OnClearSelectedItems();
	virtual void OnBuySelectedItems();

protected:
	enum class ArmorType
	{
		NONE,
		ARMOR,
		ARMOR_HELMET
	};
	struct FavoriteSet
	{
		std::string Primary;
		std::string Secondary;
		std::string Melee;
		std::string HEGrenade;
		int nFlashBang;
		int nSmokeGrenade;
		int bDefuser;
		int bNightVision;
		ArmorType iKelmet;
	};
	struct ItemInfo
	{
		std::string name;
		std::string showname;
		std::string command;
		int level;
	};
	std::vector<ItemInfo> m_BuyItemList;
	FavoriteSet m_FavoriteItems[5];
	FavoriteSet m_SelectedItems;
	size_t m_iCurrentPage;
    TeamName m_iTeam;

protected:
	vgui2::Label *m_pTitleLabel;
	// Left Column
	NewTabButton *m_pShowCTWeapon;
	NewTabButton *m_pShowTERWeapon;
	CSBuyMouseOverPanelButton * m_pSlotButtons[10]; // slot0 ... slot10
	vgui2::Button *m_pPrevBtn; // prevpage
	vgui2::Button *m_pNextBtn; // nextpage

	// Middle Column
	vgui2::Button *m_pRebuyButton; // rebuy_in
	vgui2::Button *m_pAutobuyButton; // autobuy_in

	// Lower Right Corner
	vgui2::Button *m_pBasketClear; // basketclear
	vgui2::Button *m_pBasketBuy;	// basketbuy
	vgui2::Button *m_pQuitButton;  // vguicancel

								  // Lower Weapon Slot
	WeaponImagePanel *pwpnBG;
	WeaponImagePanel *swpnBG;
	WeaponImagePanel *hgrenBG;
	vgui2::ImagePanel *sgrenBG;
	vgui2::ImagePanel *fgrenBG;
	vgui2::ImagePanel *fgren2BG;
	vgui2::ImagePanel *dfBG;
	vgui2::ImagePanel *nvBG;
	vgui2::ImagePanel *kevBG;
	WeaponImagePanel *newknifeBG;

	vgui2::ImagePanel *pammoBG;
	vgui2::ImagePanel *sammoBG;

	vgui2::ImagePanel *primaryBG;
	vgui2::ImagePanel *secondaryBG;
	vgui2::ImagePanel *knifeBG;
	vgui2::ImagePanel *grenadeBG;
	vgui2::ImagePanel *equipBG;

	// Right Fav List
	BuyPresetButton *m_pFavButtons[5]; // fav0 ... fav4
	vgui2::Button *m_pFavSaveButtons[5]; // fav_save0 .. fav_save4
	vgui2::CheckButton *m_pFavDirectBuy; // fav_direct_buy_ckbtn

	vgui2::Label *account_num;
	vgui2::Label *buytime_num;
	vgui2::Label *moneyText;

	vgui2::TextEntry *freezetime;
	vgui2::TextEntry *account;
	vgui2::TextEntry *buytime;
	vgui2::TextEntry *moneyBack;

	// ZBS
	vgui2::Label *m_pUpgradeTitle;
	vgui2::Label *m_pOppZombiUpgradeTitle;

	// DM
	vgui2::ImagePanel *m_pSetSelBg;
	vgui2::Label *m_pSetLabel;
	vgui2::Button *m_pPrevSetBtn;
	vgui2::Button *m_pNextSetBtn;

	vgui2::Label *m_pEditDescLabel_DM;
	vgui2::ImagePanel *m_pEditDescBg;
	vgui2::ImagePanel *m_pEquipSample;
};

class CCSBuySubMenu_DefaultMode : public CCSBuySubMenu
{
private:
	typedef CCSBuySubMenu BaseClass;

public:
	CCSBuySubMenu_DefaultMode(vgui2::Panel *parent, const char *name = "BuySubMenu") : CCSBuySubMenu(parent, name){}
	virtual void LoadControlSettings(const char *dialogResourceName, const char *pathID = NULL, KeyValues *pPreloadedKeyValues = NULL) override;
	virtual void PerformLayout() override;
	virtual void SetupPage(size_t iPage) override;
	virtual void OnSelectWeapon(const char *weapon) override;
	virtual void OnSelectFavoriteWeapons(int iSet) override;
};

class CCSBuySubMenu_ZombieMod : public CCSBuySubMenu
{
private:
	typedef CCSBuySubMenu BaseClass;

public:
	CCSBuySubMenu_ZombieMod(vgui2::Panel *parent, const char *name = "BuySubMenu") : CCSBuySubMenu(parent, name) {}
	virtual void LoadControlSettings(const char *dialogResourceName, const char *pathID = NULL, KeyValues *pPreloadedKeyValues = NULL) override;
	virtual void OnSelectWeapon(const char *weapon) override;
	virtual void SetupItems(cl::MoEWeaponBuyType type) override;
	virtual void OnCommand(const char *command) override;
};

class CCSBuySubMenu_DeathMatch : public CCSBuySubMenu
{
private:
	typedef CCSBuySubMenu BaseClass;

public:
	CCSBuySubMenu_DeathMatch(vgui2::Panel *parent, const char *name = "BuySubMenu") : CCSBuySubMenu(parent, name) {}
	virtual void LoadControlSettings(const char *dialogResourceName, const char *pathID = NULL, KeyValues *pPreloadedKeyValues = NULL) override;
};

#endif
