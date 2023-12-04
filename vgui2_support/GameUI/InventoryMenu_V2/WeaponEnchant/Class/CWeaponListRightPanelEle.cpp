/***********************************************
	CWeaponListRightPanelEle
	Build in 2023/11
************************************************/

#include "GameUI/InventoryMenu_V2/WeaponEnchant/CWeaponEnchant.h"
#include "CWeaponListRightPanelEle.h"
#include "GameUI/InventoryMenu_V2/Util/Util.h"

CWeaponListRightPanelEle::CWeaponListRightPanelEle(Panel* parent, const char* name) : ImagePanel(parent, name)
{
	SetImage(scheme()->GetImage("resource/itembox", false));
	SetShouldScaleImage(true);

	int textwide, texttall;
	int textwide2, texttall2;

	m_pText = new Label(this, "NameLabel", "Weapon Name");
	m_pText->SetPaintBackgroundEnabled(false);
	m_pText->SetBounds(10, 80, 160, 20);
	m_pText->GetContentSize(textwide, texttall);
	m_pText->SetMouseInputEnabled(false);

	m_pText2 = new Label(this, "ActiveLabel", "#CSO_Item_Active_Format");
	m_pText2->SetPaintBackgroundEnabled(false);
	m_pText2->SetSize(150, 20);
	m_pText2->SetFgColor(Color(130, 130, 130, 130));
	m_pText2->SetMouseInputEnabled(false);
	m_pText2->GetContentSize(textwide2, texttall2);
	m_pText2->SetPos(240 - textwide2, 65);

	m_pItemBackground = new ImagePanel(this, "WpnBackgroundImg");
	m_pItemBackground->SetImage(scheme()->GetImage("gfx/ui/panel/basket_blank_slot", false));
	m_pItemBackground->SetBounds(0, 0, 255, 81);
	m_pItemBackground->SetShouldScaleImage(true);
	m_pItemBackground->SetMouseInputEnabled(false);

	m_pWeaponImage = new ImagePanel(this, "WpnWeaponImg");
	m_pWeaponImage->SetImage(scheme()->GetImage("gfx/vgui/m4a1", false));
	m_pWeaponImage->SetBounds(0, 0, 235, 81);
	m_pWeaponImage->SetMouseInputEnabled(false);

	m_pSelectedBackground = new ImagePanel(this, "WpnSelectedImg");
	m_pSelectedBackground->SetImage(scheme()->GetImage("resource/set/itemboxselect", false));
	m_pSelectedBackground->SetShouldScaleImage(true);
	m_pSelectedBackground->SetBounds(0, 0, 255, 115);
	m_pSelectedBackground->SetVisible(false);
	m_pSelectedBackground->SetMouseInputEnabled(false);

	m_pTextLine = new ImagePanel(this, "WpnTextLine");
	m_pTextLine->SetImage(scheme()->GetImage("resource/itemline", false));
	m_pTextLine->SetBounds(10, 97, min(textwide, 150), 1);
	m_pTextLine->SetShouldScaleImage(true);
	m_pTextLine->SetMouseInputEnabled(false);

	m_pSelect = new Button(this, "WpnSelectBtn", "#CSO_Select");
	m_pSelect->SetContentAlignment(Label::a_center);
	m_pSelect->SetBounds(180, 85, 60, 24);
	m_pSelect->SetContentAlignment(Label::a_center);
	m_pSelect->AddActionSignalTarget(parent);
}

void CWeaponListRightPanelEle::SetId(int id)
{
	char szCommand[20];
	sprintf(szCommand, "OnWeaponApply_%d", id);
	m_pSelect->SetCommand(szCommand);

	int x = 760 + 10 + (id & 1) * 260;
	int y = 50 + (id >> 1) * 120;

	y += 20; // offset for butoons

	SetBounds(x, y, 255, 115);
}

