#pragma once

#include <utility>
#include <string>
#include <regex>

inline std::pair<std::string, std::string> ParseHostPort(const std::string& msg) {
	static std::regex v4(R"((^[0-9a-zA-Z]+[0-9a-zA-Z\.-]*\.[a-zA-Z]{2,4}):*(\d+)*)");
    static std::regex v6(R"((([0-9a-fA-F]{0,4}:){1,7}[0-9a-fA-F]{0,4}))");
    static std::regex v6p(R"(\[(([0-9a-fA-F]{0,4}:){1,7}[0-9a-fA-F]{0,4})\]:*(\d+)*)");
    static std::regex r3(R"((?:(?:25[0-5]|2[0-4]\d|((1\d{2})|([1-9]?\d)))\.){3}(?:25[0-5]|2[0-4]\d|((1\d{2})|([1-9]?\d)))(:\d+)*)");

	std::pair<std::string, std::string> ret{ {}, "27015" };
	auto& [host, port] = ret;

    if (std::smatch sm; std::regex_match(msg, sm, v6p) && sm.size() > 2) {
        // host:port, host, port
        host = sm[1].str();
        port = sm[3].str();
    }
    else if (std::smatch sm; std::regex_match(msg, sm, v6) && sm.size() > 1) {
        // host:port, host, port
        host = sm[1].str();
    }
    else if (std::smatch sm; std::regex_match(msg, sm, v4) && sm.size() > 2) {
		// host:port, host, port
		host = sm[1].str();
		if (sm[2].str().size()) port = sm[2].str();
	}
	else if (std::smatch sm; std::regex_match(msg, sm, r3) && sm.size() > 2) {
		// ip:port, ...
		const auto adr = sm[0].str();
		auto iter = std::find(adr.begin(), adr.end(), ':');
		host.assign(adr.begin(), iter);
		if (iter != adr.end()) port.assign(iter + 1, adr.end());
	}
	return ret;
}