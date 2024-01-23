


# Progress

* Jan 21, 2024
  * Use `dig` to get wireshark to display a real UDP DNS packet, then Export Packet Bytes into a file, and `cat file > /dev/udp/1.1.1.1/53` to see a fake `dig` session.
* Jan 22, 2024
  * Bazel target for libbdig, create `bdig_resolve_v4` function proto
