/*
imgui_menu_update.cpp
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
#include "RequestHTTPS.h"

using namespace std::chrono_literals;

extern const char *Q_buildos();
extern const char *Q_buildarch();
extern int Q_buildnum();

namespace ui {
	static boost::asio::io_context s_update_ioc;

	struct VersionInfo_s {
		int ver;
		std::string sys;
		std::vector<std::string> arch;
		std::string url;
		std::string desc;
	};

	void from_json(const nlohmann::json &j, VersionInfo_s &s)
	{
		j.at("ver").get_to(s.ver);
		j.at("sys").get_to(s.sys);
		j.at("arch").get_to(s.arch);
		j.at("url").get_to(s.url);
		j.at("desc").get_to(s.desc);
	}

	bool IsMatch(const VersionInfo_s &s)
	{
		return (s.sys == Q_buildos()) && (std::find(s.arch.begin(), s.arch.end(), Q_buildarch()) != s.arch.end());
	}

	boost::asio::awaitable<void> UpdateModel_AsyncRefresh()
	{
		// https://api.moemod.com/csmoe/version
		try {
			std::string result = co_await HTTPS_RequestAsync(s_update_ioc, "GET", "api.moemod.com", "443", "/csmoe/version", boost::asio::use_awaitable);
			auto j = nlohmann::json::parse(result);

			std::vector<VersionInfo_s> all_infos;
			j.get_to(all_infos);

			std::vector<VersionInfo_s> candidate;
			std::copy_if(all_infos.begin(), all_infos.end(), std::back_inserter(candidate), IsMatch);

			auto iter = std::max_element(candidate.begin(), candidate.end(), [](const VersionInfo_s &lhs, const VersionInfo_s &rhs){ return lhs.ver < rhs.ver; });
			if(iter != candidate.end())
			{
				const VersionInfo_s &info = *iter;
				if(info.ver > Q_buildnum())
				{
					while(ImGui::GetTopMostPopupModal())
					{
						co_await boost::asio::post(boost::asio::use_awaitable);
					}
					MsgBox_Open(std::string("发现新版本") + std::to_string(info.ver) + std::string("，更新内容：\n") + info.desc + std::string("\n是否立即更新？"), [info](){
						EngFuncs::ShellExecute( info.url.c_str(), NULL, true );
					});
				}
			}
		} catch(const std::exception &e)
		{
			MsgBox_Open(std::string("肥肠抱歉，获取更新时发生了错误：\n") + e.what());
		}
	}

	void UpdateModel_Refresh()
	{
		s_update_ioc.reset();
		boost::asio::co_spawn(s_update_ioc, UpdateModel_AsyncRefresh(), boost::asio::detached);
		s_update_ioc.restart();
	}

	// view

	void Update_Initiate()
	{
		UpdateModel_Refresh();
	}

	void Update_OnGui()
	{
		s_update_ioc.poll_one();
	}

}