#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <iterator>

namespace pinyin {

    struct HanziTableEntry {
        wchar_t first_py;  // 首字母
        std::wstring full_py;   // 全拼
        std::wstring hanzis;
    };

    inline bool IsHanziChar(wchar_t wch) {
        return wch >= 0x80;
    }

    const std::vector <HanziTableEntry> &HanziTablesSingleton();

    template<class OutputIter>
    auto GetAllPingyinForChar(wchar_t wch, OutputIter out) -> typename std::enable_if<std::is_assignable<decltype(*out), std::pair<wchar_t, std::wstring>>::value>::type
    {
        if (IsHanziChar(wch))
        {
            for (auto&& item : HanziTablesSingleton())
            {
                // 修改原算法为二分查找，但是要求汉字表已排序
                if (std::binary_search(item.hanzis.begin(), item.hanzis.end(), wch))
                {
                    *out = { item.first_py, item.full_py };
                }
            }
        }
        else
        {
            *out = { wch, { wch } };
        }
    }

    template<class OutputIter>
    auto GetAllPingyinForChar(wchar_t wch, OutputIter out) -> typename std::enable_if<std::is_assignable<decltype(*out), wchar_t>::value>::type
    {
        if (IsHanziChar(wch))
        {
            for (auto&& item : HanziTablesSingleton())
            {
                // 修改原算法为二分查找，但是要求汉字表已排序
                if (std::binary_search(item.hanzis.begin(), item.hanzis.end(), wch))
                {
                    *out = item.first_py;
                }
            }
        }
        else
        {
            *out = wch;
        }
    }

    inline std::vector<std::pair<std::wstring, std::wstring>> GetStrPinyin(const std::wstring& inStr)
    {
        // --- 条件返回
        std::vector<std::pair<std::wstring, std::wstring>> res;
        if (inStr.empty())
        {
            return res;
        }

        // --- 获取每个字的所有读音
        size_t numOfWord = 0;
        std::vector<std::vector<std::pair<wchar_t, std::wstring>>> pys;
        for (wchar_t wch : inStr)
        {
            std::vector<std::pair<wchar_t, std::wstring>> py;
            GetAllPingyinForChar(wch, std::back_inserter(py));
            if(!py.empty())
            {
                pys.emplace_back(std::move(py));
                ++numOfWord;
            }
        }

        // --- 开始排列组合
        std::vector<int> nowIndexList;
        std::vector<int> maxIndexList;
        int sum_maxIndexList = 0;
        for (size_t i = 0; i < numOfWord; i++)
        {
            nowIndexList.push_back(0);
            maxIndexList.push_back(int(pys[i].size()) - 1);
            sum_maxIndexList += maxIndexList[i];
        }

        // --- 第一次组合（所有采用第一个）
        bool mustCombination = sum_maxIndexList > 0 ? true : false;
        std::pair<std::wstring, std::wstring> firstResult;
        for (size_t i = 0; i < numOfWord; i++)
        {
            firstResult.first += pys[i][0].first;
            firstResult.second += pys[i][0].second;
        }
        res.emplace_back(firstResult);


        // --- 循环遍历
        while (mustCombination)
        {
            // --- 组合排列
            bool alreadyRunOnce = false;
            for (size_t i = 0; i < numOfWord; i++)
            {
                if (alreadyRunOnce)
                {
                    break;
                }

                if (maxIndexList[i] != 0)
                {
                    if (nowIndexList[i] < maxIndexList[i])
                    {
                        alreadyRunOnce = true;
                        nowIndexList[i]++;
                    }
                    else if (nowIndexList[i] == maxIndexList[i])
                    {
                        nowIndexList[i] = 0;
                    }
                }
            }

            // --- 组合输出字符
            std::pair<std::wstring, std::wstring> restResult;
            for (size_t i = 0; i < numOfWord; i++)
            {
                restResult.first += pys[i][nowIndexList[i]].first;
                restResult.second += pys[i][nowIndexList[i]].second;
            }
            res.emplace_back(restResult);

            // --- 退出条件
            bool canOut = true;
            for (size_t i = 0; i < numOfWord; i++)
            {
                if (nowIndexList[i] != maxIndexList[i])
                {
                    canOut = false;
                    break;
                }
            }
            if (canOut)
            {
                break;
            }
        }


        // --- 返回
        return res;
    }

}