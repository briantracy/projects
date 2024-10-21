

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <array>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <string_view>
#include <thread>

struct Packet {
  static constexpr int PayloadMaxBytes = 20;
  static constexpr int PacketMinBytes = 1;
  static constexpr int PacketMaxBytes = PacketMinBytes + PayloadMaxBytes;
  // These occupy the first 3 bits of the packet
  enum Flags : uint8_t {
    Hello = 0b00,
    Goodbye = 0b01,
    Data = 0b10,
    Heartbeat = 0b11,
    // Ack can modify any of the above types.
    Ack = 0b100,
  };

  std::array<uint8_t, PayloadMaxBytes> payload;
  uint8_t flags;
  uint8_t sequence_number;
  uint8_t payload_length;

  std::string to_string() const {
    return "Packet{type=" + std::to_string(flags & ~Flags::Ack) +
           ",ack=" + std::to_string(!!(flags & Flags::Ack)) +
           ",seq=" + std::to_string(sequence_number) +
           ",len=" + std::to_string(payload_length) + "}";
  }
};

constexpr uint8_t PacketFlagMask = 0b00000011;
enum PacketFlags {
  Data = 0b00,
  Ack = 0b01,
};


class BRPClient {
  // Calls recv in a loop
  std::thread listen_thread;
  // Sleep with these while waiting for an ack
  std::mutex ack_mutex;
  std::condition_variable ack_cv;

  int sequence_number_sent;
  int ack_number_received;

  // also need read seqno so we know what to expect.

  int sockfd;
  bool connected;
public:

  BRPClient(const char *ip, short port, bool is_server);
  ~BRPClient();
  int write(const char *data, int length);
  int read(char *buff, int length);

 private:
  void listen_thread_func();
};

BRPClient::BRPClient(const char *ip, short port, bool is_server) : connected(!is_server) {
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd == -1) {
    throw std::runtime_error("could not create socket");
  }
  sockaddr_in source_addr = {};
  source_addr.sin_family = AF_INET;
  if (is_server) {
    if (inet_pton(AF_INET, "127.0.0.1", &(source_addr.sin_addr)) != 1) {
      fprintf(stderr, "could not parse source ipv4: [%s]\n", "127.0.0.1");
    }
  } else {
  }
  source_addr.sin_port = is_server ? htons(port) : 0;
  if (bind(sockfd, (struct sockaddr *)&source_addr,
           sizeof(struct sockaddr)) == -1) {
    throw std::runtime_error{"bind error: " +
                             std::string(std::strerror(errno))};
  }

  if (!is_server) {
    sockaddr_in peer = {};
    peer.sin_family = AF_INET;
    if (inet_pton(AF_INET, ip, &(peer.sin_addr)) != 1) {
      fprintf(stderr, "could not parse source ipv4: [%s]\n", "127.0.0.1");
    }
    peer.sin_port = htons(port);
    if (connect(sockfd, (sockaddr *)&peer, sizeof(peer)) != 0) {
      perror("connect");
      fprintf(stderr, "cannot connect");
    }
  }

  listen_thread = std::thread(&BRPClient::listen_thread_func, this);
  listen_thread.detach();
}


BRPClient::~BRPClient() {
  //listen_thread.join();
}

using std::uint8_t;

int BRPClient::write(const char *data, int length) {
  assert(connected);
  constexpr int packet_size = 10;
  for (int i = 0; i < length; i+= packet_size) {
    for (int tries = 0; tries < 3; ++tries) {
      char packet[packet_size + 1];
      packet[0] = 0;
      memcpy(packet + 1, data, length);
      send(sockfd, packet, length + 1, 0);
      std::unique_lock<std::mutex> lock(ack_mutex);
      ack_cv.wait_for(lock, std::chrono::seconds(1));
      if (ack_number_received == sequence_number_sent) {
        break;
      }
    }
  }
}
int BRPClient::read(char *buff, int length) { return 0; }


void BRPClient::listen_thread_func() {
  std::cout << "listen thread\n";

  while (true) {
    char buff[1 << 16];
    sockaddr_in sender;
    socklen_t socklen = sizeof(sockaddr);
    const ssize_t bytes_read = recvfrom(sockfd, buff, sizeof(buff), 0,
                                        (sockaddr *)&sender, &socklen);

    if (bytes_read < 0) {
      throw std::runtime_error{"recvfrom error: " +
                               std::string(std::strerror(errno))};
    }
    if (!connected) {
      int r = connect(sockfd, (sockaddr *)&sender, socklen);
      assert(r == 0);
      connected = true;
    }
    printf("got packet of size: %zd!\n", bytes_read);

    if ((buff[0] & PacketFlagMask) == PacketFlags::Data) {
      printf("got data");
      if (bytes_read < 2) {
        printf("malformed packet -- too small\n");
        break;
      }
      const uint8_t seqno = static_cast<uint8_t>(buff[1]);
      printf("remote seqno = %d\n", seqno);
      const uint8_t ack_packet[2] = {PacketFlags::Ack, seqno};
      int r = send(sockfd, ack_packet, sizeof(ack_packet), 0);
      if (r == -1) {
        perror("sendto");
      }
    }

    if ((buff[0] & PacketFlagMask) == PacketFlags::Ack) {
      printf("got ack");
      if (bytes_read < 2) {
        printf("malformed packet -- too small\n");
        break;
      }
    }

  }
}

int main(const int argc, const char *const argv[]) {
  BRPClient client("127.0.0.1", 9989, false);
  BRPClient server("127.0.0.1", 9989, true);

  client.write("qqq", 3);
  char buff[3];
  server.read(buff, 3);
}