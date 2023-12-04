/***********************************************
	CWeaponEnchant
	Build in 2023/11
************************************************/

#include "CWeaponEnchant.h"
#include "GameUI/InventoryMenu_V2/Util/Util.h"
#include "GameUI/InventoryMenu_V2/Support/STLHelper.h"

#include "../3rdparty/csv-parser/csv.hpp"

//Undef it to avoid multi-def bugs...
#ifdef PropertySheet
#undef PropertySheet
#endif
//End of undef...

//We use it to parse .csv file.
#include <fs_int.h>

#include <mathlib/mathlib.h>
#include <nlohmann/json.hpp>

namespace cl 
{
	extern fs_api_t gFileSystemAPI;

	template <class T>
	inline T csv_field_get(csv::CSVField&& field) {
		if (field.is_null()) {
			return { };
		}
		else {
			return field.get<T>();
		}
	}
}
//End of CSV Parser.

//#define CREATE(name) {#name, WeaponEnchantInfo_TypeName::name}


CWeaponEnchant::CWeaponEnchant(Panel* parent) : PropertyPage(parent, "WeaponEnchant")
{
	LoadWeaponList();
	FetchEnhancementData();
	//For Sort Weapons!
	//武器类别分类的面板，这样我们可以通过武器类型找到同类武器

	m_pScrollBar = new ScrollBar(this, "VerticalScrollBar", true);
	m_pScrollBar->AddActionSignalTarget(this);
	m_pScrollBar->SetWide(16);

	for (int i = 0; i < MAX_WEAPONS_INPAGE; i++)
	{
		m_pElements[i] = new CWeaponListRightPanelEle(this, "WpnElement");
		m_pElements[i]->SetId(i);
		m_pElements[i]->AddActionSignalTarget(this);
	}

	//it follows the main menu!!because our Cursor can be anywhere!!!!!
	m_pWpnDragImage = new CWeaponListRightPanelEleDrag(parent, "WpnEleDragImg");
	m_pWpnDragImage->AddActionSignalTarget(this);
	m_pWpnDragImage->SetImage(scheme()->GetImage("gfx/vgui/m4a1", true));
	m_pWpnDragImage->SetVisible(false);
	
	m_pLeftSubPanel = new CWeaponListLeftPanel(this, "WpnLeftSubPanel");
	m_pLeftSubPanel->AddActionSignalTarget(this);
	m_pLeftSubPanel->GetSearchBox()->AddActionSignalTarget(this);

	m_pPropertySheet = new PropertySheet(this, "PropertySheet");
	m_pPropertySheet->AddActionSignalTarget(this);
	m_pPropertySheet->SetPaintBackgroundEnabled(false);

	for (int i = 0; i < 8; i++)
	{
		m_pPropertyPage[i] = new CWeaponListRightPanel(this, "PropertyPage");
		m_pPropertyPage[i]->AddActionSignalTarget(this);

		std::string szSortTypeName;
		szSortTypeName = (std::string)((i == 0) ? "#CSO_" : "#CSO_Inventory_") + WeaponEnchantSort[i].first;

		m_pPropertySheet->AddPage(m_pPropertyPage[i], szSortTypeName.c_str());
	}

}

void CWeaponEnchant::PerformLayout()
{
	BaseClass::PerformLayout();

	//TMD!!!!!its must more than 20!!!!!
	SetBounds(0, 20, 1360, 580);

	m_pPropertySheet->SetBounds(760, 20, 600, 30);

	m_pLeftSubPanel->PerformLayout();

	m_pScrollBar->SetAutoResize(PIN_TOPRIGHT, AUTORESIZE_DOWN, 0, 0, 0, 0);
	m_pScrollBar->SetRange(0, m_vecWeaponDisplayId.size() / 2 + m_vecWeaponDisplayId.size() % 2 + 1);
	m_pScrollBar->SetRangeWindow(4);
	m_pScrollBar->SetButtonPressedScrollValue(4);

	for (int i = 0; i < MAX_WEAPONS_INPAGE; i++)
		m_pElements[i]->SetId(i);
}

void CWeaponEnchant::OnMouseWheeled(int delta)
{
	int val = m_pScrollBar->GetValue();
	val -= (delta);
	m_pScrollBar->SetValue(val);
}

void CWeaponEnchant::OnScrollBarSliderMoved()
{
	InvalidateLayout();
	SwitchToLine(m_pScrollBar->GetValue());
}

