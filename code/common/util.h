#pragma once

#include <stdint.h>

#define ARRAY_LENGTH(x) sizeof(x) / sizeof (*x)

#define HB_MAX(a, b) ((a) > (b) ? (a) : (b))
#define HB_MIN(a, b) ((a) < (b) ? (a) : (b))

// TODO: probably doesn't belong here
// returns host byte order
uint32_t parse_ip4(const char *ip);
