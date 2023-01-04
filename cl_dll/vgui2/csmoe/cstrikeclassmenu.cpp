#include "hud.h"
#include "cstrikeclassmenu.h"

#include <cdll_dll.h>
#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui_controls/ImageList.h>
#include <FileSystem.h>

#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Panel.h>
#include "cso_controls/DarkTextEntry.h"



using namespace vgui2;

const Color COL_NONE = { 255, 255, 255, 255 };
const Color COL_CT = { 192, 205, 224, 255 };
const Color COL_TR = { 216, 182, 183, 255 };

struct ClassInfo
{
	std::string model;
	std::string name;
	std::string desc;
	std::string skillname;
	std::string skillname2;
};

const ClassInfo Class_TE[] = {
	//介绍图片, 按钮, 介绍
	{"yuri", "安娜","冠名：白白,QQ群：706711420"},
	{"pirateboy", "渡鸦", "冠名：橘子,QQ群：706711420"},
	{"marineboy", "恩佐", "冠名：維吉爾,QQ群：706711420"},
	{"pirategirl", "米娜","冠名：樓哥,QQ群：706711420"},
	{"rb", "红帽子","冠名：馬桶GIEGIE,QQ群：706711420"},
	{"jpngirl01", "熏","冠名：掉下來的鴿子,QQ群：706711420"},
	{"ritsuka", "伊琳娜", "冠名：南桐姥爷,QQ群：706711420"}
};

const ClassInfo Class_CT[] = {
	//介绍图片, 按钮, 介绍
	{"saf", "丛林猛虎", "冠名：慕名病嬌：706711420"},
	{"choijiyoon", "崔志云", "冠名：泡泡茶壶,QQ群：706711420"},
	{"fernando", "费尔南多", "冠名：华强哥：QQ群：706711420"},
	{"707", "707", "冠名：生化哥，直播间DI:PLAHF666,QQ群：706711420"},
	{"sozo", "雷霆小组", "冠名：空位,QQ群：706711420"},
	{"magui", "炎龙战术小队", "冠名：空位,QQ群：706711420"},
	{"natasha", "娜塔莎", "冠名：空位,QQ群：706711420"}
};

const ClassInfo Class_Zb[] = {

	{"tank", "#CSO_ZombieType_defaultzb","#CSO_zombi_Z_Label","zombicrazy","strengthrecovery"},
	{"speed", "#CSO_ZombieType_lightzb","#CSO_Lightzombi_Z_Label","zombihiding","zombijumpup"},
	{"heavy", "#CSO_ZombieType_heavyzb","#CSO_HeavyZombi_Z_Label","zombitrap","armorrecovery"},
	{"pc", "#CSO_ZombieType_pczb","#CSO_PCZombi_Z_Label","zombismoke","strengthrecovery"},
	{"heal", "#CSO_ZombieType_doctorzb","#CSO_Doctorzombi_Z_Label","zombiheal","strengthrecovery"},
	{"deimos", "#CSO_ZombieType_deimoszb","#CSO_deimoszombi_Z_Label","zombideimos","armorrecovery"},
	{"deimos2", "#CSO_ZombieType_deimos2zb","#CSO_deimos2zombi_Z_Label","zombicrazy2","armorrecovery"},
	{"zbs_deimos2", "#CSO_ZombieType_deimos2zb","#CSO_deimos2zombi_Z_Label","zombicrazy2","armorrecovery"},
	{"teleport", "#CSO_ZombieType_teleportzb","#CSO_RandomZombi_Z_Label","zombiteleport","hpbuff"},
	{"booster", "#CSO_ZombieType_boosterzb","#CSO_boosterzombi_Z_Label","dive","zombijumpup"},
	{"china", "#CSO_ZombieType_chinazb","#CSO_china_Z_Label","zombistiffen","strengthrecovery"},
	{"pass", "#CSO_ZombieType_passzb","#CSO_passzombi_Z_Label","pass","strengthrecovery"},
	{"flying", "#CSO_ZombieType_flyingzb","#CSO_flyingzombi_Z_Label","glide","zombicrazy"},
	{"revival", "#CSO_ZombieType_revivalzb","#CSO_RandomZombi_Z_Label","zombirevival","hpbuff"},
	{"stamper", "#CSO_ZombieType_undertakerzb","#CSO_undertaker_Z_Label","zombipile","armorrecovery"},
	{"resident", "#CSO_ZombieType_residentzb","#CSO_Residentzombi_Z_Label","zombipenetration","zombijumpup"},
	{"witch", "#CSO_ZombieType_witchzb","#CSO_witch_Z_Label","zombihook","zombijumpup"},
	{"aksha", "#CSO_ZombieType_akshazb","#CSO_akshazombi_Z_Label","charge","armorrecovery"},
	{"boomer", "#CSO_ZombieType_boomerzb","#CSO_boomerzombi_Z_Label","zombiselfdestruct","armorrecovery"},

	{"nemesis", "#CSO_ZombieType_nemesiszb","#CSO_zombi_Z_Label","zombitrap","armorrecovery"},
	{"spider", "#CSO_ZombieType_spiderzb","#CSO_zombi_Z_Label","webshooter","webbomb"},
	{"deathknight", "#CSO_ZombieType_deathknight","#CSO_zombi_Z_Label","protect","chargeslash"},
	{"siren", "#CSO_ZombieType_sirenzb","#CSO_zombi_Z_Label","zombitrap","armorrecovery"},
};

