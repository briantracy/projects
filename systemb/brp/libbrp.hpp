#ifndef LIBBRP_HPP
#define LIBBRP_HPP

class BRPClient {

    int write(const char *data, int length);
    int read(char *buff, int length);
};

#endif