void CWeaponEnchant::OnCommand(const char* command)
{
	if (!Q_strncmp(command, "OnWeaponApply_", 14))
	{
		int iSlot;
		sscanf(command, "OnWeaponApply_%d", &iSlot);

		//We firstly check if some weapon has already been selected?
		//if do,we retire it
		
		if (iActiveEnchantWeapon > 0)
		{
			if (m_vecWeaponEnchantList.find(iActiveEnchantWeapon) != m_vecWeaponEnchantList.cend())
			{
				if ((m_vecWeaponDisplayId[m_iLine * 2 + iSlot]) != iActiveEnchantWeapon)
				{
					WeaponEnchantInfo& sOldWeapon = m_vecWeaponEnchantList[iActiveEnchantWeapon];
					if (sOldWeapon.bOnProcess == true)
						sOldWeapon.bOnProcess = false;
				}
			}
		}
		
		//MOE ID!
		iActiveEnchantWeapon = m_vecWeaponDisplayId[m_iLine * 2 + iSlot];

		//Redirect the new weapon
		WeaponEnchantInfo& sWeapon = m_vecWeaponEnchantList[iActiveEnchantWeapon];

		//Reset the weapon state,now it has been selected!!
		sWeapon.bOnProcess = (!sWeapon.bOnProcess);

		//Now We send message to tell leftpanel that the target weapon will be changed!
		//Okay,maybe we donot need Msg...
		if (m_pLeftSubPanel)
		{
			m_pLeftSubPanel->SetOnProcessWeapon(sWeapon.bOnProcess ? iActiveEnchantWeapon : 0, sWeapon.bOnProcess ? sWeapon.szName : "");
		}

		SwitchToLine(m_iLine);
	}
}

void CWeaponEnchant::LoadWeaponList()
{
	if (!m_vecWeaponEnchantList.empty())
		m_vecWeaponEnchantList.clear();

	fs_offset_t fsLength;
	auto szData = cl::gFileSystemAPI.FS_MapFile("resource/itemfarm/enhanceinfo.csv", &fsLength, true);

	if (szData)
	{
		if (fsLength)
		{
			if (fsLength >= 3)
			{
				//FS_MapFree 应该传入FS_MapFile返回的data
				auto szCopyData = (char*)szData;
				if (memcmp(szData, "\xEF\xBB\xBF", 3) == 0)
				{
					szCopyData = (char*)szCopyData + 3;
				}

				auto iLen = strlen((char*)szCopyData);
				csv::CSVReader csv = csv::parse(std::string_view((const char*)szCopyData, iLen));

				//Get Section index!
				//We use C template to get different type data...
				for (csv::CSVRow& pRowData : csv)
				{
					//Init First?
					WeaponEnchantInfo sPrivateInfo = {};

					//Get ItemID and MoeID
					int iItemId = cl::csv_field_get<int>(pRowData[(int)csv.index_of("Id")]);
					int iMoeId = cl::csv_field_get<int>(pRowData[(int)csv.index_of("WeaponID(MoEID)")]);

					if (iItemId <= 0 || iMoeId <= 0)
						continue;

					std::string szWeaponName = cl::csv_field_get<std::string>(pRowData[(int)csv.index_of("ResourceName")]);

					sPrivateInfo.iId = iItemId;
					Q_snprintf(sPrivateInfo.szName, 63, szWeaponName.c_str());

					//Get Weapon Sort Type
					std::string szSortType = cl::csv_field_get<std::string>(pRowData[(int)csv.index_of("WeaponType")]);
					
					for (const auto& pSortTypeList : WeaponEnchantSort) 
					{
						const std::string pSortTypeName = pSortTypeList.first;
						const WeaponEnchantSort_TypeName iSortType = pSortTypeList.second;

						if (szSortType == pSortTypeName)
						{
							sPrivateInfo.iType = iSortType;
							break;
						}
					}

					//Fetch Info(Enchant property).
					for (int i = 0; i < WeaponEnchantInfo_TypeName::TYPE_NULL; i++)
					{
						const std::string iDataName = WeaponEnchantProperty[i].first;
						
						int iDataValue = cl::csv_field_get<int>(pRowData[(int)csv.index_of(iDataName.c_str())]);
						sPrivateInfo.iEnchantInfo[i] = iDataValue;
					}

					//We put MoE ID and WeaponInfo
					m_vecWeaponEnchantList.emplace(std::make_pair(iMoeId,sPrivateInfo));
				}
			}
			cl::gFileSystemAPI.FS_MapFree(szData, fsLength);
		}
	}
}
void CWeaponEnchant::RefreshList()
{
	int iPage = 0;
	int i;

	for (i = 0; i < 8; i++)
	{
		if (m_pPropertyPage[i] == m_pPropertySheet->GetActivePage())
		{
			iPage = i;
			break;
		}
	}
	//Clear
	m_vecWeaponDisplayId.clear();

	int iID;

	for (std::unordered_map<int, WeaponEnchantInfo>::const_iterator pList = m_vecWeaponEnchantList.cbegin(); pList != m_vecWeaponEnchantList.cend(); pList++)
	{
		//No need to use distance because we use unordered_map...
		//iID = std::distance(m_vecWeaponEnchantList.cbegin(), pList);

		//MoEID ??
		iID = pList->first;

		if (std::find(m_vecWeaponDisplayId.cbegin(), m_vecWeaponDisplayId.cend(), iID) != m_vecWeaponDisplayId.cend())
			continue;

		//Push the same type weapons in one page.
		if (iPage && pList->second.iType != (enum WeaponEnchantSort_TypeName)iPage)
			continue;

		if (i != WeaponEnchantSort_TypeName::TYPE_NO)
			m_vecWeaponDisplayId.push_back(iID);

	}

	//Maybe better ??
	/*
	for (const auto& pList :m_vecWeaponEnchantList)
	{
	}
	*/
}

