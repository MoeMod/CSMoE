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

#include "enginecallback_menu.h"
#include "TSourceEngineQuery.h"
#include "parse_ip.h"
#include "RequestHTTPS.h"

using namespace std::chrono_literals;

namespace ui {
	static bool s_server_open = false;
	static boost::asio::io_context s_server_ioc;

	static std::vector<std::tuple<std::chrono::high_resolution_clock::duration, boost::asio::ip::udp::endpoint, TSourceEngineQuery::ServerInfoQueryResult> > s_servers_list;

	// model
	boost::asio::awaitable<void> ProcessParseMasterServer(std::string_view data)
	{
		auto j = nlohmann::json::parse(data);
		auto j2 = j["servers"];

		std::vector<boost::asio::awaitable<void> > awaitables;
		std::transform(j2.begin(), j2.end(), std::back_inserter(awaitables), [](const std::string& srv) -> boost::asio::awaitable<void> {
			return boost::asio::co_spawn(s_server_ioc, [srv]() mutable -> boost::asio::awaitable<void> {
					if (auto [host, port] = ParseHostPort(srv); !host.empty()) {
						boost::asio::ip::udp::resolver resolver(s_server_ioc);
						auto desc_endpoints = co_await resolver.async_resolve(boost::asio::ip::udp::v4(), host.c_str(), port.c_str(), boost::asio::use_awaitable);
						for (const auto &ep: desc_endpoints) {
							auto start_req = std::chrono::high_resolution_clock::now();
							auto info = co_await TSourceEngineQuery::Co_GetServerInfoDataAsync(s_server_ioc, ep,1s);
							auto ping = (std::chrono::high_resolution_clock::now() - start_req) / 2;
							s_servers_list.erase(std::remove_if(s_servers_list.begin(), s_servers_list.end(), [ep](auto &entry){ return get<1>(entry) == ep; }), s_servers_list.end());
							s_servers_list.emplace_back(ping, ep, info);
						}
					}
			}, boost::asio::use_awaitable);
		});

		for(auto &&awaitable : awaitables)
		{
			try
			{
				co_await std::move(awaitable);
			}
			catch(const std::exception &e)
			{

			}
		}
		co_return;
	}

	boost::asio::awaitable<void> ServerModel_AsyncRefresh()
	{
		// https://api.moemod.com/csmoe/masterserver
		try {
			std::string result = co_await HTTPS_RequestAsync(s_server_ioc, "GET", "api.moemod.com", "443", "/csmoe/masterserver", boost::asio::use_awaitable);
			co_await ProcessParseMasterServer(result);
		} catch(const std::exception &e)
		{
			MsgBox_Open(std::string("肥肠抱歉，获取服务器列表时发生了错误：\n") + e.what());
		}
	}

	void ServerModel_Refresh()
	{
		s_server_ioc.restart();
		boost::asio::co_spawn(s_server_ioc, ServerModel_AsyncRefresh, boost::asio::detached);
	}

	// view

	void Server_Open()
	{
		s_server_open = true;
	}

	void Server_OnGui()
	{
		if(!s_server_open)
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
			ImGui::NewLine();

			std::sort(s_servers_list.begin(), s_servers_list.end(), [](const auto &lhs, const auto &rhs){
				return get<0>(lhs) < get<0>(rhs);
			});

			ImGui::Columns(4, "serverColumns", false);
			auto size = ImGui::GetWindowSize();
			ImGui::SetColumnWidth(0, size.x - ImGuiUtils::GetScaledValue(400));
			ImGui::SetColumnWidth(1, ImGuiUtils::GetScaledValue(180));
			ImGui::SetColumnWidth(2, ImGuiUtils::GetScaledValue(80));
			ImGui::SetColumnWidth(3, ImGuiUtils::GetScaledValue(80));
			static boost::asio::ip::udp::endpoint selected_ep;
			for(auto &[ping, ep, info] : s_servers_list)
			{
				if(ImGui::Selectable(info.ServerName.c_str(), selected_ep == ep, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick))
				{
					selected_ep = ep;
					if(ImGui::IsMouseDoubleClicked(ImGuiPopupFlags_MouseButtonLeft))
					{
						char cmd[128];
						snprintf(cmd, 128, "connect %s:%d", ep.address().to_string().c_str(), (int)ep.port());
						EngFuncs::ClientCmd(0, cmd);
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
		if(!s_server_open)
		{
			// window is closed
			s_server_ioc.stop();
			s_server_ioc.reset();
		}
	}

}