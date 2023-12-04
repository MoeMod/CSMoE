/***********************************************
	CWeaponListLeftPanel
	Build in 2023/11
************************************************/

#include "GameUI/EngineInterface.h"

#include "GameUI/InventoryMenu_V2/WeaponEnchant/CWeaponEnchant.h"
#include "CWeaponListLeftPanel.h"
#include "GameUI/InventoryMenu_V2/Util/Util.h"

#include "../3rdparty/csv-parser/csv.hpp"

//Undef it to avoid multi-def bugs...
#ifdef PropertySheet
#undef PropertySheet
#endif
//End of undef...

#include <mathlib/mathlib.h>
#include <nlohmann/json.hpp>

namespace vgui2 {
	extern cl_enginefunc_t gEngfuncs;
}


CWeaponListLeftPanel::CWeaponListLeftPanel(vgui2::Panel* parent, const char* panelName) : vgui2::Panel(parent, panelName)
{
	SetScheme("ClientScheme");

	m_pCraftBackground = new ImagePanel(this, "CraftsmanBackground");
	m_pCraftBackground->SetImage(scheme()->GetImage("Resource/Control/Basket/Basket_Blank_Slot", true));
	m_pCraftBackground->SetShouldScaleImage(true);
	m_pCraftBackground->SetScaleAmount(0);

	m_pCraftImage = new ImagePanel(this, "Craftsman");
	m_pCraftImage->SetImage(scheme()->GetImage("Resource/NPC/Henry", true));
	m_pCraftImage->SetShouldCenterImage(true);
	m_pCraftImage->SetShouldScaleImage(true);

	m_pCraftName = new Label(this, "CraftsmanName", "#CSO_Henry_Craft");
	m_pCraftDesc = new Label(this, "CraftsmanDesc", "#CSMOE_InventoryMenu_Enchant");

	m_pSearchBar = new CSearchBar(this, "WpnSearch");
	m_pSearchBar->SetAllowNonAsciiCharacters(true);

	m_pSearchText = new Label(this, "WpnSearchText", "#CSO_Search");

	m_pSearchTip = new Label(this, "WpnSearchTip", "#CSMOE_InventoryMenu_SearchTip");
	m_pSearchTip->SetContentAlignment(Label::a_center);

	m_pWeaponBackground = new ImagePanel(this, "WpnBackgroundImage");
	m_pWeaponBackground->SetImage(scheme()->GetImage("Resource/Itemfarm/SlotM", true));
	m_pWeaponBackground->SetShouldScaleImage(true);
	m_pWeaponBackground->SetScaleAmount(0);

	m_pWeaponImage = new ImagePanel(this, "WpnImage");
	m_pWeaponImage->SetShouldScaleImage(true);
	m_pWeaponImage->SetShouldCenterImage(true);

	IScheme* pScheme = scheme()->GetIScheme(GetScheme());
	HFont hFont = pScheme->GetFont("Default20", false);

	m_pWeaponLabelBG = new ImagePanel(this, "WpnLabelBG");
	m_pWeaponLabelBG->SetImage(scheme()->GetImage("Resource/Itemfarm/ResultBG", true));
	m_pWeaponLabelBG->SetShouldScaleImage(true);
	m_pWeaponLabelBG->SetShouldCenterImage(false);
	m_pWeaponLabelBG->SetScaleAmount(0);

	m_pWeaponLabel = new Label(this, "WpnLabel", "#CSO_EnhanceWeapon_Label");
	m_pWeaponLabel->SetContentAlignment(Label::a_center);
	m_pWeaponLabel->SetFont(hFont);

	hFont = pScheme->GetFont("Default16", false);

	m_pRemainingChances = new Label(this, "WpnRemainingTimes", "#CSO_RemainEnhanceCount");
	m_pRemainingChances->SetContentAlignment(Label::a_center);
	m_pRemainingChances->SetFont(hFont);

	m_pWeaponNameLabel = new Label(this, "WpnNameLabel", "");
	m_pWeaponNameLabel->SetContentAlignment(Label::a_center);
	m_pWeaponNameLabel->SetFont(hFont);

	char szPanelName[32];

	for (int i = 0; i < WeaponEnchantInfo_TypeName::TYPE_NULL; i++)
	{
		const std::string iDataName = WeaponEnchantProperty[i].first;

		Q_snprintf(szPanelName, sizeof(szPanelName), "WpnEnhancementCategoryLabel%d", i);
		m_pEnhanceCategories[i] = new Label(this, szPanelName, ("#CSO_" + iDataName + "_Label").c_str());
		m_pEnhanceCategories[i]->SetFont(hFont);

		Q_snprintf(szPanelName, sizeof(szPanelName), "WpnEnhancementLevel%d", i);
		m_pEnhanceCategoryLevels[i] = new Label(this, szPanelName, "");
		m_pEnhanceCategoryLevels[i]->SetFont(hFont);

		for (int j = 0; j < 5; j++)
		{
			//We just save the two-Dimensional to the name.
			Q_snprintf(szPanelName, sizeof(szPanelName), "WpnEnhancementLevelGauge%d%d", i, j);

			m_pEnhanceLevelGauge[i][j] = new CWeaponLevelGauge(this, szPanelName);
			m_pEnhanceLevelGauge[i][j]->AddActionSignalTarget(this);
			m_pEnhanceLevelGauge[i][j]->SetVisible(false);

			m_iEnhanceLevelGauge[i] = 0;
		}
	}

	m_pWeaponBackgroundLevel6 = new ImagePanel(this, "BackgroundImageLevel6");
	m_pWeaponBackgroundLevel6->SetImage(scheme()->GetImage("Resource/Control/Basket/Basket_Blank_Slot", true));
	m_pWeaponBackgroundLevel6->SetShouldScaleImage(true);
	m_pWeaponBackgroundLevel6->SetScaleAmount(0);

	m_pWeaponBackgroundLevel8 = new ImagePanel(this, "BackgroundImageLevel8");
	m_pWeaponBackgroundLevel8->SetImage(scheme()->GetImage("Resource/Control/Basket/Basket_Blank_Slot", true));
	m_pWeaponBackgroundLevel8->SetShouldScaleImage(true);
	m_pWeaponBackgroundLevel8->SetScaleAmount(0);

	m_pEnhancedImageLevel6 = new ImagePanel(this, "WpnImageLevel6");
	m_pEnhancedImageLevel6->SetShouldScaleImage(true);
	m_pEnhancedImageLevel6->SetShouldCenterImage(true);

	m_pEnhancedImageLevel8 = new ImagePanel(this, "WpnImageLevel8");
	m_pEnhancedImageLevel8->SetShouldScaleImage(true);
	m_pEnhancedImageLevel8->SetShouldCenterImage(true);

	m_pLevel6UnreachableImage = new ImagePanel(this, "UnreachableCoverLevel6");
	m_pLevel6UnreachableImage->SetImage(scheme()->GetImage("Resource/Itemfarm/Noenchant", true));
	m_pLevel6UnreachableImage->SetShouldScaleImage(true);
	m_pLevel6UnreachableImage->SetScaleAmount(0);
	m_pLevel6UnreachableImage->SetVisible(false);

	m_pLevel8UnreachableImage = new ImagePanel(this, "UnreachableCoverLevel8");
	m_pLevel8UnreachableImage->SetImage(scheme()->GetImage("Resource/Itemfarm/Noenchant", true));
	m_pLevel8UnreachableImage->SetShouldScaleImage(true);
	m_pLevel8UnreachableImage->SetScaleAmount(0);
	m_pLevel8UnreachableImage->SetVisible(false);

	m_pEnhancedTagLevel6 = new ImagePanel(this, "Level6Tag");
	m_pEnhancedTagLevel6->SetImage(scheme()->GetImage("Resource/Itemfarm/Tag6", true));
	m_pEnhancedTagLevel6->MoveToFront();
	m_pEnhancedTagLevel6->SetShouldScaleImage(true);

	m_pEnhancedTagLevel8 = new ImagePanel(this, "Level8Tag");
	m_pEnhancedTagLevel8->SetImage(scheme()->GetImage("Resource/Itemfarm/Tag8", true));
	m_pEnhancedTagLevel8->MoveToFront();
	m_pEnhancedTagLevel8->SetShouldScaleImage(true);

	m_pEnhanceLevelName = new ImagePanel(this, "EnhanceLevel");
	m_pEnhanceLevelName->SetShouldCenterImage(true);
	m_pEnhanceLevelName->SetShouldScaleImage(true);

	m_pApplyButton = new Button(this, "ApplyButton", "#CSO_WeaponEnhanceBtn");
	m_pApplyButton->SetFont(hFont);
	//m_pApplyButton->SetTgaGroupPath("resource/Control/Button_Glow/btn_glow");
	m_pApplyButton->AddActionSignalTarget(this);
	m_pApplyButton->SetCommand(new KeyValues("SaveEnhancementData"));

	m_pDescriptionPanel = new CWeaponDescription(this, "WpnDescPanel");
	m_pDescriptionPanel->AddActionSignalTarget(this);
	m_pDescriptionPanel->SetVisible(false);

	m_iActiveEnchantWeapon_Internal = 0;
	m_bHasUserMadeAnyChanges = false;
}

