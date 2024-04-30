#include "program.hpp"
#include "endian.hpp"
#include <stdexcept>


Program::Program(const std::vector<uint8_t> &file) {
    if (file.size() < sizeof(ProgramHeader)) {
        throw std::runtime_error{"program file smaller than header"};
    }
    auto header = ProgramHeader{};
    header.symbol_offset = load_word(&file.data()[0]);
    header.data_offset = load_word(&file.data()[4]);
    header.code_offset = load_word(&file.data()[8]);


}