
#include <cstddef>
#include <cstdint>
#include <string>

#include "libbdig.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  int x[10];
  x[11] = size;
  return 0;
}