void CWeaponListLeftPanel::PerformLayout()
{
	BaseClass::PerformLayout();

	int iOffset = 10;
	SetBounds(0, iOffset, 750, 580);

	int iCraftBackGroundWidth = 500;

	m_pCraftBackground->SetBounds(0, 30, iCraftBackGroundWidth, 120);
	m_pCraftImage->SetBounds(-20, 20, 200, 200);
	m_pCraftName->SetBounds(10, 150, 200, 24);
	m_pCraftDesc->SetBounds(160, 60, 320, 50);

	int w, t;
	m_pSearchTip->GetContentSize(w, t);
	m_pSearchTip->SetBounds(iCraftBackGroundWidth + iOffset * 3, 20, w, 20);
	m_pSearchTip->InvalidateLayout();

	int x, y;
	m_pSearchTip->GetPos(x, y);
	m_pSearchText->GetContentSize(w, t);
	m_pSearchText->SetBounds(x, y + 20, w, 20);

	m_pSearchText->GetPos(x, y);
	m_pSearchBar->SetBounds(x + w * 1.2, y, 140, 25);

	m_pWeaponBackground->SetBounds(120, 220, 235, 81);
	m_pWeaponImage->SetBounds(120, 220, 235, 81);
	m_pWeaponLabel->SetBounds(120, 310, 235, 24);
	m_pWeaponLabelBG->SetBounds(120, 310, 235, 24);

	m_pEnhanceLevelName->SetBounds(40, 530, 180, 20);
	m_pWeaponNameLabel->SetBounds(120, 330, 235, 24);
	m_pRemainingChances->SetBounds(40, 360, 235, 24);

	for (int i = 0; i < 6; i++)
	{
		m_pEnhanceCategories[i]->SetBounds(40, 390 + i * 24, 60, 24);
		m_pEnhanceCategoryLevels[i]->SetBounds(220, 390 + i * 24, 50, 24);

		for (int j = 0; j < 5; j++)
		{
			m_pEnhanceLevelGauge[i][j]->SetBounds(100 + j * 22, 400 + i * 24, 21, 5);
		}
	}
	m_pWeaponBackgroundLevel6->SetBounds(320, 390, 180, 60);
	m_pEnhancedTagLevel6->SetBounds(320, 390, 60, 60);
	m_pEnhancedImageLevel6->SetBounds(320, 390, 180, 60);
	m_pLevel6UnreachableImage->SetBounds(320, 390, 180, 60);

	m_pWeaponBackgroundLevel8->SetBounds(320, 465, 180, 60);
	m_pEnhancedTagLevel8->SetBounds(320, 465, 60, 60);
	m_pEnhancedImageLevel8->SetBounds(320, 465, 180, 60);
	m_pLevel8UnreachableImage->SetBounds(320, 465, 180, 60);
	
	m_pApplyButton->SetBounds(380, 280, 100, 48);

}