char* CWeaponEnchant::UpdateWeapon(int iTotalEnhancedLevel, const char* szOrigin)
{
	static char szModifiedName[128];
	Q_strcpy(szModifiedName, szOrigin);

	if (iTotalEnhancedLevel >= 6 && iTotalEnhancedLevel < 8)
		Q_strcat(szModifiedName, "_6.tga", 127);
	else if (iTotalEnhancedLevel >= 8)
		Q_strcat(szModifiedName, "_8.tga", 127);
	else 
		return nullptr;

	*Q_strrchr(szModifiedName, '.') = '\0';

	return szModifiedName;
}

void CWeaponEnchant::SwitchToLine(int iLine)
{
	int iDisplayCount = m_vecWeaponDisplayId.size();

	if (iLine < 0 || iLine > iDisplayCount / 2 + iDisplayCount % 2 + 1)
		return;

	m_iLine = iLine;

	for (int iSlot = 0; iSlot < MAX_WEAPONS_INPAGE; iSlot++)
	{
		if (m_iLine * 2 + iSlot >= iDisplayCount)
		{
			m_pElements[iSlot]->SetVisible(false);
		}
		else
		{
			WeaponEnchantInfo& sWeaponInfo = m_vecWeaponEnchantList[m_vecWeaponDisplayId[m_iLine * 2 + iSlot]];
			
			int iTotalEnhancedLevel = /*m_pLeftSubPanel->GetOnProcessWeaponCurrentTotalLevel()*/sWeaponInfo.iMaxCustomLevel;
			char* szUpdatedName = UpdateWeapon(iTotalEnhancedLevel, sWeaponInfo.szName);

			m_pElements[iSlot]->SetVisible(true);
			m_pElements[iSlot]->SetWeapon(szUpdatedName == nullptr ? sWeaponInfo.szName : szUpdatedName, iTotalEnhancedLevel);
			m_pElements[iSlot]->SetSelected(sWeaponInfo.bOnProcess);
		}
	}
}

void CWeaponEnchant::OnPageTabActivated(Panel* panel)
{
	RefreshList();

	PerformLayout();
	SwitchToLine(0);

	m_pScrollBar->SetValue(0);

	m_pLeftSubPanel->GetSearchBox()->SetText(L"");
}

