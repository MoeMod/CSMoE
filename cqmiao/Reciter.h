#pragma once

#include <string>

// question / answer
struct Reciter{
    std::string question;
    std::string answer;
};
Reciter GetRandomReciter() noexcept(false);