#include "user_comm.h"

static uint32_t crc32_table[256];

void crc32_init(void)
{
    uint32_t poly = 0xEDB88320; // reversed polynomial
    for (uint32_t i = 0; i < 256; i++)
    {
        uint32_t c = i;
        for (int j = 0; j < 8; j++)
            c = (c & 1) ? (poly ^ (c >> 1)) : (c >> 1);
        crc32_table[i] = c;
    }
}

uint32_t crc32_soft(const uint8_t *data, uint32_t len)
{
    uint32_t crc = 0xFFFFFFFF;

    for (uint32_t i = 0; i < len; i++)
        crc = crc32_table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);

    return crc ^ 0xFFFFFFFF;
}
