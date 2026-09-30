#include "stm32f10x.h"
#include "i2c.h"
#include "ssd1306.h"


static void Delay_ms(uint32_t ms)
{
    volatile uint32_t i;
    volatile uint32_t j;

    for (i = 0; i < ms; i++)
    {
        for (j = 0; j < 7200; j++)
        {
            __NOP();
        }
    }
}


/*
 * Draw rectangle
 */
static void DrawRectangle(uint8_t x1,
                          uint8_t y1,
                          uint8_t x2,
                          uint8_t y2)
{
    uint8_t x;
    uint8_t y;

    for (x = x1; x <= x2; x++)
    {
        SSD1306_SetPixel(x, y1, 1);
        SSD1306_SetPixel(x, y2, 1);
    }

    for (y = y1; y <= y2; y++)
    {
        SSD1306_SetPixel(x1, y, 1);
        SSD1306_SetPixel(x2, y, 1);
    }
}


/*
 * Draw filled circle
 */
static void DrawCircle(uint8_t cx,
                       uint8_t cy,
                       uint8_t radius)
{
    int16_t x;
    int16_t y;

    for (x = -radius; x <= radius; x++)
    {
        for (y = -radius; y <= radius; y++)
        {
            if ((x * x + y * y) <=
                (radius * radius))
            {
                int16_t px = cx + x;
                int16_t py = cy + y;

                if (px >= 0 && px < 128 &&
                    py >= 0 && py < 64)
                {
                    SSD1306_SetPixel(
                        (uint8_t)px,
                        (uint8_t)py,
                        1
                    );
                }
            }
        }
    }
}


/*
 * Draw simple black/white image
 */
static void DrawImage(void)
{
    uint8_t x;
    uint8_t y;

    SSD1306_Clear();

    /*
     * Outer border
     */
    DrawRectangle(
        0,
        0,
        127,
        63
    );

    /*
     * Face outline
     */
    for (x = 30; x < 98; x++)
    {
        for (y = 10; y < 54; y++)
        {
            int16_t dx = x - 64;
            int16_t dy = y - 32;

            /*
             * Ellipse equation
             */
            if (((dx * dx) * 4 +
                 (dy * dy) * 9) <=
                (34 * 34 * 4))
            {
                /*
                 * Keep only outline
                 */
                if (((dx * dx) * 4 +
                     (dy * dy) * 9) >=
                    (31 * 31 * 4))
                {
                    SSD1306_SetPixel(
                        x,
                        y,
                        1
                    );
                }
            }
        }
    }

    /*
     * Left eye
     */
    DrawCircle(
        50,
        27,
        3
    );

    /*
     * Right eye
     */
    DrawCircle(
        78,
        27,
        3
    );

    /*
     * Smile
     */
    for (x = 48; x <= 80; x++)
    {
        y = 43;

        if (x >= 52 && x <= 76)
        {
            int16_t dx = x - 64;

            y = 43 +
                (uint8_t)((dx * dx) / 80);
        }

        SSD1306_SetPixel(
            x,
            y,
            1
        );
    }

    /*
     * Send image to OLED
     */
    SSD1306_UpdateScreen();
}


int main(void)
{
    /*
     * System clock is configured by
     * system_stm32f10x.c
     */
    SystemInit();

    /*
     * Initialize I2C1
     *
     * PB6 -> SCL
     * PB7 -> SDA
     */
    I2C1_Config();

    /*
     * Initialize SSD1306
     */
    SSD1306_Init();

    /*
     * Draw black/white image
     */
    DrawImage();

    while (1)
    {
        /*
         * Keep displaying image
         */
        Delay_ms(1000);
    }
}
