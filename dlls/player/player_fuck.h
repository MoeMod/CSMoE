#pragma once

#include <vector>
#include <string>

namespace sv {

    const std::vector<std::pair<std::wstring, std::wstring>> &PlayerFuck_ReplaceMap();
    void PlayerFuck_FilterBadWords(const char * in, char *out, int out_buffer_size);
}