void CWeaponListLeftPanel::ApplySchemeSettings(vgui2::IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_pSearchTip->SetFgColor(Color(230, 140, 140, 255));
	m_pWeaponLabel->SetFgColor(Color(255, 255, 255, 255));

	m_pRemainingChances->SetFgColor(Color(230, 140, 140, 255));
}

void CWeaponListLeftPanel::SetOnProcessWeapon(int iId, const char* szName)
{
	char szImage[128];
	Q_snprintf(szImage, sizeof(szImage), "gfx/vgui/basket/%s", szName);

	IImage* pImage = scheme()->GetImage(szImage, true);
	m_pWeaponImage->SetImage(pImage);

	//Remove the weapon ?
	if (iId <= 0)
	{
		m_iActiveEnchantWeapon_Internal = -1;
		ResetPanel(false);
		return;
	}

	ResetPanel(true);
	m_iActiveEnchantWeapon_Internal = iId;

	IImage* pImages[2] = { scheme()->GetImage("Resource/Itemfarm/EmptyBox", true), scheme()->GetImage("Resource/Itemfarm/FillBox", true) };

	CWeaponEnchant* pParent = dynamic_cast<CWeaponEnchant*>(GetParent());
	const WeaponEnchantInfo& pInfo = pParent->FindWeaponInfoByID(iId);

	const int* iEnabledProp = pInfo.iCustomInfo;
	const int* iMaximumProp = pInfo.iEnchantInfo;

	int iTotalEnhanceableLevels = 0;
	int iTotalEnhancedLevels = 0;

	for (int i = 0; i < WeaponEnchantInfo_TypeName::TYPE_NULL; i++, iEnabledProp++, iMaximumProp++)
	{
		if (*iMaximumProp > 0 && *iMaximumProp < 6)
		{
			char szText[10];
			Q_snprintf(szText, sizeof(szText), "%d/%d", *iEnabledProp, *iMaximumProp);

			m_pEnhanceCategoryLevels[i]->SetVisible(true);
			m_pEnhanceCategoryLevels[i]->SetText(szText);
		}
		else
		{
			m_pEnhanceCategoryLevels[i]->SetVisible(false);
		}

		int j;
		//For a weapon,its all individual properties are no more than '5'.
		for (j = 0; j < 5; j++)
		{
			m_pEnhanceLevelGauge[i][j]->SetImage(pImages[j < *iEnabledProp ? 1 : 0]);
			m_pEnhanceLevelGauge[i][j]->SetVisible(j < *iMaximumProp);
		}
		m_iEnhanceLevelGauge[i] = *iEnabledProp;

		//We should know the total?
		iTotalEnhanceableLevels += *iMaximumProp;
		iTotalEnhancedLevels += *iEnabledProp;
	}

	//Now We need to redirect the weapon(image and name),if the weapon level is +6 or +8.
	Q_strcat(szImage, "_6.tga", 127);

	if (filesystem()->FileExists(szImage))
	{
		*Q_strrchr(szImage, '.') = '\0';
		IImage* pEnhancedImage = scheme()->GetImage(szImage, true);

		m_pEnhancedImageLevel6->SetImage(pEnhancedImage);

		if (iTotalEnhancedLevels >= 6 && iTotalEnhancedLevels < 8)
			m_pWeaponImage->SetImage(pEnhancedImage);
	}
	else
	{
		m_pEnhancedImageLevel6->SetImage(pImage);
	}

	*Q_strrchr(szImage, '6') = '8';
	Q_strcat(szImage, ".tga", 127);

	if (filesystem()->FileExists(szImage))
	{
		*Q_strrchr(szImage, '.') = '\0';
		IImage* pEnhancedImage = scheme()->GetImage(szImage, true);

		m_pEnhancedImageLevel8->SetImage(scheme()->GetImage(szImage, true));
		if (iTotalEnhancedLevels >= 8)
			m_pWeaponImage->SetImage(pEnhancedImage);
	}
	else
	{
		m_pEnhancedImageLevel8->SetImage(pImage);
	}

	m_pLevel6UnreachableImage->SetVisible(iTotalEnhanceableLevels < 6);
	m_pLevel8UnreachableImage->SetVisible(iTotalEnhanceableLevels < 8);

	//Now localize weapon name.
	const wchar_t* wstrWeapon = GetWeaponLangName(CheckCSOName(g_szCsoWpnName, szName));

	if (wstrWeapon)
	{
		// +6 / +8 ??
		if (iTotalEnhancedLevels > 0)
		{
			wchar_t wszCombinedName[128];
			const char* szWeaponLabel = GetEnhanceLevelName(m_iEnhanceLevelGauge);

			if (szWeaponLabel != NULL)
			{
				wchar_t* wstrLabelLevelName = localize()->Find(szWeaponLabel);

				if (wstrLabelLevelName)
				{
					V_snwprintf(wszCombinedName, sizeof(wszCombinedName), L"(+%d) %s %s", iTotalEnhancedLevels, wstrWeapon, wstrLabelLevelName);
					//_swprintf(wszCombinedName, L"(+%d) %s %s", iTotalEnhancedLevels, wstrWeapon, wstrLabelLevelName);
				}
				else
				{
					V_snwprintf(wszCombinedName, sizeof(wszCombinedName),  L"(+%d) %s", iTotalEnhancedLevels, wstrWeapon);
					//_swprintf(wszCombinedName, L"(+%d) %s", iTotalEnhancedLevels, wstrWeapon);
				}
			}
			else
			{
				V_snwprintf(wszCombinedName, sizeof(wszCombinedName), L"(+%d) %s", iTotalEnhancedLevels, wstrWeapon);
				//_swprintf(wszCombinedName, L"(+%d) %s", iTotalEnhancedLevels, wstrWeapon);
			}
			m_pWeaponNameLabel->SetText(wszCombinedName);
		}
		else
			m_pWeaponNameLabel->SetText(wstrWeapon);
	}

	m_pWeaponNameLabel->InvalidateLayout(true, true);

	Color cColor = UpdateEnchantColor(iTotalEnhancedLevels);
	if (cColor.r() != cColor.g() != cColor.b() != 0)
	{
		m_pWeaponNameLabel->SetFgColor(cColor);
	}

	pImages[0] = scheme()->GetImage("Resource/Itemfarm/Img_Expert", true);
	pImages[1] = scheme()->GetImage("Resource/Itemfarm/Img_Master", true);

	m_pEnhanceLevelName->SetVisible(iTotalEnhancedLevels > 5);

	if (iTotalEnhancedLevels >= 6)
	{
		m_pEnhanceLevelName->SetImage(pImages[iTotalEnhancedLevels >= 8 ? 1 : 0]);
	}

	int iRemainingTimes = /*pInfo.iMaxEnhanceLevel*/8 - iTotalEnhancedLevels;

	wstrWeapon = localize()->Find("#CSO_RemainEnhanceCount");

	if (wstrWeapon)
	{
		wchar_t wszText[128];

		V_snwprintf(wszText, sizeof(wszText), wstrWeapon, iRemainingTimes);
		//_swprintf(wszText, wstrWeapon, iRemainingTimes);

		m_pRemainingChances->SetText(wszText);
	}
	else
		m_pRemainingChances->SetText(L"#CSO_RemainEnhanceCount");

	surface()->PlaySound("events/button_1.wav");

	m_bHasUserMadeAnyChanges = false;
}

