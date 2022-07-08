#pragma once

#include <string>
#include <set>
#include <unordered_map>
#include <vector>

namespace strsearch {

    class SearchInstance
    {
    public:
        struct CharIndexEntry
        {
            const wchar_t* key;
            const wchar_t* ref_str;
#define OP(R) \
        friend bool operator R (const CharIndexEntry &lhs, const CharIndexEntry & rhs) \
		{ \
			return wcscmp(lhs.key, rhs.key) R 0; \
		} \

            OP(<)
            OP(>)
            OP(<=)
            OP(>=)
            OP(==)
            OP(!=)

#undef OP
        };

        void AddStringEntry(const std::wstring& wstr);
        std::vector<CharIndexEntry> GetSearchResultArray(const std::wstring &wkeyword, int limit = 0) const;


    private:
        const wchar_t* AddStringRef(std::wstring str);
        void PreprocessString(const wchar_t* key, const wchar_t* ref_str);
        void PreprocessStringForEachChar(const wchar_t *wstr, const wchar_t *ref_str);
        const std::vector<CharIndexEntry> &GetCharIndexEntryListForChar(wchar_t wch) const;

        std::set<std::wstring> stringStorageTable;
        std::unordered_map<wchar_t, std::vector<CharIndexEntry>> startIndex;
    };
}