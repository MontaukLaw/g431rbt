#include "user_comm.h"

uint32_t SD_BlockNR = 3862528; // 总 block 数（512B/block）

uint8_t ReadBuffer[1024] = {0};
uint8_t WriteBuffer[1024] = {0};

uint8_t sd_is_sdhc = 0; // 1 = SDHC/SDXC, 0 = SDSC

static uint8_t SD_SPI_TxRx(uint8_t tx);

static inline uint32_t SD_BlockAddr(uint32_t block)
{
    return sd_is_sdhc ? block : (block * 512);
}

uint8_t SD_Wait_NotBusy(void)
{
    uint32_t timeout = SD_BUSY_TIMEOUT;

    while (timeout--)
    {
        if (SD_SPI_TxRx(0xFF) == 0xFF)
            return 0; // OK
    }

    return 1; // Timeout
}

static uint8_t SD_SPI_TxRx(uint8_t tx)
{
    uint8_t rx = 0xFF;
    HAL_SPI_TransmitReceive(&hspi2, &tx, &rx, 1, HAL_MAX_DELAY);
    return rx;
}

// 发1个byte
void SD_Write(uint16_t IOData)
{
    uint8_t tx = (uint8_t)(IOData & 0xFF);
    SD_SPI_TxRx(tx);
}

// 发2个字节
void SD_2Byte_Write(uint16_t data)
{
    uint8_t tx[2];
    tx[0] = (uint8_t)(data >> 8);   // MSB
    tx[1] = (uint8_t)(data & 0xFF); // LSB

    HAL_SPI_Transmit(&hspi2, tx, 2, HAL_MAX_DELAY);
}

// 读1个byte
uint16_t SD_Read(void)
{
    uint8_t rx = SD_SPI_TxRx(0xFF);
    return (uint16_t)(0xFF00 | rx);
}

// 读2个字节
uint16_t SD_2Byte_Read(void)
{
    uint8_t rx[2];
    uint8_t tx[2] = {0xFF, 0xFF};

    HAL_SPI_TransmitReceive(&hspi2, tx, rx, 2, HAL_MAX_DELAY);
    return ((uint16_t)rx[0] << 8) | rx[1];
}

uint8_t SD_SendCmd(uint8_t cmd, uint32_t arg)
{
    uint8_t r1;
    uint8_t crc;

    if (cmd == 0)
        crc = 0x95;
    else if (cmd == 8)
        crc = 0x87;
    else
        crc = 0xFF;

    SD_CS_Low();

    SD_SPI_TxRx(0x40 | cmd);
    SD_SPI_TxRx(arg >> 24);
    SD_SPI_TxRx(arg >> 16);
    SD_SPI_TxRx(arg >> 8);
    SD_SPI_TxRx(arg);
    SD_SPI_TxRx(crc);

    for (uint32_t i = 0; i < 0xFFFF; i++)
    {
        r1 = SD_SPI_TxRx(0xFF);
        if (r1 != 0xFF)
            break;
    }

    return r1; // 原始 R1
}

#if 0
// 发送命令
uint16_t SD_CMD_Write(uint16_t CMDIndex, uint32_t CMDArg, uint16_t ResType, uint16_t CSLowRSV)
{
    uint16_t Response = 0xFFFF;
    uint16_t Response2;
    // 0095仅仅适合CMD0
    uint8_t crc = (CMDIndex == 0) ? 0x95 : 0xFF;

    SD_CS_Low();

    /* CMD + ARG */
    SD_SPI_TxRx(0x40 | CMDIndex);
    SD_SPI_TxRx((CMDArg >> 24) & 0xFF);
    SD_SPI_TxRx((CMDArg >> 16) & 0xFF);
    SD_SPI_TxRx((CMDArg >> 8) & 0xFF);
    SD_SPI_TxRx(CMDArg & 0xFF);
    SD_SPI_TxRx(crc);

    /* 等待响应 */
    switch (ResType)
    {
    case 1: /* R1 */
        do
        {
            Response = SD_SPI_TxRx(0xFF);
        } while (Response == 0xFF);
        Response |= 0xFF00;
        break;

    case 2: /* R1b */
        do
        {
            Response = SD_SPI_TxRx(0xFF);
        } while (Response == 0xFF);
        Response |= 0xFF00;

        /* busy */
        while (SD_SPI_TxRx(0xFF) == 0x00)
            ;
        break;

    case 3: /* R2 */
        Response = SD_SPI_TxRx(0xFF) << 8;
        Response |= SD_SPI_TxRx(0xFF);
        break;
    }

    if (!CSLowRSV)
        SD_CS_High();

    SD_SPI_TxRx(0xFF); /* extra clock */

    // DBG_PRINTF("SD CMD%d Response: 0x%04X\r\n", CMDIndex, Response);

    return Response;
}


