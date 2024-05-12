
#include <array>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <vector>


void print_bytes(std::string_view label, const uint8_t *bytes, int len) {
  std::cout << "[" << label << "](" << std::dec << len << ")[";
  for (int i = 0; i < len; ++i) {
    std::cout << std::setw(2) << std::setfill('0') << std::hex << (int)bytes[i];
    if (i != len - 1) {
      std::cout << ' ';
    }
  }
  std::cout << "]\n";
}

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
    if (file.size() < sizeof(uint32_t) * 3) {
      throw std::runtime_error{"program file smaller than header"};
    }
    print_bytes("program", file.data(), static_cast<int>(file.size()));
    const uint32_t symbol_offset = load_word(&file.data()[0]);
    const uint32_t data_offset = load_word(&file.data()[4]);
    const uint32_t code_offset = load_word(&file.data()[8]);
    if (symbol_offset >= data_offset || data_offset >= code_offset) {
      throw std::runtime_error{"symbols, data, code offsets not in order"};
    }
    if (code_offset >= static_cast<uint32_t>(file.size())) {
      throw std::runtime_error{"code offset past end of file"};
    }
    symbols = std::vector<uint8_t>(&file.data()[symbol_offset], &file.data()[data_offset]);
    data = std::vector<uint8_t>(&file.data()[data_offset], &file.data()[code_offset]);
    code = std::vector<uint8_t>(&file.data()[code_offset], &file.data()[file.size()]);
    std::cout << "loaded program\n";
    print_bytes("symbols", symbols.data(), static_cast<int>(symbols.size()));
    print_bytes("data", data.data(), static_cast<int>(data.size()));
    print_bytes("code", code.data(), static_cast<int>(code.size()));

  }
};

struct Interpreter {
  enum Register {
    A = 1, B, C, D,
    W, X, Y, Z,
    Ip, Sp, Bp
  };
  std::array<uint32_t, Register::Bp> registers;
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
  Program p{contents};
  return 0;
}