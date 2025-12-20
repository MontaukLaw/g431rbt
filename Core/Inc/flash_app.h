#ifndef _FLASH_APP_H_
#define _FLASH_APP_H_

/* ===== JEDEC 通用指令 ===== */
#define CMD_WRITE_ENABLE 0x06
#define CMD_WRITE_DISABLE 0x04
#define CMD_READ_STATUS_1 0x05
#define CMD_PAGE_PROGRAM 0x02
#define CMD_READ_DATA 0x03
#define CMD_FAST_READ 0x0B
#define CMD_SECTOR_ERASE_4K 0x20
#define CMD_CHIP_ERASE 0xC7
#define CMD_JEDEC_ID 0x9F
#define CMD_ENABLE_RESET 0x66
#define CMD_RESET_DEVICE 0x99

/* ===== Status Register bits ===== */
#define SR_WIP (1 << 0)
#define SR_WEL (1 << 1)

/* ===== 通用参数 ===== */
// #define FLASH_PAGE_SIZE 256
// #define FLASH_SECTOR_SIZE 4096

typedef struct
{
    uint32_t magic;     // 固定标志
    uint32_t seq;       // 递增序号
    uint32_t timestamp; // 可选
    uint32_t crc;       // sensor_data 和 mems_data 的 CRC32 校验值
    uint8_t sensor_data[TOTAL_POINT_NUM];
    uint8_t mems_data[MEMS_FLOAT_BYTES]; // 6轴

} log_page_t;

#define LOG_PAGE_SIZE sizeof(log_page_t)
#define LOG_MAGIC 0x4C4F4731 // "LOG1"

#define PAGE_COUNT XT25_F64F_PAGE_NUMBER
#define LOG_BASE 0x00000000 // 日志存储起始地址

#define LOG_ADDR(idx) (FLASH_BASE_ADDR + (idx) * FLASH_PAGE_SIZE)

void flash_test(void);

void print_flash_id(void);

uint16_t get_writted_number(void);

#endif