int CWeaponListLeftPanel::GetOnProcessWeapon()
{
	//Surely we can indirectly get this through parent.
	//But how to directly get it???
	return m_iActiveEnchantWeapon_Internal;
}
void CWeaponListLeftPanel::SetDescWeapon(const char* szWpnName, bool bUsing)
{
	m_pDescriptionPanel->SetWeapon(szWpnName, bUsing);
}

void CWeaponListLeftPanel::OnGaugeClicked(Panel* gauge)
{
	//Cannot change during game?
	const char* szMapName = engine->pfnGetLevelName();

	if (szMapName && *szMapName)
	{
		MessageBox* pError = new MessageBox("#CSMOE_Enhancement_Denied_Title", "#CSMOE_Enhancement_Denied_InGame", this);

		pError->DoModal();
		pError->SetDeleteSelfOnClose(true);
		pError->SetOKButtonVisible(true);

		return;
	}

	//Gauge slot
	const char* szGaugeName = gauge->GetName() + 24;	// Skip "WpnEnhancementLevelGauge"
	int iCategoryType, iCurrentLevel;

	iCategoryType = szGaugeName[0] - '0';
	iCurrentLevel = szGaugeName[1] - '0' + 1;

	if (iCategoryType < 0 || iCategoryType > 5)
		return;

	if (iCurrentLevel < 0 || iCurrentLevel > 5)
		return;

	m_bHasUserMadeAnyChanges = true;

	CWeaponEnchant* pParent = dynamic_cast<CWeaponEnchant*>(GetParent());

	if (!pParent)
		return;

	WeaponEnchantInfo& pInfo = pParent->FindWeaponInfoByID(pParent->GetOnProcessWeapon());

	//Or just use GetOnProcessWeapon() ?? 
	if (GetOnProcessWeaponCurrentTotalLevel() >= /*pInfo.iMaxEnhanceLevel*/8 && iCurrentLevel > m_iEnhanceLevelGauge[iCategoryType])
	{
		MessageBox* pError = new MessageBox("#CSMOE_Enhancement_Denied_Title", "#CSO_WeaponEnhance_LevelMAX_NorEnchant", this);

		pError->DoModal();
		pError->SetDeleteSelfOnClose(true);
		pError->SetOKButtonVisible(true);

		return;
	}
	IImage* pImages[2] = { scheme()->GetImage("Resource/Itemfarm/EmptyBox", true), scheme()->GetImage("Resource/Itemfarm/FillBox", true) };

	bool bHasEnhanced = iCurrentLevel - 1 < m_iEnhanceLevelGauge[iCategoryType];

	//Alreay enhanced? Pop Back it!!
	if (bHasEnhanced && iCurrentLevel == m_iEnhanceLevelGauge[iCategoryType])
		m_iEnhanceLevelGauge[iCategoryType] = iCurrentLevel - 1;
	//haven't enhanced? Get it enhanced...specific level + 1
	else
		m_iEnhanceLevelGauge[iCategoryType] = iCurrentLevel;

	//Check if the enhance level is beyond the maximum,if so ,turn to the maximum and strip the exceeding parts.
	int iTotalLevels = 0;
	for (int i = 0; i < 6; i++)
	{
		if (i == iCategoryType)
			continue;

		iTotalLevels += m_iEnhanceLevelGauge[i];
	}

	if (iTotalLevels + m_iEnhanceLevelGauge[iCategoryType] > /*pInfo.iMaxEnhanceLevel*/8)
	{
		m_iEnhanceLevelGauge[iCategoryType] = /*pInfo.iMaxEnhanceLevel*/8 - iTotalLevels;
	}

	for (int i = 0; i < 5; i++)
	{
		if (m_pEnhanceLevelGauge[iCategoryType][i]->IsVisible())
		{
			m_pEnhanceLevelGauge[iCategoryType][i]->SetImage(pImages[i < m_iEnhanceLevelGauge[iCategoryType] ? 1 : 0]);
		}
	}

	char szText[10];
	Q_snprintf(szText, sizeof(szText), "%d/%d", m_iEnhanceLevelGauge[iCategoryType], pInfo.iEnchantInfo[iCategoryType]);
	m_pEnhanceCategoryLevels[iCategoryType]->SetText(szText);

	//Change the weapon image anyway...
	int iTotalEnhancedLevels = GetOnProcessWeaponCurrentTotalLevel();

	//UpdateWeaponImage
	char szImage[128];
	const char* szWeaponName = pInfo.szName;

	if (iTotalEnhancedLevels >= 8)
	{
		Q_snprintf(szImage, sizeof(szImage), "gfx/vgui/basket/%s", szWeaponName);

		IImage* pImage = scheme()->GetImage(szImage, true);

		Q_strcat(szImage, "_8.tga", 127);

		if (filesystem()->FileExists(szImage))
		{
			*Q_strrchr(szImage, '.') = '\0';

			m_pWeaponImage->SetImage(scheme()->GetImage(szImage, true));
		}
		else
		{
			m_pWeaponImage->SetImage(pImage);
		}

		m_pEnhanceLevelName->SetImage(scheme()->GetImage("Resource/Itemfarm/Img_Master", true));
	}
	else if (iTotalEnhancedLevels >= 6)
	{
		Q_snprintf(szImage, sizeof(szImage), "gfx/vgui/basket/%s", szWeaponName);

		IImage* pImage = scheme()->GetImage(szImage, true);

		Q_strcat(szImage, "_6.tga", 127);

		if (filesystem()->FileExists(szImage))
		{
			*Q_strrchr(szImage, '.') = '\0';

			m_pWeaponImage->SetImage(scheme()->GetImage(szImage, true));
		}
		else
		{
			m_pWeaponImage->SetImage(pImage);
		}

		m_pEnhanceLevelName->SetImage(scheme()->GetImage("Resource/Itemfarm/Img_Expert", true));
	}
	else
	{
		Q_snprintf(szImage, sizeof(szImage), "gfx/vgui/basket/%s", szWeaponName);

		m_pWeaponImage->SetImage(scheme()->GetImage(szImage, true));
	}

	if (iTotalEnhancedLevels >= 6)
		m_pEnhanceLevelName->SetVisible(true);
	else
		m_pEnhanceLevelName->SetVisible(false);

	const wchar_t* wstrWeapon = GetWeaponLangName(CheckCSOName(g_szCsoWpnName, szWeaponName));

	//Individual tag for different enhance methods?
	if (iTotalEnhancedLevels > 0)
	{
		wchar_t wszCombinedName[128];
		const char* szWeaponLabel = GetEnhanceLevelName(m_iEnhanceLevelGauge);

		if (szWeaponLabel != NULL)
		{
			wchar_t* wstrLabelLevelName = localize()->Find(szWeaponLabel);

			if (wstrLabelLevelName)
			{
				V_snwprintf(wszCombinedName, sizeof(wszCombinedName), L"(+%d) %s %s", iTotalEnhancedLevels, wstrWeapon, wstrLabelLevelName);
				//_swprintf(wszCombinedName, L"(+%d) %s %s", iTotalEnhancedLevels, wstrWeapon, wstrLabelLevelName);
			}
			else
			{
				V_snwprintf(wszCombinedName, sizeof(wszCombinedName), L"(+%d) %s", iTotalEnhancedLevels, wstrWeapon);
				//_swprintf(wszCombinedName, L"(+%d) %s", iTotalEnhancedLevels, wstrWeapon);
			}
		}
		else
		{
			V_snwprintf(wszCombinedName, sizeof(wszCombinedName), L"(+%d) %s", iTotalEnhancedLevels, wstrWeapon);
			//_swprintf(wszCombinedName, L"(+%d) %s", iTotalEnhancedLevels, wstrWeapon);
		}
		m_pWeaponNameLabel->SetText(wszCombinedName);
	}
	else
		m_pWeaponNameLabel->SetText(wstrWeapon);

	m_pWeaponNameLabel->InvalidateLayout(true, true);

	Color cColor = UpdateEnchantColor(iTotalEnhancedLevels);
	if (cColor.r() != cColor.g() != cColor.b() != 0)
	{
		m_pWeaponNameLabel->SetFgColor(cColor);
	}

	int iRemainingTimes = /*pInfo.iMaxEnhanceLevel*/8 - iTotalEnhancedLevels;

	wstrWeapon = localize()->Find("#CSO_RemainEnhanceCount");

	if (wstrWeapon)
	{
		wchar_t wszText[128];

		V_snwprintf(wszText, sizeof(wszText), wstrWeapon, iRemainingTimes);
		//_swprintf(wszText, wstrWeapon, iRemainingTimes);

		m_pRemainingChances->SetText(wszText);
	}
	else
		m_pRemainingChances->SetText(L"#CSO_RemainEnhanceCount");

}

