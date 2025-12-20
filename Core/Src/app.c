#include "user_comm.h"

uint8_t ema_u8(uint8_t new_data, uint8_t last_data, uint8_t a_num, uint8_t a_den)
{
    return (uint8_t)((a_num * new_data + (a_den - a_num) * last_data) / a_den);
}

uint16_t ema_u16(uint16_t new_data, uint16_t last_data, uint16_t a_num, uint16_t a_den)
{
    return (uint16_t)((a_num * new_data + (a_den - a_num) * last_data) / a_den);
}