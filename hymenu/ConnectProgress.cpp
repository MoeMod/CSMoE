#include "ConnectProgress.h"
#include "Utils.h"
#include "imgui.h"
#include "imgui_utils.h"

#include "enginecallback_menu.h"

namespace ui
{
	enum EState
	{
		STATE_NONE,
		STATE_MENU, // do not hide when disconnected or in game
		STATE_DOWNLOAD, // enlarge your connectionprogress window
		STATE_CONNECTING, // showing single progress
		STATE_CONSOLE // do not show until state reset
	};

	enum ESource
	{
		SOURCE_CONSOLE,
		SOURCE_SERVERBROWSER,
		SOURCE_CREATEGAME
	};

	struct
	{
		void HandleDisconnect(void)
		{
			if (m_iState == STATE_NONE)
				return;

			if (m_iState == STATE_CONSOLE)
			{
				m_iState = STATE_NONE;
				return;
			}

			if (UI_IsVisible())
			{
				Hide();
				if (m_iSource != SOURCE_CONSOLE && m_iState != STATE_MENU)
				{
					UI_SetActiveMenu(true);
					if (m_iState == STATE_DOWNLOAD)
					{
						Show();
						return;
					}
					m_iSource = SOURCE_CONSOLE;
				}
			}

			SetCommonText("已断开连接.");

			m_iState = STATE_NONE;
		}
		void HandlePrecache(void)
		{
			SetCommonText("Precaching resources");
			m_iState = STATE_CONNECTING;
		}
		void HandleStufftext(float flProgress, const char* pszText)
		{
			SetCommonText(pszText);
			commonProgress = flProgress;
			m_iState = STATE_CONNECTING;
		}
		void HandleDownload(const char* pszFileName, const char* pszServerName, int iCurrent, int iTotal, const char* comment)
		{
			snprintf(sDownloadString, sizeof(sDownloadString) - 1, "正在下载资源 %s", pszFileName);
			snprintf(sCommonString, sizeof(sCommonString) - 1, "%d / %d %s", iCurrent + 1, iTotal, comment);
			m_iState = STATE_DOWNLOAD;
			commonProgress = (float)iCurrent / iTotal + 0.01f / iTotal * EngFuncs::GetCvarFloat("scr_download");
		}
		void SetCommonText(const char* pszText)
		{
			strncpy(sCommonString, pszText, sizeof(sCommonString));
			// snprintf( sCommonString, sizeof( sCommonString ) - 1, "%s", pszText );
		}
		void SetServer(const char* pszName)
		{
			if (m_iSource == SOURCE_CREATEGAME)
			{
				strcpy(sTitleString, "正在开始游戏...");
			}
			else
			{
				snprintf(sTitleString, sizeof(sTitleString) - 1, "正在连接到服务器...");
			}

			commonProgress = 0;
		}

		void Show()
		{
			m_bShow = true;
		}

		void Hide()
		{
			m_bShow = false;
		}

		void Disconnect()
		{
			if (m_iState == STATE_DOWNLOAD)
			{
				EngFuncs::ClientCmd(true, "http_clear\n");
				m_iState = STATE_CONNECTING;
				HandleDisconnect();
			}

			EngFuncs::ClientCmd(false, "cmd disconnect;endgame disconnect\n");
		}

		void OnGUI()
		{
			if ((m_iState != STATE_MENU && CL_IsActive()))
			{
				m_iState = STATE_NONE;
				Hide();
			}

			if (!m_bShow)
				return;

			ImGui::OpenPopup(sTitleString);
			ImGuiUtils::CenterNextWindow(ImGuiCond_Appearing);
			if (ImGui::BeginPopupModal(sTitleString, NULL, ImGuiWindowFlags_AlwaysAutoResize)) {

				ImGui::TextUnformatted(sDownloadString);
				ImGui::TextUnformatted(sCommonString);
				ImGui::ProgressBar(commonProgress, ImGuiUtils::GetScaledSize({ 480, 36 }));

				
				if(m_iState == STATE_NONE)
				{
					if (ImGui::Button("关闭", ImVec2(-1, 0)))
						Hide();
				}
				else
				{
					if (ImGui::Button("断开连接", ImVec2(-1, 0)))
						Disconnect();
				}
				ImGui::EndPopup();
			}
		}

		EState m_iState = STATE_NONE;
		ESource m_iSource = SOURCE_CONSOLE;
		bool m_bShow = false;

		float commonProgress = 0;
		char sTitleString[256] = "CSMoE Connect Progress";
		char sDownloadString[512] = "";
		char sCommonString[512] = "";
		
	} uiConnectionProgress;
	
	void UI_ConnectionProgress_f(void)
	{
		if (!strcmp(EngFuncs::CmdArgv(1), "disconnect"))
		{
			uiConnectionProgress.HandleDisconnect();
			return;
		}

		else if (!strcmp(EngFuncs::CmdArgv(1), "dl"))
		{
			uiConnectionProgress.HandleDownload(EngFuncs::CmdArgv(2), EngFuncs::CmdArgv(3), atoi(EngFuncs::CmdArgv(4)), atoi(EngFuncs::CmdArgv(5)), EngFuncs::CmdArgv(6));
		}

		else if (!strcmp(EngFuncs::CmdArgv(1), "dlend"))
		{
			uiConnectionProgress.m_iState = STATE_CONNECTING;
			uiConnectionProgress.HandleDisconnect();
			return;
		}

		else if (!strcmp(EngFuncs::CmdArgv(1), "stufftext"))
		{
			uiConnectionProgress.HandleStufftext(atof(EngFuncs::CmdArgv(2)), EngFuncs::CmdArgv(3));
		}

		else if (!strcmp(EngFuncs::CmdArgv(1), "precache"))
		{
			uiConnectionProgress.HandlePrecache();
		}

		else if (!strcmp(EngFuncs::CmdArgv(1), "menu"))
		{
			uiConnectionProgress.m_iState = STATE_MENU;
			uiConnectionProgress.m_iSource = SOURCE_SERVERBROWSER;
			if (EngFuncs::CmdArgc() > 2)
				uiConnectionProgress.SetServer(EngFuncs::CmdArgv(2));
			uiConnectionProgress.SetCommonText("正在连接到服务器...");
			uiConnectionProgress.Show();
		}

		else if (!strcmp(EngFuncs::CmdArgv(1), "localserver"))
		{
			uiConnectionProgress.m_iState = STATE_MENU;
			uiConnectionProgress.m_iSource = SOURCE_CREATEGAME;
			uiConnectionProgress.SetServer("");
			uiConnectionProgress.SetCommonText("正在启动本地服务器...");
			uiConnectionProgress.Show();
		}

		else if (!strcmp(EngFuncs::CmdArgv(1), "changelevel"))
		{
			uiConnectionProgress.m_iState = STATE_MENU;
			uiConnectionProgress.SetCommonText("服务器正在更换地图...");
			uiConnectionProgress.Show();
		}

		else if (!strcmp(EngFuncs::CmdArgv(1), "serverinfo"))
		{
			if (EngFuncs::CmdArgc() > 2)
				uiConnectionProgress.SetServer(EngFuncs::CmdArgv(2));
			uiConnectionProgress.m_iState = STATE_CONNECTING;
			uiConnectionProgress.SetCommonText("解析服务器信息...");
			uiConnectionProgress.Show();
		}
	}
	
	void ConnectProgress_Init()
	{
		EngFuncs::Cmd_AddCommand("menu_connectionprogress", UI_ConnectionProgress_f);
	}
	
	void ConnectProgress_OnGUI()
	{
		uiConnectionProgress.OnGUI();
	}
}
