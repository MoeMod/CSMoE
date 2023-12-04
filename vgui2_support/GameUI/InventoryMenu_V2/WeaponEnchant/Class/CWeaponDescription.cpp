/***********************************************
	CWeaponDescription(From SME,Modified By MOE)
	Build in 2023/11
************************************************/

#include "GameUI/InventoryMenu_V2/WeaponEnchant/CWeaponEnchant.h"
#include "CWeaponDescription.h"
#include "GameUI/InventoryMenu_V2/Util/Util.h"

CWeaponDescription::CWeaponDescription(vgui2::Panel* parent, const char* panelName) : CCSBTEBulletinFrameImage(parent, panelName)
{
	m_pTitle = new Label(this, "WpnDescTitle", "");

	m_pTextLine = new ImagePanel(this, "WpnDescTextLine");
	m_pTextLine->SetImage(scheme()->GetImage("resource/itemline", false));
	m_pTextLine->SetShouldScaleImage(true);
	m_pTextLine->SetMouseInputEnabled(false);

	int w, t;
	for (int i = 0; i < 3; i++)
	{
		m_pGradeImage[i] = new ImagePanel(this, "WpnDescGrade");

		auto image = scheme()->GetImage(m_szRankImage[0], false);
		m_pGradeImage[i]->SetImage(image);
		if (image)
		{
			image->GetSize(w, t);
			m_pGradeImage[i]->SetSize(w, t);
		}
		m_pGradeImage[i]->SetPos(18 + 73 * i, 0);

		m_pGradeText[i] = new Label(this, "WpnDescGradeText", m_szModeText[i]);
		m_pGradeText[i]->GetContentSize(w, t);
		m_pGradeText[i]->SetBounds(5 + 73 * i, 0, max(73, w), 20);
		m_pGradeText[i]->SetContentAlignment(Label::a_center);
	}

	for (int i = 0; i < 2; i++)
	{
		m_pGradeLine[i] = new ImagePanel(this, "WpnDescGradeLine");
		m_pGradeLine[i]->SetImage(scheme()->GetImage("resource/itemline", false));
		m_pGradeLine[i]->SetShouldScaleImage(true);
		m_pGradeLine[i]->SetMouseInputEnabled(false);
		m_pGradeLine[i]->SetBounds(77 + 72 * i, 0, 1, 50);
		m_pGradeLine[i]->SetDrawColor(Color(255, 255, 255, 100));
	}

	m_pText = new RichText(this, "WpnDescDesc");
	m_pText->SetPaintBackgroundEnabled(false);
	m_pText->SetVerticalScrollbar(false);
	m_pText->InsertString("");

	m_pDescImage = new ImagePanel(this, "WpnDescImg");
	m_pDescImage->SetImage(scheme()->GetImage("gfx/vgui/basket/desc/cannonex", false));

	m_pFavOnImage = new ImagePanel(this, "WpnDescFavIcon");
	m_pFavOnImage->SetImage(scheme()->GetImage("resource/weaponfavorite/weapon_favorite_on", false));

	m_pFavOnImage->GetImage()->GetSize(w, t);
	m_pFavOnImage->SetSize(w, t);

	m_iGrade = 0;
}

void CWeaponDescription::PerformLayout()
{
	BaseClass::PerformLayout();

	m_pTitle->SetSize(200, 20);

	int w, t;
	m_pTitle->GetContentSize(w, t);
	m_pTextLine->SetSize(min(w, 200), 1);

	m_pText->SetSize(225, 300);
}
void CWeaponDescription::SetWeapon(const char* weapon, bool inuse)
{
	if (!weapon)
	{
		SetVisible(false);
		return;
	}
	const char* szCsoWpn = CheckCSOName(g_szCsoWpnName, weapon);

	m_pTitle->SetText(GetWeaponLangName(szCsoWpn));
	m_pFavOnImage->SetVisible(inuse);

	char szBuffer[64];
	Q_snprintf(szBuffer, sizeof(szBuffer), "#CSO_Item_Desc_%s", szCsoWpn);

	m_pText->SetText("");
	wchar_t* pText = localize()->Find(szBuffer);

	if (!pText)
	{
		Q_snprintf(szBuffer, 63, "#Cstrike_%sDescription", szCsoWpn);
		pText = localize()->Find(szBuffer);
	}
	if (pText)
		m_pText->InsertString(pText);
	else
		m_pText->InsertString("#CSMOE_InventoryMenu_CannotFind_Desc");

	Q_snprintf(szBuffer, sizeof(szBuffer), "gfx/vgui/basket/desc/%s", weapon);
	m_pDescImage->SetImage(scheme()->GetImage(szBuffer, false));

	m_iGrade = 0;

	for (int i = 0; i < 3; i++)
	{
		m_pGradeImage[i]->SetVisible(false);
		m_pGradeText[i]->SetVisible(false);
	}

	for (int i = 0; i < 2; i++)
		m_pGradeLine[i]->SetVisible(false);

	SetVisible(true);
}

