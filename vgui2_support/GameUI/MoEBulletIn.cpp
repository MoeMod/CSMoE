#include "MoEBulletIn.h"
#include <tier1/KeyValues.h>
#include <vgui_controls/PHandle.h>

// Controls //
#include <vgui/IInput.h>
#include <vgui/IInputInternal.h>
#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include <VGUI/ILocalize.h>
#include "vgui/ISystem.h"

#include "FileSystem.h"

#include <memory>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "RequestHTTPS.h"

using namespace std::chrono_literals;
extern int Q_buildnum();

int iServerDataReceived = 0;
static bool bInit = 0;

vgui2::MessageBox* pMessageBox;

struct BulletInInfo_s {
	std::string name;
	std::string ContextFile;
	std::string Title;
	std::string Date;
};
struct BulletInContext_s {
	std::string name;
	std::string Context;
};
std::vector<BulletInInfo_s> vecServerInfo;
std::vector<BulletInContext_s> vecServerContext;

void from_json(const nlohmann::json& j, BulletInInfo_s& s)
{
	j.at("name").get_to(s.name);
	j.at("ContextFile").get_to(s.ContextFile);
	j.at("Title").get_to(s.Title);
	j.at("Date").get_to(s.Date);
}
bool IsUsingServerData(const BulletInInfo_s& s)
{
	return true;
}

boost::asio::awaitable<void> CCSBTEBulletin::UpdateModel_AsyncRefresh()
{
	// https://api.moemod.com/csmoe/bulletin
	try {

		pMessageBox = new vgui2::MessageBox("#CSMOE_Bulletin_GetServerData_Title", "#CSMOE_Bulletin_GetServerData", this);
		pMessageBox->SetOKButtonVisible(false);
		pMessageBox->SetBounds(GetWide() / 2 - 150, GetTall() / 2 - 100, 300, 200);
		pMessageBox->DoModal();
		pMessageBox->Activate();
		pMessageBox->SetAlpha(255);
		pMessageBox->MoveToFront();

		std::string result = co_await HTTPS_RequestAsync(s_bulletin_ioc, "GET", "tzres.blob.core.chinacloudapi.cn", "443", "/csmoe/tips/bulletin.json", boost::asio::use_awaitable);
		auto j = nlohmann::json::parse(result);

		//读取json文件，存入all_infos
		std::vector<BulletInInfo_s> all_infos;
		j.get_to(all_infos);

		//将读取到的all_infos 内容存入全局变量vecServerInfo，方便vgui使用...
		std::copy_if(all_infos.begin(), all_infos.end(), std::back_inserter(vecServerInfo), IsUsingServerData);
		//标识：已从服务器获取内容

		for (std::vector<BulletInInfo_s>::iterator iter = all_infos.begin(); iter != all_infos.end(); ++iter)
		{
			const BulletInInfo_s& info = *iter;

			if (!info.name.empty())
			{
				std::string SzContextFile = info.ContextFile;
				if (!SzContextFile.empty())
				{
					std::string SzPath = "/csmoe/tips/" + SzContextFile;
					std::string result2 = co_await HTTPS_RequestAsync(s_bulletin_ioc, "GET", "tzres.blob.core.chinacloudapi.cn", "443", SzPath.c_str(), boost::asio::use_awaitable);

					BulletInContext_s info3{};

					info3.name = SzContextFile;
					info3.Context = result2;

					vecServerContext.emplace_back(info3);

					std::string szFileName = "resource/cache/" + info3.name;
					FileHandle_t fh = vgui2::filesystem()->Open(szFileName.c_str(), "w", "CONFIG");
					if (fh != FILESYSTEM_INVALID_HANDLE)
					{
						vgui2::filesystem()->Write(info3.Context.c_str(), info3.Context.length(), fh);
						vgui2::filesystem()->Close(fh);
					}
				}
			}
		}
		iServerDataReceived = 1;
	}
	catch (const std::exception& e)
	{

		std::string SzError = ("Fetch Server Data Failed\n") + std::string(e.what());

		if (pMessageBox != nullptr)
		{
			pMessageBox->Close();
			pMessageBox = nullptr;
		}
		pMessageBox = new vgui2::MessageBox("#CSMOE_Bulletin_GetServerData_Title", SzError.c_str(), this);

		pMessageBox->SetOKButtonText("#CSO_OKl_Btn");
		pMessageBox->SetOKButtonVisible(true);
		pMessageBox->SetBounds(GetWide() / 2 - 150, GetTall() / 2 - 100, 300, 200);
		pMessageBox->DoModal();
		pMessageBox->Activate();
	}
}

void CCSBTEBulletin::UpdateModel_Refresh()
{
	s_bulletin_ioc.reset();

	auto bindPointer = std::bind(&CCSBTEBulletin::UpdateModel_AsyncRefresh, this);
	boost::asio::co_spawn(s_bulletin_ioc, bindPointer, boost::asio::detached);

	s_bulletin_ioc.restart();
}
void CCSBTEBulletin::Update_Initiate()
{
	UpdateModel_Refresh();

}

void CCSBTEBulletin::Update_OnGui()
{
	s_bulletin_ioc.poll_one();
}


const wchar_t* szFixedBulletinCN[] =
{
	L"原作者: [Creators]",
	L"杰拉德临死前（萌殇小白白）\n",
	L"共创人员: [Co-Creators]",
	L"过气的维吉尔 & 血小板 & 橘子  & Rentro",
	L"ZackMount    & 小文喵 & Titan & Mat\n",
	L"版权所有: [Contributors]",
	L"CSMoE Team\nBTE Team\nXash3D FWGS\n",
	L"官方组织: [Official Orgs]",
	L"QQ群:706711420 & 贴吧:百度csoldjb吧\n",
	L"",
	NULL
};

const std::pair<int, Color> FixedColorSetCN[] =
{
	std::pair<int, Color>(1, Color(255, 255, 255, 255)),
	std::pair<int, Color>(2, Color(255, 205, 20, 255)),
	std::pair<int, Color>(3, Color(255, 255, 255, 255)),
	std::pair<int, Color>(4, Color(255, 205, 20, 255)),
	std::pair<int, Color>(5, Color(255, 205, 20, 255)),
	std::pair<int, Color>(6, Color(255, 255, 255, 255)),
	std::pair<int, Color>(7, Color(255, 126, 126, 255)),
	std::pair<int, Color>(8, Color(255, 255, 255, 255)),
	std::pair<int, Color>(9, Color(40, 155, 175, 255)),
	std::pair<int, Color>(10, Color(255, 255, 255, 255)),
	std::pair<int, Color>(-1, Color())
};

using namespace vgui2;
char g_szLanguage[128] = "";



wchar_t* GetLanguageAuthorText(void)
{
	return (wchar_t*)L"CS:Mobile-Oriented Edition\n本游戏完全免费，禁止任何商业用途\n\n";
}

struct CCSBTEBulletin::impl_t
{
	vgui2::Button* Close;
	vgui2::Button* Next;

	ImagePanel* Title;
	vgui2::Label* TitleLabel;

	vgui2::TextEntry* m_TextEntry;
	CCSBTEBulletinText* m_pText;

	CCSBTEBulletinScroll* ScrollBar;
	CCSBTEBulletinImage* ImagePanel[9];

	CCSBTEBulletinColumn* Column;

	//Button* Bulletin;
	//Button* EventProgress;
};

CCSBTEBulletin::CCSBTEBulletin(Panel* parent, const char* panelName, bool showTaskbarIcon)
	: BaseClass(parent, panelName, showTaskbarIcon), pimpl(std::make_unique<impl_t>())
{

	SetBounds(241, 144, 570, 580);

	pimpl->Title = new ImagePanel(this, "Title");
	pimpl->Title->SetBounds(15, 16, 530, 64);
	pimpl->Title->SetImage("resource/Bulletin/NoticeTitle2");

	pimpl->TitleLabel = new vgui2::Label(this, "TitleLabel", "#CSO_Bulletin_Title");
	pimpl->TitleLabel->SetBounds(33, 34, 494, 27);
	pimpl->TitleLabel->SetContentAlignment(Label::a_center);

	pimpl->Next = new vgui2::Button(this, "Next", "#CSO_Bulletin_Next", this, "Next");
	pimpl->Next->SetBounds(276, 527, 124, 32);
	pimpl->Next->SetContentAlignment(Label::a_center);

	pimpl->Close = new vgui2::Button(this, "Close", "#CSO_Bulletin_Close", this, "vguicancel");
	pimpl->Close->SetBounds(417, 527, 98, 32);
	pimpl->Close->SetContentAlignment(Label::a_center);

	pimpl->m_TextEntry = new vgui2::TextEntry(this, "NoticeText");
	pimpl->m_TextEntry->SetBounds(244, 120, 300, 393);
	pimpl->m_TextEntry->SetSize(300, 393);
	pimpl->m_TextEntry->SetMaximumCharCount(-1);
	pimpl->m_TextEntry->SetText("");
	pimpl->m_TextEntry->SetAutoResize(PIN_TOPLEFT, AUTORESIZE_NO, 0, 0, 0, 0);
	pimpl->m_TextEntry->SetEditable(false);

	pimpl->m_pText = new CCSBTEBulletinText(pimpl->m_TextEntry, "NoticeRichText");
	pimpl->m_pText->SetBounds(0, 0, 300, 393);
	pimpl->m_pText->SetAutoResize(PIN_TOPLEFT, AUTORESIZE_NO, 0, 0, 0, 0);
	pimpl->m_pText->InsertColorChange(Color(255, 190, 0, 255));
	pimpl->m_pText->InsertString(GetLanguageAuthorText());
	pimpl->m_pText->SetShouldDrawStart(true);

	LoadCreator();

	pimpl->Column = new CCSBTEBulletinColumn(this, "CSBTEBulletinColumn", this, (const char*)NULL);
	pimpl->Column->AddColumn("#CSO_Bulletin_Notice", "Bulletin");
	pimpl->Column->AddColumn("#CSMOE_Bulletin_ServerUpdate", "ServerUpdate");
	pimpl->Column->AddColumn("#CSMOE_Bulletin_Announce_Title", "EventProgress");
	pimpl->Column->SetBounds(15, 91, 86, 25);
	pimpl->Column->SetColumnFlag(CCSBTEBulletinColumn::COLUMN_NO_PRESS_EFFECT);
	pimpl->Column->SetActive(0, true);
	pimpl->Column->SetClickSound("events/tab_1.wav");

	pimpl->ScrollBar = new CCSBTEBulletinScroll(this, "VerticalScrollBar", true);
	pimpl->ScrollBar->SetBounds(220, 120, 20, 405);
	pimpl->ScrollBar->AddActionSignalTarget(this);
	pimpl->ScrollBar->SetWide(16);

	for (int i = 0; i < 9; i++)
	{
		pimpl->ImagePanel[i] = new CCSBTEBulletinImage(this, "CSOBulletInPageImage");
		pimpl->ImagePanel[i]->SetPos(15, 120 + 45 * i);
		pimpl->ImagePanel[i]->SetId(i);
	}

	SetType(BULLETIN_EVENTPROGRESS, true);

	SetMenuButtonVisible(false);
	SetCloseButtonVisible(false);
	SetSizeable(false);
	SetTitle("", false);
	SetEnabled(false);

	Update_Initiate();
}
void CCSBTEBulletin::OnThink()
{
	BaseClass::OnThink();

	if (!iServerDataReceived)
		Update_OnGui();

	if (!bInit && iServerDataReceived)
	{
		ReadServerFile();
		bInit = true;
	}
	if (bInit)
	{
		if (pMessageBox != nullptr)
		{
			pMessageBox->Close();
			pMessageBox = nullptr;
			SetType(BULLETIN_BULLETIN, true);
		}

	}
}

