
#pragma once

#include "hud_sub.h"
namespace cl {
class CHudScoreBoardLegacy : public IBaseHudSub
{
public:
	CHudScoreBoardLegacy();
	~CHudScoreBoardLegacy() override;
	int VidInit(void) override;
	int Draw(float time) override;
	int DrawNewHud(float time);
	void Reset(void) override;		// called every time a server is connected to

protected:
	void BuildHudNumberRect(int moe, wrect_t *prc, int w, int h, int xOffset, int yOffset);
	int DrawHudNumber(int moe, wrect_t *prc, int x, int y, int iFlags, int iNumber, int r, int g, int b);
	int GetHudNumberWidth(int moe, wrect_t *prc, int iFlags, int iNumber);
	//newhud
public:
	static void BuildNumberRC(wrect_t(&rgrc)[10], int w, int h);
	static int DrawTexturedNumbersTopRightAligned(const CTextureRef& tex, const wrect_t(&rect)[10], int iNumber, int x, int y, int iflags, int igaps = 0, float scale = 1.0f, byte r = 255, byte g = 255, byte b = 255, byte a = 255);
	static int DrawTexturedNumbers(const CTextureRef& tex, const wrect_t(&rect)[10], int iNumber, int x, int y, int iflags, int igaps = 0, float scale = 1.0f, byte r = 255, byte g = 255, byte b = 255, byte a = 255);
	static void DrawTexturePart(const CTextureRef& tex, const wrect_t& rect, int x1, int y1, float scale = 1.0f, byte r = 255, byte g = 255, byte b = 255, byte a = 255);
	static unsigned math_log10(unsigned v, int iflags);

private:
	int m_iBGIndex;
	int m_iTextIndex;
	int m_iTTextIndex, m_iCTTextIndex;

	wrect_t m_rcNumber_Large[10];
	wrect_t m_rcNumber_Small[10];
	wrect_t m_rcNumber_csgo[10];

	int m_iOriginalBG;
	int m_iTeamDeathBG;
	int m_iUnitehBG;
	int m_iNum_L;
	int m_iNum_S;
	int m_iText_CT;
	int m_iText_T;
	int m_iText_TR;
	int m_iText_HM;
	int m_iText_ZB;
	int m_iText_1st;
	int m_iText_Kill;
	int m_iText_Round;
	int m_iNum_csgo;
	UniqueTexture m_pTexture_Board;

	int m_HUD_timer;
	int m_iTime;
	float m_fStartTime;
	bool m_bPanicColorChange;
	float m_flPanicTime;
	int m_closestRight;

	// new hud
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

	SharedTexture m_iNum_BigBlue;
	SharedTexture m_iNum_BigRed;

	SharedTexture m_iNum_BigWhite;
	int m_iColon_BigWhite;

	SharedTexture m_iNum_Center;
	UniqueTexture m_iSlash_Center;

	SharedTexture m_iNum_SmallBlue;
	SharedTexture m_iNum_SmallRed;

	wrect_t m_iNum_BigBlueC[10];
	wrect_t m_iNum_BigRedC[10];

	wrect_t m_iNum_BigWhiteC[10];

	wrect_t m_iNum_CenterC[10];

	int m_iNum_HumanSmallC[10];
	int m_iNum_HumanScenarioC[10];

	wrect_t m_iNum_SmallBlueC[10];
	wrect_t m_iNum_SmallRedC[10];

	int m_iSkillIconBG;

	UniqueTexture m_iIcon_1st;

	int m_iIcon_A_blueL;
	int m_iIcon_A_redL;

	int m_iIcon_A_blueR;
	int m_iIcon_A_redR;

	int m_iIcon_B_blueL;
	int m_iIcon_B_redL;

	int m_iIcon_B_blueR;
	int m_iIcon_B_redR;

	UniqueTexture m_iIcon_CT_Left;
	UniqueTexture m_iIcon_CT_Right;

	int m_iIcon_Goal;

	UniqueTexture m_iIcon_HM_Left;
	UniqueTexture m_iIcon_HM_Right;

	int m_iIcon_Kill;
	UniqueTexture m_iIcon_KillC;
	int m_iIcon_KillR;

	int m_iIcon_Lv;
	int m_iIcon_LvUp;
	int m_iIcon_LvMax;

	UniqueTexture m_iIcon_My;

	UniqueTexture m_iIcon_Round;
	int m_iIcon_Score;

	UniqueTexture m_iIcon_TeamKill;
	int m_iIcon_TimeAttack;
	int m_iIcon_TotalKillL;

	UniqueTexture m_iIcon_TR_Left;
	UniqueTexture m_iIcon_TR_Right;

	UniqueTexture m_iIcon_Win_Center;

	UniqueTexture m_iIcon_ZB_Left;
	UniqueTexture m_iIcon_ZB_Right;


};
}