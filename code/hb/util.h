#ifndef HBUTIL_H
#define HBUTIL_H

#define ARRAY_LENGTH(x) sizeof(x) / sizeof (*x)

// TODO: probably doesn't belong here
// returns host byte order
uint32_t parse_ip4(const char *ip);

#ifdef FAST_BUILD
#include "util.cpp"
#endif

#endif // include guard