void CWeaponEnchant::OnTextChanged()
{
	auto pSearchBox = m_pLeftSubPanel->GetSearchBox();

	if (!pSearchBox)
		return;

	const size_t iBuffersize = 64;
	static wchar_t wszBuffer[iBuffersize];
	pSearchBox->GetText(wszBuffer, iBuffersize);

	std::wstring wszInput = wszBuffer;
	std::transform(wszInput.begin(), wszInput.end(), wszInput.begin(), towupper);

	char szKey[iBuffersize];
	Q_UnicodeToUTF8(wszInput.c_str(), szKey, iBuffersize - 1);

	std::string szInput = szKey;

	pSearchBox->GetMenu()->ClearCurrentlyHighlightedItem();
	pSearchBox->RemoveAll();

	int iPage = m_pPropertySheet->GetActivePageNum();

	m_vecWeaponDisplayId.clear();

	auto fnFind = [&wszInput, &szInput](const std::pair<int, WeaponEnchantInfo>& other) {
		if (!wszInput.length())
			return true;

		std::string stemp = ToUpper(other.second.szName);
		std::wstring temp = GetWeaponLangName(CheckCSOName(g_szCsoWpnName, other.second.szName));
		std::transform(temp.begin(), temp.end(), temp.begin(), towupper);
		return (strstr(stemp.c_str(), szInput.c_str()) != NULL) || (wcsstr(temp.c_str(), wszInput.c_str()) != NULL);
	};

	for (auto p = m_vecWeaponEnchantList.cbegin(); (p = std::find_if(p, m_vecWeaponEnchantList.cend(), fnFind)) != m_vecWeaponEnchantList.cend(); p++)
	{
		if (!iPage || p->second.iType == iPage)
		{
			int iID = p->first;

			m_vecWeaponDisplayId.push_back(iID);

			if (wszInput.length())
				pSearchBox->AddItem(GetWeaponLangName(CheckCSOName(g_szCsoWpnName, p->second.szName)), new KeyValues(p->second.szName));
		}
	}
	pSearchBox->GetMenu()->Panel::SetVisible(true);
	pSearchBox->GetMenu()->MoveToFront();
	pSearchBox->GetMenu()->PerformLayout();
	pSearchBox->MoveToFront();

	PerformLayout();
	SwitchToLine(0);

	m_pScrollBar->SetValue(0);
}

void CWeaponEnchant::OnElementSelected(int index)
{
	int iId = m_vecWeaponDisplayId[m_iLine * 2 + index];
	if (m_vecWeaponEnchantList.find(iId) == m_vecWeaponEnchantList.cend())
	{
		return;
	}

	WeaponEnchantInfo& sWeapon = m_vecWeaponEnchantList[iId];

	int iTotalEnhancedLevel = /*m_pLeftSubPanel->GetOnProcessWeaponCurrentTotalLevel()*/sWeapon.iMaxCustomLevel;
	char* szUpdatedName = UpdateWeapon(iTotalEnhancedLevel, sWeapon.szName);

	char szImage[128];
	Q_snprintf(szImage, sizeof(szImage), "gfx/vgui/basket/%s", szUpdatedName == nullptr ? sWeapon.szName : szUpdatedName, iTotalEnhancedLevel);

	IImage* pImage = scheme()->GetImage(szImage, false);
	m_pWpnDragImage->SetImage(pImage);

	int w, t;
	pImage->GetSize(w, t);
	m_pWpnDragImage->SetSize(w, t);

	m_pWpnDragImage->SetVisible(true);
	m_pWpnDragImage->MoveToFront();
	m_pWpnDragImage->Repaint();

	m_pWpnDragImage->iSlot = index;
	m_pWpnDragImage->iWpnID = iId;

	m_pLeftSubPanel->SetDescWeapon(sWeapon.szName,true);
}

void CWeaponEnchant::OnElementReleased()
{
	if (!m_pWpnDragImage->IsVisible())
		return;

	int iX, iY, iW, iH;
	m_pWpnDragImage->GetBounds(iX, iY, iW, iH);

	iX += iW / 2;
	iY += iH / 2;

	//Check boundary?? The leftpanel's width is 750...height is 580
	if (iX > 0 && iX < 700 && iY > 200 && iY < 500)
	{
		int iTargetSlot = m_pWpnDragImage->iSlot;

		if(iActiveEnchantWeapon != m_pWpnDragImage->iWpnID)
			FindPanelBySlot(iTargetSlot)->ClickToSelect();
	}

	m_pWpnDragImage->SetVisible(false);
}

//External API For children...
WeaponEnchantInfo& CWeaponEnchant::FindWeaponInfoByID(int id)
{
	if (m_vecWeaponEnchantList.find(id) != m_vecWeaponEnchantList.cend())
		return m_vecWeaponEnchantList[id];
	else
		return m_vecWeaponEnchantList.begin()->second;
}

int CWeaponEnchant::GetOnProcessWeapon()
{
	return iActiveEnchantWeapon;
}