void CWeaponListLeftPanel::ResetPanel(bool bEnable)
{
	for (int i = 0; i < WeaponEnchantInfo_TypeName::TYPE_NULL; i++)
	{
		m_pEnhanceCategoryLevels[i]->SetVisible(bEnable);

		for (int j = 0; j < 5; j++)
		{
			m_pEnhanceLevelGauge[i][j]->SetVisible(bEnable);
		}
	}

	m_pEnhancedImageLevel6->SetVisible(bEnable);
	m_pEnhancedImageLevel8->SetVisible(bEnable);
	m_pWeaponImage->SetVisible(bEnable);
	m_pWeaponNameLabel->SetVisible(bEnable);
	m_pRemainingChances->SetVisible(bEnable);
}

Color CWeaponListLeftPanel::UpdateEnchantColor(int iLevel)
{
	if (iLevel == 5)
	{
		return Color(200, 255, 180, 255);
	}
	else if (iLevel == 6)
	{
		return Color(153, 204, 255, 255);
	}
	else if (iLevel == 7)
	{
		return Color(153, 204, 255, 255);
	}
	else if (iLevel == 8)
	{
		return Color(244, 114, 114, 255);
	}
	else
	{
		return Color(255,255,255,255);
	}
}

int CWeaponListLeftPanel::GetOnProcessWeaponCurrentTotalLevel()
{
	int iTotalLevel = 0;
	const int* iEnhanceData = m_iEnhanceLevelGauge;

	for (int i = 0; i < 6; i++, iEnhanceData++)
	{
		iTotalLevel += *iEnhanceData;
	}
	return iTotalLevel;
}

