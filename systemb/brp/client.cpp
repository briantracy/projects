

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <array>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <string_view>
#include <thread>

class BRPClient {
  // Calls recv in a loop
  std::thread listen_thread;

  // Sleep with these while waiting for an ack.
  std::mutex ack_mutex;
  std::condition_variable ack_cv;

  // Sleep with these while waiting for inbound data
  std::mutex read_mutex;
  std::condition_variable read_cv;

  uint8_t seqno_sent = 0;
  int ack_number_received = -1;
  uint8_t peer_seqno = 0;

  std::vector<uint8_t> read_buffer;

  int sockfd;

 public:
  BRPClient(const char *ip, short port, bool is_server);
  ~BRPClient();
  int write(const char *data, int length);
  int read(char *buff, int length);

 private:
  static constexpr uint8_t AckFlag = 0b1;
  static constexpr uint8_t DataFlag = 0b10;
  static constexpr int PacketMaxSize = 1000;
  static constexpr int MaxTransmissions = 10;
  void listen_thread_func();
};

BRPClient::BRPClient(const char *ip, short port, bool is_server) {
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd == -1) {
    throw std::runtime_error("could not create socket");
  }
  sockaddr_in source_addr = {};
  source_addr.sin_family = AF_INET;
  if (is_server) {
    if (inet_pton(AF_INET, ip, &(source_addr.sin_addr)) != 1) {
      fprintf(stderr, "could not parse source ipv4: [%s]\n", ip);
    }
  } else {
  }
  source_addr.sin_port = is_server ? htons(port) : 0;
  if (bind(sockfd, (struct sockaddr *)&source_addr, sizeof(struct sockaddr)) ==
      -1) {
    throw std::runtime_error{"bind error: " +
                             std::string(std::strerror(errno))};
  }

  if (!is_server) {
    sockaddr_in peer = {};
    peer.sin_family = AF_INET;
    if (inet_pton(AF_INET, ip, &(peer.sin_addr)) != 1) {
      fprintf(stderr, "could not parse source ipv4: [%s]\n", ip);
    }
    peer.sin_port = htons(port);
    if (connect(sockfd, (sockaddr *)&peer, sizeof(peer)) != 0) {
      perror("connect");
      fprintf(stderr, "cannot connect");
    }
  }

  listen_thread = std::thread(&BRPClient::listen_thread_func, this);
  // listen_thread.detach();
}

BRPClient::~BRPClient() { listen_thread.join(); }

using std::uint8_t;

int BRPClient::write(const char *data, int length) {
  // std::this_thread::sleep_for(std::chrono::seconds(1));
  constexpr int PayloadMaxSize = PacketMaxSize - 2;
  int bytes_sent = 0;
  do {
    char packet[PacketMaxSize];
    packet[0] = DataFlag;
    packet[1] = seqno_sent;
    const int payload_bytes = std::min(PayloadMaxSize, length - bytes_sent);
    memcpy(packet + 2, data + bytes_sent, payload_bytes);

    int tries = 1;
    for (; tries <= MaxTransmissions; ++tries) {
      send(sockfd, packet, payload_bytes + 2, 0);
      std::unique_lock<std::mutex> lock(ack_mutex);
      ack_cv.wait_for(lock, std::chrono::seconds(1));
      if (ack_number_received == seqno_sent) {
        break;
      }
      fprintf(stderr, "RETRANSMISSION %d\n", seqno_sent);
    }
    if (tries > MaxTransmissions) {
      throw std::runtime_error("too many retries");
    }
    bytes_sent += payload_bytes;
    ++seqno_sent;
  } while (bytes_sent < length);
  return 0;
}

int BRPClient::read(char *buff, int length) {
  std::unique_lock<std::mutex> lock(read_mutex);
  while (read_buffer.empty()) {
    read_cv.wait(lock);
  }
  const int bytes_read = std::min(length, (int)read_buffer.size());
  printf("reading %d bytes\n", bytes_read);
  std::memcpy(buff, read_buffer.data(), bytes_read);
  read_buffer.erase(read_buffer.begin(), read_buffer.begin() + bytes_read);
  return bytes_read;
}

void BRPClient::listen_thread_func() {
  std::cout << "listen thread\n";
  bool connected = false;
  while (true) {
    char buff[1 << 16];
    sockaddr_in sender;
    socklen_t socklen = sizeof(sockaddr);
    fprintf(stderr, "calling recvfrom\n");
    const ssize_t bytes_read =
        recvfrom(sockfd, buff, sizeof(buff), 0, (sockaddr *)&sender, &socklen);

    if (bytes_read < 0) {
      if (errno == ECONNREFUSED) {
        // close connection here
        fprintf(stderr, "remote connection close detected\n");
      }
      throw std::runtime_error{"recvfrom error: " +
                               std::string(std::strerror(errno))};
    }
    if (bytes_read < 2) {
      throw std::runtime_error("packet too small");
    }
    if (!connected) {
      int r = connect(sockfd, (sockaddr *)&sender, socklen);
      assert(r == 0);
      connected = true;
    }
    printf("got packet of size: %zd!\n", bytes_read);

    if (buff[0] == DataFlag) {
      printf("got data");
      const uint8_t seqno = static_cast<uint8_t>(buff[1]);
      printf(" -- remote seqno = %d\n", seqno);
      const uint8_t ack_packet[2] = {AckFlag, seqno};
      int r = send(sockfd, ack_packet, sizeof(ack_packet), 0);
      if (r == -1) {
        perror("sendto");
      }
      if (seqno != peer_seqno) {
        fprintf(stderr, "wrong data seqno received: %x (expected) vs %x (given)\n",
                peer_seqno, seqno);
        continue;
      }
      peer_seqno++;
      const int payload_length = bytes_read - 2;
      if (payload_length == 0) {
        // TODO detect end of transmission
      }
      std::unique_lock<std::mutex> lock(read_mutex);
      std::copy(&buff[2], &buff[bytes_read], std::back_inserter(read_buffer));
      read_cv.notify_one();
    }

    if (buff[0] == AckFlag) {
      fprintf(stderr, "got ack\n");
      const uint8_t ack_seqno = buff[1];
      std::unique_lock<std::mutex> lock(ack_mutex);
      ack_number_received = ack_seqno;
      ack_cv.notify_one();
    }
  }
}

int main(const int argc, const char *const argv[]) {
  const char *ip = argv[1];
  const uint16_t port = atoi(argv[2]);
  if (argv[3][0] == 'c') {
    BRPClient client(ip, port, false);
    char buff[1024];
    int bytes_sent = 0;
    int r = 0;
    while ((r = read(STDIN_FILENO, buff, sizeof(buff))) > 0) {
      client.write(buff, r);
      bytes_sent += r;
    }
    fprintf(stderr, "connection done, wrote %d bytes\n", bytes_sent);
    client.write(nullptr, 0);
    fprintf(stderr, "sent terminate message");
  } else {
    BRPClient server(ip, port, true);
    while (true) {
      constexpr size_t SIZE = 1024;
      char buff[SIZE + 1] = {0};
      int bytes_read = server.read(buff, SIZE);
      printf("read from client: [%s]\n", buff);
      server.write(buff, bytes_read);
    }
  }
}