void CCSBTEBulletin::OnCommand(const char* command)
{
	if (!Q_stricmp(command, "vguicancel"))
	{
		return Close();
	}
	else if (!Q_stricmp(command, "Next"))
	{
		int current = _pressed + 1;

		if (current >= (signed int)m_vecPageBulletin[(int)m_Type].size())
			return;

		int line = pimpl->ScrollBar->GetValue();
		int distance = current - 8 - line;

		if (distance > 0)
		{
			pimpl->ScrollBar->SetValue(line + distance);
			m_iCurrentPage = 7;
		}

		int iPos = m_iCurrentPage + 1;

		if (iPos > 8)
			iPos = 8;

		if (!LoadContext(iPos))
			return;

		for (int i = 0; i < 9; i++)
		{
			if (i == iPos)
				continue;

			pimpl->ImagePanel[i]->SetActive(false);
		}
	}
	else if (!Q_stricmp(command, "Bulletin"))
	{
		return SetType(BULLETIN_BULLETIN);
	}
	else if (!Q_stricmp(command, "ServerUpdate"))
	{
		return SetType(BULLETIN_SERVERUPDATE);
	}
	else if (!Q_stricmp(command, "EventProgress"))
	{
		return SetType(BULLETIN_EVENTPROGRESS);
	}
	else if (!Q_strnicmp(command, "Bulletin_SetPage", 16))
	{
		int iPos = atoi(command + 17);
		if (!LoadContext(iPos))
			return;

		for (int i = 0; i < 9; i++)
		{
			if (i == iPos)
				continue;

			pimpl->ImagePanel[i]->SetActive(false);
		}
	}
	else if (!Q_strnicmp(command, "Bulletin_SwitchLine", 19))
	{
		int line = atoi(command + 20);
		SwitchToLine(line);
	}
}

void CCSBTEBulletin::SetType(Bulletin_Type Type, bool bFirst)
{
	if (m_Type == Type && !bFirst)
		return;

	int size = (int)m_vecPageBulletin[(int)Type].size();
	if (size <= 0)
		return;

	pimpl->ScrollBar->SetContextCount(size);
	pimpl->ScrollBar->SetValue(0);

	bulletin_elem pElem;
	for (int i = 0; i < 9; i++)
	{
		pimpl->ImagePanel[i]->SetActive(false);

		if (i >= size)
		{
			pimpl->ImagePanel[i]->SetBulletinText((wchar_t*)L"", (wchar_t*)L"");
			pimpl->ImagePanel[i]->SetCanUse(false);
			continue;
		}

		pElem = m_vecPageBulletin[(int)Type].at(i);
		pimpl->ImagePanel[i]->SetBulletinText(pElem.szTitle, pElem.szDate);
		pimpl->ImagePanel[i]->SetCanUse(true);
	}

	m_Type = Type;
	LoadContext(0);
	SetEnabled(true);
}

bool CCSBTEBulletin::LoadContext(int page)
{
	int iRealPos = page + pimpl->ScrollBar->GetValue();

	if (!IsPageValid(m_Type, iRealPos))
		return false;

	pimpl->m_pText->LoadContext(m_vecPageBulletin[(int)m_Type].at(iRealPos));
	_pressed = iRealPos;

	pimpl->ImagePanel[page]->SetActive(true);

	m_iCurrentPage = page;

	return true;
}

int CCSBTEBulletin::ReadServerContextFile(bulletin_elem* pElement, std::string szFile, int iType)
{
	for (std::vector<BulletInContext_s>::iterator iter = vecServerContext.begin(); iter != vecServerContext.end(); ++iter)
	{
		const BulletInContext_s& info = *iter;

		if (!info.name.empty())
		{
			if (szFile == info.name)
			{
				std::string SzContext = info.Context;
				std::string szFileName = "resource/cache/" + info.name;

				FileHandle_t fh = vgui2::filesystem()->Open(szFileName.c_str(), "rb", "CONFIG");

				if (!SzContext.empty() && fh != FILESYSTEM_INVALID_HANDLE)
				{
					int iCurrent = 0;
					static char szLine[1024];


					while (1)
					{
						iCurrent++;
						char* p = vgui2::filesystem()->ReadLine(szLine, sizeof(szLine), fh);
						if (!p)
							break;

						ReplaceTokenAll(szLine, "\r\n", "");
						ReplaceTokenAll(szLine, "\n", "");

						if (szLine[0] == '\\')
						{
							pElement->szContext.emplace_back(szLine + 1);
						}
						else
						{
							if (szLine[0] == '<')
							{
								ReplaceTokenAll(szLine, " ", "");
								ReplaceTokenFirst(szLine, "<", "");
								ReplaceTokenFirst(szLine, ">", "");
								char* p = strtok(szLine, ":");

								if (!p)
									continue;

								if (!Q_stricmp(p, "image"))
								{
									p = strtok(NULL, ":");
									if (p)
									{
										ReplaceTokenAll(p, " ", "");

										char* p2 = strtok(p, ",");
										char str[16];

										bulletin_imgdata TempImageData{ 0 };
										//memset(&TempImageData, 0, sizeof(TempImageData));

										for (int k = 0; k < 7; k++)
										{
											if (!p2)
												break;

											char* p3 = Q_strstr(p2, ",");
											if (p3)
											{
												int pos = p2 - p3;
												Q_strncpy(str, p2 + pos + 1, pos - iType - 1);

												p2 = str;
											}

											switch (k)
											{
											case 0:Q_strcpy(TempImageData.szImage, p2); break;
											case 1:TempImageData.x = atoi(p2); break;
											case 2:TempImageData.y = atoi(p2); break;
											case 3:TempImageData.w = atoi(p2); break;
											case 4:TempImageData.h = atoi(p2); break;
											case 5:TempImageData.frame = !Q_strncmp(p2, "1", 1) ? true : false; break;
											case 6:TempImageData.solid = !Q_strncmp(p2, "1", 1) ? true : false; break;
											}

											p2 = strtok(NULL, ",");
										}

										TempImageData.line = iCurrent;

										pElement->vecImage.emplace_back(TempImageData);
										Q_strcpy(szLine, "");
									}
								}
								else if (!Q_stricmp(p, "color"))
								{
									p = strtok(NULL, ":");
									if (p)
									{
										ReplaceTokenAll(p, " ", "");
										char* p2 = strtok(p, ",");
										char str[16];
										int r = 0, g = 0, b = 0, a = 0;

										for (int k = 0; k < 4; k++)
										{
											if (!p2)
												break;

											char* p3 = Q_strstr(p2, ",");
											if (p3)
											{
												int pos = p2 - p3;
												Q_strncpy(str, p2 + pos + 1, pos - iType - 1);

												p2 = str;
											}

											switch (k)
											{
											case 0:r = atoi(p2); break;
											case 1:g = atoi(p2); break;
											case 2:b = atoi(p2); break;
											case 3:a = atoi(p2); break;
											}

											p2 = strtok(NULL, ",");
										}
										pElement->vecColor.emplace_back(iCurrent, Color(r, g, b, a));
									}
									iCurrent--;
									continue;
								}
							}

							pElement->szContext.emplace_back(szLine);
						}
					}
					vgui2::filesystem()->Close(fh);
				}
			}
		}
	}
	return 1;
}
int CCSBTEBulletin::ReadContextFile(bulletin_elem* pElement, char* szFile, int iType)
{
	static char szFileName[128];
	bool bisLangEng = (!Q_stricmp(g_szLanguage, "english"));

	if (!bisLangEng)
		sprintf(szFileName, "csmoe_%s/%s", g_szLanguage, szFile);
	else
		sprintf(szFileName, "csmoe/%s", szFile);

	FILE* fp = fopen(szFileName, "r");

	if (!bisLangEng && !fp)
	{
		sprintf(szFileName, "csmoe/%s", szFile);
		fp = fopen(szFileName, "r");
	}

	if (!fp)
		return 0;

	int iCurrent = 0;
	static char szLine[1024];

	while (1)
	{
		iCurrent++;
		char* p = fgets(szLine, sizeof(szLine), fp);

		if (!p)
			break;

		ReplaceTokenAll(szLine, "\r\n", "");
		ReplaceTokenAll(szLine, "\n", "");

		if (szLine[0] == '\\')
		{
			pElement->szContext.emplace_back(szLine + 1);
		}
		else
		{
			if (szLine[0] == '<')
			{
				ReplaceTokenAll(szLine, " ", "");
				ReplaceTokenFirst(szLine, "<", "");
				ReplaceTokenFirst(szLine, ">", "");
				char* p = strtok(szLine, ":");

				if (!p)
					continue;

				if (!Q_stricmp(p, "image"))
				{
					p = strtok(NULL, ":");
					if (p)
					{
						ReplaceTokenAll(p, " ", "");

						char* p2 = strtok(p, ",");
						char str[16];

						bulletin_imgdata TempImageData{ 0 };
						//memset(&TempImageData, 0, sizeof(TempImageData));

						for (int k = 0; k < 6; k++)
						{
							if (!p2)
								break;

							char* p3 = Q_strstr(p2, ",");
							if (p3)
							{
								int pos = p2 - p3;
								Q_strncpy(str, p2 + pos + 1, pos - iType - 1);

								p2 = str;
							}

							switch (k)
							{
							case 0:Q_strcpy(TempImageData.szImage, p2); break;
							case 1:TempImageData.x = atoi(p2); break;
							case 2:TempImageData.y = atoi(p2); break;
							case 3:TempImageData.w = atoi(p2); break;
							case 4:TempImageData.h = atoi(p2); break;
							case 5:TempImageData.frame = !Q_strncmp(p2, "1", 1) ? true : false; break;
							}

							p2 = strtok(NULL, ",");
						}

						TempImageData.line = iCurrent;

						pElement->vecImage.emplace_back(TempImageData);
						Q_strcpy(szLine, "");
					}
				}
				else if (!Q_stricmp(p, "color"))
				{
					p = strtok(NULL, ":");
					if (p)
					{
						ReplaceTokenAll(p, " ", "");
						char* p2 = strtok(p, ",");
						char str[16];
						int r = 0, g = 0, b = 0, a = 0;

						for (int k = 0; k < 4; k++)
						{
							if (!p2)
								break;

							char* p3 = Q_strstr(p2, ",");
							if (p3)
							{
								int pos = p2 - p3;
								Q_strncpy(str, p2 + pos + 1, pos - iType - 1);

								p2 = str;
							}

							switch (k)
							{
							case 0:r = atoi(p2); break;
							case 1:g = atoi(p2); break;
							case 2:b = atoi(p2); break;
							case 3:a = atoi(p2); break;
							}

							p2 = strtok(NULL, ",");
						}
						pElement->vecColor.emplace_back(iCurrent, Color(r, g, b, a));
					}
					iCurrent--;
					continue;
				}
			}

			pElement->szContext.emplace_back(szLine);
		}
	}

	fclose(fp);

	return 1;
}

