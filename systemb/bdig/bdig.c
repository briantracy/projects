

#include <stdio.h>
#include <string.h>
#include <stdint.h>


void print_hex(const uint8_t *mem, int len) {
    printf("(%d) [", len);
    for (int i = 0; i < len; ++i) {
        printf("%02x ", mem[i]);
    }
    printf("]\n");
}

// Returns the length of the encoded name, or -1 on error
int encode_dns_name(const char *name, uint8_t *output)
{
    if (name == NULL || output == NULL) { return -1; }
    const int name_length = (int)strlen(name);
    if (name_length == 0) { return -1; }

    int encoded_bytes = 1;
    uint8_t label_length = 0;
    uint8_t *length_location = &output[0];
    uint8_t *write_location = length_location + 1;

    for (int i = 0; i < name_length; ++i) {
        if (label_length > 63) { return -1; }
        if (name[i] == '.') {
            *length_location = label_length;
            label_length = 0;
            length_location = write_location;
            write_location = length_location + 1;
            ++encoded_bytes;
        } else {
            *write_location = (uint8_t)name[i];
            ++write_location;
            ++encoded_bytes;
            ++label_length;
        }
    }
    if (label_length != 0) {
        *length_location = label_length;
    }
    return encoded_bytes;
}

int main() {
    uint8_t buff[255];
    int len = encode_dns_name("abc..xyz", buff);
    print_hex(buff, (uint8_t)len);
}
