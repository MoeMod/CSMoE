#pragma once

#include "hud_sub.h"
typedef struct WeaponPickup_s
{
	char szWeapon[64];
	int iWeapon;
	float flDisplayTime;
}
WeaponPickup_t;

typedef struct AmmoPickup_s
{
	int iSprite;
	wrect_t wrect;
	int iAmount;
	float flDisplayTime;
}
AmmoPickup_t;

class CNewHud : public IBaseHudSub
{
public:
	void Initialize(void);
	int VidInit(void) override;
	int Draw(float time) override;

	int DrawNumberString(int index, int iX, int iY, int number, int r, int g, int b, int a, int iDrawZero, int maxsize, int widthplus = 0);

	int GetNumberTotalWidth(int index, int number, int iDrawZero, int maxsize, int widthplus = 0);
	int GetNumberWidth(int index);
	int GetNumberHeight(int index);
	void DrawNumber(int index, int num, int iX, int iY, int iR, int iG, int iB, int iA);

	int DrawHudNumber(int type, int iX, int iY, int number, int r, int g, int b, int a, int iDrawZero, int maxsize, int widthplus = 0);
	int GetHudNumberWidth(int type, int number, int iDrawZero, int maxsize, int widthplus = 0);

	void DisplayPlayerState(float flTime);
	void DisplayWeapon(float flTime);
	void DisplayMoney(float flTime);
	void DisplayWeaponState(float flTime);
	void DisplayItemState(float flTime);
	void DisplayScoreboard(float flTime);
	void DisplayZombieTimer(float flTime);

	void PushAmmoPickUp(int ammoid, int amount, int ammo2);
	void PushWeaponPickUp(char* szWeapon, int iWeapon);
	void SetClass(char* szClass);

	void PushMoney(int iMoney)
	{
		m_iMoneyChange = iMoney - m_iMoney;
		m_iMoney = iMoney;
		m_flMoney = cl.time + 3.0f;
	}
	void SetTime(float flTime) { m_flTime = flTime; }
	float GetTime(void) { return m_flTime; };
	void SetFlashEnable(bool bEnable) { m_bUseFlash = bEnable; };
	void SetNvgHave(bool bHave) { m_bHasNvg = bHave; };
	void SetNvgActive(bool bActive) { m_bUsingNvg = bActive; };

	void SetDefuserHave(bool bHave) { m_bHasDefuser = bHave; };

	void SetC4Have(bool bHave) { m_bHasC4 = bHave; };
	void SetC4Active(bool bActive) { m_bCanC4 = bActive; };
	void DrawNumber(int n, int x, int y, float scale, int r, int g, int b);
	void DrawNumbers(int n, int x, int y, int from_right, float scale = 1.0f, int r = 255, int g = 255, int b = 255);

public:
	int m_iIcon_MapPoint;
	int m_iMapTitleBG;

	UniqueTexture m_iScoreboardBG;
	UniqueTexture m_iScoreboardBG_GD;
	UniqueTexture m_iScoreboardBG_HMS;
	UniqueTexture m_iScoreboardBG_TIME;
	UniqueTexture m_iScoreboardBG_ZB3;
	UniqueTexture m_iScoreboardBG_ZBF;
	UniqueTexture m_iScoreboardBG_ZBG;
	UniqueTexture m_iScoreboardBG_ZSHT01;
	UniqueTexture m_iScoreboardBG_ZSHT02;
	UniqueTexture m_iScoreboardBG_ZTD01;
	UniqueTexture m_iScoreboardBG_ZTD02;

	int m_iFontWidth;
	int m_iFontWidth_Dollar;
	int m_iFontHeight;
	int m_iFontHeight_Dollar;

	int m_iPlus;

private:
	int m_iCharacterBG;
	int m_iCharacterBG_Bottom;
	int m_iCharacterBG_Top;
	int m_iDollarBG;

	int m_iIconHuman;

	int m_iLine;

