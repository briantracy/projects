/*
    middleman is a program that receives UDP packets on a specified
    IP and port, then forwards them along to a separate IP and port.

    It can also be told to drop a certain percentage of its packets.

    For a basic demo:
        bazel run :middleman -- 127.0.0.1 5555 127.0.0.1 9999 0
        printf 'packet-here' > /dev/udp/127.0.0.1/5555
    Observe that the packet makes it to port 9999.

    For a more fun demo, set up two middlemen, each pointed at the other:
        bazel run :middleman -- 127.0.0.1 5555 127.0.0.1 9999 0
        bazel run :middleman -- 127.0.0.1 9999 127.0.0.1 5555 0
    Then kick off the infinite loop with a single packet:
        printf 'loop' > /dev/udp/127.0.0.1/5555
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


int main(const int argc, const char *const argv[]) {
    if (argc != 6) {
        fprintf(stderr, "usage: %s <src_ip> <src_port> <dst_ip> <dst_port> <drop_percent>\n", argv[0]);
        return 1;
    }

    const int source_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (source_socket == -1) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in source_addr;
    source_addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, argv[1], &(source_addr.sin_addr)) != 1) {
        fprintf(stderr, "could not parse source ipv4: [%s]\n", argv[1]);
        return 1;
    }
    source_addr.sin_port = htons(atoi(argv[2]));
    if (source_addr.sin_port == 0) {
        fprintf(stderr, "invalid source port: [%s]\n", argv[2]);
        return 1;
    }

    if (bind(source_socket, (struct sockaddr *)&source_addr, sizeof(struct sockaddr)) == -1) {
        perror("bind");
        return 1;
    }

    struct sockaddr_in sink_addr;
    sink_addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, argv[3], &(sink_addr.sin_addr)) != 1) {
        fprintf(stderr, "could not parse sink ipv4: [%s]\n", argv[3]);
        return 1;
    }
    sink_addr.sin_port = htons(atoi(argv[4]));
    if (sink_addr.sin_port == 0) {
        fprintf(stderr, "invalid sink port: [%s]\n", argv[4]);
        return 1;
    }

    while (1) {
        char buff[1 << 16];
        struct sockaddr_in sender;
        socklen_t socklen = sizeof(struct sockaddr);
        const ssize_t bytes_read = recvfrom(source_socket, buff, sizeof(buff), 0, (struct sockaddr *)&sender, &socklen);
        if (bytes_read < 0) {
            perror("recvfrom");
            return 1;
        }
        printf("got packet of size: %zd!\n", bytes_read);

        const ssize_t bytes_sent = sendto(source_socket, buff, (size_t)bytes_read, 0, (struct sockaddr *)&sink_addr, sizeof(sink_addr));
        if (bytes_sent < 0) {
            perror("sendto");
        }
        if (bytes_sent != bytes_read) {
            fprintf(stderr, "mismatch in bytes received vs sent: %zd vs %zd\n", bytes_read, bytes_sent);
            return 1;
        }
    }

    printf("bye\n");
    return 0;
}
