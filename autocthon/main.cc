
#include <cstdint>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

struct ProgramHeader {
  uint32_t symbol_offset;
  uint32_t data_offset;
  uint32_t code_offset;
};

constexpr uint32_t load_word(const uint8_t *bytes) {
  return (static_cast<uint32_t>(bytes[0]) << 24) |
         (static_cast<uint32_t>(bytes[1]) << 16) |
         (static_cast<uint32_t>(bytes[2]) << 8) |
         (static_cast<uint32_t>(bytes[3]) << 0);
}

/// Represents a program's binary representation on disk
struct Program {
  std::vector<uint8_t> symbols;
  std::vector<uint8_t> data;
  std::vector<uint8_t> code;

  Program(const std::vector<uint8_t> &file) {
    if (file.size() < sizeof(ProgramHeader)) {
      throw std::runtime_error{"program file smaller than header"};
    }
    auto header = ProgramHeader{};
    header.symbol_offset = load_word(&file.data()[0]);
    header.data_offset = load_word(&file.data()[4]);
    header.code_offset = load_word(&file.data()[8]);
  }
};

int main(const int argc, const char *const argv[]) {
  (void)argc;
  (void)argv;
  std::ifstream stream{"program.atn", std::ios::in | std::ios::binary};
  if (!stream) {
    throw std::runtime_error{"cannot open input file"};
  }
  std::vector<uint8_t> contents((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
  std::cout << "mega" << contents.size() << '\n';
  contents[12] = 12;
  Program p{contents};
  return 0;
}