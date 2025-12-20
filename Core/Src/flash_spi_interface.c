#include "user_comm.h"

// 判断地址是否为扇区起始地址
static inline bool is_first_page_in_sector(uint32_t addr)
{
    return (addr % XT25_F64F_SECTOR_SIZE) == 0;
}

// !!!! flash写使能是一次性的, 不需要失能
void xt25f_cs_low(void)
{
    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET);
}

void xt25f_cs_high(void)
{
    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET);
}

uint8_t spi_txrx(uint8_t data)
{
    uint8_t received_data = 0;
    HAL_SPI_TransmitReceive(&hspi2, &data, &received_data, 1, HAL_MAX_DELAY);
    return received_data;
}

void xt25f_write_enable(void)
{
    xt25f_cs_low();
    spi_txrx(CMD_WRITE_ENABLE);
    xt25f_cs_high();
}

int xt25f_wait_busy(void)
{
    uint32_t timeout = XT25F_BUSY_TIMEOUT;
    uint8_t status;

    do
    {
        xt25f_cs_low();
        spi_txrx(CMD_READ_STATUS_1);
        status = spi_txrx(0xFF);
        xt25f_cs_high();

        if (--timeout == 0)
        {
            return -1; // timeout
        }

    } while (status & SR_WIP);

    return 0;
}

void xt25f_chip_erase(void)
{

    uint32_t start_ts = HAL_GetTick();

    printf("Starting chip erase...\r\n");
    xt25f_write_enable();

    xt25f_cs_low();
    spi_txrx(0xC7); // Chip Erase（0xC7 or 0x60）
    xt25f_cs_high();

    xt25f_wait_busy(); // ⚠ 可能几十秒

    uint32_t end_ts = HAL_GetTick();
    printf("Chip erase took %lu ms\r\n", end_ts - start_ts);
}

void xt25f_erase_sector(uint32_t addr)
{
    uint8_t tx_cmd[4];
    tx_cmd[0] = 0x20; // Sector Erase
    tx_cmd[1] = (addr >> 16) & 0xFF;
    tx_cmd[2] = (addr >> 8) & 0xFF;
    tx_cmd[3] = addr & 0xFF;

    uint8_t rx_cmd[4] = {0};

    xt25f_write_enable();
    xt25f_cs_low();
    HAL_SPI_TransmitReceive(&hspi2, tx_cmd, rx_cmd, 4, HAL_MAX_DELAY);
    xt25f_cs_high();
    xt25f_wait_busy();
}

void xt25f_page_program(uint32_t addr, const uint8_t *buf, uint16_t len)
{
    if (len == 0 || len > 256)
        return;

    // 地址按页对齐检查
    if ((addr % 256) != 0)
        return;

    uint8_t page_buf[256];
    memset(page_buf, 0xFF, 256);
    memcpy(page_buf, buf, len);

    // 如果是扇区的第一个页，则先擦除扇区
    if (is_first_page_in_sector(addr))
    {
        DBG_PRINTF("Erasing sector at address 0x%08lX\r\n", addr);
        xt25f_erase_sector(addr);
    }

    uint8_t tx_cmd[4];
    tx_cmd[0] = 0x02; // Page Program
    tx_cmd[1] = (addr >> 16) & 0xFF;
    tx_cmd[2] = (addr >> 8) & 0xFF;
    tx_cmd[3] = addr & 0xFF;

    xt25f_wait_busy();
    xt25f_write_enable();

    xt25f_cs_low();
    HAL_SPI_Transmit(&hspi2, tx_cmd, 4, HAL_MAX_DELAY);

    // 无论如何写满256字节
    HAL_SPI_Transmit(&hspi2, page_buf, XT25_F64F_PAGE_SIZE, HAL_MAX_DELAY);

    xt25f_cs_high();
    xt25f_wait_busy();
}

void xt25f_read(uint32_t addr, uint8_t *buf, uint16_t len)
{
    xt25f_cs_low();
    spi_txrx(0x03); // Read
    spi_txrx(addr >> 16);
    spi_txrx(addr >> 8);
    spi_txrx(addr);

    for (uint16_t i = 0; i < len; i++)
    {
        buf[i] = spi_txrx(0xFF);
    }

    xt25f_cs_high();
}

void xt25f_read_by_addr(uint32_t addr, uint8_t *buf, uint16_t len)
{
    uint8_t cmd[4] = {
        0x03,
        (addr >> 16) & 0xFF,
        (addr >> 8) & 0xFF,
        addr & 0xFF};

    xt25f_cs_low();

    /* 发送 READ + 地址 */
    HAL_SPI_Transmit(&hspi2, cmd, 4, HAL_MAX_DELAY);

    /* 发送 dummy，同时接收数据 */
    for (uint16_t i = 0; i < len; i++)
    {
        uint8_t dummy = 0xFF;
        HAL_SPI_TransmitReceive(&hspi2, &dummy, &buf[i], 1, HAL_MAX_DELAY);
    }

    xt25f_cs_high();
}

bool xt25f_read_id(uint8_t *id)
{
    uint8_t cmd = CMD_JEDEC_ID;

    xt25f_cs_low();

    /* 发送 RDID 命令 */
    if (HAL_SPI_TransmitReceive_DMA(&hspi2, &cmd, id, 4) != HAL_OK)
    {
        xt25f_cs_high();
        return false;
    }

    xt25f_cs_high();
    return true;
}
