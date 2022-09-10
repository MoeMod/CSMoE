#include "hud.h"
#include "cstrikeclassmenu.h"

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui_controls/ImageList.h>
#include <FileSystem.h>

#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Panel.h>

using namespace vgui2;

const Color COL_NONE = { 255, 255, 255, 255 };
const Color COL_CT = { 192, 205, 224, 255 };
const Color COL_TR = { 216, 182, 183, 255 };

struct ClassInfo
{
	const char* model;
	const wchar_t* name;
	const char* desc;
};

const ClassInfo Class_TE[] = {
	//介绍图片, 按钮, 介绍
	{"yuri", L"安娜","冠名：白白,QQ群：706711420"},
	{"pirateboy", L"渡鸦", "冠名：橘子,QQ群：706711420"},
	{"marineboy", L"恩佐", "冠名：維吉爾,QQ群：706711420"},
	{"pirategirl", L"米娜","冠名：樓哥,QQ群：706711420"},
	{"rb", L"红帽子","冠名：馬桶GIEGIE,QQ群：706711420"},
	{"jpngirl01", L"熏","冠名：掉下來的鴿子,QQ群：706711420"},
	{"ritsuka", L"伊琳娜", "冠名：南桐姥爷,QQ群：706711420"}
};

const ClassInfo Class_CT[] = {
	//介绍图片, 按钮, 介绍
	{"saf", L"丛林猛虎", "冠名：慕名病嬌：706711420"},
	{"choijiyoon", L"崔志云", "冠名：泡泡茶壶,QQ群：706711420"},
	{"fernando", L"费尔南多", "冠名：华强哥：QQ群：706711420"},
	{"707", L"707", "冠名：生化哥，直播间DI:PLAHF666,QQ群：706711420"},
	{"sozo", L"雷霆小组", "冠名：空位,QQ群：706711420"},
	{"magui", L"炎龙战术小队", "冠名：空位,QQ群：706711420"},
	{"natasha", L"娜塔莎", "冠名：空位,QQ群：706711420"}
};

CCSClassMenu::CCSClassMenu(IViewport* pViewPort) : CClassMenu(pViewPort)
{
	m_pShowCTWeapon = new NewTabButton(this, "ShowCTWeapon", "#CSO_BuyShowCT");
	m_pShowTERWeapon = new NewTabButton(this, "ShowTERWeapon", "#CSO_BuyShowTER");
	m_pShowCTWeapon->SetTextColor(COL_CT);
	m_pShowTERWeapon->SetTextColor(COL_TR);

	char buffer[64];
	for (int i = 0; i < 10; i++)
	{
		sprintf(buffer, "slot%d", i);
		m_pSlotButtons[i] = new NewMouseOverPanelButton(this, buffer, m_pPanel);
		m_pSlotButtons[i]->GetClassPanel()->SetName("ItemInfo");

		sprintf(buffer, "VGUI_ClassMenu_Select %d", i + 1);
		m_pSlotButtons[i]->SetCommand(buffer);
	}

	m_pPrevBtn = new vgui2::Button(this, "prevBtn", "#CSO_PrevBuy");
	m_pNextBtn = new vgui2::Button(this, "nextBtn", "#CSO_NextBuy");

	LoadControlSettings("Resource/UI/cso_classmenu_ver2.res", "GAME");
}

void CCSClassMenu::SetTeam(TeamName team)
{
	const ClassInfo* info = nullptr;
	int num = 0;
	Color col = COL_NONE;
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

	for (int i = 0; i < min(num, 10); ++i)
	{
		m_pSlotButtons[i]->SetFgColor(col);
		m_pSlotButtons[i]->SetText(info[i].name);
		m_pSlotButtons[i]->SetVisible(true);
		m_pSlotButtons[i]->SetHotkey('0' + i + 1);
	}

	for (int i = min(num, 10); i < 10; ++i)
	{
		m_pSlotButtons[i]->SetVisible(false);
	}

	// TODO : pages
	m_pPrevBtn->SetVisible(false);
	m_pNextBtn->SetVisible(false);
}

const char * CCSClassMenu::GetName(void)
{
	return PANEL_CLASS;
}

void CCSClassMenu::Reset(void)
{
	CClassMenu::Reset();
}

MouseOverPanelButton * CCSClassMenu::CreateNewMouseOverPanelButton(vgui2::EditablePanel *panel)
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
	BaseClass::PerformLayout();
}

void CCSClassMenu::ApplySchemeSettings(vgui2::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}

void CCSClassMenu::OnCommand(const char* command)
{
	if (!strncmp(command, "VGUI_ClassMenu_Select ", 22))
	{
		OnSelectClass(command + 22);
		return;
	}
	else if (!strncmp(command, "VGUI_ClassMenu_SetTeam", 22))
	{
		TeamName team;
		if (command[22] == '\0')
			team = UNASSIGNED;
		else
			sscanf(command + 22, "%d", &team); //"Pistol 1"

		SetTeam(team);
		return;
	}
	else if (!Q_strcmp(command, "prevpage"))
	{
		//SetupPage(m_iCurrentPage - 1);
		return;
	}
	else if (!Q_strcmp(command, "nextpage"))
	{
		//SetupPage(m_iCurrentPage + 1);
		return;
	}
	return BaseClass::OnCommand(command);
}

void CCSClassMenu::OnSelectClass(const char* name)
{
	char buffer[64];
	sprintf(buffer, "joinclass %s", name);
	cl::gEngfuncs.pfnClientCmd(buffer);
	BaseClass::OnCommand("vguicancel");
}

void CCSClassMenu::UpdateGameMode()
{
	// TODO : Team Select
	m_pShowCTWeapon->SetVisible(false);
	m_pShowTERWeapon->SetVisible(false);
}