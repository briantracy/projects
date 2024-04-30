#pragma once

#include <cstdint>

using std::uint8_t;

enum Opcode : uint8_t {
    Illegal = 0,
    LoadByte = 1,
    LoadWord = 2,
    StoreByte = 3,
    StoreWord = 4,
};