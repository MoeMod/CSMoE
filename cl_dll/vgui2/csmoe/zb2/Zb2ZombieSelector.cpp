#include "hud.h"
#include "Zb2ZombieSelector.h"
#include "CBaseViewport.h"

#include <FileSystem.h>

#include <vgui/IScheme.h>
#include <vgui/IVGui.h>
#include <vgui/ISurface.h>
#include <vgui2/src/VPanel.h>

#include <vgui_controls/Tooltip.h>

#include <vgui/IScheme.h>
#include <vgui_controls/Button.h>
#include <vgui/KeyCode.h>
#include <FileSystem.h>


using namespace vgui2;

CZb2ZombieKeeper::CZb2ZombieKeeper(void) : Frame(NULL, PANEL_ZOMBIEKEEPER)
{
	SetScheme("ClientScheme");

	SetMoveable(true);
	SetSizeable(false);
	SetProportional(false);

	SetTitleBarVisible(false);
	SetAutoDelete(false);

	int screenWide, screenTall;
	surface()->GetScreenSize(screenWide, screenTall);
	int wide, tall;
	GetSize(wide, tall);
	SetPos((screenWide / 2) - wide / 2, (screenTall / 2) - tall / 2);

	LoadControlSettings("Resource/popup_zb2zombieselector.res", "GAME");

	FindControl<Label>("TitleLabel")->SetScheme("BaseUI");
	SetTitle("#CSO_ZombiSelect", true);

	for (size_t i = 0; i < 10; i++)
	{
		m_ZombieKeeperSlot[i].m_BtnImagePanel = FindControl<SignalImagePanel>((std::string("BtnImagePanel_n_") + std::to_string(i + 1)).c_str());

		m_ZombieKeeperSlot[i].m_BtnImagePanel->GetTooltip()->SetText("");
		m_ZombieKeeperSlot[i].m_BtnImagePanel->SetImage("");

		//Create temporary objects.(Not in res!)
		m_ZombieKeeperSlot[i].m_pLockedImageBg = new ImagePanel(this, "LockedImageBg");
		m_ZombieKeeperSlot[i].m_pLockedImageBg->SetVisible(false);
		m_ZombieKeeperSlot[i].m_pLockedImageBg->SetEnabled(false);

		m_ZombieKeeperSlot[i].m_pLockedImage = new ImagePanel(this, "LockedImage");
		m_ZombieKeeperSlot[i].m_pLockedImage->SetVisible(false);
		m_ZombieKeeperSlot[i].m_pLockedImage->SetEnabled(false);

		m_ZombieKeeperSlot[i].m_pLockedLevelBar = new ImagePanel(this, "LockedLevelBar");
		m_ZombieKeeperSlot[i].m_pLockedLevelBar->SetVisible(false);
		m_ZombieKeeperSlot[i].m_pLockedLevelBar->SetEnabled(false);

		m_ZombieKeeperSlot[i].m_pLockedLevel = new Label(this, "LockedLevel", "");
		m_ZombieKeeperSlot[i].m_pLockedLevel->SetVisible(false);
		m_ZombieKeeperSlot[i].m_pLockedLevel->SetEnabled(true);

		m_ZombieKeeperSlot[i].m_pBannedImage = new ImagePanel(this, "BannedImage");
		m_ZombieKeeperSlot[i].m_pBannedImage->SetVisible(false);
		m_ZombieKeeperSlot[i].m_pBannedImage->SetEnabled(false);

		//Get Original Pos and Size
		int iWide, iHeight, iX, iY;
		m_ZombieKeeperSlot[i].m_BtnImagePanel->GetSize(iWide, iHeight);
		m_ZombieKeeperSlot[i].m_BtnImagePanel->GetPos(iX, iY);

		m_ZombieKeeperSlot[i].m_pLockedImageBg->SetSize(iWide, iHeight);
		m_ZombieKeeperSlot[i].m_pLockedImageBg->SetPos(iX, iY);
		m_ZombieKeeperSlot[i].m_pLockedImageBg->SetImage("resource/zombiez/zombie_lock_bg");

		int iLockerWide = 22, iLockerHeight = 23;

		int iLockerX = iX + (iWide - iLockerWide) / 2;
		int iLockerY = iY + (iHeight - iLockerHeight) / 2;

		m_ZombieKeeperSlot[i].m_pLockedImage->SetSize(iLockerWide, iLockerHeight);
		m_ZombieKeeperSlot[i].m_pLockedImage->SetPos(iLockerX, iLockerY);
		m_ZombieKeeperSlot[i].m_pLockedImage->SetImage("resource/zombiez/level_lock_l");

		int iLevelBarWide = 64, iLevelBarHeight = 15;

		int iLevelBarX = iX + (iWide - iLevelBarWide) / 2;
		int iLevelBarY = iY + (iHeight - iLevelBarHeight);

		m_ZombieKeeperSlot[i].m_pLockedLevelBar->SetSize(iLevelBarWide, iLevelBarHeight);
		m_ZombieKeeperSlot[i].m_pLockedLevelBar->SetPos(iLevelBarX, iLevelBarY);
		m_ZombieKeeperSlot[i].m_pLockedLevelBar->SetImage("resource/zombiez/level_lock_bg");


		m_ZombieKeeperSlot[i].m_pLockedLevel->SetPos(iLevelBarX + iLevelBarWide / 3, iLevelBarY);
		m_ZombieKeeperSlot[i].m_pLockedLevel->SetFont(scheme()->GetIScheme(m_ZombieKeeperSlot[i].m_pLockedLevel->GetScheme())->GetFont("BrightControlText", IsProportional()));
		m_ZombieKeeperSlot[i].m_pLockedLevel->SetFgColor({ 255,99,71,255 });

		int iBannedImageWide = 154, iBannedImageHeight = 53;

		int iBannedImageX = iX + (iWide - iBannedImageWide) / 2;
		int iBannedImageY = iY + (iHeight - iBannedImageHeight) / 2;

		m_ZombieKeeperSlot[i].m_pBannedImage->SetSize(iWide, iHeight);
		m_ZombieKeeperSlot[i].m_pBannedImage->SetPos(iBannedImageX, iBannedImageY);
		m_ZombieKeeperSlot[i].m_pBannedImage->SetImage("gfx/vgui/basket/cannotuse");


	}
	m_ZombieKeeperSlot[10].m_BtnControlPanel[0] = FindControl<SignalImagePanel>("prevBtnPanel");
	m_ZombieKeeperSlot[11].m_BtnControlPanel[1] = FindControl<SignalImagePanel>("nextBtnPanel");


	m_ZombieKeeperSlot[12].m_BtnControlPanel[2] = FindControl<SignalImagePanel>("closeBtnPanel");
	m_ZombieKeeperSlot[12].m_BtnControlPanel[2]->GetTooltip()->SetText("#CSO_ClosePopup");
	m_ZombieKeeperSlot[12].m_BtnControlPanel[2]->SetImage("resource/hud/zombie/zbselect_close");

	m_ZombieKeeperSlot[13].m_BtnControlPanelText[0] = FindControl<Label>("PrevLabel");
	m_ZombieKeeperSlot[14].m_BtnControlPanelText[1] = FindControl<Label>("NextLabel");
	m_ZombieKeeperSlot[15].m_BtnControlPanelText[2] = FindControl<Label>("CloseLabel");

	m_ZombieKeeperSlot[16].m_pChooseTime = FindControl<Label>("TimerLabel");

	m_ZombieKeeperSlot[13].m_BtnControlPanelText[0]->SetFont(scheme()->GetIScheme(m_ZombieKeeperSlot[13].m_BtnControlPanelText[0]->GetScheme())->GetFont("BrightControlText", IsProportional()));
	m_ZombieKeeperSlot[13].m_BtnControlPanelText[0]->SetText("#CSO_Previous_Page");
	m_ZombieKeeperSlot[13].m_BtnControlPanelText[0]->SizeToContents();

	m_ZombieKeeperSlot[14].m_BtnControlPanelText[1]->SetFont(scheme()->GetIScheme(m_ZombieKeeperSlot[14].m_BtnControlPanelText[1]->GetScheme())->GetFont("BrightControlText", IsProportional()));
	m_ZombieKeeperSlot[14].m_BtnControlPanelText[1]->SetText("#CSO_Next_Page");
	m_ZombieKeeperSlot[14].m_BtnControlPanelText[1]->SizeToContents();

	m_ZombieKeeperSlot[15].m_BtnControlPanelText[2]->SetFont(scheme()->GetIScheme(m_ZombieKeeperSlot[15].m_BtnControlPanelText[2]->GetScheme())->GetFont("BrightControlText", IsProportional()));
	m_ZombieKeeperSlot[15].m_BtnControlPanelText[2]->SetText("#CSO_ClosePopup");
	m_ZombieKeeperSlot[15].m_BtnControlPanelText[2]->SizeToContents();

	m_ZombieKeeperSlot[16].m_pChooseTime->SetFont(scheme()->GetIScheme(m_ZombieKeeperSlot[16].m_pChooseTime->GetScheme())->GetFont("BrightControlText", IsProportional()));
	m_ZombieKeeperSlot[16].m_pChooseTime->SetText("#CSO_ClosePopup");
	m_ZombieKeeperSlot[16].m_pChooseTime->SizeToContents();


}

