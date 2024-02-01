#pragma once

#include <stdint.h>

// TODO figure out the proper return type. Is it a out param of
// in_addr?
int bdig_resolve_v4(const char *domain);

// DNS Encode the given `name` into the provided `buffer`.
// It is the caller's responsibility to ensure that buffer is
// at least as large as `strlen(name)` bytes.
int bdig_domain_name_encode(const char *name, uint8_t *buffer);