void CCSBTEBulletin::LoadCreator(void)
{
	bulletin_elem temp{ 0 };
	bulletin_imgdata pLogo{ 0 };

	//memset(&temp, 0, sizeof(bulletin_elem));

	wcscpy(temp.szTitle, L"开发人员列表");
	wcscpy(temp.szDate, L"[MoE]Creator List");

	const wchar_t** ppText;
	const std::pair<int, Color>* pColorSet;

	ppText = szFixedBulletinCN;
	pColorSet = FixedColorSetCN;

	char szUTF[256];
	for (int i = 0; ppText[i] != NULL; i++)
	{
		localize()->ConvertUnicodeToANSI(ppText[i], szUTF, 255);
		temp.szContext.push_back(szUTF);
	}

	for (int i = 0; pColorSet[i].first != -1; i++)
		temp.vecColor.push_back(pColorSet[i]);

	Q_strcpy(pLogo.szImage, "resource/bulletin/citrus_logo");
	pLogo.x = 0;
	pLogo.y = 63 + 20;
	pLogo.w = 250;
	pLogo.h = 63;
	pLogo.frame = true;
	pLogo.line = 10;

	temp.vecImage.push_back(pLogo);


	m_vecPageBulletin[2].push_back(temp);
}
void CCSBTEBulletin::ReadServerFile(void)
{
	for (int i = 0; i < 3; i++)
	{
		bulletin_elem pTemp;
		bool bContinueCurrent = false;

		for (int j = 0; j < BULLETIN_MAX_PAGES; j++)
		{
			wcscpy(pTemp.szTitle, L"");
			wcscpy(pTemp.szDate, L"");
			pTemp.szContext.clear();
			pTemp.vecImage.clear();
			pTemp.vecColor.clear();

			char* szKey = va("%s%d", !i ? "Bulletin" : (i == 1 ? "ServerUpdate" : "EventProgress"), j + 1);

			for (std::vector<BulletInInfo_s>::iterator iter = vecServerInfo.begin(); iter != vecServerInfo.end(); ++iter)
			{
				const BulletInInfo_s& info = *iter;

				if (!info.name.empty())
				{
					if (info.name == szKey)
					{
						std::string SzContextFile = info.ContextFile;
						if (!SzContextFile.empty())
						{
							if (!ReadServerContextFile(&pTemp, SzContextFile, i))
							{
								bContinueCurrent = true;
								break;
							}
							Q_UTF8ToUnicode(info.Title.c_str(), pTemp.szTitle, 255);
							Q_UTF8ToUnicode(info.Date.c_str(), pTemp.szDate, 127);

							m_vecPageBulletin[i].emplace_back(pTemp);
						}
					}

				}
			}
		}
	}
}
void CCSBTEBulletin::ReadCustomizeFile(void)
{

	for (int i = 0; i < 3; i++)
	{
		bulletin_elem pTemp;
		char data[256];
		for (int j = 0; j < BULLETIN_MAX_PAGES; j++)
		{
			wcscpy(pTemp.szTitle, L"");
			wcscpy(pTemp.szDate, L"");
			pTemp.szContext.clear();
			pTemp.vecImage.clear();
			pTemp.vecColor.clear();


			char* szKey = va("%s%d", !i ? "Bulletin" : (i == 1 ? "ServerUpdate" : "EventProgress"), j + 1);

			//GetPrivateProfileString(szKey, "ContextFile", "", data, 63, "csmoe\\bulletin.ini");
			if (data[0])
			{
				if (!ReadContextFile(&pTemp, data, i))
					continue;

				//GetPrivateProfileString(szKey, "Title", "", data, 255, "csmoe\\bulletin.ini");
				Q_UTF8ToUnicode(data, pTemp.szTitle, 255);

				//GetPrivateProfileString(szKey, "Date", "", data, 127, "csmoe\\bulletin.ini");
				Q_UTF8ToUnicode(data, pTemp.szDate, 127);

				m_vecPageBulletin[i].emplace_back(pTemp);
			}
		}
	}
}

Panel* CCSBTEBulletin::CreateControlByName(const char* controlName)
{
	if (!Q_stricmp(controlName, "CTempFrame"))
	{
		return this;
	}
	else if (!Q_stricmp(controlName, "IMETextEntry"))
	{
		return pimpl->m_TextEntry;
	}
	else if (!Q_stricmp(controlName, "Label"))
	{
		vgui2::Label* p = new Label(this, controlName, "");
		p->SetVisible(false);
		return p;
	}
	return BaseClass::CreateControlByName(controlName);
}

void CCSBTEBulletin::OnMouseWheeled(int delta)
{
	int val = pimpl->ScrollBar->GetValue();
	val -= (delta);
	pimpl->ScrollBar->SetValue(val);
}

void CCSBTEBulletin::ApplySchemeSettings(IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	pimpl->TitleLabel->SetFont(pScheme->GetFont("Normal24", IsProportional()));
	pimpl->m_TextEntry->SetBackgroundSkin(pScheme, "Dark");
	//pimpl->Bulletin->SetCustomBorderScheme(pScheme, "resource/control/listpanel/column");
	//pimpl->EventProgress->SetCustomBorderScheme(pScheme, "resource/control/listpanel/column");
}

void CCSBTEBulletin::SwitchToLine(int line)
{
	int size = m_vecPageBulletin[(int)m_Type].size();
	for (int i = line; i < 9 + line; i++)
	{
		int truepos = i - line;
		if (i < size)
		{
			if (i == _pressed)
				pimpl->ImagePanel[truepos]->SetActive(true);
			else
				pimpl->ImagePanel[truepos]->SetActive(false);

			bulletin_elem pElem = m_vecPageBulletin[(int)m_Type].at(i);
			pimpl->ImagePanel[truepos]->SetBulletinText(pElem.szTitle, pElem.szDate);
			pimpl->ImagePanel[truepos]->SetCanUse(true);
		}
		else
		{
			pimpl->ImagePanel[truepos]->SetActive(false);
			pimpl->ImagePanel[truepos]->SetBulletinText((wchar_t*)L"", (wchar_t*)L"");
			pimpl->ImagePanel[truepos]->SetCanUse(false);
		}
	}
}

CCSBTEBulletinText::CCSBTEBulletinText(Panel* parent, const char* panelName) : RichText(parent, panelName)
{
	SetUnusedScrollbarInvisible(false);
	//SetPanelInteractive(false);

	memset(m_Image, NULL, sizeof(m_Image));
	memset(m_ImageData, 0, sizeof(m_ImageData));

	for (int i = 0; i < BULLETIN_MAX_IMAGE; i++)
	{
		m_ImageBackground[i] = new ImagePanel(this, "CCSBTEBulletinSubImg");
		m_ImageBackground[i]->SetImage(scheme()->GetImage("gfx/ui/panel/basket_blank_slot", false));
		m_ImageBackground[i]->SetShouldScaleImage(true);
		m_ImageBackground[i]->SetVisible(true);

		m_Image[i] = new ImagePanel(m_ImageBackground[i], "BulletinImage");
		m_Image[i]->SetShouldScaleImage(true);
		m_Image[i]->SetVisible(false);
	}

	IScheme* pScheme = scheme()->GetIScheme(GetScheme());

	if (pScheme)
		m_Font = pScheme->GetFont("Default", IsProportional());
}

CCSBTEBulletinText::~CCSBTEBulletinText()
{
	for (int i = 0; i < BULLETIN_MAX_IMAGE; i++)
	{
		if (m_Image[i])
			delete m_Image[i];

		if (m_ImageBackground[i])
			delete m_ImageBackground[i];
	}
}

void CCSBTEBulletinText::SetImage(int pos, bulletin_imgdata Data)
{
	if (pos < 0 || pos >= BULLETIN_MAX_IMAGE)
		return;

	m_Image[pos]->SetImage(Data.szImage);
	m_Image[pos]->SetSize(Data.w, Data.h);
	m_Image[pos]->SetVisible(true);

	m_ImageBackground[pos]->SetBounds(Data.x, Data.y, Data.w, Data.h);

	if (Data.frame)
	{
		m_ImageBackground[pos]->SetDrawColor(Color(255, 255, 255, 255));
	}
	else
	{
		m_ImageBackground[pos]->SetDrawColor(Color(0, 0, 0, 0));
	}

	memcpy(&m_ImageData[pos], &Data, sizeof(bulletin_imgdata));
}