CZb2ZombieKeeper::~CZb2ZombieKeeper(void)
{
}

void CZb2ZombieKeeper::ApplySchemeSettings(IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	auto pSchemeBaseUI = vgui2::scheme()->GetIScheme(vgui2::scheme()->GetScheme("BaseUI"));
	for (size_t i = 0; i < 10; i++)
	{
		if (m_ZombieKeeperSlot[i].m_BtnImagePanel)
		{
			m_ZombieKeeperSlot[i].m_BtnImagePanel->GetTooltip()->SetBackgroundSkin(pSchemeBaseUI, "DarkAlpha");

		}
	}
	m_ZombieKeeperSlot[10].m_BtnControlPanel[0]->GetTooltip()->SetBackgroundSkin(pSchemeBaseUI, "DarkAlpha");
	m_ZombieKeeperSlot[11].m_BtnControlPanel[1]->GetTooltip()->SetBackgroundSkin(pSchemeBaseUI, "DarkAlpha");
	m_ZombieKeeperSlot[12].m_BtnControlPanel[2]->GetTooltip()->SetBackgroundSkin(pSchemeBaseUI, "DarkAlpha");


}
void CZb2ZombieKeeper::PerformLayout(void)
{
	BaseClass::PerformLayout();

	for (size_t i = 0; i < 10; i++)
	{
		if (m_ZombieKeeperSlot[i].m_pLockedLevel)
		{
			m_ZombieKeeperSlot[i].m_pLockedLevel->SetFgColor({ 255,99,71,255 });
		}
	}
}
void CZb2ZombieKeeper::SetVisible(bool bShow)
{
	BaseClass::SetVisible(bShow);
}

