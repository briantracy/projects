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

    ./middleman 127.0.0.1 9999 127.0.0.1 8888 5
    ncat -u --source-port 5555 127.0.0.1 9999
    ncat -u --source-port 8888 127.0.0.1 9999
*/

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>



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

    struct sockaddr_in first_sender;
    bool got_first_packet = false;
    srand(time(NULL));
    const int drop_percent = atoi(argv[5]);
    while (1) {
        char buff[1 << 16];
        struct sockaddr_in sender;
        socklen_t socklen = sizeof(struct sockaddr);
        const ssize_t bytes_read = recvfrom(source_socket, buff, sizeof(buff), 0, (struct sockaddr *)&sender, &socklen);
        if (bytes_read < 0) {
            perror("recvfrom");
            return 1;
        }
        if (!got_first_packet) {
            first_sender = sender;
            got_first_packet = true;
        }
        printf("got packet of size: %zd!\n", bytes_read);
        const int roll = rand() % 100;
        if (roll < drop_percent) {
            printf("dropping packet (roll was %d)\n", roll);
            continue;
        }


        struct sockaddr_in destination;
        if (sender.sin_addr.s_addr == first_sender.sin_addr.s_addr &&
            sender.sin_port == first_sender.sin_port) {
            destination = sink_addr;
        } else if (sender.sin_addr.s_addr == sink_addr.sin_addr.s_addr &&
                sender.sin_port == sink_addr.sin_port) {
            destination = first_sender;
        } else {
            fprintf(stderr, "got unknown sender, port: %d\n", ntohs(sender.sin_port));
            return 1;
        }

        const ssize_t bytes_sent = sendto(source_socket, buff, (size_t)bytes_read, 0, (struct sockaddr *)&destination, sizeof(destination));
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
