#pragma once

#include <cstdint>

using std::uint8_t;
using std::uint32_t;

constexpr uint32_t load_word(const uint8_t *bytes) {
    return (bytes[0] << 24) |
        (bytes[1] << 16) |
        (bytes[2] << 8) |
        (bytes[3] << 0);
}