	int m_iNum_BigBlue;
	int m_iNum_BigRed;

	int m_iNum_BigWhite;
	int m_iColon_BigWhite;

	int m_iNum_Bottom;
	int m_iColon_Bottom;

	int m_iNum_Center;
	int m_iSlash_Center;

	int m_iPoint_Human;
	int m_iSlash_Human;

	int m_iNum_HumanSmall;
	int m_iNum_HumanScenario;

	int m_iNum_SmallBlue;
	int m_iNum_SmallRed;

	int m_iNum_Character;

	int m_iNum_BigBlueC[10];
	int m_iNum_BigRedC[10];

	int m_iNum_BigWhiteC[10];

	int m_iNum_BottomC[10];

	int m_iNum_CenterC[10];

	int m_iNum_HumanSmallC[10];
	int m_iNum_HumanScenarioC[10];

	int m_iNum_SmallBlueC[10];
	int m_iNum_SmallRedC[10];

	int m_iNum_CharacterC[10];

	int m_iSkillIconBG;

	int m_iIcon_1st;

	int m_iIcon_A_blueL;
	int m_iIcon_A_redL;

	int m_iIcon_A_blueR;
	int m_iIcon_A_redR;

	int m_iIcon_B_blueL;
	int m_iIcon_B_redL;

	int m_iIcon_B_blueR;
	int m_iIcon_B_redR;

	int m_iIcon_CT_Left;
	int m_iIcon_CT_Right;

	int m_iIcon_Goal;

	int m_iIcon_HM_Left;
	int m_iIcon_HM_Right;

	int m_iIcon_Kill;
	int m_iIcon_KillC;
	int m_iIcon_KillR;

	int m_iIcon_Lv;
	int m_iIcon_LvUp;
	int m_iIcon_LvMax;

	int m_iIcon_My;

	int m_iIcon_Round;
	int m_iIcon_Score;

	int m_iIcon_TeamKill;
	int m_iIcon_TimeAttack;
	int m_iIcon_TotalKillL;

	UniqueTexture m_iIcon_TR_Left;
	UniqueTexture m_iIcon_TR_Right;

	int m_iIcon_Win_Center;
	int m_iIcon_Win_Right;

	int m_iIcon_ZB_Left;
	int m_iIcon_ZB_Right;

	int m_iWeaponBG;

	int m_iWeapon_OffBG;
	int m_iWeapon_OnBG;

	int m_iList_NewWpn;

	int m_iIcon_ZBNotDead;
	int m_iBg_ZBNotDead;

	int m_iIcon_ZSHT_Moon;
	int m_iIcon_ZSHT_Sun;
	int m_iIcon_ZSHT_Escape;

	int m_iIcon_Damage;
	int m_iIcon_Speed;

	int m_iNvgOff;
	int m_iNvgOn;

	int m_iNvgZBOff;
	int m_iNvgZBOn;

	int m_iDefuser;

	int m_iDollar;
	int m_iMinus;

	int m_iWeaponSel;
	int m_iWeaponGet;

	int m_iC4On;
	int m_iC4Off;
	int m_iHostage;

	int m_iCross;
	int m_iSuit[2];
	int m_iSuitHelmet[2];

	int m_iDollarNumberZero;
	int m_iNumberZero;

	int m_iZombieKnife_i;
	int m_iZombieKnife_ia;

	int m_iBuyzone;

	AmmoPickup_t m_AmmoPickup[4];
	WeaponPickup_t m_WeaponPickup[4];

	char m_szClass[32];
	int m_iClass;

	int m_iIcon_Flash[3];

	bool m_bUseFlash;

	bool m_bHasNvg;
	bool m_bUsingNvg;

	bool m_bHasDefuser;

	bool m_bHasC4;
	bool m_bCanC4;

	int m_iMoney;
	int m_iMoneyChange;
	float m_flMoney;

	float m_flZBIconFlash;

	float m_flTime;
};