void CCSBTEBulletinText::LoadContext(bulletin_elem Data)
{
	ClearImageData();
	SetText(L"");

	//255,190,0,255
	InsertColorChange(Color(0, 150, 0, 255));
	wchar_t* pwcs = GetLanguageAuthorText();
	InsertString(pwcs);

	int wide = GetWide() - _vertScrollBar->GetWide();

	if (Data.szContext.size())
	{
		int line = 1, lastcor = -1, lastimg = -1;
		if (Data.vecColor.size())
			lastcor = 0;

		if (Data.vecImage.size())
			lastimg = 0;

		InsertColorChange(Color(255, 255, 255, 255));

		int totalline = 0;

		int width = 3, height = 0;

		if (m_Font)
		{
			height = surface()->GetFontTall(m_Font) + 1;
		}

		static wchar_t szText[512];

		if (pwcs)
		{
			int iLen = wcslen(pwcs);
			for (int i = 0; i < iLen; i++)
			{
				if (pwcs[i] == L'\n')
				{
					width = 3;
					totalline++;
					continue;
				}

				int wp = surface()->GetCharacterWidth(m_Font, pwcs[i]);

				if (width + wp >= wide)
				{
					width = 3;
					totalline++;
				}

				width += wp;
			}
		}

		for (auto p : Data.szContext)
		{
			if (lastcor != -1)
			{
				if (line == Data.vecColor[lastcor].first)
				{
					InsertColorChange(Data.vecColor[lastcor].second);

					lastcor++;
					if (lastcor >= (int)Data.vecColor.size())
						lastcor = -1;
				}
			}

			if (lastimg != -1)
			{
				if (line == Data.vecImage[lastimg].line)
				{
					int val = height * totalline;

					Data.vecImage[lastimg].y += val;
					SetImage(lastimg, Data.vecImage[lastimg]);
					Data.vecImage[lastimg].y -= val;

					if (height && !Data.vecImage[lastimg].solid)
					{
						int width2, height2;
						m_Image[lastimg]->GetSize(width2, height2);

						int numline = height2 / height;
						if (height2 % height)
							numline++;

						totalline += numline;

						for (int i = 0; i < numline; i++)
							InsertString(L"\n");
					}

					lastimg++;
					if (lastimg >= (int)Data.vecImage.size())
						lastimg = -1;

					line++;
					continue;
				}
			}

			totalline++;

			Q_UTF8ToUnicode(p.c_str(), szText, ARRAYSIZE(szText));

			width = 3;

			for (unsigned int i = 0; i < wcslen(szText); i++)
			{
				int wp = surface()->GetCharacterWidth(m_Font, szText[i]);
				if (width + wp >= wide)
				{
					width = 3;
					totalline++;
				}

				width += wp;
			}

			if (szText[0] == '#')
			{
				wchar_t* pwcs2 = localize()->Find(p.c_str());
				if (pwcs2)
					wcscpy(szText, pwcs2);

				InsertString(szText);
				InsertString(L"\n");
			}
			else
			{
				InsertString(szText);
				InsertString(L"\n");
			}

			line++;
		}
	}
}

void CCSBTEBulletinText::OnSliderMoved()
{
	int current = _vertScrollBar->GetValue();

	int height = 0;
	if (m_Font)
		height = surface()->GetFontTall(m_Font) + 1;

	if (height)
	{
		int a, b;
		_vertScrollBar->GetRange(a, b);

		bulletin_imgdata Temp;

		for (int i = 0; i < BULLETIN_MAX_IMAGE; i++)
		{
			if (m_Image[i])
			{
				if (m_Image[i]->IsVisible())
				{
					memcpy(&Temp, &m_ImageData[i], sizeof(bulletin_imgdata));
					Temp.y -= current * height;
					m_ImageBackground[i]->SetPos(Temp.x, Temp.y);
				}
			}
		}
	}

	BaseClass::OnSliderMoved();
}

CCSBTEBulletinScroll::CCSBTEBulletinScroll(Panel* parent, const char* panelName, bool vertical) : ScrollBar(parent, panelName, vertical)
{
	SetButtonPressedScrollValue(1);

	m_iSize = 0;
	m_bUnusedScrollbarInvis = false;
}

void CCSBTEBulletinScroll::SetContextCount(int iSize)
{
	const int displayLines = 9;

	if (iSize <= displayLines)
	{
		SetEnabled(false);
		SetRange(0, iSize);
		SetRangeWindow(iSize);
		SetValue(0);

		if (m_bUnusedScrollbarInvis)
		{
			SetVisible(false);
		}
	}
	else
	{
		if (m_bUnusedScrollbarInvis)
		{
			SetVisible(true);
		}

		SetValue(0);
		SetRange(0, iSize);
		SetRangeWindow(displayLines);
		SetEnabled(true);

		Repaint();
	}

	m_iSize = iSize;
}

void CCSBTEBulletinScroll::OnScrollBarSliderMoved()
{
	InvalidateLayout();

	SwitchToLine(GetValue());
}

void CCSBTEBulletinScroll::SwitchToLine(int line)
{
	GetParent()->OnCommand(va("Bulletin_SwitchLine_%d", line));
}

CCSBTEBulletinImage::CCSBTEBulletinImage(Panel* parent, const char* name) : ImagePanel(parent, name)
{
	SetImage(scheme()->GetImage("gfx/ui/panel/basket_blank_slot", false));
	SetShouldScaleImage(true);
	SetSize(204, 44);

	m_pActiveImage = new CCSBTEBulletinFrameImage(this, "CSOBulletInActiveImage");
	m_pActiveImage->SetVisible(false);
	m_pActiveImage->SetBounds(0, 0, 204, 44);
	m_pActiveImage->SetMouseInputEnabled(false);

	m_pMouseImage = new ImagePanel(this, "CSOBulletInMouseImage");
	m_pMouseImage->SetVisible(false);
	m_pMouseImage->SetImage(scheme()->GetImage("resource/pcbangpremium_bg", false));
	m_pMouseImage->SetShouldScaleImage(true);
	m_pMouseImage->SetBounds(0, 0, 204, 44);
	m_pMouseImage->SetMouseInputEnabled(false);

	m_pTitle = new Label(this, "BulletItTitleText", "Text");
	m_pTitle->SetBounds(2, 0, parent->GetWide(), 25);
	m_pTitle->SetMouseInputEnabled(false);

	m_pDate = new Label(this, "BulletInDateText", "Date");
	m_pDate->SetBounds(2, 15, parent->GetWide(), 25);
	m_pDate->SetMouseInputEnabled(false);
}

void CCSBTEBulletinImage::ApplySchemeSettings(IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	vgui2::HFont font = pScheme->GetFont("Default", IsProportional());
	if (font)
		m_pTitle->SetFont(font);

	m_pTitle->SetFgColor(Color{ 255, 255, 255, 255 });
}

void CCSBTEBulletinImage::OnMousePressed(vgui2::MouseCode code)
{
	if (!m_bCanUse)
		return;

	if (code == MOUSE_LEFT)
	{
		char szCommand[21];
		sprintf(szCommand, "Bulletin_SetPage_%d", m_iId);
		GetParent()->OnCommand(szCommand);
	}
}

void CCSBTEBulletinImage::OnCursorEntered()
{
	if (!m_bCanUse)
		return;

	m_pMouseImage->SetVisible(true);
}

void CCSBTEBulletinImage::OnCursorExited()
{
	if (!m_bCanUse)
		return;

	m_pMouseImage->SetVisible(false);
}

void CCSBTEBulletinImage::SetBulletinText(char* szTitle, char* szDate)
{
	if (szTitle)
	{
		if (szTitle[0] == '#')
		{
			wchar_t* p = localize()->Find(szTitle);

			if (p)
				m_pTitle->SetText(p);
			else
				m_pTitle->SetText(szTitle);
		}
		else
			m_pTitle->SetText(szTitle);
	}

	if (szDate)
	{
		if (szDate[0] == '#')
		{
			wchar_t* p = localize()->Find(szDate);

			if (p)
				m_pDate->SetText(p);
			else
				m_pDate->SetText(szDate);
		}
		else
			m_pDate->SetText(szDate);
	}
}

void CCSBTEBulletinImage::SetBulletinText(wchar_t* szTitle, wchar_t* szDate)
{
	if (szTitle)
	{
		if (szTitle[0] == '#')
		{
			static char szUTF8[128];
			Q_UnicodeToUTF8(szTitle, szUTF8, sizeof(szUTF8));

			wchar_t* p = localize()->Find(szUTF8);

			if (p)
				m_pTitle->SetText(p);
			else
				m_pTitle->SetText(szUTF8);
		}
		else
			m_pTitle->SetText(szTitle);
	}

	if (szDate)
	{
		if (szDate[0] == '#')
		{
			static char szUTF8[128];
			Q_UnicodeToUTF8(szDate, szUTF8, sizeof(szUTF8));

			wchar_t* p = localize()->Find(szUTF8);

			if (p)
				m_pDate->SetText(p);
			else
				m_pDate->SetText(szUTF8);
		}
		else
			m_pDate->SetText(szDate);
	}
}

CUtlSymbolTable g_ColumnSoundNames;

CCSBTEBulletinColumn::CCSBTEBulletinColumn(vgui2::Panel* parent, const char* panelName, Panel* pActionSignalTarget, const char* text, const char* pCmd) : Panel(parent, panelName)
{
	Initialize();

	if (pActionSignalTarget && pCmd)
	{
		AddActionSignalTarget(pActionSignalTarget);
		AddColumn(text, pCmd);
	}
}

CCSBTEBulletinColumn::CCSBTEBulletinColumn(vgui2::Panel* parent, const char* panelName, Panel* pActionSignalTarget, const wchar_t* text, const char* pCmd) : Panel(parent, panelName)
{
	Initialize();

	if (pActionSignalTarget && pCmd)
	{
		AddActionSignalTarget(pActionSignalTarget);
		AddColumn(text, pCmd);
	}
}

CCSBTEBulletinColumn::~CCSBTEBulletinColumn()
{
	for (int i = 0; i < _columnList.Size(); i++)
	{
		if (_columnList[i].textImage)
			delete _columnList[i].textImage;

		if (_columnList[i].actionMessage)
			_columnList[i].actionMessage->deleteThis();
	}
}