CCSClassMenu::CCSClassMenu(IViewport* pViewPort) : CClassMenu(pViewPort)
{

	m_pTitleLabel = new vgui2::Label(this, "CaptionLabel", CheckShowType() ? "选择默认僵尸" : "选择角色");
	m_pShowCT = new NewTabButton(this, "ShowCTWeapon", "#CSO_ClsCT");
	m_pShowTER = new NewTabButton(this, "ShowTERWeapon", "#CSO_ClsTER");

	m_pPrevBtn = new vgui2::Button(this, "prevBtn", "#CSO_PrevBuy");
	m_pNextBtn = new vgui2::Button(this, "nextBtn", "#CSO_NextBuy");

	m_pClassImage = new vgui2::ImagePanel(this, "ClassInfo");
	m_pClassImage->SetShouldScaleImage(true);

	m_pSkillInfo[0] = new vgui2::ImagePanel(this, "SkillInfo1");
	m_pSkillInfo[0]->SetShouldScaleImage(true);
	m_pSkillInfo[1] = new vgui2::ImagePanel(this, "SkillInfo2");
	m_pSkillInfo[1]->SetShouldScaleImage(true);
	m_pSkillInfo[0]->SetVisible(false);
	m_pSkillInfo[1]->SetVisible(false);

	m_pSkillInfoImage[0] = new vgui2::ImagePanel(this, "SkillInfo1_Main");
	m_pSkillInfoImage[0]->SetShouldScaleImage(true);
	m_pSkillInfoImage[1] = new vgui2::ImagePanel(this, "SkillInfo2_Main");
	m_pSkillInfoImage[1]->SetShouldScaleImage(true);
	m_pSkillInfoImage[0]->SetVisible(false);
	m_pSkillInfoImage[1]->SetVisible(false);

	m_pClassDesc = new vgui2::Label(this, "ClassDesc", "");
	m_pTipText = new vgui2::Label(this, "ClassDesc", "#CSO_ZOMBI3_KeepZombiTypeDesc");

	m_pSkillInfoText[0] = new vgui2::Label(this, "SkillInfo1_Text", "#CSO_ZombieSkill_zombicrazy");
	m_pSkillInfoText[1] = new vgui2::Label(this, "SkillInfo2_Text", "#CSO_ZombieSkill_zombicrazy");
	m_pSkillInfoText_Desc[0] = new vgui2::Label(this, "SkillInfo1_TextDesc", "#CSO_ZI_Skill_Desc_zombicrazy");
	m_pSkillInfoText_Desc[1] = new vgui2::Label(this, "SkillInfo2_TextDesc", "#CSO_ZI_Skill_Desc_zombicrazy");

	m_pShowCT->SetTextColor(COL_CT);
	m_pShowTER->SetTextColor(COL_TR);
	m_pShowCT->SetCommand("showctclass");
	m_pShowTER->SetCommand("showterclass");

	m_pShowCT->SetEnabled(true);
	m_pShowTER->SetEnabled(true);

	char buffer[64];
	for (int i = 0; i < 10; i++)
	{
		sprintf(buffer, "slot%d", i);
		m_pSlotButtons[i] = new NewMouseOverPanelButton(this, buffer, m_pPanel);
		m_pSlotButtons[i]->GetClassPanel()->SetName("ClassInfo");

		sprintf(buffer, "VGUI_ClassMenu_Select %d", i + 1);

		m_pSlotButtons[i]->SetVisible(true);
		m_pSlotButtons[i]->SetText("null");
		m_pSlotButtons[i]->SetHotkey('0' + i + 1);
		m_pSlotButtons[i]->SetCommand(buffer);
		m_pSlotButtons[i]->AddActionSignalTarget(this);

	}
	m_iCurrentPage = 0;
	m_iCurrentTeamPage = CT;

	m_pTitleLabel->SetFont(scheme()->GetIScheme(m_pTitleLabel->GetScheme())->GetFont("Title", IsProportional()));
	m_pTitleLabel->SizeToContents();

	m_pClassDesc->SetFont(scheme()->GetIScheme(m_pClassDesc->GetScheme())->GetFont("Default", IsProportional()));
	m_pClassDesc->SizeToContents();

	m_pTipText->SetFont(scheme()->GetIScheme(m_pTipText->GetScheme())->GetFont("Default", IsProportional()));
	m_pTipText->SizeToContents();

	m_pSkillInfoText[0]->SetFont(scheme()->GetIScheme(m_pSkillInfoText[0]->GetScheme())->GetFont("DefaultSmall", IsProportional()));
	m_pSkillInfoText[0]->SizeToContents();
	m_pSkillInfoText[1]->SetFont(scheme()->GetIScheme(m_pSkillInfoText[1]->GetScheme())->GetFont("DefaultSmall", IsProportional()));
	m_pSkillInfoText[1]->SizeToContents();
	m_pSkillInfoText_Desc[0]->SetFont(scheme()->GetIScheme(m_pSkillInfoText_Desc[0]->GetScheme())->GetFont("DefaultSmall", IsProportional()));
	m_pSkillInfoText_Desc[0]->SizeToContents();
	m_pSkillInfoText_Desc[1]->SetFont(scheme()->GetIScheme(m_pSkillInfoText_Desc[1]->GetScheme())->GetFont("DefaultSmall", IsProportional()));
	m_pSkillInfoText_Desc[1]->SizeToContents();

	m_pClassDesc->SetVisible(false);
	m_pTipText->SetVisible(false);

	m_pSkillInfoText[0]->SetVisible(false);
	m_pSkillInfoText[1]->SetVisible(false);
	m_pSkillInfoText_Desc[0]->SetVisible(false);
	m_pSkillInfoText_Desc[1]->SetVisible(false);
	//cso_classmenu_ver2
	LoadControlSettings("Resource/UI/cso_classmenu_ver2.res", "GAME");
}
CCSClassMenu::~CCSClassMenu(void)
{
	for (int i = 0; i < 10; i++)
	{
		m_pSlotButtons[i] = nullptr;
	}
}
void CCSClassMenu::SetupControlSettings()
{
	InvalidateLayout();
}
void CCSClassMenu::SetTeam(TeamName team)
{
	if (CheckShowType())
		return;

	const ClassInfo* info = nullptr;
	int num = 0;
	Color col = COL_NONE;

	m_pShowCT->SetActive(false);
	m_pShowTER->SetActive(false);

	m_pShowCT->SetEnabled(true);
	m_pShowTER->SetEnabled(true);

	m_pShowCT->SetVisible(true);
	m_pShowTER->SetVisible(true);

	if (team == CT)
		m_pShowCT->SetActive(true);
	if (team == TERRORIST)
		m_pShowTER->SetActive(true);

	m_iCurrentTeamPage = team;

	if (team == CT)
	{
		info = Class_CT;
		num = 7;
		col = COL_CT;
	}
	else if (team == TERRORIST)
	{
		info = Class_TE;
		num = 7;
		col = COL_TR;
	}

	for (int i = 0; i < 7; ++i)
	{
		m_pSlotButtons[i]->SetFgColor(col);
		m_pSlotButtons[i]->SetText(info[i].name.c_str());
		m_pSlotButtons[i]->SetVisible(true);
		m_pSlotButtons[i]->SetHotkey('0' + i + 1);
		m_pSlotButtons[i]->SetEnabled(true);

		char buffer[64];
		sprintf(buffer, "VGUI_ClassMenu_Select %d", i + 1);
		m_pSlotButtons[i]->SetCommand(buffer);
	}

	for (int i = min(num, 10); i < 10; ++i)
	{
		m_pSlotButtons[i]->SetText("null");
		m_pSlotButtons[i]->SetHotkey(i != 9 ? ('0' + i + 1) : '0');
		m_pSlotButtons[i]->SetEnabled(false);
		m_pSlotButtons[i]->SetVisible(true);
	}

	// TODO : pages
	m_pTitleLabel->SetText(CheckShowType() ? "选择默认僵尸" : "选择角色");

	m_pPrevBtn->SetVisible(false);
	m_pNextBtn->SetVisible(false);

	m_pTipText->SetVisible(false);
	m_pSkillInfo[0]->SetVisible(false);
	m_pSkillInfo[1]->SetVisible(false);

	m_pSkillInfoText[0]->SetVisible(false);
	m_pSkillInfoText[1]->SetVisible(false);
	m_pSkillInfoText_Desc[0]->SetVisible(false);
	m_pSkillInfoText_Desc[1]->SetVisible(false);
	m_pSkillInfoImage[0]->SetVisible(false);
	m_pSkillInfoImage[1]->SetVisible(false);

	m_iCurrentPage = 0;

}

