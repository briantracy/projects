load("@rules_cc//cc:defs.bzl", "cc_library", "cc_binary")

cc_library(
  name = "libbrp",
  hdrs = ["libbrp.hpp"],
  srcs = ["libbrp.cpp"],
)

cc_binary(
  name = "client",
  srcs = ["client.cpp"],
  deps = [":libbrp"],
)

cc_binary(
  name = "middleman",
  srcs = ["middleman.c", "libbrp.hpp"],
)
