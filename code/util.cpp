#include "hb/util.h"

#include <cstdlib>

uint32_t parse_ip4(const char *ip)
{
    uint8_t bytes[4];

	for (int i = 0; i < 4; i++)
	{
		// read number (byte) from ip string
		char *next_byte_start;
		long unsigned int byte = strtoul(ip, &next_byte_start, 10);
		// check that we still have a valid ip
		// we are limited by strtoul which returns invalid number strings as 0
		if (byte > 255)
		{
			return 0;	// 0.0.0.0 represents an invalid ip
		}
		// check that the separator is valid
		if (i < 3 && *next_byte_start != '.')
		{
			return 0;
		}

		bytes[i] = (uint8_t)byte;
		ip = next_byte_start + 1;
	}

	return bytes[0] << 24 | bytes[1] << 16 | bytes[2] << 8 | bytes[3];
}