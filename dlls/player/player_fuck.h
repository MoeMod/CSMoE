#pragma once

#include <string>

namespace sv {

    std::string PlayerFuck_FilterBadWords();
    void PlayerFuck_FilterBadWords(const char * in, char *out, int out_buffer_size);
}
