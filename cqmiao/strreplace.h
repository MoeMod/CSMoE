#pragma once

#include <vector>
#include <string>
#include <algorithm>


namespace strreplace
{
    inline std::string ReplaceKeyByMap(const char *in, const std::vector<std::pair<std::string, std::string>> &map)
    {
        std::string result;
        for (auto p = in; *p;)
        {
            // 注意FString默认使用的stricmp，而std::string默认使用strcmp
            auto comp1 = [](const std::pair<std::string, std::string>& kv, const char* text) {
                return strncmp(kv.first.c_str(), text, kv.first.size()) < 0;
            };
            auto comp2 = [](const char* text, const std::pair<std::string, std::string>& kv) {
                return strncmp(text, kv.first.c_str(), kv.first.size()) < 0;
            };
            auto iter = std::lower_bound(map.begin(), map.end(), p, comp1);
            if (iter != map.end() && !comp2(p, *iter))
            {
                result += iter->second;
                p += iter->first.length();
            }
            else
            {
                result.push_back(*p);
                ++p;
            }
        }
        return result;
    }
    inline std::wstring ReplaceKeyByMap(const wchar_t *in, const std::vector<std::pair<std::wstring, std::wstring>> &map)
    {
        std::wstring result;
        for (auto p = in; *p;)
        {
            // 注意FString默认使用的stricmp，而std::string默认使用strcmp
            auto comp1 = [](const std::pair<std::wstring, std::wstring>& kv, const wchar_t* text) {
                return wcsncmp(kv.first.c_str(), text, kv.first.size()) < 0;
            };
            auto comp2 = [](const wchar_t* text, const std::pair<std::wstring, std::wstring>& kv) {
                return wcsncmp(text, kv.first.c_str(), kv.first.size()) < 0;
            };
            auto iter = std::lower_bound(map.begin(), map.end(), p, comp1);
            if (iter != map.end() && !comp2(p, *iter))
            {
                result += iter->second;
                p += iter->first.length();
            }
            else
            {
                result.push_back(*p);
                ++p;
            }
        }
        return result;
    }
}