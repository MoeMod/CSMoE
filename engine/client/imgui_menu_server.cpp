/*
imgui_menu_server.cpp
Copyright (C) 2021 Moemod Hymei
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/


#include <vector>
#include <string>
#include <map>
#include <set>
#include <algorithm>
#include <regex>

#include <boost/asio.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/ssl.hpp>

#include <nlohmann/json.hpp>

#include <imgui.h>
#include "imgui_utils.h"

#include "imgui_menu_msgbox.h"


#include "common.h"
#include "crtlib.h"
#include "eiface.h"
#include "gl_local.h"

#include "TSourceEngineQuery.h"
#include "parse_ip.h"
#include "RequestHTTPS.h"

using namespace std::chrono_literals;

namespace ui {
	static bool s_server_open = false;
	static bool s_server_show = false;
	static boost::asio::io_context s_server_ioc;

	static std::vector<std::tuple<std::chrono::high_resolution_clock::duration, boost::asio::ip::udp::endpoint, TSourceEngineQuery::ServerInfoQueryResult> > s_servers_list;
	static std::map<boost::asio::ip::udp::endpoint, TSourceEngineQuery::ServerInfoQueryResult> s_serverinfos;
	static std::map<boost::asio::ip::udp::endpoint, TSourceEngineQuery::PlayerListQueryResult> s_playerlists;
	static std::map<boost::asio::ip::udp::endpoint, std::chrono::high_resolution_clock::duration> s_ping;
	static std::optional<boost::asio::ip::udp::endpoint> selected_ep;
	static std::set<boost::asio::ip::udp::endpoint> s_serverinfo_open;

	// model
	boost::asio::awaitable<void> ServerModel_AsyncRefreshSingleServer(boost::asio::ip::udp::endpoint ep)
	{
		try {
			auto start_req = std::chrono::high_resolution_clock::now();
			auto info = co_await TSourceEngineQuery::Co_GetServerInfoDataAsync(s_server_ioc, ep, 1s);
			auto ping = (std::chrono::high_resolution_clock::now() - start_req) / 2;
			s_servers_list.erase(std::remove_if(s_servers_list.begin(), s_servers_list.end(),
			                                    [ep](auto &entry) { return get<1>(entry) == ep; }),
			                     s_servers_list.end());
			s_servers_list.emplace_back(ping, ep, info);
			s_serverinfos[ep] = info;
			s_ping[ep] = ping;
		} catch(const std::exception &e)
		{
			// ignored
		}
	}

	boost::asio::awaitable<void> ServerModel_AsyncRefreshSingleServer(std::string host, std::string port)
	{
		try {
			boost::asio::ip::udp::resolver resolver(s_server_ioc);
			auto desc_endpoints = co_await resolver.async_resolve(host.c_str(), port.c_str(), boost::asio::use_awaitable);
			for (const auto &ep: desc_endpoints) {
				co_await ServerModel_AsyncRefreshSingleServer(ep);
			}
		} catch(const std::exception &e)
		{
			// ignored
		}
	}

	boost::asio::awaitable<void> ServerModel_AsyncRefresh()
	{
		// https://api.moemod.com/csmoe/masterserver
		try {
			std::string result = co_await HTTPS_RequestAsync(s_server_ioc, "GET", "apiv2.moemod.com", "443", "/csmoe/masterserver", boost::asio::use_awaitable);
            auto j = nlohmann::json::parse(result);
            auto j2 = j["servers"];

            for(std::string srv : j2)
            {
				if (auto [host, port] = ParseHostPort(srv); !host.empty()) {
					boost::asio::co_spawn(s_server_ioc, ServerModel_AsyncRefreshSingleServer(host, port), boost::asio::detached);
				}
            }
		} catch(const std::exception &e)
		{
			MsgBox_Open(std::string("肥肠抱歉，获取服务器列表时发生了错误：\n") + e.what());
		}
	}

	void ServerModel_Refresh()
	{
		s_server_ioc.reset();
		boost::asio::co_spawn(s_server_ioc, ServerModel_AsyncRefresh(), boost::asio::detached);
		s_server_ioc.restart();
	}

	void ServerModel_Connect(boost::asio::ip::udp::endpoint ep)
	{
		char cmd[256];
        if(ep.protocol() == boost::asio::ip::udp::v6())
		    snprintf(cmd, 256, "connect [%s]:%d;menu_connectionprogress menu server", ep.address().to_string().c_str(), (int)ep.port());
		else
            snprintf(cmd, 256, "connect %s:%d;menu_connectionprogress menu server", ep.address().to_string().c_str(), (int)ep.port());
		Cbuf_AddText(cmd);
	}

	boost::asio::awaitable<void> ServerDetailInfoModel_AsyncRefresh(boost::asio::ip::udp::endpoint ep)
	{
		try {
			auto info = co_await TSourceEngineQuery::Co_GetPlayerListDataAsync(s_server_ioc, ep,1s);
			s_playerlists[ep] = info;
		} catch(const std::exception &e)
		{
			// ignored
		}
	}

	void ServerDetailInfoModel_Refresh(boost::asio::ip::udp::endpoint ep)
	{
		s_server_ioc.reset();
		boost::asio::co_spawn(s_server_ioc, ServerModel_AsyncRefreshSingleServer(ep), boost::asio::detached);
		boost::asio::co_spawn(s_server_ioc, ServerDetailInfoModel_AsyncRefresh(ep), boost::asio::detached);
		s_server_ioc.restart();
	}

	// view
	void ServerDetailInfo_OnGui(boost::asio::ip::udp::endpoint ep)
	{
		if(s_playerlists.find(ep) == s_playerlists.end())
		{
			ServerDetailInfoModel_Refresh(ep);
			s_playerlists.emplace(ep, TSourceEngineQuery::PlayerListQueryResult{});
		}
		ImGuiUtils::CenterNextWindow(ImGuiCond_Appearing);
		bool open = true;
		if (ImGui::Begin(("服务器信息 - " + ep.address().to_string()).c_str(), &open))
		{
			auto info = s_serverinfos[ep];
			ImGui::Columns(2, nullptr, false);
			ImGui::SetColumnWidth(0, ImGuiUtils::GetScaledValue(70));


			ImGui::TextUnformatted("名称：");
			ImGui::NextColumn();
			ImGui::TextUnformatted(info.ServerName.c_str());

			ImGui::NextColumn();
			ImGui::TextUnformatted("IP地址：");
			ImGui::NextColumn();
			ImGui::TextUnformatted(boost::asio::ip::detail::endpoint(ep.address(), ep.port()).to_string().c_str());

			ImGui::NextColumn();
			ImGui::TextUnformatted("游戏：");
			ImGui::NextColumn();
			ImGui::TextUnformatted(info.Game.c_str());

			ImGui::NextColumn();
			ImGui::TextUnformatted("地图：");
			ImGui::NextColumn();
			ImGui::TextUnformatted(info.Map.c_str());

			ImGui::NextColumn();
			ImGui::TextUnformatted("玩家：");
			ImGui::NextColumn();
			ImGui::Text("%d/%d", info.PlayerCount, info.MaxPlayers);

			ImGui::NextColumn();
			ImGui::TextUnformatted("VAC：");
			ImGui::NextColumn();
			ImGui::TextUnformatted(info.VAC ? "有" : "无");

			ImGui::NextColumn();
			ImGui::TextUnformatted("延迟");
			ImGui::NextColumn();
			ImGui::Text("%d", static_cast<int>(s_ping[ep] / 1ms));

			ImGui::Columns();

			if(ImGui::BeginChild("PlayerList", ImVec2(0, -ImGuiUtils::GetScaledValue(40)), true))
			{
				ImGui::Columns(3, "serverColumns", false);
				auto size = ImGui::GetWindowSize();
				ImGui::SetColumnWidth(0, size.x - ImGuiUtils::GetScaledValue(120));
				ImGui::SetColumnWidth(1, ImGuiUtils::GetScaledValue(30));
				ImGui::SetColumnWidth(2, ImGuiUtils::GetScaledValue(80));

				auto playerlist = s_playerlists[ep];
				if(playerlist.Results.index() == 1)
				{
					for(const auto &info : get<std::vector<TSourceEngineQuery::PlayerListQueryResult::PlayerInfo_s>>(playerlist.Results))
					{
						if(ImGui::Selectable(info.Name.c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick))
						{
							// TODO
						}
						ImGui::NextColumn();
						ImGui::Text("%d", info.Score);
						ImGui::NextColumn();
						float dur = info.Duration;
						int hour = dur / 3600;
						int min = dur / 60 - 60 * hour;
						int sec = dur - 60 * min - 3600 * hour;
						if(hour)
							ImGui::Text("%2d:%2d:%2d", hour, min, sec);
						else if(min)
							ImGui::Text("%2d:%2d", min, sec);
						else if(sec)
							ImGui::Text("%2ds", sec);

						ImGui::NextColumn();
						ImGui::Separator();
					}
				}

				ImGui::Columns();

				ImGui::EndChild();
			}

			ImGui::Separator();
			ImGui::Columns(3, nullptr, false);

			if(ImGui::Button("进入"))
			{
				ServerModel_Connect(selected_ep.value());
			}

			ImGui::NextColumn();

			if(ImGui::Button("刷新"))
			{
				ServerDetailInfoModel_Refresh(ep);
			}

			ImGui::NextColumn();

			if(ImGui::Button("关闭"))
			{
				open = false;
			}
			ImGui::NextColumn();

			ImGui::Columns();

			ImGui::End();
		}

		if(!open)
		{
			s_serverinfo_open.erase(ep);
		}

	}

	void Server_Open()
	{
		s_server_open = true;
		s_server_show = true;
		ServerModel_Refresh();
	}

	void Server_Show()
	{
		s_server_show = true;
	}

	void Server_Hide()
	{
		s_server_show = false;
	}

	void ImGui_Server_Init()
	{
		Cmd_AddCommand("citrus_servers", Server_Open, "open server");
		Cmd_AddCommand("citrus_servers_show", Server_Show, "show server");
		Cmd_AddCommand("citrus_servers_hide", Server_Hide, "hide server");
	}

	void ImGui_Server_OnGui()
	{
		std::vector<boost::asio::ip::udp::endpoint> serverinfo_open2(s_serverinfo_open.begin(), s_serverinfo_open.end());
		// 不能在遍历出来再遍历
		for(auto ep : serverinfo_open2)
			ServerDetailInfo_OnGui(ep);

		if(!s_server_open || !s_server_show)
			return;
		ImGui::SetNextWindowSizeConstraints(ImGuiUtils::GetScaledSize({480, 320}), ImGui::GetIO().DisplaySize);
		ImGuiUtils::CenterNextWindow(ImGuiCond_Appearing);
		if (ImGui::Begin("柑橘 CSMoE 服务器列表", &s_server_open))
		{
			s_server_ioc.poll();

			ImGui::SameLine();
			{
				const float orig_radius = 64.f;
				static float angle = 0.0f;
				float radius = orig_radius;
				float time = ImGui::GetTime();
				float k_time = 2;
				static float quick_time;
				if(!s_server_ioc.stopped())
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

				ImGuiUtils::CitrusLogo("cirtus logo", ImGuiUtils::GetScaledSize(ImVec2(orig_radius, 48)), ImGuiUtils::GetScaledValue(radius), angle);
			}

			ImGui::SameLine();
			if(s_server_ioc.stopped())
			{
				if(ImGui::Button("全部刷新"))
				{
					ServerModel_Refresh();
				}
			}
			else
			{
				ImGui::Text("加载中，请稍等。");
			}
			ImGui::SameLine();
			if(selected_ep.has_value())
			{
				if(ImGui::Button("服务器信息"))
				{
					s_serverinfo_open.emplace(selected_ep.value());
				}
			}
			ImGui::SameLine();
			if(selected_ep.has_value())
			{
				if(ImGui::Button("连接"))
				{
					ServerModel_Connect(selected_ep.value());
				}
			}
			ImGui::NewLine();

			std::sort(s_servers_list.begin(), s_servers_list.end(), [](const auto &lhs, const auto &rhs){
				return get<0>(lhs) < get<0>(rhs);
			});

			ImGui::Columns(4, "serverColumns", false);
			auto size = ImGui::GetWindowSize();
			ImGui::SetColumnWidth(0, size.x - ImGuiUtils::GetScaledValue(220));
			ImGui::SetColumnWidth(1, ImGuiUtils::GetScaledValue(100));
			ImGui::SetColumnWidth(2, ImGuiUtils::GetScaledValue(50));
			ImGui::SetColumnWidth(3, ImGuiUtils::GetScaledValue(30));
			for(auto &[ping, ep, info] : s_servers_list)
			{
				if(ImGui::Selectable(info.ServerName.c_str(), selected_ep == ep, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick))
				{
					selected_ep = ep;
					if(ImGui::IsMouseDoubleClicked(ImGuiPopupFlags_MouseButtonLeft))
					{
						ServerModel_Connect(ep);
					}
				}
				ImGui::NextColumn();
				ImGui::TextUnformatted(info.Map.c_str());
				ImGui::NextColumn();
				ImGui::Text("%d/%d", info.PlayerCount, info.MaxPlayers);
				ImGui::NextColumn();
				ImGui::Text("%d", static_cast<int>(ping / 1ms));
				ImGui::NextColumn();
				ImGui::Separator();
			}
			ImGui::Columns();
			ImGui::End();
		}
		if(!s_server_open || !s_server_show)
		{
			// window is closed
			s_server_ioc.stop();
			s_server_ioc.reset();
		}
	}

}