#include "user_comm.h"

static uint32_t g_log_index = 0;

static bool is_page_empty(uint32_t addr)
{
    uint8_t buf[XT25_F64F_PAGE_SIZE];

    xt25f_read_by_addr(addr, buf, XT25_F64F_PAGE_SIZE);

    for (int i = 0; i < XT25_F64F_PAGE_SIZE; i++)
    {
        if (buf[i] != 0xFF)
            return false;
    }
    return true;
}

uint32_t find_next_write_addr(void)
{
    uint32_t addr = LOG_BASE;
    uint32_t end = LOG_BASE + XT25_F64F_PAGE_NUMBER * XT25_F64F_PAGE_SIZE;
    while (addr < end)
    {
        if (is_page_empty(addr))
        {
            DBG_PRINTF("Next write page at 0x%08lX\r\n", addr);
            return addr;
        }
        addr += XT25_F64F_PAGE_SIZE;
    }

    /* Flash 写满的处理策略（任选） */
    DBG_PRINTF("Flash full, wrapping to base\r\n");
    // 全片擦除
    xt25f_chip_erase();
    return LOG_BASE;
}

uint32_t find_next_write_addr_old(void)
{
    log_page_t page;
    uint32_t max_seq = 0;
    int max_index = -1;

    for (int i = 0; i < PAGE_COUNT; i++)
    {
        // xt25f_read(LOG_BASE + i * 256, (uint8_t *)&page, sizeof(page));
        xt25f_read_by_addr(LOG_BASE + i * 256, (uint8_t *)&page, sizeof(page));

        // log_page.magic
        // DBG_PRINTF("Log %d: Magic=0x%08lX\r\n", i, page.magic);

        if (page.magic == LOG_MAGIC)
        {
            if (page.seq > max_seq)
            {
                max_seq = page.seq;
                max_index = i;
            }
        }
    }

    DBG_PRINTF("Max seq: %lu at index %d\r\n", max_seq, max_index);

    return LOG_BASE + ((max_index + 1) % PAGE_COUNT) * 256;
}

void log_flash_init(void)
{
    xt25f_chip_erase(); // 客户要求：每次全清
    g_log_index = 0;
}

void print_flash_id(void)
{
    uint8_t id[4];
    if (xt25f_read_id(id))
    {
        printf("Flash ID: %02X %02X %02X %02X\r\n", id[0], id[1], id[2], id[3]);
    }
    else
    {
        printf("Failed to read Flash ID\r\n");
    }
}

void flash_test(void)
{
    uint8_t id[4];
    if (xt25f_read_id(id))
    {
        if (id[1] != 0x0B || id[2] != 0x40 || id[3] != 0x17)
        {
            DBG_PRINTF("Unexpected Flash ID, aborting test\r\n");
            return;
        }
        else
        {

            DBG_PRINTF("Good flash id\r\n");
        }
    }
    else
    {
        DBG_PRINTF("Failed to read Flash ID\r\n");
        return;
    }

    // log_flash_init();

    log_page_t log_page;
    log_page.magic = LOG_MAGIC;
    log_page.seq = 0;
    log_page.timestamp = 0x12345678;

    memset(log_page.sensor_data, 0xAA, TOTAL_POINT_NUM);
    memset(log_page.mems_data, 0x55, 24);

    // 这里的含义是计算除了magic、seq、timestamp和crc字段之外的数据的CRC
    log_page.crc = crc32_soft((uint8_t *)&(log_page.sensor_data), sizeof(log_page) - 16);
    uint32_t addr = 0;
    for (int i = 0; i < 17; i++)
    {
        log_page.seq = i;
        log_page.timestamp += 1;
        log_page.sensor_data[0] = i; // 变化一点数据
        log_page.crc = crc32_soft((uint8_t *)&(log_page.sensor_data), sizeof(log_page) - 16);

        addr = find_next_write_addr();
        DBG_PRINTF("Writing log %d to address 0x%08lX crc: 0x%08lX\r\n", i, addr, log_page.crc);

        xt25f_page_program(addr, (uint8_t *)&log_page, sizeof(log_page));
    }

    // 读取验证
    for (int i = 0; i < 17; i++)
    {
        xt25f_read_by_addr(addr - i * 256, (uint8_t *)&log_page, sizeof(log_page));

        if (log_page.magic != LOG_MAGIC)
        {
            DBG_PRINTF("Log %d: Invalid magic\r\n", i);
            continue;
        }

        uint32_t crc = crc32_soft((uint8_t *)&(log_page.sensor_data), sizeof(log_page) - 16);
        if (crc != log_page.crc)
        {
            DBG_PRINTF("Log %d: CRC error\r\n", i);
            continue;
        }

        DBG_PRINTF("Log %d: Seq=%lu, Timestamp=0x%08lX, CRC=0x%08lX\r\n",
                   i, log_page.seq, log_page.timestamp, log_page.crc);
    }
}
