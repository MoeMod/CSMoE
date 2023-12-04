#include <string>
#include <algorithm>
#include "STLHelper.h"

std::vector<std::string> ParseString(const std::string& sz)
{
	std::vector<std::string> Result;
	std::string::size_type l = 0;
	std::string::size_type r = 0;
	while ((r = sz.find_first_of(',', r)) != std::string::npos)
	{
		Result.emplace_back(Trim(sz.substr(l, r - l)));
		l = ++r;
	}
	Result.emplace_back(Trim(sz.substr(l, r)));
	return Result;
}

std::vector<std::wstring> ParseString(const std::wstring& sz)
{
	std::vector<std::wstring> Result;
	std::string::size_type l = 0;
	std::string::size_type r = 0;
	while ((r = sz.find_first_of(',', r)) != std::wstring::npos)
	{
		Result.emplace_back(Trim(sz.substr(l, r - l)));
		l = ++r;
	}
	Result.emplace_back(Trim(sz.substr(l, r)));
	return Result;
}