//Internal API...Maybe meaningless..
CWeaponListRightPanelEle* CWeaponEnchant::FindPanelBySlot(int iSlot)
{
	if (iSlot >= 0 && iSlot < MAX_WEAPONS_INPAGE)
	{
		return m_pElements[iSlot];
	}
}

//Fetch data from .json file...
void CWeaponEnchant::FetchEnhancementData()
{
	nlohmann::ordered_json pJson;

	FileHandle_t pFile = vgui2::filesystem()->Open("WeaponEnhance.json", "rb", "CONFIG");

	if (pFile != FILESYSTEM_INVALID_HANDLE)
	{
		int iLen = vgui2::filesystem()->Size(pFile);
		if (iLen > 0)
		{
			char* szBuff = (char*)_alloca(iLen + 1);
			vgui2::filesystem()->Read(szBuff, iLen, pFile);
			szBuff[iLen] = 0;

			pJson = nlohmann::ordered_json::parse(szBuff, szBuff + iLen);
		}

		for (auto& pInfo : pJson) 
		{
			int iMoEID = pInfo["WeaponID"].get<int>();

			//If it's id is valid
			if (m_vecWeaponEnchantList.find(iMoEID) != m_vecWeaponEnchantList.cend())
			{
				//So what next???
				auto& pWpnInfo = m_vecWeaponEnchantList[iMoEID];

				for (int i = 0; i < WeaponEnchantInfo_TypeName::TYPE_NULL; i++)
				{
					const std::string iDataName = WeaponEnchantProperty[i].first;
					
					int iData = pInfo[iDataName].get<int>();
					pWpnInfo.iCustomInfo[i] = clamp(iData, 0, pWpnInfo.iEnchantInfo[i]);
					pWpnInfo.iMaxCustomLevel += pWpnInfo.iCustomInfo[i];

					m_vecWeaponEnchantList[iMoEID] = pWpnInfo;
				}
			}
		}
		vgui2::filesystem()->Close(pFile);
	}
}

//Pack data and save it as .json file...
unsigned int CWeaponEnchant::PackEnhancementData(const int* pData)
{
	//Damage,Accuracy,Rebound,Weight,Repeatedly,Load
	return static_cast<unsigned int>(((pData[0] & 0xF) << 4) | (pData[1] & 0xF) | ((pData[2] & 0xF) << 8) |
		((pData[3] & 0xF) << 12) | ((pData[4] & 0xF) << 16) | ((pData[5] & 0xF) << 20));
}

void CWeaponEnchant::SaveEnhancementData(WeaponEnchantInfo& pInfo)
{
	FileHandle_t pFile = vgui2::filesystem()->Open("WeaponEnhance.json", "wb", "CONFIG");

	m_vecWeaponEnchantList[GetOnProcessWeapon()] = pInfo;

	if (pFile != FILESYSTEM_INVALID_HANDLE)
	{
		nlohmann::ordered_json pJson = nlohmann::ordered_json::array();
		std::string szContent;

		try
		{
			int iTotal = 1;
			for (auto pInfo = m_vecWeaponEnchantList.begin(); pInfo != m_vecWeaponEnchantList.end(); ++pInfo, ++iTotal)
			{
				int iMoEID = pInfo->first;
				auto pWeaponInfo = pInfo->second;

				nlohmann::ordered_json pJson_Content;

				pJson_Content["WeaponID"] = iMoEID;
				//pJson_Content["Data"] = PackEnhancementData((pWeaponInfo.iCustomInfo));

				/*
				//Write AppName
				std::string szAppName = "WeaponEnchant[";
				szAppName.append(std::to_string(iTotal));
				szAppName.append("]");

				auto& pKey = pJson[szAppName];
				pKey["Data"] = PackEnhancementData((pWeaponInfo.iCustomInfo));
				pKey["WeaponID(MoEID)"] = iMoEID;
				pKey["KnifeID"] = 0;
				*/

				for (int i = 0; i < WeaponEnchantInfo_TypeName::TYPE_NULL; i++)
				{
					const std::string iDataName = WeaponEnchantProperty[i].first;
					pJson_Content[iDataName] = pWeaponInfo.iCustomInfo[i];
				}
				pJson.emplace_back(pJson_Content);
			}
			szContent = pJson.dump(2);
		}
		catch (const std::exception& e)
		{
		}

		vgui2::filesystem()->Write(szContent.c_str(), szContent.length(), pFile);
		vgui2::filesystem()->Close(pFile);
	}
}