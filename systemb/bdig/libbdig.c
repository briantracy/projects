
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>

#include "libbdig.h"

static const char *resolver_ipv4 = "1.1.1.1";

uint8_t packet_bytes[] = {
  0x34, 0x2a, 0x01, 0x20, 0x00, 0x01, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x01, 0x0a, 0x62, 0x72, 0x69,
  0x61, 0x6e, 0x74, 0x72, 0x61, 0x63, 0x79, 0x03,
  0x78, 0x79, 0x7a, 0x00, 0x00, 0x01, 0x00, 0x01,
  0x00, 0x00, 0x29, 0x10, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00
};

static void insert_transaction_identifier(uint8_t *dns_packet) {
    srand(time(NULL));
    dns_packet[0] = rand();
    dns_packet[1] = rand();
}

int bdig_resolve_v4(const char *domain) {
    fprintf(stderr, "[info] resolving ipv4 for domain: [%s]\n", domain);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "[error] could not bind udp socket: %s\n", strerror(errno));
    }

    struct sockaddr_in resolver;
    resolver.sin_family = AF_INET;
    resolver.sin_port = htons(53);
    if (inet_pton(AF_INET, resolver_ipv4, &(resolver.sin_addr)) != 1) {
        fprintf(stderr, "could not parse resolver ipv4: [%s]\n", resolver_ipv4);
        return 1;
    }
    insert_transaction_identifier(packet_bytes);
    int rc = sendto(sockfd, packet_bytes, sizeof(packet_bytes), 0, (struct sockaddr *)&resolver, sizeof(resolver));
    if (rc < 0) {
        fprintf(stderr, "[error] sendto failed: %s\n", strerror(errno));
        return 1;
    }
    return 1;
}

// Encode a domain name in DNS query format
// Do not perform any name compression such as https://dotat.at/@/2022-07-01-dns-compress.html
//
// a.example.org
// -> [1, 'a', 7, 'e', 'x', 'a', 'm', 'p', 'l', 'e', 3, 'o', 'r', 'g', 0]
int bdig_domain_name_encode(const char *name, uint8_t *buffer) {
    if (name == NULL || buffer == NULL) {
        return -1;
    }
    int last_label_size = 0;
    uint8_t *last_size_byte = buffer;
    uint8_t *encoded_pointer = buffer + 1;
    const int len = strlen(name);
    for (int i = 0; i < len; ++i) {
        if (name[i] == '.') {
            *last_size_byte = last_label_size;
            last_size_byte += last_label_size;
            ++encoded_pointer;
        } else {
            *encoded_pointer++ = name[i];
            ++last_label_size;
        }
    }
    *encoded_pointer = '\0';
    return encoded_pointer - buffer;
}
