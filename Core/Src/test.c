#include <stdint.h>

#define MAX_POINTS 256 // 16×16，按你现在的使用场景

uint16_t g_width = 16;
uint16_t g_height = 16;
uint16_t g_value = 255;

/* 输入 & 输出数组（一维） */
uint16_t g_pointArr[MAX_POINTS];
uint16_t g_wsPointData[MAX_POINTS];

/* 行 / 列求和用 */
uint32_t g_colArr[16]; // 最大 height = 16

typedef enum
{
    PRESS_BY_ROW = 0,
    PRESS_BY_COL = 1
} press_type_t;

void pressNew1220_c(press_type_t type)
{
    uint32_t i, j;
    uint32_t den;

    /* 先拷贝输入数组 */
    for (i = 0; i < g_width * g_height; i++)
    {
        g_wsPointData[i] = g_pointArr[i];
    }

    if (type == PRESS_BY_ROW)
    {
        /* -------- row 模式：按行求和 -------- */
        for (i = 0; i < g_height; i++)
        {
            uint32_t total = 0;
            for (j = 0; j < g_width; j++)
            {
                total += g_wsPointData[i * g_width + j];
            }
            g_colArr[i] = total;
        }

        /* -------- row 模式：更新数据 -------- */
        for (i = 0; i < g_height; i++)
        {
            for (j = 0; j < g_width; j++)
            {
                uint32_t idx = i * g_width + j;

                den = g_wsPointData[idx] + g_value - g_colArr[i];
                if (den <= 0)
                    den = 1;

                g_wsPointData[idx] =
                    (uint16_t)((g_wsPointData[idx] * g_value) / den);
            }
        }
    }
    else
    {
        /* -------- col 模式：按列求和 -------- */
        for (i = 0; i < g_height; i++)
        {
            uint32_t total = 0;
            for (j = 0; j < g_width; j++)
            {
                total += g_wsPointData[j * g_height + i];
            }
            g_colArr[i] = total;
        }

        /* -------- col 模式：更新数据 -------- */
        for (i = 0; i < g_height; i++)
        {
            for (j = 0; j < g_width; j++)
            {
                uint32_t idx = j * g_height + i;

                den = g_wsPointData[idx] + g_value - g_colArr[i];
                if (den <= 0)
                    den = 1;

                g_wsPointData[idx] =
                    (uint16_t)((g_wsPointData[idx] * g_value) / den / 2);
            }
        }
    }
}

// 调用
void example_run(void)
{
    g_width = 16;
    g_height = 16;
    g_value = 255;

    /* 假设 g_pointArr 已经填好数据 */

    pressNew1220_c(PRESS_BY_COL);

    /* 结果在 g_wsPointData[] */
}