void CWeaponDescription::ApplySchemeSettings(vgui2::IScheme* pScheme)
{
	SetBgColor(Color(0, 0, 0, 0));

	std::string colorkey = "";
	Color color;
	switch (m_iGrade)
	{
	case 1:
		color = Color(255, 255, 255, 255);
		colorkey = "_grey";
		break;
	case 2:
		color = Color(120, 170, 150, 255);
		colorkey = "_green";
		break;
	case 3:
		color = Color(90, 175, 255, 255);
		colorkey = "_blue";
		break;
	case 4:
		color = Color(255, 110, 110, 255);
		colorkey = "_red";
		break;
	case 5:
		color = Color(255, 205, 20, 255);
		colorkey = "_gold";
		break;
	case 6:
		color = Color(255, 0, 220, 255);
		colorkey = "_purple";
		break;
	default:
		color = Color(255, 255, 255, 255);
		colorkey = "_grey";
	}

	if (colorkey.length())
	{
		const char* begin = "resource/control/floatingmenu2/pulldown";
		const char* middle[] = { "_top", "_center", "_bottom" };
		const char* end[] = { "_left@n", "_center@n", "_right@n" };

		std::string temp = begin;
		temp += colorkey;
		temp += middle[0];
		temp += end[0];

		if (vgui2::IImage* pImage = scheme()->GetImage(temp.c_str(), true))
		{
			m_bImageBackground = true;
			m_pTopBackground[0] = pImage;
			for (int i = 0; i < 3; i++)
			{
				temp = begin;
				temp += colorkey;
				temp += middle[0];
				temp += end[i];

				if (i != 0)
					m_pTopBackground[i] = scheme()->GetImage(temp.c_str(), true);

				temp = begin;
				temp += colorkey;
				temp += middle[1];
				temp += end[i];

				m_pCenterBackground[i] = scheme()->GetImage(temp.c_str(), true);

				temp = begin;
				temp += colorkey;
				temp += middle[2];
				temp += end[i];

				m_pBottomBackground[i] = scheme()->GetImage(temp.c_str(), true);
			}
		}
		else
			m_bImageBackground = false;
	}

	if (!m_bImageBackground)
	{
		if (vgui2::IImage* pImage = scheme()->GetImage(pScheme->GetResourceString("TextEntry/DarkAlphaTopLeft"), true))
		{
			m_bImageBackground = true;
			m_pTopBackground[0] = pImage;
			m_pTopBackground[1] = scheme()->GetImage(pScheme->GetResourceString("TextEntry/DarkAlphaTopCenter"), true);
			m_pTopBackground[2] = scheme()->GetImage(pScheme->GetResourceString("TextEntry/DarkAlphaTopRight"), true);
			m_pCenterBackground[0] = scheme()->GetImage(pScheme->GetResourceString("TextEntry/DarkAlphaMiddleLeft"), true);
			m_pCenterBackground[1] = scheme()->GetImage(pScheme->GetResourceString("TextEntry/DarkAlphaMiddleCenter"), true);
			m_pCenterBackground[2] = scheme()->GetImage(pScheme->GetResourceString("TextEntry/DarkAlphaMiddleRight"), true);
			m_pBottomBackground[0] = scheme()->GetImage(pScheme->GetResourceString("TextEntry/DarkAlphaBottomLeft"), true);
			m_pBottomBackground[1] = scheme()->GetImage(pScheme->GetResourceString("TextEntry/DarkAlphaBottomCenter"), true);
			m_pBottomBackground[2] = scheme()->GetImage(pScheme->GetResourceString("TextEntry/DarkAlphaBottomRight"), true);
		}
		else
			m_bImageBackground = false;
	}

	m_pTitle->SetFgColor(color);
	m_pTextLine->SetDrawColor(color);

	ReCalcBounds();
}
void CWeaponDescription::ReCalcBounds()
{
	int total = 30;
	if (m_pGradeImage[0]->IsVisible())
		total += 65;

	int w, t;
	m_pTitle->GetContentSize(w, t);
	m_pTextLine->SetSize(min(w, 200), 1);

	t *= m_pText->GetNumLines() + 1;

	int tall = min(t, 300);
	m_pText->SetSize(225, tall);
	total += tall;

	int tt = 0;
	if (m_pDescImage->GetImage())
	{
		m_pDescImage->GetImage()->GetSize(w, tt);
		m_pDescImage->SetSize(w, tt);

		if (total == GetTall() && tt == m_pDescImage->GetTall())
			return;

		total += tt;
	}
	else
		m_pDescImage->SetVisible(false);

	total += 10;
	SetBounds(520, 530 - total, 235, total);

	total -= 10 + tt;
	m_pDescImage->SetPos(5, total);

	total -= tall;
	m_pText->SetPos(5, total);

	if (m_pGradeImage[0]->IsVisible())
	{
		total -= 60;

		int x, y;
		for (int i = 0; i < 3; i++)
		{
			m_pGradeImage[i]->GetPos(x, y);
			m_pGradeImage[i]->SetPos(x, total);

			m_pGradeText[i]->GetPos(x, y);
			m_pGradeText[i]->SetPos(x, total + 30);
		}

		for (int i = 0; i < 2; i++)
		{
			m_pGradeLine[i]->GetPos(x, y);
			m_pGradeLine[i]->SetPos(x, total);
		}
	}

	total -= 25;

	if (m_pFavOnImage->IsVisible())
	{
		m_pFavOnImage->GetSize(w, t);
		m_pFavOnImage->SetPos(10, total + 10 - t / 2);
		m_pTitle->SetPos(w + 12, total);

		total += 17;
		m_pTextLine->SetPos(w + 12, total);
	}
	else
	{
		m_pTitle->SetPos(10, total);

		total += 17;
		m_pTextLine->SetPos(10, total);
	}
}