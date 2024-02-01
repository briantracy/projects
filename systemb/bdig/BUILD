

cc_library(
  name = "libbdig",
  hdrs = ["libbdig.h"],
  srcs = ["libbdig.c"],
)

cc_binary(
  name = "bdig",
  srcs = ["bdig.c"],
  deps = [":libbdig"],
)

cc_test(
  name = "libbdig_test",
  srcs = ["libbdig_test.cpp"],
  deps = [
    ":libbdig",
    "@googletest//:gtest_main",
  ],
  size = "small",
)

load("@rules_fuzzing//fuzzing:cc_defs.bzl", "cc_fuzz_test")
cc_fuzz_test(
    name = "libbdig_fuzz",
    srcs = ["libbdig_fuzz.cpp"],
    deps = [":libbdig"],
)
