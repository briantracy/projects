load("@rules_cc//cc:defs.bzl", "cc_library", "cc_binary")

cc_library(
  name = "libbrp",
  hdrs = ["libbrp.hpp"],
  srcs = ["libbrp.cpp"],
  copts = ["-std=c++17", "-Wpedantic", "-Wall", "-Wextra", "-Wconversion"],
)

cc_binary(
  name = "test",
  srcs = ["test.c"],
)

cc_binary(
  name = "middleman",
  srcs = ["middleman.c"],
)
