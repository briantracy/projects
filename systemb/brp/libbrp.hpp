#ifndef LIBBRP_HPP
#define LIBBRP_HPP

#include <string_view>

class BRPClient {

    int sockfd_;
public:

    static BRPClient client(std::string_view remote_ip);
    BRPClient(short listenPort);
    int write(const char *data, int length);
    int read(char *buff, int length);
};

#endif
