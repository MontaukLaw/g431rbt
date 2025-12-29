#include "user_comm.h"

#define WIDTH_MAX 16
#define HEIGHT_MAX 16

// 中间缓存用
static int32_t g_lineSum[HEIGHT_MAX > WIDTH_MAX ? HEIGHT_MAX : WIDTH_MAX];

// 0=row, 1=col
void press256(const uint8_t *in, uint8_t *out, int width, int height, int value, int type)
{
    int size = width * height;

    // 拷贝输入到输出
    for (int i = 0; i < size; i++)
    {
        out[i] = in[i];
    }

    // ----------------------------
    // 按行处理
    // ----------------------------
    if (type == BY_ROW)
    {
        // 计算每行 sum
        for (int r = 0; r < height; r++)
        {
            int32_t sum = 0;
            for (int c = 0; c < width; c++)
            {
                sum += out[r * width + c];
            }
            g_lineSum[r] = sum;
        }

        // 平滑
        for (int r = 0; r < height; r++)
        {
            int32_t sum = g_lineSum[r];
            for (int c = 0; c < width; c++)
            {
                int idx = r * width + c;
                int oldv = out[idx];

                int32_t newv = oldv + (sum - oldv) / value;

                // clamp 0~255
                if (newv < 0)
                    newv = 0;
                else if (newv > 255)
                    newv = 255;

                out[idx] = (uint8_t)newv;
            }
        }
    }

    // ----------------------------
    // 按列处理
    // ----------------------------
    else if (type == BY_COL)
    {
        // 计算每列 sum
        for (int c = 0; c < width; c++)
        {
            int32_t sum = 0;
            for (int r = 0; r < height; r++)
            {
                sum += out[r * width + c];
            }
            g_lineSum[c] = sum;
        }

        // 平滑
        for (int c = 0; c < width; c++)
        {
            int32_t sum = g_lineSum[c];
            for (int r = 0; r < height; r++)
            {
                int idx = r * width + c;
                int oldv = out[idx];

                if (oldv < 6)
                    continue; // JS 的规则

                int32_t newv = oldv + (sum - oldv) / value;

                if (newv < 0)
                    newv = 0;
                else if (newv > 255)
                    newv = 255;

                out[idx] = (uint8_t)newv;
            }
        }
    }
}
