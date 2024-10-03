#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "crc.h"

#define CRC64_POLYNOMIAL 0x42F0E1EBA9EA3693
uint64_t crc64_table[256];

static void init_crc64_table() {
	for (uint64_t i = 0; i < 256; i++) 
	{
		uint64_t crc = i;
		for (int j = 0; j < 8; j++) 
		{
			if (crc & 1)
				crc = (crc >> 1) ^ CRC64_POLYNOMIAL;
			else 
				crc >>= 1;
		}

		crc64_table[i] = crc;
	}
}

uint64_t crc64(const void *data, size_t len) {
	static bool table_initialized = false;

	if (!table_initialized) 
	{
		init_crc64_table();
		table_initialized = true;
	}

	const uint8_t *ptr = (const uint8_t*)data;
	uint64_t crc = 0xffffffffffffffffULL;

	while (len--) 
	{
		crc = (crc >> 8) ^ crc64_table[*ptr++];
		crc &= 0xffffffffffffffffULL;
	}

	return ~crc;
}