void CCSBTEBulletinColumn::Initialize()
{
	_contentAlignment = a_center;
	m_bWrap = false;

	m_sClickSoundName = UTL_INVAL_SYMBOL;

	_flag = COLUMN_NO_FLAG;
	_state = STATE_DEFAULT;
	_pointed = -1;
	_pressing = -1;
	_size = 0;

	SetTextInset(6, 0);
	SetMouseClickEnabled(MOUSE_LEFT, true);

	SetPaintBackgroundEnabled(true);
	SetDisplayType(COLUMN_DISPLAY_DEFAULT);
}

void CCSBTEBulletinColumn::SetColumnSize(int size)
{
	if (size < 0)
		return;

	int w, t;
	GetSize(w, t);

	if (_size)
		BaseClass::SetSize(w / _size * size, t);
	else
		BaseClass::SetSize(w * size, t);

	_size = size;
}

int CCSBTEBulletinColumn::GetColumnSize()
{
	return _size;
}

void CCSBTEBulletinColumn::SetWide(int wide, bool resize)
{
	if (!resize)
		BaseClass::SetWide(wide);
	else
		BaseClass::SetWide(wide * _size);
}

void CCSBTEBulletinColumn::SetSize(int wide, int tall, bool resize)
{
	if (!resize)
		BaseClass::SetSize(wide, tall);
	else
		BaseClass::SetSize(wide * _size, tall);
}

void CCSBTEBulletinColumn::SetBounds(int x, int y, int wide, int tall, bool resize)
{
	if (!resize)
		BaseClass::SetBounds(x, y, wide, tall);
	else
		BaseClass::SetBounds(x, y, wide * _size, tall);
}

int CCSBTEBulletinColumn::AddColumn(const char* text, const char* cmd)
{
	int pos = _columnList.AddToTail();
	_columnList[pos].isImage = false;
	memset(_columnList[pos].pImage, NULL, sizeof(_columnList[pos].pImage));
	_columnList[pos].textImage = new TextImage(text);
	_columnList[pos].textImage->SetColor(Color(0, 0, 0, 0));
	_columnList[pos].actionMessage = new KeyValues("Command", "command", cmd);

	SetColumnSize(_size + 1);

	InvalidateLayout();
	return pos;
}

int CCSBTEBulletinColumn::AddColumn(const wchar_t* text, const  char* cmd)
{
	int pos = _columnList.AddToTail();
	_columnList[pos].isImage = false;
	memset(_columnList[pos].pImage, NULL, sizeof(_columnList[pos].pImage));
	_columnList[pos].textImage = new TextImage(text);
	_columnList[pos].textImage->SetColor(Color(0, 0, 0, 0));
	_columnList[pos].actionMessage = new KeyValues("Command", "command", cmd);

	SetColumnSize(_size + 1);

	InvalidateLayout();
	return pos;
}

int CCSBTEBulletinColumn::AddColumn(const char* text, KeyValues* msg)
{
	int pos = _columnList.AddToTail();
	_columnList[pos].isImage = false;
	memset(_columnList[pos].pImage, NULL, sizeof(_columnList[pos].pImage));
	_columnList[pos].textImage = new TextImage(text);
	_columnList[pos].textImage->SetColor(Color(0, 0, 0, 0));
	_columnList[pos].actionMessage = msg;

	SetColumnSize(_size + 1);

	InvalidateLayout();
	return pos;
}

int CCSBTEBulletinColumn::AddColumn(const wchar_t* text, KeyValues* msg)
{
	int pos = _columnList.AddToTail();
	_columnList[pos].isImage = false;
	memset(_columnList[pos].pImage, NULL, sizeof(_columnList[pos].pImage));
	_columnList[pos].textImage = new TextImage(text);
	_columnList[pos].textImage->SetColor(Color(0, 0, 0, 0));
	_columnList[pos].actionMessage = msg;

	SetColumnSize(_size + 1);

	InvalidateLayout();
	return pos;
}

int CCSBTEBulletinColumn::AddTexturedColumn(const char* texture[4], const char* cmd)
{
	return AddTexturedColumn(cmd, texture[0], texture[1], texture[2], texture[3]);
}

int CCSBTEBulletinColumn::AddTexturedColumn(const char* texture[4], KeyValues* msg)
{
	return AddTexturedColumn(msg, texture[0], texture[1], texture[2], texture[3]);
}

int CCSBTEBulletinColumn::AddTexturedColumn(const char* cmd, const char* texdefault, const char* texfocus, const char* texpress, const char* texdisabled)
{
	int pos = _columnList.AddToTail();
	memset(_columnList[pos].pImage, 0, sizeof(_columnList[pos].pImage));
	_columnList[pos].isImage = true;

	for (int i = 0; i < 4; i++)
	{
		const char* ptexture = nullptr;
		switch (i)
		{
		case 0:ptexture = texdefault; break;
		case 1:ptexture = texfocus; break;
		case 2:ptexture = texpress; break;
		case 3:ptexture = texdisabled; break;
		}

		if (ptexture && ptexture[0])
			_columnList[pos].pImage[i] = scheme()->GetImage(ptexture, true);
	}
	_columnList[pos].textImage = nullptr;
	_columnList[pos].actionMessage = new KeyValues("Command", "command", cmd);

	SetColumnSize(_size + 1);

	InvalidateLayout();
	return pos;
}

int CCSBTEBulletinColumn::AddTexturedColumn(KeyValues* msg, const char* texdefault, const char* texfocus, const char* texpress, const char* texdisabled)
{
	int pos = _columnList.AddToTail();
	memset(_columnList[pos].pImage, 0, sizeof(_columnList[pos].pImage));

	for (int i = 0; i < 4; i++)
	{
		const char* ptexture = nullptr;
		switch (i)
		{
		case 0:ptexture = texdefault; break;
		case 1:ptexture = texfocus; break;
		case 2:ptexture = texpress; break;
		case 3:ptexture = texdisabled; break;
		}
		if (ptexture && ptexture[0])
			_columnList[pos].pImage[i] = scheme()->GetImage(ptexture, true);
	}
	_columnList[pos].textImage = nullptr;
	_columnList[pos].actionMessage = msg;

	SetColumnSize(_size + 1);

	InvalidateLayout();
	return pos;
}

int CCSBTEBulletinColumn::AddTexturedColumn(vgui2::IImage* pImage[4], const char* cmd)
{
	int pos = _columnList.AddToTail();
	_columnList[pos].isImage = true;
	for (int i = 0; i < 4; i++)
		_columnList[pos].pImage[i] = pImage[i];
	_columnList[pos].textImage = nullptr;
	_columnList[pos].actionMessage = new KeyValues("Command", "command", cmd);

	SetColumnSize(_size + 1);

	InvalidateLayout();
	return pos;
}

int CCSBTEBulletinColumn::AddTexturedColumn(vgui2::IImage* pImage[4], KeyValues* msg)
{
	int pos = _columnList.AddToTail();
	_columnList[pos].isImage = true;
	for (int i = 0; i < 4; i++)
		_columnList[pos].pImage[i] = pImage[i];
	_columnList[pos].textImage = nullptr;
	_columnList[pos].actionMessage = msg;

	SetColumnSize(_size + 1);

	InvalidateLayout();
	return pos;
}

bool CCSBTEBulletinColumn::IsTexturedColumn(int columnID)
{
	if (!_columnList.IsValidIndex(columnID))
		return false;

	return _columnList[columnID].isImage;
}

int CCSBTEBulletinColumn::FindColumn(const char* key, bool bCmd)
{
	if (_columnList.Size() <= 0)
		return -1;

	char temp[64];
	for (int i = 0; i < _columnList.Size(); i++)
	{
		if (_columnList[i].isImage)
			continue;

		if (!_columnList[i].textImage)
			continue;

		if (!bCmd) // Text
		{
			_columnList[i].textImage->GetText(temp, sizeof(temp));
			if (!Q_strncmp(key, temp, sizeof(temp)))
				return i;
		}
		else // Command
		{
			if (!Q_strcmp(_columnList[i].actionMessage->GetString("command"), key))
				return i;
		}
	}
	return -1;
}

int CCSBTEBulletinColumn::FindColumn(const wchar_t* text)
{
	if (_columnList.Size() <= 0)
		return -1;

	wchar_t temp[64];
	for (int i = 0; i < _columnList.Size(); i++)
	{
		if (_columnList[i].isImage)
			continue;

		if (!_columnList[i].textImage)
			continue;

		_columnList[i].textImage->GetText(temp, sizeof(temp));
		if (!wcsncmp(text, temp, sizeof(temp)))
			return i;
	}
	return -1;
}

int CCSBTEBulletinColumn::FindTexturedColumn(char* texture, int type)
{
	if (_columnList.Size() <= 0)
		return -1;

	IImage* pImage = scheme()->GetImage(texture, true);
	for (int i = 0; i < _columnList.Size(); i++)
	{
		if (!_columnList[i].isImage)
			continue;

		if (type == -1)
		{
			for (int j = 0; j < 4; j++)
			{
				if (_columnList[i].pImage[j] == pImage)
					return i;
			}
		}
		else if (_columnList[i].pImage[type] == pImage)
			return i;
	}
	return -1;
}

bool CCSBTEBulletinColumn::DestroyColumn(int iPos)
{
	if (iPos < 0 || iPos > _columnList.Size())
		return false;

	if (_columnList[iPos].textImage)
		delete _columnList[iPos].textImage;

	if (_columnList[iPos].actionMessage)
		_columnList[iPos].actionMessage->deleteThis();

	_columnList.Remove(iPos);
	return true;
}

bool CCSBTEBulletinColumn::DestroyColumn(char* text)
{
	int iSearch = FindColumn(text);
	if (iSearch == -1)
		return false;

	if (_columnList[iSearch].textImage)
		delete _columnList[iSearch].textImage;

	if (_columnList[iSearch].actionMessage)
		_columnList[iSearch].actionMessage->deleteThis();

	_columnList.Remove(iSearch);
	return true;
}

bool CCSBTEBulletinColumn::DestroyColumn(wchar_t* text)
{
	int iSearch = FindColumn(text);
	if (iSearch == -1)
		return false;

	if (_columnList[iSearch].textImage)
		delete _columnList[iSearch].textImage;

	if (_columnList[iSearch].actionMessage)
		_columnList[iSearch].actionMessage->deleteThis();

	_columnList.Remove(iSearch);
	return true;
}

