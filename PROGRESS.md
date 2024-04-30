# Progress

* Jan 21, 2024
  * Use `dig` to get wireshark to display a real UDP DNS packet, then Export Packet Bytes into a file, and `cat file > /dev/udp/1.1.1.1/53` to see a fake `dig` session.
* Jan 22, 2024
  * Bazel target for libbdig, create `bdig_resolve_v4` function proto
* Jan 23, 2024
  * Set up libbdig and bdig bazel dependencies
* Jan 24, 2024
  * Send fake DNS packet to 1.1.1.1, start bdig_dns_encode
* Jan 28,29
  * Bazel test library
  * bdig_dns_encode
* Jan 31
  * symlinked python -> python3 and got bazel fuzz targets in place
* Feb 3
  * Install LLVM and use it to get bazel fuzz testing to work
* Feb 11
  * LLVM toolchains in bazel. Slightly working, some linker errors with fuzzing
* Apr 29
  * Large file for autocthon, no build system