uint8_t Read_Single_Block(uint32_t block)
{
    // uint16_t resp;
    uint8_t r1 = 0;

    SD_CS_Low();
    r1 = SD_SendCmd(17, addr);
    if (r1 != 0x00)
    {
        SD_CS_High();
        return r1;
    }

    if (!SD_Wait_DataToken(0xFE))
    {

        DBG_PRINTF("SD data token timeout\r\n");
        return SD_TIMEOUT;
    }

    for (int i = 0; i < 512; i++)
        ReadBuffer[i] = SD_SPI_TxRx(0xFF);

    SD_SPI_TxRx(0xFF); // CRC
    SD_SPI_TxRx(0xFF);

    SD_CS_High();
    SD_SPI_TxRx(0xFF);

    return SD_OK;
}

uint16_t Write_Single_Block(uint32_t block)
{
    uint16_t resp;

    resp = SD_CMD_Write(24, block, 1, 1);
    if (resp != SD_OK)
    {
        DBG_PRINTF("SD CMD24 failed, Response=0x%04X\r\n", resp);
        return resp;
    }

    SD_SPI_TxRx(0xFF);
    SD_SPI_TxRx(0xFE);

    for (int i = 0; i < 512; i++)
        SD_SPI_TxRx(WriteBuffer[i]);

    SD_SPI_TxRx(0xFF);
    SD_SPI_TxRx(0xFF);

    uint8_t token = SD_SPI_TxRx(0xFF);
    if ((token & 0x1F) != 0x05)
    {
        DBG_PRINTF("SD data response token error: 0x%02X\r\n", token);
        return SD_WRITE_ERR;
    }

    if (SD_Wait_NotBusy())
    {
        DBG_PRINTF("SD write busy timeout\r\n");
        return SD_TIMEOUT;
    }

    SD_CS_High();
    SD_SPI_TxRx(0xFF);

    return SD_OK;
}

uint16_t Write_Single_Block_(uint32_t BlockAddress)
{
    uint16_t Response;

    Response = SD_CMD_Write(24, BlockAddress, 1, 1);
    if (Response != 0xFF00)
    {
        DBG_PRINTF("SD CMD24 failed, Response=0x%04X\r\n", Response);
        return Response;
    }

    SD_SPI_TxRx(0xFF);
    SD_SPI_TxRx(0xFE); /* start token */

    for (int i = 0; i < 256; i++)
        SD_2Byte_Write(WriteBuffer[i % 128]);

    /* CRC dummy */
    SD_SPI_TxRx(0xFF);
    SD_SPI_TxRx(0xFF);

    /* data response */
    Response = SD_SPI_TxRx(0xFF);

    /* busy wait */
    while (SD_SPI_TxRx(0xFF) == 0x00)
    {
        // 喂狗
        HAL_IWDG_Refresh(&hiwdg);
    }

    SD_CS_High();
    SD_SPI_TxRx(0xFF);

    return Response;
}
#endif

uint8_t Read_Single_Block(uint32_t block, uint8_t *data)
{
    uint8_t r1;
    uint32_t addr = SD_BlockAddr(block);
    uint32_t timeout;

    SD_CS_Low();

    /* CMD17: READ_SINGLE_BLOCK */
    r1 = SD_SendCmd(17, addr);
    if (r1 != 0x00)
    {
        SD_CS_High();
        return r1;
    }

    /* wait for data token 0xFE */
    timeout = 100000;
    while (timeout--)
    {
        if (SD_SPI_TxRx(0xFF) == SD_TOKEN_DATA)
            break;
    }

    if (timeout == 0)
    {
        SD_CS_High();
        return SD_TIMEOUT;
    }

    /* read 512 bytes */
    for (int i = 0; i < 512; i++)
        data[i] = SD_SPI_TxRx(0xFF);

    /* discard CRC */
    SD_SPI_TxRx(0xFF);
    SD_SPI_TxRx(0xFF);

    SD_CS_High();
    SD_SPI_TxRx(0xFF); // extra clock

    return SD_OK;
}