void CZb2ZombieKeeper::ShowPanel(bool bShow)
{
	if (BaseClass::IsVisible() == bShow)
		return;

	if (bShow)
	{
		Activate();
		SetMouseInputEnabled(true);
	}
	else
	{
		SetVisible(false);
		SetMouseInputEnabled(false);
	}
}

void CZb2ZombieKeeper::OnClose(void)
{
	BaseClass::OnClose();
}

Panel* CZb2ZombieKeeper::CreateControlByName(const char* controlName)
{
	if (!Q_stricmp("SignalImagePanel", controlName))
	{
		auto pPanel = new SignalImagePanel(this, controlName);
		return pPanel;
	}
	else if (!Q_stricmp("RoundPanel", controlName))
	{
		auto pPanel = new RoundPanel(this, controlName, RoundPanel::TOOLTIP_LIST);
		return pPanel;
	}
	else if (!Q_stricmp("ImagePanel", controlName))
	{
		auto pPanel = new ImagePanel(this, controlName);
		return pPanel;
	}
	else
		return BaseClass::CreateControlByName(controlName);

}

void CZb2ZombieKeeper::OnCommand(const char* command)
{
	if (!strncmp(command, "VGUI_BTE_Zb_Select_Zombie", 20))
	{
		char buffer[64];
		sprintf(buffer, "BTE_Zb_Select_Zombie%s", command + 20);
		cl::gEngfuncs.pfnClientCmd(buffer);

		BaseClass::OnCommand("CloseModal");
		OnClose();
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
	else if (!Q_strcmp(command, "closepage"))
	{
		BaseClass::OnCommand("CloseModal");
		OnClose();
		return;
	}

	BaseClass::OnCommand(command);
}

void CZb2ZombieKeeper::OnSetFocus()
{
	BaseClass::OnSetFocus();
}

void CZb2ZombieKeeper::SetupPage(size_t iPage)
{
	int totalpages = ((23 + 1) - 1) / 10 + 1;
	if (iPage <= 0) iPage = 0;
	else if (iPage >= totalpages)
		iPage = totalpages - 1;

	m_iCurrentPage = iPage;

	// page buttons

	m_ZombieKeeperSlot[10].m_BtnControlPanel[0] = FindControl<SignalImagePanel>("prevBtnPanel");
	m_ZombieKeeperSlot[10].m_BtnControlPanel[0]->SetEnabled(iPage != 0);
	m_ZombieKeeperSlot[10].m_BtnControlPanel[0]->SetImage(iPage != 0 ? "resource/hud/zombie/zbselect_prev_nor" : "resource/hud/zombie/zbselect_prev_dim");
	m_ZombieKeeperSlot[10].m_BtnControlPanel[0]->GetTooltip()->SetText(iPage != 0 ? "#CSO_Previous_Page" : "#CSO_Quest_Not_Available");

	m_ZombieKeeperSlot[11].m_BtnControlPanel[1] = FindControl<SignalImagePanel>("nextBtnPanel");
	m_ZombieKeeperSlot[11].m_BtnControlPanel[1]->SetEnabled(m_iCurrentPage != totalpages - 1);
	m_ZombieKeeperSlot[11].m_BtnControlPanel[1]->SetImage(m_iCurrentPage != totalpages - 1 ? "resource/hud/zombie/zbselect_next_nor" : "resource/hud/zombie/zbselect_next_dim");
	m_ZombieKeeperSlot[11].m_BtnControlPanel[1]->GetTooltip()->SetText(m_iCurrentPage != totalpages - 1 ? "#CSO_Next_Page" : "#CSO_Quest_Not_Available");

	m_ZombieKeeperSlot[12].m_BtnControlPanel[2]->SetVisible(true);
	m_ZombieKeeperSlot[12].m_BtnControlPanel[2]->SetEnabled(true);

	for (int i = 0; i < 10; ++i)
	{
		int iElement = m_iCurrentPage * 10 + i;
		if (iElement >= 23)
		{
			m_ZombieKeeperSlot[i].m_BtnImagePanel->SetVisible(false);
			m_ZombieKeeperSlot[i].m_BtnImagePanel->GetTooltip()->SetText("");

			ReSetLockSlot(i);
			ReSetBanSlot(i);
		}
		else
		{
			ReSetLockSlot(i);
			ReSetBanSlot(i);

			const char* SzDesc = KeeperClassInfo[iElement].desc.c_str();
			const int SzLevel = KeeperClassInfo[iElement].level;

			char buffer[64];
			sprintf(buffer, "VGUI_BTE_Zb_Select_Zombie%d", iElement);


			m_ZombieKeeperSlot[i].m_BtnImagePanel->GetTooltip()->SetText(SzDesc);
			m_ZombieKeeperSlot[i].m_BtnImagePanel->SetImage((std::string("resource/zombi/zombietype_") + KeeperClassInfo[iElement].name).c_str());
			m_ZombieKeeperSlot[i].m_BtnImagePanel->SetVisible(true);
			m_ZombieKeeperSlot[i].m_BtnImagePanel->SetEnabled(true);
			m_ZombieKeeperSlot[i].m_BtnImagePanel->SetCommand(buffer);

			if (SzLevel > 40)
			{
				SetBanSlot(i);
			}
			else
			{
				if ((cl::gHUD.IsZombieMod() && (cl::gHUD.m_iModRunning == MOD_ZBZ)))
				{
					if (SzLevel > cl::gHUD.m_iZlevel)
					{
						SetLockSlot(i, SzLevel);
					}
				}
			}
		}
	}
}
void CZb2ZombieKeeper::SetBanSlot(int i)
{
	if (m_ZombieKeeperSlot[i].m_pBannedImage)
		m_ZombieKeeperSlot[i].m_pBannedImage->SetVisible(true);
	if (m_ZombieKeeperSlot[i].m_pLockedImageBg)
		m_ZombieKeeperSlot[i].m_pLockedImageBg->SetVisible(true);

	m_ZombieKeeperSlot[i].m_BtnImagePanel->SetEnabled(false);
}
void CZb2ZombieKeeper::ReSetBanSlot(int i)
{
	if (m_ZombieKeeperSlot[i].m_pBannedImage)
		m_ZombieKeeperSlot[i].m_pBannedImage->SetVisible(false);
	if (m_ZombieKeeperSlot[i].m_pLockedImageBg)
		m_ZombieKeeperSlot[i].m_pLockedImageBg->SetVisible(false);

	m_ZombieKeeperSlot[i].m_BtnImagePanel->SetEnabled(true);
}
void CZb2ZombieKeeper::SetLockSlot(int i, int SzLevel)
{
	if (m_ZombieKeeperSlot[i].m_pLockedImageBg)
		m_ZombieKeeperSlot[i].m_pLockedImageBg->SetVisible(true);
	if (m_ZombieKeeperSlot[i].m_pLockedImage)
		m_ZombieKeeperSlot[i].m_pLockedImage->SetVisible(true);
	if (m_ZombieKeeperSlot[i].m_pLockedLevelBar)
		m_ZombieKeeperSlot[i].m_pLockedLevelBar->SetVisible(true);
	if (m_ZombieKeeperSlot[i].m_pLockedLevel)
	{

		char SzLevelText[32]; sprintf(SzLevelText, "Lv.%d", SzLevel);
		m_ZombieKeeperSlot[i].m_pLockedLevel->SetText(SzLevelText);
		m_ZombieKeeperSlot[i].m_pLockedLevel->SetFgColor({ 255,99,71,255 });
		m_ZombieKeeperSlot[i].m_pLockedLevel->SizeToContents();

		m_ZombieKeeperSlot[i].m_pLockedLevel->SetVisible(true);

	}
}
void CZb2ZombieKeeper::ReSetLockSlot(int i)
{
	if (m_ZombieKeeperSlot[i].m_pLockedImageBg)
		m_ZombieKeeperSlot[i].m_pLockedImageBg->SetVisible(false);
	if (m_ZombieKeeperSlot[i].m_pLockedImage)
		m_ZombieKeeperSlot[i].m_pLockedImage->SetVisible(false);
	if (m_ZombieKeeperSlot[i].m_pLockedLevelBar)
		m_ZombieKeeperSlot[i].m_pLockedLevelBar->SetVisible(false);
	if (m_ZombieKeeperSlot[i].m_pLockedLevel)
	{
		m_ZombieKeeperSlot[i].m_pLockedLevel->SetVisible(false);

		m_ZombieKeeperSlot[i].m_pLockedLevel->SetText("");
		m_ZombieKeeperSlot[i].m_pLockedLevel->SizeToContents();

	}
}
void CZb2ZombieKeeper::PaintBackground()
{
	BaseClass::PaintBackground();
}

void CZb2ZombieKeeper::OnThink()
{
	BaseClass::OnThink();

	char SzTimer[64]; sprintf(SzTimer, "Time Left:%wss", std::to_wstring((int)(cl::gHUD.m_flZombieSelectTime - cl::gHUD.m_flTime)).c_str());
	m_ZombieKeeperSlot[16].m_pChooseTime->SetText(SzTimer);
	m_ZombieKeeperSlot[16].m_pChooseTime->SizeToContents();

	if ((int)(cl::gHUD.m_flZombieSelectTime - cl::gHUD.m_flTime) <= 0)
	{
		BaseClass::OnCommand("CloseModal");
		OnClose();
		return;
	}

}