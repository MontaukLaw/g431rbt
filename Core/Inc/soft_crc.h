#ifndef _SOFT_CRC_H_
#define _SOFT_CRC_H_

void crc32_init(void);

uint32_t crc32_soft(const uint8_t *data, uint32_t len);



#endif
