#pragma once

#include <cstdint>
#include <vector>

using std::uint8_t;

struct ProgramHeader {
    uint32_t symbol_offset;
    uint32_t data_offset;
    uint32_t code_offset;
};

/// Represents a program's binary representation on disk
struct Program {
    std::vector<uint8_t> symbols;
    std::vector<uint8_t> data;
    std::vector<uint8_t> code;

    Program(const std::vector<uint8_t> &file);
};