void CWeaponListRightPanelEle::SetWeapon(char* szWpnName,int iCurrentLevel)
{
	bool bNeedModifiy = false;

	char buffer[128];
	sprintf(buffer, "gfx/vgui/basket/%s", szWpnName);
	IImage* pImage = scheme()->GetImage(buffer, false);
	m_pWeaponImage->SetImage(pImage);

	int w, h;
	pImage->GetSize(w, h);
	m_pWeaponImage->SetPos((235 - w) / 2, (81 - h) / 2);

	if (Q_strstr(szWpnName, "_6") || Q_strstr(szWpnName, "_8"))
	{
		*Q_strrchr(szWpnName, '_') = '\0';

		if(iCurrentLevel > 0)
			bNeedModifiy = true;
	}
	

	//Weapon Name...
	int textwide, texttall;
	if (!bNeedModifiy)
	{
		m_pText->SetText(GetWeaponLangName(CheckCSOName(g_szCsoWpnName, szWpnName)));
		m_pText->SetFgColor(Color(255,255,255,255));
	}
	else
	{
		const wchar_t* wstrLevelName = GetWeaponLangName(CheckCSOName(g_szCsoWpnName, szWpnName));
		wchar_t wstrWeaponName[128];

		V_snwprintf(wstrWeaponName, sizeof(wstrWeaponName), L"(+%d) %s", iCurrentLevel, wstrLevelName);
		//_swprintf(wstrWeaponName, L"(+%d) %s", iCurrentLevel, wstrLevelName);

		m_pText->SetText(wstrWeaponName);
		m_pText->InvalidateLayout(true,true);

		Color cColor = dynamic_cast<CWeaponEnchant*>(GetParent())->GetLeftPanel()->UpdateEnchantColor(iCurrentLevel);;
		if (cColor.r() != cColor.g() != cColor.b() != 0)
		{
			m_pText->SetFgColor(cColor);
		}
	}
	m_pText->GetContentSize(textwide, texttall);
	m_pText->GetSize(w, h);
	m_pTextLine->SetBounds(10, 97, min(textwide, w), 1);
}

//if 'bEditmode' is 'true' We sort all similar weapons...
void CWeaponListRightPanelEle::SetSelected(bool bSelected)
{
	if (bSelected)
	{
		m_pSelectedBackground->SetVisible(true);
		m_pText2->SetText("#CSO_Item_Active_Format");
		m_pSelect->SetText("#CSO_UnApply_Btn");
	}
	else
	{
		m_pSelectedBackground->SetVisible(false);
		m_pText2->SetText("");
		m_pSelect->SetText("#CSO_Use");
	}
}

void CWeaponListRightPanelEle::OnMousePressed(vgui2::MouseCode code)
{
	if (code != vgui2::MOUSE_LEFT)
		return;

	const char* pCommand = m_pSelect->GetCommand()->GetString("command", "");

	if (!pCommand[0])
		return;

	int id = atoi(pCommand + 14);
	PostActionSignal(new KeyValues("ElementSelected", "index", id));
}

//If Double pressed ,we certainly apply it!
void CWeaponListRightPanelEle::OnMouseDoublePressed(vgui2::MouseCode code)
{
	if (code == vgui2::MouseCode::MOUSE_LEFT)
	{
		m_pSelect->DoClick();
		ReleaseIcon();
	}
}
void CWeaponListRightPanelEle::ReleaseIcon()
{
	PostActionSignal(new KeyValues("ElementReleased"));
}
void CWeaponListRightPanelEle::ApplySchemeSettings(vgui2::IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_pText2->SetFgColor(Color(255, 150, 50, 255));
}

void CWeaponListRightPanelEleDrag::OnThink()
{
	int iCursorX, iCursorY;
	input()->GetCursorPos(iCursorX, iCursorY);

	int iX, iY;
	GetParent()->GetPos(iX, iY);
	iCursorX -= iX + 10;
	iCursorY -= iY + 10;

	int w, t;
	GetSize(w, t);
	iCursorX -= w / 2;
	iCursorY -= t / 2;

	SetPos(iCursorX, iCursorY);

	if (!input()->IsMouseDown(vgui2::MOUSE_LEFT))
	{
		PostActionSignal(new KeyValues("ElementReleased"));
	}
}
void CWeaponListRightPanelEle::ClickToSelect()
{
	m_pSelect->DoClick();
}