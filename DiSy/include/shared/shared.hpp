#pragma once

#include <chrono>

namespace shared
{
uint32_t getCurrentTime()
{
    auto d = std::chrono::system_clock::now().time_since_epoch();
    uint32_t seconds = std::chrono::duration_cast<std::chrono::seconds>(d).count();
    return seconds;
}
} // namespace shared