void CCSClassMenu::SetupPage(size_t iPage)
{
	if (!CheckShowType())
		return;
	int totalpages = ((23 + 1) - 1) / 9 + 1;
	if (iPage <= 0) iPage = 0;
	else if (iPage >= totalpages)
		iPage = totalpages - 1;

	m_pTitleLabel->SetText(CheckShowType() ? "选择默认僵尸" : "选择角色");
	m_pTitleLabel->SizeToContents();

	m_pShowCT->SetActive(false);
	m_pShowTER->SetActive(false);
	m_pShowCT->SetVisible(false);
	m_pShowTER->SetVisible(false);
	m_pShowCT->SetEnabled(false);
	m_pShowTER->SetEnabled(false);

	m_pSkillInfo[0]->SetVisible(true);
	m_pSkillInfo[1]->SetVisible(true);
	m_pSkillInfoText[0]->SetVisible(true);
	m_pSkillInfoText[1]->SetVisible(true);
	m_pSkillInfoText_Desc[0]->SetVisible(true);
	m_pSkillInfoText_Desc[1]->SetVisible(true);
	m_pSkillInfoImage[0]->SetVisible(true);
	m_pSkillInfoImage[1]->SetVisible(true);

	m_pTipText->SetVisible(true);
	m_iCurrentPage = iPage;

	// page buttons
	m_pPrevBtn->SetVisible(iPage != 0);
	m_pNextBtn->SetVisible(m_iCurrentPage != totalpages - 1);

	for (int i = 0; i < 9; ++i)
	{
		int iElement = m_iCurrentPage * 9 + i;
		if (iElement >= 23)
		{
			m_pSlotButtons[i]->SetVisible(false);
		}
		else
		{
			const char* SzGfxName = Class_Zb[iElement].model.c_str();
			const char* SzName = Class_Zb[iElement].name.c_str();
			char buffer[64];
			sprintf(buffer, "VGUI_ZombieKeeper_Select %d", iElement);


			m_pSlotButtons[i]->SetText(SzName);
			m_pSlotButtons[i]->SetVisible(true);
			m_pSlotButtons[i]->SetEnabled(true);
			m_pSlotButtons[i]->SetHotkey('0' + i + 1);
			m_pSlotButtons[i]->SetCommand(buffer);
			m_pSlotButtons[i]->AddActionSignalTarget(this);

		}
	}

	m_pSlotButtons[9]->SetText("取消（默认）选择");
	m_pSlotButtons[9]->SetVisible(true);
	m_pSlotButtons[9]->SetEnabled(true);
	m_pSlotButtons[9]->SetCommand("VGUI_ZombieKeeper_Select 66");
	m_pSlotButtons[9]->SetHotkey('0');
	m_pSlotButtons[9]->AddActionSignalTarget(this);

}

