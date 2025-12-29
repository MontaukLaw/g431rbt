#include "user_comm.h"
volatile uint8_t main_adc_busy = 0;
volatile uint8_t bat_adc_done = 0;
uint8_t adc_dma_buffer[ADC_BUFFER_SIZE] = {0};
volatile uint8_t tx_buf[TX_BUF_LEN] = {0}; // 发送缓冲区
volatile uint8_t points_data_after_proc[TOTAL_POINTS] = {0};
volatile uint8_t points_data[FRAME_LEN] = {0};
__IO static uint32_t fac_us = 0;

// 数据选择过程需要优化
void adc_data_handler_with_idx(uint8_t point_nmb)
{
    // 简单计算平均值
    uint32_t adc_sum = 0;
    uint32_t i = 0;

    for (i = 0; i < ADC_BUFFER_SIZE; i++)
    {
        adc_sum += adc_dma_buffer[i];
    }
    float result = adc_sum / (ADC_BUFFER_SIZE - 0);
    points_data[point_nmb] = result; // 将结果存储到points_data中
}

void main_adc_task(void)
{

    uint16_t input_idx = 0;
    uint16_t adc_idx = 0;
    uint16_t point_nmb = 0;

    for (input_idx = 0; input_idx < INPUT_CH_NUMBER; input_idx++)
    {

        // 先打开GPIO输出
        // set_channel_pin(input_idx_v2[input_idx], GPIO_PIN_SET);
        set_channel_pin(input_idx, GPIO_PIN_SET);

        for (adc_idx = 0; adc_idx < ADC_CHANNEL_NUMBER; adc_idx++)
        {

            set_adc_ch(adc_idx);

            delay_us(10);

            point_nmb = input_idx * ADC_CHANNEL_NUMBER + adc_idx;
            adc_data_handler_with_idx(point_nmb);
        }

        // 关闭GPIO输出
        // set_channel_pin(input_idx_v2[input_idx], GPIO_PIN_RESET);
        set_channel_pin(input_idx, GPIO_PIN_RESET);
    }

#if USE_PRESS
    // 数据压缩处理
    press256((const uint8_t *)points_data, (uint8_t *)points_data_after_proc, ADC_CHANNEL_NUMBER, INPUT_CH_NUMBER, 5, BY_COL);

#endif
    // 问答模式使用comm_handler
    uart_send();
}

uint8_t ema_u8(uint8_t new_data, uint8_t last_data, uint8_t a_num, uint8_t a_den)
{
    return (uint8_t)((a_num * new_data + (a_den - a_num) * last_data) / a_den);
}

uint16_t ema_u16(uint16_t new_data, uint16_t last_data, uint16_t a_num, uint16_t a_den)
{
    return (uint16_t)((a_num * new_data + (a_den - a_num) * last_data) / a_den);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1)
    {
        main_adc_busy = 0;
    }
    else if (hadc->Instance == ADC2)
    {
        bat_adc_done = 1;
    }
}

void delay_init(void)
{
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK); // SysTick频率为HCLK
    fac_us = 170;                                        // 不论是否使用OS,fac_us都需要使用
}

void delay_ms(uint32_t ms)
{
    uint32_t i;
    for (i = 0; i < ms; i++)
    {
        delay_us(1000);
        HAL_IWDG_Refresh(&hiwdg);
    }
}

void delay_us(uint32_t nus)
{
    uint32_t ticks;
    uint32_t told, tnow, tcnt = 0;
    uint32_t reload = SysTick->LOAD; // LOAD的值
    ticks = nus * fac_us;            // 需要的节拍数
    told = SysTick->VAL;             // 刚进入时的计数器值
    while (1)
    {
        tnow = SysTick->VAL;
        if (tnow != told)
        {
            if (tnow < told)
                tcnt += told - tnow; // 这里注意一下SYSTICK是一个递减的计数器就可以了.
            else
                tcnt += reload - tnow + told;
            told = tnow;
            if (tcnt >= ticks)
                break; // 时间超过/等于要延迟的时间,则退出.
        }
    };
}