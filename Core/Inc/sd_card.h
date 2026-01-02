#ifndef _SD_CARD_H_
#define _SD_CARD_H_

#define COL_NUM 8
#define ROW_NUM 14
#define TOTAL_POINT_NUM (COL_NUM * ROW_NUM)
#define MEMS_FLOAT_BYTES 24
// SDLG
#define SD_LOG_MAGIC 0x53444c47
typedef struct
{
    uint8_t sensor_data[TOTAL_POINT_NUM];
    uint8_t mems_data[MEMS_FLOAT_BYTES]; // 6轴
} sd_raw_data_t;

typedef struct
{
    uint32_t magic;     // 固定标志
    uint32_t seq;       // 递增序号
    uint32_t timestamp; // 可选
    uint32_t crc;       // 3个raw_data的crc32
    sd_raw_data_t data_1;
    sd_raw_data_t data_2;
    sd_raw_data_t data_3;

} sd_log_page_t;

#define SD_CS_Low() HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET);
#define SD_CS_High() HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET);

uint8_t SD_Init_SDSC(void);

void test_sd_card(int32_t block);

#define TEST_BLOCK 100
#define BYTES_PER_BLOCK 512
#define SD_BUSY_TIMEOUT 500000 // 约 500ms，视 SPI 速度

// #define SD_OK 0xFF00

// #define SD_ERR 0xFFFF
// #define SD_TIMEOUT 0xFFFE
// #define SD_WRITE_ERR 0xFFFD
#define SD_IDLE 0x01
#define SD_OK 0x00
#define SD_ERR 0x01
#define SD_TIMEOUT 0xFF
#define SD_TOKEN_DATA 0xFE
#define SD_TOKEN_START 0xFE
#define SD_DATA_ACCEPT 0x05

uint8_t Write_Single_Block(uint32_t block, uint8_t *data);

void SD_SPI_SetSlow(void);

void SD_SPI_SetFast(void);

void look_for_used_block_test(void);

extern uint8_t WriteBuffer[];

#endif