uint8_t Write_Single_Block(uint32_t block, uint8_t *data)
{
    uint8_t r1;
    uint8_t token;
    uint32_t addr = SD_BlockAddr(block);

    SD_SPI_SetSlow();
    SD_CS_Low();

    /* CMD24: WRITE_SINGLE_BLOCK */
    r1 = SD_SendCmd(24, addr);
    if (r1 != 0x00)
    {
        DBG_PRINTF("SD CMD24 failed: %02X\r\n", r1);
        SD_CS_High();
        SD_SPI_SetFast();
        return r1;
    }

    /* send start token */
    SD_SPI_TxRx(0xFF);
    SD_SPI_TxRx(SD_TOKEN_START);

    /* send 512 bytes */
    for (int i = 0; i < BYTES_PER_BLOCK; i++)
        SD_SPI_TxRx(data[i]);

    /* dummy CRC */
    SD_SPI_TxRx(0xFF);
    SD_SPI_TxRx(0xFF);

    /* read data response token */
    token = SD_SPI_TxRx(0xFF);
    if ((token & 0x1F) != SD_DATA_ACCEPT)
    {
        SD_CS_High();
        DBG_PRINTF("SD data response token error: 0x%02X\r\n", token);
        SD_SPI_SetFast();
        return token; // CRC error / write error
    }

    /* wait until card not busy */
    if (SD_Wait_NotBusy())
    {

        SD_CS_High();
        DBG_PRINTF("SD write busy timeout\r\n");
        SD_SPI_SetFast();
        return SD_TIMEOUT;
    }

    SD_CS_High();
    SD_SPI_TxRx(0xFF); // extra clock
    SD_SPI_SetFast();

    return SD_OK;
}

uint8_t SD_Reset_Card(void)
{
    uint8_t r1;

    SD_CS_High();

    /* ≥74 clocks with MOSI high */
    for (int i = 0; i < 10; i++)
        SD_SPI_TxRx(0xFF);

    SD_CS_Low();
    r1 = SD_SendCmd(0, 0); // CMD0
    SD_CS_High();
    SD_SPI_TxRx(0xFF);

    return r1; // should be 0x01
}

uint8_t SD_Initiate_Card(void)
{
    uint8_t r1;
    uint32_t retry = 100;

    do
    {
        /* CMD55 */
        SD_CS_Low();
        r1 = SD_SendCmd(55, 0);
        SD_CS_High();
        SD_SPI_TxRx(0xFF);
        if (r1 > 0x01)
            return r1;

        /* ACMD41 (SDSC, no HCS) */
        SD_CS_Low();
        r1 = SD_SendCmd(41, 0);
        SD_CS_High();
        SD_SPI_TxRx(0xFF);

        if (r1 == 0x00)
            return SD_OK;

        HAL_Delay(10);

    } while (--retry);

    return SD_TIMEOUT;
}

uint8_t SD_Get_CardInfo(void)
{
    uint8_t r1;
    uint8_t csd[16];
    uint32_t timeout;

    SD_CS_Low();
    r1 = SD_SendCmd(9, 0); // CMD9
    if (r1 != 0x00)
    {
        SD_CS_High();
        return r1;
    }

    /* wait data token 0xFE */
    timeout = 100000;
    while (timeout--)
    {
        if (SD_SPI_TxRx(0xFF) == 0xFE)
            break;
    }
    if (timeout == 0)
    {
        SD_CS_High();
        return SD_TIMEOUT;
    }

    /* read CSD (16 bytes) */
    for (int i = 0; i < 16; i++)
        csd[i] = SD_SPI_TxRx(0xFF);

    /* discard CRC */
    SD_SPI_TxRx(0xFF);
    SD_SPI_TxRx(0xFF);

    SD_CS_High();
    SD_SPI_TxRx(0xFF);

    /* -------- parse CSD v1 (SDSC) -------- */
    uint8_t read_bl_len = csd[5] & 0x0F;
    uint32_t c_size =
        ((uint32_t)(csd[6] & 0x03) << 10) |
        ((uint32_t)csd[7] << 2) |
        ((csd[8] & 0xC0) >> 6);

    uint8_t c_mult =
        ((csd[9] & 0x03) << 1) |
        ((csd[10] & 0x80) >> 7);

    uint32_t block_len = 1UL << read_bl_len;
    uint32_t mult = 1UL << (c_mult + 2);
    uint32_t capacity = (c_size + 1) * mult * block_len;

    SD_BlockNR = capacity / 512;
    sd_is_sdhc = 0;

    DBG_PRINTF("SDSC capacity: %lu bytes (%lu blocks)\r\n", capacity, SD_BlockNR);

    return SD_OK;
}

void SD_SPI_SetSlow(void)
{
    HAL_SPI_DeInit(&hspi2);

    hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128; // SPI_BAUDRATEPRESCALER_128; // 或 256
    HAL_SPI_Init(&hspi2);
}

