#ifndef _FLASH_SPI_INTERFACE_H_
#define _FLASH_SPI_INTERFACE_H_

void xt25f_cs_low(void);
void xt25f_cs_high(void);
uint8_t spi_txrx(uint8_t data);

void xt25f_write_enable(void);

int xt25f_wait_busy(void);

void xt25f_chip_erase(void);

bool xt25f_read_id(uint8_t id[3]);

void xt25f_read(uint32_t addr, uint8_t *buf, uint16_t len);

void xt25f_page_program(uint32_t addr, const uint8_t *buf, uint16_t len);

void xt25f_read_by_addr(uint32_t addr, uint8_t *buf, uint16_t len);


#endif // _FLASH_SPI_INTERFACE_H_