const char* CCSClassMenu::GetName(void)
{
	return PANEL_CLASS;
}

void CCSClassMenu::Reset(void)
{
	CClassMenu::Reset();
}

MouseOverPanelButton* CCSClassMenu::CreateNewMouseOverPanelButton(vgui2::EditablePanel* panel)
{
	return new NewMouseOverPanelButton(this, "MouseOverPanelButton", panel);
}

void CCSClassMenu::ShowPanel(bool bShow)
{
	if (bShow)
	{
		if (cl::gHUD.m_iIntermission || cl::gEngfuncs.IsSpectateOnly())
			return;
	}

	BaseClass::ShowPanel(bShow);
}

void CCSClassMenu::SetVisible(bool state)
{
	BaseClass::SetVisible(state);
}

void CCSClassMenu::Update(void)
{
}

void CCSClassMenu::PaintBackground(void)
{
	BaseClass::PaintBackground();
}

void CCSClassMenu::PerformLayout(void)
{
	int  w, h;
	GetSize(w, h);
	int w2, h2;
	float scale = h / 420.0;

	m_pTitleLabel->GetSize(w2, h2);
	m_pTitleLabel->SetPos(w / 2 - w2 / 2, vgui2::scheme()->GetProportionalScaledValue(12));

	m_pShowTER->SetBounds(
		vgui2::scheme()->GetProportionalScaledValue(12),
		vgui2::scheme()->GetProportionalScaledValue(32),
		vgui2::scheme()->GetProportionalScaledValue(49),
		vgui2::scheme()->GetProportionalScaledValue(16)
	);
	m_pShowCT->SetBounds(
		vgui2::scheme()->GetProportionalScaledValue(60),
		vgui2::scheme()->GetProportionalScaledValue(32),
		vgui2::scheme()->GetProportionalScaledValue(49),
		vgui2::scheme()->GetProportionalScaledValue(16)
	);
	m_pClassImage->SetBounds(
		vgui2::scheme()->GetProportionalScaledValue(263),
		vgui2::scheme()->GetProportionalScaledValue(50),
		vgui2::scheme()->GetProportionalScaledValue(200),
		vgui2::scheme()->GetProportionalScaledValue(150)
	);

	m_pSkillInfo[0]->SetBounds(
		vgui2::scheme()->GetProportionalScaledValue(263),
		vgui2::scheme()->GetProportionalScaledValue(230),
		vgui2::scheme()->GetProportionalScaledValue(300),
		vgui2::scheme()->GetProportionalScaledValue(75)
	);
	m_pSkillInfoImage[0]->SetBounds(
		vgui2::scheme()->GetProportionalScaledValue(263),
		vgui2::scheme()->GetProportionalScaledValue(230 + 18),
		vgui2::scheme()->GetProportionalScaledValue(36),
		vgui2::scheme()->GetProportionalScaledValue(36)
	);

	m_pSkillInfo[1]->SetBounds(
		vgui2::scheme()->GetProportionalScaledValue(263),
		vgui2::scheme()->GetProportionalScaledValue(230 + 75 + 2),
		vgui2::scheme()->GetProportionalScaledValue(300),
		vgui2::scheme()->GetProportionalScaledValue(75)
	);
	m_pSkillInfoImage[1]->SetBounds(
		vgui2::scheme()->GetProportionalScaledValue(263),
		vgui2::scheme()->GetProportionalScaledValue(230 + 75 + 18),
		vgui2::scheme()->GetProportionalScaledValue(36),
		vgui2::scheme()->GetProportionalScaledValue(36)
	);
	m_pClassDesc->SetBounds(
		vgui2::scheme()->GetProportionalScaledValue(263 + 200),
		vgui2::scheme()->GetProportionalScaledValue(80),
		vgui2::scheme()->GetProportionalScaledValue(50),
		vgui2::scheme()->GetProportionalScaledValue(50)
	);

	m_pTipText->SetPos(vgui2::scheme()->GetProportionalScaledValue(12), vgui2::scheme()->GetProportionalScaledValue(400));
	m_pSkillInfoText[0]->SetPos(vgui2::scheme()->GetProportionalScaledValue(263), vgui2::scheme()->GetProportionalScaledValue(230));
	m_pSkillInfoText[1]->SetPos(vgui2::scheme()->GetProportionalScaledValue(263), vgui2::scheme()->GetProportionalScaledValue(230 + 75));
	m_pSkillInfoText_Desc[0]->SetPos(vgui2::scheme()->GetProportionalScaledValue(263 + 54), vgui2::scheme()->GetProportionalScaledValue(230 + 10));
	m_pSkillInfoText_Desc[1]->SetPos(vgui2::scheme()->GetProportionalScaledValue(263 + 54), vgui2::scheme()->GetProportionalScaledValue(230 + 75 + 20));


	if (CheckShowType())
	{
		m_iCurrentPage = 0;
		SetupPage(0);
	}
	else
	{
		SetTeam(m_iCurrentTeamPage);
	}

	BaseClass::PerformLayout();
}

