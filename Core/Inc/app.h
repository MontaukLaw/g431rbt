#ifndef _APP_H_
#define _APP_H_

uint16_t ema_u16(uint16_t new_data, uint16_t last_data, uint16_t a_num, uint16_t a_den);

void main_adc_task(void);

void delay_us(uint32_t nus);

void delay_init(void);

extern uint8_t adc_dma_buffer[];

extern volatile uint8_t bat_adc_done;

extern volatile uint8_t tx_buf[];

#endif