void CCSBTEBulletinColumn::SetText(int index, const char* text)
{
	if (!_columnList.IsValidIndex(index))
		return;

	if (!text)
	{
		text = "";
	}

	if (_columnList[index].isImage)
		return;

	if (!_columnList[index].textImage)
		return;

	// let the text image do the translation itself
	_columnList[index].textImage->SetText(text);

	if (text[0] == '#')
	{
		SetHotkey(CalculateHotkey(localize()->Find(text)));
	}
	else
	{
		SetHotkey(CalculateHotkey(text));
	}
	InvalidateLayout();
	Repaint();
}

void CCSBTEBulletinColumn::SetText(int index, const wchar_t* unicodeString, bool bClearUnlocalizedSymbol)
{
	if (!_columnList.IsValidIndex(index))
		return;

	if (_columnList[index].isImage)
		return;

	if (!_columnList[index].textImage)
		return;

	_columnList[index].textImage->SetText(unicodeString, bClearUnlocalizedSymbol);

	//!! need to calculate hotkey from translated string
	SetHotkey(CalculateHotkey(unicodeString));

	InvalidateLayout();     // possible that the textimage needs to expand
	Repaint();
}

void CCSBTEBulletinColumn::GetText(int index, char* textOut, int bufferLen)
{
	if (!_columnList.IsValidIndex(index))
		return;

	if (_columnList[index].isImage)
		return;

	if (!_columnList[index].textImage)
		return;

	_columnList[index].textImage->GetText(textOut, bufferLen);
}

void CCSBTEBulletinColumn::GetText(int index, wchar_t* textOut, int bufLenInBytes)
{
	if (!_columnList.IsValidIndex(index))
		return;

	if (_columnList[index].isImage)
		return;

	if (!_columnList[index].textImage)
		return;

	_columnList[index].textImage->GetText(textOut, bufLenInBytes);
}

void CCSBTEBulletinColumn::SetColumnFlag(int flag)
{
	if (!flag)
		_flag = COLUMN_NO_FLAG;

	_flag |= (1 << flag);
}

void CCSBTEBulletinColumn::UnSetColumnFlag(int flag)
{
	_flag &= ~(1 << flag);
}

int CCSBTEBulletinColumn::GetColumnFlag()
{
	return _flag;
}

void CCSBTEBulletinColumn::GetContentSize(int index, int& wide, int& tall)
{
	if (!_columnList.IsValidIndex(index))
		return;

	if (GetFont() == INVALID_FONT)
	{
		IScheme* pScheme = scheme()->GetIScheme(GetScheme());
		if (pScheme)
		{
			SetFont(pScheme->GetFont("Default", IsProportional()));
		}
	}

	if (!_columnList[index].isImage)
	{
		int tx0, ty0, tx1, ty1;
		ComputeAlignment(index, tx0, ty0, tx1, ty1);
		wide = (tx1 - tx0) + _textInset[0];

		int iWide, iTall;
		_columnList[index].textImage->GetSize(iWide, iTall);
		wide -= iWide;
		_columnList[index].textImage->GetContentSize(iWide, iTall);
		wide += iWide;

		tall = max((ty1 - ty0) + _textInset[1], iTall);
	}
	else
	{
		for (int i = 0; i < 4; i++)
		{
			if (_columnList[index].pImage[i])
			{
				_columnList[index].pImage[i]->GetContentSize(wide, tall);
				break;
			}
		}
	}
}

void CCSBTEBulletinColumn::SetContentAlignment(Alignment alignment)
{
	_contentAlignment = alignment;
	Repaint();
}

void CCSBTEBulletinColumn::SetTextInset(int xInset, int yInset)
{
	_textInset[0] = xInset;
	_textInset[1] = yInset;

	int wide, tall;
	GetSize(wide, tall);

	if (_size)
		wide /= _size;

	for (int i = 0; i < _columnList.Size(); i++)
	{
		if (!_columnList[i].isImage)
			_columnList[i].textImage->SetDrawWidth(wide - _textInset[0]);
	}
}

void CCSBTEBulletinColumn::SetFont(HFont font)
{
	for (int i = 0; i < _columnList.Size(); i++)
	{
		if (!_columnList[i].isImage)
			_columnList[i].textImage->SetFont(font);
	}

	Repaint();
}

HFont CCSBTEBulletinColumn::GetFont()
{
	if (!_columnList.IsValidIndex(0))
		return INVALID_FONT;

	for (int i = 0; i < _columnList.Size(); i++)
	{
		if (!_columnList[i].isImage)
			return _columnList[0].textImage->GetFont();
	}

	return INVALID_FONT;
}

Panel* CCSBTEBulletinColumn::HasHotkey(wchar_t key)
{
#ifdef VGUI_HOTKEYS_ENABLED
	if (iswalnum(key))
		key = towlower(key);

	if (_hotkey == key)
		return this;
#endif

	return NULL;
}

void CCSBTEBulletinColumn::SetHotkey(wchar_t ch)
{
	_hotkey = ch;
}

TextImage* CCSBTEBulletinColumn::GetTextImage(int index)
{
	if (!_columnList.IsValidIndex(index))
		return NULL;

	if (_columnList[index].isImage)
		return NULL;

	return _columnList[index].textImage;
}

void CCSBTEBulletinColumn::SetOnFocus(int index, bool state)
{
	if (!_columnList.IsValidIndex(index))
		return;

	if (_flag & (1 << COLUMN_NO_FOCUS_EFFECT))
		return;

	if (state)
	{
		_pointed = index;
	}
	else if (index == _pointed)
		_pointed = -1;

	_state = state ? STATE_ONFOCUS : STATE_DEFAULT;

	for (int i = 0; i < _size; i++)
		SetFgColor(i, GetColumnFgColor(i));
}

bool CCSBTEBulletinColumn::IsOnFocus()
{
	return (_state == STATE_ONFOCUS) ? true : false;
}

void CCSBTEBulletinColumn::SetClicked(int index, bool state)
{
	if (!_columnList.IsValidIndex(index))
		return;

	if (_flag & (1 << COLUMN_NO_PRESS_EFFECT))
		return;

	if (index != _pointed && state)
		return;

	_state = state ? STATE_CLICKED : STATE_DEFAULT;

	for (int i = 0; i < _size; i++)
		SetFgColor(i, GetColumnFgColor(i));
}

bool CCSBTEBulletinColumn::IsClicked()
{
	return (_state == STATE_CLICKED) ? true : false;
}

void CCSBTEBulletinColumn::SetActive(int index, bool state)
{
	if (!_columnList.IsValidIndex(index))
		return;

	if (_flag & (1 << COLUMN_NO_ACTIVE_EFFECT))
		return;

	if (state)
	{
		_pressing = index;
	}
	else if (index == _pressing)
	{
		_pressing = -1;
	}

	for (int i = 0; i < _size; i++)
		SetFgColor(i, GetColumnFgColor(i));
}

bool CCSBTEBulletinColumn::IsActive()
{
	return false;
}

int CCSBTEBulletinColumn::CalcCursor()
{
	int x, y, x0, y0;

	input()->GetCursorPos(x, y);
	GetPos(x0, y0);

	x -= x0;
	y -= y0;

	if (GetParent())
	{
		GetParent()->GetPos(x0, y0);

		x -= x0;
		y -= y0;
	}

	GetSize(x0, y0);

	if (x > x0 || y > y0)
		return -1;

	int pos = x / (x0 / _size);

	return pos;
}

void CCSBTEBulletinColumn::DoClick()
{
	int pos = CalcCursor();
	if (_pointed != pos)
	{
		SetOnFocus(_pointed, false);
		SetOnFocus(pos, true);
		return;
	}

	if (m_sClickSoundName != UTL_INVAL_SYMBOL)
	{
		surface()->PlaySound(g_ColumnSoundNames.String(m_sClickSoundName));
	}

	SetActive(pos, true);
	FireActionSignal();
}

void CCSBTEBulletinColumn::OnCursorMoved(int x, int y)
{
	int pos = CalcCursor();
	if (pos != _pointed)
	{
		if (_pointed > 0 && _pressing != _pointed)
			SetOnFocus(_pointed, false);

		OnCursorEntered();
	}
}

void CCSBTEBulletinColumn::OnCursorEntered()
{
	SetOnFocus(CalcCursor(), true);
}

void CCSBTEBulletinColumn::OnCursorExited()
{
	SetOnFocus(_pointed, false);
}

void CCSBTEBulletinColumn::SetMouseClickEnabled(MouseCode code, bool state)
{
	if (state)
	{
		_mouseClickMask |= 1 << ((int)(code + 1));
	}
	else
	{
		_mouseClickMask &= ~(1 << ((int)(code + 1)));
	}
}

bool CCSBTEBulletinColumn::IsMouseClickEnabled(MouseCode code)
{
	if (_mouseClickMask & (1 << ((int)(code + 1))))
	{
		return true;
	}
	return false;
}

void CCSBTEBulletinColumn::SetDefaultColor(Color fgColor)
{
	if (!(_defaultFgColor == fgColor))
	{
		_defaultFgColor = fgColor;
		InvalidateLayout(false);
	}
}

void CCSBTEBulletinColumn::SetOnFocusColor(Color fgColor)
{
	if (!(_focusFgColor == fgColor))
	{
		_focusFgColor = fgColor;
		InvalidateLayout(false);
	}
}

void CCSBTEBulletinColumn::SetClickedColor(Color fgColor)
{
	if (!(_clickedFgColor == fgColor))
	{
		_clickedFgColor = fgColor;
		InvalidateLayout(false);
	}
}

void CCSBTEBulletinColumn::SetDisabledFgColor1(Color color)
{
	_disabledFgColor1 = color;
}

void CCSBTEBulletinColumn::SetDisabledFgColor2(Color color)
{
	_disabledFgColor2 = color;
}

Color CCSBTEBulletinColumn::GetDisabledFgColor1()
{
	return _disabledFgColor1;
}

Color CCSBTEBulletinColumn::GetDisabledFgColor2()
{
	return _disabledFgColor2;
}

Color CCSBTEBulletinColumn::GetColumnFgColor(int index)
{
	if (_pressing == index)
		return _focusFgColor;

	if (IsClicked() && _pointed == index)
		return _clickedFgColor;

	if (IsOnFocus() && _pointed == index)
		return _focusFgColor;

	return _defaultFgColor;
}

void CCSBTEBulletinColumn::SetFgColor(int index, Color color)
{
	bool repaint = false;
	if (!index && !(GetFgColor() == color))
	{
		BaseClass::SetFgColor(color);
		repaint = true;
	}

	if (_columnList[index].isImage)
		return;

	if (!(_columnList[index].textImage->GetColor() == color))
	{
		_columnList[index].textImage->SetColor(color);
		repaint = true;
	}

	if (repaint)
		Repaint();
}