void CCSClassMenu::ApplySchemeSettings(vgui2::IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_pClassDesc->SetFgColor(pScheme->GetColor("MapDescriptionText", Color(255, 255, 255, 0)));
	m_pTipText->SetFgColor(pScheme->GetColor("BrightBaseText", Color(255, 255, 255, 0)));

	m_pSkillInfoText[0]->SetFgColor(pScheme->GetColor("LabelDimText", Color(255, 255, 255, 180)));
	m_pSkillInfoText[1]->SetFgColor(pScheme->GetColor("LabelDimText", Color(255, 255, 255, 180)));
	m_pSkillInfoText_Desc[0]->SetFgColor(pScheme->GetColor("LabelDimText", Color(255, 255, 255, 180)));
	m_pSkillInfoText_Desc[1]->SetFgColor(pScheme->GetColor("LabelDimText", Color(255, 255, 255, 180)));

	m_pTipText->SetTextColorState(Label::CS_BRIGHT);
	m_pSkillInfo[0]->SetImage("resource/control/info_classmenu");
	m_pSkillInfo[1]->SetImage("resource/control/info_classmenu");

}

void CCSClassMenu::OnUpdateClass(const char* command)
{
	//
	if (!strncmp(command, "slot", 4))
	{
		UpdateClass(std::stoi(command + 4));
		return;
	}
}
void CCSClassMenu::OnCommand(const char* command)
{
	if (!strncmp(command, "VGUI_ClassMenu_Select ", 22))
	{
		OnSelectClass(m_iCurrentTeamPage, command + 22);
		return;
	}
	else if (!strncmp(command, "VGUI_ZombieKeeper_Select ", 25))
	{
		OnSelectClassZombie(command + 22);
		return;
	}
	else if (!strncmp(command, "VGUI_ClassMenu_SetTeam", 22))
	{
		TeamName team;
		if (command[22] == '\0')
			team = UNASSIGNED;
		else
			sscanf(command + 22, "%d", &team); //"Pistol 1"

		if (!CheckShowType())
		{
			SetTeam(CT);
			m_iCurrentTeamPage = CT;
		}

		else
		{
			m_iCurrentPage = 0;
			SetupPage(0);
		}

		return;
	}
	else if (!Q_strcmp(command, "prevpage"))
	{
		if (CheckShowType())
			SetupPage(m_iCurrentPage - 1);
		return;
	}
	else if (!Q_strcmp(command, "nextpage"))
	{
		if (CheckShowType())
			SetupPage(m_iCurrentPage + 1);
		return;
	}
	else if (!Q_strcmp(command, "showctclass"))
	{
		SetTeam(CT);
		m_iCurrentTeamPage = CT;
		return;
	}
	else if (!Q_strcmp(command, "showterclass"))
	{
		SetTeam(TERRORIST);
		m_iCurrentTeamPage = TERRORIST;
		return;
	}
	m_pViewPort->ShowBackGround(false);
	return BaseClass::OnCommand(command);
}