void CWeaponListLeftPanel::OnSaveEnhancementData()
{
	if (!m_iActiveEnchantWeapon_Internal)
		return;

	if (!m_bHasUserMadeAnyChanges)
	{
		MessageBox* pHelper = new MessageBox("#CSO_WeaponEnhanceTitle", "#CSMOE_WeaponEnchant_Helper", this);

		pHelper->DoModal();
		pHelper->SetDeleteSelfOnClose(true);
		pHelper->SetOKButtonVisible(true);

		return;
	}

	MessageBox* pCongra = new MessageBox("#CSO_WeaponEnhanceTitle", "#CSO_WeaponEnchant_Short_P1", this);

	pCongra->DoModal();
	pCongra->SetDeleteSelfOnClose(true);
	pCongra->SetOKButtonVisible(true);

	CWeaponEnchant* pParent = dynamic_cast<CWeaponEnchant*>(GetParent());

	if (!pParent)
		return;

	WeaponEnchantInfo& pInfo = pParent->FindWeaponInfoByID(pParent->GetOnProcessWeapon());

	for (int i = 0; i < 6; i++)
	{
		pInfo.iCustomInfo[i] = clamp(m_iEnhanceLevelGauge[i], 0, pInfo.iEnchantInfo[i]);
		pInfo.iMaxCustomLevel = GetOnProcessWeaponCurrentTotalLevel();
	}

	pParent->SaveEnhancementData(pInfo);
	pParent->SwitchToLine(pParent->GetScrollBarLine());
}


