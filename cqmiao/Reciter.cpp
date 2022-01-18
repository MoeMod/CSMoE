//
// Created by 梅峰源 on 2020-03-10.
//

#include "Reciter.h"

#include <nlohmann/json.hpp>
#include <random>
#include <fstream>
#include <sstream>

Reciter GetRandomReciter() noexcept(false)
{
    using nlohmann::json;
    std::ifstream i("Reciter/universal/universal.json");
    json j;
    i >> j;

    static std::random_device rd;
    const json &res = j["repositories"];
    // 去掉非高考篇目
    std::size_t random = 999;
    while(random >= 134)
        random = std::uniform_int_distribution<std::size_t>(0, res.size() - 1)(rd);

    const json &poem = res[random];
    auto normal_size = 0, comp_size = 0;
    auto normal_iter = poem.find("normal");
    auto comp_iter = poem.find("comprehensions");
    if(normal_iter != poem.end())
        normal_size = normal_iter->size();
    if(comp_iter != poem.end())
        comp_size = comp_iter->size();

    auto idx = std::uniform_int_distribution<std::size_t>(0, normal_size + comp_size - 1)(rd);
    if(idx < normal_size)
    {
        // pick normal
        auto &item = (*normal_iter)[idx];
        std::vector<std::string> vec(item.begin(), item.end());

        // 随机选一个空
        auto idx_ans = std::uniform_int_distribution<std::size_t>(0, vec.size() - 1)(rd);
        std::string ans(std::exchange(vec[idx_ans], "______________"));

        std::ostringstream oss1;
        std::copy(vec.begin(), vec.end(), std::ostream_iterator<std::string>(oss1, " "));
        oss1 << "（";
        if(auto iter = poem.find("author"); iter != poem.end())
            oss1 << iter->get<std::string>();
        if(auto iter = poem.find("title"); iter != poem.end())
            oss1 << "《" << iter->get<std::string>() << "》";
        oss1 << "）";
        Reciter ret = {oss1.str(), ans};
        return ret;
    }
    else
    {
        idx -= normal_size;
        auto &item = (*comp_iter)[idx];
        std::string cont = item["content"].get<std::string>();
        auto &ans = item["answer"];

        auto iter = cont.end();
        while((iter = std::find(cont.begin(), cont.end(), '$')) != cont.end())
        {
            *iter++ = '_';
            *iter++ = '_';
        }

        std::ostringstream oss1;
        if(auto iter = poem.find("author"); iter != poem.end())
            oss1 << iter->get<std::string>();
        if(auto iter = poem.find("title"); iter != poem.end())
            oss1 << "《" << iter->get<std::string>() << "》中";
        oss1 << cont;
        oss1 << "（多句请用空格分隔）";

        std::ostringstream oss2;
        std::copy(ans.begin(), ans.end(), std::ostream_iterator<std::string>(oss2, " "));
        Reciter ret = {oss1.str(), oss2.str()};
        ret.answer.pop_back();
        return ret;
    }
}