void CCSClassMenu::OnSelectClass(TeamName team, const char* name)
{
	char buffer[64];
	sprintf(buffer, "jointeam %d;joinclass %s", team, name);
	cl::gEngfuncs.pfnClientCmd(buffer);
	BaseClass::OnCommand("vguicancel");
}
void CCSClassMenu::OnSelectClassZombie(const char* name)
{

	char buffer[64];
	sprintf(buffer, "bte_zb2_zombie_keep %s", name);
	cl::gEngfuncs.pfnClientCmd(buffer);
	BaseClass::OnCommand("vguicancel");
}

void CCSClassMenu::UpdateGameMode()
{
	//LoadControlSettings("Resource/UI/cso_classmenu_ver2.res", "GAME");
	InvalidateLayout();

	m_pShowCT->SetVisible(false);
	m_pShowTER->SetVisible(false);

	return;
}

void CCSClassMenu::UpdateClass(int i)
{
	const ClassInfo* info = nullptr;
	//Zb Mod
	if (!CheckShowType())
	{
		if (m_iCurrentTeamPage == TERRORIST)
		{
			info = Class_TE;
		}
		else if (m_iCurrentTeamPage == CT)
		{
			info = Class_CT;
		}
		else
			info = Class_CT;
	}
	else
	{
		info = Class_Zb;
	}


	int iElement = m_iCurrentPage * 9 + i;
	if (iElement > 23)iElement = 0;
	bool bEnabled = info[iElement].model.c_str() ? true : false;

	m_pClassImage->SetVisible(bEnabled);

	if (!bEnabled)
		return;

	if (!CheckShowType())
	{
		if (i >= 7)
			return;
		//ClassImage Show
		char szBuffer[64];
		sprintf(szBuffer, "gfx/vgui/%s", info[iElement].model.c_str());
		m_pClassImage->SetImage(szBuffer);

		//ClassDesc Show
		std::string szClassDesc; szClassDesc = info[iElement].model;
		szClassDesc[0] = std::toupper(info[iElement].model[0]);
		sprintf(szBuffer, "#Cstrike_%s_Label", szClassDesc.c_str());

		auto SzCheckValidVguiString = vgui2::localize()->Find((char*)szBuffer);
		char* szBuffer2;

		if (SzCheckValidVguiString != nullptr)
		{
			szBuffer2 = new char[std::char_traits<wchar_t>::length(vgui2::localize()->Find((char*)szBuffer)) + 1];
			sprintf(szBuffer2, "%s", szBuffer);
		}

		m_pClassDesc->SetVisible(true);
		m_pClassDesc->SetTextColorState(Label::CS_BRIGHT);
		m_pClassDesc->SetText(SzCheckValidVguiString != nullptr ? szBuffer2 : szBuffer);

		if (SzCheckValidVguiString != nullptr && szBuffer2)
			delete[] szBuffer2;
	}
	else
	{

		char szBuffer[128];
		sprintf(szBuffer, (i == 9) ? "resource/hud/zombie/randomzombie" : "resource/hud/zombie/%s_zombi", info[iElement].model.c_str());
		m_pClassImage->SetImage(szBuffer);


		m_pClassDesc->SetVisible(true);
		m_pClassDesc->SetTextColorState(Label::CS_BRIGHT);
		m_pClassDesc->SetText((i == 9) ? "#CSO_RandomZombi_Z_Label" : info[iElement].desc.c_str());

		sprintf(szBuffer, (i == 9) ? "" : "resource/zombi/zombieskill_%s", info[iElement].skillname.c_str());
		m_pSkillInfoImage[0]->SetImage(szBuffer);
		sprintf(szBuffer, (i == 9) ? "" : "resource/zombi/zombieskill_%s", info[iElement].skillname2.c_str());
		m_pSkillInfoImage[1]->SetImage(szBuffer);

		char szBuffer2[128];
		sprintf(szBuffer2, (i == 9) ? "" : "#CSO_ZombieSkill_%s", info[iElement].skillname.c_str());
		m_pSkillInfoText[0]->SetText(szBuffer2);
		m_pSkillInfoText[0]->SizeToContents();
		sprintf(szBuffer2, (i == 9) ? "" : "#CSO_ZombieSkill_%s", info[iElement].skillname2.c_str());
		m_pSkillInfoText[1]->SetText(szBuffer2);
		m_pSkillInfoText[1]->SizeToContents();

		char szBuffer3[128];
		if (info[iElement].model == "spider" || info[iElement].model == "meatwallzb" || info[iElement].model == "deathknight")
		{
			std::string SzTempText = info[iElement].skillname;
			SzTempText[0] = std::toupper(SzTempText[0]);
			sprintf(szBuffer3, (i == 9) ? "" : "#CSO_Zombi5_zbsk_%s_desc", SzTempText.c_str());
		}
		else
			sprintf(szBuffer3, (i == 9) ? "" : "#CSO_ZI_Skill_Desc_%s", info[iElement].skillname.c_str());

		m_pSkillInfoText_Desc[0]->SetText(szBuffer3);

		if (info[iElement].model == "spider" || info[iElement].model == "meatwallzb" || info[iElement].model == "deathknight")
		{
			std::string SzTempText = info[iElement].skillname2;
			SzTempText[0] = std::toupper(SzTempText[0]);
			sprintf(szBuffer3, (i == 9) ? "" : "#CSO_Zombi5_zbsk_%s_desc", SzTempText.c_str());
		}
		else
			sprintf(szBuffer3, (i == 9) ? "" : "#CSO_ZI_Skill_Desc_%s", info[iElement].skillname2.c_str());

		m_pSkillInfoText_Desc[1]->SetText(szBuffer3);

		char sdaa[64]; sprintf(sdaa, "Sznam After Cast= %d\n", iElement);
		cl::gEngfuncs.pfnConsolePrint(sdaa);
	}
}


bool IsValidEntity(cl_entity_s* pEntity)
{
	if (pEntity && pEntity->model && pEntity->model->name[0]
		&& (cl::g_PlayerExtraInfo[pEntity->index].teamnumber == TEAM_CT) || (cl::g_PlayerExtraInfo[pEntity->index].teamnumber == TEAM_TERRORIST))
		return true;

	return false;
}
bool CCSClassMenu::CheckShowType()
{
	return ((cl::gHUD.IsZombieMod() && (cl::gHUD.m_iModRunning != MOD_ZB4)) && IsValidEntity(cl::gEngfuncs.GetEntityByIndex(cl::gEngfuncs.GetLocalPlayer()->index))) ? true : false;
	//return ((cl::gHUD.IsZombieMod() && (cl::gHUD.m_iModRunning != MOD_ZB4)) && (cl::g_iTeamNumber != TEAM_UNASSIGNED) && cl::gEngfuncs.GetLocalPlayer()->model!=NULL && cl::gEngfuncs.GetLocalPlayer()->curstate.playerclass)? true : false;
}