void CCSBTEBulletinColumn::SetImageBorderByName(IScheme* pScheme, char* szName)
{
	if (!szName)
		return;

	IImage* pImage = scheme()->GetImage(va("%s_left_c", szName), true);

	if (pImage)
	{
		_imageborder = true;
		_clickedImage[0] = scheme()->GetImage(va("%s_left_c", szName), true);
		_clickedImage[1] = scheme()->GetImage(va("%s_center_c", szName), true);
		_clickedImage[2] = scheme()->GetImage(va("%s_right_c", szName), true);
		_defaultImage[0] = scheme()->GetImage(va("%s_left_n", szName), true);
		_defaultImage[1] = scheme()->GetImage(va("%s_center_n", szName), true);
		_defaultImage[2] = scheme()->GetImage(va("%s_right_n", szName), true);
		_focusImage[0] = scheme()->GetImage(va("%s_left_o", szName), true);
		_focusImage[1] = scheme()->GetImage(va("%s_center_o", szName), true);
		_focusImage[2] = scheme()->GetImage(va("%s_right_o", szName), true);
	}
}

void CCSBTEBulletinColumn::SetImageBorderByString(IScheme* pScheme, char* szName)
{
	char szResource[32];
	strcpy(szResource, szName);

	strcat(szResource, "/LeftC");

	const char* enableImage = pScheme->GetResourceString(szResource);

	if (enableImage[0])
	{
		_imageborder = true;
		char key[2];
		for (int i = 0; i < 3; i++)
		{
			IImage** ppimage;

			switch (i)
			{
			case 0:
			{
				ppimage = _clickedImage;
				sprintf(key, "C");
				break;
			}
			case 1:
			{
				ppimage = _defaultImage;
				sprintf(key, "N");
				break;
			}
			case 2:
			{
				ppimage = _focusImage;
				sprintf(key, "O");
				break;
			}
			}

			Q_strcpy(szResource, szName);
			strcat(szResource, "/Left");
			strcat(szResource, key);
			ppimage[0] = scheme()->GetImage(pScheme->GetResourceString(szResource), true);

			Q_strcpy(szResource, szName);
			strcat(szResource, "/Center");
			strcat(szResource, key);
			ppimage[1] = scheme()->GetImage(pScheme->GetResourceString(szResource), true);

			Q_strcpy(szResource, szName);
			strcat(szResource, "/Right");
			strcat(szResource, key);
			ppimage[2] = scheme()->GetImage(pScheme->GetResourceString(szResource), true);
		}
	}
}

void CCSBTEBulletinColumn::SetCommand(int index, const char* command)
{
	if (!_columnList.IsValidIndex(index))
		return;

	SetCommand(index, new KeyValues("Command", "command", command));
}

void CCSBTEBulletinColumn::SetCommand(int index, KeyValues* message)
{
	if (!_columnList.IsValidIndex(index))
		return;

	if (_columnList[index].actionMessage)
	{
		_columnList[index].actionMessage->deleteThis();
	}

	_columnList[index].actionMessage = message;
}

void CCSBTEBulletinColumn::SetClickSound(const char* sound)
{
	if (sound)
	{
		m_sClickSoundName = g_ColumnSoundNames.AddString(sound);
	}
	else
	{
		m_sClickSoundName = UTL_INVAL_SYMBOL;
	}
}

void CCSBTEBulletinColumn::SetDisplayType(DisplayType type)
{
	_displaytype = type;
}

void CCSBTEBulletinColumn::FireActionSignal()
{
	int current = _pointed;

	if (!_columnList.IsValidIndex(current))
		return;

	if (_columnList[current].actionMessage)
	{
		// see if it's a url
		if (!stricmp(_columnList[current].actionMessage->GetName(), "command")
			&& !strnicmp(_columnList[current].actionMessage->GetString("command", ""), "url ", strlen("url "))
			&& strstr(_columnList[current].actionMessage->GetString("command", ""), "://"))
		{
			// it's a command to launch a url, run it
			vgui2::system()->ShellExecute("open", _columnList[current].actionMessage->GetString("command", "      ") + 4);
		}
		PostActionSignal(_columnList[current].actionMessage->MakeCopy());
	}
}

void CCSBTEBulletinColumn::PerformLayout()
{
	for (int i = 0; i < _size; i++)
	{
		if (_columnList[i].isImage)
			continue;

		SetFgColor(i, GetColumnFgColor(i));

		int wide, tall;
		Panel::GetSize(wide, tall);
		wide -= _textInset[0]; // take inset into account

		if (m_bWrap)
		{
			int twide, ttall;
			_columnList[i].textImage->GetContentSize(twide, ttall);
			_columnList[i].textImage->SetSize(wide, ttall);
		}
		else
		{
			int twide, ttall;
			_columnList[i].textImage->GetContentSize(twide, ttall);

			// tell the textImage how much space we have to draw in
			if (wide < twide)
				_columnList[i].textImage->SetSize(wide, ttall);
			else
				_columnList[i].textImage->SetSize(twide, ttall);
		}
	}

	BaseClass::PerformLayout();
}

wchar_t CCSBTEBulletinColumn::CalculateHotkey(const char* text)
{
	for (const char* ch = text; *ch != 0; ch++)
	{
		if (*ch == '&')
		{
			// get the next character
			ch++;

			if (*ch == '&')
			{
				// just an &
				continue;
			}
			else if (*ch == 0)
			{
				break;
			}
			else if (isalnum(*ch))
			{
				// found the hotkey
				return (wchar_t)tolower(*ch);
			}
		}
	}

	return '\0';
}

wchar_t CCSBTEBulletinColumn::CalculateHotkey(const wchar_t* text)
{
	if (text)
	{
		for (const wchar_t* ch = text; *ch != 0; ch++)
		{
			if (*ch == '&')
			{
				// get the next character
				ch++;

				if (*ch == '&')
				{
					// just an &
					continue;
				}
				else if (*ch == 0)
				{
					break;
				}
				else if (iswalnum(*ch))
				{
					// found the hotkey
					return (wchar_t)towlower(*ch);
				}
			}
		}
	}

	return '\0';
}

void CCSBTEBulletinColumn::ComputeAlignment(int index, int& tx0, int& ty0, int& tx1, int& ty1)
{
	if (!_columnList.IsValidIndex(index))
		return;

	if (_columnList[index].isImage)
		return;

	int wide, tall;
	GetPaintSize(wide, tall);

	if (_size)
		wide /= _size;

	int tWide, tTall;

	// text bounding box
	tx0 = 0;
	ty0 = 0;

	// loop through all the images and calculate the complete bounds
	int maxX = 0, maxY = 0;

	int actualXAlignment = _contentAlignment;

	if (!_columnList[index].textImage)
		return;

	_columnList[index].textImage->GetSize(tWide, tTall);
	if (tWide > wide)
		actualXAlignment = Label::a_west;

	// x align text
	switch (actualXAlignment)
	{
		// left
	case Label::a_northwest:
	case Label::a_west:
	case Label::a_southwest:
	{
		tx0 = 0;
		break;
	}
	// center
	case Label::a_north:
	case Label::a_center:
	case Label::a_south:
	{
		tx0 = (wide - tWide) / 2;
		break;
	}
	// right
	case Label::a_northeast:
	case Label::a_east:
	case Label::a_southeast:
	{
		tx0 = wide - tWide;
		break;
	}
	}

	// y align text
	switch (_contentAlignment)
	{
		//top
	case Label::a_northwest:
	case Label::a_north:
	case Label::a_northeast:
	{
		ty0 = 0;
		break;
	}
	// center
	case Label::a_west:
	case Label::a_center:
	case Label::a_east:
	{
		ty0 = (tall - tTall) / 2;
		break;
	}
	// south
	case Label::a_southwest:
	case Label::a_south:
	case Label::a_southeast:
	{
		ty0 = tall - tTall;
		break;
	}
	}

	tx1 = tx0 + tWide;
	ty1 = ty0 + tTall;
}

void CCSBTEBulletinColumn::Paint()
{
	int labelWide, labelTall;
	GetSize(labelWide, labelTall);

	if (_size)
		labelWide /= _size;

	for (int i = 0; i < _columnList.Size(); i++)
	{
		if (!_columnList[i].isImage)
		{
			if (!_columnList[i].textImage)
				continue;

			int tx0, ty0, tx1, ty1;
			ComputeAlignment(i, tx0, ty0, tx1, ty1);

			int x = labelWide * i + tx0, y = _textInset[1] + ty0;
			int imageYPos = 0; // a place to save the y offset for when we draw the disable version of the image

			switch (_contentAlignment)
			{
				// left
			case Label::a_northwest:
			case Label::a_west:
			case Label::a_southwest:
			{
				x += _textInset[0];
				break;
			}
			// right
			case Label::a_northeast:
			case Label::a_east:
			case Label::a_southeast:
			{
				x -= _textInset[0];
				break;
			}
			}

			switch (_displaytype)
			{
			case COLUMN_DISPLAY_DEFAULT:
			{
				if (_pressing != i)
					y += 2;
				break;
			}
			case COLUMN_DISPLAY_CHAT:
			{
				if (_pressing != i)
					y += 1;
				break;
			}
			}

			// draw
			imageYPos = y;
			_columnList[i].textImage->SetPos(x, y);

			if (_contentAlignment == Label::a_west || _contentAlignment == Label::a_center || _contentAlignment == Label::a_east)
			{
				int iw, it;
				_columnList[i].textImage->GetSize(iw, it);
				if (it < (ty1 - ty0))
				{
					imageYPos = ((ty1 - ty0) - it) / 2 + y;
					_columnList[i].textImage->SetPos(x, ((ty1 - ty0) - it) / 2 + y);
				}
			}

			if (IsEnabled())
			{
				_columnList[i].textImage->SetColor(GetColumnFgColor(i));
				_columnList[i].textImage->Paint();
			}
			else
			{
				// draw disabled version, with embossed look
				// offset image
				_columnList[i].textImage->SetPos(x + 1, imageYPos + 1);
				_columnList[i].textImage->SetColor(_disabledFgColor1);
				_columnList[i].textImage->Paint();

				surface()->DrawFlushText();

				// overlayed image
				_columnList[i].textImage->SetPos(x, imageYPos);
				_columnList[i].textImage->SetColor(_disabledFgColor2);
				_columnList[i].textImage->Paint();
			}

			surface()->DrawFlushText();
		}
		else
		{
			IImage* pImage = nullptr;

			if (!IsEnabled())
				pImage = _columnList[i].pImage[3];
			else
			{
				if (_pressing == i)
					pImage = _columnList[i].pImage[1];
				else if (IsClicked() && _pointed == i)
					pImage = _columnList[i].pImage[2];
				else if (IsOnFocus() && _pointed == i)
					pImage = _columnList[i].pImage[1];
			}

			if (!pImage)
			{
				if (!_columnList[i].pImage[0])
					continue;

				pImage = _columnList[i].pImage[0];
			}

			int iw, it;
			pImage->GetSize(iw, it);
			int x = labelWide * i + max((labelWide - iw), 0) / 2;
			int y = max((labelTall - it), 0) / 2;

			pImage->SetPos(x, y);
			pImage->Paint();
		}
	}
}

