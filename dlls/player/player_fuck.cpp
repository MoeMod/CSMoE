#include "extdll.h"
#include "util.h"
#include "enginecallback.h"

#include "player_fuck.h"
#include "pinyin.h"
#include "strreplace.h"
#include "strmatch.h"

#include "tier1/strtools.h"

#include <sstream>
#include <string>

namespace sv {

    const std::vector<std::pair<std::wstring, std::wstring>> &PlayerFuck_ReplaceMap() {
        static std::vector<std::pair<std::wstring, std::wstring>> singleton;

        if(singleton.empty())
        {
            int length = 0;
            char *data = (char *)engfunc::LOAD_FILE_FOR_ME("resource/bad_words.csv", &length);
            if(data)
            {
                std::istringstream iss(data);
                std::string line;
                while(getline(iss, line))
                {
                    line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
                    if(line.empty())
                        continue;
                    wchar_t buffer[1024];
                    int buffer_len = V_UTF8ToUnicode(line.c_str(), buffer, sizeof(buffer)) / sizeof(wchar_t);

                    singleton.emplace_back(buffer, L"（哔~）"); // L"※"
                    std::push_heap(singleton.begin(), singleton.end());

                    if(buffer_len >= 3 && buffer_len <= 7)
                    {
                        for (auto &&pinyin: pinyin::GetStrPinyin(buffer)) {

                            if(pinyin.first.length() >= 3)
                            {
                                singleton.emplace_back(pinyin.first, L"（哔~）");
                                std::push_heap(singleton.begin(), singleton.end());
                            }

                            if(pinyin.second.length() >= 6)
                            {
                                singleton.emplace_back(pinyin.second, L"（哔~）");
                                std::push_heap(singleton.begin(), singleton.end());
                            }
                        }
                    }

                }
                std::sort_heap(singleton.begin(), singleton.end());
                std::unique(singleton.begin(), singleton.end());
                engfunc::FREE_FILE(data);
            }
        }

        return singleton;
    }

    // TODO : no more unicode<->utf8 convert?
    // TODO： pinyin detect
    void PlayerFuck_FilterBadWords(const char *in, char *out, int out_buffer_size) {
        wchar_t win[1024];
        V_UTF8ToUnicode(in, win, sizeof(win));
        std::wstring wout = strreplace::ReplaceKeyByMap(win, PlayerFuck_ReplaceMap());
        V_UnicodeToUTF8(wout.c_str(), out, out_buffer_size);
    }

}