void SD_SPI_SetFast(void)
{
    HAL_SPI_DeInit(&hspi2);

    hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8; // 或 4
    HAL_SPI_Init(&hspi2);
}

uint8_t SD_Init_SDSC(void)
{
    uint8_t r1;
    delay_us(1000);

    SD_SPI_SetSlow();

    /* CMD0 */
    r1 = SD_Reset_Card();
    if (r1 != SD_IDLE)
    {
        DBG_PRINTF("CMD0 failed: %02X\r\n", r1);
        goto error;
    }

    /* ACMD41 */
    r1 = SD_Initiate_Card();
    if (r1 != SD_OK)
    {
        DBG_PRINTF("ACMD41 failed\r\n");
        goto error;
    }

    /* CMD9 */
    r1 = SD_Get_CardInfo();
    if (r1 != SD_OK)
    {
        DBG_PRINTF("CMD9 failed\r\n");
        goto error;
    }

    /* CMD16: set block length = 512 (SDSC only) */
    SD_CS_Low();
    r1 = SD_SendCmd(16, 512);
    SD_CS_High();
    SD_SPI_TxRx(0xFF);

    if (r1 != 0x00)
    {
        DBG_PRINTF("CMD16 failed: %02X\r\n", r1);
        goto error;
    }

    SD_SPI_SetFast();
    return SD_OK;

error:
    SD_SPI_SetFast();
    return SD_ERR;
}

void test_sd_card(int32_t block)
{
    DBG_PRINTF("sd_is_sdhc = %d\r\n", sd_is_sdhc);
    for (int i = 0; i < 512; i++)
        WriteBuffer[i] = i & 0xFF;

    SD_SPI_SetSlow();
    if (Write_Single_Block(block, WriteBuffer) != SD_OK)
    {
        DBG_PRINTF("Write failed\r\n");
        return;
    }
    SD_SPI_SetFast();

    // SD_Wait_NotBusy();
    // HAL_Delay(5); // 1~5ms 都可以，先保守

    if (Read_Single_Block(block, ReadBuffer) != SD_OK)
    {
        DBG_PRINTF("Read failed\r\n");
        return;
    }

    for (int i = 0; i < 512; i++)
    {
        if (ReadBuffer[i] != WriteBuffer[i])
        {
            DBG_PRINTF("Mismatch at %d\r\n", i);
            return;
        }
    }

    DBG_PRINTF("SD block %lu OK\r\n", block);
}

static sd_log_page_t sd_log_page = {0};

void look_for_used_block_test(void)
{
    // step 1. write blog with 100 sd_log_page_t

    for (uint32_t i = 0; i < 100; i++)
    {
        sd_log_page.magic = SD_LOG_MAGIC;
        sd_log_page.seq++;
        sd_log_page.timestamp = HAL_GetTick();
        for (uint32_t j = 0; j < TOTAL_POINT_NUM; j++)
        {
            sd_log_page.data_1.sensor_data[j] = (uint8_t)(i + j);
            sd_log_page.data_2.sensor_data[j] = (uint8_t)(i + j + 1);
            sd_log_page.data_3.sensor_data[j] = (uint8_t)(i + j + 2);
        }

        // crc
        sd_log_page.crc = crc32_soft((uint8_t *)&sd_log_page.data_1, (sizeof(sd_raw_data_t) * 3));

        // 写入 sd_log_page 到 SD 卡
        //  memcpy(WriteBuffer, &sd_log_page, sizeof(sd_log_page_t));

        if (Write_Single_Block(i, (uint8_t *)&sd_log_page) != SD_OK)
        {
            DBG_PRINTF("Write block %lu failed\r\n", i);
            return;
        }

        memset(&sd_log_page, 0, sizeof(sd_log_page_t));

        HAL_IWDG_Refresh(&hiwdg);
    }

    memset(&sd_log_page, 0, sizeof(sd_log_page_t));

    // step 2. 依次读取, 确定最后一个有效 block
    int32_t last_used_block = -1;
    for (uint32_t i = 0; i < SD_BlockNR; i++)
    {
        if (Read_Single_Block(i, (uint8_t *)&sd_log_page) != SD_OK)
        {
            DBG_PRINTF("Read block %lu failed\r\n", i);
            return;
        }

        if (sd_log_page.magic != SD_LOG_MAGIC)
        {
            DBG_PRINTF("Block %lu: invalid magic: 0x%08lX\r\n", i, sd_log_page.magic);
            break;
        }

        last_used_block = i;
    }
}
