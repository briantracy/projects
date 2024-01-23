

#include <array>
#include <cstdint>

#include "libbrp.hpp"

using std::uint8_t;

struct Packet {
    static constexpr int PayloadMaxBytes = 100;
    enum Flags {
        // The following packet types are all mutually exclusive
        Hello =     (1 << 0),
        Goodbye =   (1 << 1),
        Data =      (1 << 2),
        // Heartbeat = (1 << 3), No heartbeats yet
        // Ack can modify any of the above flags.
        Ack =       (1 << 7),
        // Bits 4,5,6 are reserved
    };

    uint8_t flags;
    uint8_t sequence_number;
    std::array<uint8_t, PayloadMaxBytes> payload;
};
static_assert(sizeof(Packet) == 2 + Packet::PayloadMaxBytes, "packet definition malformed");

BRPClient::BRPClient(short listenPort) {

}
