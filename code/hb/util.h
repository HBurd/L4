#ifndef HBUTIL_H
#define HBUTIL_H

#include <stdint.h>

#define ARRAY_LENGTH(x) sizeof(x) / sizeof (*x)

// TODO: probably doesn't belong here
// returns host byte order
uint32_t parse_ip4(const char *ip);

#endif // include guard
