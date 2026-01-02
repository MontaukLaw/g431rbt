#ifndef _USER_COMM_H_
#define _USER_COMM_H_

#include <stdint.h>
#include <string.h>
#include "main.h"
#include "spi.h"

#include <stdbool.h>
#include <stdio.h>
#include "usart.h"
#include "comm.h"
#include "bl.h"
#include "bat_val.h"
#include "charge_state.h"
#include "app.h"
#include "led.h"
#include "switch_hal.h"
#include "iwdg.h"
#include "sd_card.h"
#include <stdint.h>

#include "XT25F64F.h"
#include "flash_app.h"
#include "flash_spi_interface.h"
#include "soft_crc.h"
#include "algorithm.h"
#include "key.h"

#define DEBUG_PRINT 1

#if DEBUG_PRINT
#define DBG_PRINTF(fmt, ...) \
    printf("[%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)
#else
#define DBG_PRINTF(...) ((void)0)
#endif

typedef struct
{
    GPIO_TypeDef *port;
    uint16_t pin;
} GPIO_Channel;

#define CHARGING_FINISHED 1
#define CHARGING_ONGOING 2
#define CHARGING_NO_POWER 0

#define RESET_GQ_KEY_SHAKE_DELAY 5
// 6秒关灯
#define PWOER_DOWN_COUNTER 350 // 600

#define ICM42688DelayMs(_nms) HAL_Delay(_nms)

#define CH_DEF(n) {D##n##_GPIO_Port, D##n##_Pin}

#define INPUT_CH_NUMBER 16

#define ADC_CHANNEL_NUMBER 16

#define ADC_BUFFER_SIZE 10 // 80 // 10 // Define the size of the ADC buffer

#define TOTAL_POINTS (ADC_CHANNEL_NUMBER * INPUT_CH_NUMBER)

// 点数量加上帧尾, 加左右两边帧头
// 16个字节的MEMS数据
#define OLD_FRAME_LEN (TOTAL_POINTS + 8 + 4 + 16)

#define FRAME_LEN (TOTAL_POINTS + 4)
#define FRAME_HEAD_LEN 4

// 帧头4B,报文1B,类型1B,CMD1B,结果1B,校验和1B
#define TX_BUF_BYTES (TOTAL_POINTS + FRAME_HEAD_LEN + 5)

#define TX_BUF_LEN 512

#define STANDARD_PROTOCAL_LEN 64

#define UART_RX_BUF_LEN STANDARD_PROTOCAL_LEN

// 采集数据指令
#define CMD_GET_DATA 0x01

#define CMD_TYPE_REQUEST 0x00
#define CMD_TYPE_RESPONSE 0x01
#define CMD_TYPE_NOTIFICATION 0x02

#define CMD_TOTAL_LEN 0x08

#define CMD_RESULT_SUCCESS 0x00
#define CMD_RESULT_FAIL 0x01

#define ADC2_DMA_BUF_LEN 100

#define USE_PRESS 1

#define BY_COL 1
#define BY_ROW 0

#define MAX_STATIC_CALI_COUNTER 100

#define QMI8658 1
#define ICM42688 2

#define MEMS_USING QMI8658

#define DEG2RAD 0.017453293f /* 度转弧度 π/180 */

#endif // _USER_COMM_H_
