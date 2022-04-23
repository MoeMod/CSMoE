#include "strsearch.h"

#include "pinyin.h"
#include "strmatch.h"

#include <algorithm>
#include <iterator>

namespace strsearch {

    void SearchInstance::PreprocessString(const wchar_t* key, const wchar_t* ref_str)
    {
        auto wch = *key;
        std::vector<CharIndexEntry> &vec_ref = startIndex[wch];
        CharIndexEntry entry{ key, ref_str };
        if(!std::binary_search(vec_ref.begin(), vec_ref.end(), entry))
        {
            vec_ref.push_back(entry);
            std::inplace_merge(vec_ref.begin(), vec_ref.end() - 1, vec_ref.end());
        }
    }

    void SearchInstance::PreprocessStringForEachChar(const wchar_t *wstr, const wchar_t *ref_str)
    {
        // 祈祷strlen比自己手写for更快
        std::size_t len = wcslen(wstr);
        for (std::size_t i = 0; i < len; ++i)
        {
            PreprocessString(wstr + i, ref_str);
        }
    }

    auto SearchInstance::GetCharIndexEntryListForChar(wchar_t wch) const -> const std::vector<CharIndexEntry> &
    {
        static const std::vector<CharIndexEntry> empty_res;
        auto iter = startIndex.find(wch);
        if (iter == startIndex.end())
            return empty_res;
        return iter->second;
    }

    const wchar_t* SearchInstance::AddStringRef(std::wstring str)
    {
        auto iter = stringStorageTable.emplace(std::move(str)).first;
        return iter->c_str();
    }

    void SearchInstance::AddStringEntry(const std::wstring &wstr) {
        const wchar_t *str_self = AddStringRef(wstr);
        for (auto &&pinyin: pinyin::GetStrPinyin(wstr)) {
            const wchar_t *str_pinyin_init = AddStringRef(std::move(pinyin.first));
            auto str_pinyin_init_len = wcslen(str_pinyin_init);
            PreprocessStringForEachChar(str_pinyin_init, str_self);

            const wchar_t *str_pinyin_full = AddStringRef(std::move(pinyin.second));
            auto str_pinyin_full_len = wcslen(str_pinyin_full);
            for (std::size_t i = 0; i < str_pinyin_full_len; ++i) {
                if (std::find(str_pinyin_init, str_pinyin_init + str_pinyin_init_len, str_pinyin_full[i]) !=
                    str_pinyin_init + str_pinyin_init_len) {
                    PreprocessString(str_pinyin_full + i, str_self);
                }
            }
        }
        PreprocessStringForEachChar(str_self, str_self);
    }

    auto SearchInstance::GetSearchResultArray(const std::wstring &wkeyword, const int limit) const -> std::vector<CharIndexEntry>
    {
        std::vector<CharIndexEntry> res;
        if (wkeyword.empty())
            return res;

        const wchar_t first_char = wkeyword.front();
        const auto& startRange = GetCharIndexEntryListForChar(first_char);

        if (wkeyword.size() == 1)
        {
            // 只有一个字符的时候随便返回点什么就可以了
            std::copy(startRange.begin(), startRange.end(), std::back_inserter(res));
            if (limit > 0 && res.size() > limit)
                res.resize(limit);
        }
        else
        {
            // 两个字符以上时候需要强排序
            std::vector<CharIndexEntry> candidates;
            std::copy_if(startRange.begin(), startRange.end(), std::back_inserter(candidates), [&wkeyword](const CharIndexEntry& entry) {
                return std::equal(wkeyword.begin(), wkeyword.end(), entry.key);
            });

            auto compare_func = [&wkeyword](const CharIndexEntry& a, const CharIndexEntry& b) {
                return strmatch::CalcMinEditDistance(wkeyword.c_str(), a.key) < strmatch::CalcMinEditDistance(wkeyword.c_str(), b.key);
            };
            // 按最小编辑距离从小到大排序
            if (limit > 0 && candidates.size() > limit)
                std::partial_sort(candidates.begin(), candidates.begin() + limit, candidates.end(), compare_func);
            else
                std::sort(candidates.begin(), candidates.end(), compare_func);

            // 获取到源字符串并且返回
            std::copy(candidates.begin(), candidates.end(), std::back_inserter(res));
            if (limit > 0 && res.size() > limit)
                res.resize(limit);
        }
        return res;
    }
}