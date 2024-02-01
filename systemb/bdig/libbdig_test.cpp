
#include <gtest/gtest.h>

extern "C" {
#include "libbdig.h"
}

TEST(libbdig_test, dns_name) {

    uint8_t buffer[256];
    ASSERT_EQ(bdig_domain_name_encode(nullptr, buffer), -1);
    ASSERT_EQ(bdig_domain_name_encode("a", nullptr), -1);


}