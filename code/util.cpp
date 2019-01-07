#include "hb/util.h"

#include <cstdio>

uint32_t parse_ip4(char *ip)
{
    uint8_t b0, b1, b2, b3;
    sscanf(ip, "%hhu.%hhu.%hhu.%hhu", &b0, &b1, &b2, &b3);
    return b0 << 24 | b1 << 16 | b2 << 8 | b3;
}
