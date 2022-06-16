#include "imgui_connectprogress.h"
#include "imgui.h"
#include "imgui_utils.h"

#include "client.h"

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
		SetCommonText("正在缓存资源...");
		m_iState = STATE_CONNECTING;
	}
	void HandleStufftext(float flProgress, const char* pszText)
	{
		if(!strncmp(pszText, "Getting ", 8))
			pszText = "正在读取服务器信息...";
		SetCommonText(pszText);
		commonProgress = flProgress;
		m_iState = STATE_CONNECTING;
	}
	void HandleDownload(const char* pszFileName, const char* pszServerName, int iCurrent, int iTotal, const char* comment)
	{
		snprintf(sDownloadString, sizeof(sDownloadString) - 1, "正在下载资源 %s", pszFileName);
		snprintf(sCommonString, sizeof(sCommonString) - 1, "%d / %d %s", iCurrent + 1, iTotal, comment);
		m_iState = STATE_DOWNLOAD;
		commonProgress = (float)iCurrent / iTotal + 0.01f / iTotal * scr_download->value;
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
			Cbuf_AddText("http_clear\n");
			m_iState = STATE_CONNECTING;
			HandleDisconnect();
		}

		Cbuf_AddText("cmd disconnect;endgame disconnect\n");
	}

	void OnGUI()
	{
		if ((m_iState != STATE_MENU && CL_Active()))
		{
			m_iState = STATE_NONE;
			Hide();
		}

		if (!m_bShow)
			return;

		ImGui::OpenPopup(sTitleString);
		ImGuiUtils::CenterNextWindow(ImGuiCond_Appearing);
		ImGui::SetNextWindowSize(ImVec2(512, -1), ImGuiCond_Always);
		if (ImGui::BeginPopupModal(sTitleString, NULL)) {

			const float orig_radius = 128.f;
			static float angle = 0.0f;
			float radius = orig_radius;
			float time = ImGui::GetTime();
			float k_time = 2;
			static float quick_time;
			if(m_iState != STATE_NONE)
			{
				quick_time = time * k_time;
				angle += ImGui::GetIO().DeltaTime * 300.0f;
				angle = fmodf(angle, 360.0f);
			}
			if ((int)quick_time % 3)
			{
				radius = orig_radius * 0.75;
			}
			else
			{
				radius = orig_radius * 0.75 - sin(quick_time * M_PI) * orig_radius * 0.25;
			}

			ImGuiUtils::CitrusLogo("cirtus logo", ImGuiUtils::GetScaledSize(ImVec2(-1, orig_radius)), ImGuiUtils::GetScaledValue(radius), angle);

			ImGui::TextUnformatted(sDownloadString);
			ImGui::TextUnformatted(sCommonString);
			ImGui::ProgressBar(commonProgress, ImGuiUtils::GetScaledSize({ -1, 36 }));
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.00f, 0.45f, 0.00f, 1.00f));
			if(m_iState == STATE_DOWNLOAD)
			{
				ImGui::ProgressBar(scr_download->value / 100.0f, ImGuiUtils::GetScaledSize({ -1, 36 }));
			}
			else
			{
				ImGui::ProgressBar(scr_loading->value / 100.0f, ImGuiUtils::GetScaledSize({ -1, 36 }));
			}
			ImGui::PopStyleColor();


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
	if (!strcmp(Cmd_Argv(1), "disconnect"))
	{
		uiConnectionProgress.HandleDisconnect();
		return;
	}

	else if (!strcmp(Cmd_Argv(1), "dl"))
	{
		uiConnectionProgress.HandleDownload(Cmd_Argv(2), Cmd_Argv(3), atoi(Cmd_Argv(4)), atoi(Cmd_Argv(5)), Cmd_Argv(6));
	}

	else if (!strcmp(Cmd_Argv(1), "dlend"))
	{
		uiConnectionProgress.m_iState = STATE_CONNECTING;
		uiConnectionProgress.HandleDisconnect();
		return;
	}

	else if (!strcmp(Cmd_Argv(1), "stufftext"))
	{
		uiConnectionProgress.HandleStufftext(atof(Cmd_Argv(2)), Cmd_Argv(3));
	}

	else if (!strcmp(Cmd_Argv(1), "precache"))
	{
		uiConnectionProgress.HandlePrecache();
	}

	else if (!strcmp(Cmd_Argv(1), "menu"))
	{
		uiConnectionProgress.m_iState = STATE_MENU;
		uiConnectionProgress.m_iSource = SOURCE_SERVERBROWSER;
		if (Cmd_Argc() > 2)
			uiConnectionProgress.SetServer(Cmd_Argv(2));
		uiConnectionProgress.SetCommonText("正在连接到服务器...");
		uiConnectionProgress.Show();
	}

	else if (!strcmp(Cmd_Argv(1), "localserver"))
	{
		uiConnectionProgress.m_iState = STATE_MENU;
		uiConnectionProgress.m_iSource = SOURCE_CREATEGAME;
		uiConnectionProgress.SetServer("");
		uiConnectionProgress.SetCommonText("正在启动本地服务器...");
		uiConnectionProgress.Show();
	}

	else if (!strcmp(Cmd_Argv(1), "changelevel"))
	{
		uiConnectionProgress.m_iState = STATE_MENU;
		uiConnectionProgress.SetCommonText("服务器正在更换地图...");
		uiConnectionProgress.Show();
	}

	else if (!strcmp(Cmd_Argv(1), "serverinfo"))
	{
		if (Cmd_Argc() > 2)
			uiConnectionProgress.SetServer(Cmd_Argv(2));
		uiConnectionProgress.m_iState = STATE_CONNECTING;
		uiConnectionProgress.SetCommonText("解析服务器信息...");
		uiConnectionProgress.Show();
	}
}

void ImGui_ConnectProgress_Init()
{
	//Cmd_AddCommand("menu_connectionprogress", UI_ConnectionProgress_f, "open ConnectionProgress");
}

void ImGui_ConnectProgress_OnGUI()
{
	uiConnectionProgress.OnGUI();
}

