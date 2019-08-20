#pragma once

#include <stdint.h>
#include <assert.h>

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof (*x))

#define HB_MAX(a, b) ((a) > (b) ? (a) : (b))
#define HB_MIN(a, b) ((a) < (b) ? (a) : (b))

// Bitfields
#define BF_SIZE(type, bits) (((bits) + 8 * sizeof(type) - 1) / (8 * sizeof(type))) // round up
#define BF_INDEX(bf, bit)  ((bit) / (8 * sizeof(*(bf))))
#define BF_OFFSET(bf, bit) ((bit) % (8 * sizeof(*(bf))))
#define BF_READ(bf, bit)   (bf)[BF_INDEX(bf, bit)] & 1 << BF_OFFSET(bf, bit)
#define BF_SET(bf, bit)    ((bf)[BF_INDEX(bf, bit)] |= 1 << BF_OFFSET(bf, bit))
#define BF_RESET(bf, bit)  ((bf)[BF_INDEX(bf, bit)] &= ~(1 << BF_OFFSET(bf, bit)))
#define BF_WRITE(bf, bit, val) BF_RESET(bf, bit); val ? BF_SET(bf, bit) : 0

template <typename T, uint32_t max_size>
struct Array
{
    uint32_t size = 0;
    T data[max_size];
    void push(T element)
    {
        assert(size < max_size);
        data[size++] = element;
    }

    T pop()
    {
        assert(size);
        return data[--size];
    }

    T& operator[](uint32_t idx)
    {
        return data[idx];
    }
};

// TODO: probably doesn't belong here
// returns host byte order
uint32_t parse_ip4(const char *ip);