void CCSBTEBulletinColumn::PaintBackground()
{
	if (_imageborder)
	{
		int wide, tall;
		GetSize(wide, tall);

		if (_size)
			wide /= _size;

		int x = 0;

		for (int i = 0; i < _size; i++)
		{
			IImage** ppimage;

			if (_pressing == i)
				ppimage = _focusImage;
			else if (IsClicked() && _pointed == i)
				ppimage = _clickedImage;
			else if (IsOnFocus() && _pointed == i)
				ppimage = _focusImage;
			else
				ppimage = _defaultImage;

			switch (_displaytype)
			{
			case COLUMN_DISPLAY_DEFAULT:
			{
				int offset = 0;

				if (_pressing != i)
					offset = 2;

				ppimage[0]->SetPos(x, offset);
				ppimage[0]->SetSize(10, tall - offset);
				ppimage[0]->Paint();
				ppimage[1]->SetPos(x + 10, offset);
				ppimage[1]->SetSize(wide - 20, tall - offset);
				ppimage[1]->Paint();
				ppimage[2]->SetPos(x + wide - 10, offset);
				ppimage[2]->SetSize(10, tall - offset);
				ppimage[2]->Paint();

				break;
			}
			case COLUMN_DISPLAY_CHAT:
			{
				int offset = 1, offset2 = 0;

				if (_pressing != i)
					offset2 = 2;
				else
					offset = 0;

				ppimage[0]->SetPos(x, offset);
				ppimage[0]->SetSize(10, tall - offset2);
				ppimage[0]->Paint();
				ppimage[1]->SetPos(x + 10, offset);
				ppimage[1]->SetSize(wide - 20, tall - offset2);
				ppimage[1]->Paint();
				ppimage[2]->SetPos(x + wide - 10, offset);
				ppimage[2]->SetSize(10, tall - offset2);
				ppimage[2]->Paint();

				break;
			}
			}

			x += wide;
		}

		return;
	}

	BaseClass::PaintBackground();
}

void CCSBTEBulletinColumn::ApplySchemeSettings(IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	if (GetFont() == INVALID_FONT)
	{
		SetFont(pScheme->GetFont("Default", IsProportional()));
	}

	int xInset = 0, yInset = 0;

	const char* resourceString = pScheme->GetResourceString("Button.TextInsetX");

	if (resourceString[0])
		xInset = atoi(resourceString);

	resourceString = pScheme->GetResourceString("Button.TextInsetY");

	if (resourceString[0])
		yInset = atoi(resourceString);

	SetTextInset(xInset, yInset);

	if (m_bWrap)
	{
		//tell it how big it is
		int wide, tall;
		Panel::GetSize(wide, tall);

		if (_size)
			wide /= _size;
		wide -= _textInset[0];		// take inset into account

		for (int i = 0; i < _size; i++)
		{
			if (_columnList[i].isImage)
				continue;

			_columnList[i].textImage->SetSize(wide, tall);
			_columnList[i].textImage->RecalculateNewLinePositions();
		}
	}
	else
	{
		// if you don't set the size of the image, many, many buttons will break - we might want to look into fixing this all over the place later
		int wide, tall;

		for (int i = 0; i < _size; i++)
		{
			if (_columnList[i].isImage)
				continue;

			_columnList[i].textImage->GetContentSize(wide, tall);
			_columnList[i].textImage->SetSize(wide, tall);
		}
	}

	_defaultFgColor = GetSchemeColor("DisabledText1", GetSchemeColor("BaseText", Color(255, 255, 255, 50), pScheme), pScheme);
	_focusFgColor = GetSchemeColor("BaseText", GetSchemeColor("BaseText", Color(255, 255, 255, 255), pScheme), pScheme);
	_clickedFgColor = GetSchemeColor("BrightBaseText", GetSchemeColor("BaseText", Color(255, 255, 255, 200), pScheme), pScheme);

	_disabledFgColor1 = GetSchemeColor("DisabledText1", GetSchemeColor("BaseText", Color(255, 255, 255, 255), pScheme), pScheme);
	_disabledFgColor2 = GetSchemeColor("DisabledText2", GetSchemeColor("BaseText", Color(255, 255, 255, 255), pScheme), pScheme);

	SetImageBorderByName(pScheme, (char*)"resource/control/listpanel/column");
}

void CCSBTEBulletinColumn::SetWrap(bool bWrap)
{
	m_bWrap = bWrap;
	for (int i = 0; i < _size; i++)
	{
		if (_columnList[i].isImage)
			continue;

		_columnList[i].textImage->SetWrap(m_bWrap);
	}
}

void CCSBTEBulletinColumn::OnMousePressed(MouseCode code)
{
	if (!IsEnabled())
		return;

	if (!IsMouseClickEnabled(code))
		return;

	if (_flag & (1 << COLUMN_NO_PRESS_EFFECT))
	{
		if (IsKeyBoardInputEnabled())
		{
			RequestFocus();
		}

		DoClick();
	}
	else
		SetClicked(CalcCursor(), true);
}

void CCSBTEBulletinColumn::OnMouseReleased(MouseCode code)
{
	if (IsClicked())
	{
		if (IsKeyBoardInputEnabled())
		{
			RequestFocus();
		}

		DoClick();
	}

	SetOnFocus(CalcCursor(), true);
}

void CCSBTEBulletinColumn::OnMouseDoublePressed(MouseCode code)
{
	OnMousePressed(code);
}

void CCSBTEBulletinColumn::OnKeyCodePressed(KeyCode code)
{
	if (code == KEY_SPACE || code == KEY_ENTER)
	{
		OnMousePressed(MOUSE_LEFT);
	}
	else
	{
		BaseClass::OnKeyCodePressed(code);
	}
}

KeyValues* CCSBTEBulletinColumn::GetActionMessage(int index)
{
	if (!_columnList.IsValidIndex(index))
		return NULL;

	if (!_columnList[index].actionMessage)
		return NULL;

	return _columnList[index].actionMessage->MakeCopy();
}

CCSBTEBulletinFrameImage::CCSBTEBulletinFrameImage(Panel* parent, const char* panelName) : Panel(parent, panelName)
{
	SetMouseInputEnabled(false);
	m_bImageBackground = false;
}

void CCSBTEBulletinFrameImage::ApplySchemeSettings(IScheme* pScheme)
{
	SetBgColor(Color(0, 0, 0, 0));

	vgui2::IImage* pImage = scheme()->GetImage("resource/control/selected/selected_top_left", true);

	if (pImage)
	{
		m_bImageBackground = true;
		m_pTopBackground[0] = pImage;
		m_pTopBackground[1] = scheme()->GetImage("resource/control/selected/selected_top_center", true);
		m_pTopBackground[2] = scheme()->GetImage("resource/control/selected/selected_top_right", true);
		m_pCenterBackground[0] = scheme()->GetImage("resource/control/selected/selected_center_left", true);
		m_pCenterBackground[1] = scheme()->GetImage("resource/control/selected/selected_center_center", true);
		m_pCenterBackground[2] = scheme()->GetImage("resource/control/selected/selected_center_right", true);
		m_pBottomBackground[0] = scheme()->GetImage("resource/control/selected/selected_bottom_left", true);
		m_pBottomBackground[1] = scheme()->GetImage("resource/control/selected/selected_bottom_center", true);
		m_pBottomBackground[2] = scheme()->GetImage("resource/control/selected/selected_bottom_right", true);
	}
}

void CCSBTEBulletinFrameImage::PaintBackground()
{
	if (m_bImageBackground)
	{
		int wide, tall;
		GetSize(wide, tall);

		int ewide, etall;
		m_pTopBackground[0]->GetContentSize(ewide, etall);

		m_pTopBackground[0]->SetPos(0, 0);
		m_pTopBackground[0]->SetSize(ewide, etall);
		m_pTopBackground[0]->Paint();
		m_pTopBackground[1]->SetPos(ewide, 0);
		m_pTopBackground[1]->SetSize(wide - ewide * 2, etall);
		m_pTopBackground[1]->Paint();
		m_pTopBackground[2]->SetPos(wide - ewide, 0);
		m_pTopBackground[2]->SetSize(ewide, etall);
		m_pTopBackground[2]->Paint();

		m_pCenterBackground[0]->SetPos(0, etall);
		m_pCenterBackground[0]->SetSize(ewide, tall - etall * 2);
		m_pCenterBackground[0]->Paint();
		m_pCenterBackground[1]->SetPos(ewide, etall);
		m_pCenterBackground[1]->SetSize(wide - ewide * 2, tall - etall * 2);
		m_pCenterBackground[1]->Paint();
		m_pCenterBackground[2]->SetPos(wide - ewide, etall);
		m_pCenterBackground[2]->SetSize(ewide, tall - etall * 2);
		m_pCenterBackground[2]->Paint();

		m_pBottomBackground[0]->SetPos(0, tall - etall);
		m_pBottomBackground[0]->SetSize(ewide, etall);
		m_pBottomBackground[0]->Paint();
		m_pBottomBackground[1]->SetPos(ewide, tall - etall);
		m_pBottomBackground[1]->SetSize(wide - ewide * 2, etall);
		m_pBottomBackground[1]->Paint();
		m_pBottomBackground[2]->SetPos(wide - ewide, tall - etall);
		m_pBottomBackground[2]->SetSize(ewide, etall);
		m_pBottomBackground[2]->Paint();
	}
}