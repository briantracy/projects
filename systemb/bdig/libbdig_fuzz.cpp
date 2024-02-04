
#include <cstddef>
#include <cstdint>
#include <string>

#include "libbdig.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {

  if (data[0] == 12) {
    __builtin_trap();
  }
  return 0;
}